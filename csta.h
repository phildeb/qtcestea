#ifndef CSTA_H
#define CSTA_H

#include "common.h"
#include <csta_const.h>
#include <utils.h>
#include <sip.h>
#include <rtp.h>
#include <QMap>
#include <QString>

#define CSTA_BUF_MAX_SIZE 1024

class CCSTACommand{
public:
    static BYTE                         invokeid;
    static QMap<BYTE,QByteArray>        _extensionByInvokeid;
    static QMap<QByteArray,QByteArray>  _crossrefByextension;

    CCSTACommand(){memset(_internal_buf,0,sizeof(_internal_buf));_internal_buf_size=0;}
    CCSTACommand(BYTE* data,unsigned int len){
        _internal_buf_size = len;
        //if ( len > sizeof(_internal_buf) ) _internal_buf_size = sizeof(_internal_buf)-1;
        if ( data!=NULL && _internal_buf_size>0 )
            memcpy(_internal_buf,data,_internal_buf_size);
    }
    ~CCSTACommand(){}

    void parseEvent();

    int startIpRecording(QString extension,QString str_ipaddress,unsigned short port1,unsigned short port2);
    int stopIpRecording(QString extension,QString str_ipaddress);
    int stopMonitorExtension(QString extension);//char crossrefid1,char crossrefid2,char crossrefid3,char crossrefid4);
    int startMonitorExtension(QString extension);

    int superviseResponse(BYTE invokeid1,BYTE invokeid2);
    void parseCstaReportEvent();
    void parseCstaMonitorStartResponse();
    void parseCstaMonitorStopResponse();
    void hexdump();
    bool checkInternalPointedArea(BYTE* ptrField,unsigned short lenField);

    BYTE            _internal_buf[CSTA_BUF_MAX_SIZE];
    unsigned short  _internal_buf_size;
    QByteArray      _originalData;
    QByteArray      _data;

    QString         _answeringDevice;
    QString         _callingDevice;

    unsigned short  _lastReportEventInvokeID;
    int             _lastReportEventcrossRefIdLen;
    QByteArray      _lastReportEventcrossRefId;

    // TAG Type/lenght/Vlaue
    QByteArray      _TLVValue;
    int             _TLVLength;
    int             _TLVTag;
    BYTE* TLVReadValue(BYTE* ptr_in, int len);
    BYTE* TLVReadLength(BYTE* ptrField,int* tagV,int* tagL);
    BYTE* TLVReadTag(BYTE* ptr_in, int* tagV,int* tagL);
    const char* tag_to_string(int tag);
private:
};

/*class SipSubscriber{
public:
    QString                 _sip_proxy_ip_addr;
    SOCKET                  _sip_rtp_tx_sock;
    int                     _sip_rtp_tx_port;
};*/

struct CSocketPort{
    SOCKET                  _sip_rtp_tx_sock;
    int                     _sip_rtp_tx_port;
};

#define MAX_SUBSCRIBER 5

class CCSTARTPRecvInOutThread: public QThread
{
public:
    CCSTARTPRecvInOutThread(){
        _isStopped=false; _sipSubscribersCount = 0;
        for (int i=0; i<MAX_SUBSCRIBER; i++ ){
            _sipSubscribersArray[i]._sip_rtp_tx_port =0;
            _sipSubscribersArray[i]._sip_rtp_tx_sock =0;
        }
    }

    ~CCSTARTPRecvInOutThread(){}
    bool init(QString csta_client_ip,int rtp_port1,int rtp_port2,SOCKET sip_rtp_socket,int sip_rtp_tx_port, QString sip_proxy_ip_addr );
    void run();
    void stop();

    bool                    _isStopped;
    char                    _strwavFilename[256];

    QString                 _extension_recorded;
    QString                 _rtp_listen_ip_addr;
    int                     _rtpPort1;
    int                     _rtpPort2;
    SOCKET                  _rtp_rx_sock1;
    SOCKET                  _rtp_rx_sock2;

    time_t                  _ltime;
    struct timeval          _tbegin;
    struct timeval          _tend;
    double                  _texec;
    struct addrinfo         _hints;

    // transmit rtp packet to sip user agent
    QString                 _sip_proxy_ip_addr; // the same asterisk for all subsciber !

    SOCKET                  _sip_rtp_tx_sock;
    quint16                 _sip_rtp_tx_port;

    // manage more than 2 listener
    CSocketPort             _sipSubscribersArray[MAX_SUBSCRIBER];
    quint16                 _sipSubscribersCount;
    bool addSubscriber(SOCKET sock, quint16 port){
        for (int i=0; i<MAX_SUBSCRIBER; i++ ){
            if ( _sipSubscribersArray[i]._sip_rtp_tx_port == 0 ){
                _sipSubscribersArray[i]._sip_rtp_tx_port = port;
                _sipSubscribersArray[i]._sip_rtp_tx_sock = sock;
                _sipSubscribersCount++;
                qDebug() << _sipSubscribersCount << " addSubscriber port " << _sipSubscribersArray[i]._sip_rtp_tx_port << " sock " <<_sipSubscribersArray[i]._sip_rtp_tx_sock;
                return true;
                break;
            }
        }
        return false;
    }
    bool removeSubscriber(SOCKET sock, quint16 port){
        for (int i=0; i<MAX_SUBSCRIBER; i++ ){
            if ( _sipSubscribersArray[i]._sip_rtp_tx_port == port ){
                _sipSubscribersArray[i]._sip_rtp_tx_port = 0;
                _sipSubscribersArray[i]._sip_rtp_tx_sock = 0;
                _sipSubscribersCount--;
                qDebug() << _sipSubscribersCount << " removeSubscriber port " << _sipSubscribersArray[i]._sip_rtp_tx_port << " sock " <<_sipSubscribersArray[i]._sip_rtp_tx_sock;
                return true;
                break;
            }
        }
        return false;
    }
    //QList<SipSubscriber*>   _sipSubscribersList;
    //QMutex                  _sipSubscribersListMutex;
};

class CCSTALink : public QThread
{
public:

    CCSTALink(){
        _isConnected=false;
        _is_stopped=false;
        _commandsToSend.setMax(100);
        _commandsReceived.setMax(100);
        CSTA_RTP_PORT_START = 40000;
        CSTA_RTP_PORT_END = 60000;
        _next_csta_rtp_port_pair=CSTA_RTP_PORT_START;
    }

    // params.ini
    quint16                     CSTA_RTP_PORT_START;
    quint16                     CSTA_RTP_PORT_END;
    quint16                     _OXE_CSTA_PORT;
    QString                     _OXE_IP_ADDRESS;
    QString                     _CSTA_local_ip_addr; /* OXE server address */

    bool                        _is_stopped;
    pthread_t                   _csta_thread_id;
    bool                        _isConnected;
    int                         _sock;
    struct sockaddr_in          _oxe_server;

    quint16                     _next_csta_rtp_port_pair;
    // link keep alive with status report inquiry from oxe every 30 seconds
    int                         _lastReportStatusCrossRefIdLen;
    QByteArray                  _lastReportStatusCrossRefId;

    QAsyncQueue<CCSTACommand*>  _commandsToSend;
    QAsyncQueue<CCSTACommand*>  _commandsReceived;
    QMap<QString,CCSTARTPRecvInOutThread*>   _rtpThreadByextension;
    //QMap<QString,QString>   _extensionsRecordedBySipCaller;

    void    processReceivedCommand(CCSTACommand* pCmd);
    void    run();
    bool    init();
    bool    startNetwork();
    bool    stopNetwork();
    bool    startAuth();
    void    clearAudioBuffer();
    void    startMonitor(QString extension, QString sip_proxy_ip_addr, int rtp_playing_sound_udp_port,SOCKET rtp_udp_send_socket);
    void    stopMonitor(QString extension, QString sip_proxy_ip_addr, int rtp_playing_sound_udp_port,SOCKET rtp_udp_send_socket);
};

#endif // CSTA_H

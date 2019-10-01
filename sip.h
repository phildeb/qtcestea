#ifndef SIP_H
#define SIP_H

#include "common.h"
#include "utils.h"
#include "rtp.h"
using namespace std; // std:string

#define SIP_PORT_DEFAULT 5060
#define SIP_MSG_MAX_LEN (1*1024)
#define RTP_ALAW_PACKET_LEN 172
#define CALLID_LEN 6
#define TAG_BRANCH_LEN 6

#define EXTENSION_MIN_LEN 4 /* phone extension must be 4 digits minimum */

class CSipMessage
{
public:
    CSipMessage(){
        _isConsummed = false;
        _pBufLen = SIP_MSG_MAX_LEN;
        _pBuf = (BYTE*)malloc(_pBufLen);

        _pBufLenSDP = SIP_MSG_MAX_LEN;
        _pBufSDP = (BYTE*)malloc(_pBufLenSDP);
    }
    CSipMessage(BYTE* data,unsigned int dataLen){
        _isConsummed = false;
        //_buf.open(QBuffer::::Write);_buf.setData((const char*)data,dataLen);
        if ( dataLen>0 ){
            _pBufLen = dataLen;
            _pBuf = (BYTE*)malloc(_pBufLen);
            if ( _pBuf ){
                memcpy(_pBuf ,data, _pBufLen);
            }
        }
        _pBufLenSDP = 0;
        _pBufSDP = NULL;
    }
    ~CSipMessage(){
        if (_pBuf) free(_pBuf);
    }
    bool parse();

    int         _isConsummed;        // has been proceed by an agent, do not send to others agents

    QString     _startLine;
    QString     _methodLine;    // exemple INVITE REGISTER CANCEL...
    int         _statusCode;        // exemple 401
    bool        _isSipResponse;    // exemple SIP/2.0 401 Unauthorized
    QString     _reasonPhrase;  // exemple Unauthorized

    QString     _cSeqLine;
    QString     _methodCSeq;
    int         _numberCSeq;

    QString     _authNonce;
    QString     _authAlgo;

    QString     _callID;
    QString     _callid_line;

    QString     _toLine;
    QString     _toTag;

    QString     _fromLine;
    QString     _fromTag;

    QString     _viaLine;

    int         _rtp_port;
    int         _codec;

    // header SIP
    QList<QString> _sipFieldList;
    void setMethod(QString str_method,QString str_phone_number_called,QString sip_proxy_address);
    void setRegister(QString sip_proxy_address);
    void setInvite(QString str_phone_number_called,QString sip_proxy_address);
    void setVia(QString str_ip_caller,QString via_tag_branch);
    void setAck(QString called_number,QString sip_proxy_address);
    void setCallId(QString call_id);
    void setFrom(QString phone_name, QString phone_number,QString sip_proxy_address,QString from_tag);
    void setTo(QString phone_name, QString phone_number,QString sip_proxy_address,QString to_tag);
    void setCSeq(int cseq, QString method);
    void setContact(QString phone_number,QString sip_proxy_address);
    void setBye(QString str_phone_number,QString sip_proxy_address);

    // header SDP
    QList<QString> _sdpFieldList;
    void setSDP_audio(quint16 audio_rtp_port);
    void setSDP_c(QString str_ip_caller);
    void setSDP_o(QString str_ip_caller);

    void prepareBuffer();
    void prepareBufferWithSDP();

    BYTE*           _pBufSDP;
    BYTE*           _pBuf;

    unsigned int    _preparedLenSDP;
    unsigned int    _pBufLenSDP;
    unsigned int    _preparedLen;
    unsigned int    _pBufLen;

    int str_append_CRLF(const BYTE* stringtocopy);
    int str_append_CRLF_SDP(const BYTE* stringtocopy);
};

#if 1
class CRTPSendThread: public QThread
{
public:
    bool init(QString filepath,int rtp_port, QString sip_proxy_ip_addr);
    void run();
    void stop();

    CVoicePacket* _currentVoicePacketIn;
    CVoicePacket* _currentVoicePacketOut;
    bool    _isStopped;

    QString _wavFilepath;
    FILE*   _fd;

    // rtp sending
    int     _rtpPort;
    SOCKET	_rtp_udp_send_socket;
    int     _rtp_tx_sequence_number;
    QString _rtp_proxy_ip_addr;

    unsigned long   _my_timestamp;
    struct timeval tbegin;
    struct timeval tend;
    double texec;
};
#endif

enum USER_AGENT_STATE{
    USER_AGENT_IDLE=1
    ,USER_AGENT_RINGING=2
    ,USER_AGENT_ANSWERING
    ,USER_AGENT_INITIATE_OUTGOING
    ,USER_AGENT_IN_INCOMING_COMM
    ,USER_AGENT_IN_OUTGOING_COMM
};
class CUserAgentGroup;

class CUserAgent //: public QThread
{
public:
    CUserAgent(CUserAgentGroup* pGroup){_pUaGroup = pGroup;_autoAnswer=true; _is_registered=false;}
    ~CUserAgent(){}    
    bool init(QString sip_proxy_address,QString local_address,QString phone, QString pwd);

    int genRandomRtpPort();
    int genRandomSequenceNumber(int nb_digits);
    QString genRandomCallID(QString prefix);
    QString genRandomBranch(QString suffix=NULL); // branch cookie starting with z9hG4bK
    QString genRandomTag();

    bool sipRegister();
    bool sipRegisterChallenge(CSipMessage* pSipMsg,QString call_id,const char* nonce,int sec);
    bool sipUnRegister();
    void checkRegistration();

    // incoming call
    bool sipByeIncomingCall();
    void sip_200OK_invite_incoming(CSipMessage* pSipMsg);

    // outgoing call
    bool sipInvite();
    void sip_INVITE_with_auth(char* digest);
    bool sip_ack_invite_200_OK_outgoing(CSipMessage* pSipMsg);
    bool sip_ack_bye_200_OK_outgoing(char* to_tag,int cseq);
    void sip_ack_invite_401_outgoing(char* to_tag);
    bool sipByeOutgoingCall();

    // incoming or outgoing call end
    bool sipAckBye(QString callid,int numberCSeq, const char* to_line, const char* from_line, const char* via_line);

    int  send(CSipMessage* pmsg);

    // call state machine
    void unregisteredState(CSipMessage*);
    void idleState(CSipMessage*);
    void ringingState(CSipMessage*);
    void initiateState(CSipMessage*);
    void incomingCommState(CSipMessage*);
    void outgoingCommState(CSipMessage*);
    void changeState(USER_AGENT_STATE new_state);
    USER_AGENT_STATE _state;


    CUserAgentGroup*    _pUaGroup; // manage all SIP packet sending and receiving
    QString             _to_tag;
    QString             _to_line;
    QString             _from_tag;
    QString             _from_line;
    QString             _via_line;
    quint16             _bye_cseq;

    // outgoing call
    QString             _dialing_call_id;
    quint16             _dialing_cseq;
    QString             _dialing_branch_tag_via;
    QString             _dialing_from_tag;
    QString             _current_outgoing_call_id;
    QDateTime           _outgoing_established_date_time;

    // incoming call
    bool                _autoAnswer;
    QString             _ringing_call_id;
    int                 _ringingInviteMessageRtpPort;
    QString             _current_incoming_call_id;
    QDateTime           _incoming_established_date_time;

    // registration
    QString             _current_register_call_id;
    QString             _current_unregister_call_id;
    QString             _unregister_tag_from;
    QString             _register_tag_from;
    QString             _register_branch_tag_via;
    QString             _unregister_branch_tag_via;
    QString             _register_tag_to_received;
    int                 _register_cseq;
    QDateTime           _registered_date_time;
    QDateTime           _unregistered_date_time;
    bool                _is_registered;

    CRTPSendThread      _rtp_playing_sound_thread;// playing sound thru rtp port
    int                 _rtp_playing_sound_udp_port; // tx rtp packet on this port to proxy SIP
    SOCKET              _rtp_udp_send_socket;    

    // params.ini
    QString             SIP_PROXY_ADDR;
    QString             _local_address;
    QString             _phone_number;
    QString             _md5_password;
    quint16             _rtp_port_answer;
    quint16             _rtp_port_dial;
    QString             _called_number;

    // csta
    QString             _csta_extension_recorded;

public slots:
    void onSIPMessage(CSipMessage*);
};

class CCSTALink;

class CUserAgentGroup : public QThread // main SIP message thread and list of agent management
{
public:
    CUserAgentGroup(){_myCSTAlink = NULL;}
    ~CUserAgentGroup();
    QList<CUserAgent*> _uaList;

    void run();
    void startNetwork();
    void startRegistration();
    void stopRegistration();
    void stopNetwork();
    void writeSettings();
    void readSettings();

    // csta
    void setCstaLink(CCSTALink* ptr){_myCSTAlink = ptr;}
    CCSTALink*                  _myCSTAlink;

    bool                        _is_stopped;
    QDateTime                   _start_date_time;
    QDateTime                   _registration_date_time;

    // params.ini
    QString                     _sip_proxy_address;//QString             SIP_PROXY_ADDR;
    QString                     _local_address;

    // sip udp port 5060 sending and receiving
    pthread_t                   _udp_sip_t_id;
    struct sockaddr_in          _asteriskServAddr; /* Asterisk server address */
    SOCKET                      _sip_recv_send_socket;
    QAsyncQueue<CSipMessage*>   _sipMsgToSend;
    QAsyncQueue<CSipMessage*>   _sipMsgReceived;
};
#if 0
typedef struct {
    /*time_t start_time;
    time_t stop_time;
    char to_uri[URI_MAX_SIZE];
    char from_uri[URI_MAX_SIZE];
    char from_tag[TAG_MAX_SIZE];
    int rtp_tcp_port;
    int rtp_udp_port;
    int rtcp_tcp_port;
    int rtcp_udp_port;
    int sip_tcp_port;
    int sip_udp_port;
    //pthread_t		udp_rtp_rx_handle;
    //int cseq;
    //char contact[URI_MAX_SIZE];
    //char call_id[CALL_ID_MAX_SIZE];*/
    // SIP
    SOCKET		udp_send_socket;
    char to_tag[TAG_MAX_SIZE];
    // RTP
    SOCKET		rtp_recv_socket;
    SOCKET		rtp_send_socket2;
    int			rtp_port_tx; // tx rtp packet on this port to proxy SIP
    int			rtp_port_rx; // rx rtp packet on this port from proxy SIP
}sip_call_t;

int rtp_send(sip_call_t* sip_call, const char* ptr_udp_data, int len_udp_data);
int udp_send(sip_call_t* sip_call, const char* ptr_udp_data, int len_udp_data);
void init_call(sip_call_t* sip_call);
void sip_INVITE(sip_call_t* sip_call);
void sip_ANSWER(sip_call_t* sip_call);
void sip_bye(sip_call_t* sip_call);
#endif

#define UDP_SIP_PORT 5060
#define MAX_SIMULTANEOUS_COMM 2500
//#define MAX_RTP_PORT 65534
//#define MIN_RTP_PORT 1024

// buffering pour eviter de saturer le disque
#define MAX_AUDIO_LEN (5000) /* packets de 160 octets en loi A et de 20 octets en g729 ; 8000 octets/sec en loi A et 1000 octets/sec en g729 */
/* 20 octets = 160 miliseconds en g729 */
#define MAX_FNAME 256
#define SEGMENT_DURATION 15 /* x secondes dans chaque fichier pour ecoute temps reelle */

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518
/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14
/* Ethernet addresses are 6 bytes */
//#define ETHER_ADDR_LEN  6 /usr/include/net/ethernet.h:60:0: note: this is the location of the previous definition

/* Ethernet header */
struct sniff_ethernet {
    u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
    u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
    u_short ether_type;                     /* IP? ARP? RARP? etc */
};

/* IP header */
struct sniff_ip {
    u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
    u_char  ip_tos;                 /* type of service */
    u_short ip_len;                 /* total length */
    u_short ip_id;                  /* identification */
    u_short ip_off;                 /* fragment offset field */
#define IP_RF 0x8000            /* reserved fragment flag */
#define IP_DF 0x4000            /* dont fragment flag */
#define IP_MF 0x2000            /* more fragments flag */
#define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
    u_char  ip_ttl;                 /* time to live */
    u_char  ip_p;                   /* protocol */
    u_short ip_sum;                 /* checksum */
    struct  in_addr ip_src,ip_dst;  /* source and dest address */
};
#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
    u_short th_sport;               /* source port */
    u_short th_dport;               /* destination port */
    tcp_seq th_seq;                 /* sequence number */
    tcp_seq th_ack;                 /* acknowledgement number */
    u_char  th_offx2;               /* data offset, rsvd */
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
    u_char  th_flags;
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_ECE  0x40
#define TH_CWR  0x80
#define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
    u_short th_win;                 /* window */
    u_short th_sum;                 /* checksum */
    u_short th_urp;                 /* urgent pointer */
};

typedef struct{
    int active;

    string callid; // uniq callid
    string str_from; // contient le from
    string str_to; // contient le to
    string str_contact; // contient le contact
    string str_codec;// "8" ou "0" ou "18"

    int codec;
    int ssrc_in;
    int ssrc_out;
    int expected_seq_in;
    int expected_seq_out;
    int prev_ts_in;
    int prev_ts_out;

    int last_floor_sec;
    int spying;

    char horodate_200_OK[64];
    char horodate_BYE[64];

    char fname_in[MAX_FNAME];
    char fname_out[MAX_FNAME];
    char fname_realtime_in[MAX_FNAME];
    char fname_realtime_out[MAX_FNAME];
    char fname_rtp_in[MAX_FNAME];
    char fname_rtp_out[MAX_FNAME];
    char fname_sig[MAX_FNAME];

    //int realtime_segment_in;
    //int realtime_segment_out;
    int realtime_segment;
    int passage_seconde;

    unsigned int rtp_in;
    unsigned int rtp_out;

    time_t ts_200OK;
    struct timeval tm_200OK;
    time_t ts_BYE;

    int ts_last_rtp_in;
    int ts_last_rtp_out;

    int nb_rtp_bytes_in;
    int nb_rtp_bytes_out;

    struct timeval prec_in;
    struct timeval now_in;

    struct timeval prec_out;
    struct timeval now_out;

    char audio_buf_in[ MAX_AUDIO_LEN];
    int audio_len_in;

    char unused1[128];

    char audio_buf_out[ MAX_AUDIO_LEN];
    int audio_len_out;

    char unused2[128];
}TComm;
/*
typedef std::pair<std::string, std::string> TStrStrPair;
typedef map<string,string> dialog_map;
typedef vector<dialog_map> TSipDialogArr; // un vecteur de map <keyvalue> SIP dans lequel on fait ptr->push_back(mydialog) a chaque paquet SIP recu
typedef map<string, TSipDialogArr*> TCallMap; // le premier string est le call-id 2dfb527efee6ce1a et le 2eme element est le tableau des packets concernant l'appel TSipDialogArr (dans lequel on retrouve la paire [Call-ID] --> [2dfb527efee6ce1a]

extern int count_sip_packet;
extern int count_udp_packet;
extern int count_rtp_packet;

extern void log_dialog(const string callid, dialog_map& dd, const char* ip_src,const char* ip_dst, string& logfilename);
extern void mytrim(string& s);
extern void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace);
extern void mylog(unsigned char level,const char* fmt, ...);
extern void dump_dialog(dialog_map& dd);
extern void print_payload(const u_char *payload, int len);
extern void siplog(const char* sipstr,int size);*/

#endif // SIP_H

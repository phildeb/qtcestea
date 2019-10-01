#include "common.h"
#include "sip.h"
#include "utils.h"
#include "md5.h"
#include "rtp.h"
#include "sip.h"
#include <csta.h>

//#define TEST_PLAYING_FILE
void CUserAgent::ringingState(CSipMessage* pSipMsg)
{
    if ( pSipMsg != NULL)
        pSipMsg->parse();

    if ( pSipMsg->_isSipResponse ){
        if (pSipMsg->_statusCode == 401){
            if ( pSipMsg->_methodCSeq == "REGISTER" ){
                //sipRegisterChallenge(pSipMsg->_authNonce.toLatin1().constData());
            }
        }
    }else{ // SIP REQUEST ( INVITE CANCEL INFO UPDATE REFER BYE...)

        if ( pSipMsg->_methodLine == "CANCEL" )
        {
            if (pSipMsg->_callID == _ringing_call_id ){
                changeState(USER_AGENT_IDLE);
            }
        }
        else if ( pSipMsg->_methodLine == "BYE" )
        {
            if (pSipMsg->_callID == _current_incoming_call_id ) {
                changeState(USER_AGENT_IDLE);
                // send 200 OK to accept the BYE
                sipAckBye(_current_incoming_call_id,pSipMsg->_numberCSeq
                          ,pSipMsg->_toLine.toLatin1().constData()
                          ,pSipMsg->_fromLine.toLatin1().constData()
                          ,pSipMsg->_viaLine.toLatin1().constData()
                          );

                ///////////////// SIP ///////////////:
                if ( _rtp_udp_send_socket>0 ){
                    mylog(2,"rtp_udp_send_socket WAS [%d]",_rtp_udp_send_socket);
                    close(_rtp_udp_send_socket);
                    _rtp_udp_send_socket = -1;
                    mylog(2,"rtp_udp_send_socket IS [%d]",_rtp_udp_send_socket);
                }
            }
        }
        else if ( pSipMsg->_methodLine == "ACK" )//if (0 == strncasecmp ("ACK", first_word, strlen("ACK") ) )
        {
            mylog(5,"SIP REQUEST HANDLED : [%s]",pSipMsg->_methodLine.toLatin1().constData());

            if (pSipMsg->_callID == _ringing_call_id ){

                changeState(USER_AGENT_IN_INCOMING_COMM);
                _current_incoming_call_id = _ringing_call_id;

                /////////////// SIP //////////////////
                /*if ( _rtp_playing_sound_thread.isRunning() ){
                _rtp_playing_sound_thread._isStopped = true;
                usleep(ONE_SEC_USLEEP/2);
            }
            if ( _rtp_receiving_sound_thread.isRunning() ){
                _rtp_receiving_sound_thread._isStopped = true;
                usleep(ONE_SEC_USLEEP/2);
            }*/

                _rtp_playing_sound_udp_port = _ringingInviteMessageRtpPort; // store rtp port present in SIP INVITE SDP message
                //_rtp_port_rx = ; //RTP_PORT_ANSWER_VALUE;

                _from_line  = pSipMsg->_fromLine;
                _to_line = pSipMsg->_toLine;

#ifdef TEST_PLAYING_FILE
                if ( _rtp_playing_sound_udp_port > 0){
                    mylog(5,"sip_call->rtp_port_tx=[%d]\n",_rtp_playing_sound_udp_port);
                    char wavpath[256]="./printemps-alaw.wav";
                    if ( _rtp_playing_sound_thread.init(QString(wavpath),_rtp_playing_sound_udp_port,SIP_PROXY_ADDR) ){
                        _rtp_playing_sound_thread.start();
                    }
                }
#else
                struct addrinfo hints;
                memset(&hints,0,sizeof(hints));
                hints.ai_family=AF_INET;
                hints.ai_socktype=SOCK_DGRAM;
                hints.ai_protocol=IPPROTO_UDP;//hints.ai_flags=AI_PASSIVE|AI_ADDRCONFIG;

                char portname[25]={0};
                sprintf(portname,"%d",_rtp_playing_sound_udp_port);
                struct addrinfo* server=0;
                int err=getaddrinfo(SIP_PROXY_ADDR.toLatin1().constData(),portname,&hints,&server);
                if (err!=0) {
                    mylog(1,"failed to resolve local socket address (err=%d)",err);
                }

                if ( _rtp_udp_send_socket>0 ){
                    mylog(2,"rtp_udp_send_socket WAS [%d]",_rtp_udp_send_socket);
                    close(_rtp_udp_send_socket);
                    _rtp_udp_send_socket = -1;
                    mylog(2,"rtp_udp_send_socket IS [%d]",_rtp_udp_send_socket);
                }
                _rtp_udp_send_socket = socket(server->ai_family,server->ai_socktype,server->ai_protocol);
                mylog(2,"rtp_udp_send_socket [%d]",_rtp_udp_send_socket);

                //////////////////// CSTA ////////////////////////

                QString s = pSipMsg->_fromLine;
                int first_colon = s.indexOf("@");// recherche numero appelant
                if ( -1!=first_colon ) {
                    QString lef = s.mid(0,first_colon).trimmed();
                    QString rig = s.mid(first_colon+1).trimmed();
                    qDebug() << "split [" << rig << "] [" << lef << "]";

                    s=lef;
                    if ( s.length()> strlen("sip:") ){
                        int first_colon = s.indexOf("sip:");
                        QString lef = s.mid(0,first_colon).trimmed();
                        QString rig = s.mid(first_colon + strlen("sip:")).trimmed();
                        qDebug() << "split [" << rig << "] [" << lef << "]";
                        _csta_extension_recorded = filter_ascii_digits_only(rig.toLatin1());
                        //stopCSTA();
                        //_csta_extension_recorded = "19302701";
                        //startCSTA();
                        if ( _pUaGroup != NULL && _pUaGroup->_myCSTAlink!=NULL  ){
                            if ( _csta_extension_recorded.length() )
                                _pUaGroup->_myCSTAlink->startMonitor(_csta_extension_recorded,_pUaGroup->_sip_proxy_address,
                                            _rtp_playing_sound_udp_port,_rtp_udp_send_socket);
                        }
                    }
                }
#endif
            }
        }
    }
}

void CUserAgent::incomingCommState(CSipMessage* pSipMsg)
{
    /*<--- SIP read from UDP:192.168.0.181:5060 --->
    BYE sip:1111@192.168.0.230:5060 SIP/2.0
    Via: SIP/2.0/UDP 192.168.0.181:5060;branch=z9hG4bK86080b3d9853c79a7.6c60109c8d52da1f5
    Max-Forwards: 70
    From: "9000" <sip:9000@192.168.0.230:5060>;tag=b0140ce37c
    To: "1111" <sip:1111@192.168.0.230:5060>;tag=as1b85dfe3
    Call-ID: d6c7a8d7a880d644
    CSeq: 12315 BYE
    Allow: INVITE, ACK, CANCEL, BYE, NOTIFY, REFER, OPTIONS, UPDATE, PRACK, SUBSCRIBE, INFO
    Allow-Events: talk, hold, conference, LocalModeStatus
    Authorization: Digest username="9000",realm="asterisk",nonce="1d1f6d6f",uri="sip:1111@192.168.0.230:5060",response="17f39ea00161a7de43a502752333f91b",algorithm=MD5
    Supported: gruu, path, timer
    User-Agent: Aastra 51i/2.5.1.41
    Content-Length: 0

    <------------->
    --- (13 headers 0 lines) ---
    Sending to 192.168.0.181:5060 (NAT)
    Scheduling destruction of SIP dialog 'd6c7a8d7a880d644' in 32000 ms (Method: BYE)
    [Nov 25 12:24:08] NOTICE[23855]: chan_sip.c:4380 send_response: send_response

    <--- Transmitting (NAT) to 192.168.0.181:5060 --->
    SIP/2.0 200 OK
    Via: SIP/2.0/UDP 192.168.0.181:5060;branch=z9hG4bK86080b3d9853c79a7.6c60109c8d52da1f5;received=192.168.0.181;rport=5060
    From: "9000" <sip:9000@192.168.0.230:5060>;tag=b0140ce37c
    To: "1111" <sip:1111@192.168.0.230:5060>;tag=as1b85dfe3
    Call-ID: d6c7a8d7a880d644
    CSeq: 12315 BYE
    Server: Asterisk PBX 1.8.32.3
    Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, SUBSCRIBE, NOTIFY, INFO, PUBLISH, MESSAGE
    Supported: replaces, timer
    Content-Length: 0*/
    if ( pSipMsg != NULL)
        pSipMsg->parse();

    if ( pSipMsg->_isSipResponse ){
        if (pSipMsg->_statusCode == 401){
            if ( pSipMsg->_methodCSeq == "REGISTER" ){
                //sipRegisterChallenge(pSipMsg->_authNonce.toLatin1().constData());
            }
        }
    }else{ // SIP REQUEST ( INVITE CANCEL INFO UPDATE REFER BYE...)

        if ( pSipMsg->_methodLine == "CANCEL" )
        {
            if (pSipMsg->_callID == _current_incoming_call_id ){
                changeState(USER_AGENT_IDLE);
            }
        }
        else if ( pSipMsg->_methodLine == "BYE" )
        {
            if (pSipMsg->_callID == _current_incoming_call_id )
            {
                changeState(USER_AGENT_IDLE);

                sipAckBye(_current_incoming_call_id,pSipMsg->_numberCSeq
                          ,pSipMsg->_toLine.toLatin1().constData()
                          ,pSipMsg->_fromLine.toLatin1().constData()
                          ,pSipMsg->_viaLine.toLatin1().constData()
                          );// send 200 OK to accept the BYE

                ///////////////// SIP ///////////////:
                if ( _rtp_udp_send_socket>0 ){
                    mylog(2,"rtp_udp_send_socket WAS [%d]",_rtp_udp_send_socket);
                    close(_rtp_udp_send_socket);
                    _rtp_udp_send_socket = -1;
                    mylog(2,"rtp_udp_send_socket IS [%d]",_rtp_udp_send_socket);
                }

#ifdef TEST_PLAYING_FILE
                if ( _rtp_playing_sound_thread.isRunning() ){
                    _rtp_playing_sound_thread._isStopped = true;
                    usleep(ONE_SEC_USLEEP/2);
                    _rtp_playing_sound_thread.stop();
                    _rtp_playing_sound_thread.terminate();
                }
#else
                //////////////////// CSTA ////////////////// stop recording and monitoring current extension
                if ( _pUaGroup != NULL && _pUaGroup->_myCSTAlink!=NULL  )
                {
                    if ( _csta_extension_recorded.length() )
                    {
                        _pUaGroup->_myCSTAlink->stopMonitor(_csta_extension_recorded,_pUaGroup->_sip_proxy_address,
                                        _rtp_playing_sound_udp_port,_rtp_udp_send_socket);
                    }
                    _csta_extension_recorded = "";
                }
#endif
            }
        }
        else if ( pSipMsg->_methodLine == "ACK" )
        {
            mylog(5,"SIP REQUEST UNHANDLED : [%s]",pSipMsg->_methodLine.toLatin1().constData());
            mylog(5,"SIP REQUEST UNHANDLED : [%s]",pSipMsg->_methodLine.toLatin1().constData());
            mylog(5,"SIP REQUEST UNHANDLED : [%s]",pSipMsg->_methodLine.toLatin1().constData());
            mylog(5,"SIP REQUEST UNHANDLED : [%s]",pSipMsg->_methodLine.toLatin1().constData());
            mylog(5,"SIP REQUEST UNHANDLED : [%s]",pSipMsg->_methodLine.toLatin1().constData());
            mylog(5,"SIP REQUEST UNHANDLED : [%s]",pSipMsg->_methodLine.toLatin1().constData());
        }
    }
}


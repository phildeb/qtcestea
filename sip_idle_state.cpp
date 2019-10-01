#include "common.h"
#include "sip.h"
#include "utils.h"
#include "md5.h"
#include "rtp.h"
#include "sip.h"
#include <csta.h>

void CUserAgent::idleState(CSipMessage* pSipMsg)
{
    if ( pSipMsg != NULL)
        pSipMsg->parse();

    if ( pSipMsg->_isSipResponse ){

        if (401==pSipMsg->_statusCode)
        {

            if ( pSipMsg->_methodCSeq == "REGISTER" && pSipMsg->_numberCSeq == _register_cseq ){

                if ( pSipMsg->_callID == _current_register_call_id)
                {
                    pSipMsg->_isConsummed = true;
                    sipRegisterChallenge(pSipMsg,_current_register_call_id,pSipMsg->_authNonce.toLatin1().constData(),3600);
                }
                else if ( pSipMsg->_callID == _current_unregister_call_id){
                    pSipMsg->_isConsummed = true;
                    sipRegisterChallenge(pSipMsg,_current_unregister_call_id,pSipMsg->_authNonce.toLatin1().constData(),0);
                }
            }
            /*else if ( pSipMsg->_methodCSeq == "INVITE" ){ // DONE in initiateState !!!!

                //sipRegisterChallenge(pSipMsg->_authNonce.toLatin1().constData());
                mylog(5,"sip_ack_invite <- sip_ack_invite TAG:[%s]\n",pSipMsg->_toTag.toLatin1().constData());
                sip_ack_invite_outgoing((char*)pSipMsg->_toTag.toLatin1().constData());


                char digest[256]={0};
                int digest_len=sizeof(digest);

                //int build_reply_digest( char* username, const char* realm, const char* secret,const char* method, const char* uri, const char* nonce,char* digest, int digest_len)
                build_reply_digest(
                            _phone_number.toLatin1().constData(),
                            "asterisk",
                            _md5_password.toLatin1().constData(),
                            "INVITE",
                            QString("sip:%1@%2").arg(_called_number).arg(SIP_PROXY_ADDR).toLatin1().constData(),
                            pSipMsg->_authNonce.toLatin1().constData(),
                            digest,
                            digest_len
                            );

                mylog(5,"sip_INVITE_with_auth <-- sequence phrase:[%s]\n",
                      pSipMsg->_methodCSeq.toLatin1().constData());
                mylog(5,"sip_INVITE_with_auth <-- sequence number:[%d]\n",
                      pSipMsg->_numberCSeq);
                sip_INVITE_with_auth(digest);
            }*/
        }
        else if (200==pSipMsg->_statusCode) // REGISTER 200 OK response from sip proxy
        {
            if (pSipMsg->_methodCSeq == "REGISTER" && pSipMsg->_numberCSeq == _register_cseq ){

                pSipMsg->_isConsummed = true;

                if ( pSipMsg->_callID == _current_register_call_id)
                {
                    _is_registered = true;
                    _registered_date_time = QDateTime::currentDateTime();

                    qDebug() << "_registered_date_time="<<_registered_date_time;
                    qDebug() << "_registered_date_time="<<_registered_date_time;
                    qDebug() << "_registered_date_time="<<_registered_date_time;
                }else if ( pSipMsg->_callID == _current_unregister_call_id){

                    _is_registered = false;
                    _unregistered_date_time = QDateTime::currentDateTime();

                    qDebug() << "_unregistered_date_time="<<_unregistered_date_time;
                    qDebug() << "_unregistered_date_time="<<_unregistered_date_time;
                    qDebug() << "_unregistered_date_time="<<_unregistered_date_time;

                }

            }else if (pSipMsg->_methodCSeq == "BYE" ){
                mylog(5,"RX CODE %d cseq[%d %s] Call-ID:[%s]",pSipMsg->_statusCode,
                      pSipMsg->_numberCSeq, pSipMsg->_methodCSeq.toLatin1().constData(),pSipMsg->_callID.toLatin1().constData());
                mylog(5,"RX CODE %d cseq[%d %s] Call-ID:[%s]",pSipMsg->_statusCode,
                      pSipMsg->_numberCSeq, pSipMsg->_methodCSeq.toLatin1().constData(),pSipMsg->_callID.toLatin1().constData());
                mylog(5,"RX CODE %d cseq[%d %s] Call-ID:[%s]",pSipMsg->_statusCode,
                      pSipMsg->_numberCSeq, pSipMsg->_methodCSeq.toLatin1().constData(),pSipMsg->_callID.toLatin1().constData());

                sip_ack_bye_200_OK_outgoing(pSipMsg->_toTag.toLatin1().data(),pSipMsg->_numberCSeq);
            }
        }

    }else{ // SIP REQUEST ( INVITE CANCEL BYE...)

        if ( pSipMsg->_methodLine == "INVITE" )
        {
            if ( _autoAnswer == false ) return;

            QString call_number,s = pSipMsg->_toLine;
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
                    call_number = filter_ascii_digits_only(rig.toLatin1());

                }
            }

            if ( call_number == _phone_number ){
                mylog(5,"ACCEPTING incoming call to %s => INVITE audio port[%d]",call_number.toLatin1().constData(),pSipMsg->_rtp_port);

                pSipMsg->_isConsummed = true;
                _ringingInviteMessageRtpPort = pSipMsg->_rtp_port;
                _ringing_call_id = pSipMsg->_callID;

                changeState(USER_AGENT_RINGING);//waiting for pSipMsg->_methodLine == "ACK" ....
                sip_200OK_invite_incoming(pSipMsg);
            }
        }
    }
    return;
#if 0
// pas la meme reponse si 401 suite a CSEQ INVITE ou CSEQ REGISTER
/*<--- Reliably Transmitting (no NAT) to 192.168.0.5:42572 --->
SIP/2.0 401 Unauthorized
Via: SIP/2.0/UDP 192.168.0.5:5060;branch=z9hG4bKc0a801c200000017438f03a4000057c500000004;received=192.168.0.5;rport=42572
From: 9002<sip:9002@192.168.0.230>;tag=1557189023335
To: <sip:9000@192.168.0.230>;tag=as562c8abb
Call-ID: 3C3B2BC8-A92F-4CB4-94BE-91E3C53CF28E@192.168.0.5
CSeq: 1 INVITE
Server: Asterisk PBX 11.13.1~dfsg-2+b1
Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, SUBSCRIBE, NOTIFY, INFO, PUBLISH, MESSAGE
Supported: replaces, timer
WWW-Authenticate: Digest algorithm=MD5, realm="asterisk", nonce="2f21bf50"
Content-Length: 0
*/
#endif
}

void CUserAgent::changeState(USER_AGENT_STATE new_state)
{
    mylog(2," ============= [%s] PREVIOUS STATE %d ================",_phone_number.toLatin1().constData(),_state);
    _state = new_state;
    mylog(2," ============= [%s] NEW STATE %d ================",_phone_number.toLatin1().constData(),_state);
}


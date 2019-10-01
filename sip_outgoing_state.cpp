#include "common.h"
#include "sip.h"
#include "utils.h"
#include "md5.h"
#include "rtp.h"
#include "sip.h"
#include <csta.h>

#define TEST_PLAYING_FILE

void CUserAgent::initiateState(CSipMessage* pSipMsg)
{
    if ( pSipMsg != NULL)
        pSipMsg->parse();

    if ( pSipMsg->_isSipResponse ){

        mylog(5,"initiateState RX CODE %d cseq[%d %s] Call-ID:[%s]",pSipMsg->_statusCode,
              pSipMsg->_numberCSeq, pSipMsg->_methodCSeq.toLatin1().constData(),pSipMsg->_callID.toLatin1().constData());
        mylog(5,"initiateState RX CODE %d cseq[%d %s] Call-ID:[%s]",pSipMsg->_statusCode,
              pSipMsg->_numberCSeq, pSipMsg->_methodCSeq.toLatin1().constData(),pSipMsg->_callID.toLatin1().constData());
        mylog(5,"initiateState RX CODE %d cseq[%d %s] Call-ID:[%s]",pSipMsg->_statusCode,
              pSipMsg->_numberCSeq, pSipMsg->_methodCSeq.toLatin1().constData(),pSipMsg->_callID.toLatin1().constData());

        if (401==pSipMsg->_statusCode)
        {
            if ( pSipMsg->_methodCSeq == "INVITE" ){

                mylog(5,"sip_ack_invite <- sip_ack_invite TAG:[%s]\n",pSipMsg->_toTag.toLatin1().constData());
                sip_ack_invite_401_outgoing((char*)pSipMsg->_toTag.toLatin1().constData());

                char digest[256]={0};
                int digest_len=sizeof(digest);

                build_reply_digest(
                            _phone_number.toLatin1().constData(),
                            "asterisk",
                            _md5_password.toLatin1().constData(),
                            "INVITE",
                            QString("sip:%1@%2").arg(_called_number).arg(SIP_PROXY_ADDR).toLatin1().constData(),
                            pSipMsg->_authNonce.toLatin1().constData(),
                            digest, digest_len
                            );

                mylog(5,"sip_INVITE_with_auth <-- sequence phrase:[%s]\n",pSipMsg->_methodCSeq.toLatin1().constData());
                mylog(5,"sip_INVITE_with_auth <-- sequence number:[%d]\n",pSipMsg->_numberCSeq);
                sip_INVITE_with_auth(digest);
            }
        }
        else if (200==pSipMsg->_statusCode) // todo : check CSEQ INVITE value
        {
            if ( pSipMsg->_methodCSeq == "INVITE" && pSipMsg->_callID == _dialing_call_id)
            {
                mylog(5,"outgoing call => RX CODE 200 audio port[%d]",pSipMsg->_rtp_port);
                _rtp_playing_sound_udp_port = pSipMsg->_rtp_port; // store rtp port SIP INVITE SDP message in user agent member
                _current_outgoing_call_id = pSipMsg->_callID;
                _to_tag = pSipMsg->_toTag;
                _from_tag = pSipMsg->_fromTag;
                _from_line  = pSipMsg->_fromLine;
                _to_line = pSipMsg->_toLine;
                _via_line = pSipMsg->_viaLine;

                sip_ack_invite_200_OK_outgoing(pSipMsg);//->_toTag.toLatin1().data(), pSipMsg->_numberCSeq);
                changeState(USER_AGENT_IN_OUTGOING_COMM);
#ifdef TEST_PLAYING_FILE
                char wavpath[256]="./printemps-alaw.wav";
                if ( _rtp_playing_sound_thread.init(QString(wavpath),_rtp_playing_sound_udp_port,SIP_PROXY_ADDR) ){
                    changeState(USER_AGENT_IN_OUTGOING_COMM);
                    _rtp_playing_sound_thread.start();
                }
#endif
            }
        }

    }else{ // SIP REQUEST ( INVITE CANCEL BYE...)
    }
}

void CUserAgent::outgoingCommState(CSipMessage* pSipMsg)
    {
    if ( pSipMsg != NULL)
        pSipMsg->parse();

    if ( pSipMsg->_isSipResponse ){

        mylog(5,"outgoingCommState RX CODE %d cseq[%d %s] Call-ID:[%s]",pSipMsg->_statusCode,
              pSipMsg->_numberCSeq, pSipMsg->_methodCSeq.toLatin1().constData(),pSipMsg->_callID.toLatin1().constData());
        mylog(5,"outgoingCommState RX CODE %d cseq[%d %s] Call-ID:[%s]",pSipMsg->_statusCode,
              pSipMsg->_numberCSeq, pSipMsg->_methodCSeq.toLatin1().constData(),pSipMsg->_callID.toLatin1().constData());
        mylog(5,"outgoingCommState RX CODE %d cseq[%d %s] Call-ID:[%s]",pSipMsg->_statusCode,
              pSipMsg->_numberCSeq, pSipMsg->_methodCSeq.toLatin1().constData(),pSipMsg->_callID.toLatin1().constData());

        if (200==pSipMsg->_statusCode) // todo : check CSEQ INVITE value
        {

            if ( pSipMsg->_callID == _current_outgoing_call_id)
            {
                if ( pSipMsg->_methodCSeq == "BYE")
                {
                    sip_ack_bye_200_OK_outgoing(pSipMsg->_toTag.toLatin1().data(),pSipMsg->_numberCSeq);

                    changeState(USER_AGENT_IDLE);
                    mylog(5,"outgoing call => RX CODE 200 [%s]",pSipMsg->_methodCSeq.toLatin1().constData());
                    _rtp_playing_sound_udp_port = 0;
                    _current_outgoing_call_id = "";
                    //_to_tag = "";_from_line  = "";_to_line = "";

#ifdef TEST_PLAYING_FILE
                    if ( _rtp_playing_sound_thread.isRunning() ){
                        close(_rtp_playing_sound_thread._rtp_udp_send_socket);
                        _rtp_playing_sound_thread._isStopped = true;
                        usleep(ONE_SEC_USLEEP/2);
                        _rtp_playing_sound_thread.stop();
                        _rtp_playing_sound_thread.terminate();
                    }
#endif
                }

            }
        }


    }else{ // SIP REQUEST ( INVITE CANCEL INFO UPDATE REFER BYE...)

        if ( pSipMsg->_methodLine == "BYE" )
        {
            if (pSipMsg->_callID == _current_outgoing_call_id ) {
                changeState(USER_AGENT_IDLE);
                // send 200 OK to accept the BYE
                sipAckBye(_current_outgoing_call_id,pSipMsg->_numberCSeq
                          ,pSipMsg->_toLine.toLatin1().constData()
                          ,pSipMsg->_fromLine.toLatin1().constData()
                          ,pSipMsg->_viaLine.toLatin1().constData()
                          );
                _current_outgoing_call_id = "";
#ifdef TEST_PLAYING_FILE
                if ( _rtp_playing_sound_thread.isRunning() ){
                    close(_rtp_playing_sound_thread._rtp_udp_send_socket);
                    _rtp_playing_sound_thread._isStopped = true;
                    usleep(ONE_SEC_USLEEP/2);
                    _rtp_playing_sound_thread.stop();
                    _rtp_playing_sound_thread.terminate();
                }
#endif
            }
        }
    }
}


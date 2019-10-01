#include "common.h"
#include "sip.h"
#include "utils.h"
#include "md5.h"
#include "rtp.h"
#include "sip.h"

#define VIA_TAG_BRANCH1                 "z9hG4bK16516509551111"
//#define FROM_TAG1                       "19774779591111" //From: <sip:9005@192.168.0.230>;tag=1977477959

bool CUserAgent::sipInvite()
{
    _dialing_call_id = genRandomCallID("out");
    _dialing_cseq++;
    _dialing_branch_tag_via = genRandomBranch();
    _dialing_from_tag = genRandomTag();

    /*<--- SIP read from UDP:29.11.0.221:33069 --->
        INVITE sip:9003@29.11.0.222:5060 SIP/2.0
        Via: SIP/2.0/UDP 29.11.0.221;rport;branch=z9hG4bK16516509551111
        Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING,PRACK
        Call-ID: CID1111111111
        CSeq: 1 INVITE
        From: 9999<sip:9999@29.11.0.222>;tag=19774779591111
        To: 9003<sip:9003@29.11.0.222>;tag=
        User-Agent: DebreuilSystems
        Max-Forwards: 70
        Content-Type: application/sdp
        Content-Length=183

        v=0
        o=- 3342338646 3342338646 IN IP4 29.11.0.221
        s=sujet
        c=IN IP4 29.11.0.221
        t=0 0
        a=sendrecv
        m=audio 55555 RTP/AVP
        a=rtpmap:8 PCMA/8000
        a=rtpmap:101 telephone-event/8000
        <------------->
        --- (11 headers 9 lines) ---
        [Dec 24 15:50:09] NOTICE[13239]: chan_sip.c:4380 send_response: send_response

        <--- Reliably Transmitting (NAT) to 29.11.0.221:33069 --->
        SIP/2.0 481 Call/T  ransaction Does Not Exist*/
    CSipMessage* p_msg = new CSipMessage();
    p_msg->setInvite(QString(_called_number),QString(SIP_PROXY_ADDR));
    p_msg->setVia(QString(_local_address),_dialing_branch_tag_via);
    p_msg->_sipFieldList.append("Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING,PRACK");
    p_msg->setCallId(_dialing_call_id);
    p_msg->setCSeq(_dialing_cseq,QString("INVITE"));
    p_msg->setContact(_phone_number,QString(SIP_PROXY_ADDR));

    p_msg->setFrom(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),_dialing_from_tag);
    p_msg->setTo(QString(_called_number),QString(_called_number),QString(SIP_PROXY_ADDR),QString(""));
    p_msg->_sipFieldList.append("User-Agent: DebreuilSystems");

    p_msg->_sdpFieldList.append("v=0");
    p_msg->setSDP_o(_local_address);
    p_msg->_sdpFieldList.append("s=sujetentetesdp");
    p_msg->setSDP_c(_local_address);
    p_msg->_sdpFieldList.append("t=0 0");
    p_msg->_sdpFieldList.append("a=sendrecv"); // todo test a=recv
    p_msg->setSDP_audio(_rtp_port_dial);
    p_msg->_sdpFieldList.append("a=rtpmap:8 PCMA/8000");
    p_msg->_sdpFieldList.append("a=rtpmap:101 telephone-event/8000");

    send(p_msg);//sip_send_with_sdp(p_msg->_pBuf, p_msg->_preparedLen, p_msg->_pBufSDP, p_msg->_preparedLenSDP);
    return true;
}

void CUserAgent::sip_INVITE_with_auth(char* digest)
{    /*Why is that? I think that authentication on every call is a very good thing. Registration does NOT equal authentication.
     * It is only a way to let Asterisk know where to find the registered extensions. If there is no authentication on individual calls,
     * then possible attackers have a much eazier job when they want to use your system for :twisted: evil :twisted: puporses.
      As far as the "Correct auth, but based on stale nonce received from" message -
    I had this problem on some IP Phones. It is an IP phone issue, not an Asterisk issue.
    Asterisk just wants to let you know that the IP Phone does not change auth. parameters when it renews the SIP registration.
    i've add parameter pedantic=no to sip.conf. it works! my sip gates now reg and auth without problems on my asterisk.*/

    _dialing_cseq++;
    CSipMessage* p_msg = new CSipMessage();
    p_msg->setInvite(QString(_called_number),QString(SIP_PROXY_ADDR));
    p_msg->setVia(QString(_local_address),_dialing_branch_tag_via);
    p_msg->setCallId(_dialing_call_id);
    p_msg->setCSeq(_dialing_cseq,QString("INVITE"));
    p_msg->setContact(_phone_number,QString(SIP_PROXY_ADDR));
    p_msg->_sipFieldList.append("Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING,PRACK");
    p_msg->_sipFieldList.append(digest);

    p_msg->setFrom(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),_dialing_from_tag);
    p_msg->setTo(QString(_called_number),QString(_called_number),QString(SIP_PROXY_ADDR),QString(""));
    p_msg->_sipFieldList.append("User-Agent: DebreuilSystems");

    p_msg->_sdpFieldList.append("v=0");
    p_msg->setSDP_o(_local_address);
    p_msg->_sdpFieldList.append("s=sujetentetesdp");
    p_msg->setSDP_c(_local_address);
    p_msg->_sdpFieldList.append("t=0 0");
    p_msg->_sdpFieldList.append("a=sendrecv"); // todo test a=recv
    p_msg->setSDP_audio(_rtp_port_dial);
    p_msg->_sdpFieldList.append("a=rtpmap:8 PCMA/8000");
    p_msg->_sdpFieldList.append("a=rtpmap:101 telephone-event/8000");

    send(p_msg);//sip_send_with_sdp(p_msg->_pBuf, p_msg->_preparedLen, p_msg->_pBufSDP, p_msg->_preparedLenSDP);

}

bool CUserAgent:: sipByeIncomingCall()
{
    /*    *CLI> Retransmitting #9 (NAT) to 192.168.0.5:38332:
    BYE sip:9999@192.168.0.230:5060 SIP/2.0
    Via: SIP/2.0/UDP 192.168.0.230:5060;branch=z9hG4bK24a7c871;rport
    Max-Forwards: 70
    From: "9003" <sip:9003@192.168.0.230>;tag=as7dbf72be
    To: <sip:9999@192.168.0.5:5060;LINEID=da8fc4c53766b50b1d5879473d0bbbaf>
    Call-ID: 1455b9057353e4333004bdc1682b4471@192.168.0.230:5060
    CSeq: 103 BYE
    User-Agent: Asterisk PBX 1.8.32.3
    X-Asterisk-HangupCause: Normal Clearing
    X-Asterisk-HangupCauseCode: 16
    Content-Length: 0    */
    CSipMessage* p_msg = new CSipMessage();
    _bye_cseq++;

    p_msg->setBye(_phone_number,QString(SIP_PROXY_ADDR));
    p_msg->setVia(QString(_local_address),QString(VIA_TAG_BRANCH1));
    p_msg->setCallId(_current_incoming_call_id);
    p_msg->setCSeq(_bye_cseq ,QString("BYE"));
    //p_msg->setFrom(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),QString(FROM_TAG1));
    //p_msg->setTo(QString(_called_number),QString(_called_number),QString(SIP_PROXY_ADDR),to_tag);
    p_msg->_sipFieldList.append(_from_line);
    p_msg->_sipFieldList.append(_to_line);

    send(p_msg);//sip_send(p_msg->_pBuf, p_msg->_preparedLen );
    return true;
}

bool CUserAgent:: sipByeOutgoingCall()
{
    /*    *CLI> Retransmitting #9 (NAT) to 192.168.0.5:38332:
    BYE sip:9999@192.168.0.230:5060 SIP/2.0
    Via: SIP/2.0/UDP 192.168.0.230:5060;branch=z9hG4bK24a7c871;rport
    Max-Forwards: 70
    From: "9003" <sip:9003@192.168.0.230>;tag=as7dbf72be
    To: <sip:9999@192.168.0.5:5060;LINEID=da8fc4c53766b50b1d5879473d0bbbaf>
    Call-ID: 1455b9057353e4333004bdc1682b4471@192.168.0.230:5060
    CSeq: 103 BYE
    User-Agent: Asterisk PBX 1.8.32.3
    X-Asterisk-HangupCause: Normal Clearing
    X-Asterisk-HangupCauseCode: 16
    Content-Length: 0    */
    /*<--- SIP read from UDP:29.11.0.221:49876 --->
    BYE sip:9999@29.11.0.222:5060 SIP/2.0
    Via: SIP/2.0/UDP 29.11.0.221:5060;rport;branch=z9hG4bK16516509551111
    Call-ID: CID1111111111
    CSeq: 3 BYE
    Max-Forwards: 70
    Content-Length=0*/
    CSipMessage* p_msg = new CSipMessage();
    _bye_cseq++;
    p_msg->setBye(QString(_called_number),QString(SIP_PROXY_ADDR));
    p_msg->setVia(QString(_local_address),QString(VIA_TAG_BRANCH1));
    p_msg->setCallId(_current_outgoing_call_id);
    p_msg->setCSeq(_bye_cseq ,QString("BYE"));
    p_msg->setFrom(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),_from_tag);
    p_msg->setTo(QString(_called_number),QString(_called_number),QString(SIP_PROXY_ADDR),_to_tag);
    send(p_msg);//sip_send(p_msg->_pBuf, p_msg->_preparedLen );
    return true;
}


bool CUserAgent::sipAckBye(QString callid,int numberCSeq,const char* to_line, const char* from_line, const char* via_line)
{/*<--- SIP read from UDP:29.11.0.221:45220 --->
    SIP/2.0 200 OK
    Via: SIP/2.0/UDP 29.11.0.222:5060;branch=z9hG4bK34b815e4;rport
    Call-ID: CID1111111111
    CSeq: 102 BYE
    From: 9003<sip:9003@29.11.0.222>;tag=as79b4f998
    To: 9999<sip:9999@29.11.0.222>;tag=19774779591111
    Max-Forwards: 70
    Content-Length=0
    <------------->
    --- (8 headers 0 lines) ---
    [Dec 24 17:10:53] NOTICE[13239]: chan_sip.c:21762 handle_response: handle_response
    SIP Response message for INCOMING dialog BYE arrived
    Really destroying SIP dialog 'CID1111111111' Method: ACK
    Really destroying SIP dialog 'REGCID123456789@29.11.0.221' Method: REGISTER*/

    /*ERROR:
    <--- SIP read from UDP:29.11.0.221:37621 --->
    SIP/2.0 200 OK
    Via: SIP/2.0/UDP 29.11.0.222:5060;branch=z9hG4bK06de3368;rport
    Call-ID: outz3h8x7c1a9j3@29.11.0.221
    CSeq: 102 BYE
    From: 9003<sip:9003@29.11.0.222>;tag=as6ef92f01
    To: 9999<sip:9999@29.11.0.222>;tag=19774779591111
    Max-Forwards: 70
    Content-Length=0
    SIP/2.0 200 OK
    Via: SIP/2.0/UDP 29.11.0.222:5060;branch=z9hG4bK06de3368;rport
    Call-ID: outz3h8x7c1a9j3@29.11.0.221
    CSeq: 102 BYE
    From: 9003<sip:9003@29.11.0.222>;tag=as6ef92f01
    To: 9999<sip:9999@29.11.0.222>;tag=19774779591111
    Max-Forwards: 70
    Content-Type: application/sdp
    Content-Length=0

    <------------->
    --- (17 headers 0 lines) ---
    [Dec 26 13:46:14] WARNING[18338]: chan_sip.c:25941 handle_incoming: Misrouted SIP response '200 OK' with Call-ID 'outz3h8x7c1a9j3@29.11.0.221', too many vias
    Retransmitting #3 (NAT) to 29.11.0.221:37621:
    BYE sip:9999@29.11.0.222:5060 SIP/2.0
    Via: SIP/2.0/UDP 29.11.0.222:5060;branch=z9hG4bK06de3368;rport
    Max-Forwards: 70
    From: 9003<sip:9003@29.11.0.222>;tag=as6ef92f01
    To: 9999<sip:9999@29.11.0.222>;tag=19774779591111
    Call-ID: outz3h8x7c1a9j3@29.11.0.221
    CSeq: 102 BYE
    User-Agent: Asterisk PBX 1.8.32.3
    Proxy-Authorization: Digest username="9999", realm="asterisk", algorithm=MD5, uri="sip:29.11.0.222", nonce="", response="16368c1b05aa9d08fa36d29ab229f97f"
    X-Asterisk-HangupCause: Normal Clearing
    X-Asterisk-HangupCauseCode: 16
    Content-Length: 0*/
    CSipMessage* p_msg = new CSipMessage();

    p_msg->_sipFieldList.append("SIP/2.0 200 OK");
    //p_msg->setVia(QString(IP_CALLER),QString(VIA_TAG_BRANCH1));
    p_msg->setCallId(callid);
    p_msg->setCSeq(numberCSeq ,QString("BYE"));
    p_msg->_sipFieldList.append(from_line);
    p_msg->_sipFieldList.append(to_line);
    p_msg->_sipFieldList.append(via_line);

    send(p_msg);//sip_send(p_msg->_pBuf, p_msg->_preparedLen );
    return true;
}

void CUserAgent::sip_ack_invite_401_outgoing(char* to_tag)
{
#ifdef OLD_MSG_FORMAT
    BYTE msg_ack[1024]={
        "ACK sip:"_called_number"@"SIP_PROXY_ADDR" SIP/2.0\r\n"
        "Via: SIP/2.0/UDP "IP_CALLER";rport;branch="VIA_TAG_BRANCH1"\r\n"
        "Call-ID: "UA_CALLID1"\r\n"
        "CSeq: "CSEQ_INVITE1" ACK\r\n"
        "From: "UA_NUMBER_LOCAL"<sip:"UA_NUMBER_LOCAL"@"SIP_PROXY_ADDR">;tag="FROM_TAG1"\r\n"
        //"To: <sip:"_called_number"@"SIP_PROXY_ADDR">\r\n"
        "Max-Forwards: 70\r\n"
        "Content-Length=0\r\n"
        "\r\n"
    };
#else
    CSipMessage* p_msg = new CSipMessage();

    p_msg->setAck(QString(_called_number),QString(SIP_PROXY_ADDR));
    p_msg->setVia(QString(_local_address),QString(VIA_TAG_BRANCH1));
    p_msg->setCallId(_dialing_call_id);
    p_msg->setCSeq(_dialing_cseq ,QString("ACK"));
    p_msg->setFrom(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),_from_tag);
    p_msg->setTo(QString(_called_number),QString(_called_number),QString(SIP_PROXY_ADDR),to_tag);

    send(p_msg);//sip_send(p_msg->_pBuf, p_msg->_preparedLen );
#endif
}

bool CUserAgent::sip_ack_bye_200_OK_outgoing(char* to_tag,int cseq)
{
    CSipMessage* p_msg = new CSipMessage();
    p_msg->setAck(QString(_called_number),QString(SIP_PROXY_ADDR));
    p_msg->setVia(QString(_local_address),QString(VIA_TAG_BRANCH1));
    p_msg->setCallId(_current_outgoing_call_id);
    p_msg->setCSeq(cseq ,QString("ACK"));
    p_msg->setFrom(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),_from_tag);
    p_msg->setTo(QString(_called_number),QString(_called_number),QString(SIP_PROXY_ADDR),to_tag);
    send(p_msg);
}

bool CUserAgent::sip_ack_invite_200_OK_outgoing(CSipMessage* pSipMsg)
{
    /*<--- SIP read from UDP:29.11.0.221:58693 --->
    ACK sip:9003@29.11.0.222 SIP/2.0
    Via: SIP/2.0/UDP 29.11.0.221:5060;rport;branch=z9hG4bK16516509551111
    Call-ID: CID1111111111
    CSeq: 2 ACK
    From: 9999<sip:9999@29.11.0.222>;tag=19774779591111
    To: 9003<sip:9003@29.11.0.222>
    Max-Forwards: 70
    Content-Length=0*/
    CSipMessage* p_msg = new CSipMessage();
    p_msg->setAck(QString(_called_number),QString(SIP_PROXY_ADDR));
    //p_msg->setVia(QString(_local_address),QString(VIA_TAG_BRANCH1));
    p_msg->_sipFieldList.append(pSipMsg->_fromLine);
    p_msg->_sipFieldList.append(pSipMsg->_toLine);
    p_msg->_sipFieldList.append(pSipMsg->_viaLine);
    p_msg->setCallId(_current_outgoing_call_id);
    p_msg->setCSeq(pSipMsg->_numberCSeq ,QString("INVITE"));

    //p_msg->setFrom(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),_from_tag);
    //p_msg->setTo(QString(_called_number),QString(_called_number),QString(SIP_PROXY_ADDR),to_tag);
    send(p_msg);//sip_send(p_msg->_pBuf, p_msg->_preparedLen );
    return true;
}

void CUserAgent::sip_200OK_invite_incoming(CSipMessage* pSipMsg)
{    /*    -- Called SIP/9999
<--- SIP read from UDP:29.11.0.221:45081 --->
SIP/2.0 200 OK
Via: SIP/2.0/UDP 29.11.0.221:5060;rport;branch=z9hG4bK16516509551111
Contact: <sip:9999@29.11.0.222:5060>
Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING,PRACK
Max-Forwards: 70
Content-Type: application/sdp
Call-ID: 46fa8bfa4e0b017e29b4eb8e5b31ee9d@29.11.0.222:5060
From: "9000" <sip:19303814@29.11.0.222>;tag=as05138ab4
To: <sip:9999@29.11.0.221:5060;LINEID=da8fc4c53766b50b1d5879473d0bbbaf>
CSeq: 102 INVITE
Content-Length: 191

v=0
o=- 3342338646 3342338646 IN IP4 29.11.0.221
s=sujetentetesdp
c=IN IP4 29.11.0.221
t=0 0
a=sendrecv
m=audio 44444 RTP/AVP 8
a=rtpmap:8 PCMA/8000
a=rtpmap:101 telephone-event/8000
<------------->
--- (11 headers 9 lines) ---*/
    CSipMessage* p_msg = new CSipMessage();

    p_msg->_sipFieldList.append("SIP/2.0 200 OK");
    //p_msg->setAck(QString(_called_number),QString(SIP_PROXY_ADDR));
    p_msg->setVia(QString(_local_address),QString(VIA_TAG_BRANCH1));
    p_msg->_sipFieldList.append("Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING,PRACK");
    p_msg->setCallId(pSipMsg->_callID);
    p_msg->setCSeq(pSipMsg->_numberCSeq,pSipMsg->_methodCSeq); //sprintf(stringtocopy,"CSeq: %d INVITE", pSipMsg->_numberCSeq );

    //p_msg->setFrom(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),QString(FROM_TAG1));
    p_msg->_sipFieldList.append(pSipMsg->_fromLine);
    //p_msg->setTo(QString(_called_number),QString(_called_number),QString(SIP_PROXY_ADDR),to_tag);
    p_msg->_sipFieldList.append(pSipMsg->_toLine);
    p_msg->_sdpFieldList.append("v=0");
    p_msg->setSDP_o(_local_address);
    p_msg->_sdpFieldList.append("s=sujet");
    p_msg->setSDP_c(_local_address);
    p_msg->_sdpFieldList.append("t=0 0");
    p_msg->_sdpFieldList.append("a=sendrecv"); // todo test a=recv
    p_msg->setSDP_audio(_rtp_port_answer);//RTP_PORT_ANSWER_STRING);
    p_msg->_sdpFieldList.append("a=rtpmap:8 PCMA/8000");
    p_msg->_sdpFieldList.append("a=rtpmap:101 telephone-event/8000");

    send(p_msg);//sip_send_with_sdp(p_msg->_pBuf, p_msg->_preparedLen, p_msg->_pBufSDP, p_msg->_preparedLenSDP);
}

bool CSipMessage::parse()
{
    int nb_received = _pBufLen;
    char achBuffer[SIP_MSG_MAX_LEN];	// receive a datagram on the bound port number.
    memcpy(achBuffer,_pBuf,_pBufLen);
    char start_line[SIP_MSG_MAX_LEN]={0};
    int nb_bytes = get_line_before_CRLF( (const char*)achBuffer,nb_received, start_line, sizeof(start_line)-1 );
    if ( nb_bytes == 0 )
        return false;
    else
    {
        _startLine = start_line;
        mylog(5,"CSipMessage::parse: start_line:[%s]\n",start_line);
        // exemple:
        // appel entrant: start_line:[INVITE sip:9999@192.168.0.5:5060;LINEID=da8fc4c53766b50b1d5879473d0bbbaf SIP/2.0]
        // reponse auth: SIP/2.0 401 Unauthorized

        // FIRST : retrive Call-ID
        char* ptr = strstr(achBuffer, "Call-ID:");
        if  (NULL != ptr )
        {
            char str[512]= {0};
            int k = first_CRLF_len(ptr, sizeof(achBuffer) );
            if ( k>0 ) memcpy(str, ptr,k);str[k]='\0';
            mylog(5,"call_id_line:[%s]",str);
            _callid_line = str;

            // TROUVER LE CALL ID
            int first_colon = _callid_line.indexOf(":");
            if ( -1!=first_colon ) {
                QString rig = _callid_line.mid(0,first_colon).trimmed();
                QString lef = _callid_line.mid(first_colon+1).trimmed();
                rig=rig.trimmed();lef=lef.trimmed();
                //qDebug() << "separator : =>[" << rig << "] [" << lef << "]";
                /*if ( "CSeq" == rig ) {s_cseq = lef;}*/
                if ( "Call-ID" == rig ) {
                    _callID = lef;
                    /*lef.replace(QChar('@'),QString("_at_"));
                  s_call_ID_value = ascii_only(lef);*/
                    mylog(5,"CSipMessage::parse _callID:[%s]",_callID.toLatin1().constData());
                }
            }
        }

        char* str_CSeq = strstr(achBuffer,"CSeq:");
        int CSeq = 0;
        if ( NULL!=str_CSeq )
        {
            char* ptr = str_CSeq + strlen("CSeq:");
            //mylog(5,"before:%s\n",ptr);
            while (' '== *ptr)	ptr++; /* Skip spaces */
            while ( ( *ptr >= '0') && ( *ptr <='9') )
            {
                CSeq = (CSeq*10) + ( *ptr - '0' );
                //mylog(5,"SIP REQUEST: No de sequence:%d\n",CSeq);
                _numberCSeq = CSeq;
                ptr++;
            }
            //mylog(5,"SIP REQUEST: No de sequence:%d\n",CSeq);
            //mylog(5,"behind:%s\n",ptr);
            while (' '== *ptr)	ptr++; /* Skip spaces */

            str_CSeq = ptr;
            char str_CSeq[256]={0};
            strcpy_till_CRLF(str_CSeq, ptr);
            _methodCSeq = str_CSeq;

            mylog(5,"SIP REQUEST sequence [%d %s]\n",CSeq, str_CSeq);
        }

        char tag_line[512]= {0};
        ptr = strstr(achBuffer, (const char*)"tag=");// recuperer le tag du from pour le reinjecter
        if  (NULL != ptr )
        {
            int k = first_CRLF_len(ptr, sizeof(achBuffer) );
            if ( k>0 ) memcpy(tag_line, ptr,k);tag_line[k]='\0';
            mylog(5,"tag_line:[%s]",tag_line);
            _fromTag = tag_line;
        }

        char to_line[512]= {0};
        ptr = strstr(achBuffer, "To:");
        if  (NULL != ptr )
        {
            int k = first_CRLF_len(ptr, sizeof(achBuffer));
            if ( k>0 ) memcpy(to_line, ptr,k);to_line[k]='\0';
            mylog(5,"to_line:[%s]",to_line);
            _toLine = to_line;
        }
        char via_line[512]= {0};
        ptr = strstr(achBuffer, "Via:");
        if  (NULL != ptr )
        {
            int k = first_CRLF_len(ptr, sizeof(achBuffer));
            if ( k>0 ) memcpy(via_line, ptr,k);via_line[k]='\0';
            mylog(5,"_viaLine:[%s]",via_line);
            _viaLine = via_line;
        }
        char from_line[512]= {0};
        ptr = strstr(achBuffer, "From:");
        if  (NULL != ptr )
        {
            int k = first_CRLF_len(ptr, sizeof(achBuffer));
            if ( k>0 ) memcpy(from_line, ptr,k);from_line[k]='\0';
            mylog(5,"from_line:[%s]",from_line);
            //ptr = strstr(ptr+strlen("From:"), "tag=");mylog(5,"From tag : %s",ptr);
            _fromLine = from_line;
        }

        if (0 != strncmp ((const char *) start_line, (const char *) "SIP/", 4))
        {
            _isSipResponse = false;
            mylog(5,"                    == SIP REQUEST ==");
            char first_word[128]={0};
            {
                int i=0;
                char* ptr=start_line;
                while (' '!= *ptr)	{
                    first_word[i++] = *ptr;
                    ptr++;
                }
            }
            mylog(5,"CSipMessage::parse: SIP REQUEST first_word=[%s]",first_word);
            _methodLine = first_word;

            //if (0 == strncasecmp ("INVITE", first_word, strlen("INVITE") ) )
            {
                char via_line[512]= {0};
                char to_line[512]= {0};
                char tag_line[512]= {0};
                char from_line[512]= {0};

                int rtp_port=-1;
                int codec=-1;
                QString s = (char*)achBuffer;
                if ( s.contains("m=audio") && s.contains("RTP/AVP") ){
                    qDebug() << "found m=audio  and RTP/AVP!!!";
                    int idx1 = s.indexOf("m=audio");
                    if ( -1!=idx1 ) {
                        QString ss = s.mid(idx1+QString("m=audio").length());
                        ss = ss.trimmed();
                        int idx2 = ss.indexOf(" ");
                        //qDebug() << idx2 << " " << idx1 <<  "found after m=audio:" << ss;
                        if ( -1!=idx2 ) {
                            QString sss = s.mid(idx1 + QString("m=audio").length() ,idx2+1 );
                            sss = sss.trimmed();
                            //qDebug() << "found m=audio port in string :" << sss;
                            rtp_port = sss.toInt();
                            qDebug() << "found rtp_port=" << rtp_port;
                            _rtp_port = rtp_port;
                        }
                    }
                    int idx3 = s.indexOf("RTP/AVP");
                    if ( -1 != idx3 ) {
                        QString ss = s.mid(idx3+QString("RTP/AVP").length());
                        ss = ss.trimmed();
                        //qDebug() << "found RTP/AVP in string :" << ss;
                        int idx2 = ss.indexOf(" ");
                        if ( -1!=idx2 ) {
                            QString sss = s.mid(idx3 + QString("RTP/AVP").length() ,idx2+1 );
                            sss = sss.trimmed();
                            codec = sss.toInt();
                            qDebug() << "found codec=" << codec;
                            _codec = codec;
                        }
                    }
                }

                mylog(5,"CSipMessage::parse: rtp_port=[%d]\n",rtp_port);
                /*sip_call->rtp_port_tx  = rtp_port;
                mylog(5,"sip_call->rtp_port_tx=[%d]\n",sip_call->rtp_port_tx);
                mylog(5,"sip_call->rtp_port_tx=[%d]\n",sip_call->rtp_port_tx);
                mylog(5,"sip_call->rtp_port_tx=[%d]\n",sip_call->rtp_port_tx);
                mylog(5,"sip_call->rtp_port_tx=[%d]\n",sip_call->rtp_port_tx);*/



            }

        }else{

            mylog(5,"                    == SIP RESPONSE ==");
            _isSipResponse = true;
            // exmple parsing [SIP/2.0 401 Unauthorized]
            char *statuscode = strchr (start_line, ' ');	/* search for first SPACE */
            if ( nb_bytes > 0 )
            {
                int status_code;
                if (sscanf(statuscode + 1, "%d", &status_code) != 1)
                {
                    mylog(5,"Non-numeric status code:%s",statuscode);
                }
                else //if (sscanf(statuscode + 1, "%d", &status_code) != 1)
                {
                    mylog(5,"status_code:%d",status_code);
                    _statusCode = status_code;

                    char *reasonphrase = NULL;
                    reasonphrase = strchr (statuscode + 1, ' ');	/* search for 2nd SPACE */
                    if ( reasonphrase != NULL )
                    {
                        *reasonphrase = 0;
                        reasonphrase++;
                        _reasonPhrase = reasonphrase;
                        mylog(5,"reasonphrase[%s]",reasonphrase);
                    }

                    //exemple parsing [CSeq: 1 INVITE]
                    char* str_CSeq = strstr((char*)achBuffer,"CSeq:");
                    int CSeq = 0;
                    if ( NULL!=str_CSeq )
                    {
                        char* ptr = str_CSeq + strlen("CSeq:");
                        //mylog(5,"before:%s\n",ptr);
                        while (' '== *ptr)	ptr++; /* Skip spaces */
                        while ( ( *ptr >= '0') && ( *ptr <='9') ){
                            CSeq = (CSeq*10) + ( *ptr - '0' );
                            //mylog(5,"No de sequence:%d\n",CSeq);
                            ptr++;
                        }
                        //mylog(5,"behind:%s\n",ptr);
                        while (' '== *ptr)	ptr++; /* Skip spaces */

                        str_CSeq = ptr;
                        char str_CSeq[256]={0};
                        strcpy_till_CRLF(str_CSeq, ptr);

                        _methodCSeq = str_CSeq;
                        mylog(5,"SIP RESPONSE sequence [%d %s]\n",CSeq, str_CSeq);
                    }

                    // exemple parsing [To: <sip:9000@192.168.0.230>;tag=as562c8abb]
                    char* str_tag_to = strstr((char*)achBuffer,"\r\nTo:");
                    if ( NULL!=str_tag_to )
                    {
                        //mylog(5,"try reading tag TO...");
                        char* ptr_to = str_tag_to + strlen("\r\nTo:")+1;
                        if ( ptr_to )
                        {
                            //mylog(5,"str_tag_to[%s]", ptr_to);
                            char* ptr_to_tag = strstr(str_tag_to, "tag=") ;
                            if (ptr_to_tag)
                            {
                                ptr_to_tag = ptr_to_tag + strlen("tag=");
                                //mylog(5,"ptr_to_tag : %s", ptr_to_tag ) ;
                                int i=0;
                                while (( *ptr_to_tag != '\r' ) &&( *ptr_to_tag != '\n' )){
                                    _toTag[i++] = *ptr_to_tag ;
                                    _toTag[i]='\0';
                                    ptr_to_tag++;
                                }
                                mylog(5,"_toTag[%s]", _toTag.toLatin1().constData() );
                            }
                        }
                    }

                    // exemple parsing [WWW-Authenticate: Digest algorithm=MD5, realm="asterisk", nonce="2f21bf50"]
                    char* str_tag_nonce = strstr((char*)achBuffer,"nonce=");
                    if ( NULL!=str_tag_nonce )
                    {
                        char* ptr_nonce = str_tag_nonce + strlen("nonce=");
                        while( *ptr_nonce==' ' || *ptr_nonce=='"' )
                            ptr_nonce++;

                        char nonce[256]={0};
                        strcpy_till_CRLF(nonce, ptr_nonce);

                        _authNonce = nonce;
                    }

                    // if response 200 OK to INVITE, look for the audio port !!!
                    int rtp_port=-1;
                    int codec=-1;
                    QString s = (char*)achBuffer;
                    if ( s.contains("m=audio") && s.contains("RTP/AVP") ){
                        qDebug() << "found m=audio  and RTP/AVP!!!";
                        int idx1 = s.indexOf("m=audio");
                        if ( -1!=idx1 ) {
                            QString ss = s.mid(idx1+QString("m=audio").length());
                            ss = ss.trimmed();
                            int idx2 = ss.indexOf(" ");
                            //qDebug() << idx2 << " " << idx1 <<  "found after m=audio:" << ss;
                            if ( -1!=idx2 ) {
                                QString sss = s.mid(idx1 + QString("m=audio").length() ,idx2+1 );
                                sss = sss.trimmed();
                                //qDebug() << "found m=audio port in string :" << sss;
                                rtp_port = sss.toInt();
                                qDebug() << "found rtp_port=" << rtp_port;
                                _rtp_port = rtp_port;
                            }
                        }
                        int idx3 = s.indexOf("RTP/AVP");
                        if ( -1 != idx3 ) {
                            QString ss = s.mid(idx3+QString("RTP/AVP").length());
                            ss = ss.trimmed();
                            //qDebug() << "found RTP/AVP in string :" << ss;
                            int idx2 = ss.indexOf(" ");
                            if ( -1!=idx2 ) {
                                QString sss = s.mid(idx3 + QString("RTP/AVP").length() ,idx2+1 );
                                sss = sss.trimmed();
                                codec = sss.toInt();
                                qDebug() << "found codec=" << codec;
                                _codec = codec;
                            }
                        }
                    }

                }
            }
        }
    }
    return true;
}


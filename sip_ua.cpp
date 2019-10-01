#include "common.h"
#include "sip.h"
#include "utils.h"
#include "md5.h"
#include "rtp.h"
#include "sip.h"
#include "globales.h"

bool CUserAgent::init(QString sip_proxy_address,QString local_address, QString phone, QString pwd)
{
    SIP_PROXY_ADDR = sip_proxy_address;
    _local_address = local_address;
    _phone_number = phone;
    _md5_password = pwd;
    _register_cseq = 0;

    //strncpy(UA_NUMBER_LOCAL,"9999",sizeof(UA_NUMBER_LOCAL));//strncpy(UA_PASSWD,"ghdcss",sizeof(UA_NUMBER_LOCAL));
    _state = USER_AGENT_IDLE;    //_sip_udp_recv_send_socket = -1;
    _to_tag.clear();
    //start();//pthread_create( & _udp_sip_t_id, NULL, udpSipListeningThread, (void*)this);
    return true;
}

void CUserAgent::checkRegistration()
{
    QDateTime now = QDateTime::currentDateTime();
    qDebug() << "_registered_date_time was " << _registered_date_time.secsTo(now);
    if ( _is_registered ){
        int nbsecs = _registered_date_time.secsTo(now);
        if ( nbsecs > 600 ){
            _is_registered = false;
            sipRegister();
        }
    }
}

void CUserAgent::onSIPMessage(CSipMessage* pSipMsg)
{
    mylog(5,"==> onSIPMessage %p ==",pSipMsg);
    if ( _state == USER_AGENT_IDLE ) {
        idleState(pSipMsg);
    }
    else if ( _state == USER_AGENT_IN_INCOMING_COMM ) {
        incomingCommState(pSipMsg);
    }
    else if ( _state == USER_AGENT_IN_OUTGOING_COMM ) {
        outgoingCommState(pSipMsg);
    }
    else if ( _state == USER_AGENT_INITIATE_OUTGOING) {
        initiateState(pSipMsg);
    }
    else if ( _state == USER_AGENT_RINGING) {
        ringingState(pSipMsg);
    }
    //if (pSipMsg)       delete pSipMsg; // todo : store in database before freeing memory
    //mylog(5,"<== onSIPMessage  %p ==",pSipMsg);
}

int  CUserAgent::send(CSipMessage* pmsg)
{
    if ( _pUaGroup!=NULL ){
        _pUaGroup->_sipMsgToSend.push(pmsg);
    }
}

#if 0
int CUserAgent::sip_send_with_sdp(BYTE* ptr_udp_data, int len_udp_data,BYTE* ptr_udp_data2, int len_udp_data2)
{
    mylog(5,"TRY udp_send %d+%d bytes on socket %d", len_udp_data, len_udp_data2,_sip_udp_recv_send_socket );
    if ( _sip_udp_recv_send_socket > 0)
    {
        char bigbuf[1024]={0};
        if (len_udp_data< sizeof(bigbuf))
            memcpy(bigbuf,ptr_udp_data,len_udp_data);
        if ((len_udp_data+len_udp_data2) < sizeof(bigbuf))
            memcpy(bigbuf+len_udp_data,ptr_udp_data2,len_udp_data2);

        /* Construct the server address structure */
        memset(&_asteriskServAddr, 0, sizeof(_asteriskServAddr));    /* Zero out structure */
        _asteriskServAddr.sin_family = AF_INET;                 /* Internet addr family */
        _asteriskServAddr.sin_addr.s_addr = inet_addr(_rtp_proxy_addr.toLatin1().constData());  /* Server IP address */
        _asteriskServAddr.sin_port   = htons(SIP_PORT_DEFAULT);     /* Server port */

        int bytes_sent=0;
        if ( (bytes_sent = sendto(_sip_udp_recv_send_socket, bigbuf, len_udp_data+len_udp_data2, 0,
                                (struct sockaddr *)&_asteriskServAddr, sizeof(_asteriskServAddr))) != len_udp_data+len_udp_data2){
            /* Send data to the server */
            //mylog(5,"sendto() sent a different number of bytes than expected");
        }
        //hexdump((BYTE*)bigbuf,len_udp_data+len_udp_data2,"udp_send2");
        mylog(5,"============== << sip_send_with_sdp returned %d on socket %d ============",
              bytes_sent, _sip_udp_recv_send_socket );
        return bytes_sent;
    }
    return 0;
}

int CUserAgent::sip_send(BYTE* ptr_udp_data, int len_udp_data)
{
    int bytes_sent=0;
    //if (_sip_udp_recv_send_socket<=0) _sip_udp_recv_send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    mylog(5,"============== >> trying udp_send %d bytes on socket %d ... ============",len_udp_data, _sip_udp_recv_send_socket );
    mylog(5,"============== >> trying udp_send %d bytes on socket %d ... ============",len_udp_data, _sip_udp_recv_send_socket );
    mylog(5,"============== >> trying udp_send %d bytes on socket %d ... ============",len_udp_data, _sip_udp_recv_send_socket );
    mylog(5,"============== >> trying udp_send %d bytes on socket %d ... ============",len_udp_data, _sip_udp_recv_send_socket );
    mylog(5,"============== >> trying udp_send %d bytes on socket %d ... ============",len_udp_data, _sip_udp_recv_send_socket );
    if ( _sip_udp_recv_send_socket > 0)
    {
        memset(&_asteriskServAddr, 0, sizeof(_asteriskServAddr));    /* Zero out structure */
        _asteriskServAddr.sin_family = AF_INET;                 /* Internet addr family */
        _asteriskServAddr.sin_addr.s_addr = inet_addr(_rtp_proxy_addr.toLatin1().constData());  /* Server IP address */
        _asteriskServAddr.sin_port   = htons(SIP_PORT_DEFAULT);     /* Server port */

        if ( (bytes_sent = sendto(_sip_udp_recv_send_socket, ptr_udp_data, len_udp_data, 0,
                                (struct sockaddr *)&_asteriskServAddr, sizeof(_asteriskServAddr))) != len_udp_data)       /* Send the string to the server */
        {
            mylog(5,"sendto() sent a different number of bytes than expected");
        }
        //hexdump((BYTE*)ptr_udp_data,bytes_sent,"udp_send");
    }
    mylog(5,"============== << sendto returned %d on socket %d ============",bytes_sent, _sip_udp_recv_send_socket );
    return bytes_sent;
}
#endif

#if 0
void CUserAgent::run()
{
    while(1)
    {
        int nb_msg=0;
        CSipMessage* pMsg = NULL;
        //mylog(5,"CUserAgent::run");
        usleep(1000*1000);
        if ( g_sip_msg_queue.count() ){
            pMsg = g_sip_msg_queue.pull();
        }
        if (NULL!=pMsg){
            mylog(5,"CUserAgent g_queue.pull %p ...",pMsg);
            onSIPMessage(pMsg);
            delete pMsg;
        }
        /*mylog(5,"CUserAgent _sipMessageListMutex.tryLock...");
        {
            _sipMessageListMutex.lock();
            nb_msg=_sipMessageList.size();
            if ( nb_msg >= 1 )
                pMsg = _sipMessageList.takeFirst();
            _sipMessageListMutex.unlock();
        }*/        
    }
}
#endif

/*
--- (10 headers 0 lines) ---

<--- SIP read from UDP:10.253.253.5:5060 --->
INVITE sip:9000@10.253.253.21:5060 SIP/2.0
CSeq: 3 INVITE
Via: SIP/2.0/UDP 10.253.253.5:5060;branch=z9hG4bK50e05089-588d-e511-8951-6cf049152c3d;rport
User-Agent: Ekiga/4.0.1
Authorization: Digest username="9002", realm="asterisk", nonce="13b00aa6", uri="sip:9000@10.253.253.21:5060", algorithm=MD5, response="38140fe585d0dc294668af56b7a4acee"
From: "phd" <sip:9002@192.168.0.230>;tag=28f32e89-588d-e511-8951-6cf049152c3d
Call-ID: 3ef62e89-588d-e511-8951-6cf049152c3d@phd-GA-MA78LM-S2H
Supported: 100rel,replaces
To: <sip:9000@192.168.0.230>;tag=as77c4ac58
Contact: "phd" <sip:9002@10.253.253.5:5060>
Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING,PRACK
Content-Length: 226
Content-Type: application/sdp
Max-Forwards: 70

v=0
o=- 1447957510 2 IN IP4 10.253.253.5
s=Ekiga/4.0.1
c=IN IP4 10.253.253.5
t=0 0
m=audio 5070 RTP/AVP 8 101
a=sendrecv
a=rtpmap:8 PCMA/8000/1
a=rtpmap:101 telephone-event/8000
a=fmtp:101 0-16,32,36
a=maxptime:240
<------------->
--- (14 headers 11 lines) ---
Sending to 10.253.253.5:5060 (no NAT)
Found RTP audio format 8
Found RTP audio format 101
Found audio description format PCMA for ID 8
Found audio description format telephone-event for ID 101
Capabilities: us - (ulaw|alaw), peer - audio=(alaw)/video=(nothing)/text=(nothing), combined - (alaw)
Non-codec capabilities (dtmf): us - 0x1 (telephone-event|), peer - 0x1 (telephone-event|), combined - 0x1 (telephone-event|)
Peer audio RTP is at port 10.253.253.5:5070

<--- Transmitting (no NAT) to 10.253.253.5:5060 --->
SIP/2.0 100 Trying
Via: SIP/2.0/UDP 10.253.253.5:5060;branch=z9hG4bK50e05089-588d-e511-8951-6cf049152c3d;received=10.253.253.5;rport=5060
From: "phd" <sip:9002@192.168.0.230>;tag=28f32e89-588d-e511-8951-6cf049152c3d
To: <sip:9000@192.168.0.230>;tag=as77c4ac58
Call-ID: 3ef62e89-588d-e511-8951-6cf049152c3d@phd-GA-MA78LM-S2H
CSeq: 3 INVITE
Server: Asterisk PBX 11.13.1~dfsg-2+b1
Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, SUBSCRIBE, NOTIFY, INFO, PUBLISH, MESSAGE
Supported: replaces, timer
Contact: <sip:9000@10.253.253.21:5060>
Content-Length: 0


<------------>
Audio is at 13648
Adding codec 100004 (alaw) to SDP
Adding non-codec 0x1 (telephone-event) to SDP

<--- Reliably Transmitting (no NAT) to 10.253.253.5:5060 --->
SIP/2.0 200 OK
Via: SIP/2.0/UDP 10.253.253.5:5060;branch=z9hG4bK50e05089-588d-e511-8951-6cf049152c3d;received=10.253.253.5;rport=5060
From: "phd" <sip:9002@192.168.0.230>;tag=28f32e89-588d-e511-8951-6cf049152c3d
To: <sip:9000@192.168.0.230>;tag=as77c4ac58
Call-ID: 3ef62e89-588d-e511-8951-6cf049152c3d@phd-GA-MA78LM-S2H
CSeq: 3 INVITE
Server: Asterisk PBX 11.13.1~dfsg-2+b1
Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, SUBSCRIBE, NOTIFY, INFO, PUBLISH, MESSAGE
Supported: replaces, timer
Contact: <sip:9000@10.253.253.21:5060>
Content-Type: application/sdp
Content-Length: 244

v=0
o=root 96845050 96845051 IN IP4 10.253.253.21
s=Asterisk PBX 11.13.1~dfsg-2+b1
c=IN IP4 10.253.253.21
t=0 0
m=audio 13648 RTP/AVP 8 101
a=rtpmap:8 PCMA/8000
a=rtpmap:101 telephone-event/8000
a=fmtp:101 0-16
a=ptime:20
a=sendrecv

<------------>

<--- SIP read from UDP:10.253.253.5:5060 --->
ACK sip:9000@10.253.253.21:5060 SIP/2.0
CSeq: 3 ACK
Via: SIP/2.0/UDP 10.253.253.5:5060;branch=z9hG4bK428d5289-588d-e511-8951-6cf049152c3d;rport
Authorization: Digest username="9002", realm="asterisk", nonce="13b00aa6", uri="sip:9000@10.253.253.21:5060", algorithm=MD5, response="4faead8be2edd7d932a8dc4fea86536d"
From: "phd" <sip:9002@192.168.0.230>;tag=28f32e89-588d-e511-8951-6cf049152c3d
Call-ID: 3ef62e89-588d-e511-8951-6cf049152c3d@phd-GA-MA78LM-S2H
To: <sip:9000@192.168.0.230>;tag=as77c4ac58
Contact: "phd" <sip:9002@10.253.253.5:5060>
Content-Length: 0
Max-Forwards: 70

<------------->
--- (10 headers 0 lines) ---
    -- Executing [9000@from-sip:3] Playback("SIP/9002-00000007", "beep") in new stack
    -- <SIP/9002-00000007> Playing 'beep.gsm' (language 'en')
    -- Executing [9000@from-sip:4] Playback("SIP/9002-00000007", "beep") in new stack
    -- <SIP/9002-00000007> Playing 'beep.gsm' (language 'en')
    -- Executing [9000@from-sip:5] SayDigits("SIP/9002-00000007", "9000") in new stack
    -- <SIP/9002-00000007> Playing 'digits/9.gsm' (language 'en')
    -- <SIP/9002-00000007> Playing 'digits/0.gsm' (language 'en')
    -- <SIP/9002-00000007> Playing 'digits/0.gsm' (language 'en')
    -- <SIP/9002-00000007> Playing 'digits/0.gsm' (language 'en')

<--- SIP read from UDP:10.253.253.5:5060 --->
BYE sip:9000@10.253.253.21:5060 SIP/2.0
CSeq: 4 BYE
Via: SIP/2.0/UDP 10.253.253.5:5060;branch=z9hG4bK24167c8b-588d-e511-8951-6cf049152c3d;rport
User-Agent: Ekiga/4.0.1
Authorization: Digest username="9002", realm="asterisk", nonce="13b00aa6", uri="sip:9000@10.253.253.21:5060", algorithm=MD5, response="9887bf3197bcab211c1f16017f60d125"
From: "phd" <sip:9002@192.168.0.230>;tag=28f32e89-588d-e511-8951-6cf049152c3d
Call-ID: 3ef62e89-588d-e511-8951-6cf049152c3d@phd-GA-MA78LM-S2H
To: <sip:9000@192.168.0.230>;tag=as77c4ac58
Contact: "phd" <sip:9002@10.253.253.5:5060>
Content-Length: 0
Max-Forwards: 70

<------------->
--- (11 headers 0 lines) ---
Sending to 10.253.253.5:5060 (no NAT)
Scheduling destruction of SIP dialog '3ef62e89-588d-e511-8951-6cf049152c3d@phd-GA-MA78LM-S2H' in 32000 ms (Method: BYE)

<--- Transmitting (no NAT) to 10.253.253.5:5060 --->
SIP/2.0 200 OK
Via: SIP/2.0/UDP 10.253.253.5:5060;branch=z9hG4bK24167c8b-588d-e511-8951-6cf049152c3d;received=10.253.253.5;rport=5060
From: "phd" <sip:9002@192.168.0.230>;tag=28f32e89-588d-e511-8951-6cf049152c3d
To: <sip:9000@192.168.0.230>;tag=as77c4ac58
Call-ID: 3ef62e89-588d-e511-8951-6cf049152c3d@phd-GA-MA78LM-S2H
CSeq: 4 BYE
Server: Asterisk PBX 11.13.1~dfsg-2+b1
Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, SUBSCRIBE, NOTIFY, INFO, PUBLISH, MESSAGE
Supported: replaces, timer
Content-Length: 0


<------------>
  == Spawn extension (from-sip, 9000, 5) exited non-zero on 'SIP/9002-00000007'
liberation*CLI>
*/

int CUserAgent::genRandomSequenceNumber(int nb_digits)
{
    QString str_rand;
    srand(time(NULL)+nb_digits*_phone_number.toInt());
    static const char alnum[] = "0123456789";
    for (int i = 0; i < nb_digits; ++i){
        str_rand += alnum [ rand() % (sizeof(alnum) - 1) ];
    }
    return str_rand.toInt();
}

int CUserAgent::genRandomRtpPort()
{
    qsrand(QDateTime::currentDateTime().toTime_t());
    int port = 40000 + qrand()%10000;
    return port;
}

QString CUserAgent::genRandomCallID(QString prefix)
{/*If you’ve got a basic knowledge of SIP then you are aware of the header Call-ID.  Call-ID appears in every SIP request and every SIP response.
It is required to be globally unique and is generally a GUID (Globally Unique Identifier) associated with the IP addresses of the sender.
For instance, a typical Call-ID might look as follows: 77_1c6d7a3d17ea48dd6d916db4_I@10.100.1.9*/
    QString str,str_rand;
    srand(time(NULL)+_phone_number.toInt());
    static const char alpha[] = "abcdefghijklmnopqrstuvwxyz";
    static const char alnum[] = "0123456789";
    for (int i = 0; i < CALLID_LEN; ++i){
        str_rand += alpha [ rand() % (sizeof(alpha) - 1) ];
        str_rand += alnum [ rand() % (sizeof(alnum) - 1) ];
    }
    str = QString("%3%1@%2").arg(str_rand).arg(_local_address).arg(prefix);
    return str;
}

QString CUserAgent::genRandomBranch(QString suffix)
{/*The branch parameter always begins with the same string of seven characters — “z9hG4bK.”
This requirement was added to identify that the branch was created in accordance with RFC 3261 and not the older RFC 2543 which did not require global uniqueness*/
    QString str,str_rand;
    srand(time(NULL));
    //static const char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const char alnum[] = "0123456789";
    for (int i = 0; i < TAG_BRANCH_LEN; ++i){
        //str_rand += alpha [ rand() % (sizeof(alpha) - 1) ];
        str_rand += alnum [ rand() % (sizeof(alnum) - 1) ];
    }
    str = QString("z9hG4bK%1%2").arg(str_rand).arg(suffix);
    return str;
}

QString CUserAgent::genRandomTag()
{/*The branch parameter always begins with the same string of seven characters — “z9hG4bK.”
This requirement was added to identify that the branch was created in accordance with RFC 3261 and not the older RFC 2543 which did not require global uniqueness*/
    QString str,str_rand;
    srand(time(NULL));
    static const char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const char alnum[] = "0123456789";
    for (int i = 0; i < TAG_BRANCH_LEN; ++i){
        //str_rand += alpha [ rand() % (sizeof(alpha) - 1) ];
        str_rand += alnum [ rand() % (sizeof(alnum) - 1) ];
    }
    //str = QString("%1@%2").arg(str_rand).arg(_local_address);
    str = QString("%1").arg(str_rand);
    return str;
}

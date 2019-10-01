#include "common.h"
#include "sip.h"
#include "utils.h"
#include "md5.h"
#include "rtp.h"
#include "sip.h"
#include "globales.h"

void* udpSipListeningThread2(void* ctx)
{
    if ( ctx==NULL) return NULL;
    CUserAgentGroup* uag = (CUserAgentGroup*)ctx;
    mylog(5,"--> udpSipListeningThread2 on socket %d\n",uag->_sip_recv_send_socket);

    memset(&uag->_asteriskServAddr, 0, sizeof(uag->_asteriskServAddr));    /* Zero out structure */
    uag->_asteriskServAddr.sin_family = AF_INET;                 /* Internet addr family */
    uag->_asteriskServAddr.sin_addr.s_addr = inet_addr(uag->_sip_proxy_address.toLatin1().constData());  /* Server IP address */
    uag->_asteriskServAddr.sin_port   = htons(SIP_PORT_DEFAULT);     /* Server port */

    if (uag->_sip_recv_send_socket<=0)
        uag->_sip_recv_send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    mylog(5,"_sip_udp_recv_send_socket=%d",uag->_sip_recv_send_socket);
    while(uag->_sip_recv_send_socket > 0 )
    {
        struct sockaddr_in fromAddr;     /* Source address of SIP response */
        socklen_t fromAddrLen = sizeof(fromAddr);
        BYTE achBuffer[SIP_MSG_MAX_LEN];	// receive a datagram on the bound port number.
        int nb_received = recvfrom( uag->_sip_recv_send_socket,achBuffer,sizeof(achBuffer),0,(struct sockaddr *)&fromAddr,&fromAddrLen );

        if ( nb_received < 0 ){
            mylog(5, "udpSipListeningThread2 recvfrom error %d\n", nb_received);
            mylog(5, "udpSipListeningThread2 recvfrom error %d\n", nb_received);
            mylog(5, "udpSipListeningThread2 recvfrom error %d\n", nb_received);
            break;
        }
        if (uag->_asteriskServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr ){
            mylog(5, "--- udpSipListeningThread2 Error: received %d bytes from unknown source %s:%d ----",nb_received,inet_ntoa ( fromAddr.sin_addr ), ntohs ( fromAddr.sin_port ));
            continue;
        }
        mylog(5, "--- SIP:recvfrom %d bytes from %s:%d ----",nb_received,inet_ntoa ( fromAddr.sin_addr ), ntohs ( fromAddr.sin_port ));
        achBuffer[nb_received] = '\0';
        char start_line[SIP_MSG_MAX_LEN]={0};
        int nb_bytes = get_line_before_CRLF((const char*)achBuffer, nb_received, start_line, sizeof(start_line)-1 );
        if ( nb_bytes > 0 )
        {
            mylog(5,"start_line:[%s]\n",start_line);// ex appel entrant: start_line:[INVITE sip:9999@192.168.0.5:5060;LINEID=da8fc4c53766b50b1d5879473d0bbbaf SIP/2.0]
            CSipMessage* pMsg = new CSipMessage(achBuffer,nb_received);
            if (pMsg!=NULL){
                uag->_sipMsgReceived.push(pMsg);
            }
        }
    }
    mylog(5,"<-- udpSipListeningThread2 ending... \n");
    return NULL;
}

void CUserAgentGroup::run()
{
    while(!_is_stopped){

        usleep(ONE_SEC_USLEEP/2);
        /*for(int i=0;i<_uaList.size();i++){
                _uaList[i]->checkRegistration();
        }*/
        while ( !_sipMsgToSend.isEmpty() ){
            mylog(2,"%d _sipMsgToSend",_sipMsgToSend.count());
            CSipMessage* pMsg = _sipMsgToSend.pull();
            if( pMsg != NULL ){
                pMsg->prepareBufferWithSDP();

                BYTE* ptr_udp_data =  pMsg->_pBuf;
                int len_udp_data= pMsg->_preparedLen;
                BYTE* ptr_udp_data2 = pMsg->_pBufSDP;
                int len_udp_data2=pMsg->_preparedLenSDP;
                if ( _sip_recv_send_socket > 0)
                {
                    char bigbuf[1024]={0};
                    if (len_udp_data< sizeof(bigbuf))
                        memcpy(bigbuf,ptr_udp_data,len_udp_data);
                    if ((len_udp_data+len_udp_data2) < sizeof(bigbuf))
                        memcpy(bigbuf+len_udp_data,ptr_udp_data2,len_udp_data2);

                    /* Construct the server address structure */
                    memset(&_asteriskServAddr, 0, sizeof(_asteriskServAddr));    /* Zero out structure */
                    _asteriskServAddr.sin_family = AF_INET;                 /* Internet addr family */
                    _asteriskServAddr.sin_addr.s_addr = inet_addr(_sip_proxy_address.toLatin1().constData());  /* Server IP address */
                    _asteriskServAddr.sin_port   = htons(SIP_PORT_DEFAULT);     /* Server port */

                    int bytes_sent=0;
                    if ( (bytes_sent = sendto(_sip_recv_send_socket, bigbuf, len_udp_data+len_udp_data2, 0,
                                              (struct sockaddr *)&_asteriskServAddr, sizeof(_asteriskServAddr))) != len_udp_data+len_udp_data2){
                        /* Send data to the server */
                        //mylog(5,"sendto() sent a different number of bytes than expected");
                    }
                    //hexdump((BYTE*)bigbuf,len_udp_data+len_udp_data2,"udp_send2");
                    mylog(5,"============== << sip_send_with_sdp returned %d on socket %d ============",bytes_sent, _sip_recv_send_socket );
                    usleep(ONE_SEC_USLEEP/20);
                }
            }
        }
        while ( !_sipMsgReceived.isEmpty() ){
            mylog(2,"%d _sipMsgReceived",_sipMsgReceived.count());
            CSipMessage* pMsg = _sipMsgReceived.pull();//cstaLink->_sipMsgReceived.push(pCmd);
            if ( pMsg != NULL ) {
                for(int i=0;i<_uaList.size();i++){
                    if( !pMsg->_isConsummed )
                        _uaList[i]->onSIPMessage(pMsg);
                }
                delete pMsg;
            }
        }

    }
}

void CUserAgentGroup::stopNetwork()
{
    _is_stopped = true;
    close(_sip_recv_send_socket);
    usleep(ONE_SEC_USLEEP);
    pthread_cancel(_udp_sip_t_id);
    usleep(ONE_SEC_USLEEP);
    terminate();
}

void CUserAgentGroup::startRegistration()
{
    _registration_date_time = QDateTime::currentDateTime();
    qDebug() << "startRegistration:" << _registration_date_time.toString();

    QStringList phone_list;
    for(int i=0;i<_uaList.size();i++)
    {
        CUserAgent* pAgt = _uaList[i];
        //if ( !phone_list.contains(pAgt->_phone_number) )
            pAgt->sipRegister();
        //phone_list.append(pAgt->_phone_number);
    }
}

void CUserAgentGroup::stopRegistration()
{
    qDebug() << "stopRegistration:";

    QStringList phone_list;
    for(int i=0;i<_uaList.size();i++)
    {
        CUserAgent* pAgt = _uaList[i];
        //if ( !phone_list.contains(pAgt->_phone_number))
            pAgt->sipUnRegister();
        //phone_list.append(pAgt->_phone_number);
    }
    usleep(2*ONE_SEC_USLEEP);
}


void CUserAgentGroup::startNetwork()
{
    _start_date_time = QDateTime::currentDateTime(); // IDEM _start_date_time.setTime_t(time(NULL));
    qDebug() << "startNetwork:" << _start_date_time.toString();
    _sipMsgToSend.setMax(100);
    _sipMsgReceived.setMax(100);

    memset(&_asteriskServAddr, 0, sizeof(_asteriskServAddr));    /* Zero out structure */
    _asteriskServAddr.sin_family = AF_INET;                 /* Internet addr family */
    _asteriskServAddr.sin_addr.s_addr = inet_addr(_sip_proxy_address.toLatin1().constData());  /* Server IP address */
    _asteriskServAddr.sin_port   = htons(SIP_PORT_DEFAULT);     /* Server port */

    if (_sip_recv_send_socket<=0)
        _sip_recv_send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    _is_stopped = false;
    start();
    pthread_create( & _udp_sip_t_id, NULL, udpSipListeningThread2, (void*)this);
}

CUserAgentGroup::~CUserAgentGroup()
{
    for(int i=0;i<_uaList.size();i++){
        delete _uaList[i];
    }
}

void CUserAgentGroup::readSettings(){
    QSettings settings("params.ini", QSettings::IniFormat);
    qDebug() << settings.fileName();
    _local_address = settings.value("local_address").toString();
    _sip_proxy_address = settings.value("sip_proxy_address").toString();
    int size = settings.value("agent_count").toInt();
    qDebug() << settings.value("csta_server_address").toString();
    qDebug() << settings.value("csta_server_port").toString();

    for(int i=0;i<size;i++){
        int idx=i+1;
        CUserAgent* pAgt = new CUserAgent(this);
        if ( pAgt!=NULL){
            pAgt->_phone_number = settings.value(QString("agent%1/phone_number").arg(idx) ).toString();
            pAgt->_called_number = settings.value(QString("agent%1/called_number").arg(idx) ).toString();
            pAgt->_rtp_port_answer = settings.value(QString("agent%1/rtp_port_answer").arg(idx) ).toInt();
            pAgt->_rtp_port_dial = settings.value(QString("agent%1/rtp_port_dial").arg(idx) ).toInt();

            pAgt->_md5_password = settings.value(QString("agent%1/md5_password").arg(idx) ).toString();
            pAgt->init(_sip_proxy_address,_local_address,pAgt->_phone_number,pAgt->_md5_password);
            qDebug() << pAgt->_phone_number;
            qDebug() << pAgt->_md5_password;
            qDebug() << pAgt->_rtp_port_answer;
            qDebug() << pAgt->_rtp_port_dial;
            qDebug() << pAgt->_called_number;
            _uaList.append(pAgt);
        }
    }
}

void CUserAgentGroup::writeSettings()
{/*
    QCoreApplication::setOrganizationName("MySoft");
    QCoreApplication::setOrganizationDomain("mysoft.com");
    QCoreApplication::setApplicationName("Star Runner");
    QSettings settings("params.ini", QSettings::IniFormat);

    settings.setValue("sip_proxy_address",SIP_PROXY_ADDR);
    settings.setValue("local_address",_local_address);
    settings.setValue("agent_count", _uaList.size());

    int size=_uaList.size();
    for(int i=0;i<size;i++){
        int idx=i+1;
        CUserAgent* pAgt = _uaList[i];
        if ( pAgt!=NULL){
            settings.setValue(QString("agent%1/phone_number").arg(idx), pAgt->_phone_number);
            settings.setValue(QString("agent%1/rtp_port_answer").arg(idx), pAgt->_rtp_port_answer);
            settings.setValue(QString("agent%1/md5_password").arg(idx), pAgt->_md5_password);
            qDebug() << pAgt->_phone_number;
            qDebug() << pAgt->_md5_password;
            qDebug() << pAgt->_rtp_port_answer;
        }
    }*/
}


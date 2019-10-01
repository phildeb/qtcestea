#include <csta.h>
#include <utils.h>
#include <csta_gui.h>
#include "common.h"
#include "sip.h"
#include "utils.h"
#include "md5.h"
#include "rtp.h"
#include "sip.h"
#include "globales.h"

#define BYTES_20MS_ALAW_RTP 172

void CCSTARTPRecvInOutThread::run()
{
    int _rtp_rx_sock1_count=0;
    int _rtp_rx_sock2_count=0;
    char _rtp_rx_sock1_buffer[BYTES_20MS_ALAW_RTP]={0};
    char _rtp_rx_sock2_buffer[BYTES_20MS_ALAW_RTP]={0};
    static int nb_time_run=0;
    int LOOP=0;
    nb_time_run++;
    mylog(5,"CCSTARTPRecvInOutThread entered for %d time",nb_time_run);
    mylog(5,"CCSTARTPRecvInOutThread entered for %d time",nb_time_run);
    mylog(5,"CCSTARTPRecvInOutThread entered for %d time",nb_time_run);
    fd_set readset;
    while (!_isStopped)
    {
        LOOP++;
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 100*ONE_MILLISEC_USLEEP;

        int maxfd = -1;
        FD_ZERO(&readset);
        /* Add all of the interesting fds to readset */
        if ( _rtp_rx_sock1 > 0 || _rtp_rx_sock2 > 0) {
            if ( _rtp_rx_sock1>0 ) { FD_SET(_rtp_rx_sock1, &readset);maxfd = _rtp_rx_sock1;}
            if ( _rtp_rx_sock2>0 ) { FD_SET(_rtp_rx_sock2, &readset);maxfd = _rtp_rx_sock2;}
            if ( _rtp_rx_sock1 > _rtp_rx_sock2)
                maxfd = _rtp_rx_sock1;
        }else{
            usleep(2*ONE_SEC_USLEEP);
            mylog(5,"CCSTARTPRecvInOutThread usleep %d usec...(_rtp_rx_sock1=%d _rtp_rx_sock2=%d)",
                  2*ONE_SEC_USLEEP,_rtp_rx_sock1,_rtp_rx_sock2);
            continue;
        }       
        int activity = select( maxfd + 1 , &readset , NULL , NULL , &tv);
        if ( activity == -1 ){
            mylog(5,"Error in Select!!!");
            _isStopped = true;
        }
        if ( activity == 0 ){
            mylog(5,"Timeout _rtp_rx_sock1=%d (%d) _rtp_rx_sock2=%d (%d) %s",
                  _rtp_rx_sock1,_rtpPort1,_rtp_rx_sock2,_rtpPort2,_extension_recorded.toLatin1().constData());
            continue;
        }
        else
        {
            if (FD_ISSET(_rtp_rx_sock1, &readset))
            {
                struct sockaddr_storage src_addr;
                socklen_t src_addr_len=sizeof(src_addr);
                _rtp_rx_sock1_count=recvfrom(_rtp_rx_sock1,
                                             &_rtp_rx_sock1_buffer[0],
                        BYTES_20MS_ALAW_RTP,
                        0,
                        (struct sockaddr*)&src_addr,
                        &src_addr_len);
                if (_rtp_rx_sock1_count == 0)
                {
                    mylog(2,"handle_close(fd[%d])",_rtp_rx_sock1);
                }
                else if (_rtp_rx_sock1_count < 0)
                {
                    mylog(2,"recvfrom err %s",strerror(errno));
                    //                            recvfrom err Mauvais descripteur de fichier
                    //handle_error fd[0] errno 88 recvfrom err Socket operation on non-socket
                    if (errno == EAGAIN){
                        mylog(2,"The kernel didn't have any data for us to read");
                    }else{
                        mylog(2,"handle_error fd[%d] errno %d",_rtp_rx_sock1,errno);
                    }
                }else{
                    if ( (LOOP % 50) ==0) mylog(5,"_rtp_rx_sock1=%d recvfrom %d bytes",_rtp_rx_sock1,_rtp_rx_sock1_count);
                    if (_rtp_rx_sock1_count>BYTES_20MS_ALAW_RTP){
                        mylog(2,"datagram too large for buffer: truncated !!");
                    }
                }
            }
            if (FD_ISSET(_rtp_rx_sock2, &readset))
            {
                struct sockaddr_storage src_addr;
                socklen_t src_addr_len=sizeof(src_addr);
                _rtp_rx_sock2_count=recvfrom(_rtp_rx_sock2,
                                             &_rtp_rx_sock2_buffer[0],
                        BYTES_20MS_ALAW_RTP,
                        0,
                        (struct sockaddr*)&src_addr,
                        &src_addr_len);
                if (_rtp_rx_sock2_count == 0)
                {
                    mylog(2,"handle_close(fd[%d])",_rtp_rx_sock1);
                }
                else if (_rtp_rx_sock2_count < 0)
                {
                    mylog(2,"recvfrom err %s",strerror(errno));
                    //                            recvfrom err Mauvais descripteur de fichier
                    //handle_error fd[0] errno 88 recvfrom err Socket operation on non-socket
                    if (errno == EAGAIN){
                        mylog(2,"The kernel didn't have any data for us to read");
                    }else{
                        mylog(2,"handle_error fd[%d] errno %d",_rtp_rx_sock2,errno);
                    }
                }else{
                    if ( (LOOP % 50) ==0) mylog(5,"_rtp_rx_sock2=%d recvfrom %d bytes",_rtp_rx_sock2,_rtp_rx_sock2_count);
                    if (_rtp_rx_sock2_count>BYTES_20MS_ALAW_RTP){
                        mylog(2,"datagram too large for buffer: truncated !!");
                    }
                }
            }


            if ( _rtp_rx_sock1_count==BYTES_20MS_ALAW_RTP && _rtp_rx_sock2_count==BYTES_20MS_ALAW_RTP )
            {

                char rtp_payload[BYTES_20MS_ALAW_RTP]={0};

                // VOICE RTP
                for ( int j = OFFSET_RTP_IN_UDP_HEADER; j < BYTES_20MS_ALAW_RTP; j++)
                { // ADD 2 FLUX
                    unsigned short rtp_buffer_pcm_in = 0;
                    {
                        rtp_buffer_pcm_in = AlawToLinear16(_rtp_rx_sock1_buffer[j]);
                        //rtp_payload[j] = Linear16ToAlaw(rtp_buffer_pcm_in);
                    }
                    unsigned short rtp_buffer_pcm_out = 0;
                    {
                        rtp_buffer_pcm_out = AlawToLinear16(_rtp_rx_sock2_buffer[j]);
                        //rtp_payload[j] = Linear16ToAlaw(rtp_buffer_pcm_out);
                    }
                    unsigned short rtp_buffer_pcm_sum = rtp_buffer_pcm_in + rtp_buffer_pcm_out;
                    //rtp_payload[j] = Linear16ToAlaw(rtp_buffer_pcm_in);
                    rtp_payload[j] = Linear16ToAlaw(rtp_buffer_pcm_sum);
                }/*for*/

                // HEADER RTP
                for ( int j = 0; j < OFFSET_RTP_IN_UDP_HEADER; j++)
                { // entete RTP
                    //if ( _rtp_rx_sock1_count>0 )
                        rtp_payload[j] = _rtp_rx_sock1_buffer[j];
                    /*else if ( _rtp_rx_sock2_count>0 )
                        rtp_payload[j] = _rtp_rx_sock2_buffer[j];*/
                }


                for (int i=0; i<MAX_SUBSCRIBER; i++ )
                {
                    if ( _sipSubscribersArray[i]._sip_rtp_tx_port > 0 )
                    {
                        _sip_rtp_tx_port = _sipSubscribersArray[i]._sip_rtp_tx_port ;
                        _sip_rtp_tx_sock = _sipSubscribersArray[i]._sip_rtp_tx_sock ;

                        if ( (LOOP % 50) ==0){
                            mylog(8,"TX %d/%d SIP RTP =>%s:%d socket %d",_rtp_rx_sock1_count,_rtp_rx_sock2_count,
                                  _sip_proxy_ip_addr.toLatin1().constData(), _sip_rtp_tx_port,_sip_rtp_tx_sock);
                            mylog(8,"from CSTA duplicate rtp port %d and %d",_rtpPort1,_rtpPort2);
                        }

                        /*  12:29:09.407 _rtp_rx_sock2=17 recvfrom 172 bytes
                            12:29:09.410 _sipSubscribersArray[0] port:10220 sock:15
                            12:29:09.411 _sipSubscribersArray[1] port:12718 sock:19
                            12:29:09.411 _sipSubscribersArray[2] port:0 sock:0
                            12:29:09.411 _sipSubscribersArray[3] port:0 sock:0
                            12:29:09.411 _sipSubscribersArray[4] port:0 sock:0                        -*/


                        if ( _sip_proxy_ip_addr.length() && _sip_rtp_tx_port && _sip_rtp_tx_sock){

                            struct sockaddr_in asteriskServAddr_rtp_tx; /* Asterisk server address */
                            memset(&asteriskServAddr_rtp_tx, 0, sizeof(asteriskServAddr_rtp_tx));    /* Zero out structure */
                            asteriskServAddr_rtp_tx.sin_family = AF_INET;                 /* Internet addr family */
                            asteriskServAddr_rtp_tx.sin_addr.s_addr = inet_addr(_sip_proxy_ip_addr.toLatin1().constData());  /* Server IP address */
                            asteriskServAddr_rtp_tx.sin_port   = htons(_sip_rtp_tx_port);     /* send to this Server port */

                            int bytes_sent = sendto(_sip_rtp_tx_sock, rtp_payload, BYTES_20MS_ALAW_RTP,
                                                    0,(struct sockaddr *)&asteriskServAddr_rtp_tx,sizeof(asteriskServAddr_rtp_tx));
                        }
                    }
                }

                // EMPTY 2 BUFFERS
                _rtp_rx_sock1_count = 0;
                _rtp_rx_sock2_count = 0;
                memset(_rtp_rx_sock1_buffer,0,sizeof(_rtp_rx_sock1_buffer));
                memset(_rtp_rx_sock2_buffer,0,sizeof(_rtp_rx_sock2_buffer));

            }
        }

    }/*while*/
    mylog(5,"CCSTARTPRecvThread ended after %d loop",LOOP);
    mylog(5,"CCSTARTPRecvThread ended after %d loop",LOOP);
    mylog(5,"CCSTARTPRecvThread ended after %d loop",LOOP);
}

bool CCSTARTPRecvInOutThread::init(QString csta_client_ip,int rtp_port1, int rtp_port2,
                                   SOCKET rtp_udp_send_socket, int sip_tx_port, QString sip_proxy_ip_addr)
{
    _isStopped = false;

    _rtp_listen_ip_addr = csta_client_ip;
    // store SIP RTP proxy adress and port to send RTP from CSTA
    _sip_proxy_ip_addr = sip_proxy_ip_addr;

    // todo :
    //addSubscriber(rtp_udp_send_socket,sip_tx_port);//_sip_rtp_tx_port = sip_tx_port;    _sip_rtp_tx_sock = rtp_udp_send_socket;


    if ( _rtp_rx_sock1>0 ){
        mylog(2,"_rtp_rx_sock1 WAS [%d]",_rtp_rx_sock1);
        close(_rtp_rx_sock1);
        _rtp_rx_sock1 = -1;
        mylog(2,"_rtp_rx_sock1 IS [%d]",_rtp_rx_sock1);
    }

    if ( _rtp_rx_sock2>0 ){
        mylog(2,"_rtp_rx_sock2 WAS [%d]",_rtp_rx_sock2);
        close(_rtp_rx_sock2);
        _rtp_rx_sock2 = -1;
        mylog(2,"_rtp_rx_sock2 IS [%d]",_rtp_rx_sock2);
    }

    time( &_ltime );
    struct tm *utc2 ;utc2 = localtime( &_ltime );
    sprintf( _strwavFilename, "./CCSTARTPRecvThread_rtp_port-%d-%d-%02d-%02d-%4d %02dh%02dm%02ds.wav",
             rtp_port1,rtp_port2,
             utc2->tm_mday,utc2->tm_mon+1,utc2->tm_year+1900,
             utc2->tm_hour, utc2->tm_min, utc2->tm_sec);


    //_bigSoundBuffer = q;
    if (1){
        char str_udp_port1[10]={0};
        if (_rtp_rx_sock1>0)
            close(_rtp_rx_sock1);
        _rtpPort1 = rtp_port1;
        sprintf(str_udp_port1,"%d",rtp_port1);

        memset(&_hints,0,sizeof(_hints));
        _hints.ai_family=AF_UNSPEC;
        _hints.ai_socktype=SOCK_DGRAM;
        _hints.ai_protocol=0;
        _hints.ai_flags=AI_PASSIVE|AI_ADDRCONFIG;

        struct addrinfo* res=0;
        int err=getaddrinfo(_rtp_listen_ip_addr.toLatin1().constData(),str_udp_port1,&_hints,&res);
        if (err!=0) {
            mylog(5,"failed to resolve local socket address (err=%d)",err);
            return false;
        }
        mylog(5,"OK resolve local socket address %s:%s!",_rtp_listen_ip_addr.toLatin1().constData(),str_udp_port1);

        _rtp_rx_sock1=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
        if (_rtp_rx_sock1==-1) {
            mylog(2,"%s _rtp_rx_sock=%d",strerror(errno),_rtp_rx_sock1);
            return false;
        }
        mylog(5,"socket created OK:%d",_rtp_rx_sock1);
        if (bind(_rtp_rx_sock1,res->ai_addr,res->ai_addrlen)==-1) {
            mylog(2,"bind %s _rtp_rx_sock=%d",strerror(errno),_rtp_rx_sock1);
            /*13:46:11.602 OK resolve local socket address 10.253.253.3:55555!
            13:46:11.602 socket created OK:17
            13:46:11.602 bind Adresse déjà utilisée _rtp_rx_sock=17*/

            int reuse_true = 1;
            setsockopt(_rtp_rx_sock1,SOL_SOCKET,SO_REUSEADDR,&reuse_true,sizeof(int));
            if (bind(_rtp_rx_sock1,res->ai_addr,res->ai_addrlen)==-1) {
                mylog(2,"bind %s _rtp_rx_sock=%d",strerror(errno),_rtp_rx_sock1);
                return false;
            }
        }
        mylog(5,"bind OK");
        freeaddrinfo(res);
    }

    if(1){
        char str_udp_port2[10]={0};
        if (_rtp_rx_sock2>0)
            close(_rtp_rx_sock2);
        _rtpPort2 = rtp_port2;
        sprintf(str_udp_port2,"%d",rtp_port2);

        memset(&_hints,0,sizeof(_hints));
        _hints.ai_family=AF_UNSPEC;
        _hints.ai_socktype=SOCK_DGRAM;
        _hints.ai_protocol=0;
        _hints.ai_flags=AI_PASSIVE|AI_ADDRCONFIG;

        struct addrinfo* res=0;
        int err=getaddrinfo(_rtp_listen_ip_addr.toLatin1().constData(),str_udp_port2,&_hints,&res);
        if (err!=0) {
            mylog(5,"failed to resolve local socket address (err=%d)",err);
            return false;
        }
        mylog(5,"OK resolve local socket address %s:%s!",_rtp_listen_ip_addr.toLatin1().constData(),str_udp_port2);

        _rtp_rx_sock2=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
        if (_rtp_rx_sock2==-1) {
            mylog(2,"%s _rtp_rx_sock=%d",strerror(errno),_rtp_rx_sock2);
            return false;
        }
        mylog(5,"socket created OK:%d",_rtp_rx_sock2);
        if (bind(_rtp_rx_sock2,res->ai_addr,res->ai_addrlen)==-1) {
            mylog(2,"bind %s _rtp_rx_sock=%d",strerror(errno),_rtp_rx_sock2);
            /*13:46:11.602 OK resolve local socket address 10.253.253.3:55555!
            13:46:11.602 socket created OK:17
            13:46:11.602 bind Adresse déjà utilisée _rtp_rx_sock=17*/

            int reuse_true = 1;
            setsockopt(_rtp_rx_sock2,SOL_SOCKET,SO_REUSEADDR,&reuse_true,sizeof(int));
            if (bind(_rtp_rx_sock2,res->ai_addr,res->ai_addrlen)==-1) {
                mylog(2,"bind %s _rtp_rx_sock=%d",strerror(errno),_rtp_rx_sock2);
                return false;
            }
        }
        mylog(5,"bind OK");
        freeaddrinfo(res);
    }
    return true;
}

void CCSTARTPRecvInOutThread::stop()
{
    mylog(5,"CCSTARTPRecvThread stop");
    //The close call only marks the TCP socket closed. It is not usable by process anymore. But kernel may still hold some resources for a period (TIME_WAIT, 2MLS etc stuff).
    _isStopped = true;
    if ( _rtp_rx_sock1>0 ){
        mylog(2,"_rtp_rx_sock1 WAS [%d]",_rtp_rx_sock1);
        close(_rtp_rx_sock1);
        _rtp_rx_sock1 = -1;
        mylog(2,"_rtp_rx_sock1 IS [%d]",_rtp_rx_sock1);
    }

    if ( _rtp_rx_sock2>0 ){
        mylog(2,"_rtp_rx_sock2 WAS [%d]",_rtp_rx_sock2);
        close(_rtp_rx_sock2);
        _rtp_rx_sock2 = -1;
        mylog(2,"_rtp_rx_sock2 IS [%d]",_rtp_rx_sock2);
    }
    usleep(ONE_SEC_USLEEP/2);
    //int retshutdown = shutdown(_rtp_rx_sock,SHUT_RDWR);
}

#if 0
void CCSTARTPRecvThread::run()
{
    int LOOP=0;
    mylog(5,"CCSTARTPRecvThread entered");
    while (1){
        LOOP++;
        char buffer[200];
        struct sockaddr_storage src_addr;
        socklen_t src_addr_len=sizeof(src_addr);
        //mylog(5,"CCSTARTPRecvThread recvfrom...");
        ssize_t count=recvfrom(_rtp_rx_sock,buffer,sizeof(buffer),0,(struct sockaddr*)&src_addr,&src_addr_len);
        //mylog(5,"CCSTARTPRecvThread recvfrom<-%d",count);
        if (count==-1) {
            mylog(2,"recvfrom err %s",strerror(errno));//recvfrom err Mauvais descripteur de fichier
            break;
        } else if (count==sizeof(buffer)) {
            mylog(2,"datagram too large for buffer: truncated");
        } else {
            //handle_datagram(buffer,count);
            //mylog(5, "UDP recvfrom %d bytes",count);
            // 160 bytes/packet => 20ms , 50 packet=>1000 ms
            char* rtp_payload = &buffer[OFFSET_RTP_IN_UDP_HEADER];
            count -= OFFSET_RTP_IN_UDP_HEADER;
            if (1){
                //mylog(2,"_cstaRtpMsgQueue.count=%d",_cstaRtpMsgQueue.count() );
                //mylog(2,"_bigSoundBuffer->count=%d",_bigSoundBuffer->count());
                CRtpMessage* pRtpPacket = new CRtpMessage((BYTE*)rtp_payload,count);
                if ( pRtpPacket ) {
                    _cstaRtpMsgQueue.append(pRtpPacket);
                    if ( _cstaRtpMsgQueue.count() >= RTP_PACKET_BUNCH )
                    {
                        if ( (LOOP % 50) ==0) mylog(2,"_cstaRtpMsgQueue.count=RTP_PACKET_BUNCH !!! (%d)",_cstaRtpMsgQueue.count() );
                        CVoicePacket* pVoicePacket = new CVoicePacket();
                        if (pVoicePacket){
                            // copy all RTP packet
                            if ( !_bigSoundBuffer->isFull() ) {
                                pVoicePacket->copySound(&_cstaRtpMsgQueue);
                                if ( (LOOP % 50) ==0) mylog(2,"pushed NEW VOICE PAQUET");
                                _bigSoundBuffer->push(pVoicePacket); // thread safe !
                                if ( (LOOP % 50) ==0) mylog(2,"NOW queue contains %d VOICE PAQUET",_bigSoundBuffer->count());
                            }else{
                                mylog(2,"FULL !!! queue contains %d VOICE PAQUET",_bigSoundBuffer->count());
                                mylog(2,"FULL !!! queue contains %d VOICE PAQUET",_bigSoundBuffer->count());
                                mylog(2,"FULL !!! queue contains %d VOICE PAQUET",_bigSoundBuffer->count());
                                mylog(2,"FULL !!! queue contains %d VOICE PAQUET",_bigSoundBuffer->count());
                                delete pVoicePacket;
                            }
                        }
                        _cstaRtpMsgQueue.clear(); // ASSERT not calling CRtpMessage destructor !
                        if ( (LOOP % 50) ==0) mylog(2,"_cstaRtpMsgQueue.clear=>_cstaRtpMsgQueue.count=%d",_cstaRtpMsgQueue.count() );
                    }else{
                        if ( (LOOP % 50) ==0) mylog(2,"new CRtpMessage _cstaRtpMsgQueue.count=%d < RTP_PACKET_BUNCH",_cstaRtpMsgQueue.count() );
                    }
                }
            }
            if (0){ //mylog(5,"csta recording in %s",_strwavFilename);
                FILE* fwav = fopen(_strwavFilename,"a+b");
                if ( NULL!=fwav ){
                    /*char rtp_payload_inv[MAX_MSGLEN];
                for ( int k=0; k < err ;k++){
                    BYTE orig=rtp_buffer[k];
                    BYTE dest=0; // reverse the bits!
                    for (int j=0;j<8;j++){dest |= ( ( orig >> j ) &  1 )  << (7-j);}
                    rtp_payload_inv[k] = dest;
                }
                fwrite(rtp_payload_inv, 1, err, fwav ) ;*/
                    int nb_written = fwrite(rtp_payload, 1, count, fwav ) ;
                    if (nb_written!=count)
                        mylog(5,"err :csta:%d rtp byte in %s",nb_written,_strwavFilename);
                    fclose(fwav);
                }
            }
            if ( (LOOP % 50) ==0) mylog(8,"CSTA:g_sound_msg_queue_in %d g_sound_msg_queue_out %d",g_sound_msg_queue_in.count(),g_sound_msg_queue_out.count());
        }
    }
    mylog(5,"CCSTARTPRecvThread ended");
}

void CCSTARTPRecvThread::run()
{
    int LOOP=0;
    mylog(5,"CCSTARTPRecvThread entered");
    fd_set readset;
    while (1)
    {
        LOOP++;
        char buffer[256]={0};

        int maxfd = -1;
        FD_ZERO(&readset);

        int n_sockets=1;
        int fd[1];
        fd[0] = _rtp_rx_sock;
        /* Add all of the interesting fds to readset */
        for (int i=0; i < n_sockets; ++i) {
            if (fd[i]>maxfd) maxfd = fd[i];
            FD_SET(fd[i], &readset);
        }
        /* Process all of the fds that are still set in readset */
        for (int i=0; i < n_sockets; ++i) {
            if (FD_ISSET(fd[i], &readset)) {

                struct sockaddr_storage src_addr;
                socklen_t src_addr_len=sizeof(src_addr);
                //mylog(5,"CCSTARTPRecvThread recvfrom...");
                ssize_t count=recvfrom(fd[i],buffer,sizeof(buffer),0,(struct sockaddr*)&src_addr,&src_addr_len);

                //n = recvfrom(fd[i], buf, sizeof(buf), 0);
                if (count == 0) {
                    mylog(2,"handle_close(fd[%d])",i);
                } else if (count < 0) {
                    mylog(2,"recvfrom err %s",strerror(errno));//recvfrom err Mauvais descripteur de fichier
                    if (errno == EAGAIN){
                        mylog(2,"The kernel didn't have any data for us to read");
                    }else{
                        mylog(2,"handle_error fd[%d] errno %d",i,errno);
                    }
                } else {
                    //handle_input(fd[i], buf, n);
                    //mylog(5,"CCSTARTPRecvThread recvfrom<-%d",count);
                    if (count==sizeof(buffer)) {
                        mylog(2,"datagram too large for buffer: truncated !!");
                    }
                    if(1)
                    {
                        //handle_datagram(buffer,count);
                        //mylog(5, "UDP recvfrom %d bytes",count);
                        // 160 bytes/packet => 20ms , 50 packet=>1000 ms
                        char* rtp_payload = &buffer[OFFSET_RTP_IN_UDP_HEADER];
                        count -= OFFSET_RTP_IN_UDP_HEADER;
                        if (1){
                            //mylog(2,"_cstaRtpMsgQueue.count=%d",_cstaRtpMsgQueue.count() );
                            //mylog(2,"_bigSoundBuffer->count=%d",_bigSoundBuffer->count());
                            CRtpMessage* pRtpPacket = new CRtpMessage((BYTE*)rtp_payload,count);
                            if ( pRtpPacket ) {
                                _cstaRtpMsgQueue.append(pRtpPacket);
                                if ( _cstaRtpMsgQueue.count() >= RTP_PACKET_BUNCH )
                                {
                                    if ( (LOOP % 50) ==0) mylog(2,"_cstaRtpMsgQueue.count=RTP_PACKET_BUNCH !!! (%d)",_cstaRtpMsgQueue.count() );
                                    CVoicePacket* pVoicePacket = new CVoicePacket();
                                    if (pVoicePacket){
                                        // copy all RTP packet
                                        if ( !_bigSoundBuffer->isFull() ) {
                                            pVoicePacket->copySound(&_cstaRtpMsgQueue);
                                            if ( (LOOP % 50) ==0) mylog(2,"pushed NEW VOICE PAQUET");
                                            _bigSoundBuffer->push(pVoicePacket); // thread safe !
                                            if ( (LOOP % 50) ==0) mylog(2,"NOW queue contains %d VOICE PAQUET",_bigSoundBuffer->count());
                                        }else{
                                            mylog(2,"FULL !!! queue contains %d VOICE PAQUET",_bigSoundBuffer->count());
                                            mylog(2,"FULL !!! queue contains %d VOICE PAQUET",_bigSoundBuffer->count());
                                            _bigSoundBuffer->clean();
                                            mylog(2,"FULL !!! queue contains %d VOICE PAQUET",_bigSoundBuffer->count());
                                            mylog(2,"FULL !!! queue contains %d VOICE PAQUET",_bigSoundBuffer->count());
                                            delete pVoicePacket;
                                        }
                                    }
                                    _cstaRtpMsgQueue.clear(); // ASSERT not calling CRtpMessage destructor !
                                    if ( (LOOP % 50) ==0) mylog(2,"_cstaRtpMsgQueue.clear=>_cstaRtpMsgQueue.count=%d",_cstaRtpMsgQueue.count() );
                                }else{
                                    if ( (LOOP % 50) ==0) mylog(2,"new CRtpMessage _cstaRtpMsgQueue.count=%d < RTP_PACKET_BUNCH",_cstaRtpMsgQueue.count() );
                                }
                            }
                        }
                        if (0){ //mylog(5,"csta recording in %s",_strwavFilename);
                            FILE* fwav = fopen(_strwavFilename,"a+b");
                            if ( NULL!=fwav ){
                                /*char rtp_payload_inv[MAX_MSGLEN];
                                    for ( int k=0; k < err ;k++){
                                        BYTE orig=rtp_buffer[k];
                                        BYTE dest=0; // reverse the bits!
                                        for (int j=0;j<8;j++){dest |= ( ( orig >> j ) &  1 )  << (7-j);}
                                        rtp_payload_inv[k] = dest;
                                    }
                                    fwrite(rtp_payload_inv, 1, err, fwav ) ;*/
                                int nb_written = fwrite(rtp_payload, 1, count, fwav ) ;
                                if (nb_written!=count)
                                    mylog(5,"err :csta:%d rtp byte in %s",nb_written,_strwavFilename);
                                fclose(fwav);
                            }
                        }
                        if ( (LOOP % 50) ==0) mylog(8,"CSTA:g_sound_msg_queue_in %d g_sound_msg_queue_out %d",g_sound_msg_queue_in.count(),g_sound_msg_queue_out.count());

                    }
                }
            }
        }
    }
    mylog(5,"CCSTARTPRecvThread ended");
}

bool CCSTARTPRecvThread::init(const char* oxe_ip, const char* csta_client_ip, int rtp_port,QAsyncQueue<CVoicePacket*> *q)
{
    //char strwavFilename[200]={0};
    char str_udp_port[10]={0};

    strncpy(_OXE_IP_ADDRESS,oxe_ip,sizeof(_OXE_IP_ADDRESS));
    strncpy(_CSTA_CLIENT_IP_ADDRESS,csta_client_ip,sizeof(_CSTA_CLIENT_IP_ADDRESS));

    _bigSoundBuffer = q;

    if (_rtp_rx_sock>0)
        close(_rtp_rx_sock);

    _rtpPort = rtp_port;
    sprintf(str_udp_port,"%d",rtp_port);

    time( &_ltime );
    struct tm *utc2 ;utc2 = localtime( &_ltime );
    sprintf( _strwavFilename, "./CCSTARTPRecvThread_rtp_port%d-%02d-%02d-%4d %02dh%02dm%02ds.wav",
             rtp_port,
             utc2->tm_mday,utc2->tm_mon+1,utc2->tm_year+1900,
             utc2->tm_hour, utc2->tm_min, utc2->tm_sec);


    memset(&_hints,0,sizeof(_hints));
    _hints.ai_family=AF_UNSPEC;
    _hints.ai_socktype=SOCK_DGRAM;
    _hints.ai_protocol=0;
    _hints.ai_flags=AI_PASSIVE|AI_ADDRCONFIG;

    struct addrinfo* res=0;
    int err=getaddrinfo(_CSTA_CLIENT_IP_ADDRESS,str_udp_port,&_hints,&res);
    if (err!=0) {
        mylog(5,"failed to resolve local socket address (err=%d)",err);
        return false;
    }
    mylog(5,"OK resolve local socket address %s:%s!",_OXE_IP_ADDRESS,str_udp_port);

    _rtp_rx_sock=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if (_rtp_rx_sock==-1) {
        mylog(2,"%s _rtp_rx_sock=%d",strerror(errno),_rtp_rx_sock);
        return false;
    }
    mylog(5,"socket created OK:%d",_rtp_rx_sock);
    if (bind(_rtp_rx_sock,res->ai_addr,res->ai_addrlen)==-1) {
        mylog(2,"bind %s _rtp_rx_sock=%d",strerror(errno),_rtp_rx_sock);
        /*13:46:11.602 OK resolve local socket address 10.253.253.3:55555!
13:46:11.602 socket created OK:17
13:46:11.602 bind Adresse déjà utilisée _rtp_rx_sock=17*/

        int reuse_true = 1;
        setsockopt(_rtp_rx_sock,SOL_SOCKET,SO_REUSEADDR,&reuse_true,sizeof(int));
        if (bind(_rtp_rx_sock,res->ai_addr,res->ai_addrlen)==-1) {
            mylog(2,"bind %s _rtp_rx_sock=%d",strerror(errno),_rtp_rx_sock);
            return false;
        }
    }
    mylog(5,"bind OK");
    freeaddrinfo(res);
    return true;
}

void CCSTARTPRecvThread::stop()
{
    mylog(5,"CCSTARTPRecvThread stop");
    //The close call only marks the TCP socket closed. It is not usable by process anymore. But kernel may still hold some resources for a period (TIME_WAIT, 2MLS etc stuff).
    if (_rtp_rx_sock>0)
        close(_rtp_rx_sock);

    //int retshutdown = shutdown(_rtp_rx_sock,SHUT_RDWR);
}
#endif


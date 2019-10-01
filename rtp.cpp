#include "common.h"
#include "utils.h"
#include "sip.h"
#include "rtp.h"
#include "globales.h"

//sox toto.wav -r 8000 -e a-law -c 1 titi.wav
// G711 8000Hz alaw : 8000 octets/sec -> 160 octets/20millisecondes
#if 1
void CRTPSendThread::run()
{
    static int LOOP=0;
    gettimeofday(&tbegin,NULL);
    while( !_isStopped ) //while(_fd && !feof(_fd) && !_isStopped)
    {
        LOOP++;
        if ( (LOOP % 50) ==0)
            mylog(8,"CRTPSendThread loop %d",LOOP);
        usleep(19 * ONE_MILLISEC_USLEEP);//20 milliseconds

        char rtp_buffer[256]={0};
        //unsigned short rtp_buffer_pcm[512]={0};
        // build RTP header
        rtp_buffer[0]  |= ( RTP_VERSION_2 << 6) ;
        rtp_buffer[0]  &= VERSION_MASK ;

        rtp_buffer[1]  |= ( PT_G711_PCMA & PT_MASK );
        rtp_buffer[1]  |= MARKER_FALSE ;

        _rtp_tx_sequence_number++;
        unsigned char poids_fort = _rtp_tx_sequence_number/256;
        unsigned char poids_faible = _rtp_tx_sequence_number - (256*poids_fort);
        rtp_buffer[2] = poids_fort;
        rtp_buffer[3] = poids_faible;
        /*
            Unsigned 16 bit conversion:
            swapped = (num>>8) | (num<<8);
            Unsigned 32-bit conversion:
            swapped = ((num>>24)&0xff) | // move byte 3 to byte 0
                                ((num<<8)&0xff0000) | // move byte 1 to byte 2
                                ((num>>8)&0xff00) | // move byte 2 to byte 1
                                ((num<<24)&0xff000000); // byte 0 to byte 3
            This swaps the byte orders from positions 1234 to 4321.
            If your input was 0xdeadbeef, a 32-bit endian swap might have output of 0xefbeadde.
            */
        // timestamp sur 4 octets:
        _my_timestamp += 160;
        unsigned long swapped_ts= ((_my_timestamp>>24)&0xff) | // move byte 3 to byte 0
                ((_my_timestamp<<8)&0xff0000) | // move byte 1 to byte 2
                ((_my_timestamp>>8)&0xff00) | // move byte 2 to byte 1
                ((_my_timestamp<<24)&0xff000000); // byte 0 to byte 3
        unsigned long* ptr_timestamp = (unsigned long*)( &rtp_buffer[4] );
        (*ptr_timestamp) = swapped_ts;
        //mylog(2,"%x != %x",_my_timestamp,swapped_ts);
        /*//poids_fort = (unsigned char) myts/256;
                //poids_faible = (unsigned char) (myts  - (256*poids_fort) );
                unsigned char b3 = (unsigned char) myts >> 24 & 0x0ff;
                unsigned char b2 = (unsigned char) myts >> 16 & 0x0ff;
                unsigned char b1 = (unsigned char) myts >> 8 & 0x0ff;
                unsigned char b0 = (unsigned char) myts >> 8 & 0x0ff;
                rtp_buffer[4] = b3;
                rtp_buffer[5] = b2;
                rtp_buffer[6] = b1;
                rtp_buffer[7] = b0;
                mylog(2,"b3 %02X b2 %02X b1 %02X b0 %02X",b3,b2,b1,b0);*/
        unsigned long* ptr_no_source_identifier = (unsigned long*)( &rtp_buffer[8] );
        (*ptr_no_source_identifier) = 0xABADCAFE; // dans wireshark: FECA ADAB

        char* rtp_payload = &rtp_buffer[12];

        if (1)
        {

            int nread = fread(rtp_payload, 1 , 160, _fd ) ;
            if ( (LOOP % 50) ==0)
                mylog(2,"fread %d bytes for 20ms",nread);

            if ( nread == 0 )
                continue;
            /*for ( int k=0; k < nread ;k++)
                    {
                        short orig=rtp_buffer_orig[k];
                        rtp_payload[k]ueue read 160 bytes for 20ms
16:07:23.093 sendto() 0 bytes sent
16:07:23.093 texec = 20,000000 msec
16:07:23.111 CUserAgent g_queue.pull 0x7fe5fc000910 ...
16:07:23.111 ==> onSIPMessage 0x7fe5f = Linear16ToAlaw(orig);
                    }
                    for ( int k=0; k < nread ;k++)
                    {
                        BYTE orig=rtp_buffer_orig[k];
                        //BYTE dest=0; // reverse the bits!
                        //for (int j=0;j<8;j++){dest |= ( ( orig >> j ) &  1 )  << (7-j);}
                        rtp_payload[k] = orig;
                    }*/
            if ( (LOOP % 50) ==0)
                mylog(5,"TRY sendto() RTP %d bytes upd port %d",nread+12,_rtpPort);

            struct sockaddr_in asteriskServAddr_rtp_tx; /* Asterisk server address */
            memset(&asteriskServAddr_rtp_tx, 0, sizeof(asteriskServAddr_rtp_tx));    /* Zero out structure */
            asteriskServAddr_rtp_tx.sin_family = AF_INET;                 /* Internet addr family */
            asteriskServAddr_rtp_tx.sin_addr.s_addr = inet_addr(_rtp_proxy_ip_addr.toLatin1().constData());  /* Server IP address */
            asteriskServAddr_rtp_tx.sin_port   = htons(_rtpPort);     /* send to this Server port */

            int bytes_sent=0;
            if ( (bytes_sent = sendto(_rtp_udp_send_socket, rtp_buffer, nread+12, 0,(struct sockaddr *)&asteriskServAddr_rtp_tx,sizeof(asteriskServAddr_rtp_tx)) )
                    != (nread+12))        /* Send the string to the server */
            {
                //mylog(5,"sendto() sent a different number of bytes than expected");
            }
            //mylog(5,"sendto() %d bytes sent",bytes_sent);

            if (feof(_fd))
                _fd = fopen(_wavFilepath.toLatin1().constData(),"r+b");
        }

        if (_isStopped){
            fclose(_fd);
            _fd = NULL;
            close(_rtp_udp_send_socket);
            _rtp_udp_send_socket = -1;
            break;
        }
        gettimeofday(&tend,NULL);
        texec=((double)(1000*(tend.tv_sec-tbegin.tv_sec) +( (tend.tv_usec-tbegin.tv_usec) / 1000) ));
        if ((LOOP % 50)==0) mylog(2,"texec = %f msec",texec);
        memcpy( (void*)&tbegin, (void*)&tend, sizeof(tbegin) );
        /*if (texec <= 21.0){
            unsigned long wait_ms = (unsigned long)(20-texec);
            mylog(2,"wait_ms = %d msec",wait_ms);
            usleep( wait_ms * 1000);//20 milliseconds        }*/
    }
    if (_fd)
        fclose(_fd);
    if ( _rtp_udp_send_socket > 0)
        close(_rtp_udp_send_socket);
}

void CRTPSendThread::stop()
{
    _isStopped = true;
    usleep(1000*1000);//fclose(_fd);    close(_rtp_udp_send_socket);
    terminate();
}


bool CRTPSendThread::init(QString filepath,int rtp_port, QString sip_proxy_ip_addr)
{
    _rtp_proxy_ip_addr = sip_proxy_ip_addr;
    texec=0.0;
    _currentVoicePacketIn = NULL;
    _isStopped = false;
    _rtpPort = rtp_port;
    _wavFilepath = filepath;

    char portname[25]={0};
    sprintf(portname,"%d",_rtpPort);

    struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_protocol=IPPROTO_UDP;
    //hints.ai_flags=AI_PASSIVE|AI_ADDRCONFIG;

    struct addrinfo* server=0;
    int err=getaddrinfo(_rtp_proxy_ip_addr.toLatin1().constData(),portname,&hints,&server);
    if (err!=0) {
        mylog(1,"failed to resolve local socket address (err=%d)",err);
        return false;
    }

    _rtp_udp_send_socket = socket(server->ai_family,server->ai_socktype,server->ai_protocol);
    mylog(2,"rtp_udp_send_socket [%d]",_rtp_udp_send_socket);

    _fd = fopen(_wavFilepath.toLatin1().constData(),"r+b");// prepare sound:
    if ( NULL==_fd ){
        mylog(2,"open audio file error [%s]",_wavFilepath.toLatin1().constData());
        close(_rtp_udp_send_socket);
        return false;
    }else{
        mylog(2,"open audio file OK [%s]",_wavFilepath.toLatin1().constData());
    }
    return true;
}
#endif

#if 0

static unsigned short rtp_tx_sequence_number = 0;
static FILE* fwav_tx = 0;
//DWORD timestamp = 0;


struct sockaddr_in asteriskServAddr_rtp_rx; /* Asterisk server address */

void* rtp_rx_thread (void* WorkContext)
{
    char		strwavFilename[128]={0};

    sip_call_t* sip_call = (sip_call_t*)WorkContext;
    FILE* fwav=NULL;
    mylog(5, "debut de rtp_rx_thread UDP port RTP %d\n",sip_call->rtp_port_rx);
#if 0
    /* Construct the server address structure */
    memset(&asteriskServAddr_rtp_rx, 0, sizeof(asteriskServAddr_rtp_rx));    /* Zero out structure */
    asteriskServAddr_rtp_rx.sin_family = AF_INET;                 /* Internet addr family */
    asteriskServAddr_rtp_rx.sin_addr.s_addr = inet_addr(SIP_PROXY_ADDR);  /* Server IP address */
    //saUdpCli4.sin_addr.s_addr=INADDR_ANY; // Ecoute sur toutes les IP locales
    asteriskServAddr_rtp_rx.sin_port   = htons(sip_call->rtp_port_rx);     /* listen on port */
    mylog(5, "local IP address : %s:%d",inet_ntoa ( asteriskServAddr_rtp_rx.sin_addr ), ntohs ( asteriskServAddr_rtp_rx.sin_port ));
    // Create a socket for receiving data
#endif

    const char* hostname=NULL;//SIP_PROXY_ADDR; /* wildcard */
    char portname[25]={0};//="daytime"
    sprintf(portname,"%d",sip_call->rtp_port_rx);
    struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_protocol=IPPROTO_UDP;
    hints.ai_flags=AI_PASSIVE|AI_ADDRCONFIG;
    struct addrinfo* res=0;
    int err=getaddrinfo(hostname,portname,&hints,&res);
    if (err!=0) {
        mylog(1,"failed to resolve local socket address (err=%d)",err);
    }
    sip_call->rtp_recv_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    mylog(5, "created socket %d .........\n", sip_call->rtp_recv_socket);
    /*int fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if (fd==-1) {
        die("%s",sdietrerror(errno));
    }*/
    if (bind(sip_call->rtp_recv_socket,res->ai_addr,res->ai_addrlen)==-1) {
        mylog(1,"%s",strerror(errno));
        mylog(1,"rtp_rx_thread bind error %x\n",err);
        exit(-1);
    }
    {
        time_t ltime;
        struct tm *utc2 ;
        time( &ltime );
        utc2 = localtime( &ltime );
        sprintf( strwavFilename, "./rtp_%02d-%02d-%4d %02dh%02dm%02ds.wav",utc2->tm_mday,utc2->tm_mon+1,utc2->tm_year+1900,utc2->tm_hour, utc2->tm_min, utc2->tm_sec);
    }
    while(sip_call->rtp_recv_socket != SOCKET_ERROR)
    {
        struct sockaddr_in fromAddr;     /* Source address of SIP response */
        socklen_t fromAddrLen=0;
        char rtp_buffer[RTP_ALAW_PACKET_LEN];

        // receive a datagram on the bound port number.
        mylog(5, "recvfrom socket %d .........\n", sip_call->rtp_recv_socket);
        int err = recvfrom ( sip_call->rtp_recv_socket,rtp_buffer,
                             RTP_ALAW_PACKET_LEN,0,(struct sockaddr *)&fromAddr,&fromAddrLen );
        if ( err < 0 ){
            mylog(5, "rtp_rx_thread recvfrom error %d\n", err);
            Sleep(2000);
            //break;
            continue;
        }
        /*nSize = sizeof ( SOCKADDR_IN );
        err = recvfrom ( udp_rtp_socket,rtp_buffer,sizeof(rtp_buffer),0,(SOCKADDR FAR *) &asteriskServAddr_rtp_rx,&nSize );

        if ( SOCKET_ERROR == err ){
            mylog(5, "recvfrom error %d\n", err);
            break;
        }*/
        mylog(5, "rtp rx %d bytes from %s:%d",err,inet_ntoa ( asteriskServAddr_rtp_rx.sin_addr ), ntohs ( asteriskServAddr_rtp_rx.sin_port ));
        mylog(5, "rtp rx %d bytes from %s:%d",err,inet_ntoa ( asteriskServAddr_rtp_rx.sin_addr ), ntohs ( asteriskServAddr_rtp_rx.sin_port ));
        mylog(5, "rtp rx %d bytes from %s:%d",err,inet_ntoa ( asteriskServAddr_rtp_rx.sin_addr ), ntohs ( asteriskServAddr_rtp_rx.sin_port ));
        //debug_printf( "%s\n", rtp_buffer );
        char* rtp_payload = &rtp_buffer[12];
        err -= 12;
        fwav = fopen(strwavFilename,"a+b");
        if ( NULL!=fwav )
        {
            /*char rtp_payload_inv[MAX_MSGLEN];
            for ( int k=0; k < err ;k++)
            {
                BYTE orig=rtp_buffer[k];
                BYTE dest=0; // reverse the bits!
                for (int j=0;j<8;j++){dest |= ( ( orig >> j ) &  1 )  << (7-j);}
                rtp_payload_inv[k] = dest;
            }
            fwrite(rtp_payload_inv, 1, err, fwav ) ;*/
            fwrite(rtp_payload, 1, err, fwav ) ;
            fclose(fwav);
        }
    }
    return 0;
}

/*L'entête d'un paquet RTP est obligatoirement constituée de 12 octets, eventuellement suivie d'une liste d'identificateurs de sources contributeurs CSRCs dans le cas d'un mixer. Cette entête précède le "payload" qui représente les données utiles.
Les types de payload déjà standardisés sont décrits dans le fichier "rtp-payload.html".
+-------------------------+--------------------------------------
|  RTP header (12 octets) |    Payload                         ...
+-------------------------+--------------------------------------
Format de l''entête RTP
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|X|  CC   |M|     PT      |       sequence number         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           timestamp                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           synchronization source (SSRC) identifier            |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|            contributing source (CSRC) identifiers             |
|                             ....                              |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+

    * version V : 2 bits, V=2
    * padding P : 1 bit, si P=1 le paquet contient des octets additionnels de bourrage (padding) pour finir le dernier paquet.
    * extension X : 1 bit, si X=1 l'entête est suivie d'un paquet d'extension
    * CSRC count CC : 4 bits, contient le nombre de CSRC qui suivent l'entête
    * marker M : 1 bit, son interpretation est définie par un profil d'application (profile)
    * payload type PT : 7 bits, ce champ identifie le type du payload (audio, video, image, texte, html, etc.)
    * sequence number : 16 bits, sa valeur initiale est aléatoire et il s'incrémente de 1 à chaque paquet envoyé, il peut servir à détecter des paquets perdus
    * timestamp : 32 bits, réflète l'instant d'échantillonage du premier octet du paquet
    * SSRC: 32 bits, identifie de manière unique la source, sa valeur est choisie de manières aléatoire par l'application
    * CSRC : 32 bits, identifie les sources contribuantes.*/


void* rtp_tx_thread (void* WorkContext)
{
    sip_call_t* sip_call = (sip_call_t*)WorkContext;
    if ( sip_call->rtp_port_tx < 1024 ) {
        mylog(1,"bad rtp port %d", sip_call->rtp_port_tx);
        return NULL;
    }

#if 0
    {
        sip_call->rtp_send_socket2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        mylog(5, "created rtp_send_socket %d .........\n", sip_call->rtp_send_socket2);

        /* Construct the server address structure */
        struct sockaddr_in asteriskServAddr_rtp_tx; /* Asterisk server address */
        memset(&asteriskServAddr_rtp_tx, 0, sizeof(asteriskServAddr_rtp_tx));    /* Zero out structure */
        asteriskServAddr_rtp_tx.sin_family = AF_INET;                 /* Internet addr family */
        asteriskServAddr_rtp_tx. = SOCK_DGRAM,
                asteriskServAddr_rtp_tx.sin_addr.s_addr = inet_addr(SIP_PROXY_ADDR);  /* Server IP address */
        asteriskServAddr_rtp_tx.sin_port   = htons(sip_call->rtp_port_tx);     /* Server port */
    }
#else
    //const char* hostname=SIP_PROXY_ADDR; /* wildcard */

    char portname[25]={0};
    sprintf(portname,"%d",sip_call->rtp_port_tx);

    struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_protocol=IPPROTO_UDP;
    //hints.ai_flags=AI_PASSIVE|AI_ADDRCONFIG;
    struct addrinfo* server=0;
    int err=getaddrinfo(SIP_PROXY_ADDR,portname,&hints,&server);
    if (err!=0) {
        mylog(1,"failed to resolve local socket address (err=%d)",err);
        mylog(1,"failed to resolve local socket address (err=%d)",err);
        mylog(1,"failed to resolve local socket address (err=%d)",err);
        return(NULL);
    }
    //sip_call->rtp_send_socket2 = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    sip_call->rtp_send_socket2 = socket(server->ai_family,server->ai_socktype,server->ai_protocol);
    /*if (bind(sip_call->rtp_send_socket,res->ai_addr,res->ai_addrlen)==-1) {
       mylog(1,"%s",strerror(errno));
       mylog(1,"rtp_rx_thread bind error %x\n",err);
       exit(-1);
   }*/
#endif

    mylog(5,"debut de rtp_tx_thread UDP socket %d\n",sip_call->rtp_send_socket2);
    mylog(5,"debut de rtp_tx_thread UDP socket %d\n",sip_call->rtp_send_socket2);
    mylog(5,"debut de rtp_tx_thread UDP socket %d\n",sip_call->rtp_send_socket2);
    mylog(5,"debut de rtp_tx_thread UDP socket %d\n",sip_call->rtp_send_socket2);


    char buff[2]={0xaa,0x55};
    int bytes_sent=0;
    if (bytes_sent = sendto(sip_call->rtp_send_socket2, buff, sizeof(buff), 0,
                            server->ai_addr,server->ai_addrlen)
            != sizeof(buff))       /* Send the string to the server */
    {
        mylog(5,"sendto() sent a different number of bytes than expected");
    }
    mylog(5,"FIRST sendto() %d bytes sent",bytes_sent);
    mylog(5,"FIRST sendto() %d bytes sent",bytes_sent);
    mylog(5,"FIRST sendto() %d bytes sent",bytes_sent);
    mylog(5,"FIRST sendto() %d bytes sent",bytes_sent);


    //char wavpath[256]="/tmp/alaw.wav";
    //char wavpath[256]="/tmp/rtp_raw.alaw";
    char wavpath[256]="/home/phd/printemps-alaw.wav";
    fwav_tx = fopen(wavpath,"r+b");
    if ( NULL==fwav_tx ){
        mylog(2,"open audio file error [%s]",wavpath);
        mylog(2,"open audio file error [%s]",wavpath);
        mylog(2,"open audio file error [%s]",wavpath);
        mylog(2,"open audio file error [%s]",wavpath);
        return 0;
    }

    struct timeval tbegin,tend;
    double texec=0.;
    gettimeofday(&tbegin,NULL);

    unsigned long myts=0;
    mylog(2,"open audio file OK [%s]",wavpath);
    while(fwav_tx)//&& (SOCKET_ERROR != sip_call->rtp_send_socket) )
    {
        char rtp_buffer[512]={0};
        // G711 8000Hz alaw : 8000 octets/sec -> 160 octets/20millisecondes
        {
            // build RTP header
            rtp_buffer[0]  |= ( RTP_VERSION_2 << 6) ;
            rtp_buffer[0]  &= VERSION_MASK ;

            rtp_buffer[1]  |= ( PT_G711_PCMA & PT_MASK );
            rtp_buffer[1]  |= MARKER_FALSE ;

            rtp_tx_sequence_number++;
            unsigned char poids_fort = rtp_tx_sequence_number/256;
            unsigned char poids_faible = rtp_tx_sequence_number - (256*poids_fort);
            rtp_buffer[2] = poids_fort;
            rtp_buffer[3] = poids_faible;
            /*
Unsigned 16 bit conversion:

swapped = (num>>8) | (num<<8);

Unsigned 32-bit conversion:

swapped = ((num>>24)&0xff) | // move byte 3 to byte 0
                    ((num<<8)&0xff0000) | // move byte 1 to byte 2
                    ((num>>8)&0xff00) | // move byte 2 to byte 1
                    ((num<<24)&0xff000000); // byte 0 to byte 3

This swaps the byte orders from positions 1234 to 4321. If your input was 0xdeadbeef, a 32-bit endian swap might have output of 0xefbeadde.
*/
            // timestamp sur 4 octets:
            myts += 160;
            unsigned long swapped_ts= ((myts>>24)&0xff) | // move byte 3 to byte 0
                    ((myts<<8)&0xff0000) | // move byte 1 to byte 2
                    ((myts>>8)&0xff00) | // move byte 2 to byte 1
                    ((myts<<24)&0xff000000); // byte 0 to byte 3
            unsigned long* ptr_timestamp = (unsigned long*)( &rtp_buffer[4] );
            (*ptr_timestamp) = swapped_ts;
            mylog(2,"%x != %x",myts,swapped_ts);

            /*//poids_fort = (unsigned char) myts/256;
                //poids_faible = (unsigned char) (myts  - (256*poids_fort) );
                unsigned char b3 = (unsigned char) myts >> 24 & 0x0ff;
                unsigned char b2 = (unsigned char) myts >> 16 & 0x0ff;
                unsigned char b1 = (unsigned char) myts >> 8 & 0x0ff;
                unsigned char b0 = (unsigned char) myts >> 8 & 0x0ff;
                rtp_buffer[4] = b3;
                rtp_buffer[5] = b2;
                rtp_buffer[6] = b1;
                rtp_buffer[7] = b0;
                mylog(2,"b3 %02X b2 %02X b1 %02X b0 %02X",b3,b2,b1,b0);*/

            unsigned long* ptr_no_source_identifier = (unsigned long*)( &rtp_buffer[8] );
            (*ptr_no_source_identifier) = 0xABADCAFE; // dans wireshark: FECA ADAB


            char* rtp_payload = &rtp_buffer[12];
            //short rtp_buffer_orig[2*160]={0};
            int nread = fread(rtp_payload, 1 , 160, fwav_tx ) ;
            mylog(2,"fread %d bytes for 20ms",nread);

            if ( nread == 0 )
                break;
            else{
                char* rtp_payload = &rtp_buffer[12];
                /*for ( int k=0; k < nread ;k++)
                    {
                        short orig=rtp_buffer_orig[k];
                        rtp_payload[k] = Linear16ToAlaw(orig);
                    }
                    for ( int k=0; k < nread ;k++)
                    {
                        BYTE orig=rtp_buffer_orig[k];
                        //BYTE dest=0; // reverse the bits!
                        //for (int j=0;j<8;j++){dest |= ( ( orig >> j ) &  1 )  << (7-j);}
                        rtp_payload[k] = orig;
                    }*/
            }

            mylog(5,"TRY sendto() RTP %d bytes sent",nread+12);

            struct sockaddr_in asteriskServAddr_rtp_tx; /* Asterisk server address */
            memset(&asteriskServAddr_rtp_tx, 0, sizeof(asteriskServAddr_rtp_tx));    /* Zero out structure */
            asteriskServAddr_rtp_tx.sin_family = AF_INET;                 /* Internet addr family */
            asteriskServAddr_rtp_tx.sin_addr.s_addr = inet_addr(SIP_PROXY_ADDR);  /* Server IP address */
            asteriskServAddr_rtp_tx.sin_port   = htons(sip_call->rtp_port_tx);     /* Server port */

            int bytes_sent=0;
            if (bytes_sent = sendto(sip_call->rtp_send_socket2, rtp_buffer, nread+12, 0,
                                    (struct sockaddr *)&asteriskServAddr_rtp_tx,sizeof(asteriskServAddr_rtp_tx))
                    != nread+12)       /* Send the string to the server */
            {
                mylog(5,"sendto() sent a different number of bytes than expected");
            }
            mylog(5,"sendto() %d bytes sent",bytes_sent);

            gettimeofday(&tend,NULL);
            texec=((double)(1000*(tend.tv_sec-tbegin.tv_sec)+((tend.tv_usec-tbegin.tv_usec)/1000)))/1000.;
            memcpy( (void*)&tbegin, (void*)&tend, sizeof(tbegin) );

            mylog(2,"texec=%.5f",texec);

            usleep(20*1000);//20 milliseconds
            //rtp_send(&my_call,rtp_buffer, nread+12);

        }
    }
    fclose(fwav_tx);
    printf("fin de rtp_tx_thread UDP\n");
    return 0;
}


void init_call(sip_call_t* sip_call)
{
    memset(sip_call,0,sizeof(sip_call_t));
    sip_call->udp_send_socket = -1;
    sip_call->rtp_send_socket2 = -1;
    pthread_create(&udp_sip_t_id, NULL, udpListeningThread, (void*)sip_call);
}

void sip_INVITE(sip_call_t* sip_call)
{    /*Why is that? I think that authentication on every call is a very good thing. R
     * egistration does NOT equal authentication.
     * It is only a way to let Asterisk know where to find the registered extensions.
     * If there is no authentication on individual calls,
     * then possible attackers have a much eazier job when they want to use your system for :twisted: evil :twisted: puporses.
      As far as the "Correct auth, but based on stale nonce received from" message -
    I had this problem on some IP Phones. It is an IP phone issue, not an Asterisk issue.
    Asterisk just wants to let you know that the IP Phone does not change auth. parameters when it renews the SIP registration.
    i've add parameter pedantic=no to sip.conf. it works! my sip gates now reg and auth without problems on my asterisk.*/
    char msg_sdp[]={
        "v=0\r\n"
        "o=- 3342338646 3342338646 IN IP4 "IP_CALLER"\r\n"
        "s=sujetentetesdp\r\n"
        "c=IN IP4 "IP_CALLER"\r\n"
        "t=0 0\r\n"
        "a=sendrecv\r\n"
        "m=audio "RTP_PORT_STRING" RTP/AVP 8\r\n"
        "a=rtpmap:8 PCMA/8000\r\n"
        "a=rtpmap:101 telephone-event/8000"
    };
    char msg_invite[1024]=
    {
        "INVITE sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR":5060 SIP/2.0\r\n"
        "CSeq: "CSEQ_INVITE1" INVITE\r\n"
        "Via: SIP/2.0/UDP "IP_VIA":5060;rport;branch="VIA_TAG_BRANCH1"\r\n"
        "Contact: <sip:"UA_NUMBER_LOCAL"@"SIP_PROXY_ADDR":5060>\r\n"
        "Call-ID: "UA_CALLID1"\r\n"
        "From: "UA_NUMBER_LOCAL"<sip:"UA_NUMBER_LOCAL"@"SIP_PROXY_ADDR">;tag="FROM_TAG1"\r\n"
        "To: <sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR">\r\n"

        "Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING,PRACK\r\n"
        "Max-Forwards: 70\r\n"
        //"User-Agent: DebreuilSystems UA "__DATE__" "__TIME__"\r\n"
        //"Subject: Phone call\r\n"
        "Content-Type: application/sdp\r\n"
        "\r\n"
    };
    char stringtocopy[256]={0};sprintf(stringtocopy,"Content-Length: %d",sizeof(msg_sdp));
    int nb = str_append_CRLF(stringtocopy, msg_invite,sizeof(msg_invite) );

    udp_send2(sip_call, msg_invite, nb, msg_sdp , sizeof(msg_sdp) );
}

void sip_INVITE_with_auth(sip_call_t* sip_call,char* digest)
{    /*Why is that? I think that authentication on every call is a very good thing. R
     * egistration does NOT equal authentication.
     * It is only a way to let Asterisk know where to find the registered extensions.
     * If there is no authentication on individual calls,
     * then possible attackers have a much eazier job when they want to use your system for :twisted: evil :twisted: puporses.
      As far as the "Correct auth, but based on stale nonce received from" message -
    I had this problem on some IP Phones. It is an IP phone issue, not an Asterisk issue.
    Asterisk just wants to let you know that the IP Phone does not change auth. parameters when it renews the SIP registration.
    i've add parameter pedantic=no to sip.conf. it works! my sip gates now reg and auth without problems on my asterisk.*/
    char msg_sdp[]={
        "v=0\r\n"
        "o=- 3342338646 3342338646 IN IP4 "IP_CALLER"\r\n"
        "s=sujetentetesdp\r\n"
        "c=IN IP4 "IP_CALLER"\r\n"
        "t=0 0\r\n"
        "a=sendrecv\r\n"
        "m=audio "RTP_PORT_STRING" RTP/AVP 8\r\n"
        "a=rtpmap:8 PCMA/8000\r\n"
        "a=rtpmap:101 telephone-event/8000"
    };
    char msg_invite[1024]=
    {
        "INVITE sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR":5060 SIP/2.0\r\n"
        "CSeq: "CSEQ_INVITE2" INVITE\r\n"
        "Via: SIP/2.0/UDP "IP_VIA":5060;rport;branch="VIA_TAG_BRANCH2"\r\n"
        "Contact: <sip:"UA_NUMBER_LOCAL"@"SIP_PROXY_ADDR":5060>\r\n"
        "Call-ID: "UA_CALLID1"\r\n"
        "From: "UA_NUMBER_LOCAL"<sip:"UA_NUMBER_LOCAL"@"SIP_PROXY_ADDR">;tag="FROM_TAG1"\r\n"
        "To: <sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR">\r\n"

        "Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING,PRACK\r\n"
        "Max-Forwards: 70\r\n"
        //"User-Agent: DebreuilSystems UA 1.0\r\n"
        //"Subject: Phone call\r\n"
        "\r\n"
    };
    str_append_CRLF(digest, msg_invite,sizeof(msg_invite) );
    str_append_CRLF("Content-Type: application/sdp",msg_invite,sizeof(msg_invite) );

    char stringtocopy[256]={0};
    sprintf(stringtocopy,"Content-Length: %d",sizeof(msg_sdp));
    int nb = str_append_CRLF(stringtocopy, msg_invite,sizeof(msg_invite) );//set_invite_content_len(ptr_udp_data,len_udp_data);

    udp_send2(sip_call, msg_invite, nb, msg_sdp , sizeof(msg_sdp));//int udp_send(const char* data, int data_len)
}


void sip_ack_invite(sip_call_t* sip_call,char* to_tag)
{
    char msg_ack[1024]={
        "ACK sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR" SIP/2.0\r\n"
        "Via: SIP/2.0/UDP "IP_CALLER";rport;branch="VIA_TAG_BRANCH1"\r\n"
        "Call-ID: "UA_CALLID1"\r\n"
        "CSeq: "CSEQ_INVITE1" ACK\r\n"
        "From: "UA_NUMBER_LOCAL"<sip:"UA_NUMBER_LOCAL"@"SIP_PROXY_ADDR">;tag="FROM_TAG1"\r\n"
        //"To: <sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR">\r\n"
        "Max-Forwards: 70\r\n"
        "Content-Length=0\r\n"
        "\r\n"
    };
    char stringtocopy[1024]={0};
    int len = sprintf(stringtocopy, "To:"UA_NUMBER_CALLED" <sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR">;tag=%s" , to_tag );
    int nb = str_append_CRLF(stringtocopy, msg_ack,sizeof(msg_ack) );
    udp_send(sip_call, msg_ack, nb);
}

void sip_ack_invite_200_OK(sip_call_t* sip_call,char* to_tag)
{
    char msg_ack[1024]={
        "ACK sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR" SIP/2.0\r\n"
        "Via: SIP/2.0/UDP "IP_CALLER";rport;branch="VIA_TAG_BRANCH1"\r\n"
        "Call-ID: "UA_CALLID1"\r\n"
        "CSeq: "CSEQ_INVITE2" ACK\r\n"
        "From: "UA_NUMBER_LOCAL"<sip:"UA_NUMBER_LOCAL"@"SIP_PROXY_ADDR">;tag="FROM_TAG1"\r\n"
        //"To: <sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR">\r\n"
        "Max-Forwards: 70\r\n"
        "Content-Length=0\r\n"
        "\r\n"
    };
    char stringtocopy[1024]={0};
    int len = sprintf(stringtocopy, "To:"UA_NUMBER_CALLED" <sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR">;tag=%s" , to_tag );
    int nb = str_append_CRLF(stringtocopy, msg_ack,sizeof(msg_ack) );
    udp_send(sip_call, msg_ack, nb);
}

void sip_bye(sip_call_t* sip_call)
{
    char msg_bye[1024]={
        "BYE sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR" SIP/2.0\r\n"
        "Via: SIP/2.0/UDP "IP_CALLER";rport;branch="VIA_TAG_BRANCH1"\r\n"
        "Call-ID: "UA_CALLID1"\r\n"
        "CSeq: "CSEQ_BYE" BYE\r\n"
        "From: "UA_NUMBER_LOCAL"<sip:"UA_NUMBER_LOCAL"@"SIP_PROXY_ADDR">;tag="FROM_TAG1"\r\n"
        //"To: <sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR">\r\n"
        "Max-Forwards: 70\r\n"
        "Content-Length=0\r\n"
        "\r\n"
    };
    char stringtocopy[1024]={0};
    int len = sprintf(stringtocopy, "To:"UA_NUMBER_CALLED" <sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR">;tag=%s" , sip_call->to_tag );
    int nb = str_append_CRLF(stringtocopy, msg_bye,sizeof(msg_bye) );
    udp_send(sip_call, msg_bye, nb);
}


int udp_send(sip_call_t* sip_call, const char* ptr_udp_data, int len_udp_data)
{
    int bytes_sent=0;
    mylog(5,"============== >> udp_send %d bytes on socket %d ============", len_udp_data, sip_call->udp_send_socket );
    //hexdump((BYTE*)ptr_udp_data,len_udp_data,"SENT");    return 0;


    if ( sip_call->udp_send_socket > 0)
    {
        /* Construct the server address structure */
        memset(&g_asteriskServAddr, 0, sizeof(g_asteriskServAddr));    /* Zero out structure */
        g_asteriskServAddr.sin_family = AF_INET;                 /* Internet addr family */
        g_asteriskServAddr.sin_addr.s_addr = inet_addr(SIP_PROXY_ADDR);  /* Server IP address */
        g_asteriskServAddr.sin_port   = htons(SIP_PORT_DEFAULT);     /* Server port */

        if (bytes_sent = sendto(sip_call->udp_send_socket, ptr_udp_data, len_udp_data, 0,
                                (struct sockaddr *)&g_asteriskServAddr, sizeof(g_asteriskServAddr)) != len_udp_data)       /* Send the string to the server */
        {
            mylog(5,"sendto() sent a different number of bytes than expected");
        }
        {
            hexdump((BYTE*)ptr_udp_data,bytes_sent,"udp_send");
        }
    }
    mylog(5,"============== << udp_send %d bytes on socket %d ============", bytes_sent, sip_call->udp_send_socket );
    return bytes_sent;
}

int udp_send2(sip_call_t* sip_call, char* ptr_udp_data, int len_udp_data,char* ptr_udp_data2, int len_udp_data2)
{
    mylog(5,"TRY udp_send %d bytes on socket %d", len_udp_data, sip_call->udp_send_socket );
    /*if ( sip_call->udp_send_socket < 0)    {
        sip_call->udp_send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }*/
    if ( sip_call->udp_send_socket > 0)
    {
        char bigbuf[2048]={0};
        memcpy(bigbuf,ptr_udp_data,len_udp_data);
        memcpy(bigbuf+len_udp_data,ptr_udp_data2,len_udp_data2);

        /* Construct the server address structure */
        memset(&g_asteriskServAddr, 0, sizeof(g_asteriskServAddr));    /* Zero out structure */
        g_asteriskServAddr.sin_family = AF_INET;                 /* Internet addr family */
        g_asteriskServAddr.sin_addr.s_addr = inet_addr(SIP_PROXY_ADDR);  /* Server IP address */
        g_asteriskServAddr.sin_port   = htons(SIP_PORT_DEFAULT);     /* Server port */

        int bytes_sent=0;
        if (bytes_sent = sendto(sip_call->udp_send_socket, bigbuf, len_udp_data+len_udp_data2, 0,
                                (struct sockaddr *)&g_asteriskServAddr, sizeof(g_asteriskServAddr)) != len_udp_data+len_udp_data2){
            /* Send data to the server */
            //mylog(5,"sendto() sent a different number of bytes than expected");
        }
        hexdump((BYTE*)bigbuf,len_udp_data+len_udp_data2,"udp_send2");
        return bytes_sent;
    }
    return 0;
}

void* udpListeningThread (void* WorkContext)
{
    sip_call_t* sip_call = (sip_call_t*)WorkContext;
    if ( NULL==sip_call ) {
        mylog(5,"udpListeningThread sip_call error\n");
        return 0;
    }
    /*SOCKADDR_IN saUdpCli5;  // remote IP
    saUdpCli5.sin_family=AF_INET;
    saUdpCli5.sin_addr.s_addr=INADDR_ANY; // Ecoute sur toutes les IP locales
    saUdpCli5.sin_port=htons(SIP_PORT_DEFAULT); // Ecoute sur le port 5060
    mylog(5, "local IP address : %s:%d",inet_ntoa ( saUdpCli5.sin_addr ), ntohs ( saUdpCli5.sin_port ));
    sip_call->udp_send_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if ( SOCKET_ERROR == sip_call->udp_send_socket )	{
        mylog(5,"sip_call->udp_send_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP) error\n");
        return 0;
    }*/
    sip_call->udp_send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    mylog(5,"debut de ListeningThread UDP\n");
    while (1)
    {
        char* ptr=NULL;
        /*int err = bind( sip_call->udp_send_socket, (LPSOCKADDR)&saUdpCli5, sizeof(saUdpCli5) );
        if ( err == SOCKET_ERROR ) {
            mylog(5,"udpListeningThread: bind error %x\n",err);		//closesocket( sip_call->udp_send_socket );
            Sleep(3000);
        }else{
            mylog(5,"udpListeningThread bind OK\n");
        }*/
        while(sip_call->udp_send_socket > 0 )
        {
            struct sockaddr_in fromAddr;     /* Source address of SIP response */
            socklen_t fromAddrLen = sizeof(fromAddr);

            char achBuffer[MAX_MSGLEN];	// receive a datagram on the bound port number.
            int nb_received = recvfrom ( sip_call->udp_send_socket,achBuffer,MAX_MSGLEN,0,
                                         (struct sockaddr *)&fromAddr,&fromAddrLen );

            if ( nb_received < 0 ){
                mylog(5, "udpListeningThread:DoUdpServer recvfrom error %d\n", nb_received);
                mylog(5, "udpListeningThread:DoUdpServer recvfrom error %d\n", nb_received);
                mylog(5, "udpListeningThread:DoUdpServer recvfrom error %d\n", nb_received);
                Sleep(2000);
                continue;
            }
            if (g_asteriskServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr )
            {
                mylog(5,"Error: received a packet from unknown source.\n");
                mylog(5,"Error: received a packet from unknown source.\n");
                mylog(5,"Error: received a packet from unknown source.\n");
            }
            //mylog(5, "DoUdpServer recvfrom %d bytes", err);
            achBuffer[nb_received] = '\0';
            mylog(5, "--- SIP:recvfrom %d bytes from %s:%d ----",
                  nb_received,inet_ntoa ( fromAddr.sin_addr ), ntohs ( fromAddr.sin_port ));

            char start_line[MAX_MSGLEN]={0};
            int nb_bytes = get_line_before_CRLF(achBuffer, nb_received, start_line, sizeof(start_line)-1 );

            if ( nb_bytes > 0 )
            {
                mylog(5,"start_line:[%s]\n",start_line);
                mylog(5,"start_line:[%s]\n",start_line);
                mylog(5,"start_line:[%s]\n",start_line);
                mylog(5,"start_line:[%s]\n",start_line);
                // appel entrant: start_line:[INVITE sip:9999@192.168.0.5:5060;LINEID=da8fc4c53766b50b1d5879473d0bbbaf SIP/2.0]

                //ptr = (char*)strstr((const char *) start_line, (const char*)"SIP/");

                if (0 == strncmp ((const char *) start_line, (const char *) "SIP/", 4))
                {
                    mylog(5,"SIP RESPONSE");
                    //message_startline_parseresp(start_line);
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

                            char *reasonphrase = NULL;
                            reasonphrase = strchr (statuscode + 1, ' ');	/* search for 2nd SPACE */
                            if ( reasonphrase != NULL )
                            {
                                *reasonphrase = 0;
                                reasonphrase++;

                                mylog(5,"reasonphrase[%s]",reasonphrase);
                            }

                            char* str_CSeq = strstr(achBuffer,"CSeq:");
                            int CSeq = 0;
                            if ( NULL!=str_CSeq )
                            {
                                char* ptr = str_CSeq + strlen("CSeq:");
                                //mylog(5,"before:%s\n",ptr);
                                while (' '== *ptr)	ptr++; /* Skip spaces */
                                while ( ( *ptr >= '0') && ( *ptr <='9') ){
                                    CSeq = (CSeq*10) + ( *ptr - '0' );
                                    mylog(5,"No de sequence:%d\n",CSeq);
                                    ptr++;
                                }
                                //mylog(5,"behind:%s\n",ptr);
                                while (' '== *ptr)	ptr++; /* Skip spaces */

                                str_CSeq = ptr;
                                char str_CSeq[256]={0};
                                strcpy_till_CRLF(str_CSeq, ptr);

                                mylog(5,"SIP RESPONSE sequence string:[%s]\n",str_CSeq);
                            }

                            if (1)
                            {
                                char* str_tag_to = strstr(achBuffer,"\r\nTo:");
                                if ( NULL!=str_tag_to )
                                {
                                    mylog(5,"try reading tag TO...");
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
                                                sip_call->to_tag[i++] = *ptr_to_tag ;
                                                sip_call->to_tag[i]='\0';
                                                ptr_to_tag++;
                                            }
                                            mylog(5,"TAG[%s]", sip_call->to_tag);
                                        }
                                    }
                                }
                            }

                            if (100==status_code)
                            {
                                mylog(5,"CODE MESSAGE 100: TRYING UNHANDLED");
                            }
                            else if (180==status_code)
                            {
                                mylog(5,"CODE MESSAGE 100: RINING UNHANDLED");
                            }
                            else if (401==status_code)
                            {
                                mylog(5,"401: demande de mot de passe");
                                // todo: pas la meme reponse si CSEQ INVITE ou CSEQ REGISTER
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
                                char* str_tag_nonce = strstr(achBuffer,"nonce=");
                                if ( NULL!=str_tag_nonce )
                                {
                                    char* ptr_nonce = str_tag_nonce + strlen("nonce=");
                                    while( *ptr_nonce==' ' || *ptr_nonce=='"' )
                                        ptr_nonce++;

                                    char nonce[256]={0};
                                    strcpy_till_CRLF(nonce, ptr_nonce);


                                    mylog(5,"401 : sequence string:[%s]\n",str_CSeq);
                                    //int strncasecmp(const char *s1, const char *s2, size_t n);
                                    if ( 0==strncasecmp(str_CSeq,"INVITE" , strlen("INVITE") ) )
                                    {
                                        mylog(5,"sip_ack_invite <- sip_ack_invite TAG:[%s]\n",sip_call->to_tag);
                                        sip_ack_invite(sip_call,sip_call->to_tag);


                                        char digest[256]={0};
                                        int digest_len=sizeof(digest);

                                        build_reply_digest(
                                                    UA_NUMBER_LOCAL, "asterisk", UA_PASSWD,
                                                    "INVITE", "sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR, nonce,
                                                    digest, digest_len);


                                        mylog(5,"sip_INVITE_with_auth <-- sequence phrase:[%s]\n",str_CSeq);
                                        sip_INVITE_with_auth(sip_call,digest);
                                    }
                                    else if ( 0==strncasecmp(str_CSeq,"REGISTER" , strlen("REGISTER") ) )
                                    {
                                        char digest[256]={0};
                                        int digest_len=sizeof(digest);

                                        build_reply_digest(
                                                    UA_NUMBER_LOCAL, "asterisk", UA_PASSWD,
                                                    "REGISTER", "sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR, nonce,
                                                    digest, digest_len);

                                        mylog(5,"sip_register <-- sequence phrase:[%s]\n",str_CSeq);
                                        sip_register(sip_call,digest, digest_len);
                                    }else{
                                        mylog(5,"UNHANDLED CSEQ [%s]", str_CSeq);
                                        mylog(5,"UNHANDLED CSEQ [%s]", str_CSeq);
                                        mylog(5,"UNHANDLED CSEQ [%s]", str_CSeq);
                                    }
                                }
                            }
                            else if (200==status_code)
                            {
                                mylog(5,"RX CODE 200");
                                mylog(5,"RX CODE 200");
                                //mylog(5,"RX CODE 200 CSEQ=%s",str_CSeq);
                                mylog(5,"No de sequence:%d\n",CSeq);
                                /*if ( 0==strncasecmp(str_CSeq,CSEQ_INVITE2,strlen(CSEQ_INVITE2) ) ){
                                    //if ( 2 == CSeq ) // CSeq: 2 BYE
                                    mylog(5,"200 suite "CSEQ_INVITE2);
                                }*/
                                //else if ( 1 == CSeq ) // CSeq: 1 INVITE
                                if ( CSeq==CSEQ_INVITE2_INT ) //0==strncasecmp(str_CSeq,CSEQ_INVITE2,strlen(CSEQ_INVITE2) ) )
                                {
                                    mylog(5,"RX CODE 200 on CSEQ %d",CSEQ_INVITE2_INT);
                                    mylog(5,"RX CODE 200 on CSEQ %d",CSEQ_INVITE2_INT);
                                    mylog(5,"RX CODE 200 on CSEQ %d",CSEQ_INVITE2_INT);
                                    sip_ack_invite_200_OK(sip_call,sip_call->to_tag);

                                    int rtp_port=-1;
                                    int codec=-1;
                                    QString s=achBuffer;
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
                                            }
                                        }
                                    }
                                    if(0){
                                        sip_call->rtp_port_tx = rtp_port;
                                        mylog(5,"sip_call->rtp_port_tx=[%d]\n",sip_call->rtp_port_tx);
                                        if (sip_call->rtp_port_rx>0){

                                            pthread_create(&rtp_tx_t_id, NULL, rtp_tx_thread, (void*)sip_call); // listen on UDP rtp_port_rx
                                        }
                                        mylog(5,"sip_call->rtp_port_rx=[%d]\n",sip_call->rtp_port_rx);
                                        sip_call->rtp_port_rx = RTP_PORT_VALUE;
                                        if (sip_call->rtp_port_rx>0){
                                            pthread_create(&rtp_rx_t_id, NULL, rtp_rx_thread, (void*)sip_call); // listen on UDP rtp_port_rx
                                        }
                                    }

                                }
                            }else{
                                mylog(5,"UNHANDLED status_code %d", status_code);
                            }
                        }//if (sscanf(statuscode + 1, "%d", &status_code) != 1)
                    }else{ //if ( nb_bytes > 0 )
                        mylog(5,"REQUEST : %s",start_line );
                    }//if ( nb_bytes > 0 )

                }
                //else if ( NULL != (ptr = strstr((const char *) start_line, (const char *) "SIP/")) )
                else //if (0)// NULL != ptr )
                {
                    mylog(5,"SIP REQUEST");
                    mylog(5,"SIP REQUEST");
                    mylog(5,"SIP REQUEST");
                    mylog(5,"SIP REQUEST");

                    char first_word[128]={0};
                    {
                        int i=0;
                        ptr=start_line;
                        while (' '!= *ptr)	{
                            first_word[i++] = *ptr;
                            ptr++;
                        }
                    }

                    mylog(5,"SIP REQUEST first_word=[%s]",first_word);
                    if (0 == strncasecmp ("INVITE", first_word, strlen("INVITE") ) )
                    {
                        char to_line[512]= {0};
                        char tag_line[512]= {0};
                        char from_line[512]= {0};
                        char call_id_line[512]= {0};

                        int rtp_port=-1;
                        int codec=-1;
                        QString s=achBuffer;
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
                                }
                            }
                        }

                        sip_call->rtp_port_tx  = rtp_port;
                        mylog(5,"sip_call->rtp_port_tx=[%d]\n",sip_call->rtp_port_tx);
                        mylog(5,"sip_call->rtp_port_tx=[%d]\n",sip_call->rtp_port_tx);
                        mylog(5,"sip_call->rtp_port_tx=[%d]\n",sip_call->rtp_port_tx);
                        mylog(5,"sip_call->rtp_port_tx=[%d]\n",sip_call->rtp_port_tx);
                        mylog(5,"sip_call->rtp_port_tx=[%d]\n",sip_call->rtp_port_tx);

                        ptr = strstr(achBuffer, "tag=");// recuperer le tag du from pour le reinjecter
                        if  (NULL != ptr )
                        {
                            int k = first_CRLF_len(ptr, sizeof(achBuffer) );
                            if ( k>0 ) memcpy(tag_line, ptr,k);tag_line[k]='\0';
                            mylog(5,"tag_line:[%s]",tag_line);
                        }
                        ptr = strstr(achBuffer, "Call-ID:");
                        if  (NULL != ptr )
                        {
                            int k = first_CRLF_len(ptr, sizeof(achBuffer) );
                            if ( k>0 ) memcpy(call_id_line, ptr,k);call_id_line[k]='\0';
                            mylog(5,"call_id_line:[%s]",call_id_line);
                        }
                        ptr = strstr(achBuffer, "To:");
                        if  (NULL != ptr )
                        {
                            int k = first_CRLF_len(ptr, sizeof(achBuffer));
                            if ( k>0 ) memcpy(to_line, ptr,k);to_line[k]='\0';
                            mylog(5,"to_line:[%s]",to_line);
                        }
                        ptr = strstr(achBuffer, "From:");
                        if  (NULL != ptr )
                        {
                            int k = first_CRLF_len(ptr, sizeof(achBuffer));
                            if ( k>0 ) memcpy(from_line, ptr,k);from_line[k]='\0';
                            mylog(5,"from_line:[%s]",from_line);
                            //ptr = strstr(ptr+strlen("From:"), "tag=");mylog(5,"From tag : %s",ptr);
                        }

                        char* str_CSeq = strstr(achBuffer,"CSeq:");
                        int CSeq = 0;
                        if ( NULL!=str_CSeq )
                        {
                            char* ptr = str_CSeq + strlen("CSeq:");
                            //mylog(5,"before:%s\n",ptr);
                            while (' '== *ptr)	ptr++; /* Skip spaces */
                            while ( ( *ptr >= '0') && ( *ptr <='9') ){
                                CSeq = (CSeq*10) + ( *ptr - '0' );
                                mylog(5,"SIP REQUEST: No de sequence:%d\n",CSeq);
                                ptr++;
                            }
                            //mylog(5,"behind:%s\n",ptr);
                            while (' '== *ptr)	ptr++; /* Skip spaces */

                            str_CSeq = ptr;
                            char str_CSeq[256]={0};
                            strcpy_till_CRLF(str_CSeq, ptr);

                            mylog(5,"SIP REQUEST sequence string:[%s]\n",str_CSeq);
                        }



                        {
                            char msg_sdp[]={
                                "v=0\r\n"
                                "o=- 3342338646 3342338646 IN IP4 "IP_CALLER"\r\n"
                                "s=sujetentetesdp\r\n"
                                "c=IN IP4 "IP_CALLER"\r\n"
                                "t=0 0\r\n"
                                "a=sendrecv\r\n"
                                "m=audio "RTP_PORT_ANSWER_STRING" RTP/AVP 8\r\n"
                                "a=rtpmap:8 PCMA/8000\r\n"
                                "a=rtpmap:101 telephone-event/8000"
                            };
                            char msg_200_OK[1024]=
                            {
                                "SIP/2.0 200 OK\r\n"
                                "Via: SIP/2.0/UDP "IP_VIA":5060;rport;branch="VIA_TAG_BRANCH1"\r\n"
                                "Contact: <sip:"UA_NUMBER_LOCAL"@"SIP_PROXY_ADDR":5060>\r\n"
                                "Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING,PRACK\r\n"
                                "Max-Forwards: 70\r\n"
                                "Content-Type: application/sdp\r\n"
                                "\r\n"
                            };
                            int nb=0;
                            char stringtocopy[256]={0};

                            sprintf(stringtocopy,"%s", call_id_line);
                            nb = str_append_CRLF(stringtocopy, msg_200_OK,sizeof(msg_200_OK) );

                            sprintf(stringtocopy,"%s", from_line);
                            nb = str_append_CRLF(stringtocopy, msg_200_OK,sizeof(msg_200_OK) );
                            sprintf(stringtocopy,"%s", to_line);
                            nb = str_append_CRLF(stringtocopy, msg_200_OK,sizeof(msg_200_OK) );
                            sprintf(stringtocopy,"CSeq: %d INVITE", CSeq );
                            nb = str_append_CRLF(stringtocopy, msg_200_OK,sizeof(msg_200_OK) );

                            sprintf(stringtocopy,"Content-Length: %d",sizeof(msg_sdp));
                            nb = str_append_CRLF(stringtocopy, msg_200_OK,sizeof(msg_200_OK) );

                            udp_send2(sip_call, msg_200_OK, nb, msg_sdp , sizeof(msg_sdp) );


                        }
                    }
                    else if (0 == strncasecmp ("ACK", first_word, strlen("ACK") ) )
                    {
                        mylog(5,"SIP REQUEST HANDLED : [%s]",first_word);
                        mylog(5,"SIP REQUEST HANDLED : [%s]",first_word);
                        mylog(5,"SIP REQUEST HANDLED : [%s]",first_word);
                        mylog(5,"SIP REQU_call->rtp_poEST HANDLED : [%s]",first_word);
                        mylog(5,"SIP REQUEST HANDLED : [%s]",first_word);


                        sip_call->rtp_port_rx = RTP_PORT_ANSWER_VALUE;

                        if (sip_call->rtp_port_tx>0){
                            mylog(5,"sip_call->rtp_port_tx=[%d]\n",sip_call->rtp_port_tx);
                            pthread_create(&rtp_tx_t_id, NULL, rtp_tx_thread, (void*)sip_call); // listen on UDP rtp_port_rx
                        }
                    }
                    else
                    {
                        mylog(5,"unknown start line");
                    }
                }
            }//if ( nb_bytes > 0 )
        }//while(sip_call->udp_send_socket != SOCKET_ERROR)
    }
    printf("udpListeningThread ended normaly\n");
    return 0;
}

#endif

#if 0
int rtp_send(sip_call_t* sip_call, const char* ptr_udp_data, int len_udp_data)
{
    int bytes_sent=0;
    mylog(5,"============== >> rtp_send %d bytes on socket %d ============",
          len_udp_data, sip_call->rtp_port_tx );
    //hexdump((BYTE*)ptr_udp_data,len_udp_data,"rtp_send");    return 0;

    if ( SOCKET_ERROR == sip_call->rtp_send_socket )
    {
        sip_call->rtp_send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if ( SOCKET_ERROR != sip_call->rtp_send_socket )
        {

            /* Construct the server address structure */
            memset(&asteriskServAddr_rtp_tx, 0, sizeof(asteriskServAddr_rtp_tx));    /* Zero out structure */
            asteriskServAddr_rtp_tx.sin_family = AF_INET;                 /* Internet addr family */
            asteriskServAddr_rtp_tx.sin_addr.s_addr = inet_addr(SIP_PROXY_ADDR);  /* Server IP address */
            //tx_rtp_asteriskServAddr.sin_addr.s_addr = htonl ( INADDR_ANY );
            asteriskServAddr_rtp_tx.sin_port   = htons(sip_call->rtp_port_tx);     /* Server port */

            if (bytes_sent = sendto(sip_call->udp_send_socket, ptr_udp_data, len_udp_data, 0,
                                    (struct sockaddr *)&asteriskServAddr_rtp_tx, sizeof(asteriskServAddr_rtp_tx)) != len_udp_data)       /* Send the string to the server */
            {
                mylog(5,"sendto() sent a different number of bytes than expected");
            }
            {
                hexdump((BYTE*)ptr_udp_data,bytes_sent,"udp_send");
            }
        }
    }
    mylog(5,"============== << udp_send %d bytes on socket %d ============", bytes_sent, sip_call->udp_send_socket );
    return bytes_sent;
}

int rtp_send(sip_call_t* sip_call, const char* ptr_udp_data, int len_udp_data)
{
    int err=0;
    if ( SOCKET_ERROR == sip_call->rtp_send_socket )
    {
        sip_call->rtp_send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if ( SOCKET_ERROR != sip_call->rtp_send_socket )
        {
            SOCKADDR_IN saUdpCli2;
            saUdpCli2.sin_family = AF_INET;
            saUdpCli2.sin_addr.s_addr = htonl ( INADDR_ANY );
            saUdpCli2.sin_port = htons ( sip_call->rtp_port_remot );

            err = bind ( sip_call->rtp_send_socket, (SOCKADDR *) &saUdpCli2, sizeof (SOCKADDR_IN) );
            if ( err == SOCKET_ERROR ) {
                debug_tx_printf("rtp_send: bind error\n");
            }else{
                debug_tx_printf("rtp_send: bind ok\n");
            }
        }
    }
    //sip_call->rtp_send_socket = sip_call->udp_send_socket ;
    if ( (SOCKET_ERROR == sip_call->rtp_send_socket)  || (sip_call->rtp_port_remot<1024) )
    {
        mylog(5,"rtp send network error on port %d",sip_call->rtp_port_remot);
    }else{
        struct sockaddr_in rtpServAddr; /* Asterisk server address */
        /* Construct the server address structure */
        memset(&rtpServAddr, 0, sizeof(rtpServAddr));    /* Zero out structure */
        rtpServAddr.sin_family = AF_INET;                 /* Internet addr family */
        rtpServAddr.sin_addr.s_addr = inet_addr(IP_CALLER);  /* Server IP address */
        rtpServAddr.sin_port   = htons(sip_call->rtp_port_remot);     /* Server port */
        if (sendto(sip_call->rtp_send_socket, ptr_udp_data, len_udp_data, 0,
                   (struct sockaddr *)&rtpServAddr, sizeof(rtpServAddr))
                != len_udp_data)
        {       /* Send the string to the server */
            mylog(5,"sendto() sent a different number of bytes than expected");
        }else{
            mylog(5,"rtp_send TX:%d bytes ------------------------------------------", err );
            mylog(5,"%s", ptr_udp_data );
        }
    }
    return err;
}

#endif

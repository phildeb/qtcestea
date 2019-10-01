#if 0
#include <sip.h>
#include <utils.h>

static pthread_t    pthread_monitoring;
static int count_sip_packet=0;
static int count_udp_packet=0;
static int count_rtp_packet=0;
static int count_loop=0;

#define LOOP_DELAY 2

// current communication
static QString 	sip_msg_first_line;
static QString 	s_cseq;
static QString 	s_callid;
static QStringList sip_msg_list;
static QStringList sdp_msg_list;
static QString s_ip_in;
static QString s_ip_out;
static unsigned short len_SDP = 0;
static unsigned short rtp_port_in=0;
static unsigned short rtp_port_out=0;
static unsigned short codec=0; // 8=alaw 18=g729 0=ulaw

#define MAX_RTP_PORT 65000
#define MIN_RTP_PORT 1024

// stockage temporaire sur INVITE
static QString mapRTPINVITE_callid[MAX_RTP_PORT]; // tableau des "callid" par port rtp in
static QMap<QString, QString> mapIPINVITE;
// stockage des communications
static QMap<QString, QString> mapIPIN;// tableau des "callid" par port ip in
static QMap<QString, QString> mapIPOUT;
static QString mapRTPIN[MAX_RTP_PORT]; // tableau des "callid" par port rtp in
static QString mapRTPOUT[MAX_RTP_PORT];

void callback_got_packet(u_char *, const struct pcap_pkthdr *header, const u_char *packet);
/* QMap<QString, int>::iterator i;
 for (i = map.begin(); i != map.end(); ++i) cout << i.key() << ": " << i.value() << endl;*/

void init_pcap(int argc, char *argv[])
{
   char *dev = NULL;       /* capture device name */
   char errbuf[PCAP_ERRBUF_SIZE];      /* error buffer */
   pcap_t *handle;            /* packet capture handle */
   char filter_exp[128];

   //char filter_exp[] = "ip";     /* filter expression [3] */
   //char filter_exp[] = "udp port 5060";     /* filter expression [3] */
   //char filter_exp[] = "udp portrange 5060-65535";     /* filter expression [3] */
   sprintf(filter_exp,"udp portrange %d-%d",MIN_RTP_PORT,MAX_RTP_PORT);     /* filter expression [3] */
   //sprintf(filter_exp,"host 93.20.94.14 and udp portrange %d-%d",MIN_RTP_PORT,MAX_RTP_PORT);     /* poste 0179718258 ip 93.20.94.14  */
   // 93.20.94.5 poste tout pres de la baie
   //sprintf(filter_exp,"udp port 5060");
   mylog(1,"PCAP filter:%s\n",filter_exp);
   int num_packets = -1;//10;         /* number of packets to capture */


   /* check for capture device name on command-line */
   if (argc < 2) {
      mylog(2, "error: unrecognized command-line options\n\n");
      exit(EXIT_FAILURE);
   }else if (argc == 2) {
      dev = argv[1];
   }else if (argc > 2) {
      mylog(2, "error: unrecognized command-line options\n\n");
      exit(EXIT_FAILURE);
   }else {
      /* find a capture device if not specified on command-line */
      dev = pcap_lookupdev(errbuf);
      if (dev == NULL) {
         mylog(2, "Couldn't find default device: %s\n",errbuf);
         mylog(1, "Couldn't find default device: %s\n",errbuf);
         exit(EXIT_FAILURE);
      }
   }



   /* get network number and mask associated with capture device */
   bpf_u_int32 mask;       /* subnet mask */
   bpf_u_int32 net;        /* ip */
   if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
      mylog(2, "Couldn't get netmask for device %s: %s\n",dev, errbuf);
      net = 0;
      mask = 0;
   }

   /* print capture info */
   mylog(1,"Device: [%s]\n", dev);

   /* open capture device */
   handle = pcap_open_live(dev, SNAP_LEN, 1, 1000, errbuf);
   if (handle == NULL) {
      mylog(2, "Couldn't open device %s: [%s]\n", dev, errbuf);
      exit(EXIT_FAILURE);
   }

   /* make sure we're capturing on an Ethernet device [2] */
   if (pcap_datalink(handle) != DLT_EN10MB) {
      mylog(2, "%s is not an Ethernet\n", dev);
      exit(EXIT_FAILURE);
   }

   /* compile the filter expression */
   struct bpf_program fp;        /* compiled filter program (expression) */
   if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
      mylog(2, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
      exit(EXIT_FAILURE);
   }

   /* apply the compiled filter */
   if (pcap_setfilter(handle, &fp) == -1) {
      mylog(2, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
      exit(EXIT_FAILURE);
   }

   /*if (pthread_create(&pthread_monitoring, NULL, monitoring_thread, NULL)) {
     perror("thread(0)");
     exit(-4);
     }*/
   /* now we can set our callback function */
   mylog(1,"entering pcap_loop...");
   pcap_loop(handle, num_packets, callback_got_packet, NULL);
}

void reset_current_dialog()
{
   sip_msg_first_line= "";
   s_cseq ="" ;
   s_callid="";
   mapRTPOUT[rtp_port_out]="";
   mapRTPIN[rtp_port_in]="";
   mapIPIN[s_ip_in]="";
   mapIPOUT[s_ip_out]="";
}

void reset_sip_msg() // reset current received sip message
{
   sip_msg_list.clear();
   sdp_msg_list.clear();
   len_SDP=0;
   sip_msg_first_line = "";
}



void process_SIP_with_SDP(const char* ip_in, const char* ip_out)
{
   QString s_call_ID_value;
   unsigned short rtp_port=-1;

   if ( sip_msg_list.size()==0 ) return;
   else {
      qDebug() << " == process_SIP_with_SDP == "<< sip_msg_list.size();
   }

   qDebug() << " == first_line "<< sip_msg_first_line;
   // process SIP HEADER to get call id
   for (int i=0; i < sip_msg_list.size(); i++)
   {
      QString s = sip_msg_list[i];
      //qDebug() << "split SIP line:"<<sip_msg_list[i];

      // TROUVER LE CALL ID
      int first_colon = s.indexOf(":");
      if ( -1!=first_colon ) {
         QString rig = s.mid(0,first_colon).trimmed();
         QString lef = s.mid(first_colon+1).trimmed();
         rig=rig.trimmed();
         lef=lef.trimmed();
         qDebug() << "separator : =>[" << rig << "] [" << lef << "]";
         if ( "CSeq" == rig ) {
            s_cseq = lef;
            qDebug() << " s_cseq = "<< s_cseq;
            qDebug() << " s_cseq = "<< s_cseq;
         }
         if ( "Call-ID" == rig ) {
            lef.replace(QChar('@'),QString("_at_"));
            s_call_ID_value = ascii_only(lef);
            qDebug() << " s_call_ID_value = "<< s_call_ID_value;
            qDebug() << " s_call_ID_value = "<< s_call_ID_value;
         }
         else{

         }
      }
   }
   // process SDP to get audio rtp port and codec
   for (int i=0; i < sdp_msg_list.size(); i++)
   {
      QString s = sdp_msg_list[i];
      //qDebug() << "analysing SDP line:"<<s;
      QStringList sl = s.split("=",QString::SkipEmptyParts);
      //for (int i=0; i < sl.size(); i++)			  {			  qDebug() << "spliting:"<<sl[i];			  }

      int first_colon = s.indexOf("=");
      if ( -1!=first_colon ) {
         QString rig = s.mid(0,first_colon).trimmed();
         QString lef = s.mid(first_colon+1).trimmed();
         qDebug() << "split [" << rig << "] [" << lef << "]";
      }

      // TROUVER LE PORT RTP AUDIO et CODEC
      if ( s.contains("m=audio") && s.contains("RTP/AVP") ){
         qDebug() << "found m=audio  and RTP/AVP!!!";
         int idx1 = s.indexOf("m=audio");
         if ( -1!=idx1 ) {
            QString ss = s.mid(idx1+QString("m=audio").length());
            ss = ss.trimmed();
            int idx2 = ss.indexOf(" ");
            qDebug() << idx2 << " " << idx1 <<  "found after m=audio:" << ss;
            if ( -1!=idx2 ) {
               QString sss = s.mid(idx1 + QString("m=audio").length() ,idx2+1 );
               sss = sss.trimmed();
               qDebug() << "found m=audio port in string :" << sss;
               rtp_port = sss.toInt();
               qDebug() << "found rtp_port=" << rtp_port;
            }
         }
         int idx3 = s.indexOf("RTP/AVP");
         if ( -1 != idx3 ) {
            QString ss = s.mid(idx3+QString("RTP/AVP").length());
            ss = ss.trimmed();
            qDebug() << "found RTP/AVP in string :" << ss;
            int idx2 = ss.indexOf(" ");
            if ( -1!=idx2 ) {
               QString sss = s.mid(idx3 + QString("RTP/AVP").length() ,idx2+1 );
               sss = sss.trimmed();
               codec = sss.toInt();
               qDebug() << "found codec=" << codec;
            }
         }
      }
   }
   if ( sip_msg_list[0].startsWith("CANCEL")) {
      if ( s_callid == s_call_ID_value)
      {
         qDebug() << " CANCEL current comm";
         reset_current_dialog();
      }
   }
   else if ( sip_msg_list[0].startsWith("BYE")) {
         // store in temp array ...
         //mapRTPINVITE_callid[rtp_port]="";//s_call_ID_value;
         qDebug() << "BYE";
         qDebug() << "BYE";
         qDebug() << "BYE"; exit(0);
   }
   else if ( sip_msg_list[0].startsWith("INVITE")) {
         // store in temp INVITE array ...
         mapRTPINVITE_callid[rtp_port]=s_call_ID_value; // sauver le port RTP source du paquet INVITE
         mapIPINVITE[ip_in]=s_call_ID_value; // sauver l'adresse IP source du paquet INVITE pour ce callid
         s_callid = s_call_ID_value; // store current call callid !
   }
   else if ( sip_msg_list[0].startsWith("SIP/2.0 200 OK")) {
      // try to find corresponding trp_port from INVITE
      for (int j=MIN_RTP_PORT; j<MAX_RTP_PORT; j++){
         if ( mapRTPINVITE_callid[j] == s_call_ID_value) {
            qDebug() << "FOUND INVITE matching 200 OK! " << j << " " << s_call_ID_value;
            mapRTPINVITE_callid[j] =""; // vider pour prochain appel qui utilisera ce meme port RTP
            mapRTPIN[j]=s_call_ID_value;
            mapRTPOUT[j]=s_call_ID_value;
            mapRTPOUT[rtp_port]=s_call_ID_value;
            mapRTPIN[rtp_port]=s_call_ID_value;
            qDebug() << "FOUND INVITE matching 200 OK!";
            qDebug() << "FOUND INVITE matching 200 OK!";
            break;
         }
      }
      //exit(0);
   }
   reset_sip_msg(); // reset current received sip message
}

void process_SIP(const char* ip_in, const char* ip_out)
{
   qDebug() << " == first_line "<< sip_msg_first_line;
   //QStringList sl = s.split(":",QString::SkipEmptyParts);
   //for (int i=0; i < sl.size(); i++)			  {			  qDebug() << "split:"<<sl[i];			  }
   qDebug() << " == process_SIP TODO == ";
   reset_sip_msg(); // reset current received sip message
}


/*UDP Header   |-Source Port      : 5060   |-Destination Port : 5060   |-UDP Length       : 929   |-UDP Checksum     : 64489
19:23:57.309 IP Header
19:23:57.309    |-IP Version        : 4
19:23:57.309    |-IP Header Length  : 5 DWORDS or 20 Bytes
19:23:57.309    |-Type Of Service   : 184
19:23:57.309    |-IP Total Length   : 949  Bytes(Size of Packet)
19:23:57.310    |-Identification    : 1884
19:23:57.310    |-TTL      : 64
19:23:57.310    |-Protocol : 17
19:23:57.310    |-Checksum : 63718
19:23:57.310    |-Source IP        : 93.20.94.1
19:23:57.310    |-Destination IP   : 93.20.94.20
19:23:57.310 SIP UDP Header |-Source Port      : 05060 |-Destination Port : 05060 |-UDP Checksum     : 929
19:23:57.310 UDP Header (SIP)
19:23:57.310    |-Source Port      : 5060
19:23:57.310    |-Destination Port : 5060
19:23:57.310    |-UDP Length       : 929
19:23:57.310    |-UDP Checksum     : 64489
19:23:57.310 ===> PARSE SIP PACKET 922 bytes (93.20.94.1 to 93.20.94.20)
[0D]
"SIP/2.0 200 OK"   startsWith  "SIP/2.0"		*/


void process_line(QByteArray& qb,const char lb1, const char lb2,const char lb3,const char lb4,
   int idx, int size,int nb_bytes_from_last_CRLF, int nb_bytes_from_last_CRLFCRLF,const char* ip_in, const char* ip_out)
{
   //printf("process_line buf[%d]/%d ",idx,size);   printf(" CRLF[%d]/CRLFCRLF[%d] len=%d\n",nb_bytes_from_last_CRLF,nb_bytes_from_last_CRLFCRLF,qb.length());
   if ( len_SDP>0 && nb_bytes_from_last_CRLFCRLF == len_SDP ) {
      qDebug() << " == END OF SDP == ";
      qDebug() << " == END OF SDP == ";
      if ( sdp_msg_list.size()>0 && sip_msg_list.size()>0 )
         process_SIP_with_SDP(ip_in,ip_out);
   }
   if ( idx == size-1 ) {
      // dernier octet du paquet lu !
      qDebug() << " == END OF PACKET == ";
      qDebug() << " == END OF PACKET == ";
      if ( sdp_msg_list.size()>0 && sip_msg_list.size()>0 )
         process_SIP_with_SDP(ip_in,ip_out);
      else if ( sdp_msg_list.size()==0 && sip_msg_list.size()>0 )
         process_SIP(ip_in,ip_out);
      // else message SIP sur plusieurs paquets UDP ???
      return;
   }

   if ( qb.size() == 0 ){
      if ( lb1==0x0D && lb2==0x0A && lb3==0x0D && lb4==0x0A )  {
         qDebug() << " == CRLFCRLF == ";
         qDebug() << " == CRLFCRLF == ";
         if ( len_SDP == 0 )
         {
            for (int i=0; i < sip_msg_list.size(); i++)
            {
               QString str = QString("msg SIP %1/%2").arg(i).arg(sip_msg_list.size());
               QString qb = sip_msg_list[i];
               qDebug() << str << " = "<< qb ;
               int first_colon = qb.indexOf(":");
               if ( -1!=first_colon ) {
                  QString rig = qb.mid(0,first_colon).trimmed();
                  QString lef = qb.mid(first_colon+1).trimmed();
                  //qDebug() << "split [" << rig << "] [" << lef << "]";
                  if ( rig.contains("Content-Length") ){
                     if ( lef.toInt() > 0 ){
                        len_SDP = lef.toInt();
                        qDebug() << "SDP : CRLF after Content-Length="<< lef.toInt();
                        qDebug() << " == SEPARATEUR SDP == ";
                        qDebug() << " == SEPARATEUR SDP == ";
                        return;
                     }else{
                        qDebug() << "ignore with NULL Content-Length="<< lef.toInt();
                        qDebug() << " == SEPARATEUR SIP == ";
                        qDebug() << " == SEPARATEUR SIP == ";
                        process_SIP(ip_in,ip_out);
                        return;
                     }
                  }
               }
            }
         }else{
            process_SIP_with_SDP(ip_in,ip_out);
            len_SDP = 0;
            return;
         }
      }else
         return;
   }

   if ( qb.size() > 0 )
   {
      //qDebug() << "==>  analysing " << qb;
      /*QByteArray sip_cmd("SIP/2.0");
        if ( qb.startsWith(sip_cmd) ){
        qDebug() << " == START OF SIP/2.0 == ";
        qDebug() << qb << "  startsWith " << sip_cmd;		  }*/

      // TROUVER UN DEBUT DE MESSAGE SIP grace a un message connu !
      if ( qb.startsWith("SIP/2.0 401 Unauthorized") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith SIP/2.0 401 Unauthorized";
         reset_sip_msg();
      }
      if ( qb.startsWith("SIP/2.0 200 OK") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith SIP/2.0 200 OK";
         reset_sip_msg();
      }
      if ( qb.startsWith("CANCEL") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith SIP/2.0 CANCEL";
         reset_sip_msg();
      }
      if ( qb.startsWith("ACK") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith ACK";
         reset_sip_msg();
      }
      if ( qb.startsWith("OPTION") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith ACK";
         reset_sip_msg();
      }
      if ( qb.startsWith("BYE") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith BYE";
         reset_sip_msg();
      }
      if ( qb.startsWith("SUBSCRIBE") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith SUBSCRIBE";
         reset_sip_msg();
      }
      if ( qb.startsWith("NOTIFY") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith NOTIFY";
         reset_sip_msg();
      }
      if ( qb.startsWith("REGISTER") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith REGISTER";
         //"REGISTER sip:93.20.94.1 SIP/2.0"
         reset_sip_msg();
      }
      if ( qb.startsWith("SIP/2.0 1") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith SIP/2.0 1XX";
         reset_sip_msg();
      }
      if ( qb.startsWith("SIP/2.0 3") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith SIP/2.0 3XX";
         reset_sip_msg();
      }
      if ( qb.startsWith("SIP/2.0 4") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith SIP/2.0 4XX";
         reset_sip_msg();
      }
      if ( qb.startsWith("SIP/2.0 5") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith SIP/2.0 5XX";
         reset_sip_msg();
      }
      if ( qb.startsWith("SIP/2.0 6") ){
         qDebug() << " == START OF SIP MSG == ";
         qDebug() << qb << "  startsWith SIP/2.0 6XX";
         reset_sip_msg();
      }

      //qDebug() << "  nb lines=" << sip_msg_list.count();
      if ( sip_msg_list.size() == 0 ) sip_msg_first_line = qb;
      if(len_SDP>0)
      {
         sdp_msg_list << qb;
         //QString str = QString("append msg SDP No %1 [%2]").arg(sdp_msg_list.size()).arg(QString(qb)); qDebug() << str ;
      }else{
         sip_msg_list << qb;
         //QString str = QString("append msg SIP No %1 [%2]").arg(sip_msg_list.size()).arg(QString(qb)); qDebug() << str ;
      }
   }
}

void rtp_parse(struct udphdr *udph,unsigned short srcport,unsigned short dstport,const u_char * data ,
               int Size,const char* ip_in, const char* ip_out)
{
   const u_char* ptr = &data[0];
   ptr+=12; // version/P/X/CC/M/PT => 16 bits et sequence number => 16 bits
   // timestamp => 32 bits
   // SSRC id => 32 bits
   // CSRC id => 32 bits ???? present si seulement csrc_count>0
   printf("rtp_parse packet Size = %ld header size:ptr-data = %ld\n", Size, ptr-data );

   const u_char* ptr_header = &data[0];
   unsigned int *rtpheader = (unsigned int *) &ptr_header[0];

   int seqno = ntohl(rtpheader[0]);
   int version = (seqno & 0xC0000000) >> 30;
   int payloadtype = (seqno & 0x7f0000) >> 16;
   int sequ_number = (seqno & 0xffff) ;
   int csrc_count  = (seqno & 0x0f000000) ;
   int padding = seqno & (1 << 29);
   int ext = seqno & (1 << 28);
   int timestamp = ntohl(rtpheader[1]);
   int marker = seqno & (1 << 24);
   //int ssrc = rtpheader[2];
   //printf("RTP payloadtype=%d sequence %d Timestamp %d csrc_count=%x ext=%d marker=%d padding=%d\n",payloadtype,sequ_number,timestamp,csrc_count,ext,marker,padding);

   if ( (ptr-data) < Size )
   {

         qDebug() << "A" << mapRTPIN[srcport];
         qDebug() << "B" << mapRTPOUT[dstport];
         if ( mapRTPIN[srcport] == mapRTPOUT[dstport] )
         {
            printf("OK IN RTP VERSION %d payloadtype=%d padding=%x ext=%x timestamp=%x ",version,payloadtype,padding,ext,timestamp);
         }
         if ( mapRTPOUT[srcport] == mapRTPIN[dstport] )
         {
            printf("OK OUT RTP VERSION %d payloadtype=%d padding=%x ext=%x timestamp=%x ",version,payloadtype,padding,ext,timestamp);
         }
   }
}

void sip_parse(struct udphdr *udph,const u_char * data , int Size, const char* ip_src, const char* ip_dst)
{
   static int nb_bytes_from_last_CRLF=0;
   static int nb_bytes_from_last_CRLFCRLF=0;
   static u_char last_four_bytes[4]={0};

   //int srcport = udph->source;	int dstport = udph->dest;
   QByteArray s;

   mylog(9, "UDP Header (SIP)");
   mylog(9, "   |-Source Port      : %d" , ntohs(udph->source));
   mylog(9, "   |-Destination Port : %d" , ntohs(udph->dest));
   mylog(9, "   |-UDP Length       : %d" , ntohs(udph->len));
   mylog(9, "   |-UDP Checksum     : %d\n" , ntohs(udph->check));
   mylog(6,"===> PARSE SIP PACKET %d bytes (%s to %s)\n",Size,ip_src,ip_dst);
   /*00:17:32.310 ===> PARSE SIP PACKET 396 bytes (192.168.3.230 to 192.168.3.54)   */

   for ( int j=0; j<Size; j++)
   {
      u_char curbyte = data[j];

      last_four_bytes[0]=last_four_bytes[1];
      last_four_bytes[1]=last_four_bytes[2];
      last_four_bytes[2]=last_four_bytes[3];
      last_four_bytes[3]=curbyte;

      if ( isprint(curbyte) ) {
         //printf("%c",curbyte);
      }
      //else

      if ( last_four_bytes[2]==0x0D && last_four_bytes[3]==0x0A ){
         //printf("          **  CR LF **\n");
         nb_bytes_from_last_CRLF=0;
      }else{
         nb_bytes_from_last_CRLF++;
      }

      if ( last_four_bytes[0]==0x0D && last_four_bytes[1]==0x0A && last_four_bytes[2]==0x0D && last_four_bytes[3]==0x0A ){
         nb_bytes_from_last_CRLFCRLF=0;
         printf("          **  CR LF CR LF **\n");
      }else{
         //printf("[%02X]",last_four_bytes[0]); printf("[%02X]",last_four_bytes[1]);
         //printf("[%02X]",last_four_bytes[2]); printf("[%02X]\n",last_four_bytes[3]);
         nb_bytes_from_last_CRLFCRLF++;
      }

      if ( isprint(curbyte) ) { // il faut attendre un caractere non imprimable pour avoir une trame a traiter
         s+=curbyte;
         //qDebug() << s;
      }else{
         s = s.trimmed();
         process_line(s,last_four_bytes[0],last_four_bytes[1],last_four_bytes[2],last_four_bytes[3],j,Size, nb_bytes_from_last_CRLF,nb_bytes_from_last_CRLFCRLF,ip_src,ip_dst);
         s.clear();
      }
   }
}


void process_udp_packet(const u_char *Buffer , int Size)//,const char * ip_src, const char* ip_dst)
{
   //struct iphdr *iph = (struct iphdr *)(Buffer +  14);//sizeof(struct ethhdr));unsigned short iphdrlen = iph->ihl*4;
   struct udphdr *udph = (struct udphdr*)(Buffer + 14 + 20 );//iphdrlen  + sizeof(struct ethhdr));

   //int header_size =  sizeof(struct ethhdr) + iphdrlen + sizeof(struct udphdr);
   //fprintf(stdout , "\n\n>>>>>>*******************UDP Packet*************************\n");
   //print_ip_header(Buffer,Size);

   struct iphdr *iph = (struct iphdr *)(Buffer  + sizeof(struct ethhdr) );
   struct sockaddr_in source,dest;
   memset(&source, 0, sizeof(source));
   memset(&dest, 0, sizeof(dest));
   source.sin_addr.s_addr = iph->saddr;
   dest.sin_addr.s_addr = iph->daddr;
         char ip_src[128];
         char ip_dst[128];
         strcpy(ip_src,inet_ntoa(source.sin_addr) );
         strcpy(ip_dst,inet_ntoa(dest.sin_addr) );

   if (1){
      mylog(9, "IP Header\n");
      mylog(9, "   |-IP Version        : %d\n",(unsigned int)iph->version);
      mylog(9, "   |-IP Header Length  : %d DWORDS or %d Bytes\n",(unsigned int)iph->ihl,((unsigned int)(iph->ihl))*4);
      mylog(9, "   |-Type Of Service   : %d\n",(unsigned int)iph->tos);
      mylog(9, "   |-IP Total Length   : %d  Bytes(Size of Packet)\n",ntohs(iph->tot_len));
      mylog(9, "   |-Identification    : %d\n",ntohs(iph->id));
      //fprintf(stdout , "   |-Reserved ZERO Field   : %d\n",(unsigned int)iphdr->ip_reserved_zero);
      //fprintf(stdout , "   |-Dont Fragment Field   : %d\n",(unsigned int)iphdr->ip_dont_fragment);
      //fprintf(stdout , "   |-More Fragment Field   : %d\n",(unsigned int)iphdr->ip_more_fragment);
      mylog(9, "   |-TTL      : %d\n",(unsigned int)iph->ttl);
      mylog(9, "   |-Protocol : %d\n",(unsigned int)iph->protocol);
      mylog(9, "   |-Checksum : %d\n",ntohs(iph->check));
      mylog(9, "   |-Source IP        : %s\n" , inet_ntoa(source.sin_addr) );
      mylog(9, "   |-Destination IP   : %s\n" , inet_ntoa(dest.sin_addr) );
   }

   if (1){
      fprintf(stdout , "UDP Header");
      fprintf(stdout , "   |-Source Port      : %d" , ntohs(udph->source));
      fprintf(stdout , "   |-Destination Port : %d" , ntohs(udph->dest));
      fprintf(stdout , "   |-UDP Length       : %d" , ntohs(udph->len));
      fprintf(stdout , "   |-UDP Checksum     : %d" , ntohs(udph->check));
      fprintf(stdout , "\n");
   }
   if ( ntohs(udph->dest) == UDP_SIP_PORT )
   {
      count_sip_packet++;
      //print_ip_header(Buffer,Size);
      printf("SIP UDP Header |-Source Port      : %5.5d |-Destination Port : %5.5d |-UDP Checksum     : %d", ntohs(udph->source)         ,ntohs(udph->dest)               ,ntohs(udph->len)               ,ntohs(udph->check)              );
         sip_parse(udph, Buffer + (14+20+8) , Size - (14+20+8), ip_src, ip_dst);//inet_ntoa(source.sin_addr), inet_ntoa(dest.sin_addr));
   }
   else if ( ntohs(udph->dest) >= MIN_RTP_PORT)// est ce un packet RTP ???
   {
      rtp_parse(udph,ntohs(udph->source),ntohs(udph->dest),Buffer + (14+20+8) , Size - (14+20+8),ip_src,ip_dst);
   }
}


void callback_got_packet(u_char *, const struct pcap_pkthdr *header, const u_char *packet)
{
   //const struct sniff_ethernet *ethernet;  /* The ethernet header [1] */    /* define ethernet header */
   //const struct sniff_ethernet *ethernet = (struct sniff_ethernet*)(packet);    /* define/compute ip header offset */
   // const struct sniff_ip *ip;              /* The IP header */
   const struct sniff_ip *ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
   int size_ip = IP_HL(ip)*4;
   if (size_ip < 20) {
      //mylog(9,"   * Invalid IP header length: %u bytes\n", size_ip);
      return;
   }

   int size = header->len;
   switch(ip->ip_p) {
      case IPPROTO_UDP:
         //printf("   Protocol: UDP\n");
         count_udp_packet++;
         process_udp_packet(packet , size);
         return;
         /*case IPPROTO_ICMP:
           mylog(9,"   Protocol: ICMP\n");
           return;
           case IPPROTO_TCP:
           mylog(9,"   Protocol: TCP\n");
           break;
           case IPPROTO_IP:
           mylog(9,"   Protocol: IP\n");
           return;
           default:
           mylog(9,"   Protocol: unknown %x\n",ip->ip_p);
           return;*/
   }

}
#endif

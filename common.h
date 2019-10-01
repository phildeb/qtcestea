#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <openssl/md5.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ctype.h>
#include <syslog.h>
#include <math.h>

#include <arpa/inet.h> //inet_addr
#include <sys/socket.h>    //socket
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <arpa/inet.h> // for inet_ntoa()
#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/ip_icmp.h>   //Provides declarations for icmp header
#include <netinet/udp.h> //Provides declarations for udp header
#include <netinet/tcp.h> //Provides declarations for tcp header
#include <netinet/ip.h>  //Provides declarations for ip header
#include <arpa/inet.h> //inet_addr
#include <sys/socket.h>    //socket
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <arpa/inet.h> // for inet_ntoa()
#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/ip_icmp.h>   //Provides declarations for icmp header
#include <netinet/udp.h> //Provides declarations for udp header
#include <netinet/tcp.h> //Provides declarations for tcp header
#include <netinet/ip.h>  //Provides declarations for ip header

#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <sstream>
#include <QtCore>
#include <QString>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QMap>

#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <sstream>
using namespace std; // std:string

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#define SOCKET int
#define SOCKET_ERROR -1
#define MIN(a,b) (a<b?a:b)
#define Sleep(a) usleep(a*1000*1000)
#define APP_NAME        "csta_recorder"
#define APP_COPYRIGHT   "Copyright (c) 2005-201 Debreuil Systems SAS France"

#define ONE_MILLISEC_USLEEP (1000)
#define ONE_SEC_USLEEP (1000*1000)

#endif // COMMON_H

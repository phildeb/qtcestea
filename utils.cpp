#include <common.h>
#include <utils.h>
#include <termios.h>
#include "mainwindow.h"

static struct termios oldSettings;
static struct termios newSettings;

static int log_level = 4;

void CDebugThread::run()
{
    while(!_isStopped){
        sleep(10);
        QSettings settings("params.ini", QSettings::IniFormat);
        qDebug() << settings.fileName();
        log_level = settings.value("log_level").toInt();
        qDebug() << "log_level="<<log_level;
    }
}


/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
  tcgetattr(0, &oldSettings); /* grab old terminal i/o settings */
  newSettings = oldSettings; /* make new settings same as old settings */
  newSettings.c_lflag &= ~ICANON; /* disable buffered i/o */
  newSettings.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &newSettings); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
  tcsetattr(0, TCSANOW, &oldSettings);
}

void tstart(struct timeval* _tstart)
{
    gettimeofday(_tstart,NULL);
}
void tend(struct timeval* _tend)
{
    gettimeofday(_tend,NULL);
}

double tval(struct timeval _tstart,struct timeval _tend)
{
    double t1, t2;
    t1 =  (double)_tstart.tv_sec
            + (double)_tstart.tv_usec/(1000*1000);
    t2 =  (double)_tend.tv_sec +
            (double)_tend.tv_usec/(1000*1000);
    return t2-t1;
}

char* strcpy_till_CRLF(char* dataout, char* datain)
{
    if (dataout==NULL || datain==NULL )
        return NULL;

    char* ptrin = datain;
    char* ptrout = dataout;

    while( *ptrin!=0 && *ptrin!='\r' && *ptrin!='\n' && *ptrin!='"'  )
    {
        *ptrout++=*ptrin++;
        *ptrout=0;
        //mylog(5,"dataout=[%s]\n",dataout);
    }
    mylog(5,"strcpy_till_CRLF dataout=[%s]\n",dataout);
    return dataout;
}

char* strstr_ignoring_CRLF(char* stringtofind, char* data,int size)
{
    if (stringtofind==NULL || data==NULL || size==0 )
        return NULL;
    char* ptr = data;
    size-=strlen(stringtofind);
    // rechercher le premier debut du SDP (RFC2327)
    for (int i=0; i<size; i++,ptr++)
    {
        if ( strstr(ptr,stringtofind) == 0 )
        {
            // found !
            return ptr-1;
        }
        if ( ('\r'==data[i-3]) && ('\n'==data[i-2]) && ('\r'==data[i-1]) && ('\n'==data[i]) )
        {
            break;
        }
    }
    return NULL;
}

int string_append_CRLF(const BYTE* stringtocopy, BYTE* data,int size)
{
    if (stringtocopy == NULL || data == NULL || size == 0 )
        return 0;
    int lentocopy = strlen((char*)stringtocopy);
    if ( lentocopy == 0 )
        return 0;

    bool CRLFCRLF_found=false;

    BYTE* ptr=data;
    // rechercher le premier CRLF CRLF
    for (int i=0; i<size; i++)
    {
        if ( ('\r'==data[i-3]) && ('\n'==data[i-2]) && ('\r'==data[i-1]) && ('\n'==data[i]) )
        {
            CRLFCRLF_found = true;
            ptr = &data[i-1]; // keep first CRLF, replace at second CRLF
            memcpy(ptr,stringtocopy,lentocopy);
            ptr+=lentocopy;
            memcpy( ptr,"\r\n\r\n", 4);
            ptr+=4;
            mylog(5,"str_append_CRLF %04d bytes[%s]",ptr-data,stringtocopy);
            return ptr-data;
            break;
        }
    }
    if  (!CRLFCRLF_found) {
        ptr = &data[0]; // first field
        memcpy(ptr,stringtocopy,lentocopy);
        ptr+=lentocopy;
        memcpy( ptr,"\r\n\r\n", 4);
        ptr+=4;
        mylog(5,"str_append_CRLF %04d bytes[%s]",ptr-data,stringtocopy);
        return ptr-data;
    }
    return 0;
}
int get_line_before_CRLF(const char* sip_data, int sip_data_len, char* dest, int dest_len)
{
    //const char* ptr = sip_data;
    int len = 0;
    if (sip_data_len==0) return 0;
    if (dest_len==0) return 0;
    do
    {
        dest[len]=sip_data[len];
        dest[len+1]=0;
        len++;
        if ( ('\r'==sip_data[len-1]) && ('\n'==sip_data[len]) ) {
            dest[len-1]=0;
            return len-2;
        }

    }while ( ( len < sip_data_len ) && (len < dest_len) );
    return 0;
}

void bigIndian_byteReverse(unsigned char *buf, unsigned longs)
{
    uint32_t t;
    do {
        t = (uint32_t) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
                                                            ((unsigned) buf[1] << 8 | buf[0]);
        *(uint32_t *) buf = t;
        buf += 4;
    } while (--longs);
}

void mylog(unsigned char level,const char* fmt, ...)
{
   //return; //phd20161010
   if ( level > log_level ) return;
    static QMutex mut;
    if (mut.tryLock()){
        if ( 0==fmt ) return;
        if ( 0==fmt[0] ) return;
        if ( '\n'==fmt[0] ) return;

        struct timeval tv;
        gettimeofday (&tv, NULL);
        long milliseconds;
        milliseconds = tv.tv_usec / 1000;

        char time_string[40];
        struct tm* ptm = localtime (&tv.tv_sec);
        strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);

        va_list args;
        va_start(args, fmt);
        //nb = vsnprintf(temp, sizeof(temp) , fmt, args);
        char temp[1024]={0};
        vsprintf(temp, fmt, args);
        va_end (args);

        // oter tous les '\n' en fin de chaine
        while ( temp!=NULL && temp[0] && temp[strlen(temp)-1] == '\n' ) temp[strlen(temp)-1] = 0;

        char horodate[128];
        time_t t = time(NULL);
        strftime(horodate, sizeof(horodate), "%Y-%m-%d", localtime(&t));
        char filename[128];
        //sprintf(filename,"/var/log/recorder/log-%s-PID-%d-%s.log",progname,getpid(),horodate);
        sprintf(filename,"log-cstarec-PID-%d-%s.log",getpid(),horodate);

        FILE* fout=fopen(filename,"a+t") ;
        if ( fout) {
            //strftime(horodate, sizeof(horodate), "%Y-%m-%d %H:%M:%S", localtime(&t));
            strftime(horodate, sizeof(horodate), "%H:%M:%S", localtime(&t));
            fprintf(fout,"%s.%03ld %s\n",horodate,milliseconds,temp);
            fclose(fout);
            fprintf(stderr,"%s.%03ld %s\n",horodate,milliseconds,temp);
        }
        mut.unlock();
    }
}

unsigned char hex2bin(char c)
{
    char out_c=0;
    if      (c >= 'a' && c <= 'f')  out_c = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')  out_c = c - 'A' + 10;
    else if (c >= '0' && c <= '9')  out_c = c - '0';
    else {     fprintf(stderr,"hex2bin err %c\n",c);   exit(-1); }
    return (unsigned char)out_c;
}

void hexdump(unsigned char* buf, int len, const char* prefix=">")
{
    char horodate[128];
    time_t t = time(NULL);
    //strftime(horodate, sizeof(horodate), "%Y-%m-%d", localtime(&t));
    strftime(horodate, sizeof(horodate), "%H:%M:%S", localtime(&t));

    char prev_char='a';
    char* str = (char*)malloc(3*len+1);
    char* str_ascii = (char*)malloc(3*len+1);
    if ( str ){
        int str_len=0;
        int str_len_ascii=0;
        for( int k=0,m=0; k< len; k++,m++){
            if (isalnum(buf[k]) ||  ispunct(buf[k]) )
                str_len_ascii += sprintf(str_ascii+str_len_ascii, " %c ", buf[k]);
            else
                str_len_ascii += sprintf(str_ascii+str_len_ascii, "...");

            str_len += sprintf(str+str_len, "%02X-", buf[k]);
            if ( (m && ((m % 64)==0)) || ( prev_char=='\r' && buf[k]=='\n' ) ){
                //fprintf(stderr,"%s:[%s]\n",horodate,str);
                fprintf(stderr,"%s:[%s]\n",horodate,str_ascii);
                str_len=0;
                str_len_ascii=0;
                m=0;
            }
            prev_char = buf[k];
        }
        /*if (prefix!=NULL)
            fprintf(stderr,"%s[%s]",prefix, str);
        else
            fprintf(stderr,"[%s]",str);*/

        if (0){

            struct timeval tv;
            gettimeofday (&tv, NULL);
            long milliseconds;
            milliseconds = tv.tv_usec / 1000;

            char time_string[40];
            struct tm* ptm = localtime (&tv.tv_sec);
            strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);

            char horodate[128];
            time_t t = time(NULL);
            strftime(horodate, sizeof(horodate), "%Y-%m-%d", localtime(&t));
            char filename[128];
            //sprintf(filename,"/var/log/recorder/log-%s-PID-%d-%s.log",progname,getpid(),horodate);
            sprintf(filename,"log-cstarec-PID-%d-%s.log",getpid(),horodate);

            FILE* fout=fopen(filename,"a+t") ;
            if ( fout) {
                //strftime(horodate, sizeof(horodate), "%Y-%m-%d %H:%M:%S", localtime(&t));
                strftime(horodate, sizeof(horodate), "%H:%M:%S", localtime(&t));
                fprintf(fout,"%s.%03ld[%s]\n",horodate,milliseconds,str);
                fclose(fout);
            }
        }
        free (str);
    }
}

short Normalize(short* value)
{
    unsigned short msb, nextmsb;
    short numShiftBits;

    numShiftBits=0;
    msb= (*value & 0x8000) >> 15;
    nextmsb=(*value & 0x4000) >> 14;

    while (msb == nextmsb)
    {
        *value <<= 1;
        numShiftBits++;
        msb = (*value & 0x8000) >> 15;
        nextmsb = (*value & 0x4000) >> 14;
    } // end while
    return (numShiftBits);
}

BYTE Linear16ToAlaw(short originalSample)
{
    BYTE newSample;
    unsigned short numberOfShiftBits;
    unsigned short X, YYY, ZZZZ;

    if (originalSample==-32768)
        newSample=0x2A;
    else
    {
        X=(originalSample & 0x8000)>>8;

        if (originalSample & 0x8000)
            originalSample=-originalSample;

        if (originalSample<0x0100)
        {
            ZZZZ=(originalSample>>4) & 0x000f;
            YYY=0x0000;
        }
        else
        {
            numberOfShiftBits=Normalize(&originalSample);
            ZZZZ=(originalSample & 0x3c00)>>10;
            YYY=(7-numberOfShiftBits)<<4;
        }

        newSample=X|YYY|ZZZZ;
        newSample^=0xD5;
    }
    return (newSample) ;
}

void set_invite_content_len_3digits(char* data,int sdp_len)
{
    // rechercher le debut du SDP (RFC2327)
    mylog(5,"set_invite_content_len_3digits len=%d",sdp_len);

    if ( sdp_len <= 999)
    {
        char* ptr_orig = strstr(data , "Content-Length: ");
        char* ptr = strstr(data , "Content-Length:");
        mylog(5,"AVANT ptr=[%s]",ptr_orig);
        ptr += strlen("Content-Length: ");        //ptr += 1;

        int centaines = sdp_len/100;
        sdp_len = sdp_len - (centaines*100);
        int dixaines = sdp_len / 10;
        sdp_len = sdp_len - (dixaines*10);
        ptr[0] = '0'+centaines;
        ptr[1] = '0'+dixaines;
        ptr[2] = '0'+sdp_len;

        mylog(5,"APRES ptr=[%s]",ptr_orig);
    }
}

int first_CRLFCRLF_len(char* data,int size)
{
    // rechercher le premier debut du SDP (RFC2327)
    for (int i=3; i<size; i++)
    {
        if ( ('\r'==data[i-3]) && ('\n'==data[i-2]) && ('\r'==data[i-1]) && ('\n'==data[i]) )
        {
            return i+1;
        }
    }
    return 0;
}

int first_CRLF_len(char* data,int size)
{
    // rechercher le premier debut du SDP (RFC2327)
    for (int i=1; i<size; i++)
    {
        if (  ('\r'==data[i-1]) && ('\n'==data[i]) )
        {
            return i-1;
        }
    }
    return 0;
}

void set_invite_content_len(char* data,int len)
{
    // rechercher le debut du SDP (RFC2327)
    mylog(5,"rechercher le debut du SDP (RFC2327) len=%d",len);
    int i_sdp=0;
    int sdp_len = 0;
    for (int i=3; i<len; i++)    {
        if ( ('\r'==data[i-3]) && ('\n'==data[i-2]) && ('\r'==data[i-1]) && ('\n'==data[i]) )
        {
            i_sdp = i;
            mylog(5,"i_sdp = %d",i_sdp);
            break;
        }
    }
    int i_end = i_sdp ;
    for (int i=i_sdp+1; i<len; i++)    {
        if ( ('\r'==data[i-3]) && ('\n'==data[i-2]) && ('\r'==data[i-1]) && ('\n'==data[i]) )
        {
            mylog(5,"i_end = %d",i_end);
            i_end = i;
            break;
        }

    }
    sdp_len = i_end - i_sdp - 2;
    mylog(5,"sdp_len = %d",sdp_len);

    if ( sdp_len <= 999)
    {
        char* ptr_orig = strstr(data , "Content-Length:");
        char* ptr = strstr(data , "Content-Length:");
        mylog(5,"AVANT ptr=[%s]",ptr_orig);
        ptr += strlen("Content-Length:");
        ptr += 1;

        int centaines = sdp_len/100;
        sdp_len = sdp_len - (centaines*100);
        int dixaines = sdp_len / 10;
        sdp_len = sdp_len - (dixaines*10);
        ptr[0] = '0'+centaines;
        ptr[1] = '0'+dixaines;
        ptr[2] = '0'+sdp_len;

        mylog(5,"APRES ptr=[%s]",ptr_orig);
    }
}

QString ascii_only(QString& source)
{
    QString result;
    for (int i=0; i<source.length(); i++)
    {
        if ( ('0'<= source[i] && source[i]<='9') || ('a'<= source[i] && source[i]<='z') || ('A'<= source[i] && source[i]<='Z')  )
        {
            result+=source[i];
        }else result+="_";
    }
    return result;
}

/*uint32_t parseIPV4string(char* ipAddress)
{
    //An IP address, e.g. 192.168.0.1 can be written as an integer easily by writing it in hex which becomes 0xC0 0xA8 0x00 0x01 or just 0xC0A8000
    char ipbytes[4];
    sscanf(ipAddress, "%d.%d.%d.%d", &ipbytes[3], &ipbytes[2], &ipbytes[1], &ipbytes[0]);
    return ipbytes[0] | ipbytes[1] << 8 | ipbytes[2] << 16 | ipbytes[3] << 24;
}*/

void showhex(QByteArray ba, QString& resHex,QString& resDec)
{
    //A QByteArray is "0x10, 0x00, 0xFF"
    //resHex = "1000FF"
    //resDec = "16, 0, 255"
    for (int i = 0; i < ba.size(); i++) {

        resHex.append( QString::number(ba.at(i), 16).rightJustified(2, '0') );

        if ( !resDec.isEmpty() ) resDec.append( ", " );
        resDec.append( QString::number(ba.at(i) ));
    }
}

QString filter_ascii_letters_digits_only(QByteArray ba)
{
    QString result;
    for (int i = 0; i < ba.size(); i++) {
        char car = ba.constData()[i];
        if ( ('0'<= car && car<='9') || ('a'<= car && car<='z') || ('A'<= car && car<='Z')  )
                result+=car;
    }
    return result;
}

QString filter_ascii_digits_only(QByteArray ba)
{
    QString result;
    for (int i = 0; i < ba.size(); i++) {
        char car = ba.constData()[i];
        if ( ('0'<= car && car<='9')  )
                result+=car;
    }
    return result;
}

//-------------------------------------------------------------------------
// ==> UINT AlawToLinear16(BYTE originalSample)
//-------------------------------------------------------------------------
unsigned short AlawToLinear16(BYTE originalSample)
{
    unsigned short newSample;
    unsigned short X,YYY;
    BYTE tempValue;

    tempValue=originalSample^0xD5;
    X=(tempValue & 0x0080)>>7;

    newSample=((tempValue & 0x000f)<<1)|0x0001;

    YYY=(tempValue & 0x0070)>>4;
    if ((YYY-1)==0)
        newSample|=0x0020;
      else if ((YYY-1)>0)
        {
        newSample|=0x0020;
        newSample<<=(YYY-1);
        }
    newSample<<=3;
    if (X)
        newSample=-newSample;

    return newSample;
}

/* Read 1 character without echo */
char getch(void){
  return getchar();
}

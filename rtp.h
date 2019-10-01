#ifndef RTP_H
#define RTP_H

#define MARKER_FALSE			0x00
#define MARKER_TRUE				0x80
// packet type
#define PT_MASK					0x7F // 01111111b
#define PT_G711_PCMA			0x08
#define PT_GSM_610				0x03
#define VERSION_MASK			0xC0 // 11000000b
#define RTP_VERSION_2			0x02

#define OFFSET_RTP_IN_UDP_HEADER    12
#define RTP_PACKET_BUNCH            50 /* x20 milliseconds */
#define RTP_PACKET_MILLISECONDS     20  /*     milliseconds */

class CRtpMessage
{
public:
    CRtpMessage(BYTE* data,unsigned int dataLen)
    {
        //_buf.open(QBuffer::::Write);
        //_buf.setData((const char*)data,dataLen);
        if ( dataLen>0 ){
            _pBufLen = dataLen;
            _pBuf = (BYTE*)malloc(dataLen);
            if ( _pBuf ){
                memcpy(_pBuf ,data, dataLen);
            }
        }
    }
    ~CRtpMessage(){
        //qDebug() << "DTOR CRtpMessage";
        if ( _pBuf)  free(_pBuf);
    }
    bool parse();

    int     _rtp_port;
    int     _codec;

    BYTE*   _pBuf;
    unsigned int _pBufLen;
};

class CVoicePacket
{
public:
    QList<CRtpMessage*> _queueRTP;
    CVoicePacket(){}
    ~CVoicePacket(){}
    bool copySound(QList<CRtpMessage*> *container)
    {
        foreach (CRtpMessage* pm, *container) {
            _queueRTP.append(pm);
        }
        return true;
    }
};




#endif // RTP_H

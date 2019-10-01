#ifndef SIP_CPP
#define SIP_CPP

#include "common.h"
#include "sip.h"
#include "utils.h"
#include "md5.h"
#include "rtp.h"
#include "sip.h"

void CSipMessage::setInvite(QString str_phone_number_called,QString sip_proxy_address)
{
    QString strinvite = QString("INVITE sip:%1@%2:5060 SIP/2.0").arg(str_phone_number_called).arg(sip_proxy_address);
    qDebug() << strinvite;
    _sipFieldList.append(strinvite);
}

void CSipMessage::setRegister(QString sip_proxy_address)
{
    QString strregister = QString("REGISTER sip:%1 SIP/2.0").arg(sip_proxy_address);
    _sipFieldList.append(strregister);
}

void CSipMessage::setMethod(QString str_method,QString str_phone_number_called,QString sip_proxy_address)
{
    QString strmethod = QString("%3 sip:%1@%2:5060 SIP/2.0").arg(str_phone_number_called).arg(sip_proxy_address).arg(str_method);
    _sipFieldList.append(strmethod);
}


void CSipMessage::setBye(QString str_phone_number,QString sip_proxy_address)
{   //BYE sip:9999@192.168.0.230:5060 SIP/2.0
    QString strbye= QString("BYE sip:%1@%2:5060 SIP/2.0").arg(str_phone_number).arg(sip_proxy_address);
    _sipFieldList.append(strbye);
}

void CSipMessage::setSDP_audio(quint16 audio_rtp_port)
{
    QString str_audio = QString("m=audio %1 RTP/AVP").arg(audio_rtp_port);
    _sdpFieldList.append(str_audio);
}

void CSipMessage::setSDP_c(QString str_ip_caller)
{
    QString str_c = QString("c=IN IP4 %1").arg(str_ip_caller);
    _sdpFieldList.append(str_c);
}

void CSipMessage::setSDP_o(QString str_ip_caller)
{
    QString str_o = QString("o=- 3342338646 3342338646 IN IP4 %1").arg(str_ip_caller);
    _sdpFieldList.append(str_o);
}

void CSipMessage::setVia(QString str_ip_caller,QString via_tag_branch)
{
    //"Via: SIP/2.0/UDP "IP_CALLER";rport;branch="VIA_TAG_BRANCH1"\r\n"
    //"Via: SIP/2.0/UDP "IP_VIA":5060;rport;branch="VIA_TAG_BRANCH1"\r\n"
    QString strvia = QString("Via: SIP/2.0/UDP %1:5060;rport;branch=%2").arg(str_ip_caller).arg(via_tag_branch);
    _sipFieldList.append(strvia);
}

void CSipMessage::setAck(QString called_number,QString sip_proxy_address)
{   //"ACK sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR" SIP/2.0\r\n"
    QString strmethod = QString("ACK sip:%1@%2 SIP/2.0").arg(called_number).arg(sip_proxy_address);
    _sipFieldList.append(strmethod);
}

void CSipMessage::setCallId(QString call_id)
{   //Call-ID: "UA_CALLID1"\r\n"
    QString strcid = QString("Call-ID: %1").arg(call_id);
    _sipFieldList.append(strcid);
}

void CSipMessage::setCSeq(int cseq, QString method)
{   //"CSeq: "CSEQ_INVITE2" ACK\r\n"
    QString strcseq = QString("CSeq: %1 %2").arg(cseq).arg(method);
    _sipFieldList.append(strcseq);
}

void CSipMessage::setContact(QString phone_number,QString sip_proxy_address)
{   //Contact: <sip:9999@29.11.0.222:5060>
    QString strcontact = QString("Contact: <sip:%1@%2:5060>").arg(phone_number).arg(sip_proxy_address);
    _sipFieldList.append(strcontact);
}


void CSipMessage::setFrom(QString phone_name, QString phone_number,QString sip_proxy_address,QString from_tag)
{
    QString strfrom;
    if (from_tag.length())
        strfrom = QString("From: %1<sip:%2@%3>;tag=%4").arg(phone_name).arg(phone_number).arg(sip_proxy_address).arg(from_tag);
    else
        strfrom = QString("From: %1<sip:%2@%3>").arg(phone_name).arg(phone_number).arg(sip_proxy_address);
    _sipFieldList.append(strfrom);
}

void CSipMessage::setTo(QString phone_name, QString phone_number,QString sip_proxy_address,QString to_tag)
{   //int len = sprintf((char*)stringtocopy, "To:"UA_NUMBER_CALLED" <sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR">;tag=%s" , to_tag );
    QString strto;
    if ( to_tag.length())
        strto = QString("To: %1<sip:%2@%3>;tag=%4").arg(phone_name).arg(phone_number).arg(sip_proxy_address).arg(to_tag);
    else
        strto = QString("To: %1<sip:%2@%3>").arg(phone_name).arg(phone_number).arg(sip_proxy_address);
    _sipFieldList.append(strto);
}

void CSipMessage::prepareBuffer()
{
    _preparedLen = 0;
    if ( _pBuf!=NULL && _pBufLen>0 )    {
        for (int i=0; i < _sipFieldList.size(); i++)
        {
            QByteArray ba = _sipFieldList[i].toLocal8Bit();
            _preparedLen = str_append_CRLF((const BYTE*)ba.constData());//, _pBuf, _pBufLen );
        }

        _preparedLen = str_append_CRLF((const BYTE*)"Max-Forwards: 70");//, _pBuf, _pBufLen );
        _preparedLen = str_append_CRLF((const BYTE*)"Content-Length=0");//, _pBuf, _pBufLen );
    }
}


int CSipMessage::str_append_CRLF(const BYTE* stringtocopy)
{
    BYTE* data = _pBuf;
    int size = _pBufLen;

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
            mylog(5,"str_append_CRLF %d bytes           [%s]",ptr-data,stringtocopy);
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
            mylog(5,"str_append_CRLF %d bytes           [%s]",ptr-data,stringtocopy);
        return ptr-data;
    }
    return 0;
}

int CSipMessage::str_append_CRLF_SDP(const BYTE* stringtocopy)
{
    BYTE* data = _pBufSDP;
    int size = _pBufLenSDP;

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
            mylog(5,"str_append_CRLF %d bytes[%s]",ptr-data,stringtocopy);
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
        mylog(5,"str_append_CRLF %d bytes[%s]",ptr-data,stringtocopy);
        return ptr-data;
    }
    return 0;
}

void CSipMessage::prepareBufferWithSDP()
{
    _preparedLen = 0;
    _preparedLenSDP = 0;
    if ( _pBuf!=NULL && _pBufLen>0 )    {

        for (int i=0; i < _sdpFieldList.size(); i++)
        {
            QByteArray ba = _sdpFieldList[i].toLocal8Bit();
            _preparedLenSDP = str_append_CRLF_SDP((const BYTE*)ba.constData());//, _pBufSDP, _pBufLenSDP );
        }
        for (int i=0; i < _sipFieldList.size(); i++)
        {
            QByteArray ba = _sipFieldList[i].toLocal8Bit();
            _preparedLen = str_append_CRLF((const BYTE*)ba.constData() );//, _pBuf, _pBufLen );
        }
        _preparedLen = str_append_CRLF((const BYTE*)"Max-Forwards: 70");//, _pBuf, _pBufLen );
        if ( _preparedLenSDP > 0 ){
            _preparedLen = str_append_CRLF((const BYTE*)"Content-Type: application/sdp");//, _pBuf, _pBufLen );
            _preparedLen = str_append_CRLF((const BYTE*)QString("Content-Length=%1").arg(_preparedLenSDP).toLatin1().constData());//, _pBuf, _pBufLen );
        }else{
            _preparedLen = str_append_CRLF((const BYTE*)QString("Content-Length=0").toLatin1().constData());
        }
    }
}


#endif // SIP_CPP

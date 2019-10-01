#include <common.h>
#include <csta.h>
#include <csta_const.h>
#include <utils.h>

extern QAsyncQueue<CVoicePacket*> g_sound_msg_queue_in;
extern QAsyncQueue<CVoicePacket*> g_sound_msg_queue_out;

//extern CUserAgent _myUserAgent;
/*unsigned short CCSTACommand::invokeid = 10;
QMap<QString,QByteArray>   _crossrefByextension;
QMap<unsigned short,QString>     _extensionByInvokeid;*/

//static BYTE invokeid = 5;
QMap<QByteArray,QByteArray>   CCSTACommand::_crossrefByextension;
QMap<BYTE,QByteArray>     CCSTACommand::_extensionByInvokeid;
BYTE CCSTACommand::invokeid = 5;

int CCSTACommand::startMonitorExtension(QString extension)
{/*(1)xa000000> tail -f /tmpd/logfile.2-10.253.253.5
Host< = 16
a1 0e 02 01 01 02 01 47 30 06 80 04 33 30 30 31
aPDU-rose
invoke
             { -- SEQUENCE --
                 invokeID
                     1,
                 operationValue
                     71,
                 argument
                     { -- SEQUENCE --
                         monitorObject
                            device
                                dialingNumber
                                     '33303031'H  -- "3001" --
                     }
             } */
    mylog(5,"CCSTACommand::monitorExtension...");

    BYTE* debut = _internal_buf;
    BYTE* ptr = debut;
    *ptr++ = 0;
    BYTE* ptr_len1 = ptr;   ptr++;//*ptr++ = 24+3; // tcp len following

    *ptr++ = 0xA1; // REQUEST PDU
    BYTE* ptr_len2 = ptr ; ptr++; //*ptr++ = 22+3; // len following

    qDebug() << "monitorExtension invokeid=" << invokeid;
    qDebug() << "monitorExtension invokeid=" << invokeid;
    qDebug() << "monitorExtension invokeid=" << invokeid;

    _extensionByInvokeid[invokeid] = extension.toLocal8Bit();

    // invokeID INTEGER: 0x0001
    *ptr++ = INTEGER_TAG;
    *ptr++ = 1; // integer len
    *ptr++ = invokeid++;

    // serviceID INTEGER : operationValue
    *ptr++ = INTEGER_TAG;
    *ptr++ = 1; // integer len
    *ptr++ = MONITORSTARTSID; // decimal 71 = 0x47

    // monitor start serviceArgs
    // callingDevice DeviceID
    /*DeviceID ::= CHOICE
            {dialingNumber               [0] IMPLICIT      NumberDigits,
            deviceNumber                [1] IMPLICIT      DeviceNumber}*/

    *ptr++ = 0x30; // SEQUENCE tag
    BYTE* ptr_len3 = ptr; ptr++;//*ptr++ = 0x06  ;// length following

    //dialingNumber
    /*NumberDigits             ::= IA5String -- Matracom specs -- (SIZE(0..20))*/
    *ptr++ = 0x80; *ptr++ = extension.length();
    for (int i=0; i<extension.length() ; i++){
        *ptr++ = extension[i].toAscii();
        //*ptr++ = '1'; *ptr++ = '9';*ptr++ = '3';*ptr++ = '0' ;  // ascii string
        //*ptr++ = digit1; *ptr++ = digit2;*ptr++ = digit3;*ptr++ = digit4;  // ascii string
    }

    _internal_buf_size = ptr - debut;
    ptr--;
    /*mylog(5,"_internal_buf_size = %d",_internal_buf_size);
    mylog(5,"ptr-debut = %d",ptr - debut);
    mylog(5,"ptr-ptr_len3 = %d",ptr-ptr_len3);
    mylog(5,"ptr-ptr_len2 = %d",ptr-ptr_len2);
    mylog(5,"ptr-ptr_len1 = %d",ptr-ptr_len1);*/
    *ptr_len3 = (ptr-ptr_len3);
    *ptr_len2 = (ptr-ptr_len2);
    *ptr_len1 = (ptr-ptr_len1);
    return _internal_buf_size ;
    /**ptr_len2 = ptr - debut - 2 - 2;    *ptr_len1 = ptr - debut - 2 ;    return ptr - orig_debut ;*/
}

#if 0
int CCSTACommand::startMonitorExtension(char digit1,char digit2,char digit3,char digit4)
{/*(1)xa000000> tail -f /tmpd/logfile.2-10.253.253.5
Host< = 16
a1 0e 02 01 01 02 01 47 30 06 80 04 33 30 30 31
aPDU-rose
invoke
             { -- SEQUENCE --
                 invokeID
                     1,
                 operationValue
                     71,
                 argument
                     { -- SEQUENCE --
                         monitorObject
                            device
                                dialingNumber
                                     '33303031'H  -- "3001" --
                     }
             } */
    mylog(5,"CCSTACommand::monitorExtension...");

    BYTE* debut = _internal_buf;
    BYTE* ptr = debut;
    *ptr++ = 0;
    BYTE* ptr_len1 = ptr;   ptr++;//*ptr++ = 24+3; // tcp len following

    *ptr++ = 0xA1; // REQUEST PDU
    BYTE* ptr_len2 = ptr ; ptr++; //*ptr++ = 22+3; // len following

    // invokeID INTEGER: 0x0001
    *ptr++ = INTEGER_TAG;
    *ptr++ = 1; // integer len
    *ptr++ = invokeid++;

    // serviceID INTEGER : operationValue
    *ptr++ = INTEGER_TAG;
    *ptr++ = 1; // integer len
    *ptr++ = MONITORSTARTSID; // decimal 71 = 0x47

    // monitor start serviceArgs
    // callingDevice DeviceID
    /*DeviceID ::= CHOICE
            {dialingNumber               [0] IMPLICIT      NumberDigits,
            deviceNumber                [1] IMPLICIT      DeviceNumber}*/

    *ptr++ = 0x30; // SEQUENCE tag
    BYTE* ptr_len3 = ptr; ptr++;//*ptr++ = 0x06  ;// length following

    //dialingNumber
    /*NumberDigits             ::= IA5String -- Matracom specs -- (SIZE(0..20))*/
    *ptr++ = 0x80; *ptr++ = 8;
    *ptr++ = '1'; *ptr++ = '9';*ptr++ = '3';*ptr++ = '0' ;  // ascii string
    *ptr++ = digit1; *ptr++ = digit2;*ptr++ = digit3;*ptr++ = digit4;  // ascii string

    _internal_buf_size = ptr - debut;
    ptr--;
    /*mylog(5,"_internal_buf_size = %d",_internal_buf_size);
    mylog(5,"ptr-debut = %d",ptr - debut);
    mylog(5,"ptr-ptr_len3 = %d",ptr-ptr_len3);
    mylog(5,"ptr-ptr_len2 = %d",ptr-ptr_len2);
    mylog(5,"ptr-ptr_len1 = %d",ptr-ptr_len1);*/
    *ptr_len3 = (ptr-ptr_len3);
    *ptr_len2 = (ptr-ptr_len2);
    *ptr_len1 = (ptr-ptr_len1);
    return _internal_buf_size ;
    /**ptr_len2 = ptr - debut - 2 - 2;    *ptr_len1 = ptr - debut - 2 ;    return ptr - orig_debut ;*/
}
#endif

int CCSTACommand::stopMonitorExtension(QString extension)//char crossrefid1,char crossrefid2,char crossrefid3,char crossrefid4)
{
    QByteArray crossrefid;

    qDebug()<< extension.toLocal8Bit();

    // affectation precedente : _crossrefByextension[extension] = recv_crossRefId.toHex();
    QMap<QByteArray, QByteArray>::const_iterator i = _crossrefByextension.find(extension.toLocal8Bit());
    if ( i != _crossrefByextension.end() )
    {
        QByteArray crossrefid_hex =  i.value();
        qDebug()<< crossrefid_hex;

        crossrefid = QByteArray::fromHex( crossrefid_hex );
        qDebug()<< extension;

        if ( crossrefid.length() == 4 )
        {
            _crossrefByextension[extension.toLocal8Bit()] = QByteArray::fromHex("0");

            char crossrefid1 = crossrefid.at(0);//.constData()[0].toAscii();
            qDebug()<< crossrefid1;
            char crossrefid2 = crossrefid.at(1);//constData()[1].toAscii();
            qDebug()<< crossrefid2;
            char crossrefid3 = crossrefid.at(2);//constData()[2].toAscii();
            qDebug()<< crossrefid3;
            char crossrefid4 = crossrefid.at(3);//constData()[3].toAscii();
            qDebug()<< crossrefid4;


            /*Host< = 14
a1 0c 02 01 01 02 01 49 55 04 01 42 f9 00

Host> = 12
a2 0a 02 01 01 30 05 02 01 49 05 00


--Service : SER_SYSTEM_STATUS (rose-id=2097 host=0)
--| 	cstaSearCrid=0=0x0
--| 	cstaSearRoid=2097=0x831
--| 	Connection 1 : (call=0,neqt=0,type=TYPE_NOT_PROVIDED)
--| 	Connection 2 : (call=0,neqt=0,type=TYPE_NOT_PROVIDED)
--| 	        	(L)Device 1 : ABSENT
--| 	        	(L)Device 2 : ABSENT
--| 	cstaSearFilt=0x00000000 0x00000000
--| 	Feature : ident=0 invoke=0 val1=0 val2=0
InComming:
ENTREE:
aPDU-rose
invoke
             { -- SEQUENCE --
                 invokeID
                     1,
                 operationValue
                     73,
                 argument
crossRefIdentifier
                         '0142f900'H  -- ".B.." --
             }


--Invoke: SER_MONITOR_STOP (rose-id=1 host=0)
--| 	cstaSearCrid=21166336=0x142f900
--| 	cstaSearRoid=1=0x1
--| 	Connection 1 : (call=0,neqt=0,type=TYPE_NOT_PROVIDED)
--| 	Connection 2 : (call=0,neqt=0,type=TYPE_NOT_PROVIDED)
--| 	        	(L)Device 1 : ABSENT
--| 	        	(L)Device 2 : ABSENT
--| 	cstaSearFilt=0x00000000 0x00000000
--| 	Feature : ident=0 invoke=0 val1=0 val2=0
Outgoing:
ENTREE:
aPDU-rose
retResult
             { -- SEQUENCE --
                 invokeID
                     1,

                     { -- SEQUENCE --
                         operationValue
                             73,
                         result
noData
                                 NULL
                     }
             }


--Service : SER_MONITOR_STOP (rose-id=1 host=0)
--| 	cstaSearCrid=0=0x0
--| 	cstaSearRoid=1=0x1
--| 	Connection 1 : (call=0,neqt=0,type=TYPE_NOT_PROVIDED)
--| 	Connection 2 : (call=0,neqt=0,type=TYPE_NOT_PROVIDED)
--| 	        	(L)Device 1 : ABSENT
--| 	        	(L)Device 2 : ABSENT
--| 	cstaSearFilt=0x00000000 0x00000000
--| 	Feature : ident=0 invoke=0 val1=0 val2=0
(1)xa000000>
*/
            mylog(5,"CCSTACommand::stopMonitorExtension...");
            BYTE* debut = _internal_buf;
            BYTE* ptr = debut;
            *ptr++ = 0;
            BYTE* ptr_len1 = ptr;   ptr++;//*ptr++ = 24+3; // tcp len following

            *ptr++ = 0xA1; // REQUEST PDU
            BYTE* ptr_len2 = ptr ; ptr++; //*ptr++ = 22+3; // len following

            // invokeID INTEGER: 0x0001
            *ptr++ = INTEGER_TAG;
            *ptr++ = 1; // integer len
            *ptr++ = invokeid++;//   *ptr++ = 1;//invokeid_supervise++;

            // serviceID INTEGER : operationValue
            *ptr++ = INTEGER_TAG;
            *ptr++ = 1; // integer len
            *ptr++ = MONITORSTOPSID; // decimal 73 = 0x49

            //crossref ID
            *ptr++ = 0x55; *ptr++ = 4; *ptr++ = crossrefid1; *ptr++ = crossrefid2;*ptr++ = crossrefid3;*ptr++ = crossrefid4;  // ascii string

            _internal_buf_size = ptr - debut;
            ptr--;
            /*mylog(5,"_internal_buf_size = %d",_internal_buf_size);
    mylog(5,"ptr-debut = %d",ptr - debut);
    //mylog(5,"ptr-ptr_len3 = %d",ptr-ptr_len3);
    mylog(5,"ptr-ptr_len2 = %d",ptr-ptr_len2);
    mylog(5,"ptr-ptr_len1 = %d",ptr-ptr_len1);*/
            //*ptr_len3 = (ptr-ptr_len3);
            *ptr_len2 = (ptr-ptr_len2);
            *ptr_len1 = (ptr-ptr_len1);
            return _internal_buf_size ;
            /**ptr_len2 = ptr - debut - 2 - 2;    *ptr_len1 = ptr - debut - 2 ;    return ptr - orig_debut ;*/
        }
    }
}


void CCSTACommand::parseCstaMonitorStartResponse()
{
    //00 2c a2 2a 02 01 01 30 25 02 01 47 30 20 55 04 01 42
    //f9 00 30 18 80 03 02 80 00 81 02 00 7f 82 02 02
    //00 83 02 06 00 85 02 01 6e 84 01 00
    mylog(5,"CCSTACommand::parseCstaMonitorStartResponse...finding crossrefid...");

    int tagV=-1;
    int tagL=-1;
    BYTE* p1 = TLVReadTag(_internal_buf+4,&tagV,&tagL);     // 02: tag type integer
    BYTE* p2 = TLVReadLength(p1,&tagV,&tagL);               // 01: tag len = 1
    BYTE* p3 = TLVReadValue(p2,tagL);                // 01 : tag value : invokeid
    BYTE recv_invokeID = _TLVValue[0];
    qDebug() << "parseCstaMonitorStartResponse _invokeID=" << recv_invokeID;
    qDebug() << "parseCstaMonitorStartResponse _invokeID=" << recv_invokeID;
    qDebug() << "parseCstaMonitorStartResponse _invokeID=" << recv_invokeID;

    BYTE* p4 = TLVReadTag(p3,&tagV,&tagL);                // 30 : tag SEQUENCE
    BYTE* p5 = TLVReadLength(p4,&tagV,&tagL);                // 25 : len SEQUENCE

    BYTE* p6 = TLVReadTag(p5,&tagV,&tagL);                  // 02 : tag type integer
    BYTE* p7 = TLVReadLength(p6,&tagV,&tagL);               // 01 : tag len = 1
    BYTE* p8 = TLVReadValue(p7,tagL);                // 47 : tag value : MONITORSTATSID

    //_operationValue = (CSTAServices)_TLVValue.at(0);
    //_operationValue = _TLVValue.at(0);
    if ( tagL == 1 &&  _TLVValue[0]== MONITORSTARTSID ){
        BYTE* p9 = TLVReadTag(p8,&tagV,&tagL);                // 30 : tag SEQUENCE
        BYTE* p10 = TLVReadLength(p9,&tagV,&tagL);                // 25 : len SEQUENCE

        int crossideref_value =-1;
        BYTE* p11 = TLVReadTag(p10,&crossideref_value,&tagL);                  // 0x55 : tag type ???
        int _crossRefIdLen =-1;
        BYTE* p12 = TLVReadLength(p11,&_crossRefIdLen, &tagL );               // 0x04 : tag len = 4

        BYTE* p13 = TLVReadValue(p12,_crossRefIdLen);                // 0142f900 : tag value : CROSSREFID
        QByteArray recv_crossRefId = _TLVValue;
        mylog(5,"** MONITORSTARTSID _crossRefIdLen=%d **",_crossRefIdLen);
        qDebug() << "recv_crossRefId (hex) =" << recv_crossRefId.toHex();
        //qDebug()<< _TLVValue;

        //qDebug() << recv_crossRefId;
        QMap<BYTE, QByteArray>::const_iterator i = _extensionByInvokeid.find(recv_invokeID);
        if ( i != _extensionByInvokeid.end() )
        {
            QByteArray extension = i.value();
            qDebug()<< "FOUND extension:" << extension;
            _crossrefByextension[extension] = recv_crossRefId.toHex();
            qDebug()<< "FOUND recv_crossRefId:" << recv_crossRefId.toHex();
        }

        /*QByteArray extension = _extensionByInvokeid[recv_invokeID];
        qDebug()<< extension;
        _crossrefByextension[extension] = recv_crossRefId.toHex();
        qDebug()<< _crossrefByextension[extension];
        if ( _crossrefByextension.contains(extension.toLocal8Bit()) ){}*/
    }
}

void CCSTACommand::parseCstaReportEvent()
{   /* Host< = 16 start monitor request 3001
    a1 0e 02 01 01 02 01 47 30 06 80 04 33 30 30 31

    Host> = 44 monitor started response (crossrefid=0142f900)
    a2 2a 02 01 01 30 25 02 01 47 30 20 55 04 01 42
    f9 00 30 18 80 03 02 80 00 81 02 00 7f 82 02 02
    00 83 02 06 00 85 02 01 6e 84 01 00

    delivered event request A0 A3 (crossrefid=0142f900)
    CCSTACommand [00-83-A1-81-80-02-02-4C-9C-02-01-15-30-77-55-04-01-42-F9-00-
    A0-32-A3-30-30-2E-6B-0A-82-02-00-58-83-04-10-96-01-00-63-06-84-04-33-30-30-31-61-06-84-04-33-33-33-31-62-06-84-04-33-30-30-31-64-02-88-00-4E-01-02-0A-01-16-7E-3B-A0-0F-17-0D-31-35-31-31-32-39-32-32-34-35-33-39-5A-A1-28-30-12-06-06-2B-0C-89-36-84-00-30-08-80-04-61-62-63-64-81-00-30-12-06-06-2B-0C-89-36-84-09-04-08-03-72-5B-56-58-00-01-00-]

    Host> = 131 delivered
    a1 81 80 02 02 4c 9c 02 01 15 30 77 55 04 01 42
    f9 00 a0 32 a3 30 30 2e 6b 0a 82 02 00 58 83 04
    10 96 01 00 63 06 84 04 33 30 30 31 61 06 84 04
    33 33 33 31 62 06 84 04 33 30 30 31 64 02 88 00
    4e 01 02 0a 01 16 7e 3b a0 0f 17 0d 31 35 31 31
    32 39 32 32 34 35 33 39 5a a1 28 30 12 06 06 2b
    0c 89 36 84 00 30 08 80 04 61 62 63 64 81 00 30
    12 06 06 2b 0c 89 36 84 09 04 08 03 72 5b 56 58
    00 01 00
    */
    mylog(5,"CCSTACommand::parseCstaReportEvent...");
    int tagV=-1;
    int tagL=-1;
    BYTE* p1 = TLVReadTag(_internal_buf+4,&tagV,&tagL);     // 02: tag type integer
    BYTE* p2 = TLVReadLength(p1,&tagV,&tagL);               // 02: tag len = 2
    BYTE* p3 = TLVReadValue(p2,tagV);                // 01 : tag value : invokeid 4C-9E
    _lastReportEventInvokeID = tagV;
    mylog(3,"parseCstaReportEvent _lastReportEventInvokeID=%d",_lastReportEventInvokeID);

    BYTE* p4 = TLVReadTag(p3,&tagV,&tagL);                  // 02: tag type integer
    BYTE* p5 = TLVReadLength(p4,&tagV,&tagL);               // 01: tag len = 1
    BYTE* p6 = TLVReadValue(p5,tagV);                       // 15: 0x15 = 21 CSTAEVENTREPORTSID
    //qDebug()<< _TLVValue;

    if ( tagL == 1 &&  _TLVValue[0]== CSTAEVENTREPORTSID ){
        BYTE* p7 = TLVReadTag(p6,&tagV,&tagL);                // 30 : tag SEQUENCE
        BYTE* p8 = TLVReadLength(p7,&tagV,&tagL);             // 63 : len SEQUENCE

        int crossideref_value =-1;
        BYTE* p9 = TLVReadTag(p8,&crossideref_value,&tagL);         // 0x55 : tag type ???

        _lastReportEventcrossRefIdLen =-1;
        BYTE* p10 = TLVReadLength(p9,&_lastReportEventcrossRefIdLen, &tagL );      // 0x04 : tag len = 4

        BYTE* p11 = TLVReadValue(p10,_lastReportEventcrossRefIdLen);               // 0142f900 : tag value : CROSSREFID
        _lastReportEventcrossRefId = _TLVValue;
        mylog(5,"** storing _lastReportEventcrossRefId: (%d bytes) = [%s] **",_lastReportEventcrossRefIdLen, _lastReportEventcrossRefId.toHex().constData());

        //qDebug()<< _TLVValue;

        BYTE* p12 = TLVReadTag(p11,&tagV,&tagL);                // A0 : tag SEQUENCE OF
        BYTE* p13 = TLVReadLength(p12,&tagV,&tagL);             // 32 : len SEQUENCE OF

        BYTE* p14 = TLVReadTag(p13,&tagV,&tagL);                // A5 : tag SEQUENCE OF + 5

        if ( tagL == 1 && tagV == (0xA2 & 0x1F) ) {
            qDebug()<< "CLEARED EVENT 0xA2 & 0x1F !!!";
            qDebug()<< "CLEARED EVENT 0xA2 & 0x1F !!!";
            qDebug()<< "CLEARED EVENT 0xA2 & 0x1F !!!";

            /*g_sound_msg_queue_out.clean();
            g_sound_msg_queue_in.clean();
            if ( _myUserAgent._rtp_playing_sound_thread._currentVoicePacketIn ) {
                mylog(2,"HANGUP _currentVoicePacketIn->_queueRTP %d packet",_myUserAgent._rtp_playing_sound_thread._currentVoicePacketIn->_queueRTP.count());
                delete _myUserAgent._rtp_playing_sound_thread._currentVoicePacketIn;
                _myUserAgent._rtp_playing_sound_thread._currentVoicePacketIn=NULL;
            }
            if ( _myUserAgent._rtp_playing_sound_thread._currentVoicePacketOut ) {
                mylog(2,"HANGUP _currentVoicePacketOut->_queueRTP %d packet",_myUserAgent._rtp_playing_sound_thread._currentVoicePacketOut->_queueRTP.count());
                delete _myUserAgent._rtp_playing_sound_thread._currentVoicePacketOut;
                _myUserAgent._rtp_playing_sound_thread._currentVoicePacketOut=NULL;
            }*/
        }
        else if ( tagL == 1 && tagV == (0xA5 & 0x1F) ) {
            qDebug()<< "ESTABLISHED EVENT 0xA5 & 0x1F !!!";
            qDebug()<< "ESTABLISHED EVENT 0xA5 & 0x1F !!!";
            qDebug()<< "ESTABLISHED EVENT 0xA5 & 0x1F !!!";

            BYTE* p15 = TLVReadLength(p14,&tagV,&tagL);             // 30 : len SEQUENCE


            BYTE* p16 = TLVReadTag(p15,&tagV,&tagL);                // 30 : tag SEQUENCE
            BYTE* p17 = TLVReadLength(p16,&tagV,&tagL);             // 2E : len SEQUENCE

            BYTE* p18 = TLVReadTag(p17,&tagV,&tagL);                // 6B : tag ???
            BYTE* p19 = TLVReadLength(p18,&tagV,&tagL);             // 01 : tag len
            BYTE* p20 = TLVReadValue(p19,tagV);                     // value :  82-02-00-58-83-04-10-96-01-00
            //qDebug()<< _TLVValue;

            BYTE* p21 = TLVReadTag(p20,&tagV,&tagL);                // 63 : tag ???
            BYTE* p22 = TLVReadLength(p21,&tagV,&tagL);             // 06 : tag len
            BYTE* p23 = TLVReadValue(p22,tagV);                     // value :  84-04-33-30-30-31 answering device
            //_answeringDevice = ascii_only(_TLVValue.constData());
            _answeringDevice = filter_ascii_digits_only(_TLVValue); //QString::fromLatin1(_TLVValue.data());
            qDebug()<< "_answeringDevice:" << _answeringDevice;

            BYTE* p24 = TLVReadTag(p23,&tagV,&tagL);                // 61 : tag ???
            BYTE* p25 = TLVReadLength(p24,&tagV,&tagL);             // 06 : tag len
            BYTE* p26 = TLVReadValue(p25,tagV);                     // value :  84-04-33-33-33-31 calling device
            //qDebug()<< _TLVValue;
            _callingDevice = filter_ascii_digits_only(_TLVValue);;//QString::fromLatin1(_TLVValue.data());
            qDebug()<< "_callingDevice:" << _callingDevice;
        }
    }

    /* established event request A0 A5 (crossrefid=0142f900)
CCSTACommand [00-6E-A1-6C-02-02-4C-9E-02-01-15-30-63-55-04-01-42-F9-00-
A0-32-A5-30-30-2E-6B-0A-82-02-00-58-83-04-10-96-01-00-63-06-84-04-33-30-30-31-61-06-84-04-33-33-33-31-62-06-84-04-33-30-30-31-64-02-88-00-4E-01-03-0A-01-16-7E-27-A0-0F-17-0D-31-35-31-31-32-39-32-32-34-35-34-35-5A-A1-14-30-12-06-06-2B-0C-89-36-84-09-04-08-03-72-5B-56-58-00-01-00-]

Host> = 110 established
a1 6c 02 02 4c 9e 02 01 15 30 63 55 04 01 42 f9
00 a0 32 a5 30 30 2e 6b 0a 82 02 00 58 83 04 10
96 01 00 63 06 84 04 33 30 30 31 61 06 84 04 33
33 33 31 62 06 84 04 33 30 30 31 64 02 88 00 4e
01 03 0a 01 16 7e 27 a0 0f 17 0d 31 35 31 31 32
39 32 32 34 35 34 35 5a a1 14 30 12 06 06 2b 0c
89 36 84 09 04 08 03 72 5b 56 58 00 01 00

Host> = 90 hangup
a1 58 02 02 4c 9f 02 01 15 30 4f 55 04 01 42 f9
00 a0 1e a2 1c 30 1a 6b 0a 82 02 00 58 83 04 10
96 01 00 63 06 84 04 33 30 30 31 4e 01 00 0a 01
30 7e 27 a0 0f 17 0d 31 35 31 31 32 39 32 32 34
35 34 39 5a a1 14 30 12 06 06 2b 0c 89 36 84 09
04 08 03 72 5b 56 58 00 01 00

hangup event request A0 A2 (crossrefid=0142f900)
CCSTACommand [00-5A-A1-58-02-02-4C-9F-02-01-15-30-4F-55-04-01-42-F9-00-
A0-1E-A2-1C-30-1A-6B-0A-82-02-00-58-83-04-10-96-01-00-63-06-84-04-33-30-30-31-4E-01-00-0A-01-30-7E-27-A0-0F-17-0D-31-35-31-31-32-39-32-32-34-35-34-39-5A-A1-14-30-12-06-06-2B-0C-89-36-84-09-04-08-03-72-5B-56-58-00-01-00-]
*/
}


void CCSTACommand::parseCstaMonitorStopResponse()
{
    mylog(5,"CCSTACommand::parseCstaMonitorStopEvent...");
}

int CCSTACommand::superviseResponse(BYTE invokeid1,BYTE invokeid2)
{/*
// recv 00 0c a1 0a 02 02 1f c8 02 01 34 0a 01 02
Host> = 12
a1 0a 02 02 2b ff 02 01 34 0a 01 02

// send 00 0d a2 0b 02 02 1f c8 30 05 02 01 34 05 00
Host< = 13
a2 0b 02 02 2b ff 30 05 01 02 34 05 00

Host> = 7
a4 05 05 00 80 01 00

Outgoing:
ENTREE:
aPDU-rose
invoke
             { -- SEQUENCE --
                 invokeID
                     11266,
                 operationValue
                     52,
                 argument
systemStatus
                         2
             }

SORTIE: longueur 13
retResult
         { -- SEQUENCE --
             invokeID
                 11266,

                 { -- SEQUENCE --
                     operationValue
                         52,
                     result
noData
                             NULL
                 }
         }*/
    mylog(5,"buildSuperviseResponse");
    BYTE* debut = _internal_buf;
    BYTE* ptr = debut;
    *ptr++ = 0;
    BYTE* ptr_len1 = ptr;   ptr++;//*ptr++ = 24+3; // tcp len following

    *ptr++ = 0xA2; // RESPONSE PDU
    BYTE* ptr_len2 = ptr ; ptr++; //*ptr++ = 22+3; // len following

    // invokeID INTEGER:
    *ptr++ = INTEGER_TAG;
    *ptr++ = 2; // integer len
    *ptr++ = invokeid1;
    *ptr++ = invokeid2;

    *ptr++ = 0x30; // SEQUENCE tag
    BYTE* ptr_len3 = ptr; ptr++;//*ptr++ = 0x06  ;// length following

    // serviceID INTEGER : operationValue
    *ptr++ = INTEGER_TAG;
    *ptr++ = 1; // integer len
    *ptr++ = SYSTEMSTATUSSID; // decimal 52 = 0x34

    *ptr++ = 0x05; *ptr++ = 0x00;

    _internal_buf_size = ptr - debut;
    ptr--;
    /*mylog(5,"_internal_buf_size = %d",_internal_buf_size);
    mylog(5,"ptr-debut = %d",ptr - debut);
    mylog(5,"ptr-ptr_len3 = %d",ptr-ptr_len3);
    mylog(5,"ptr-ptr_len2 = %d",ptr-ptr_len2);
    mylog(5,"ptr-ptr_len1 = %d",ptr-ptr_len1);*/
    *ptr_len3 = (ptr-ptr_len3);
    *ptr_len2 = (ptr-ptr_len2);
    *ptr_len1 = (ptr-ptr_len1);
    return _internal_buf_size ;
}

int CCSTACommand::stopIpRecording(QString extension,QString str_ipaddress)
{
    struct sockaddr_in sa;
    //char str[INET_ADDRSTRLEN];
    // store this IP address in sa:
    inet_pton(AF_INET, str_ipaddress.toLatin1().constData() /*"192.0.2.33"*/, &(sa.sin_addr));    // now get it back and print it
    //inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);    //printf("%s\n", str); // prints "192.0.2.33"
    uint32_t ip32 = (uint32_t)sa.sin_addr.s_addr;
    BYTE ip4 = ip32>>24 & 0xFF;
    BYTE ip3 = ip32>>16 & 0xFF;
    BYTE ip2 = ip32>>8 & 0xFF;
    BYTE ip1 = ip32>>0 & 0xFF;

    mylog(5,"CCSTACommand::stopIpRecording...");
    //mylog(5,"ip1=%02X",ip1);    mylog(5,"ip2=%02X",ip2);    mylog(5,"ip3=%02X",ip3);    mylog(5,"ip4=%02X",ip4);

    BYTE* debut = _internal_buf;// + 2;
    BYTE* ptr = debut;
    *ptr++ = 0;
    BYTE* ptr_len1 = ptr; ptr++;   //*ptr++ = 24+3; // tcp len following

    *ptr++ = 0xA1; // REQUEST PDU
    BYTE* ptr_len2 = ptr ; ptr++;//*ptr++ = 22+3; // len following

    // invokeID INTEGER: 0x0001
    *ptr++ = INTEGER_TAG;
    *ptr++ = 1; // integer len
    *ptr++ = invokeid++;

    // serviceID INTEGER operationValue=51
    *ptr++ = INTEGER_TAG;
    *ptr++ = 1; // integer len
    *ptr++ = 0x33; // decimal 51

    *ptr++ = 0x30; // sequence tag
    BYTE* ptr_len3 = ptr; ptr++;// length following

    //SEQUENCE OF
    *ptr++ = 0xa1; // sequence of tag
    BYTE* ptr_len4 = ptr; ptr++;// length following

    *ptr++ = 0x30; // sequence tag
    BYTE* ptr_len5 = ptr; ptr++;// length following

    //manufacturer 06 05 2b 0c 89 36 18
    *ptr++ = 6;*ptr++ = 5;*ptr++ = 0x2B;*ptr++ = 0x0c;*ptr++ = 0x89;*ptr++ = 0x36;*ptr++ = 0x18;

    //startIPRecordingMessageArgument
    //SEQUENCE
    *ptr++ = 0x30; // sequence tag
    BYTE* ptr_len6 = ptr; ptr++;// length following

    //recordedDevice dialingNumber
    //*ptr++ = 0x80; *ptr++=4; *ptr++=digit1; *ptr++=digit2; *ptr++=digit3; *ptr++=digit4;
    *ptr++ = 0x80; *ptr++ = extension.length();
    for (int i=0; i<extension.length() ; i++){
        *ptr++ = extension[i].toAscii();
        //*ptr++ = '1'; *ptr++ = '9';*ptr++ = '3';*ptr++ = '0' ;  // ascii string
        //*ptr++ = digit1; *ptr++ = digit2;*ptr++ = digit3;*ptr++ = digit4;  // ascii string
    }

    //    '33303031'H  -- "3001" --,
    //loggerIPAddress
    *ptr++ = 0x04;*ptr++ = 4;*ptr++=ip1; *ptr++=ip2; *ptr++=ip3; *ptr++=ip4;

    _internal_buf_size = ptr - debut;
    ptr--;
    /*mylog(5,"_internal_buf_size = %d",_internal_buf_size);
    mylog(5,"ptr-debut = %d",ptr - debut);
    mylog(5,"ptr-ptr_len6 = %d",ptr-ptr_len6);
    mylog(5,"ptr-ptr_len5 = %d",ptr-ptr_len5);
    mylog(5,"ptr-ptr_len4 = %d",ptr-ptr_len4);
    mylog(5,"ptr-ptr_len3 = %d",ptr-ptr_len3);
    mylog(5,"ptr-ptr_len2 = %d",ptr-ptr_len2);
    mylog(5,"ptr-ptr_len1 = %d",ptr-ptr_len1);*/
    *ptr_len6 = (ptr-ptr_len6);
    *ptr_len5 = (ptr-ptr_len5);
    *ptr_len4 = (ptr-ptr_len4);
    *ptr_len3 = (ptr-ptr_len3);
    *ptr_len2 = (ptr-ptr_len2);
    *ptr_len1 = (ptr-ptr_len1);
    return _internal_buf_size ;
}

#if 0
int CCSTACommand::stopIpRecording(char digit1,char digit2,char digit3,char digit4,const char* str_ipaddress)
{
    struct sockaddr_in sa;
    //char str[INET_ADDRSTRLEN];
    // store this IP address in sa:
    inet_pton(AF_INET, str_ipaddress/*"192.0.2.33"*/, &(sa.sin_addr));    // now get it back and print it
    //inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);    //printf("%s\n", str); // prints "192.0.2.33"
    uint32_t ip32 = (uint32_t)sa.sin_addr.s_addr;
    BYTE ip4 = ip32>>24 & 0xFF;
    BYTE ip3 = ip32>>16 & 0xFF;
    BYTE ip2 = ip32>>8 & 0xFF;
    BYTE ip1 = ip32>>0 & 0xFF;

    mylog(5,"CCSTACommand::stopIpRecording...");
    mylog(5,"ip1=%02X",ip1);
    mylog(5,"ip2=%02X",ip2);
    mylog(5,"ip3=%02X",ip3);
    mylog(5,"ip4=%02X",ip4);

    BYTE* debut = _internal_buf;// + 2;
    BYTE* ptr = debut;
    *ptr++ = 0;
    BYTE* ptr_len1 = ptr; ptr++;   //*ptr++ = 24+3; // tcp len following

    *ptr++ = 0xA1; // REQUEST PDU
    BYTE* ptr_len2 = ptr ; ptr++;//*ptr++ = 22+3; // len following

    // invokeID INTEGER: 0x0001
    *ptr++ = INTEGER_TAG;
    *ptr++ = 1; // integer len
    *ptr++ = invokeid++;

    // serviceID INTEGER operationValue=51
    *ptr++ = INTEGER_TAG;
    *ptr++ = 1; // integer len
    *ptr++ = 0x33; // decimal 51

    *ptr++ = 0x30; // sequence tag
    BYTE* ptr_len3 = ptr; ptr++;// length following

    //SEQUENCE OF
    *ptr++ = 0xa1; // sequence of tag
    BYTE* ptr_len4 = ptr; ptr++;// length following

    *ptr++ = 0x30; // sequence tag
    BYTE* ptr_len5 = ptr; ptr++;// length following

    //manufacturer 06 05 2b 0c 89 36 18
    *ptr++ = 6;*ptr++ = 5;*ptr++ = 0x2B;*ptr++ = 0x0c;*ptr++ = 0x89;*ptr++ = 0x36;*ptr++ = 0x18;

    //startIPRecordingMessageArgument
    //SEQUENCE
    *ptr++ = 0x30; // sequence tag
    BYTE* ptr_len6 = ptr; ptr++;// length following

    //recordedDevice dialingNumber
    *ptr++ = 0x80; *ptr++=4; *ptr++=digit1; *ptr++=digit2; *ptr++=digit3; *ptr++=digit4;
    //    '33303031'H  -- "3001" --,
    //loggerIPAddress
    *ptr++ = 0x04;*ptr++ = 4;*ptr++=ip1; *ptr++=ip2; *ptr++=ip3; *ptr++=ip4;

    _internal_buf_size = ptr - debut;
    ptr--;
    /*mylog(5,"_internal_buf_size = %d",_internal_buf_size);
    mylog(5,"ptr-debut = %d",ptr - debut);
    mylog(5,"ptr-ptr_len6 = %d",ptr-ptr_len6);
    mylog(5,"ptr-ptr_len5 = %d",ptr-ptr_len5);
    mylog(5,"ptr-ptr_len4 = %d",ptr-ptr_len4);
    mylog(5,"ptr-ptr_len3 = %d",ptr-ptr_len3);
    mylog(5,"ptr-ptr_len2 = %d",ptr-ptr_len2);
    mylog(5,"ptr-ptr_len1 = %d",ptr-ptr_len1);*/
    *ptr_len6 = (ptr-ptr_len6);
    *ptr_len5 = (ptr-ptr_len5);
    *ptr_len4 = (ptr-ptr_len4);
    *ptr_len3 = (ptr-ptr_len3);
    *ptr_len2 = (ptr-ptr_len2);
    *ptr_len1 = (ptr-ptr_len1);
    return _internal_buf_size ;
}
#endif

int CCSTACommand::startIpRecording(QString extension,QString str_ipaddress,unsigned short port1,unsigned short port2)
{    /*002ba1290201040201333021a11f301d06052b0c893618301480043330303004040afdfd050202138b0202138c
    Host< = 43
    a1 29 02 01 04 02 01 33 30 21 a1 1f 30 1d 06 05
    2b 0c 89 36 18 30 14 80 04 33 30 30 31 04 04 0a
    fd fd 05 02 02 13 8b 02 02 13 8c
    InComming:
    ENTREE:
    aPDU-rose
    invoke
                 { -- SEQUENCE --
                     invokeID
                         4,
                     operationValue
                         51,
                     argument
                         { -- SEQUENCE --
                             privateData
                                 { -- SEQUENCE OF --
                                    privateData
                                         { -- SEQUENCE --
                                             manufacturer
                                                 {1 3 12 1206 24},

                                                    startIPRecordingMessageArgument
                                                     { -- SEQUENCE --
                                                         recordedDevice
                                                        dialingNumber
                                                                 '33303031'H  -- "3001" --,
                                                         loggerIPAddress
                                                             '0afdfd05'H  -- "...." --,
                                                         port1
                                                             5003,
                                                         port2
                                                             5004
                                                     }
                                         }
                                 }
                         }
                 }
    */
    BYTE porta1 = port1 >> 8;
    BYTE porta2 = port1 & 0xFF;
    BYTE portb1 = port2 >> 8;
    BYTE portb2 = port2 & 0xFF;

    struct sockaddr_in sa;
    char str[INET_ADDRSTRLEN];
    // store this IP address in sa:
    inet_pton(AF_INET, str_ipaddress.toLatin1().constData() /*"192.0.2.33"*/, &(sa.sin_addr));
    // now get it back and print it
    //inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);
    //printf("%s\n", str); // prints "192.0.2.33"
    uint32_t ip32 = (uint32_t)sa.sin_addr.s_addr;
    BYTE ip4 = ip32>>24 & 0xFF;
    BYTE ip3 = ip32>>16 & 0xFF;
    BYTE ip2 = ip32>>8 & 0xFF;
    BYTE ip1 = ip32>>0 & 0xFF;

    mylog(5,"CCSTACommand::buildRecordExtension...");
    //mylog(5,"porta1=%02X",porta1);mylog(5,"porta2=%02X",porta2);mylog(5,"portb1=%02X",portb1);mylog(5,"portb2=%02X",portb2);
    //mylog(5,"ip1=%02X",ip1);mylog(5,"ip2=%02X",ip2);mylog(5,"ip3=%02X",ip3);mylog(5,"ip4=%02X",ip4);

    BYTE* debut = _internal_buf;// + 2;
    BYTE* ptr = debut;
    *ptr++ = 0;
    BYTE* ptr_len1 = ptr; ptr++;   //*ptr++ = 24+3; // tcp len following

    *ptr++ = 0xA1; // REQUEST PDU
    BYTE* ptr_len2 = ptr ; ptr++;//*ptr++ = 22+3; // len following


    // invokeID INTEGER: 0x0001
    *ptr++ = INTEGER_TAG;
    *ptr++ = 1; // integer len
    *ptr++ = invokeid++;//*ptr++ = 0x04;

    // serviceID INTEGER operationValue=51
    *ptr++ = INTEGER_TAG;
    *ptr++ = 1; // integer len
    *ptr++ = 0x33; // decimal 51

    *ptr++ = 0x30; // sequence tag
    BYTE* ptr_len3 = ptr; ptr++;// length following

    //SEQUENCE OF
    *ptr++ = 0xa1; // sequence of tag
    BYTE* ptr_len4 = ptr; ptr++;// length following

    *ptr++ = 0x30; // sequence tag
    BYTE* ptr_len5 = ptr; ptr++;// length following

    //manufacturer 06 05 2b 0c 89 36 18
    *ptr++ = 6;*ptr++ = 5;*ptr++ = 0x2B;*ptr++ = 0x0c;*ptr++ = 0x89;*ptr++ = 0x36;*ptr++ = 0x18;

    //startIPRecordingMessageArgument
    //SEQUENCE
    *ptr++ = 0x30; // sequence tag
    BYTE* ptr_len6 = ptr; ptr++;// length following

    //recordedDevice dialingNumber
    //*ptr++ = 0x80; *ptr++=4; *ptr++=digit1; *ptr++=digit2; *ptr++=digit3; *ptr++=digit4;
    //    '33303031'H  -- "3001" --,
    /**ptr++ = 0x80; *ptr++ = 8;
        *ptr++ = '1'; *ptr++ = '9';*ptr++ = '3';*ptr++ = '0' ;  // ascii string
        *ptr++ = digit1; *ptr++ = digit2;*ptr++ = digit3;*ptr++ = digit4;  // ascii string*/
    *ptr++ = 0x80; *ptr++ = extension.length();
    for (int i=0; i<extension.length() ; i++){
        *ptr++ = extension[i].toAscii();
        //*ptr++ = '1'; *ptr++ = '9';*ptr++ = '3';*ptr++ = '0' ;  // ascii string
        //*ptr++ = digit1; *ptr++ = digit2;*ptr++ = digit3;*ptr++ = digit4;  // ascii string
    }


    //loggerIPAddress
    *ptr++ = 0x04;*ptr++ = 4;*ptr++=ip1; *ptr++=ip2; *ptr++=ip3; *ptr++=ip4;

    //'0afdfd05'H  -- "...." --,
    //port1
    *ptr++ = 0x02;*ptr++ = 2;*ptr++=porta1;*ptr++=porta2;
    //5003,
    //port2
    *ptr++ = 0x02;*ptr++ = 2;*ptr++=portb1;*ptr++=portb2;
    //5004

    _internal_buf_size = ptr - debut;
    ptr--;
    /*mylog(5,"_internal_buf_size = %d",_internal_buf_size);
    mylog(5,"ptr-debut = %d",ptr - debut);
    mylog(5,"ptr-ptr_len6 = %d",ptr-ptr_len6);
    mylog(5,"ptr-ptr_len5 = %d",ptr-ptr_len5);
    mylog(5,"ptr-ptr_len4 = %d",ptr-ptr_len4);
    mylog(5,"ptr-ptr_len3 = %d",ptr-ptr_len3);
    mylog(5,"ptr-ptr_len2 = %d",ptr-ptr_len2);
    mylog(5,"ptr-ptr_len1 = %d",ptr-ptr_len1);*/
    *ptr_len6 = (ptr-ptr_len6);
    *ptr_len5 = (ptr-ptr_len5);
    *ptr_len4 = (ptr-ptr_len4);
    *ptr_len3 = (ptr-ptr_len3);
    *ptr_len2 = (ptr-ptr_len2);
    *ptr_len1 = (ptr-ptr_len1);
    return _internal_buf_size ;
}

#if 0
int CCSTACommand::startIpRecording(char digit1,char digit2,char digit3,char digit4,const char* str_ipaddress,unsigned short port1,unsigned short port2)
{    /*002ba1290201040201333021a11f301d06052b0c893618301480043330303004040afdfd050202138b0202138c
    Host< = 43
    a1 29 02 01 04 02 01 33 30 21 a1 1f 30 1d 06 05
    2b 0c 89 36 18 30 14 80 04 33 30 30 31 04 04 0a
    fd fd 05 02 02 13 8b 02 02 13 8c
    InComming:
    ENTREE:
    aPDU-rose
    invoke
                 { -- SEQUENCE --
                     invokeID
                         4,
                     operationValue
                         51,
                     argument
                         { -- SEQUENCE --
                             privateData
                                 { -- SEQUENCE OF --
                                    privateData
                                         { -- SEQUENCE --
                                             manufacturer
                                                 {1 3 12 1206 24},

                                                    startIPRecordingMessageArgument
                                                     { -- SEQUENCE --
                                                         recordedDevice
                                                        dialingNumber
                                                                 '33303031'H  -- "3001" --,
                                                         loggerIPAddress
                                                             '0afdfd05'H  -- "...." --,
                                                         port1
                                                             5003,
                                                         port2
                                                             5004
                                                     }
                                         }
                                 }
                         }
                 }
    */
    BYTE porta1 = port1 >> 8;
    BYTE porta2 = port1 & 0xFF;
    BYTE portb1 = port2 >> 8;
    BYTE portb2 = port2 & 0xFF;

    struct sockaddr_in sa;
    //char str[INET_ADDRSTRLEN];
    // store this IP address in sa:
    inet_pton(AF_INET, str_ipaddress/*"192.0.2.33"*/, &(sa.sin_addr));
    // now get it back and print it
    //inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);
    //printf("%s\n", str); // prints "192.0.2.33"
    uint32_t ip32 = (uint32_t)sa.sin_addr.s_addr;
    BYTE ip4 = ip32>>24 & 0xFF;
    BYTE ip3 = ip32>>16 & 0xFF;
    BYTE ip2 = ip32>>8 & 0xFF;
    BYTE ip1 = ip32>>0 & 0xFF;

    mylog(5,"CCSTACommand::buildRecordExtension...");
    //mylog(5,"porta1=%02X",porta1);mylog(5,"porta2=%02X",porta2);mylog(5,"portb1=%02X",portb1);mylog(5,"portb2=%02X",portb2);
    //mylog(5,"ip1=%02X",ip1);mylog(5,"ip2=%02X",ip2);mylog(5,"ip3=%02X",ip3);mylog(5,"ip4=%02X",ip4);

    BYTE* debut = _internal_buf;// + 2;
    BYTE* ptr = debut;
    *ptr++ = 0;
    BYTE* ptr_len1 = ptr; ptr++;   //*ptr++ = 24+3; // tcp len following

    *ptr++ = 0xA1; // REQUEST PDU
    BYTE* ptr_len2 = ptr ; ptr++;//*ptr++ = 22+3; // len following


    // invokeID INTEGER: 0x0001
    *ptr++ = INTEGER_TAG;
    *ptr++ = 1; // integer len
    *ptr++ = invokeid++;//*ptr++ = 0x04;

    // serviceID INTEGER operationValue=51
    *ptr++ = INTEGER_TAG;
    *ptr++ = 1; // integer len
    *ptr++ = 0x33; // decimal 51

    *ptr++ = 0x30; // sequence tag
    BYTE* ptr_len3 = ptr; ptr++;// length following

    //SEQUENCE OF
    *ptr++ = 0xa1; // sequence of tag
    BYTE* ptr_len4 = ptr; ptr++;// length following

    *ptr++ = 0x30; // sequence tag
    BYTE* ptr_len5 = ptr; ptr++;// length following

    //manufacturer 06 05 2b 0c 89 36 18
    *ptr++ = 6;*ptr++ = 5;*ptr++ = 0x2B;*ptr++ = 0x0c;*ptr++ = 0x89;*ptr++ = 0x36;*ptr++ = 0x18;

    //startIPRecordingMessageArgument
    //SEQUENCE
    *ptr++ = 0x30; // sequence tag
    BYTE* ptr_len6 = ptr; ptr++;// length following

    //recordedDevice dialingNumber
    //*ptr++ = 0x80; *ptr++=4; *ptr++=digit1; *ptr++=digit2; *ptr++=digit3; *ptr++=digit4;
    //    '33303031'H  -- "3001" --,
    *ptr++ = 0x80; *ptr++ = 8;
    *ptr++ = '1'; *ptr++ = '9';*ptr++ = '3';*ptr++ = '0' ;  // ascii string
    *ptr++ = digit1; *ptr++ = digit2;*ptr++ = digit3;*ptr++ = digit4;  // ascii string


    //loggerIPAddress
    *ptr++ = 0x04;*ptr++ = 4;*ptr++=ip1; *ptr++=ip2; *ptr++=ip3; *ptr++=ip4;

    //'0afdfd05'H  -- "...." --,
    //port1
    *ptr++ = 0x02;*ptr++ = 2;*ptr++=porta1;*ptr++=porta2;
    //5003,
    //port2
    *ptr++ = 0x02;*ptr++ = 2;*ptr++=portb1;*ptr++=portb2;
    //5004

    _internal_buf_size = ptr - debut;
    ptr--;
    /*mylog(5,"_internal_buf_size = %d",_internal_buf_size);
    mylog(5,"ptr-debut = %d",ptr - debut);
    mylog(5,"ptr-ptr_len6 = %d",ptr-ptr_len6);
    mylog(5,"ptr-ptr_len5 = %d",ptr-ptr_len5);
    mylog(5,"ptr-ptr_len4 = %d",ptr-ptr_len4);
    mylog(5,"ptr-ptr_len3 = %d",ptr-ptr_len3);
    mylog(5,"ptr-ptr_len2 = %d",ptr-ptr_len2);
    mylog(5,"ptr-ptr_len1 = %d",ptr-ptr_len1);*/
    *ptr_len6 = (ptr-ptr_len6);
    *ptr_len5 = (ptr-ptr_len5);
    *ptr_len4 = (ptr-ptr_len4);
    *ptr_len3 = (ptr-ptr_len3);
    *ptr_len2 = (ptr-ptr_len2);
    *ptr_len1 = (ptr-ptr_len1);
    return _internal_buf_size ;
}
#endif

void CCSTACommand::hexdump()
{
    //fprintf(stderr,"--> CCSTACommand _internal_buf_size[%d]\n",_internal_buf_size);
    char* str = (char*)malloc(3*_internal_buf_size+1);
    if ( str ){
        int str_len=0;
        for( int k=0,m=0; k< _internal_buf_size; k++,m++){
            str_len += sprintf(str+str_len, "%02X-", _internal_buf[k]);
        }
        fprintf(stderr,"CCSTACommand [%s]\n",str);
    }
}

bool CCSTACommand::checkInternalPointedArea(BYTE* ptrField,unsigned short lenField)
{
    if (!( ptrField>=_internal_buf && ptrField <_internal_buf+_internal_buf_size ) ){
        qDebug() << "BAD POINTER"; return false;
    }if (!( ptrField + lenField <_internal_buf+_internal_buf_size ) ){
        qDebug() << "BAD POINTER LENGTH"; return false;
    }return true;
}



BYTE* CCSTACommand::TLVReadValue(BYTE* ptr_in, int len)
{
    if ( checkInternalPointedArea(ptr_in,len) == false ) return NULL;
    BYTE* ptrNextField = NULL;
    BYTE* ptr = ptr_in;
    _TLVValue = QByteArray((const char*)ptr_in, len);
    //mylog(5,"** TLVReadValue starting at byte: %02X ( %d bytes )**", *ptr_in, len);
    //hexdump(ptr_in,len);
    ptrNextField = ptr_in + len;
    //mylog(5,"** %p pointe sur le champ suivant %02X",ptrNextField,*ptrNextField);
    return ptrNextField;
}

BYTE* CCSTACommand::TLVReadLength(BYTE* ptrField, int* tagV, int* tagL)
{
    //Q_ASSERT(ptrField>=_internal_buf);    Q_ASSERT(ptrField<_internal_buf+_internal_buf_size);
    if ( checkInternalPointedArea(ptrField,1) == false ) return NULL;
    BYTE* ptrNextField = NULL;
    BYTE* ptr = ptrField ;

    //mylog(5,"******************************************************************");
    //mylog(5,"** TLVReadLength: LONGUEUR starting at byte: %02X **", *ptr);
    int len_len = 0; // longueur du champ
    if  (  ( *ptr & 0x80) != 0x80  ) {
        len_len = 1;
        *tagL = 1;
        *tagV = *ptr;
        //mylog(5,"champ d'un seul octet: premier bit a ZERO, longueur: forme courte len_len=%d value of len=%d", len_len,*tagV);

        ptr ++;// pointe sur le champ suivant
    }else{ // champ de plusieurs octets: premier bit a UN
        if   (  ( *ptr & 0x7F) == 0  )  {
            //mylog(5,"TLVReadLength: forme indefinie");
            return NULL;
        }else{	// champ sur plusieurs octet: premier bit a UN et au moins un des autres bits non nul
            int k=0;
            int len_len =  *ptr & 0x7F;
            int len = 0;
            for (k=0; k <len_len; k++){ // 128 = 1000 0001 0000 0000
                ptr++;
                if ( checkInternalPointedArea(ptr,1) == false ) return NULL;
                len = 256*len + (*ptr) ; // 1024 = 1000 0010  0000 0100 0000 0000
                //mylog(5,"TLVReadLength octet[%d]: forme longue %02X len=%d", k, *ptr,len);
            }
            //mylog(5,"len_len=%d len=%d", len_len, len);
            *tagL = len_len;
            *tagV = len;
            //mylog(5,"longueur: forme longue de %d octets = %d", len_len, len);
        }
        ptr ++;// pointe sur le champ suivant
    }
    ptrNextField = ptr;
    //mylog(5,"** %p pointe sur le champ suivant %02X",ptrNextField,*ptrNextField);
    return ptrNextField;
}

BYTE* CCSTACommand::TLVReadTag(BYTE* ptr_in, int* tagV,int* tagL)
{
    if ( checkInternalPointedArea(ptr_in,1) == false ) return NULL;
    BYTE* ptrNextField = NULL;
    BYTE* ptr = ptr_in;
    //mylog(5,"******************************************************************");
    //mylog(5,"** TAG starting first byte %02X  **", *ptr);
    /*if  ( (*ptr & 0x20) == 0x20)  {mylog(5,"              structured TYPE");}
    if  ( (*ptr & 0xC0) == 0xC0 ) {mylog(5,"              private TYPE");}
    if  ( (*ptr & 0x40) == 0x40 ) {mylog(5,"              application TYPE");}
    if  ( (*ptr & 0x80 ) == 0x80) {mylog(5,"              context-specific TYPE");}*/

    if  ( ( *ptr & 0x1F) <= 30 ) {
        *tagL = 1;
        *tagV = *ptr & 0x1F;
        //mylog(5,"tag: forme courte %02X == %s", *tagV, tag_to_string(*tagV) );
        ptr++; // pointe sur le champ suivant
    }else{
        //mylog(5,"tag: forme longue %02X", *ptr);
        *tagL = *ptr;
        ptr++;	// sauter l'octet qui donne la longueur du tag
        while (*ptr & 0x80 ) {
            //mylog(5,"jumping over tag len byte %02X", *ptr);
            if ( checkInternalPointedArea(ptr,1) == false ) return NULL;
            *tagV += *ptr;
            ptr++; // pointe sur le champ suivant
        }
    }
    ptrNextField = ptr;
    //mylog(5,"** %p pointe sur le champ suivant %02X",ptrNextField,*ptrNextField);
    return ptrNextField; // pointe sur le champ Length suivant le tag
}
/*(1)xa000000> cat /tmpd/logfile.2-10.253.253.5
Host< = 16
a1 0e 02 01 01 02 01 47 30 06 80 04 33 30 30 31

Host> = 44
a2 2a 02 01 01 30 25 02 01 47 30 20 55 04 01 42
f9 00 30 18 80 03 02 80 00 81 02 00 7f 82 02 02
00 83 02 06 00 85 02 01 6e 84 01 00

Host> = 12
a1 0a 02 02 2b bf 02 01 34 0a 01 02

(1)xa000000>

(1)xa000000> pilot2a -aB  /tmpd/logfile.2-10.253.253.5
-- Pilot version: 3.2
-- |	Copyright 2001, 2002 A.B.S. Brest
InComming:
ENTREE:
aPDU-rose
invoke
             { -- SEQUENCE --
                 invokeID
                     1,
                 operationValue
                     71,
                 argument
                     { -- SEQUENCE --
                         monitorObject
device
dialingNumber
                                     '33303031'H  -- "3001" --
                     }
             }

Outgoing:
ENTREE:
aPDU-rose
retResult
             { -- SEQUENCE --
                 invokeID
                     1,

                     { -- SEQUENCE --
                         operationValue
                             71,
                         result
                             { -- SEQUENCE --
                                 crossRefIdentifier
                                     '0142f900'H  -- ".B.." --,
                                 monitorFilter
                                     { -- SEQUENCE --
                                         call
                                             '8000'H,
                                         feature
                                             '7f'H,
                                         agent
                                             '00'H,
                                         maintenance
                                             '00'H,
                                         voiceUnit
                                             '6e'H,
                                         privatedata
FALSE
                                     }
                             }
                     }
             }


--Service : SER_MONITOR_START (rose-id=1 host=0)
--| 	cstaSearCrid=21166336=0x142f900
--| 	cstaSearRoid=1=0x1
--| 	Connection 1 : (call=0,neqt=0,type=TYPE_NOT_PROVIDED)
--| 	Connection 2 : (call=0,neqt=0,type=TYPE_NOT_PROVIDED)
--| 	        	(L)Device 1 : ABSENT
--| 	        	(L)Device 2 : ABSENT
--| 	cstaSearFilt=0xffff8001 0xfffff8c0
--| 	Feature : ident=0 invoke=0 val1=0 val2=0
Outgoing:
ENTREE:
aPDU-rose
invoke
             { -- SEQUENCE --
                 invokeID
                     11199,
                 operationValue
                     52,
                 argument
systemStatus
                         2
             }

SORTIE: longueur 13
retResult
         { -- SEQUENCE --
             invokeID
                 11199,

                 { -- SEQUENCE --
                     operationValue
                         52,
                     result
noData
                             NULL
                 }
         }
*/
const char* CCSTACommand::tag_to_string(int tag)
{
    switch(tag){
    case 1: return("BOOLEAN"); break;
    case INTEGER_TAG: return("INTEGER"); break;
    case 3: return("BIT STRING"); break;
    case 4: return("OCTET STRING"); break;
    case 6: return("OBJECT IDENTIFIER"); break;
    case 9: return("REAL"); break;
    case 10: return("ENUMERATED"); break;
    case SEQUENCE_TAG: return("SEQUENCE OF"); break;
    case 17: return("SET OF"); break;
    case 18: return("NUMERIC STRING"); break;
    case 19: return("PRINTABLE STRING"); break;
    case 22: return("IA5STRING"); break;
    default: return "unkonwn";
    }
}

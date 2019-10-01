#include "common.h"
#include "sip.h"
#include "utils.h"
#include "md5.h"
#include "sip.h"

//#define REGISTER_TAG1                   "2987238973897389"
//#define UNREGISTER_TAG1                 "76767545645689"
//#define VIA_REGISTER_TAG_BRANCH1        "z9hG4bK16516198290821111" //Via: SIP/2.0/UDP 192.168.0.5:5060;rport;branch=z9hG4bK1651650955
//#define VIA_UNREGISTER_TAG_BRANCH1      "z9hG4bK16516867787111"

bool CUserAgent::sipRegister()
{
#ifdef OLD_MSG_FORMAT
    BYTE msg_register[1024]={
        "REGISTER sip:"_phone_number"@"SIP_PROXY_ADDR" SIP/2.0\r\n"
        "From: "_phone_number" <sip:"_phone_number"@"SIP_PROXY_ADDR">;tag=75011f9b\r\n"
        "To: <sip:"_phone_number"@"SIP_PROXY_ADDR">\r\n"
        "Call-ID: "UA_REG_CALLID"@"IP_CALLER"\r\n"
        "CSeq: 1 REGISTER\r\n"
        "Contact: <sip:"_phone_number"@"IP_CALLER":5060;LINEID=da8fc4c53766b50b1d5879473d0bbbaf>\r\n"
        "Expires: 3600\r\n"
        "Date: Tue, 06 Dec 20 05 14:26:41 GMT\r\n"
        "Max-Forwards: 20\r\n"
        "User-Agent: DebreuilPhone UA 1.0\r\n"
        "Accept-Language: fr\r\n"
        "Via: SIP/2.0/UDP "IP_CALLER";branch=z9hG4bK-ca5f8d2e8587b7ec7d5b1a63086cb7ff;rport\r\n"
        "Authorization: \r\n"
        "Content-Length: 0\r\n"
        "\r\n"
    };
#else
    _current_register_call_id = genRandomCallID(QString("reg%1_").arg(_phone_number));
    _register_cseq = genRandomSequenceNumber(3);
    _register_tag_from = genRandomTag();
    _register_branch_tag_via = genRandomBranch();

    CSipMessage* p_msg = new CSipMessage();

    p_msg->setMethod(QString("REGISTER"),_phone_number,QString(SIP_PROXY_ADDR));
    p_msg->setFrom(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),_register_tag_from);
    p_msg->setTo(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),QString(""));
    p_msg->setCallId(_current_register_call_id);
    p_msg->setCSeq(_register_cseq,QString("REGISTER"));
    p_msg->setContact(_phone_number,QString(SIP_PROXY_ADDR));
    p_msg->_sipFieldList.append("Expires: 3600");
    p_msg->setVia(QString(_local_address),QString(_register_branch_tag_via));
    p_msg->_sipFieldList.append("Authorization: ");

    send(p_msg);//p_msg->prepareBuffer();//    sip_send(p_msg->_pBuf, p_msg->_preparedLen );

#endif
    return true;
}
bool CUserAgent::sipUnRegister()
{    /*REGISTER sip:29.11.0.222 SIP/2.0
    CSeq: 56 REGISTER
    Via: SIP/2.0/UDP 109.8.41.225:5060;branch=z9hG4bK98d5161e-b3a9-e511-8453-6cf049152c3d;rport
    User-Agent: Ekiga/4.0.1
    Authorization: Digest username="9001", realm="asterisk", nonce="18facb8b", uri="sip:29.11.0.222", algorithm=MD5, response="6bffb670e7874ef9ed469a0c8715751e"
    From: <sip:9001@29.11.0.222>;tag=a83ae0b7-b2a9-e511-8453-6cf049152c3d
    Call-ID: a231e0b7-b2a9-e511-8453-6cf049152c3d@phd-GA-MA78LM-S2H
    To: <sip:9001@29.11.0.222>
    Contact: <sip:9001@192.168.0.5:5060>
    Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING,PRACK
    Expires: 0
    Content-Length: 0
    Max-Forwards: 70*/
    _current_unregister_call_id = genRandomCallID("unreg");
    _register_cseq = genRandomSequenceNumber(3);
    _unregister_tag_from = genRandomTag();
    _unregister_branch_tag_via = genRandomBranch();

    CSipMessage* p_msg = new CSipMessage();

    p_msg->setRegister(QString(SIP_PROXY_ADDR));
    p_msg->setFrom(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),_unregister_tag_from);
    p_msg->setTo(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),QString(""));

    p_msg->setCallId(_current_unregister_call_id);
    p_msg->setCSeq(_register_cseq,QString("REGISTER"));
    p_msg->setContact(_phone_number,QString(SIP_PROXY_ADDR));
    p_msg->_sipFieldList.append("Expires: 0");
    p_msg->_sipFieldList.append(
                QString("Authorization: Digest username=\"%1\", realm=\"asterisk\", nonce=\"18facb8b\", uri=\"sip:%2@%3\", algorithm=MD5, response=\"6bffb670e7874ef9ed469a0c8715751e\""
                        )
                .arg(_phone_number).arg(_phone_number).arg(SIP_PROXY_ADDR)
                );
    p_msg->setVia(QString(_local_address),_unregister_branch_tag_via);

    send(p_msg);//p_msg->prepareBuffer();//    sip_send(p_msg->_pBuf, p_msg->_preparedLen );
    return true;
}

bool CUserAgent::sipRegisterChallenge(CSipMessage* pSipMsg,QString call_id,const char* nonce, int seconds_before_expires)
{
    qDebug() << "sipRegisterChallenge nonce:" << nonce;

#ifdef OLD_MSG_FORMAT
    BYTE msg_register[1024]={
        "REGISTER sip:"_phone_number"@"SIP_PROXY_ADDR" SIP/2.0\r\n"
        "From: "_phone_number" <sip:"_phone_number"@"SIP_PROXY_ADDR">;tag=75011f9b\r\n"
        "To: <sip:"_phone_number"@"SIP_PROXY_ADDR">\r\n"
        "Call-ID: "UA_REG_CALLID"@"IP_CALLER"\r\n"
        "CSeq: 1 REGISTER\r\n"
        "Contact: <sip:"_phone_number"@"IP_CALLER":5060;LINEID=da8fc4c53766b50b1d5879473d0bbbaf>\r\n"
        "Expires: 3600\r\n"
        "Max-Forwards: 20\r\n"
        "User-Agent: DebreuilPhone UA 1.0\r\n"
        "Accept-Language: fr\r\n"
        "Via: SIP/2.0/UDP "IP_CALLER";branch=z9hG4bK-ca5f8d2e8587b7ec7d5b1a63086cb7ff;rport\r\n"
        "Authorization: \r\n"
        "Content-Length: 0\r\n"
        "\r\n"VIA_TAG_BRANCH1
    };
#else
    /*<--- Transmitting (NAT) to 29.11.0.221:53293 --->
    SIP/2.0 403 Forbidden
    Via: SIP/2.0/UDP 29.11.0.221:5060;branch=z9hG4bK16516198290821111;received=29.11.0.221;rport=53293
    From: 9999<sip:9999@29.11.0.222>;tag=2987238973897389
    To: 9003<sip:9003@29.11.0.222>;tag=as374da165
    Call-ID: J2U5O6K6E5P1@29.11.0.221
    CSeq: 1 REGISTER
    Server: Asterisk PBX 1.8.32.3
    Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, SUBSCRIBE, NOTIFY, INFO, PUBLISH, MESSAGE
    Supported: replaces, timer
    Content-Length: 0*/
    char digest[256]={0};
    int digest_len=sizeof(digest);
    build_reply_digest(
                _phone_number.toLatin1().constData(),
                "asterisk",
                _md5_password.toLatin1().constData(),
                "REGISTER",
                QString("sip:%1@%2").arg(_phone_number).arg(SIP_PROXY_ADDR).toLatin1().constData(),
                nonce,
                digest, digest_len
                );

    CSipMessage* p_msg = new CSipMessage();
    _register_cseq++;

    p_msg->setRegister(QString(SIP_PROXY_ADDR));
    p_msg->setFrom(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),_register_tag_from);
    p_msg->setTo(_phone_number,_phone_number,QString(SIP_PROXY_ADDR),QString(""));
    p_msg->setCallId(call_id);
    p_msg->setCSeq(_register_cseq,QString("REGISTER"));
    //p_msg->setContact(_phone_number,QString(SIP_PROXY_ADDR));
    p_msg->_sipFieldList.append(QString("Expires: %1").arg(seconds_before_expires));
    p_msg->setVia(QString(_local_address),QString(_register_branch_tag_via));
    p_msg->_sipFieldList.append(digest);

    send(p_msg);//sip_send(p_msg->_pBuf, p_msg->_preparedLen );
#endif
    return true;
}


#if 0 /*AST_1_8*/
/*! \brief  Build reply digest
\return	Returns -1 if we have no auth
\note	Build digest challenge for authentication of registrations and calls
    Also used for authentication of BYE
*/
static int build_reply_digest(struct sip_pvt *p, int method, char* digest, int digest_len)
{
    char a1[256];
    char a2[256];
    char a1_hash[256];
    char a2_hash[256];
    char resp[256];
    char resp_hash[256];
    char uri[256];
    char opaque[256] = "";
    char cnonce[80];
    const char *username;
    const char *secret;
    const char *md5secret;
    struct sip_auth *auth;	/* Realm authentication credential */
    struct sip_auth_container *credentials;

    if (!ast_strlen_zero(p->domain))
        snprintf(uri, sizeof(uri), "%s:%s", p->socket.type == SIP_TRANSPORT_TLS ? "sips" : "sip", p->domain);
    else if (!ast_strlen_zero(p->uri))
        ast_copy_string(uri, p->uri, sizeof(uri));
    else
        snprintf(uri, sizeof(uri), "%s:%s@%s", p->socket.type == SIP_TRANSPORT_TLS ? "sips" : "sip", p->username, ast_sockaddr_stringify_host_remote(&p->sa));

    snprintf(cnonce, sizeof(cnonce), "%08lx", (unsigned long)ast_random());

    /* Check if we have peer credentials */
    ao2_lock(p);
    credentials = p->peerauth;
    if (credentials) {
        ao2_t_ref(credentials, +1, "Ref peer auth for digest");
    }
    ao2_unlock(p);
    auth = find_realm_authentication(credentials, p->realm);
    if (!auth) {
        /* If not, check global credentials */
        if (credentials) {
            ao2_t_ref(credentials, -1, "Unref peer auth for digest");
        }
        ast_mutex_lock(&authl_lock);
        credentials = authl;
        if (credentials) {
            ao2_t_ref(credentials, +1, "Ref global auth for digest");
        }
        ast_mutex_unlock(&authl_lock);
        auth = find_realm_authentication(credentials, p->realm);
    }

    if (auth) {
        ast_debug(3, "use realm [%s] from peer [%s][%s]\n", auth->username, p->peername, p->username);
        username = auth->username;
        secret = auth->secret;
        md5secret = auth->md5secret;
        if (sipdebug)
            ast_debug(1, "Using realm %s authentication for call %s\n", p->realm, p->callid);
    } else {
        /* No authentication, use peer or register= config */
        username = p->authname;
        secret = p->relatedpeer
            && !ast_strlen_zero(p->relatedpeer->remotesecret)
                ? p->relatedpeer->remotesecret : p->peersecret;
        md5secret = p->peermd5secret;
    }
    if (ast_strlen_zero(username)) {
        /* We have no authentication */
        if (credentials) {
            ao2_t_ref(credentials, -1, "Unref auth for digest");
        }
        return -1;
    }

    /* Calculate SIP digest response */
    snprintf(a1, sizeof(a1), "%s:%s:%s", username, p->realm, secret);
    snprintf(a2, sizeof(a2), "%s:%s", sip_methods[method].text, uri);
    if (!ast_strlen_zero(md5secret))
        ast_copy_string(a1_hash, md5secret, sizeof(a1_hash));
    else
        ast_md5_hash(a1_hash, a1);
    ast_md5_hash(a2_hash, a2);

    p->noncecount++;
    if (!ast_strlen_zero(p->qop))
        snprintf(resp, sizeof(resp), "%s:%s:%08x:%s:%s:%s", a1_hash, p->nonce, (unsigned)p->noncecount, cnonce, "auth", a2_hash);
    else
        snprintf(resp, sizeof(resp), "%s:%s:%s", a1_hash, p->nonce, a2_hash);
    ast_md5_hash(resp_hash, resp);

    /* only include the opaque string if it's set */
    if (!ast_strlen_zero(p->opaque)) {
        snprintf(opaque, sizeof(opaque), ", opaque=\"%s\"", p->opaque);
    }

    /* XXX We hard code our qop to "auth" for now.  XXX */
    if (!ast_strlen_zero(p->qop))
        snprintf(digest, digest_len, "Digest username=\"%s\", realm=\"%s\", algorithm=MD5, uri=\"%s\", nonce=\"%s\", response=\"%s\"%s, qop=auth, cnonce=\"%s\", nc=%08x", username, p->realm, uri, p->nonce, resp_hash, opaque, cnonce, (unsigned)p->noncecount);
    else
        snprintf(digest, digest_len, "Digest username=\"%s\", realm=\"%s\", algorithm=MD5, uri=\"%s\", nonce=\"%s\", response=\"%s\"%s", username, p->realm, uri, p->nonce, resp_hash, opaque);

    append_history(p, "AuthResp", "Auth response sent for %s in realm %s - nc %d", username, p->realm, p->noncecount);

    if (credentials) {
        ao2_t_ref(credentials, -1, "Unref auth for digest");
    }
    return 0;
}
#endif

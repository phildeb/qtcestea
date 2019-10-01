#ifndef MD5_H
#define MD5_H

#define byteReverse(buf, len)	/* Nothing */

#define uint32_t unsigned int

typedef struct
{
    uint32_t buf[4];
    uint32_t bits[2];
    unsigned char in[64];
}MD5Context;

void MD5Init(MD5Context *context);
void MD5Update(MD5Context *context, unsigned char const *buf,unsigned len);
void MD5Final(unsigned char digest[16], MD5Context *context);
void MD5Transform(uint32_t buf[4], uint32_t const in[16]);
void ast_md5_hash(char *output, char *input);
/*! Build reply digest Returns -1 if we have no auth
Build digest challenge for authentication of registrations and calls Also used for authentication of BYE
build_reply_digest(    UA_NUMBER_LOCAL, "asterisk", UA_PASSWD, "REGISTER", "sip:"UA_NUMBER_CALLED"@"SIP_PROXY_ADDR, nonce,    digest, digest_len); */
int build_reply_digest(const char* username,const char* realm,const char* secret,const char* method, const char* uri, const char* nonce,char* digest, int digest_len);

#endif // MD5_H

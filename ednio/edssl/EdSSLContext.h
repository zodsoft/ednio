/*
 * EdSSL.h
 *
 *  Created on: Aug 5, 2014
 *      Author: netmind
 */

#ifndef EDSSL_H_
#define EDSSL_H_

#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>

#include "EdSSLSocket.h"

namespace edft
{

enum {
	SSL_VER_V23,
	SSL_VER_V3,
	SSL_VER_TLSV1,
	SSL_VER_TLSV11,
	SSL_VER_TLSV12,
	SSL_VER_DTLSV1,
};



int EdSSLInit();
bool EdSSLIsInit();

class EdSSLContext
{
public:
	EdSSLContext();
	virtual ~EdSSLContext();


	static SSL_CTX* buildServerCtx(int sslmethod, const char* certfile, const char* privkeyfile);
	static SSL_CTX* buildClientCtx(int ver);
	static SSL_CTX* buildCtx(int ver);
	static EdSSLContext* getDefaultEdSSL();
	static void freeDefaultEdSSL();
	static int password_cb(char *buf, int size, int rwflag, void *userdata);

	void open(int ver);
	void close();

	SSL_CTX* getContext();
	int setSSLCertFile(const char* certfile, const char* privkeyfile);
	int setSSLCertMem(void *crt, int crtlen, void* key, int keylen);
	void setCertPassword(const char* pw);
private:
	int dgPasswordCb(char* buf, int size, int rwflag);

private:
	SSL_CTX *mCtx;
	char mPasswd[20];

};

extern __thread class EdSSLContext *_tDefEdSSL;
;
} /* namespace edft */

#endif /* EDSSL_H_ */

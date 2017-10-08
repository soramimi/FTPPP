/***************************************************************************
                          ftplib.h  -  description
                             -------------------
    begin                : Son Jul 27 2003
    copyright            : (C) 2013 by magnus kulke
    email                : mkulke@gmail.com
 ***************************************************************************/

// modified for qFTP by S.Fuchita (@soramimi_jp) 2017

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        * 
 *   published by the Free Software Foundation; either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/
 
/***************************************************************************
 * Note: ftplib, on which ftplibpp was originally based upon used to be    *
 * licensed as GPL 2.0 software, as of Jan. 26th 2013 its author Thomas    *
 * Pfau allowed the distribution of ftplib via LGPL. Thus the license of   *
 * ftplibpp changed aswell.                                                *
 ***************************************************************************/
 
#ifndef FTPLIB_H
#define FTPLIB_H

//#define NOSSL 1

#include <stdint.h>
#include <QIODevice>
#include <memory>

#if defined(_WIN32)

#if BUILDING_DLL
# define DLLIMPORT __declspec (dllexport)
#else /* Not BUILDING_DLL */
# define DLLIMPORT __declspec (dllimport)
#endif /* Not BUILDING_DLL */

#include <time.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#endif

#ifdef NOLFS
#define int64_t long
#endif

#if defined(__APPLE__)
#define int64_t __darwin_off_t
#define fseeko64 fseeko
#define fopen64 fopen
#endif

#ifndef NOSSL
#include <openssl/ssl.h>
#endif

#ifndef _FTPLIB_SSL_CLIENT_METHOD_
#define _FTPLIB_SSL_CLIENT_METHOD_ TLSv1_2_client_method
#endif//_FTPLIB_SSL_CLIENT_METHOD_

using namespace std;

/**
  *@author mkulke
  */

typedef int (*FtpCallbackXfer)(int64_t xfered, void *arg);
typedef int (*FtpCallbackIdle)(void *arg);
typedef void (*FtpCallbackLog)(char *str, void *arg, bool out);

#ifndef NOSSL
typedef bool (*FtpCallbackCert)(void *arg, X509 *cert);
#endif

class ftplib {
private:
	struct ftphandle;
	typedef std::shared_ptr<ftphandle> ftphandle_ptr;
public:

	enum accesstype {
		Dir = 1,
		DirVerbose,
		FileRead,
		FileWrite,
		FileReadAppend,
		FileWriteAppend
	}; 

	enum transfermode {
		Ascii = 'A',
		Image = 'I'
	};

	enum connmode {
		pasv = 1,
		port
	};

	enum fxpmethod {
		defaultfxp = 0,
        alternativefxp
	};

	enum dataencryption {
        unencrypted = 0,
        secure
    };

private:
	ftphandle_ptr mp_ftphandle;

	int ftpXfer(QIODevice *io, const char *path, ftphandle_ptr nControl, accesstype type, transfermode mode);
	int ftpOpenPasv(ftphandle_ptr nControl, ftphandle_ptr *nData, transfermode mode, int dir, char *cmd);
	int ftpSendCmd(const char *cmd, char expresp, ftphandle_ptr nControl);
	int ftpAcceptConnection(ftphandle_ptr nData, ftphandle_ptr nControl);
	int ftpOpenPort(ftphandle_ptr nControl, ftphandle_ptr *nData, transfermode mode, int dir, char *cmd);
	int ftpRead(void *buf, int max, ftphandle_ptr nData);
	int ftpWrite(void *buf, int len, ftphandle_ptr nData);
	int ftpAccess(const char *path, accesstype type, transfermode mode, ftphandle_ptr nControl, ftphandle_ptr *nData);
	int ftpClose(ftphandle_ptr nData);

	int socket_wait(ftphandle_ptr ctl);
	int readline(char *buf,int max,ftphandle_ptr ctl);
	int writeline(char *buf, int len, ftphandle_ptr nData);
	int readresp(char c, ftphandle_ptr nControl);

	void clearHandle();
	int correctPasvResponse(unsigned char *v);
public:
	ftplib();
	~ftplib();
	char *lastResponse();
	int connect(const char *host);
	int login(const char *user, const char *pass);
	int site(const char *cmd);
	int raw(const char *cmd);
	int systype(char *buf, int max);
	int mkdir(const char *path);
	int chdir(const char *path);
	int cdup();
	int rmdir(const char *path);
	int pwd(char *path, int max);
	int nlst(QIODevice *outputfile, const char *path);
	int dir(QIODevice *outputfile, const char *path);
	int size(const char *path, int *size, transfermode mode);
	int moddate(const char *path, char *dt, int max);
	int get(QIODevice *outputfile, const char *path, transfermode mode, int64_t offset = 0);
	int put(QIODevice *inputfile, const char *path, transfermode mode, int64_t offset = 0);
	int rename(const char *src, const char *dst);
	int remove(const char *path);
#ifndef NOSSL    
	int setDataEncryption(dataencryption enc);
	int negotiateEncryption();
	void setCallbackCertFunction(FtpCallbackCert pointer);
#endif
    int quit();
	void setCallbackIdleFunction(FtpCallbackIdle pointer);
	void setCallbackLogFunction(FtpCallbackLog pointer);
	void setCallbackXferFunction(FtpCallbackXfer pointer);
	void setCallbackArg(void *arg);
	void setCallbackBytes(int64_t bytes);
	void setCorrectPasv(bool b);
	void setCallbackIdletime(int time);
	void setConnmode(connmode mode);
	static int fxp(ftplib* src, ftplib* dst, const char *pathSrc, const char *pathDst, transfermode mode, fxpmethod method);
    
	ftphandle_ptr rawOpen(const char *path, accesstype type, transfermode mode);
	int rawClose(ftphandle_ptr handle);
	int rawWrite(void* buf, int len, ftphandle_ptr handle);
	int rawRead(void* buf, int max, ftphandle_ptr handle);
};

#endif

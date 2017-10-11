#include "FTP.h"
#include "ftplib.h"

void FTP::create()
{
	close();
	ptr = ftp_ptr(new ftplib);
}

void FTP::reset()
{
	ptr.reset();
}

bool FTP::open(const QString &server, const QString &user, const QString &pass)
{
	reset();
	ftp_ptr p = ftp_ptr(new ftplib);
	if (p->connect(server.toStdString().c_str())) {
		if (p->login(user.toStdString().c_str(), pass.toStdString().c_str())) {
			ptr = p;
			return true;
		}
	}
	return false;
}

bool FTP::feat(QIODevice *outputfile)
{
	return ptr ? ptr->feat(outputfile) : false;
}

bool FTP::dir(QIODevice *outputfile, const char *path)
{
	return ptr ? ptr->dir(outputfile, path) : false;
}

bool FTP::mlsd(QIODevice *outputfile, const char *path)
{
	return ptr ? ptr->mlsd(outputfile, path) : false;
}

void FTP::quit()
{
	if (ptr) ptr->quit();
	reset();
}

void FTP::close()
{
	quit();
}



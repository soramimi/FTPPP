#ifndef FTP_H
#define FTP_H

#include <QIODevice>
#include <QString>
#include <memory>

class ftplib;

typedef std::shared_ptr<ftplib> ftp_ptr;

class FTP {
private:
	ftp_ptr ptr;
public:
	void create();
	void reset();
public:
	bool open(const QString &server, const QString &user, const QString &pass);
	void close();
	bool feat(QIODevice *outputfile);
	bool dir(QIODevice *outputfile, const char *path);
	bool mlsd(QIODevice *outputfile, const char *path);
	void quit();
	std::string pwd();
};

#endif // FTP_H

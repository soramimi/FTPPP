#include "MySettings.h"
#include "main.h"
#include "joinpath.h"
#include <QApplication>
#include <QDir>
#include <QString>
#include <QStandardPaths>
#include <QDebug>

QString makeApplicationDataDir()
{
	QString dir;
	dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	if (!QFileInfo(dir).isDir()) {
		QDir().mkpath(dir);
	}
	return dir;
}

QString makeApplicationStorageDir()
{
	QString dir;
	dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
	if (!QFileInfo(dir).isDir()) {
		QDir().mkpath(dir);
	}
	return dir;
}

MySettings::MySettings(QObject *)
	: QSettings(joinpath(makeApplicationDataDir(), qApp->applicationName() + ".ini"), QSettings::IniFormat)
{
//	qDebug() << (joinpath(makeApplicationDataDir(), qApp->applicationName() + ".ini"));
}


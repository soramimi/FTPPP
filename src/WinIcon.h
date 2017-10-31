#ifndef WINICON_H
#define WINICON_H

#include <QIcon>
#include <QString>

class WinIcon {
public:
	static QIcon iconFromExtensionLarge(const QString &ext);
	static QIcon iconFromExtensionSmall(const QString &ext);
};

#endif // WINICON_H

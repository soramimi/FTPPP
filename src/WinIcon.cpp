#include "WinIcon.h"

#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <commoncontrols.h>
#include <QtWinExtras/QtWinExtras>

namespace {
QIcon iconFromExtension_(QString const &ext, UINT flag)
{
	QIcon icon;
	QString name = "*." + ext;
	SHFILEINFOW shinfo;
	if (SHGetFileInfoW((wchar_t const *)name.utf16(), 0, &shinfo, sizeof(shinfo), flag | SHGFI_ICON | SHGFI_USEFILEATTRIBUTES) != 0) {
		QPixmap pm = QtWin::fromHICON(shinfo.hIcon);
		if (!pm.isNull()) {
			icon = QIcon(pm);
		}
	}
	return icon;
}
}

QIcon WinIcon::iconFromExtensionLarge(QString const &ext)
{
	return iconFromExtension_(ext, SHGFI_LARGEICON);
}

QIcon WinIcon::iconFromExtensionSmall(QString const &ext)
{
	return iconFromExtension_(ext, SHGFI_SMALLICON);
}

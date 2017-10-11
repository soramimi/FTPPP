#include "MainWindow.h"
#include "LegacyWindowsStyleTreeControl.h"
#include <QApplication>
#include <QProxyStyle>
#include "main.h"

bool start_with_shift_key = false;

class MyStyle : public QProxyStyle {
private:
	LegacyWindowsStyleTreeControl legacy_windows_;
public:
	MyStyle()
		: QProxyStyle(0)
	{
	}
	void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const
	{
		if (element == QStyle::PE_IndicatorBranch) {
			if (legacy_windows_.drawPrimitive(element, option, painter, widget)) {
				return;
			}
		}
		QProxyStyle::drawPrimitive(element, option, painter, widget);
	}
};

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setOrganizationName(ORGANIZTION_NAME);
	a.setApplicationName(APPLICATION_NAME);

	QStyle *style = new MyStyle();
	QApplication::setStyle(style);

	if (QApplication::queryKeyboardModifiers() & Qt::ShiftModifier) {
		start_with_shift_key = true;
	}

	MainWindow w;
	w.show();

	return a.exec();
}

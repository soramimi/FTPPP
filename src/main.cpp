#include "MainWindow.h"
#include "LegacyWindowsStyleTreeControl.h"
#include <QApplication>
#include <QProxyStyle>

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
	QStyle *style = new MyStyle();
	QApplication::setStyle(style);

	MainWindow w;
	w.show();

	return a.exec();
}

#include "LegacyWindowsStyleTreeControl.h"
#include <QStyleOption>
#include <QPainter>

static unsigned char plus_image[9][9] = {
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
	0x80, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x80,
	0x80, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x80,
	0x80, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x80,
	0x80, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x80,
	0x80, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x80,
	0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
};

static unsigned char minus_image[9][9] = {
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
	0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
	0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
	0x80, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x80,
	0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
	0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
	0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
};

LegacyWindowsStyleTreeControl::LegacyWindowsStyleTreeControl()
{
	// create brush
    {
        QImage img(2, 2, QImage::Format_RGB32);
        QRgb *p;
        QRgb t = qRgb(0xff, 0xff, 0xff);
        QRgb u = qRgb(0x80, 0x80, 0x80);
        p = (QRgb *)img.scanLine(0);
        p[0] = u;
        p[1] = t;
        p = (QRgb *)img.scanLine(1);
        p[0] = t;
        p[1] = u;
        br_branch = QBrush(img);
    }

    // create open/close buttons
	QImage img_plus(9, 9, QImage::Format_RGB32);
	QImage img_minus(9, 9, QImage::Format_RGB32);
	for (int y = 0; y < 9; y++) {
		for (int x = 0; x < 9; x++) {
			unsigned char v;
			v = plus_image[y][x];
			((QRgb *)img_plus.scanLine(y))[x] = qRgb(v, v, v);
			v = minus_image[y][x];
			((QRgb *)img_minus.scanLine(y))[x] = qRgb(v, v, v);
		}
	}
	pm_plus = QPixmap::fromImage(img_plus);
	pm_minus = QPixmap::fromImage(img_minus);
}

bool LegacyWindowsStyleTreeControl::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *) const
{
	if (element == QStyle::PE_IndicatorBranch) {
		int ox = option->rect.x() + option->rect.width() / 2;
		int oy = option->rect.y() + option->rect.height() / 2;
		if (option->state & QStyle::State_Item) {
			painter->fillRect(ox, oy, option->rect.right() - ox + 1, 1, br_branch);
		}
		if (option->state & QStyle::State_Sibling) {
			painter->fillRect(ox, oy, 1, option->rect.bottom() - oy + 1, br_branch);
		}
		if (option->state & (QStyle::State_Open | QStyle::State_Children | QStyle::State_Item | QStyle::State_Sibling)) {
			painter->fillRect(ox, option->rect.y(), 1, oy - option->rect.y(), br_branch);
		}
		if (option->state & QStyle::State_Children) {
			if (option->state & QStyle::State_Open) {
				painter->drawPixmap(ox - 4, oy - 4, pm_minus);
			} else {
				painter->drawPixmap(ox - 4, oy - 4, pm_plus);
			}
		}
		return true;
	}
	return false;
}

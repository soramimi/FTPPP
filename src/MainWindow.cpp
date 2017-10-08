#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ftplib.h"
#include <functional>
#include <QBuffer>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
}

MainWindow::~MainWindow()
{
	delete ui;
}

QStringList splitLines(QByteArray const &ba, std::function<QString(char const *ptr, size_t len)> tos)
{
	QStringList list;
	char const *begin = ba.data();
	char const *end = begin + ba.size();
	char const *ptr = begin;
	char const *left = ptr;
	while (1) {
		ushort c = 0;
		if (ptr < end) {
			c = *ptr;
		}
		if (c == '\n' || c == '\r' || c == 0) {
			list.push_back(tos(left, ptr - left));
			if (c == 0) break;
			if (c == '\n') {
				ptr++;
			} else if (c == '\r') {
				ptr++;
				if (ptr < end && *ptr == '\n') {
					ptr++;
				}
			}
			left = ptr;
		} else {
			ptr++;
		}
	}
	return list;
}

class FileItem {
public:
	QString name;
	bool isdir = false;
	QDateTime datetime;
	uint64_t size;
	QString type;
	int attr = 0;
	QString owner;
public:
	bool parseList(QString const &line)
	{
		QStringList vals;
		ushort const *begin = line.utf16();
		ushort const *end = begin + line.size();
		ushort const *left = begin;
		ushort const *ptr = begin;
		while (vals.size() < 8) {
			ushort c = 0;
			if (ptr < end) {
				c = *ptr;
			}
			if (c == 0 || c == ' ') {
				if (left < ptr) {
					QString s = QString::fromUtf16(left, ptr - left);
					vals.push_back(s);
				}
				if (c == 0) break;
				ptr++;
				left = ptr;
			} else {
				ptr++;
			}
		}
		while (ptr < end && *ptr == ' ') {
			ptr++;
		}
		vals.push_back(QString::fromUtf16(ptr));

		if (vals.size() == 9) {
			QString s = vals[8];
			if (s.isEmpty()) return false;
			if (s == ".") return false;
			name = s;

			s = vals[0];
			if (s.size() >= 10) {
				isdir = (s[0] == 'd');
				if (s[1] == 'r') attr |= 0400;
				if (s[2] == 'w') attr |= 0200;
				if (s[3] == 'x') attr |= 0100;
				if (s[4] == 'r') attr |= 0040;
				if (s[5] == 'w') attr |= 0020;
				if (s[6] == 'x') attr |= 0010;
				if (s[7] == 'r') attr |= 0004;
				if (s[8] == 'w') attr |= 0002;
				if (s[9] == 'x') attr |= 0001;
			}

			owner = vals[2];
			s = vals[4];
			size = strtoull(s.toStdString().c_str(), 0, 10);

			QString months = "JanFebMarAprMayJunJulAugSepOctNovDec";
			int month = months.indexOf(vals[5]);
			if (month >= 0) month = month / 3 + 1;

			s = vals[6];
			int day = s.toInt();

			int year = 0;
			int hour = 0;
			int minute = 0;

			s =  vals[7];
			if (s.indexOf(':')) {
				if (sscanf(s.toStdString().c_str(), "%u:%u", &hour, &minute) != 2) {
					hour = minute = 0;
				}
			} else {
				year = atoi(s.toStdString().c_str());
			}

			if (year == 0) {
				year = QDateTime::currentDateTime().date().year();
			}

			datetime = QDateTime(QDate(year, month, day), QTime(hour, minute, 0));

			return true;
		}
		return false;
	}
	QString attrString() const
	{
		char tmp[10] = "---------";
		if (attr & 0400) tmp[0] = 'r';
		if (attr & 0200) tmp[1] = 'w';
		if (attr & 0100) tmp[2] = 'x';
		if (attr & 0040) tmp[3] = 'r';
		if (attr & 0020) tmp[4] = 'w';
		if (attr & 0010) tmp[5] = 'x';
		if (attr & 0004) tmp[6] = 'r';
		if (attr & 0002) tmp[7] = 'w';
		if (attr & 0001) tmp[8] = 'x';
		return tmp;
	}
	QString datetimeString() const
	{
		char tmp[100];
		sprintf(tmp, "%04u-%02u-%02u %02u:%02u"
				, datetime.date().year()
				, datetime.date().month()
				, datetime.date().day()
				, datetime.time().hour()
				, datetime.time().minute()
				);
		return tmp;
	}
	QString typeString() const
	{
		if (!isdir) {
			int i = name.lastIndexOf('.');
			if (i > 0) {
				return name.mid(i + 1);
			}
		}
		return QString();
	}
};

void MainWindow::on_pushButton_clicked()
{
	QBuffer b;
	if (b.open(QBuffer::WriteOnly)) {
		ftplib ftp;
		ftp.connect("ftp.jaist.ac.jp");
		ftp.login("anonymous", "foo@example.com");
		ftp.dir(&b, "/pub/qtproject");
		ftp.quit();
	}
	QStringList lines = splitLines(b.buffer(), [](char const *ptr, size_t len){
		return QString::fromUtf8(ptr, len);
	});

	QList<FileItem> files;
	for (int row = 0; row < lines.size(); row++) {
		QString const &line = lines[row];
		FileItem t;
		if (t.parseList(line)) {
			files.push_back(t);
		}
	}

	QStringList cols = {
		tr("Name"),
		tr("Time"),
		tr("Size"),
		tr("Type"),
		tr("Attribute"),
		tr("Owner"),
	};


	ui->tableWidget_remote->clear();
	ui->tableWidget_remote->setColumnCount(cols.size());
	ui->tableWidget_remote->setRowCount(files.size());
	ui->tableWidget_remote->setShowGrid(false);
	ui->tableWidget_remote->horizontalHeader()->setStretchLastSection(true);
	ui->tableWidget_remote->verticalHeader()->setVisible(false);

	for (int col = 0; col < cols.size(); col++) {
		QTableWidgetItem *item = new QTableWidgetItem();
		item->setText(cols[col]);
		ui->tableWidget_remote->setHorizontalHeaderItem(col, item);
	}

	for (int row = 0; row < files.size(); row++) {
		ui->tableWidget_remote->setRowHeight(row, 20);
		FileItem const &t = files[row];
		int col = 0;
		auto AddColumn = [&](QString const &text, bool alignright){
			QTableWidgetItem *item = new QTableWidgetItem();
			item->setText(text);
			if (alignright) {
				item->setTextAlignment(Qt::AlignRight);
			}
			ui->tableWidget_remote->setItem(row, col, item);
			col++;
		};
		AddColumn(t.name, false);
		AddColumn(t.datetimeString(), false);
		AddColumn(t.isdir ? QString::fromLatin1("<DIR>") : QString::number(t.size), true);
		AddColumn(t.typeString(), false);
		AddColumn(t.attrString(), false);
		AddColumn(t.owner, false);
	}
	ui->tableWidget_remote->resizeColumnsToContents();
}

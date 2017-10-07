#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ftplib.h"
#include <functional>
#include <QBuffer>

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

void MainWindow::on_pushButton_clicked()
{
	QBuffer b;
	if (b.open(QBuffer::WriteOnly)) {
		ftplib ftp;
		ftp.Connect("ftp.jaist.ac.jp");
		ftp.Login("anonymous", "foo@example.com");
		ftp.Dir(&b, "/pub/qtproject");
		ftp.Quit();
	}
	QStringList lines = splitLines(b.buffer(), [](char const *ptr, size_t len){
		return QString::fromUtf8(ptr, len);
	});

	ui->tableWidget_remote->clear();
	ui->tableWidget_remote->setColumnCount(1);
	ui->tableWidget_remote->setRowCount(lines.size());
	for (int row = 0; row < lines.size(); row++) {
		QString const &line = lines[row];
		QTableWidgetItem *item = new QTableWidgetItem();
		item->setText(line);
		ui->tableWidget_remote->setItem(row, 0, item);
	}
}

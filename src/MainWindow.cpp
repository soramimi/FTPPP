#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <functional>
#include <QBuffer>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <set>
#include <QFileIconProvider>
#include "MySettings.h"
#include "SettingsDialog.h"
#include "joinpath.h"


extern bool start_with_shift_key;


class FileItem;
typedef QList<FileItem> FileItemList;

enum ItemRole {
	Item_Path = Qt::UserRole,
};

struct MainWindow::Private {
	int timer_counter = 0;
	int ftp_refresh_counter = 0;
	std::set<QString> feat;
	std::map<QString, FileItemList> filesmap;
	QString current_path;
	FTP ftp;
	QFileIconProvider icon_provider;
};

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, m(new Private)
{
	ui->setupUi(this);
	ui->splitter->setSizes({100,300});

	if (!start_with_shift_key) {
		Qt::WindowStates state = windowState();
		MySettings settings;

		settings.beginGroup("MainWindow");
		bool maximized = settings.value("Maximized").toBool();
		restoreGeometry(settings.value("Geometry").toByteArray());
		ui->splitter->restoreState(settings.value("SplitterState").toByteArray());
		settings.endGroup();
		if (maximized) {
			state |= Qt::WindowMaximized;
			setWindowState(state);
		}
	}

	startTimer(10);
}

MainWindow::~MainWindow()
{
	m->ftp.close();
	delete m;
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
	QString symlink;
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

FTP &MainWindow::ftp()
{
	return m->ftp;
}

void MainWindow::updateFeature()
{
	m->feat.clear();

	QBuffer b;
	if (b.open(QBuffer::WriteOnly)) {
		ftp().feat(&b);
	}
	QStringList lines = splitLines(b.buffer(), [](char const *ptr, size_t len){
		return QString::fromUtf8(ptr, len).trimmed();
	});
	for (QString const &line : lines) {
		QString s = line;
		int i = s.indexOf(' ');
		if (i >= 0) {
			s = s.mid(0, i);
		}
		if (!s.isEmpty()) {
			m->feat.insert(s);
//			qDebug() << s;
		}
	}
}

bool MainWindow::queryFeatureAvailable(QString const &name)
{
	auto it = m->feat.find(name);
	return it != m->feat.end();
}

void MainWindow::fetchList(QString const &path)
{
	bool mlsd = queryFeatureAvailable("MLST") || queryFeatureAvailable("MLSD");

	QBuffer b;
	if (b.open(QBuffer::WriteOnly)) {
		std::string d = path.toStdString();
		if (mlsd) {
			ftp().mlsd(&b, d.c_str());
		} else {
			ftp().dir(&b, d.c_str());
		}
	}

	QList<FileItem> files;

	if (mlsd) {
		QByteArray ba = b.buffer();
		if (!ba.isEmpty()) {
			std::vector<std::string> list;
			char const *begin = ba.data();
			char const *end = begin + ba.size();
			char const *left = begin;
			char const *ptr = begin;
			while (1) {
				int c = 0;
				if (ptr < end) {
					c = *ptr & 0xff;
				}
				if (c == 0 || c == ';' || c == '\n' || c == '\r') {
					if (left < ptr) {
						std::string s(left, ptr - left);
						list.push_back(s);
					}
					if (c != ';') {
						int n = (int)list.size();
						if (n > 0) {
							FileItem item;
							n--;
							char const *p = list[n].c_str();
							if (*p == ' ') {
								item.name = p + 1;
							}
							if (!item.name.isEmpty() && item.name != "." && item.name != "..") {
								for (int i = 0; i < n; i++) {
									char const *k = list[i].c_str();
									char const *eq = strchr(k, '=');
									if (eq) {
										std::string key(k, eq - k);
										std::string val(eq + 1);
										if (key == "modify") {
											int year, month, day, hour, minute;
											if (sscanf(val.c_str(), "%04u%02u%02u%02u%02u", &year, &month, &day, &hour, &minute) == 5) {
												item.datetime = QDateTime(QDate(year, month, day), QTime(hour, minute, 0));
											}
										} else if (key == "type") {
											item.isdir = (val == "dir" || val == "pdir");
										} else if (key == "size") {
											item.size = strtoull(val.c_str(), 0, 10);
										} else if (key == "UNIX.mode") {
											item.attr = strtoul(val.c_str(), 0, 8);
										} else if (key == "UNIX.owner") {
											item.owner = QString::fromStdString(val);
										}
									}
								}
								files.push_back(item);
							}
						}
						list.clear();
					}
					if (c == 0) break;
					ptr++;
					left = ptr;
				} else {
					ptr++;
				}
			}
		}
	} else {
		QStringList lines = splitLines(b.buffer(), [](char const *ptr, size_t len){
			return QString::fromUtf8(ptr, len);
		});
		for (int row = 0; row < lines.size(); row++) {
			QString const &line = lines[row];
			FileItem item;
			if (item.parseList(line) && item.name != "." && item.name != "..") {
				int i = item.name.indexOf(" -> ");
				if (i == 0) {
					// skip
				} else {
					if (i > 0) {
						item.symlink = item.name.mid(i + 4);
						item.name = item.name.mid(0, i);
					}
					files.push_back(item);
				}
			}
		}
	}

	std::sort(files.begin(), files.end(), [](FileItem const &a, FileItem const &b){
		if (a.isdir && !b.isdir) return true;
		if (!a.isdir && b.isdir) return false;
		return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
	});

	QStringList subdirs;
	for (FileItem const &item : files) {
		if (item.isdir) {
			subdirs.push_back(item.name);
		}
	}

	m->filesmap[path] = files;
	updateFilesView(path);
	updateTreeView(path, &subdirs);
}

void MainWindow::updateTreeView(QString const &path, QStringList const *children)
{
	QStringList list;
	if (path.isEmpty() || path == "/") {
		list.push_back("/");
	} else {
		if (path.utf16()[0] != '/') return;
		list = path.split('/');
		list[0] = "/";
	}

	QTreeWidgetItem *item = ui->treeWidget_remote->topLevelItem(0);
	if (!item) return;

	QString s;

	for (int i = 0; i < list.size(); i++) {
		item->setExpanded(true);
		for (int j = 0; j < item->childCount(); j++) {
			QTreeWidgetItem *child = item->child(j);
			if (child->text(0) == list[i]) {
				item = child;
				if (item->childCount() == 1) {
					child = item->child(0);
					if (child->text(0).isEmpty()) {
						item->removeChild(child);
					}
				}
				goto next;
			}
		}
		{
			QTreeWidgetItem *newitem = new QTreeWidgetItem();
			newitem->setText(0, list[i]);
			newitem->setData(0, Item_Path, s / list[i]);
			newitem->setIcon(0, m->icon_provider.icon(QFileIconProvider::Folder));
			item->addChild(newitem);
			item = newitem;
		}
next:;
		s = s / list[i];
	}

	if (children) {
		for (QString const &subdir : *children) {
			QTreeWidgetItem *newitem = new QTreeWidgetItem();
			newitem->setText(0, subdir);
			newitem->setData(0, Item_Path, s / subdir);
			newitem->setIcon(0, m->icon_provider.icon(QFileIconProvider::Folder));
			QTreeWidgetItem *dummyitem = new QTreeWidgetItem();
			newitem->addChild(dummyitem);
			item->addChild(newitem);
		}
	}

	ui->treeWidget_remote->blockSignals(true);
	ui->treeWidget_remote->setCurrentItem(item);
	item->setExpanded(true);
	ui->treeWidget_remote->blockSignals(false);
}

void MainWindow::updateFilesView(QString const &path)
{
	auto it = m->filesmap.find(path);
	if (it == m->filesmap.end()) return;

	FileItemList const &files = it->second;

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
			if (col == 0) {
				item->setData(Item_Path, path / text);
			}
			ui->tableWidget_remote->setItem(row, col, item);
			col++;
			return item;
		};
		QFileInfo info(t.name);
		QTableWidgetItem *item = AddColumn(t.name, false);
		if (t.isdir) {
			item->setIcon(m->icon_provider.icon(QFileIconProvider::Folder));
		} else {
			item->setIcon(m->icon_provider.icon(QFileIconProvider::File));
		}
		AddColumn(t.datetimeString(), false);
		AddColumn(t.isdir ? QString::fromLatin1("<DIR>") : QString::number(t.size), true);
		AddColumn(t.typeString(), false);
		AddColumn(t.attrString(), false);
		AddColumn(t.owner, false);
	}
	ui->tableWidget_remote->resizeColumnsToContents();
}

void MainWindow::changeDir(QString const path)
{
	m->current_path = path;
	if (path.startsWith('/')) {
		auto it = m->filesmap.find(path);
		if (it == m->filesmap.end()) {
			fetchList(path);
		} else {
			updateFilesView(path);
			updateTreeView(path, nullptr);
		}
	}
}

void MainWindow::on_tableWidget_remote_itemDoubleClicked(QTableWidgetItem *)
{
	int row = ui->tableWidget_remote->currentRow();
	QTableWidgetItem *item = ui->tableWidget_remote->item(row, 0);
	if (!item) return;
	QString path = item->data(Item_Path).toString();
	fetchList(path);
}

void MainWindow::on_treeWidget_remote_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	if (!current) return;

	QString path = current->data(0, Item_Path).toString();
	qDebug() << path;
	changeDir(path);
}

void MainWindow::on_treeWidget_remote_itemExpanded(QTreeWidgetItem *item)
{
	if (item && item->isExpanded()) {
		QString path = item->data(0, Item_Path).toString();
		changeDir(path);
	}
}

bool MainWindow::connect_()
{
	QString server = "ftp.jaist.ac.jp";
	QString user = "anonymous";
	QString pass = "who@example.com";

	ftp().close();

	if (!ftp().open(server, user, pass)) return false;

	updateFeature();

	if (ui->treeWidget_remote->topLevelItemCount() == 0) {
		ui->treeWidget_remote->clear();
		QTreeWidgetItem *item = new QTreeWidgetItem();
		item->setText(0, server);
		ui->treeWidget_remote->addTopLevelItem(item);
	}

	return  true;
}

void MainWindow::on_pushButton_clicked()
{
	if (connect_()) {
		fetchList("/");
	}
}


void MainWindow::on_action_settings_triggered()
{
	SettingsDialog dlg(this);
	if (dlg.exec() == QDialog::Accepted) {

	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	setWindowOpacity(0);
	Qt::WindowStates state = windowState();
	bool maximized = (state & Qt::WindowMaximized) != 0;
	if (maximized) {
		state &= ~Qt::WindowMaximized;
		setWindowState(state);
	}
	{
		MySettings settings;

		settings.beginGroup("MainWindow");
		settings.setValue("Maximized", maximized);
		settings.setValue("Geometry", saveGeometry());
		settings.setValue("SplitterState", ui->splitter->saveState());
		settings.endGroup();
	}
	QMainWindow::closeEvent(event);
}

void MainWindow::timerEvent(QTimerEvent *event)
{
	m->timer_counter++;
	if (m->timer_counter >= 100) {
		m->timer_counter = 0;

		m->ftp_refresh_counter++;
		if (m->ftp_refresh_counter >= 30) {
			m->ftp_refresh_counter = 0;
			ftp().pwd();
		}
	}
}


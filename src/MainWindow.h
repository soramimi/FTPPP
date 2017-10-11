#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

namespace Ui {
class MainWindow;
}

class QTableWidgetItem;
class QTreeWidgetItem;

class ftplib;

typedef std::shared_ptr<ftplib> ftp_ptr;

class MainWindow : public QMainWindow
{
	Q_OBJECT
private:
	Ui::MainWindow *ui;

	struct Private;
	Private *m;

	void fetchList(const QString &path);

	bool queryFeatureAvailable(const QString &name);
	void updateFeature(ftp_ptr ftp);
	ftp_ptr connectFTP();
	void updateFilesView(const QString &path);
	void updateTreeView(const QString &path, const QStringList *children);
	void changeDir(const QString path);
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_pushButton_clicked();

	void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);
	void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void on_treeWidget_itemExpanded(QTreeWidgetItem *item);
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "FTP.h"

namespace Ui {
class MainWindow;
}

class QTableWidgetItem;
class QTreeWidgetItem;



class MainWindow : public QMainWindow
{
	Q_OBJECT
private:
	Ui::MainWindow *ui;

	struct Private;
	Private *m;

	void fetchList(const QString &path, bool expand_tree);

	bool queryFeatureAvailable(const QString &name);
	void updateFeature();
	bool connect_();
	void updateFilesView(const QString &path);
	void updateTreeView(const QString &path, const QStringList *children, bool expand_tree);
	void changeDir(const QString path, bool expand_tree);
	FTP &ftp();
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_pushButton_clicked();

	void on_tableWidget_remote_itemDoubleClicked(QTableWidgetItem *item);
	void on_treeWidget_remote_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void on_treeWidget_remote_itemExpanded(QTreeWidgetItem *item);
	void on_action_settings_triggered();

	// QWidget interface
protected:
	void closeEvent(QCloseEvent *event);

	// QObject interface
protected:
	void timerEvent(QTimerEvent *event);
};

#endif // MAINWINDOW_H

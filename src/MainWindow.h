#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

namespace Ui {
class MainWindow;
}

class ftplib;

typedef std::shared_ptr<ftplib> ftp_ptr;

class MainWindow : public QMainWindow
{
	Q_OBJECT
private:
	Ui::MainWindow *ui;

	struct Private;
	Private *m;

	void fetchList();

	bool queryFeatureAvailable(const QString &name);
	void updateFeature();
	ftp_ptr connectFTP();
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_pushButton_clicked();

};

#endif // MAINWINDOW_H

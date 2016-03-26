#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "kdevice.h"
#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow(void);

	void linkDev(std::shared_ptr<kDevice> devc);

signals:
	void ctlReconnect(void);

public slots:
	void devInfo(int8_t id, QString what);
	void devBtnScreenKeyDown(uint8_t keyNum);
	void devBtnScreenKeyUp(uint8_t keyNum);

	void ctlManuallyConnect(void);
	void ctlProgress(unsigned int value);
	void ctlReady(void);
	void ctlStop(QString reason);

private:
	Ui::MainWindow *ui;

	std::shared_ptr<kDevice> dev;
};

#endif // MAINWINDOW_H

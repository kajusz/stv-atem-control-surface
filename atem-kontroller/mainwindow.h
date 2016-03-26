#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "kdevice.h"

#include "libatem.h"

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
	// slots for the kdevice class
	void devInfo(int8_t id, QString what);
	void devBtnScreenKeyDown(uint8_t keyNum);
	void devBtnScreenKeyUp(uint8_t keyNum);

	// slots for the control class
	void ctlManuallyConnect(void);
	void ctlProgress(unsigned int value);
	void ctlReady(void);
	void ctlStop(QString reason);

private:
	// qt designer object
	Ui::MainWindow *ui;

	// reference to the kontroller
	std::shared_ptr<kDevice> dev;
};

#endif // MAINWINDOW_H

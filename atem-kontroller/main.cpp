#include "mainwindow.h"

#include <QApplication>
#include <QSettings>

#include "control.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	// Needed for QSettings
	QCoreApplication::setOrganizationName("TheD");
	QCoreApplication::setOrganizationDomain("kajusz.me");
	QCoreApplication::setApplicationName("atem-kontroller");

	std::shared_ptr<kDevice> d(new kDevice);
	std::shared_ptr<QAtemConnection> atem(new QAtemConnection);

	w.linkDev(d);
	control c(d);
	c.registerAtem(atem);

	QObject::connect(&c, &control::mwwManuallyConnect, &w, &MainWindow::ctlManuallyConnect);
	QObject::connect(&c, &control::mwwProgress, &w, &MainWindow::ctlProgress);
	QObject::connect(&c, &control::mwwReady, &w, &MainWindow::ctlReady);
	QObject::connect(&c, &control::mwwStop, &w, &MainWindow::ctlStop);
	QObject::connect(&w, &MainWindow::ctlReconnect, &c, &control::mwwConnect);

	QObject::connect(d.get(), &kDevice::sigConnected, &c, &control::devConnected);
	QObject::connect(d.get(), &kDevice::sigDisconnected, &c, &control::devDisconnected);
//	QObject::connect(d.get(), &kDevice::sigBtnCommandKeyDown, &c, &control::devBtnCommandKeyDown);
	QObject::connect(d.get(), &kDevice::sigBtnCommandKeyUp, &c, &control::devBtnCommandKeyUp);
//	QObject::connect(d.get(), &kDevice::sigBtnGroupKeyDown, &c, &control::devBtnGroupKeyDown);
	QObject::connect(d.get(), &kDevice::sigBtnGroupKeyUp, &c, &control::devBtnGroupKeyUp);
	QObject::connect(d.get(), &kDevice::sigTBarMove, &c, &control::devTBarMove);
	QObject::connect(d.get(), &kDevice::sigEncMove, &c, &control::devEncMove);
	QObject::connect(d.get(), &kDevice::sigJoystickMove, &c, &control::devJoystickMove);

//	void sigBtnKeyboardKeyDown(uint8_t rawCode);
//	void sigBtnKeyboardKeyUp(uint8_t rawCode);

	QObject::connect(d.get(), &kDevice::sigInfo, &w, &MainWindow::devInfo);
	QObject::connect(d.get(), &kDevice::sigBtnScreenKeyDown, &w, &MainWindow::devBtnScreenKeyDown);

	c.doTasks();

	return a.exec();
}

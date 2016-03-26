#include "mainwindow.h"

#include <QApplication>
#include <QSettings>

#include "control.h"

#include "libatem.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	// Needed for QSettings
	QCoreApplication::setOrganizationName("TheD");
	QCoreApplication::setOrganizationDomain("kajusz.me");
	QCoreApplication::setApplicationName("atem-kontroller");

	// create kontroller object
	std::shared_ptr<kDevice> d(new kDevice);

	// setup signals & slots, kdevice and mainwindow
	w.linkDev(d);

	// create control object, setup signals & slots
	control c(d);

	// setup signals & slots, control and mainwindow
	QObject::connect(&c, &control::mwwManuallyConnect, &w, &MainWindow::ctlManuallyConnect);
	QObject::connect(&c, &control::mwwProgress, &w, &MainWindow::ctlProgress);
	QObject::connect(&c, &control::mwwReady, &w, &MainWindow::ctlReady);
	QObject::connect(&c, &control::mwwStop, &w, &MainWindow::ctlStop);
	QObject::connect(&w, &MainWindow::ctlReconnect, &c, &control::mwwConnect);

	// create atem objects
	QSettings settings;
	for (int i = 0; i < settings.value(QString("atem/count"), 1).toInt(); ++i)
	{
		atemPtr atem(new QAtemConnection);
		c.registerAtem(atem);
	}

//	void sigBtnKeyboardKeyDown(uint8_t rawCode);
//	void sigBtnKeyboardKeyUp(uint8_t rawCode);

	// connect to objects
	c.doTasks();

	return a.exec();
}

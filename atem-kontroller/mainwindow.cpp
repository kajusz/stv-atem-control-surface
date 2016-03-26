#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->p0pBar->setValue(0);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::linkDev(std::shared_ptr<kDevice> devc)
{
	dev = devc;
}

void MainWindow::devInfo(int8_t id, QString what)
{
//	ui->p0text->append("kDevice: " << id << ": " << what);
}

void MainWindow::devBtnScreenKeyDown(uint8_t keyNum)
{
	qDebug() << "ScreenKeyD " << keyNum;
}

void MainWindow::devBtnScreenKeyUp(uint8_t keyNum)
{
	qDebug() << "ScreenKeyU " << keyNum;
}

void MainWindow::ctlManuallyConnect(void)
{
	QSettings settings;

	std::vector<device_t> q = dev->getConfig();
	QStringList items;
	QString str;
	for (std::vector<device_t>::const_iterator it = q.begin(); it != q.end(); ++it)
	{
		str = it->name.c_str();
		items << str;
	}

	bool ok;
	QString item = QInputDialog::getItem(this, "Please choose a device", "Please choose a device", items, 0, false, &ok);
	if (ok && !item.isEmpty())
	{
		for (std::vector<device_t>::const_iterator it = q.begin(); it != q.end(); ++it)
		{
			if (item.compare(it->name.c_str()) == 0)
			{
				device_t tq = *it;
				settings.setValue("dev/devHid", tq.data.c_str());
				settings.setValue("dev/devName", tq.name.c_str());

				emit ctlReconnect();
				return;
			}
		}
	}
	else
	{
		// QUIT
	}
}

void MainWindow::ctlProgress(unsigned int value)
{
	ui->p0pBar->setValue(value);
}

void MainWindow::ctlReady(void)
{
	ui->stack->setCurrentIndex(1);
}

void MainWindow::ctlStop(QString reason)
{
	ui->stack->setCurrentIndex(0);
	ui->p0text->append(reason);
}

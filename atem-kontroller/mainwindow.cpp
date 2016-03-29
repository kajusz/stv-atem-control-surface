#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QNetworkInterface>
#include <QtDebug>

// ctor
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// timer on home page
	updateTime();
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
	timer->start(1000);
}

// dtor
MainWindow::~MainWindow()
{
	delete ui;
}

// setup signals and slots
void MainWindow::linkDev(std::shared_ptr<kDevice> devc)
{
	dev = devc;
	connect(dev.get(), &kDevice::sigInfo, this, &MainWindow::devInfo);
	connect(dev.get(), &kDevice::sigBtnScreenKeyDown, this, &MainWindow::devBtnScreenKeyDown);
}

// add atem object to the container
void MainWindow::registerAtem(atemPtr atem)
{
	bmd.push_back(atem);
}

// logging for kontroller
void MainWindow::devInfo(int8_t id, QString what)
{
	ui->p0text->append(QString("kDevice: %1: %2").arg(id).arg(what));
}

// buttons !
void MainWindow::devBtnScreenKeyDown(uint8_t keyNum)
{
	qDebug() << "ScreenKeyD " << keyNum;
}

void MainWindow::devBtnScreenKeyUp(uint8_t keyNum)
{
	qDebug() << "ScreenKeyU " << keyNum;
}

// no kontroller stored in settings, so choose one
void MainWindow::ctlManuallyConnect(void)
{
	QSettings settings;

	std::vector<device_t> q = dev->getConfig();
	QStringList items;
	for (std::vector<device_t>::const_iterator it = q.begin(); it != q.end(); ++it)
		items << QString(it->name.c_str());

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
		QCoreApplication::quit();
	}
}

// do something with the progress bar
void MainWindow::ctlProgress(unsigned int value)
{
	ui->p0pBar->setValue(value);
}

// go to home page
void MainWindow::ctlReady(void)
{
	for (atemVec::const_iterator it = bmd.begin(); it != bmd.end(); ++it)
	{
		atemPtr ti = *it;
		ui->atm1ip->setText(ti->address().toString());
		ui->atm1model->setText(ti->productInformation());
		ui->atm1version->setText(QString("%1.%2").arg(ti->majorVersion()).arg(ti->minorVersion()));
	}

	QList<QHostAddress> list = QNetworkInterface::allAddresses();
	for(int i = 0 ; i < list.count(); i++)
	{
		ui->labKip->setText("127.0.0.1");
		if(!list[i].isLoopback() && list[i].protocol() == QAbstractSocket::IPv4Protocol && (list[i].toString().startsWith("192") || list[i].toString().startsWith("10")))
		{
			ui->labKip->setText(list[i].toString());
			break;
		}
	}

	ui->labKsoft->setText(KDEVICE_VERSION);
	ui->labKhard->setText(dev->getVersion().c_str());

	ui->stack->setCurrentIndex(1);
}

// go to loading page, usuaally because of a fatal error
void MainWindow::ctlStop(QString reason)
{
	ui->stack->setCurrentIndex(0);
	ui->p0text->append(reason);
}

void MainWindow::updateTime()
{
	ui->p1Timer->display(QTime::currentTime().toString("HH:mm:ss"));
}

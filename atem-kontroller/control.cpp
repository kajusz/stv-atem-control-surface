#include "control.h"

#include <QSettings>
#include <QtDebug>

#define TASKS 2

control::control(std::shared_ptr<kDevice> devc)
{
	dev = devc;

}
control::~control(void)
{

}

void control::registerAtem(std::shared_ptr<QAtemConnection> atem)
{
	bmd.push_back(atem);
}

void control::doTasks(void)
{
	iconnect();

	QSettings settings;
	int x = 0;
	for (std::vector<std::shared_ptr<QAtemConnection>>::const_iterator it = bmd.begin(); it != bmd.end(); ++it, ++x)
	{
		std::shared_ptr<QAtemConnection> ti = *it;

		connect(ti.get(), &QAtemConnection::connected, [=]() { this->atmConnected(x); });
		connect(ti.get(), &QAtemConnection::disconnected, [=]() { this->atmDisconnected(x); });

		ti->connectToSwitcher(QHostAddress(settings.value(QString("atem/%1/ip").arg(x), "192.168.10.240").toString()));
		if (ti->isConnected())
			incrementalProgress();
	}

}

void control::mwwConnect(void)
{
	iconnect();
}

void control::devConnected(void)
{
	incrementalProgress();
}

void control::devDisconnected(void)
{

}

void control::devBtnCommandKeyDown(uint8_t keyNum)
{
	qDebug() << "CommandKeyD " << keyNum;
}

void control::devBtnCommandKeyUp(uint8_t keyNum)
{
	qDebug() << "CommandKeyU " << keyNum;
}

void control::devBtnGroupKeyDown(uint8_t gpId, uint8_t keyNum)
{
	qDebug() << "Group " << gpId << " KeyD " << keyNum;
}

void control::devBtnGroupKeyUp(uint8_t gpId, uint8_t keyNum)
{
	qDebug() << "Group " << gpId << " KeyU " << keyNum;
}

void control::devTBarMove(uint16_t pos)
{
	qDebug() << "TBar " << pos;
}

void control::devEncMove(uint8_t encId, int8_t data)
{
	qDebug() << "Enc " << encId << " " << data;
}

void control::devJoystickMove(joystick_t data)
{
	qDebug() << "Joystick x " << data.x << " y " << data.y << " z " << data.z;
}

void control::atmConnected(atemId id)
{
	qDebug() << "Connected " << id;
}

void control::atmDisconnected(atemId id)
{
	qDebug() << "Disconnected " << id;
}

void control::atmProgramInputChanged(atemId id, quint8 me, quint16 oldIndex, quint16 newIndex)
{
	qDebug() << "pgm ch " << id << " me " << me << " oldIndex " << oldIndex << " newIndex " << newIndex;
}

void control::atmPreviewInputChanged(atemId id, quint8 me, quint16 oldIndex, quint16 newIndex)
{
	qDebug() << "pgm ch " << id << " me " << me << " oldIndex " << oldIndex << " newIndex " << newIndex;
}



void control::iconnect(void)
{
	QSettings settings;

	if (settings.value("dev/kontroller", true).toBool())
	{
		std::vector<device_t> q = dev->getConfig();
		std::string storedDevHid = settings.value("dev/devHid", "__INVALID__").toString().toStdString();
		std::string storedDevName = settings.value("dev/devName", "__INVALID__").toString().toStdString();

		if (storedDevHid.compare("__INVALID__") == 0)
		{
			qDebug() << "First run";
			emit mwwManuallyConnect();
			return;
		}

		bool foundDevice = false;
		device_t foundDeviceDevT;

		for (std::vector<device_t>::iterator it = q.begin(); it != q.end(); ++it)
			if (storedDevHid.compare(it->data) == 0)
				if (storedDevName.compare(it->name) == 0)
				{
					foundDevice = true;
					foundDeviceDevT = *it;
					break;
				}

		if (foundDevice)
		{
			qDebug() << "Attempting to connect";
			dev->setConfig(foundDeviceDevT);
			dev->begin();
		}
		else
		{
			qDebug() << "Failed to find saved device";
			emit mwwManuallyConnect();
		}
	}
}

void control::incrementalProgress(void)
{
	emit mwwProgress((100/TASKS)*(++tasks));
}

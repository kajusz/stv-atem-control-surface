#include "control.h"

#include <QSettings>
#include <QtDebug>

#include "qatemmixeffect.h"

control::control(std::shared_ptr<kDevice> devc)
{
	dev = devc;
	connect(dev.get(), &kDevice::sigConnected, this, &control::devConnected);
	connect(dev.get(), &kDevice::sigDisconnected, this, &control::devDisconnected);
}
control::~control(void)
{
	for (atemVec::const_iterator it = bmd.begin(); it != bmd.end(); ++it)
	{
		atemPtr ti = *it;
		ti->disconnectFromSwitcher();
	}
}

void control::registerAtem(atemPtr atem)
{
	bmd.push_back(atem);
}

void control::doTasks(void)
{
	// connect to kontroller
	mwwConnect();

	// connect to atems
	QSettings settings;
	totalTasks = 1 + settings.value(QString("atem/count"), 1).toInt();
	int x = 0;
	for (atemVec::const_iterator it = bmd.begin(); it != bmd.end(); ++it, ++x)
	{
		atemPtr ti = *it;

		connect(ti.get(), &QAtemConnection::connected, [=]() { this->atmConnected(x); });
		connect(ti.get(), &QAtemConnection::disconnected, [=]() { this->atmDisconnected(x); });

		ti->connectToSwitcher(QHostAddress(settings.value(QString("atem/%1/ip").arg(x), "192.168.10.240").toString()));
		if (ti->isConnected())
			incrementalProgress();
	}
}

void control::mwwConnect(void)
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

void control::devConnected(void)
{
//	connect(dev.get(), &kDevice::sigBtnCommandKeyDown, this, &control::devBtnCommandKeyDown);
	connect(dev.get(), &kDevice::sigBtnCommandKeyUp, this, &control::devBtnCommandKeyUp);
//	connect(dev.get(), &kDevice::sigBtnGroupKeyDown, this, &control::devBtnGroupKeyDown);
	connect(dev.get(), &kDevice::sigBtnGroupKeyUp, this, &control::devBtnGroupKeyUp);
	connect(dev.get(), &kDevice::sigTBarMove, this, &control::devTBarMove);
	connect(dev.get(), &kDevice::sigEncMove, this, &control::devEncMove);
	connect(dev.get(), &kDevice::sigJoystickMove, this, &control::devJoystickMove);

	incrementalProgress();
}

void control::devDisconnected(void)
{
	emit mwwStop("control: kontroller disconnected");
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
	QAtemMixEffect *ame = bmd.at(id)->mixEffect(0);

	if(ame)
	{
		connect(ame, &QAtemMixEffect::programInputChanged, [=](quint8 me, quint16 oldIndex, quint16 newIndex) { this->atmProgramInputChanged(id, me, oldIndex, newIndex); });
		connect(ame, &QAtemMixEffect::previewInputChanged, [=](quint8 me, quint16 oldIndex, quint16 newIndex) { this->atmPreviewInputChanged(id, me, oldIndex, newIndex); });
	}

	incrementalProgress();
}

void control::atmDisconnected(atemId id)
{
	emit mwwStop(QString("control: atem %1 disconnected").arg(id));
}

void control::atmProgramInputChanged(atemId id, quint8 me, quint16 oldIndex, quint16 newIndex)
{
	qDebug() << "pgm ch " << id << " me " << me << " oldIndex " << oldIndex << " newIndex " << newIndex;
}

void control::atmPreviewInputChanged(atemId id, quint8 me, quint16 oldIndex, quint16 newIndex)
{
	qDebug() << "pgm ch " << id << " me " << me << " oldIndex " << oldIndex << " newIndex " << newIndex;
}

void control::incrementalProgress(void)
{
	float val = (100/totalTasks)*(++tasks);
	if (val == 100)
	{
		emit mwwReady();
	}
	else
		emit mwwProgress(val);
}

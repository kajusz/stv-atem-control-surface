#include "control.h"

#include <QSettings>
#include <QtDebug>

#include "qatemmixeffect.h"
#include "qatemdownstreamkey.h"

#include "translate.h"

// ctor
control::control(std::shared_ptr<kDevice> devc)
{
	// setup signals and slots
	dev = devc;
	connect(dev.get(), &kDevice::sigConnected, this, &control::devConnected);
	connect(dev.get(), &kDevice::sigDisconnected, this, &control::devDisconnected);
}

// dtor
control::~control(void)
{
	// disconnect from all the switchers
	for (atemVec::const_iterator it = bmd.begin(); it != bmd.end(); ++it)
	{
		atemPtr ti = *it;
		ti->disconnectFromSwitcher();
	}
}

// add atem object to the container
void control::registerAtem(atemPtr atem)
{
	bmd.push_back(atem);
}

// initialise
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

		connect(ti.get(), &QAtemConnection::connected, [id, this]() { this->atmConnected(x); });
		connect(ti.get(), &QAtemConnection::disconnected, [id, this]() { this->atmDisconnected(x); });

		ti->connectToSwitcher(QHostAddress(settings.value(QString("atem/%1/ip").arg(x), "192.168.10.240").toString()));
		if (ti->isConnected())
			incrementalProgress();
	}
}

// connect to kontroller
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
			emit mwwManuallyConnect();
			return;
		}

		bool foundDevice = false;
		device_t foundDeviceDevT;

		for (std::vector<device_t>::iterator it = q.begin(); it != q.end(); ++it)
			if (storedDevHid.compare(it->data) == 0 && storedDevName.compare(it->name) == 0)
			{
				foundDevice = true;
				foundDeviceDevT = *it;
				break;
			}

		if (foundDevice)
		{
			dev->setConfig(foundDeviceDevT);
			dev->begin();
		}
		else
			emit mwwManuallyConnect();
	}
}

// setup signals and slots for device
void control::devConnected(void)
{
	connect(dev.get(), &kDevice::sigBtnCommandKeyUp, this, &control::devBtnCommandKeyUp);
	connect(dev.get(), &kDevice::sigBtnGroupKeyUp, this, &control::devBtnGroupKeyUp);

	connect(dev.get(), &kDevice::sigTBarMove, this, &control::devTBarMove);
	connect(dev.get(), &kDevice::sigJoystickMove, this, &control::devJoystickMove);

	incrementalProgress();
}

// oh no
void control::devDisconnected(void)
{
	emit mwwStop("control: kontroller disconnected");
}

void control::devBtnCommandKeyUp(uint8_t keyNum)
{
	atemPtr ti = bmd.at(0);
	if (ti)
	{
		QAtemMixEffect *ame = bmd.at(0)->mixEffect(0);
		switch (keyNum)
		{
				// sequence control
			case 32: // play/pause
			case 33: // prev
			case 34: // insert
			case 35: // set
			case 36: // delete
			case 37: // next

			case 38: // direct patt
				// key
			case 45: // edge
			case 46: // shadow
			case 47: // out line
			case 48: // mask
			case 49: // mask invert
			case 50: // key invert
				// key insert
			case 51: // key bus
			case 52: // matt

			case 53: // editor enable
			case 54: // key 1
			case 55: // key 2

				// key source
			case 56: // key bus
			case 57: // lum
			case 58: // chroma key
			case 59: // pst patt

				// joystick keys
			case 60: // lock/def
			case 65: // sens hi/low
				break;


			case 119: ame->setUpstreamKeyOnNextTransition(0, !bitRead(ame->keyersOnNextTransition(), 1)); break;
			case 120: ame->setUpstreamKeyOnNextTransition(1, !bitRead(ame->keyersOnNextTransition(), 2)); break;
			case 121: ame->setUpstreamKeyOnNextTransition(2, !bitRead(ame->keyersOnNextTransition(), 3)); break;
			case 122: ame->setUpstreamKeyOnNextTransition(3, !bitRead(ame->keyersOnNextTransition(), 4)); break;

			case 129: ame->setBackgroundOnNextTransition(!bitRead(ame->keyersOnNextTransition(), 0)); break;
			case 128: break; //Fill
			case 127: break; //Key

			case 111: break;
			case 112:
			{
				QAtemDownstreamKey *dsk = ti->downstreamKey(0);
				dsk->setTie(!dsk->tie());
				break;
			}
			case 113:
			{
				QAtemDownstreamKey *dsk = ti->downstreamKey(1);
				dsk->setTie(!dsk->tie());
				break;
			}
			case 114: ame->setTransitionPreview(!ame->transitionPreviewEnabled()); break; // prev trans

			case 104: ame->setTransitionType(0); break; // mix
			case 105: ame->setTransitionType(1); break; // dip
			case 106: ame->setTransitionType(2); break; // wipe

			case 126: break;
			case 125: ame->toggleFadeToBlack(); break;
			case 124: ame->cut(); break;
			case 123: ame->autoTransition(); break;
		}
	}
	qDebug() << "CommandKeyU " << keyNum;
}

void control::devBtnGroupKeyUp(uint8_t gpId, uint8_t keyNum)
{
	QAtemMixEffect *ame = bmd.at(0)->mixEffect(0);

	switch (gpId)
	{
		case 2:
			ame->changePreviewInput(translateKeyNumToInputId(keyNum));
			break;
		case 1:
			ame->changeProgramInput(translateKeyNumToInputId(keyNum));
			break;
		case 0:
			ame->changePreviewInput(translateKeyNumToInputId(keyNum));
			break;
	}

	qDebug() << "Group " << gpId << " KeyU " << keyNum;
}

void control::devTBarMove(uint16_t pos)
{
//	QAtemMixEffect *ame = bmd.at(0)->mixEffect(0);
//	ame->setTransitionPosition(pos);
	qDebug() << "TBar " << pos;
}

void control::devJoystickMove(joystick_t data)
{
	qDebug() << "Joystick x " << data.x << " y " << data.y << " z " << data.z;
}

void control::atmConnected(atemId id)
{
	atemPtr ti = bmd.at(id);
	QAtemMixEffect *ame = bmd.at(id)->mixEffect(0);

	if (ame)
	{
		connect(ame, &QAtemMixEffect::programInputChanged, [id, this](quint8 me, quint16 oldIndex, quint16 newIndex) { this->atmProgramInputChanged(id, me, oldIndex, newIndex); });
		connect(ame, &QAtemMixEffect::previewInputChanged, [id, this](quint8 me, quint16 oldIndex, quint16 newIndex) { this->atmPreviewInputChanged(id, me, oldIndex, newIndex); });

		connect(ame, &QAtemMixEffect::transitionPreviewChanged, [id, this](quint8 me, bool state) { this->atmTransitionPreviewChanged(id, me, state); });
		connect(ame, &QAtemMixEffect::fadeToBlackChanged, [id, this](quint8 me, bool fading, bool enabled) { this->atmFadeToBlackChanged(id, me, fading, enabled); });
		connect(ame, &QAtemMixEffect::currentTransitionStyleChanged, [id, this](quint8 me, quint8 style) { this->atmCurrentTransitionStyleChanged(id, me, style); });
		connect(ame, &QAtemMixEffect::keyersOnCurrentTransitionChanged, [id, this](quint8 me, quint8 keyers) { this->atmKeyersOnCurrentTransitionChanged(id, me, keyers); });
		connect(ame, &QAtemMixEffect::upstreamKeyOnAirChanged, [id, this](quint8 me, quint8 keyer, bool state) { this->atmUpstreamKeyOnAirChanged(id, me, keyer, state); });
	}

	incrementalProgress();

	if (id == 0 && ame)
	{
		uint8_t keyNum, led;

		keyNum = translateInputIdToKeyNum(ame->programInput());
		led = translateKeyNumToLedNum(2, translateInputIdToKeyNum(keyNum));
		dev->setLed(led, rgbf(true, dev->getLed(led), RED));
		dev->setLed(translateKeyNumToLedNum(1, translateInputIdToKeyNum(keyNum)), rgbf(RED));

		keyNum = translateInputIdToKeyNum(ame->previewInput());
		led = translateKeyNumToLedNum(2, translateInputIdToKeyNum(keyNum));
		dev->setLed(led, rgbf(true, dev->getLed(led), GREEN));
		dev->setLed(translateKeyNumToLedNum(0, translateInputIdToKeyNum(keyNum)), rgbf(GREEN));


		dev->setLed(119, rgbf(true, rgbf(bitRead(ame->keyersOnNextTransition(), 1), GREEN), rgbf(bitRead(ame->keyersOnCurrentTransition(), 1), RED)));
		dev->setLed(120, rgbf(true, rgbf(bitRead(ame->keyersOnNextTransition(), 2), GREEN), rgbf(bitRead(ame->keyersOnCurrentTransition(), 2), RED)));
		dev->setLed(121, rgbf(true, rgbf(bitRead(ame->keyersOnNextTransition(), 3), GREEN), rgbf(bitRead(ame->keyersOnCurrentTransition(), 3), RED)));
		dev->setLed(122, rgbf(true, rgbf(bitRead(ame->keyersOnNextTransition(), 4), GREEN), rgbf(bitRead(ame->keyersOnCurrentTransition(), 4), RED)));

		dev->setLed(129, rgbf(bitRead(ame->keyersOnNextTransition(), 0), GREEN));
		//dev->setLed(128, rgbf());
		//dev->setLed(127, rgbf());

		//dev->setLed(111, rgbf());
		dev->setLed(112, rgbf(true, rgbf(ti->downstreamKey(0)->tie(), GREEN), rgbf(ti->downstreamKey(0)->onAir(), RED)));
		dev->setLed(113, rgbf(true, rgbf(ti->downstreamKey(1)->tie(), GREEN), rgbf(ti->downstreamKey(1)->onAir(), RED)));
		dev->setLed(114, rgbf(ame->transitionPreviewEnabled(), RED));

		dev->setLed(104, rgbf(ame->currentTransitionStyle() == 0, ORANGE));
		dev->setLed(105, rgbf(ame->currentTransitionStyle() == 1, ORANGE));
		dev->setLed(106, rgbf(ame->currentTransitionStyle() == 2, ORANGE));

		if (ame->fadeToBlackFading())
		{
			dev->setLed(125, rgbf(RED, ame->fadeToBlackEnabled()));
		}
	}
}

void control::atmDisconnected(atemId id)
{
	emit mwwStop(QString("control: atem %1 disconnected").arg(id));
}

void control::atmProgramInputChanged(atemId id, quint8 me, quint16 oldIndex, quint16 newIndex)
{
	uint8_t led;
	led = translateKeyNumToLedNum(2, translateInputIdToKeyNum(oldIndex));
	dev->setLed(led, rgbf(false, dev->getLed(led), RED));
	led = translateKeyNumToLedNum(2, translateInputIdToKeyNum(newIndex));
	dev->setLed(led, rgbf(true, dev->getLed(led), RED));

	dev->setLed(translateKeyNumToLedNum(1, translateInputIdToKeyNum(oldIndex)), rgbf(OFF));
	dev->setLed(translateKeyNumToLedNum(1, translateInputIdToKeyNum(newIndex)), rgbf(RED));

	qDebug() << "pgm ch " << id << " me " << me << " oldIndex " << oldIndex << " newIndex " << newIndex;
}

void control::atmPreviewInputChanged(atemId id, quint8 me, quint16 oldIndex, quint16 newIndex)
{
	if (oldIndex == newIndex)
	{
//		dev->setLed(translateKeyNumToLedNum(0, translateInputIdToKeyNum(newIndex)), rgbf(RED));
		return;
	}

	uint8_t led;
	led = translateKeyNumToLedNum(2, translateInputIdToKeyNum(oldIndex));
	dev->setLed(led, rgbf(false, dev->getLed(led), GREEN));
	led = translateKeyNumToLedNum(2, translateInputIdToKeyNum(newIndex));
	dev->setLed(led, rgbf(true, dev->getLed(led), GREEN));

	dev->setLed(translateKeyNumToLedNum(0, translateInputIdToKeyNum(oldIndex)), rgbf(OFF));
	dev->setLed(translateKeyNumToLedNum(0, translateInputIdToKeyNum(newIndex)), rgbf(GREEN));

	qDebug() << "prv ch " << id << " me " << me << " oldIndex " << oldIndex << " newIndex " << newIndex;
}

void control::atmTransitionPreviewChanged(atemId id, quint8 me, bool state)
{
	if (id == 0 && me == 0)
		dev->setLed(114, rgbf(state, RED));
}

void control::atmFadeToBlackChanged(atemId id, quint8 me, bool fading, bool enabled)
{
	if (id == 0 && me == 0)
	{
		if (fading == true)
			dev->setLed(125, rgbf(RED));
		else if (enabled == true)
			dev->setLed(125, rgbf(RED, true));
		else
			dev->setLed(125, rgbf(OFF));
	}
}

void control::atmCurrentTransitionStyleChanged(atemId id, quint8 me, quint8 style)
{
	if (id == 0 && me == 0)
		switch (style)
		{
			case 0:
				dev->setLed(104, rgbf(ORANGE));
				dev->setLed(105, rgbf(OFF));
				dev->setLed(106, rgbf(OFF));
				break;
			case 1:
				dev->setLed(104, rgbf(OFF));
				dev->setLed(105, rgbf(ORANGE));
				dev->setLed(106, rgbf(OFF));
				break;
			case 2:
				dev->setLed(104, rgbf(OFF));
				dev->setLed(105, rgbf(OFF));
				dev->setLed(106, rgbf(ORANGE));
				break;
		}
}

void control::atmKeyersOnCurrentTransitionChanged(atemId id, quint8 me, quint8 keyers)
{
	if (id == 0 && me == 0)
	{
		dev->setLed(119, rgbf(bitRead(keyers, 1), dev->getLed(119), GREEN));
		dev->setLed(120, rgbf(bitRead(keyers, 2), dev->getLed(120), GREEN));
		dev->setLed(121, rgbf(bitRead(keyers, 3), dev->getLed(121), GREEN));
		dev->setLed(122, rgbf(bitRead(keyers, 4), dev->getLed(122), GREEN));
		dev->setLed(129, rgbf(bitRead(keyers, 0), GREEN, OFF));
	}
}

void control::atmUpstreamKeyOnAirChanged(atemId id, quint8 me, quint8 keyer, bool state)
{
	if (id == 0 && me == 0)
		dev->setLed(119 + keyer, rgbf(state, dev->getLed(119 + keyer), RED));
}

void control::incrementalProgress(void)
{
	float val = (100/totalTasks)*(++tasks);
	if (val == 100)
		emit mwwReady();
	else
		emit mwwProgress(val);
}

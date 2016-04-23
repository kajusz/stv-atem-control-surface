// Device framework
#include "kdevice.h"
#include <QtDebug>

// Serial port implementation
#include <serial/serial.h>

#define trycatch(act) try { act; } catch (serial::SerialException &e) { QString str = "SerialException: "; str.append(e.what()); emit sigInfo(2,str); } catch (serial::IOException &e) { QString str = "IOException: "; str.append(e.getErrorNumber()); str.append(": ");  str.append(e.what()); emit sigInfo(2, str); } catch (serial::PortNotOpenedException &e) { QString str = "PortNotOpenedException: "; str.append(e.what()); emit sigInfo(2, str); } catch (std::invalid_argument &e) { QString str = "invalid_argument: "; str.append(e.what()); emit sigInfo(2, str); } /*catch (InvalidConfigurationException &e) { QString str = "InvalidConfigurationException: "; str.append(e.what()); emit sigInfo(2, str); }*/ catch (...) { throw; }

#define msgWrite(a, b, c) { uint8_t data[3]; data[0] = a; data[1] = b; data[2] = c; trycatch(port->write(data, 3)) }

kDevice::kDevice()
{
	// create the port
	port = new serial::Serial();

	// create the timer for pooling the serial port
	mTimer = new QTimer(this);
	connect(mTimer, SIGNAL(timeout()), this, SLOT(checkSerialData()));

	// initialise vars
	version = "";
	for (int i=0; i<2; i++)
	{
		buf[i] = 0;
		modifierKey[i] = false;
		joystickBuffer[i] = 0;
	}
}

kDevice::~kDevice()
{
	// clean up
	this->close();

	// destroy the timer
	delete mTimer;

	// free resources
	delete port;
}

std::vector<device_t> kDevice::getConfig(void)
{
	std::vector<device_t> ret;

	// list the available ports
	std::vector<serial::PortInfo> ports = serial::list_ports();
	std::vector<serial::PortInfo>::iterator it = ports.begin();

	// iterate over the available devices
	while (it != ports.end())
	{
		serial::PortInfo device = *it++;
		device_t kdev;
		kdev.data = device.hardware_id;
		kdev.id = device.port;
		kdev.name = device.description;
		ret.push_back(kdev);
	}

	return ret;
}

const bool kDevice::setConfig(const device_t& p_dev)
{
	portData = p_dev;
	return true;
}

void kDevice::begin(void)
{
	// we're opening the port, so it should not be open
	assert(!port->isOpen());

	// the trycatches are only on things that actually produce exceptions
	// functions which throw are not consistent with the documentation so beware of this on unix
	port->setPort(portData.id); //non
	port->setBaudrate(115200);
	port->setTimeout(serial::Timeout::simpleTimeout(1000)); //non
	port->setBytesize(serial::bytesize_t::eightbits);
	port->setParity(serial::parity_t::parity_none);
	port->setStopbits(serial::stopbits_t::stopbits_one);
	port->setFlowcontrol(serial::flowcontrol_t::flowcontrol_none);

	trycatch(port->open())

	port->setBreak(false);
	port->setDTR(false);
	port->setRTS(false);

	// check if we opened the port successfully
	if (port->isOpen())
	{
		// enable serial
		msgWrite(28, 127, 191)

		mTimer->start(10);

		// get firmware version, this is part of initialisation
		msgWrite(31, 64, 128)

		assert(mTimer->isActive());
	}
	else
		emit sigInfo(0, "kSerialDevice: Failed to open port.");
}

void kDevice::reset(void)
{
	mTimer->stop();

	// resetting is pointless if the port is not open
	assert(port->isOpen());

	// enable serial
	msgWrite(28, 127, 190)

	// send a reset command to the device
	msgWrite(27, 127, 191)

	// flush the socket (just in case)
	trycatch(port->flush())

	// disable serial
	msgWrite(28, 127, 191)

	mTimer->start(10);
}

void kDevice::close(void)
{
	// this is not an assert because this is called in destructors
	if (port->isOpen())
	{
		// clear leds
		msgWrite(17, 95, 191)

		// disable serial
		msgWrite(28, 127, 190)

		// send a reset command to the device
		msgWrite(27, 127, 191)

		mTimer->stop();

		// actually close the port
		port->close();

		// check (just in case)
		assert(!port->isOpen());
	}
}

void kDevice::tLed(const uint8_t& led, const rgbf& colour)
{
	// obviously check if it's open
	assert(port->isOpen());

	ledCols[led] = colour;

	// convert our arbitrary rgbf to simple tri-state colour + off + flash
	uint8_t ledCol = 0;
	if (colour.r > 192 && ((colour.g > 128 && colour.g < 192) || colour.g == 255) && colour.b < 32)
		ledCol = 4; // orange
	else if (colour.r > 192 && colour.g < 32 && colour.b < 32)
		ledCol = 6; // red
	else if (colour.r < 32 && colour.g > 192 && colour.b < 32)
		ledCol = 5; // green
	else
		ledCol = 7; // off
	if (colour.flash == true)
		ledCol = ledCol - 4;

	uint8_t x = 64 + (ledCol << 2) + ((led & 192) >> 6);
	uint8_t y = 128 + (led & 63);

	msgWrite(17, x, y)
}

void kDevice::setLeds(const uint8_t& start, const uint8_t& end, const rgbf& colour)
{
	for (int i = start; i < end; ++i)
		tLed(i, colour);
}

void kDevice::setLedsBtnGp(const uint8_t& btnGp, const rgbf& colour)
{
	switch (btnGp)
	{
		case 2:
			setLeds(92, 99, colour);
			setLeds(115, 118, colour);
			break;
		case 1:
			setLeds(84, 91, colour);
			setLeds(107, 110, colour);
			break;
		case 0:
			setLeds(76, 83, colour);
			setLeds(100, 103, colour);
			break;
	}
}

void kDevice::checkSerialData()
{
	size_t num = 0;
	trycatch(num = port->available())

	// check is any data is ready to be read
	if (num > 0)
	{
		std::vector<uint8_t> q;

		try { port->read(q, num); }
		catch (serial::SerialException &e)
		{ QString str = "SerialException: "; str.append(e.what()); emit sigInfo(2,str); }
		catch (serial::IOException &e)
		{ QString str = "IOException: "; str.append(e.getErrorNumber()); str.append(": ");  str.append(e.what()); emit sigInfo(2, str); }
		catch (serial::PortNotOpenedException &e)
		{ QString str = "PortNotOpenedException: "; str.append(e.what()); emit sigInfo(2, str); }
		catch (std::invalid_argument &e)
		{ QString str = "invalid_argument: "; str.append(e.what()); emit sigInfo(2, str); }
		/*catch (InvalidConfigurationException &e)
		{ QString str = "InvalidConfigurationException: "; str.append(e.what()); emit sigInfo(2, str); }*/
		catch (std::out_of_range &e)
		{ QString str = "out_of_range: "; str.append(e.what()); emit sigInfo(2, str); }
		catch (...)
		{ throw; }

		for (uint8_t i = 0; i < num; i++)
		{
			// want xx??????
			uint8_t a = q.at(i) >> 6;

			// check packet header, see which packet we recieved
			switch (a)
			{
			case 0:
			{
				buf[0] = q.at(i) & 63; // zero header bits and cache the packet
				break;
			}
			case 1:
			{
				buf[1] = q.at(i) & 63; // zero header bits and cache the packet
				break;
			}
			case 2:
			{
				if (bitRead(buf[0], 5) == 1) // 3 byte packet, process it
				{
					uint8_t cmd = buf[0] & 15; // only want the last 4 bits
					assert(cmd != 0);

					uint8_t a = buf[1] >> 2; // last 2 bits belong to z
					assert(a < 16); // 4 bit packet, anything bigger is invalid

					uint8_t b = ((buf[1] & 3) << 6) + (q.at(i) & 63); // rebuild 3rd packet from chunk 2 and 3
					this->process(cmd, a, b);
					break;
				}
				else // 4 byte packet, cache it
				{
					buf[2] = q.at(i) & 63;
					break;
				}
			}
			case 3:
			{
				//unused
			}
			}
		}
	}
}

void kDevice::process(uint8_t cmd, uint8_t a, uint8_t b)
{
	switch (cmd)
	{
	case 3://switch
	{
		uint8_t myState = a;
		assert(myState < 4 && myState > 0);
		filterKeyEvt(b, (keyState)myState);
		break;
	}
	case 6://adc
	{
		uint8_t myNum = a;
		assert(myNum < 8);
		filterAdcEvt((adcType) myNum, b);
		break;
	}
	case 14://version
	{
// bodged up initialisation, we connected successfully so the device sent its version
		char buff[100];
		std::sprintf(buff, "%i.%i", a, b);
		version = buff;

		this->initialise();

		emit sigConnected();
		break;
	}
	default:
	{
		emit sigInfo(1, "kSerialDevice: Recieved unknown data.");
		break;
	}
	}
}

void kDevice::initialise(void)
{
	this->tLed(61, rgbf(joystickCfg.all));
	this->tLed(62, rgbf(joystickCfg.x));
	this->tLed(63, rgbf(joystickCfg.y));
	this->tLed(64, rgbf(joystickCfg.z));
}

void kDevice::filterAdcEvt(adcType adc, int8_t data)
{
	switch (adc)
	{
	case tBar:
	{
		emit sigTBarMove(data);
		break;
	}
	case pot0:
	case pot1:
	case pot2:
	case pot3:
	{
		assert(data == 1 || data == -1);

		emit sigEncMove(adc, data);
		break;
	}
	case joyX:
	case joyY:
	case joyZ:
	{
		switch(adc)
		{
		case joyX:
			if (joystickCfg.x) return; break;
		case joyY:
			if (joystickCfg.y) return; break;
		case joyZ:
			if (joystickCfg.z) return; break;
		}

		joystickBuffer[adc - 5] = data;
		joystick_t joy;
		joy.x = joystickBuffer[0];
		joy.y = joystickBuffer[1];
		joy.z = joystickBuffer[2];

		switch (adc)
		{
		case joyX:
			joy.x_moved = true; break;
		case joyY:
			joy.y_moved = true; break;
		case joyZ:
			joy.z_moved = true; break;
		}

		emit sigJoystickMove(joy);
		break;
	}
	}
}

void kDevice::filterKeyEvt(uint8_t keyNum, keyState state)
{
	switch (keyNum)
	{
		// disp keys
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15: this->btnScreenKey(keyNum, state); break;
		// numpad keys
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21: this->btnKeyboardKey(keyNum, state); break;
	case 22:
	case 23: this->btnScreenKey(keyNum, state); break;
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31: this->btnKeyboardKey(keyNum, state); break;
		// sequence keys
	case 32: // play/pause
	case 33: // prev
	case 34: // insert
	case 35: // set
	case 36: // delete
	case 37: // next
		// direct patt
	case 38: this->btnCommandKey(keyNum, state); break;
		// status
	case 40: this->btnScreenKey(keyNum, state); break;
		// arrow keys
	case 41:
	case 42:
	case 43:
	case 44: this->btnKeyboardKey(keyNum, state); break;
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
		//
	case 53: // editor enable
	case 54: // key 1
	case 55: // key 2
		// key source
	case 56: // key bus
	case 57: // lum
	case 58: // chroma key
	case 59: // pst patt
		// joystick keys
	case 60: this->btnCommandKey(keyNum, state); break;
	case 61:
	case 62:
	case 63:
	case 64: this->btnJoystickKey(keyNum, state); break;
	case 65: this->btnCommandKey(keyNum, state); break;
		// gui keys
	case 66:
	case 67:
	case 68:
	case 69:
	case 70:
	case 71:
	case 72:
	case 73:
	case 74:
	case 75: this->btnScreenKey(keyNum, state); break;
		// switcher keys
	case 76:
	case 77:
	case 78:
	case 79:
	case 80:
	case 81:
	case 82:
	case 83: this->btnGroupKey(0, keyNum - 76, state); break;
	case 84:
	case 85:
	case 86:
	case 87:
	case 88:
	case 89:
	case 90:
	case 91: this->btnGroupKey(1, keyNum - 84, state); break;
	case 92:
	case 93:
	case 94:
	case 95:
	case 96:
	case 97:
	case 98:
	case 99: this->btnGroupKey(2, keyNum - 92, state); break;
	case 100:
	case 101:
	case 102:
	case 103: this->btnGroupKey(0, keyNum - 92, state); break;
		// mix, wipe, 3d effect
	case 104:
	case 105:
	case 106: this->btnCommandKey(keyNum, state); break;
		// switcher keys
	case 107:
	case 108:
	case 109:
	case 110: this->btnGroupKey(1, keyNum - 99, state); break;
		// bkgd, key 1/2, priority
	case 111:
	case 112:
	case 113:
	case 114: this->btnCommandKey(keyNum, state); break;
		// switcher keys
	case 115:
	case 116:
	case 117:
	case 118: this->btnGroupKey(2, keyNum - 107, state); break;
		// key: source, insert, source, insert
	case 119:
	case 120:
	case 121:
	case 122:
		// pgm trans, cut, dsk trans, black trans
	case 123:
	case 124:
	case 125:
	case 126:
		// wipe direction: nor rev, rev, nor
	case 127:
	case 128:
	case 129: this->btnCommandKey(keyNum, state); break;
	default:
		return;
	}
}

void kDevice::btnCommandKey(uint8_t keyNum, keyState state)
{
	if (state == KEYDOWN)
		emit sigBtnCommandKeyDown(keyNum);
	else if (state == KEYUP)
		emit sigBtnCommandKeyUp(keyNum);
}

void kDevice::btnGroupKey(uint8_t row, uint8_t keyNum, keyState state)
{
	if (state == KEYDOWN && keyNum == 11)
		modifierKey[row] = true;
	else if (state == KEYUP && keyNum == 11)
		modifierKey[row] = false;

	uint8_t ret;

	if (modifierKey[row] == true)
		ret = keyNum + 12;
	else
		ret = keyNum;

	if (state == KEYDOWN)
		emit sigBtnGroupKeyDown(row, ret);
	else if (state == KEYUP)
		emit sigBtnGroupKeyUp(row, ret);
}

void kDevice::btnJoystickKey(uint8_t keyNum, keyState state)
{
	if (state == KEYDOWN)
	{
		switch (keyNum)
		{
		case 61://xyz
			joystickCfg.all = !joystickCfg.all;
			this->tLed(61, rgbf(joystickCfg.all));
			joystickCfg.x = joystickCfg.all;
			joystickCfg.y = joystickCfg.all;
			joystickCfg.z = joystickCfg.all;
			this->tLed(62, rgbf(joystickCfg.x));
			this->tLed(63, rgbf(joystickCfg.y));
			this->tLed(64, rgbf(joystickCfg.z));
			break;
		case 62://x
			joystickCfg.x = !joystickCfg.x;
			this->tLed(62, rgbf(joystickCfg.x));
			break;
		case 63://y
			joystickCfg.y = !joystickCfg.y;
			this->tLed(63, rgbf(joystickCfg.y));
			break;
		case 64://z
			joystickCfg.z = !joystickCfg.z;
			this->tLed(64, rgbf(joystickCfg.z));
			break;
		}
	}
}

void kDevice::btnKeyboardKey(uint8_t keyNum, keyState state)
{
	uint8_t rawCode;

	switch (keyNum)
	{
	case 16:/*   7   */ rawCode = 103; break;
	case 17:/*   4   */ rawCode = 100; break;
	case 18:/*   8   */ rawCode = 104; break;
	case 19:/*   5   */ rawCode = 101; break;
	case 20:/*   9   */ rawCode = 105; break;
	case 21:/*   6   */ rawCode = 102; break;
	case 24:/*   1   */ rawCode =  97; break;
	case 25:/*   0   */ rawCode =  96; break;
	case 26:/*   2   */ rawCode =  98; break;
	case 27:/* shift */ rawCode =  16; break;
	case 28:/*   3   */ rawCode =  99; break;
	case 29:/* clear */ rawCode =   8; break;
	case 30:/*  +/-  */ rawCode = 109; break;
	case 31:/* enter */ rawCode =  13; break;
	case 41:/*  up   */ rawCode =  38; break;
	case 42:/* right */ rawCode =  37; break;
	case 43:/* left  */ rawCode =  39; break;
	case 44:/* down  */ rawCode =  40; break;
	}

	if (state == KEYDOWN)
		emit sigBtnKeyboardKeyDown(rawCode);
	else if (state == KEYUP)
		emit sigBtnKeyboardKeyUp(rawCode);
}

void kDevice::btnScreenKey(uint8_t keyNum, keyState state)
{
	if (state == KEYDOWN)
		emit sigBtnScreenKeyDown(keyNum);
	else if (state == KEYUP)
		emit sigBtnScreenKeyUp(keyNum);
}

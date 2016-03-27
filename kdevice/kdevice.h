#pragma once

/* TODO:
 * joystick disabling needs to be done in hardware (currently in initialise() and in filterAdcEvt)
 * rescale tbar value
 *
 *
 */

// qt
#include <QObject>
#include <QString>
#include <QTimer>

// Standard Libraries
#include <cstdint>
#include <vector>
#include <cstring>
#include <cassert>

// colour handling
#include "rgbf.h"

// Forward declare the serial port class
namespace serial
{
class Serial;
}

// TX/RX values and limits
//#include "../firmware/VPS400/VPS400.h"
#ifndef VPS400_H
#define VPS400_H

#ifndef bitRead
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#endif

enum keyState
{
	INVALID = 0,
	KEYDOWN = 1,
	KEYHOLD = 2,
	KEYUP = 3,
};

enum adcType
{
	tBar = 0,
	pot0 = 1,
	pot1 = 2,
	pot2 = 3,
	pot3 = 4,
	joyX = 5,
	joyY = 6,
	joyZ = 7,
};
#endif

// structure for storing device data
struct device_t
{
	std::string id;
	std::string name;
	std::string data;
	device_t()
	{
		id = "255";
		name = "_INVALID_";
		data = "_INVALID_";
	}
};

// structure for storing joystick movement
struct joystick_t
{
	int8_t x;
	bool x_moved;
	int8_t y;
	bool y_moved;
	int8_t z;
	bool z_moved;
	joystick_t()
	{
		x = 0;
		y = 0;
		z = 0;
		x_moved = false;
		y_moved = false;
		z_moved = false;
	}
};

#include "kdevice_global.h"

class KDEVICESHARED_EXPORT kDevice : public QObject
{
	Q_OBJECT

public:
	kDevice();
	~kDevice();

	std::vector<device_t> getConfig(void);
	const bool setConfig(const device_t& p_dev);

	void begin(void);
	void reset(void);
	void close(void);

	const rgbf& getLed(const uint8_t& led) { return ledCols[led]; }

	const std::string getVersion(void) { return version; }

public slots:
	void setLed(const uint8_t& led, const rgbf& colour) { tLed(led, colour); }
	void setLeds(const uint8_t& start, const uint8_t& end, const rgbf& colour);

signals:
	void sigInfo(int8_t id, QString what); // used for spittion out errors, @param - id number, @param - text

	void sigConnected(void); // emitted only when device is present
	void sigDisconnected(void); // currently unused

	// @param - key number
	void sigBtnCommandKeyDown(uint8_t keyNum);
	void sigBtnCommandKeyUp(uint8_t keyNum);
	// @param - key data
	void sigBtnKeyboardKeyDown(uint8_t rawCode);
	void sigBtnKeyboardKeyUp(uint8_t rawCode);
	// @param1 - button group, param2 - key number
	void sigBtnGroupKeyDown(uint8_t gpId, uint8_t keyNum);
	void sigBtnGroupKeyUp(uint8_t gpId, uint8_t keyNum);
	// @param - key number
	void sigBtnScreenKeyDown(uint8_t keyNum);
	void sigBtnScreenKeyUp(uint8_t keyNum);

	void sigTBarMove(uint16_t pos); // @param - fader position
	void sigEncMove(uint8_t encId, int8_t data); // @param1 - encoder number, @param2 - amount changed
	void sigJoystickMove(joystick_t data); // @param - joystick data

private:
	// serial port objects
	serial::Serial *port;
	device_t portData;

	QTimer *mTimer;

	std::string version;

	rgbf ledCols[143];

	uint8_t buf[3];
	bool modifierKey[3];

	int8_t joystickBuffer[3];
	struct joystickCfg_t
	{
		bool all;
		bool x;
		bool y;
		bool z;
		joystickCfg_t()
		{
			all = true;
			x = true;
			y = true;
			z = true;
		}
	} joystickCfg;

protected:
	// initialisation of serial connection
	void initialise(void);

	void tLed(const uint8_t& led, const rgbf& colour);

	void process(uint8_t cmd, uint8_t a, uint8_t b);

	void filterAdcEvt(adcType adc, int8_t data);
	void filterKeyEvt(uint8_t keyNum, keyState state);

	void btnCommandKey(uint8_t keyNum, keyState state);
	void btnGroupKey(uint8_t row, uint8_t keyNum, keyState state);
	void btnJoystickKey(uint8_t keyNum, keyState state);
	void btnKeyboardKey(uint8_t keyNum, keyState state);
	void btnScreenKey(uint8_t keyNum, keyState state);

private slots:
	void checkSerialData(); // called by timer
};

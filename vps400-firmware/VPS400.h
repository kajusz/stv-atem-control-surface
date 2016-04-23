#include <stdint.h>

#ifndef VPS400_H
#define VPS400_H

#ifndef bitRead
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#endif

//#define OPTIMISED

#define ADDRESSPINS 3
#define DGIOPINS 8
#define FLIPFLOPS 35
#define TOTALLEDS 144 //KEYCARDSWITCHES + BUSCARDSWITCHES + STATUSLEDS
#define TOTALSWITCHES 130

#define KEYCARDFLIPFLOPS 16
#define KEYADDRESSES 8
#define KEY245OEPINS 4
#define KEY238E3PINS 2
#define KEYCARDSWITCHES 76

#define BUSCARDFLIPFLOPS 19
#define BUSADDRESSES 7 // Last address unused (8x7 grid)
#define BUS238E3PINS 3 // Number of 74HC238 ICs
#define BUSCARDSWITCHES 54

#define JOYSTICKPINS 3

#define VERSIONMAJOR 2
#define VERSIONMINOR 25

// Common
static const uint8_t key245BusOEPin = 15;
static const uint8_t keyAdPins[ADDRESSPINS] = { 5, 2, 3 }; // A0, A1, A2 pins for HC138 (key grid) and HC238
static const uint8_t key245BusTrPin = 26; //TR Pin of the HC245 for the main 8-bit data bus
static const uint8_t key245GridOEPins[KEY245OEPINS] = { 22, 23, 24, 25 }; // OE(bar) pins for key grid/buttons
static const uint8_t key138E3Pin = 27; // E3 Pin for the key grid IC
static const uint8_t key238E3Pins[KEY238E3PINS] = { 28, 29 }; // E3 Pins for 238s
static const uint8_t keyIoPins[DGIOPINS] = { 37, 36, 35, 34, 33, 32, 31, 30 }; // key grid input/led data output

static const uint8_t bus245BusOEPin = 14;
static const uint8_t busAdPins[ADDRESSPINS] = { 41, 40, 39 }; // A0, A1, A2 pins for HC138 (key grid) and HC238
static const uint8_t bus245BusTrPin = 6; //TR Pin of the HC245 for the main 8-bit data bus
static const uint8_t bus245GridOEPin = 16; // OE(bar) pins for key grid/buttons
static const uint8_t bus138E3Pin = 17; // E3 Pin for the key grid IC
static const uint8_t bus238E3Pins[BUS238E3PINS] = { 7, 8, 9 }; // E3 Pins for 238s
static const uint8_t busIoPins[DGIOPINS] = { 49, 48, 47, 46, 45, 44, 43, 42 }; // key grid input/led data output

static const uint8_t potPinPairs[8] = { 62, 63, 64, 65, 66, 67, 68, 69 }; // { A8, A9, A10, A11, A12, A13, A14, A15 };

static const uint8_t joystickXyzPins[JOYSTICKPINS] = { 54, 55, 56 }; // { A0, A1, A2 }; //

static const uint8_t tBarPotPin = 57; // A3;
static const uint8_t tBarServoPin = 58; // A4;

static const int8_t encDir[16] = { 0, -1,  1,  0,  1,  0,  0, -1, -1,  0,  0,  1,  0,  1, -1,  0 };
static const int16_t joystickShift[JOYSTICKPINS] = { 127, 128, 131 };

// Keys
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

class VPS
{
protected:

// Buffer
	struct dataBuffer
	{
		uint8_t p1;
		uint8_t p2;
		uint8_t p3;
		dataBuffer()
		{
			p1 = 0;
			p2 = 0;
			p3 = 0;
		}
	};

// Keys
	struct keyBuffer
	{
		keyState state;
		uint8_t time;
		uint8_t integrator;
		keyState intBuf;
		keyBuffer()
		{
			state = KEYUP;
			time = 0;
			integrator = 0;
		}
	};

public:
// Common
	VPS();
	~VPS() {}

// Input / Output
	void initIo(void);

// Inputs - Keys Grid
	void detectKeysGrid(void);

// Inputs - Other Dev
	void detectEncoders(void);
	void detectFaderBar(void);
	void detectJoystick(void);

// Serial
	void doTasks(void);
	void sendData(void);

	typedef void (*dataSignature)(const uint8_t&, const uint8_t&, const uint8_t&);

protected:
// Inputs - Keys Grid
	void doDebounce(const uint8_t& swNum, const bool& rawState);
	void cueSendKey(const uint8_t& swNum, const keyState& kState);

// Inputs - Other Dev
	void cueOtherDev(const uint8_t& adcNum, const uint8_t& data);

// Outputs - Leds
	void setLed(const uint8_t& state, const uint8_t& ledNum);
	void flashLed(const uint8_t& state, const uint8_t& ledNum);
	uint8_t updateLedBuffer(const uint8_t& bufNum, const uint8_t& ledState, const uint8_t& bitShift);
	void allLedsOff(void);

// Outputs - Other Dev
	void moveTBar(const uint16_t& pos);
	void doBuzzer(const uint8_t& dur, const uint8_t& freq);

// Other
	void doSpecials(const bool& xord, const uint8_t& bitMask);
	dataSignature dataTxFn;

public:
	void registerSerialHandler(dataSignature ptr) { dataTxFn = ptr; }
	void registerSerialData(const uint8_t& b);
	void registerSerialData(const uint8_t& p1, const uint8_t& p2, const uint8_t& p3);

private:
// Input / Output - Abstraction
	void prepareKeysIo(void);
	void prepareLedsIo(const bool& card);
	uint16_t adcRead(const uint8_t& pinNum);
	bool pinRead(const uint8_t pinNum);
	uint8_t ioRead(const bool& card);
	void ioWrite(const bool& card, const uint8_t& buffer);
	void addressWrite(const uint8_t& ic, const uint8_t& addr);
	void keyAddressWrite(const uint8_t& addr);
	void busAddressWrite(const uint8_t& addr);
	void keysGridOutputEnable(const uint8_t& ic);
	void keysGridOutputDisable(const uint8_t& ic);
protected:
// Input / Output
	bool transmit;
	bool keyPinsStateChanged; // reduce bus i/o state changes
	uint16_t currentSpecial; // bit masked special features
	/* Bit: 0: All Off, 1: Test Mode, 2: Don't buffer keys */

// Input - Keys Grid
	keyBuffer debounceBuffer[TOTALSWITCHES];

// Input - Other Dev
	uint16_t tBarPosition;

	int8_t encState[4];

	int8_t joystickBuffer[JOYSTICKPINS]; /**/

// Output - Leds
	uint8_t ledBuffer[FLIPFLOPS]; /**/
//	uint8_t flashBuffer[TOTALLEDS];

// Output - Other Dev

// Buffer
	uint8_t quickBuff[3];

	dataBuffer rxBuffer[32];
	int8_t rxBufferCE; // current entry in the buffer above

	dataBuffer txBuffer[16];
	int8_t txBufferCE; // current entry in the buffer above

private:
// Common

};
#endif
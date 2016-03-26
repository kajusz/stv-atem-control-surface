#include "VPS400.h"
#include <Arduino.h>

void VPS::detectKeysGrid(void)
{
	prepareKeysIo();
	uint8_t io = 0;
	uint8_t swNum = 0;

// KEY Card
	keysGridOutputEnable(0);
	for (uint8_t c = 0; c < 5; c++)
	{
		keyAddressWrite(c);
		io = ioRead(0);
		for (uint8_t r = 0; r < DGIOPINS; r++)
			doDebounce(r + (c * 8), bitRead(io, r));
	}

	keyAddressWrite(5);
	io = ioRead(0);
	for (uint8_t r = 0; r < 5; r++)// Status + Arrow Keys, (three missing)
		doDebounce(r + (5 * 8), bitRead(io, r));

	keyAddressWrite(6);
	io = ioRead(0);
	for (uint8_t r = 0; r < DGIOPINS; r++)// Key, Key Insert
		doDebounce(r + 45, bitRead(io, r));

	keyAddressWrite(7);
	io = ioRead(0);
	for (uint8_t r = 0; r < 7; r++)// Editor Enable, Key 1, Key 2, Key Source, (one missing)
		doDebounce(r + 53, bitRead(io, r));
	keysGridOutputDisable(0);

	keysGridOutputEnable(1);
	io = ioRead(0);
	for (uint8_t r = 0; r < 6; r++)//Joystick keys
		doDebounce(r + 60, bitRead(io, r));
	keysGridOutputDisable(1);

	keysGridOutputEnable(2);
	io = ioRead(0);
	for (uint8_t r = 0; r < DGIOPINS; r++)//F1 -> F4, Disp Shift, U1 -> U3
		doDebounce(r + 66, bitRead(io, r));
	keysGridOutputDisable(2);

	keysGridOutputEnable(3);
	io = ioRead(0);
	for (uint8_t r = 0; r < 2; r++)//U4, U5
		doDebounce(r + 74, bitRead(io, r));
	keysGridOutputDisable(3);

// BUS Card
	keysGridOutputEnable(4);
	for (uint8_t c = 0; c < 3; c++)
	{
		busAddressWrite(c);
		io = ioRead(1);
		for (uint8_t r = 0; r < DGIOPINS; r++)
			doDebounce(r + (c * 8) + 76, bitRead(io, r));
	}

	busAddressWrite(3);
	io = ioRead(1);
	for (uint8_t r = 0; r < 7; r++)//8, bkgd/matt, 3d effect, shift, mix, wipe, 3d effect
		doDebounce(r + 100, bitRead(io, r));

	for (uint8_t c = 4; c < 6; c++)
	{
		busAddressWrite(c);
		io = ioRead(1);
		for (uint8_t r = 0; r < DGIOPINS; r++)
			doDebounce(r + (c * 8) + 75, bitRead(io, r));
	}

	busAddressWrite(6);
	io = ioRead(1);
	for (uint8_t r = 0; r < 7; r++)//pgm trans, cut, dsk trans, black trans, nor rev, rev, nor
		doDebounce(r + 123, bitRead(io, r));

	keysGridOutputDisable(4);
}

void VPS::doDebounce(const uint8_t& swNum, const bool& rawState)
{
	switch (debounceBuffer[swNum].state)
	{
		case KEYDOWN:
			if (rawState == 0 && ++debounceBuffer[swNum].time > 200) // Pressed
			{
				debounceBuffer[swNum].state = KEYHOLD;
				cueSendKey(swNum, KEYHOLD);
			}
			else if (rawState == 1) // Not Pressed
			{
				debounceBuffer[swNum].state = KEYUP;
				debounceBuffer[swNum].time = 0;
				cueSendKey(swNum, KEYUP);
			}
			break;
		case KEYUP:
			if (rawState == 0) // Pressed
			{
				debounceBuffer[swNum].state = KEYDOWN;
				cueSendKey(swNum, KEYDOWN);
			}
			break;
		case KEYHOLD:
			if (rawState == 1) // Not Pressed
			{
				debounceBuffer[swNum].state = KEYUP;
				debounceBuffer[swNum].time = 0;
				cueSendKey(swNum, KEYUP);
			}
			break;
	}
}

void VPS::cueSendKey(const uint8_t& swNum, const keyState& kState)
{
	if (bitRead(currentSpecial, 1))
	{
		if (kState == KEYDOWN)
			setLed(5, swNum);
		else if (kState == KEYHOLD)
			setLed(4, swNum);
		else if (kState == KEYUP)
			setLed(7, swNum);
	}

	uint8_t a = 35; // command for send keys
	uint8_t b = 64; // packet 2
	uint8_t c = 128;// packet 3

	b = b + (((uint8_t) kState) << 2) + ((swNum & 192) >> 6);
	c = c + (swNum & 63);

 	if (bitRead(currentSpecial, 2))
		dataTxFn(a, b, c);
	else
	{
		txBufferCE++;
		txBuffer[txBufferCE].p1 = a;
		txBuffer[txBufferCE].p2 = b;
		txBuffer[txBufferCE].p3 = c;
	}
}
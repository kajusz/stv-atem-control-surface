#include "VPS400.h"

#ifndef OPTIMISED
#include <Arduino.h>



void VPS::initIo(void)
{
// Common
	for (uint8_t i = 0; i < DGIOPINS; i++)
		pinMode(keyIoPins[i], OUTPUT);

	for (uint8_t i = 0; i < DGIOPINS; i++)
		pinMode(busIoPins[i], OUTPUT);

	pinMode(key245BusTrPin, OUTPUT);
	digitalWrite(key245BusTrPin, 0);

	pinMode(bus245BusTrPin, OUTPUT);
	digitalWrite(bus245BusTrPin, 0);

	pinMode(key245BusOEPin, OUTPUT);
	digitalWrite(key245BusOEPin, 0);

	pinMode(bus245BusOEPin, OUTPUT);
	digitalWrite(bus245BusOEPin, 0);

	for (uint8_t i = 0; i < ADDRESSPINS; i++)
		pinMode(keyAdPins[i], OUTPUT);

	for (uint8_t i = 0; i < ADDRESSPINS; i++)
		pinMode(busAdPins[i], OUTPUT);

// Keys
	for (uint8_t i = 0; i < KEY245OEPINS; i++)
	{
		pinMode(key245GridOEPins[i], OUTPUT);
		digitalWrite(key245GridOEPins[i], 1);
	}

	pinMode(key138E3Pin, OUTPUT);
	digitalWrite(key138E3Pin, 0);

	pinMode(bus245GridOEPin, OUTPUT);
	digitalWrite(bus245GridOEPin, 1);

	pinMode(bus138E3Pin, OUTPUT);
	digitalWrite(bus138E3Pin, 0);

// Leds
	for (uint8_t i = 0; i < KEY238E3PINS; i++)
	{
		pinMode(key238E3Pins[i], OUTPUT);
		digitalWrite(key238E3Pins[i], 0);
	}

	for (uint8_t i = 0; i < BUS238E3PINS; i++)
	{
		pinMode(bus238E3Pins[i], OUTPUT);
		digitalWrite(bus238E3Pins[i], 0);
	}

// ALL LEDS OFF
	allLedsOff();

// Keys
	prepareKeysIo();

// Pots
	for (uint8_t i = 0; i < 8; i++)
	{
		pinMode(potPinPairs[i], INPUT);
		digitalWrite(potPinPairs[i], HIGH);
	}

// tBar
	pinMode(tBarPotPin, INPUT);
//	pinMode(tBarServoPin, OUTPUT);

// Joystick
	for (uint8_t i = 0; i < JOYSTICKPINS; i++)
		pinMode(joystickXyzPins[i], INPUT);
}

void VPS::prepareKeysIo(void)
{
	if (keyPinsStateChanged == true)
	{
		for (uint8_t i = 0; i < DGIOPINS; i++)
			pinMode(keyIoPins[i], INPUT);
	
		for (uint8_t i = 0; i < DGIOPINS; i++)
			pinMode(busIoPins[i], INPUT);
	
		digitalWrite(key245BusTrPin, 1);
		digitalWrite(bus245BusTrPin, 1);
	
		keyPinsStateChanged = false;
	}
}

void VPS::prepareLedsIo(const bool& card)
{
	if (keyPinsStateChanged == false)
	{
		if (card == 0) // KEY card
		{
			for (uint8_t i = 0; i < DGIOPINS; i++)
				pinMode(keyIoPins[i], OUTPUT);
			digitalWrite(key245BusTrPin, 0);
		}
		else // BUS card
		{
			for (uint8_t i = 0; i < DGIOPINS; i++)
				pinMode(busIoPins[i], OUTPUT);
			digitalWrite(bus245BusTrPin, 0);
		}
		keyPinsStateChanged = true;
	}
}

uint16_t VPS::adcRead(const uint8_t& pinNum)
{
	return analogRead(pinNum);
}

bool VPS::pinRead(const uint8_t pinNum)
{
	return digitalRead(pinNum);
}

uint8_t VPS::ioRead(const bool& card)
{
	uint8_t ret = 0;
	if (card == 0)
		for (uint8_t r = 0; r < DGIOPINS; r++)
			bitWrite(ret, r, digitalRead(keyIoPins[r]));
	else
		for (uint8_t r = 0; r < DGIOPINS; r++)
			bitWrite(ret, r, digitalRead(busIoPins[r]));
	return ret;
}

void VPS::ioWrite(const bool& card, const uint8_t& buffer)
{
	if (card == 0) // KEY card
	{
		for (uint8_t r = 0; r < 8; r++)
			digitalWrite(keyIoPins[r], bitRead(buffer, r));
	}
	else // BUS card
	{
		for (uint8_t r = 0; r < 8; r++)
			digitalWrite(busIoPins[r], bitRead(buffer, r));
	}
}

void VPS::keyAddressWrite(const uint8_t& addr)
{
	digitalWrite(keyAdPins[0], bitRead(addr, 0));
	digitalWrite(keyAdPins[1], bitRead(addr, 1));
	digitalWrite(keyAdPins[2], bitRead(addr, 2));
}

void VPS::busAddressWrite(const uint8_t& addr)
{
	digitalWrite(busAdPins[0], bitRead(addr, 0));
	digitalWrite(busAdPins[1], bitRead(addr, 1));
	digitalWrite(busAdPins[2], bitRead(addr, 2));
}

void VPS::addressWrite(const uint8_t& ic, const uint8_t& addr)
{
	if (ic == 0 || ic == 1)
	{
		keyAddressWrite(addr);

		digitalWrite(key238E3Pins[ic], 1);
		digitalWrite(key238E3Pins[ic], 0);
	}
	else if (ic == 2 || ic == 3|| ic == 4)
	{
		busAddressWrite(addr);

		digitalWrite(bus238E3Pins[ic - 2], 1);
		digitalWrite(bus238E3Pins[ic - 2], 0);
	}
}

void VPS::keysGridOutputEnable(const uint8_t& ic)
{
	if (ic == 0)
	{
		digitalWrite(key138E3Pin, 1);
		digitalWrite(key245GridOEPins[0], 0);
	}
	else if (ic > 0 && ic < 4)
	{
		digitalWrite(key245GridOEPins[ic], 0);
	}
	else if (ic == 4)
	{
		digitalWrite(bus138E3Pin, 1);
		digitalWrite(bus245GridOEPin, 0);
	}
}

void VPS::keysGridOutputDisable(const uint8_t& ic)
{
	if (ic == 0)
	{
		digitalWrite(key245GridOEPins[0], 1);
		digitalWrite(key138E3Pin, 0);
	}
	else if (ic > 0 && ic < 4)
	{
		digitalWrite(key245GridOEPins[ic], 1);
	}
	else if (ic == 4)
	{
		digitalWrite(bus245GridOEPin, 1);
		digitalWrite(bus138E3Pin, 0);
	}
}

#endif
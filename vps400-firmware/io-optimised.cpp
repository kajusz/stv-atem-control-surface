#include "VPS400.h"

#ifdef OPTIMISED
#define __AVR_ATmega2560__ 1
#include <avr/io.h>
#include <binary.h>
void VPS::initIo(void)
{
	//for (uint8_t i = 0; i < KEY245OEPINS; i++) pinMode(key245GridOEPins[i], OUTPUT);
	//pinMode(key245BusTrPin, OUTPUT);
	//pinMode(key138E3Pin, OUTPUT);
	//for (uint8_t i = 0; i < KEY238E3PINS; i++) pinMode(key238E3Pins[i], OUTPUT);
	DDRA = B11111111;

	//for (uint8_t i = 0; i < KEY245OEPINS; i++) digitalWrite(key245GridOEPins[i], 1);
	//digitalWrite(key245BusTrPin, 0);
	//digitalWrite(key138E3Pin, 0);
	//for (uint8_t i = 0; i < KEY238E3PINS; i++) digitalWrite(key238E3Pins[i], 0);
	PORTA = B00001111;

	DDRB = B00000000; // 8 unused pins
	PORTB = B11111111; // internal pull-up on 8 unused pins

	//for (uint8_t i = 0; i < DGIOPINS; i++) pinMode(keyIoPins[i], OUTPUT);
	DDRC = B11111111;

	DDRD = B00000000; // 8 unused pins
	PORTD = B11111111; // internal pull-up on 8 unused pins

	//for (uint8_t i = 0; i < ADDRESSPINS; i++) pinMode(keyAdPins[i], OUTPUT);
	DDRE = B00111010; // 3 unused pins + serial
	PORTE = B11000100; // internal pull-up on 3 unused pins

	//for (uint8_t i = 0; i < JOYSTICKPINS; i++) pinMode(joystickXyzPins[i], INPUT);
	//pinMode(tBarPotPin, INPUT);
	//NOT pinMode(tBarServoPin, OUTPUT);
	DDRF = B00000000; // 4 unused pins
	PORTF = B11110000; // internal pull-up on 4 unused pins

	//for (uint8_t i = 0; i < ADDRESSPINS; i++) pinMode(busAdPins[i], OUTPUT);
	DDRG = B00000111; // 3 unused pins, 2 null pins
	PORTG = B00111000; // internal pull-up on 3 unused pins

	//pinMode(bus138E3Pin, OUTPUT);
	//pinMode(bus245GridOEPin, OUTPUT);
	//pinMode(bus245BusTrPin, OUTPUT);
	//for (uint8_t i = 0; i < BUS238E3PINS; i++) pinMode(bus238E3Pins[i], OUTPUT);
	DDRH = B01110111; // 2 unused pins
	//digitalWrite(bus138E3Pin, 0);
	//digitalWrite(bus245GridOEPin, 1);
	//digitalWrite(bus245BusTrPin, 0);
	//for (uint8_t i = 0; i < BUS238E3PINS; i++) digitalWrite(bus238E3Pins[i], 0);
	PORTH = B10000110; // internal pull-up on 2 unused pins

	//pinMode(key245BusOEPin, OUTPUT);
	//pinMode(bus245BusOEPin, OUTPUT);
	DDRJ = B00000011; // 6 unused pins
	//digitalWrite(key245BusOEPin, 0);
	//digitalWrite(bus245BusOEPin, 0);
	PORTJ = B11111100; // internal pull-up on 6 unused pins

	//for (uint8_t i = 0; i < 8; i++) pinMode(potPinPairs[i], INPUT);
	DDRK = B00000000;
	//for (uint8_t i = 0; i < 8; i++) digitalWrite(potPinPairs[i], HIGH);
	PORTK = B11111111;

	//for (uint8_t i = 0; i < DGIOPINS; i++) pinMode(busIoPins[i], OUTPUT);
	DDRL = B11111111;

// ALL LEDS OFF
	{
		// KEY Card
		for (uint8_t k = 0; k < 16; k++)
			ledBuffer[k] = 255;
		ioWrite(0, 255);
		for (uint8_t k = 0; k < 16; k++)
			addressWrite(((unsigned int) k / 8), k % 8);

		// BUS Card
		for (uint8_t k = 16; k < 30; k++)
			ledBuffer[k] = 255;
		ioWrite(1, 255);
		for (uint8_t k = 16; k < 30; k++)
			addressWrite(((unsigned int) k / 8), k % 8);

		for (uint8_t k = 30; k < 34; k++)
			ledBuffer[k] = 0;
		ioWrite(1, 0);
		for (uint8_t k = 30; k < 34; k++)
			addressWrite(((unsigned int) k / 8), k % 8);

		ledBuffer[34] = 119;
		ioWrite(1, 119);
		addressWrite(4, 2);
	}

	prepareKeysIo();
}

void VPS::prepareKeysIo(void)
{
	if (keyPinsStateChanged == true)
	{
		//for (uint8_t i = 0; i < DGIOPINS; i++) pinMode(keyIoPins[i], INPUT);
		DDRC = B00000000;
		//for (uint8_t i = 0; i < DGIOPINS; i++) pinMode(busIoPins[i], INPUT);
		DDRL = B00000000;

		PORTA = (1 << PA4); //digitalWrite(key245BusTrPin, 1);

		PORTH = (1 << PH3); //digitalWrite(bus245BusTrPin, 1);

		keyPinsStateChanged = false;
	}
}

void VPS::prepareLedsIo(const bool& card)
{
	if (keyPinsStateChanged == false)
	{
		if (card == 0) // KEY card
		{
			//for (uint8_t i = 0; i < DGIOPINS; i++) pinMode(keyIoPins[i], OUTPUT);
			DDRC = B11111111;
			PORTA = (0 << PA4); //digitalWrite(key245BusTrPin, 0);
		}
		else // BUS card
		{
			//for (uint8_t i = 0; i < DGIOPINS; i++) pinMode(busIoPins[i], OUTPUT);
			DDRL = B11111111;
			PORTH = (0 << PH3); //digitalWrite(bus245BusTrPin, 0);
		}
		keyPinsStateChanged = true;
	}
}

uint16_t VPS::adcRead(const uint8_t& pinNum)
{
	//used for tBar and joystick
	return analogRead(pinNum);
}

bool VPS::pinRead(const uint8_t pinNum)
{
	//used for pots
	return digitalRead(pinNum);
}

uint8_t VPS::ioRead(const bool& card)
{
	uint8_t ret;
	if (card == 0)
		//for (uint8_t r = 0; r < DGIOPINS; r++) bitWrite(ret, r, digitalRead(keyIoPins[r]));
		ret = PINC;
	else
		//for (uint8_t r = 0; r < DGIOPINS; r++) bitWrite(ret, r, digitalRead(busIoPins[r]));
		ret = PINL;
	return ret;
}

void VPS::ioWrite(const bool& card, const uint8_t& buffer)
{
	if (card == 0) // KEY card
	{
		//for (uint8_t r = 0; r < 8; r++) digitalWrite(keyIoPins[r], bitRead(buffer, r));
		PORTC = buffer;
	}
	else // BUS card
	{
		//for (uint8_t r = 0; r < 8; r++) digitalWrite(busIoPins[r], bitRead(buffer, r));
		PORTL = buffer;
	}
}

void VPS::keyAddressWrite(const uint8_t& addr)
{
	PORTE = (bitRead(addr, 0) << PE3);
	PORTE = (bitRead(addr, 1) << PE4);
	PORTE = (bitRead(addr, 2) << PE5);
}

void VPS::busAddressWrite(const uint8_t& addr)
{
	PORTG = (bitRead(addr, 0) << PG0);
	PORTG = (bitRead(addr, 1) << PG1);
	PORTG = (bitRead(addr, 2) << PG2);
}

void VPS::addressWrite(const uint8_t& ic, const uint8_t& addr)
{
	if (ic < 2)
	{
		keyAddressWrite(addr);
		if (ic == 0)
		{
			PORTA = (1 << PA6);
			PORTA = (0 << PA6);
		}
		else if (ic == 1)
		{
			PORTA = (1 << PA7);
			PORTA = (0 << PA7);
		}
	}
	else if (ic > 1 && ic < 5)
	{
		busAddressWrite(addr);
		if (ic == 2)
		{
			PORTH = (1 << PH4);
			PORTH = (0 << PH4);
		}
		else if (ic == 3)
		{
			PORTH = (1 << PH5);
			PORTH = (0 << PH5);
		}
		else if (ic == 4)
		{
			PORTH = (1 << PH6);
			PORTH = (0 << PH6);
		}
	}
}

void VPS::keysGridOutputEnable(const uint8_t& ic)
{
	if (ic == 0)
	{//B00111110;
		PORTA = (1 << PA5); //digitalWrite(key138E3Pin, 1);
		PORTA = (0 << PA0); //digitalWrite(key245GridOEPins[0], 0);
	}
	else if (ic == 1)
	{//B00011101;
		PORTA = (0 << PA1); //digitalWrite(key245GridOEPins[1], 0);
	}
	else if (ic == 2)
	{//B00011011;
		PORTA = (0 << PA2); //digitalWrite(key245GridOEPins[2], 0);
	}
	else if (ic == 3)
	{//B00010111;
		PORTA = (0 << PA3); //digitalWrite(key245GridOEPins[3], 0);
	}
	else if (ic == 4)
	{//B10001101
		PORTH = (1 << PH0); //digitalWrite(bus138E3Pin, 1);
		PORTH = (0 << PH1); //digitalWrite(bus245GridOEPin, 0);
	}
}

void VPS::keysGridOutputDisable(const uint8_t& ic)
{
	if (ic < 4)
	{
		//digitalWrite(key138E3Pin, 0);
		//digitalWrite(key245GridOEPins[0], 1);
		//digitalWrite(key245GridOEPins[1], 1);
		//digitalWrite(key245GridOEPins[2], 1);
		//digitalWrite(key245GridOEPins[3], 1);
		PORTA = B00011111; 
	}
	else if (ic == 4)
	{//B10001110
		PORTH = (0 << PH0); //digitalWrite(bus138E3Pin, 0);
		PORTH = (1 << PH1); //digitalWrite(bus245GridOEPin, 1);
	}
}

#endif
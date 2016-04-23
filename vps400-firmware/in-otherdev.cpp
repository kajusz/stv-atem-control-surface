#include "VPS400.h"
#include <Arduino.h>

void VPS::detectFaderBar(void)
{
	uint16_t tBar = adcRead(tBarPotPin);

	if (tBar < 394)
		tBar = 394;
	else if (tBar > 664)
		tBar = 664;

	tBar = (tBar - 394) * 0.9445;

	if (tBar != tBarPosition)
	{
		tBarPosition = tBar;
		cueOtherDev(0, tBar);
	}
}

void VPS::detectEncoders(void)
{
	for (uint8_t p = 0; p < 4; p++)
	{
		bool sig1 = pinRead(potPinPairs[p * 2]);
		bool sig2 = pinRead(potPinPairs[p * 2 + 1]);
		int8_t curState = sig1 | (sig2 << 1);

		if (encState[p] != curState)
		{
			int8_t dir = encDir[curState | (encState[p] << 2)]; //-1 = acw, 1 = cw // change to bool
			if (curState == 3 && dir != 0) // LATCHSTATE 3
				cueOtherDev(p + 1, (uint8_t) dir);

			encState[p] = curState;
		}
	}
}

void VPS::detectJoystick(void)
{
	for (uint8_t j = 0; j < JOYSTICKPINS; j++)
	{
		uint16_t adcVal = adcRead(joystickXyzPins[j]);
		int8_t joyZero = ((adcVal / 4) - joystickShift[j]) / 8;
//limits
/*
-519 < x < 499
-508 < y < 511
-508 < z < 511
*/
		if (joyZero < 1 && joyZero > -1)
		{
			//dfa
		}
		else
		{
			if (joystickBuffer[j] != joyZero)
			{
				cueOtherDev(j + 5, (uint8_t) joyZero);
				joystickBuffer[j] = joyZero;
			}
		}
	}
}

void VPS::cueOtherDev(const uint8_t& adcNum, const uint8_t& data)
{
	uint8_t a = 38; // command for send adc
	uint8_t b = 64; // packet 2
	uint8_t c = 128;// packet 3

	b = b + (adcNum << 2) + ((data & 192) >> 6);
	c = c + (data & 63);

 	if (currentSpecial == 4)
		dataTxFn(a, b, c);
	else
	{
		txBufferCE++;
		txBuffer[txBufferCE].p1 = a;
		txBuffer[txBufferCE].p2 = b;
		txBuffer[txBufferCE].p3 = c;
	}
}
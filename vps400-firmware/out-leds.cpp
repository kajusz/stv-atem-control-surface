#include "VPS400.h"
#include <Arduino.h>

void VPS::allLedsOff(void)
{
			Serial1.println("alo");
	// KEY Card
	for (uint8_t k = 0; k < 16; k++)
		ledBuffer[k] = 255;
	ioWrite(0, 255);
	for (uint8_t k = 0; k < 16; k++)
		addressWrite(((unsigned int)k / 8), k % 8);

	// BUS Card
	for (uint8_t k = 16; k < 30; k++)
		ledBuffer[k] = 255;
	ioWrite(1, 255);
	for (uint8_t k = 16; k < 30; k++)
		addressWrite(((unsigned int)k / 8), k % 8);

	for (uint8_t k = 30; k < 34; k++)
		ledBuffer[k] = 0;
	ioWrite(1, 0);
	for (uint8_t k = 30; k < 34; k++)
		addressWrite(((unsigned int)k / 8), k % 8);

	ledBuffer[34] = 119;
	ioWrite(1, 119);
	addressWrite(4, 2);
}

void VPS::setLed(const uint8_t& state, const uint8_t& ledNum)
{
	/* State:
	7 = Off
	6 = Red
	5 = Green
	4 = Orange
	*/
	if (ledNum == 255)
	{
		if (state == 7)
			allLedsOff();
		else
		{
			Serial1.println("naughty");
			prepareLedsIo(0);
			uint8_t singleState = 255 * (~bitRead(state, 1) && bitRead(state, 0));
			uint8_t dualState = (state - 4) * 85;

			for (uint8_t r = 0; r < 2; r++)
				ledBuffer[r] = singleState;
			for (uint8_t r = 2; r < 8; r++)
				ledBuffer[r] = dualState;
			ledBuffer[8] = singleState;
			for (uint8_t r = 9; r < 13; r++)
				ledBuffer[r] = dualState;
			for (uint8_t r = 13; r < 16; r++)
				ledBuffer[r] = singleState;

			for (uint8_t k = 0; k < 16; k++)
			{
				ioWrite(0, ledBuffer[k]);
				addressWrite(((unsigned int)k / 8), k % 8);
			}

			prepareLedsIo(1);

			for (uint8_t k = 16; k < 30; k++)
				ledBuffer[k] = dualState;
			dualState = ~dualState;
			for (uint8_t k = 30; k < 34; k++)
				ledBuffer[k] = dualState;

			if (state == 5)
				ledBuffer[34] = 0;
			else
				ledBuffer[34] = 119;

			for (uint8_t k = 16; k < 35; k++)
			{
				ioWrite(1, ledBuffer[k]);
				addressWrite(((unsigned int)k / 8), k % 8);
			}
		}
	}
	else
	{
		uint8_t ic = 0;
		uint8_t data = 0;
		uint8_t cp = 0;
		uint8_t bufNum = 0;

		if (ledNum < KEYCARDSWITCHES) // KEY Card
		{
			prepareLedsIo(0);

			ic = (unsigned int) ledNum / 39; // 0 = IC4, 1 = IC12
			if ((ledNum > 15 && ledNum < 39) || (ledNum > 44 && ledNum < 60))
			{
				data = (ledNum - (45 * ic)) % 4;
				cp = (((ledNum - (45 * ic) - data) / 4) - (2 * !ic) + ic) % 8;

				data = data * 2;
				bufNum = cp + (ic * 8);

				bitWrite(ledBuffer[bufNum], data, bitRead(state, 0));
				bitWrite(ledBuffer[bufNum], data + 1, bitRead(state, 1));
			}
			else // if (ledNum < 16 || (ledNum > 39 && ledNum < 45) || (ledNum > 59 && ledNum < 76))
			{
				if (state == 7 || state == 5)
				{
					if (ledNum < 16)
					{
						data = ledNum % 8;
						cp = ledNum / 8; //((ledNum - data) / 8);
					}
					else if (ledNum == 40)
					{
						data = 4;
						cp = 0;
					}
					else if (ledNum > 40 && ledNum < 45)
					{
						data = (ledNum - 1) % 8;
						cp = 0; //((ledNum - 1 - data) / 8) - 5;
					}
					else if (ledNum > 59 && ledNum < 66)
					{
						data = (ledNum - 4) % 8;
						cp = 5; //((ledNum - 4 - data) / 8) - 2;
					}
					else if (ledNum > 65 && ledNum < 76)
					{
						data = (ledNum - 2) % 8;
						cp = ((ledNum - 2) / 8) - 2;//((ledNum - 2 - data) / 8) - 2;
					}
					else
						return;

					bufNum = cp + (ic * 8);
					bitWrite(ledBuffer[bufNum], data, bitRead(state, 1));
				}
				else
					return;
			}

			ioWrite(0, ledBuffer[bufNum]);
			addressWrite(ic, cp);
		}
		else if (ledNum >= KEYCARDSWITCHES && ledNum < TOTALLEDS) // BUS Card
		{
			prepareLedsIo(1);
			uint8_t num = ledNum - KEYCARDSWITCHES + 1;
			bufNum = KEYCARDFLIPFLOPS;

			if (ledNum > 75 && ledNum < 130)
			{
				ic = num / 32; // 0 = IC3, 1 = IC12
				data = (num - !ic) % 4;
				cp = ((num - !ic - data) / 4) % 8;
				data = data * 2;

				bufNum += cp + (ic * 8);
				bitWrite(ledBuffer[bufNum], data, bitRead(state, 0));
				bitWrite(ledBuffer[bufNum], data + 1, bitRead(state, 1));
			}
			else if ((num >= 55 && num <= 65) && ((num % 3) != 0))
			{//130, 131, 133, 134, 136, 137, 139, 140 = up, down, key1 on, key2 on,
				ic = (unsigned int) (num / 30); // 1 = IC12, 2 = IC21
				data = (num - 1) % 3;
				cp = ((num - data - 1) % 2) - (6 * ic) + 12;
				data = data * 2;

				bufNum += cp + (ic * 8);
				bitWrite(ledBuffer[bufNum], data, bitRead(~state, 0));
				bitWrite(ledBuffer[bufNum], data + 1, bitRead(~state, 1));
			}
			else if ((num == 67 || num == 68) && (state == 7 || state == 5)) //142, 143 (over)
			{
				ic = 2;
				cp = 2;
				data = (num - 67) * 4;

				bufNum += cp + (ic * 8);
				bitWrite(ledBuffer[bufNum], data, bitRead(state, 1));
				bitWrite(ledBuffer[bufNum], data + 1, bitRead(state, 1));
				bitWrite(ledBuffer[bufNum], data + 2, bitRead(state, 1));
			}

			ioWrite(1, ledBuffer[bufNum]);
			addressWrite(ic + 2, cp);
		}
	}
}

void VPS::flashLed(const uint8_t& state, const uint8_t& ledNum)
{
	/* State:
	3 = Flash Off
	2 = Flash Red
	1 = Flash Green
	0 = Flash Orange
	*/
}
#include "VPS400.h"

void VPS::doTasks()
{
// Process flash buffer













// Process serial buffer
	if (rxBufferCE > -1)
	{
		for (uint8_t i = 0; i <= rxBufferCE; i++)
		{
			if ((rxBuffer[i].p1 >> 4) == 1)
			{
				uint8_t cmd = rxBuffer[i].p1 & 15;
				
				uint8_t a = rxBuffer[i].p2;
				uint8_t b = rxBuffer[i].p3;
				
				switch (cmd)
				{
					case 1:
					{
						if (b == 39 || b == 132 || b == 135 || b == 138 || b == 141)
							break;
	
						if (bitRead(a, 2))
							setLed(a, b);
						else
							flashLed(a, b);
						break;
					}
					case 5:
					{
						doBuzzer(a, b);
						break;
					}
					case 11:
					{
						initIo();
						break;
					}
					case 13:
					{
						if (bitRead(a, 2))
						{
							currentSpecial = 0;
						}
						else
							doSpecials(bitRead(a, 0), b);
						break;
					}
					case 15:
					{
						uint8_t a = 62;
						uint8_t b = 64 + (VERSIONMAJOR << 2) + ((VERSIONMINOR & 192) >> 6);
						uint8_t c = 128 + (VERSIONMINOR & 63);

						txBufferCE++;
						txBuffer[txBufferCE].p1 = a;
						txBuffer[txBufferCE].p2 = b;
						txBuffer[txBufferCE].p3 = c;
						break;
					}
					default:
						break;
				}
			}
		}

		rxBufferCE = -1;
	}
}

void VPS::sendData(void)
{
	if (txBufferCE > -1)
	{
		for (uint8_t i = 0; i <= txBufferCE; i++)
			dataTxFn(txBuffer[i].p1, txBuffer[i].p2, txBuffer[i].p3);

		txBufferCE = -1;
	}
}

void VPS::doSpecials(const bool& xord, const uint8_t& bitMask)
{
	if (xord)
		currentSpecial = bitMask;
	else
		currentSpecial = currentSpecial ^ bitMask;
}

void VPS::registerSerialData(const uint8_t& b)
{
	uint8_t q = b >> 6;
	switch (q)
	{
		case 0:
		{
			quickBuff[0] = b & 63;
			break;
		}
		case 1:
		{
			quickBuff[1] = b & 63;
			break;
		}
		case 2:
		{
			uint8_t x = quickBuff[0];
			uint8_t y = quickBuff[1] >> 2;
			uint8_t z = ((quickBuff[1] & 3) << 6) + (b & 63);
			this->registerSerialData(x, y, z);
			break;
		}
		case 3:
		{
			break;
		}
	}
}

void VPS::registerSerialData(const uint8_t& p1, const uint8_t& p2, const uint8_t& p3)
{
	rxBufferCE++;
	rxBuffer[rxBufferCE].p1 = p1;
	rxBuffer[rxBufferCE].p2 = p2;
	rxBuffer[rxBufferCE].p3 = p3;
}
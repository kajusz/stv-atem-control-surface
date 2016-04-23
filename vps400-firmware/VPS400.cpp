#include "Arduino.h"
#include "VPS400.h"

void setup();
void loop();

void txData(const uint8_t& p1, const uint8_t& p2, const uint8_t& p3);

//debug buffer
uint8_t buff[3];
void debugOut(uint8_t cmd, uint8_t a, uint8_t b, bool rx);

VPS vps;

void setup()
{
// Comms
	Serial.begin(115200);

// Debug
	Serial1.begin(115200);
	Serial1.println("Setup done");

// Library
	vps.initIo();
//	analogReference(EXTERNAL);

	vps.registerSerialHandler(&txData);
}

void loop()
{
	if (Serial.available() > 0)
	{
		uint8_t q = Serial.read();
		vps.registerSerialData(q);

		switch (q >> 6)
		{
		case 0: buff[0] = q & 63; break;
		case 1: buff[1] = q & 63; break;
		case 2:
		{
			debugOut((buff[0] & 15), (buff[1] >> 2), (((buff[1] & 3) << 6) + (q & 63)), true);
			break;
		}
		}
	}

	vps.doTasks();

// Detect changes in controls
	vps.detectKeysGrid();
	vps.detectFaderBar();
	vps.detectEncoders();
	vps.detectJoystick();

// Send changes
	vps.sendData();
}

void txData(const uint8_t& p1, const uint8_t& p2, const uint8_t& p3)
{
	/*
	TX:
	v1
	|0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1|
	|0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5|
	|0|0|1|1|0| |sta|    swNum      |
	|0|1|1|0| |addr |    adcVal     |
	v2
	|0 0 0 0 0 0 0 0|0 0 1 1 1 1 1 1|1 1 1 1 2 2 2 2|
	|0 1 2 3 4 5 6 7|8 9 0 1 2 3 4 5|6 7 8 9 0 1 2 3|
	|0|0|1|0|0 0 1 1|0 1|0|state| > |1 0|   swNum   |
	|0|0|1|0|0 1 1 0|0 1|       | > |1 0|   adcVal  |
	|0|0|1|1|1 1 1 0|0 1| minor | > |1 0|   major   | send version
         ^ means 3 byte packet

	1: led
	...
	3: button
	...
	5: buzzer
	6: adc
	...
	11: reset
	12: connected
	13: config
	14: version
	15: version

	RX:
	v1
	|0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1|
	|0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5|
	|1|0|0|0|0|state|    ledNum     |
	|1|0|1|0|  dur  |    freq       |
	|1|1|1|1|0|r|0|*|    special    | * = 1 then don't xor
	v2
	|0 0 0 0 0 0 0 0|0 0 1 1 1 1 1 1|1 1 1 1 2 2 2 2|
	|0 1 2 3 4 5 6 7|8 9 0 1 2 3 4 5|6 7 8 9 0 1 2 3|
	|0|0|0|1|0 0 0 1|0 1|0|state| > |1 0|  ledNum   |
	|0|0|0|1|0 1 0 1|0 1|  dur  | > |1 0|   freq    |
	|0|0|0|1|1 0 1 1|0 1|1 1 1 1|1 1|1 0|1 1 1 1 1 1| reset
	|0|0|0|1|1 1 0 0|0 1|1 1 1 1|1 1|1 0|1 1 1 1 1 x| connect (enable transmitting)
	|0|0|0|1|1 1 0 1|0 1|0 r 0 x| > |1 0|  special  | r = reset config. x = xor
	|0|0|0|1|1 1 1 1|0 1|0 0 0 0|0 0|1 0|0 0 0 0 0 0| get version
	*/
	Serial.write(p1);
	Serial.write(p2);
	Serial.write(p3);

	debugOut((p1 & 15), ((p2 & 60) >> 2), (((p2 & 3) << 6) + (p3 & 63)), false);
}

void debugOut(uint8_t cmd, uint8_t a, uint8_t b, bool rx)
{
	if (rx)
	{
		Serial1.print("(RX) ");
		/*
	|0 0 0 0 0 0 0 0|0 0 1 1 1 1 1 1|1 1 1 1 2 2 2 2|
	|0 1 2 3 4 5 6 7|8 9 0 1 2 3 4 5|6 7 8 9 0 1 2 3|
	|0|0|0|1|0 0 0 1|0 1|0|state| > |1 0|  ledNum   |
	|0|0|0|1|0 1 0 1|0 1|  dur  | > |1 0|   freq    |
	|0|0|0|1|1 0 1 1|0 1|1 1 1 1|1 1|1 0|1 1 1 1 1 1| reset
	|0|0|0|1|1 1 0 1|0 1|0 r s x| > |1 0|  special  | r = reset config, s = set, x = xor
	|0|0|0|1|1 1 1 1|0 1|0 0 0 0|0 0|1 0|0 0 0 0 0 0| get version
	*/
		switch (cmd)
		{
		case 1://led
			Serial1.print("Led ");
			Serial1.print(b);
			Serial1.print(", state: ");
			switch (a)
			{
			case 7: Serial1.print("Off"); break;
			case 6: Serial1.print("Red"); break;
			case 5: Serial1.print("Green"); break;
			case 4: Serial1.print("Orange"); break;
			case 3: Serial1.print("Flash Off"); break;
			case 2: Serial1.print("Flash Red"); break;
			case 1: Serial1.print("Flash Green"); break;
			case 0: Serial1.print("Flash Orange"); break;
			}
			break;
		case 5://buzz
			Serial1.print("Buzz: freq ");
			Serial1.print(b);
			Serial1.print(", duration ");
			Serial1.print(a);
			break;
		case 11://reset
			if (a == 15 && b == 255)
			{
				Serial1.print("Reset request");
				break;
			}
		case 12:
			{
				Serial1.print("Serial: ");
				if (bitRead(b, 0))
					Serial1.print("enable");
				else
					Serial1.print("disable");
				
				break;
			}
		case 13://special
			if (a == 1)
			{
				Serial1.print("XOR Special ");
				Serial1.print(b);
				break;
			}
			else if (a == 2)
			{
				Serial1.print("Set Special to");
				Serial1.print(b);
				break;
			}
			else if (a == 4 && b == 0)
			{
				Serial1.print("Reset special");
				break;
			}
		case 15://version
			if (a == 0 && b == 0)
			{
				Serial1.print("Version request");
				break;
			}
		default:
			Serial1.print("Unknown (");
			Serial1.print(cmd, BIN);
			Serial1.print(", ");
			Serial1.print(a, BIN);
			Serial1.print(", ");
			Serial1.print(b, BIN);
			Serial1.print(")");
			break;
		}
	}
	else
	{
		Serial1.print("(TX) ");
		switch (cmd)
		{
		case 3://switch
			Serial1.print("Switch ");
			Serial1.print(b);
			Serial1.print(", state: ");
			if (a == 1)
				Serial1.print("KEYDOWN");
			else if (a == 2)
				Serial1.print("KEYHOLD");
			else if (a == 3)
				Serial1.print("KEYUP");
			break;
		case 6://adc
			Serial1.print("ADC ");
			Serial1.print(a);
			switch (a)
			{
			case 0:
				Serial1.print(" (tBar), position: ");
				Serial1.print(b);
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				Serial1.print(" (enc), direction: ");
				if ((int)b == 255)
					Serial1.print("CW");
				else if (b == 1)
					Serial1.print("ACW");

				Serial1.print(" (");
				Serial1.print(b);
				Serial1.print(")");
				break;
			case 5:
			case 6:
			case 7:
				if (a == 5)
					Serial1.print(" (joy X), step: ");
				else if (a == 6)
					Serial1.print(" (joy Y), step: ");
				else if (a == 7)
					Serial1.print(" (joy Z), step: ");

				Serial1.print((int8_t)b);
				Serial1.print(" (");
				Serial1.print(b);
				Serial1.print(")");
				break;
			}
			break;
		case 14://version
			Serial1.print("Version ");
			Serial1.print(a);
			Serial1.print(".");
			Serial1.print(b);
			break;
		default:
			Serial1.print("Unknown (");
			Serial1.print(cmd, BIN);
			Serial1.print(", ");
			Serial1.print(a, BIN);
			Serial1.print(", ");
			Serial1.print(b, BIN);
			Serial1.print(")");
			break;
		}
	}
	Serial1.println(".");
}

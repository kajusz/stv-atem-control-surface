#pragma once

uint8_t translateKeyNumToLedNum(uint8_t btnGp, uint8_t keyNum)
{
	// bounds
	assert(btnGp < 3);
	assert(keyNum < 22);

	// overlap
	if (keyNum > 10)
		keyNum = keyNum - 11;

	switch (btnGp)
	{
	case 2:
	{
		if (keyNum < 8)
			return 92 + keyNum;
		else
			return 107 + keyNum; // 115 -> 118
	}
	case 1:
	{
		if (keyNum < 8)
			return 84 + keyNum;
		else
			return 99 + keyNum; // 107 -> 110
	}
	case 0:
	{
		if (keyNum < 8)
			return 76 + keyNum;
		else
			return 92 + keyNum; // 100 -> 103
	}
	}
}

uint8_t translateInputIdToKeyNum(quint16 inputId)
{
	switch (inputId)
	{
	case 0: return 0;// Black
	case 1: return 1;// Input 1
	case 2: return 2;// Input 2
	case 3: return 3;// Input 3
	case 4: return 4;// Input 4
	case 5: return 5;// Input 5
	case 6: return 6;// Input 6
	case 7: return 7;// Input 7
	case 8: return 8;// Input 8
	case 1000: return 11; // Color Bars
	case 2001: return 9; // Color 1
	case 2002: return 20; // Color 2
	case 3010: return 10; // Media Player 1
	case 3020: return 21; // Media Player 2
	default:
		return 23;
	}
}

quint16 translateKeyNumToInputId(uint8_t keyNum)
{
	switch (keyNum)
	{
	case 0: return 0;// Black
	case 1: return 1;// Input 1
	case 2: return 2;// Input 2
	case 3: return 3;// Input 3
	case 4: return 4;// Input 4
	case 5: return 5;// Input 5
	case 6: return 6;// Input 6
	case 7: return 7;// Input 7
	case 8: return 8;// Input 8
	case 9: return 2001; // Color 1
	case 10: return 3010; // Media Player 1
	case 11: return 1000; // Color Bars
	case 12: return 1;// Input 1
	case 13: return 2;// Input 2
	case 14: return 3;// Input 3
	case 15: return 4;// Input 4
	case 16: return 5;// Input 5
	case 17: return 6;// Input 6
	case 18: return 7;// Input 7
	case 19: return 8;// Input 8
	case 20: return 2002; // Color 2
	case 21: return 3020; // Media Player 2
	default:
		return -1;
	}
}

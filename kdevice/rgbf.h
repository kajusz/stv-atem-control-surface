#pragma once

#include <cstdint>

enum colours
{
    OFF,
    RED,
    GREEN,
    ORANGE,
};

#define RGBQUICK(x, y, z) r = x; g = y; b = z;

struct rgbf
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    bool flash = false;
    rgbf() { RGBQUICK(0, 0, 0) }
    rgbf(uint8_t p_r, uint8_t p_g, uint8_t p_b) { RGBQUICK(p_r, p_g, p_b) }
    rgbf(colours colour, bool _flash = false)
    {
		flash = _flash;
        switch (colour)
        {
        case OFF:
            RGBQUICK(0, 0, 0)
            break;
        case RED:
            RGBQUICK(255, 0, 0)
            break;
        case GREEN:
            RGBQUICK(0, 255, 0)
            break;
        case ORANGE:
			RGBQUICK(255, 255, 0)
            break;
        }
    }
	rgbf(bool which, colours coltrue, colours colfalse = OFF)
	{
		colours colour;
		if (which)
		{
			colour = coltrue;
		}
		else
		{
			colour = colfalse;
		}

		switch (colour)
		{
		case OFF:
			RGBQUICK(0, 0, 0)
			break;
		case RED:
			RGBQUICK(255, 0, 0)
			break;
		case GREEN:
			RGBQUICK(0, 255, 0)
			break;
		case ORANGE:
			RGBQUICK(255, 255, 0)
			break;
		}
	}
	rgbf(bool addrem, const rgbf& base, const colours& col)
	{
		*this = rgbf(addrem, base, rgbf(col));
	}
	rgbf(bool addrem, const rgbf& base, const rgbf& col)
	{
		if (addrem) // add
		{
			r = base.r | col.r;
			g = base.g | col.g;
			b = base.b | col.b;

		}
		else // rem
		{
			if (base.r + base.g + base.b == 0)
			{
				RGBQUICK(0,0,0)
			}
			else
			{
				r = base.r ^ col.r;
				g = base.g ^ col.g;
				b = base.b ^ col.b;
			}
		}
	}
    rgbf(bool state)
    {
        if (state)
        {
            RGBQUICK(0, 0, 0)
        }
        else
        {
            RGBQUICK(0, 255, 0)
        }
    }
};

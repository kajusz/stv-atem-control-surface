#pragma once

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
    rgbf(colours colour)
    {
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
            RGBQUICK(255, 165, 0)
            break;
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

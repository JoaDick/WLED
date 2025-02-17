/**
 * Some utilities for making 2D WLED effect implementations easier.
 */

#include "FXutils2D.h"

//--------------------------------------------------------------------------------------------------

void PxMatrix::do_fill(PxColor color)
{
    for (int x = 0; x < _sizeX; ++x)
    {
        for (int y = 0; y < _sizeX; ++y)
        {
            setPixelColor(x, y, color);
        }
    }
}

void PxMatrix::do_fadeToBlackBy(uint8_t fadeBy)
{
    if (fadeBy)
    {
        for (int x = 0; x < _sizeX; ++x)
        {
            for (int y = 0; y < _sizeX; ++y)
            {
                setPixelColor(x, y, getPixelColor(x, y).fadeToBlackBy(fadeBy));
            }
        }
    }
}

void PxMatrix::do_fadeLightBy(uint8_t fadeBy)
{
    if (fadeBy)
    {
        for (int x = 0; x < _sizeX; ++x)
        {
            for (int y = 0; y < _sizeX; ++y)
            {
                setPixelColor(x, y, getPixelColor(x, y).fadeLightBy(fadeBy));
            }
        }
    }
}

void PxMatrix::do_fadeToColorBy(const PxColor color, uint8_t fadeBy)
{
    if (fadeBy)
    {
        for (int x = 0; x < _sizeX; ++x)
        {
            for (int y = 0; y < _sizeX; ++y)
            {
                setPixelColor(x, y, getPixelColor(x, y).fadeToColorBy(color, fadeBy));
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------

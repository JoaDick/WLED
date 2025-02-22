/**
 * Some utilities for making WLED effect implementations easier.
 */

#include "wled.h"

#include "FXutilsCore.h"

//--------------------------------------------------------------------------------------------------

#define FOO_ALT 1
namespace
{

  uint8_t fadeByte(uint8_t a, uint8_t b, int16_t deltaScaleFactor)
  {
    int16_t delta = 0;
    if (a != b)
    {
      const int16_t delta0 = int16_t(b) - int16_t(a);
      delta = (256 * delta0) / deltaScaleFactor;
      delta /= 256;
      if (delta == 0)
      {
        delta = (delta0 > 0) ? 1 : -1;
      }
    }
    return a + delta;
  }

}

//--------------------------------------------------------------------------------------------------

/*
 * color add function that preserves ratio
 * original idea: https://github.com/Aircoookie/WLED/pull/2465 by https://github.com/Proto-molecule
 * speed optimisations by @dedehai
 */
PxColor &PxColor::addColor(PxColor color, bool preserveCR)
{
#if (0)
  // TODO: compare against original implementation
  wrgb = color_add(wrgb, color, preserveCR);

#else
  wrgb = color_add(wrgb, color, preserveCR);

  // setPixelColor(index, color_add(getPixelColor(index), color, preserveCR));

  // _seg.addPixelColor(index, color, preserveCR);
  // void addPixelColor(int n, uint32_t color, bool preserveCR = true) { setPixelColor(n, color_add(getPixelColor(n), color, preserveCR)); }
#endif
  return *this;
}

/*
 * color blend function, based on FastLED blend function
 * the calculation for each color is: result = (A*(amountOfA) + A + B*(amountOfB) + B) / 256 with amountOfA = 255 - amountOfB
 */
PxColor &PxColor::blendColor(PxColor color, uint8_t blendAmount)
{
#if (0)
  // TODO: compare against FastLED's implementation
  wrgb = color_blend(wrgb, color, blendAmount);

#else
  wrgb = color_blend(wrgb, color, blendAmount);

// ToDo...
// setPixelColor(index, color_blend(getPixelColor(index), color, blend));

// _seg.blendPixelColor(index, color, blend);
// void blendPixelColor(int n, uint32_t color, uint8_t blend) { setPixelColor(n, color_blend(getPixelColor(n), color, blend)); }
#endif
  return *this;
}

/*
 * fades color toward black
 * if using "video" method the resulting color will never become black unless it is already black
 */
/// Fades pixel to black using nscale8()
PxColor &PxColor::fadeToBlackBy(uint8_t fadeBy)
{
#if (0)
  // TODO: compare against FastLED's implementation
  wrgb = color_fade(wrgb, 255 - fadeBy, false);

#else
  wrgb = color_fade(wrgb, 255 - fadeBy, false);

// setPixelColor(index, color_fade(getPixelColor(index), 255 - fadeBy, false));
// setPixelColor(index, color_fade(getPixelColor(index), fadeBy, false));

// _seg.fade???(index, fadeBy);

// FastLED - also fade_raw()
// nscale8( leds, num_leds, 255 - fadeBy);
#endif
  return *this;
}

/*
 * fades color toward black
 * if using "video" method the resulting color will never become black unless it is already black
 */
// name from FastLED
PxColor &PxColor::fadeLightBy(uint8_t fadeBy)
{
#if (0)
  // TODO: compare against FastLED's implementation
  wrgb = color_fade(wrgb, 255 - fadeBy, true);

#else
  wrgb = color_fade(wrgb, 255 - fadeBy, true);

// setPixelColor(index, color_fade(getPixelColor(index), 255 - fadeBy, true));
// setPixelColor(index, color_fade(getPixelColor(index), fadeBy, true));

// _seg.fadePixelColor(index, fadeBy);
// void fadePixelColor  (int index,              uint8_t fade) { setPixelColor  (index, color_fade(getPixelColor  (index), fade, true)); }
// void fadePixelColorXY(uint16_t x, uint16_t y, uint8_t fade) { setPixelColorXY(x, y,  color_fade(getPixelColorXY(x, y),  fade, true)); }

// FastLED - also fade_video()
// nscale8_video( leds, num_leds, 255 - fadeBy);
#endif
  return *this;
}

PxColor &PxColor::fadeToColorBy(PxColor color, uint8_t fadeBy)
{
  if (*this != color)
  {
    const int16_t deltaScaleFactor = 256 - fadeBy;
    *this = PxColor(fadeByte(r(), color.r(), deltaScaleFactor),
                    fadeByte(g(), color.g(), deltaScaleFactor),
                    fadeByte(b(), color.b(), deltaScaleFactor),
                    fadeByte(w(), color.w(), deltaScaleFactor));
  }
  return *this;
}

//--------------------------------------------------------------------------------------------------

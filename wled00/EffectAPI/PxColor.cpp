/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "PxColor.h"

//--------------------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------------

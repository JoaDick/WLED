/**
 * Some utilities for making 2D WLED effect implementations easier.
 */

#include "FXutils2D.h"

//--------------------------------------------------------------------------------------------------

void Wu_Pixel_f(PxMatrix &pxm, const NPoint &pos, PxColor color)
{
  // implement me properly!
  // see Segment::wu_pixel(uint32_t x, uint32_t y, CRGB c)
  pxm.setPixelColor(pxm.toAbs(pos), color);
}

#if (0)
#define WU_WEIGHT(a, b) ((uint8_t)(((a) * (b) + (a) + (b)) >> 8))
void Segment::wu_pixel(uint32_t x, uint32_t y, CRGB c)
{ // awesome wu_pixel procedure by reddit u/sutaburosu
  if (!isActive())
    return; // not active
  // extract the fractional parts and derive their inverses
  unsigned xx = x & 0xff, yy = y & 0xff, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (int i = 0; i < 4; i++)
  {
    int wu_x = (x >> 8) + (i & 1);        // precalculate x
    int wu_y = (y >> 8) + ((i >> 1) & 1); // precalculate y
    CRGB led = getPixelColorXY(wu_x, wu_y);
    CRGB oldLed = led;
    led.r = qadd8(led.r, c.r * wu[i] >> 8);
    led.g = qadd8(led.g, c.g * wu[i] >> 8);
    led.b = qadd8(led.b, c.b * wu[i] >> 8);
    if (led != oldLed)
      setPixelColorXY(wu_x, wu_y, led); // don't repaint if same color
  }
}
#undef WU_WEIGHT
#endif

//--------------------------------------------------------------------------------------------------

/**
 * Some utilities for making WLED effect implementations easier.
 */

#include "FXutils.h"
#include "FXutils1D.h"

//--------------------------------------------------------------------------------------------------

int makeDelta(uint8_t val1, uint8_t val2, uint8_t deltaScale)
{
  int delta = 0;
  if (val1 != val2)
  {
    const int fracDelta = (256 * (val2 - val1)) / deltaScale;
    delta = fracDelta / 256;
    // if fade isn't complete, make sure applied delta is at least 1 (fixes rounding issues)
    if (delta == 0)
    {
      delta = (fracDelta > 0) ? 1 : -1;
    }
  }
  return delta;
}

PxColor PxColor::fadeToColorBy(PxColor color, uint8_t fadeBy)
{
  if (wrgb != color.wrgb)
  {
    // fading algorithm adapted from Segment::fade_out()
    const uint8_t deltaScale = 255 - fadeBy;
    if (deltaScale > 0)
    {
      const auto w1 = W(wrgb);
      const auto r1 = R(wrgb);
      const auto g1 = G(wrgb);
      const auto b1 = B(wrgb);

      const auto w2 = W(color.wrgb);
      const auto r2 = R(color.wrgb);
      const auto g2 = G(color.wrgb);
      const auto b2 = B(color.wrgb);

      wrgb = PxColor(r1 + makeDelta(r1, r2, deltaScale),
                     g1 + makeDelta(g1, g2, deltaScale),
                     b1 + makeDelta(b1, b2, deltaScale),
                     w1 + makeDelta(w1, w2, deltaScale))
                 .wrgb;
    }
    else
    {
      wrgb = color.wrgb;
    }
  }
  return *this;
}

//--------------------------------------------------------------------------------------------------

uint16_t EffectRunner::showFallbackEffect(uint32_t now)
{
  WledPxArray pixels(seg);
  pixels.fill(config.fxColor());
  pixels.lineCentered(beatsin16(30, 0, seglen - 1), 0.2 * seglen, 0x100000);
  return 0;
}

//--------------------------------------------------------------------------------------------------

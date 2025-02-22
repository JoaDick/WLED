/**
 * Some utilities for making WLED effect implementations easier.
 */

#include "FXutils.h"

//--------------------------------------------------------------------------------------------------

uint16_t EffectRunner::showFallbackEffect(uint32_t now)
{
  WledPxArray pixels(seg);
  pixels.fill(config.fxColor());
  lineCentered(pixels, beatsin16(30, 0, pixels.size() - 1), 0.2 * pixels.size(), 0x100000);
  return 0;
}

//--------------------------------------------------------------------------------------------------

/**
 * Some utilities for making WLED effect implementations easier.
 */

#include "FXutils.h"
#include "FXutils1D.h"

//--------------------------------------------------------------------------------------------------

uint16_t EffectRunner::showFallbackEffect(uint32_t now)
{
  PxStrip pixels(seg);
  pixels.fill(config.fxColor());
  pixels.lineCentered(beatsin16(30, 0, seglen - 1), 0.2 * seglen, 0x100000);
  return 0;
}

//--------------------------------------------------------------------------------------------------

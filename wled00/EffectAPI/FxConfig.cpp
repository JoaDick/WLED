/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "FxConfig.h"

//--------------------------------------------------------------------------------------------------

uint8_t FxConfig::paletteBlend() const
{
  /* TODO: Which one is correct???
  ... From fx.h
  uint8_t WS2812FX::paletteBlend;

  ... and FX.cpp
  // paletteBlend: 0 - wrap when moving, 1 - always wrap, 2 - never wrap, 3 - none (undefined)
  #define PALETTE_SOLID_WRAP (strip.paletteBlend == 1 || strip.paletteBlend == 3)
  #define PALETTE_MOVING_WRAP !(strip.paletteBlend == 2 || (strip.paletteBlend == 0 && SEGMENT.speed == 0))

  ... or from wled.h
  WLED_GLOBAL uint8_t paletteBlend _INIT(0);        // determines blending and wrapping of palette: 0: blend, wrap if moving (SEGMENT.speed>0); 1: blend, always wrap; 2: blend, never wrap; 3: don't blend or wrap

  --> see https://github.com/wled/WLED/issues/5295
  */

#if (0)
  // This one seems abandoned; it isn't written anywhere.
  // --> But it is still used by the macros PALETTE_SOLID_WRAP and PALETTE_MOVING_WRAP (!?!)
  return strip.paletteBlend;
#else
  // This one seems to be the correct one; it is written in set.cpp
  return ::paletteBlend;
#endif
}

//--------------------------------------------------------------------------------------------------

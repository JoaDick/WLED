/**
 * Interfaces and utilities for creating class-based WLED effects.
 * @author Joachim Dick
 */

#include "wled.h"
#include "WledEffect.h"

uint16_t WledEffect::renderEffect(Segment &seg, uint32_t now)
{
  FxConfig configuration(seg);
  FxEnv runtimeEnvironment{now, configuration};

  // just a sanity check (out of pure curiosity)
  if (&seg != &_fxs.seg)
  {
    // somehow purple
    seg.fill(0x440022);
    return 0;
  }

  return showEffect(seg, runtimeEnvironment);
}

uint16_t WledEffect::showFallbackEffect(Segment &seg, FxEnv &env)
{
  // somehow red
  seg.fill(0x440000);
  return 0;
}

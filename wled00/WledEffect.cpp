/**
 * Interfaces and utilities for creating class-based WLED effects.
 * @author Joachim Dick
 */


#include "wled.h"
#include "WledEffect.h"


WledEffect::WledEffect(Segment& seg)
{
  std::ignore = seg; // might later become useful
}


uint16_t WledEffect::render_function(Segment& seg, uint32_t now, uint16_t defaultFrametime)
{
  if(!seg.effect) {
    seg.fill(seg.getCurrentColor(0));
    return defaultFrametime;
  }
  const uint16_t frametime = seg.effect->renderEffect(seg, now);
  return frametime ? frametime : defaultFrametime;
}

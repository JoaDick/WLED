/**
 * Interface and helpers for creating class-based WLED effects.
 *
 * (c) 2025 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "WledEffect.h"

//--------------------------------------------------------------------------------------------------

uint16_t WledFxBase::showWledEffect(FxEnv &env)
{
  // Must effect adapt to a changed Segment?
  if (&env.seg() != _seg)
  {
    _seg = &env.seg();
    _seglen = _seg->vLength();
    _segW = _seg->vWidth();
    _segH = _seg->vHeight();
    if (onSegmentChanged(env) == false)
    {
      return pleaseKillMe(env);
    }
  }

  // Must effect adapt to changed Segment dimensions?
  if (env.seglen() != _seglen ||
      env.segW() != _segW ||
      env.segH() != _segH)
  {
    _seglen = _seg->vLength();
    _segW = _seg->vWidth();
    _segH = _seg->vHeight();
    if (onSegmentDimensionChanged(env) == false)
    {
      return pleaseKillMe(env);
    }
  }

  if (env.seg().call == 0)
  {
    initEffect(env);
  }
  return showEffect(env);
}

bool WledFxBase::onSegmentChanged(FxEnv &) { return true; }
bool WledFxBase::onSegmentDimensionChanged(FxEnv &) { return true; }
void WledFxBase::initEffect(FxEnv &) {}

//--------------------------------------------------------------------------------------------------

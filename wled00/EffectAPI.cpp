/**
 * Interfaces and helper classes for creating class-based effects.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "EffectAPI.h"

//--------------------------------------------------------------------------------------------------

void FxEnv::showFallbackEffect()
{
  seg().fill(seg().getCurrentColor(0));
  setBroken();
}

uint16_t FxEnv::showEffect(uint32_t now)
{
  updateTime(now);
  if (_effect)
  {
    _effect->show(*this);
  }
  else
  {
    showFallbackEffect();
  }
  return _frametime;
}

bool FxEnv::updateSegment(Segment &seg)
{
#if (1)
  const uint16_t new_seglen = seg.virtualLength();
  const uint16_t new_segW = seg.virtualWidth();
  const uint16_t new_segH = seg.virtualHeight();
#else
  const uint16_t new_seglen = seg.vLength();
  const uint16_t new_segW = seg.vWidth();
  const uint16_t new_segH = seg.vHeight();
#endif
  const bool new_is2D = seg.is2D();

  const bool dimensionChanged = (_seglen != new_seglen) ||
                                (_segW != new_segW) ||
                                (_segH != new_segH) ||
                                (_is2D != new_is2D);

  _seglen = new_seglen;
  _segW = new_segW;
  _segH = new_segH;
  _is2D = new_is2D;
  _seg = &seg;

  return dimensionChanged;
}

void FxEnv::updateTime(uint32_t now)
{
  _deltaT = now - _now;
  _age += _deltaT;
  _now = now;
}

//--------------------------------------------------------------------------------------------------

EffectBase::EffectBase(FxSetup &fxs) { fxs.env.seg().fill(0); }

void EffectBase::show(FxEnv &env)
{
  if (env.isBroken())
  {
    env.showFallbackEffect();
  }
  else
  {
    showEffect(env);
  }
}

//--------------------------------------------------------------------------------------------------

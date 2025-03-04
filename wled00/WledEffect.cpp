/**
 * Interface and helpers for creating class-based WLED effects.
 *
 * (c) 2025 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "WledEffect.h"
#include "../usermods/EffectProfiler/EffectProfilerTrigger.h"

//--------------------------------------------------------------------------------------------------

uint16_t WledEffect::render_function(Segment &seg, uint32_t now, uint16_t defaultFrametime)
{
  EffectProfilerTrigger profiler;
  const uint16_t frametime = seg.effect->render(seg, now);
  return frametime ? frametime : defaultFrametime;
}

//--------------------------------------------------------------------------------------------------

uint16_t WledFxBase::showWledEffect(FxEnv &env)
{
  // We have to be recreated from scratch when any essential property of the Segment has changed
  // during runtime. This is the case e.g. when "Mirror effect" setting is changed.
  if (mustRecreate(env))
  {
    // kill current effect; a new one will be created upon the next frame
    env.seg().effect.reset();
    // don't change the Segment during this frame
    return 0;
  }

  if (env.seg().call == 0)
  {
    initEffect(env);
  }
  return showEffect(env);
}

void WledFxBase::initEffect(FxEnv &) {}

bool WledFxBase::mustRecreate(const FxEnv &env) const
{
  if (env.seglen() != _seglen ||
      env.segW() != _segW ||
      env.segH() != _segH)
  {
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------

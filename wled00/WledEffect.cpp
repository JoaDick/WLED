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
  if (mustRecreate(env))
  {
    // We have to be recreated from scratch when any essential property of the Segment has changed
    // during runtime. This is the case e.g. when "Mirror effect" setting is changed.
    return pleaseKillMe(env);
  }
  if (env.seg().call == 0)
  {
    initEffect(env);
  }
  return showEffect(env);
}

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

void WledFxBase::initEffect(FxEnv &) {}

//--------------------------------------------------------------------------------------------------

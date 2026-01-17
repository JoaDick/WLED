/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "EffectBase.h"

//--------------------------------------------------------------------------------------------------

EffectBase::EffectBase(FxSetup &fxs)
{
  FxEnv &env = fxs.env();
  Segment &seg = env.seg();
  seg.clear();
}

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

/**
 * Helper classes for integrating class-based effects into the WLED framework.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "EffectAdapter.h"

//--------------------------------------------------------------------------------------------------

bool EffectController::updateSegment(Segment &seg)
{
  const bool dimensionChanged = FxEnv::updateSegment(seg);
  if ((dimensionChanged == true) && (FxProperties::_isResizeSupportEnabled == false))
  {
    FxEnv::setBroken();
  }
  if ((FxEnv::is2D() == false) && (FxProperties::_isRequired2D == true))
  {
    FxEnv::setBroken();
  }
  return !FxEnv::isBroken();
}

//--------------------------------------------------------------------------------------------------

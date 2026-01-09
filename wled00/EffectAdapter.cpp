/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "EffectAdapter.h"

//--------------------------------------------------------------------------------------------------
// class EffectController

bool EffectController::updateSegment(Segment &seg)
{
  SegEnv::updateSegment(seg);
  const bool dimensionChanged = FxEnv::updateSegment(seg, *this);
  if ((dimensionChanged == true) && (FxProperties::_isSupported_SegmentResize == false))
  {
    FxEnv::setBroken();
  }
  if ((FxEnv::is2D() == false) && (FxProperties::_isRequired_2D == true))
  {
    FxEnv::setBroken();
  }
  return !FxEnv::isBroken();
}

//--------------------------------------------------------------------------------------------------
// class EffectAdapter

EffectAdapterPtr EffectAdapter::clone(Segment &seg)
{
  EffectAdapterPtr retval = do_clone();
  if (retval)
  {
    retval->updateSegment(seg);
  }
  return retval;
}

//--------------------------------------------------------------------------------------------------

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
  if ((dimensionChanged == true) && (FxProperties::_isSupported_segmentResize == false))
  {
    FxEnv::setBroken();
  }

  if ((FxProperties::_required_minSeglen != 0) && (FxEnv::seglen() < FxProperties::_required_minSeglen))
  {
    FxEnv::setBroken();
  }
  if ((FxProperties::_required_is2D == true) && (FxEnv::is2D() == false))
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

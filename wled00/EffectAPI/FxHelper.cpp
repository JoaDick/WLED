/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "FxHelper.h"

//--------------------------------------------------------------------------------------------------

bool PeriodicTrigger::check(uint32_t now)
{
  if (_delta != 0 && now < _triggerTime)
    return false;
  _triggerTime += _delta;
  if (_triggerTime <= now)
    _triggerTime = now + _delta;
  return true;
}

//--------------------------------------------------------------------------------------------------

/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "EffectHandle.h"
#include "EffectAdapter.h"

//--------------------------------------------------------------------------------------------------
// class EffectHandle

uint16_t EffectHandle::showEffect(uint32_t now)
{
  if (!_fxAdapter)
  {
    return 0;
  }
  const uint16_t frametime = _fxAdapter->showEffect(now);
  return frametime ? frametime : FRAMETIME;
}

bool EffectHandle::onSegmentChanges()
{
  if (_fxAdapter && (_fxAdapter->updateSegment(*_fxData.seg) == false))
  {
    reset();
    return false;
  }
  return true;
}

void EffectHandle::cloneEffectFrom(const EffectHandle &src)
{
  reset();
  if (src._fxAdapter)
  {
    _fxAdapter = src._fxAdapter->clone(*_fxData.seg);
  }
}

void EffectHandle::moveEffectFrom(EffectHandle &src) noexcept
{
  _fxAdapter = std::move(src._fxAdapter);
  updateSegment(*_fxData.seg);
}

void EffectHandle::swap(EffectHandle &other) noexcept
{
  std::swap(_fxAdapter, other._fxAdapter);
  updateSegment(*_fxData.seg);
  other.updateSegment(*other._fxData.seg);
}

void EffectHandle::updateSegment(Segment &seg)
{
  _fxData.seg = &seg;
  onSegmentChanges();
}

void EffectHandle::adjustAfterRawCopy(Segment &newSeg)
{
  Segment &orgSeg = *_fxData.seg;
  adjustAfterRawMove(newSeg);
  cloneEffectTo(orgSeg._effectHandle);
}

void EffectHandle::adjustAfterRawMove(Segment &newSeg)
{
  // release the unique_ptr at the original Segment's handle; the effect is now owned by this handle
  _fxData.seg->_effectHandle._fxAdapter.release();
  updateSegment(newSeg);
}

//--------------------------------------------------------------------------------------------------

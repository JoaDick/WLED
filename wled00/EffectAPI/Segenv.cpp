/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "Segenv.h"

//--------------------------------------------------------------------------------------------------

size_t Segenv::_allDataSize = 0;

Segenv::Segenv(const Segenv &other)
    : step{other.step}, _call{other._call}
{
  if (this != &other)
  {
    memcpy(buffer, other.buffer, sizeof(buffer));
    if (other._dataSize)
    {
      if (allocateData(other._dataSize))
      {
        memcpy(_data, other._data, _dataSize);
      }
    }
  }
}

bool Segenv::allocateData(size_t size)
{
  // Note: Checking for size == 0 is intentionally omitted (no pointless paranoia checks).
  // --> We can always successfully allocate zero bytes :-)

  // not enough memory allocated yet?
  if (_dataSize < size)
  {
    deallocateData();

#ifndef BOARD_HAS_PSRAM
    // limit to MAX_SEGMENT_DATA if there is no PSRAM, otherwise prefer functionality over speed
    if (Segment::getUsedSegmentData() + _allDataSize + size > MAX_SEGMENT_DATA)
    {
      DEBUG_PRINTF_P(PSTR("Segenv: SegmentData limit reached [%d/%d/%d]\n"), size, Segment::getUsedSegmentData(), _allDataSize);
      errorFlag = ERR_NORAM;
      onSegenvAllocFailed();
      return false;
    }
#endif

    // prefer DRAM over PSRAM for speed
    _data = allocate_buffer(size, BFRALLOC_PREFER_DRAM | BFRALLOC_CLEAR);
    if (_data == nullptr)
    {
      DEBUG_PRINTF_P(PSTR("Segenv %p: Allocate failed [%d/%d/%d]\n"), this, size, Segment::getUsedSegmentData(), _allDataSize);
      errorFlag = ERR_NORAM;
      DEBUG_PRINTLN(F("!!! Segenv: Allocation failed. !!!"));
      onSegenvAllocFailed();
      return false;
    }

    _dataSize = size;
    _allDataSize += _dataSize;
    // DEBUG_PRINTF_P(PSTR("Segenv %p: Allocated [%d] @ %p\n"), this, _dataSize, _data);
  }

  return true;
}

void Segenv::deallocateData()
{
  if (_dataSize)
  {
    // DEBUG_PRINTF_P(PSTR("Segenv %p: Deallocate [%d] @ %p\n"), this, _dataSize, _data);
    d_free(_data);
    _allDataSize -= _dataSize;
    _dataSize = 0;
    _data = nullptr;
  }
}

void Segenv::reset()
{
  deallocateData();
  _call = 0;
  step = 0;
  memset(buffer, 0, sizeof(buffer));
}

//--------------------------------------------------------------------------------------------------

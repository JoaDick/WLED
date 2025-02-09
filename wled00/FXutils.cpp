/**
 * Some utilities for making WLED effect implementations easier.
 */

#include "FXutils.h"

//--------------------------------------------------------------------------------------------------

uint16_t EffectRunner::showFallbackEffect(uint32_t now)
{
  seg.fill(config.fxColor());
  for (unsigned i = 0; i < seglen; ++i)
  {
    if ((i & 0xC) == 0xC)
    {
      seg.setPixelColor(i, 0xFF0000);
    }
  }
  return 0;
}

//--------------------------------------------------------------------------------------------------

void PxArray::lineAbs(int first, int last, PxColor color)
{
  constrainIndex(first);
  constrainIndex(last);

  while (first < last)
  {
    setColor(first++, color);
  }
  while (first > last)
  {
    setColor(first--, color);
  }
  setColor(last, color);
}

void PxArray::lineRel(int start, int length, PxColor color)
{
  if (length > 0)
  {
    lineAbs(start, start + length - 1, color);
  }
  else if (length < 0)
  {
    lineAbs(start, start + length + 1, color);
  }
}

bool PxArray::constrainIndex(int &index) const
{
  if (index < 0)
  {
    index = 0;
    return true;
  }
  if (index >= _size)
  {
    index = _size;
    return true;
  }
  return false;
}

/**
 * Some utilities for making 1D WLED effect implementations easier.
 */

#include "FXutils1D.h"

//--------------------------------------------------------------------------------------------------

void PxArray::lineAbs(int first, int last, PxColor color)
{
  constrainIndex(first);
  constrainIndex(last);

  while (first < last)
  {
    setPixelColor(first++, color);
  }
  while (first > last)
  {
    setPixelColor(first--, color);
  }
  setPixelColor(last, color);
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

PxArray &PxArray::operator=(const PxArray &other)
{
  if (this != &other)
  {
    const int sz = min(_size, other._size);
    for (int i = 0; i < sz; ++i)
    {
      setPixelColor(i, other.getPixelColor(i));
    }
  }
  return *this;
}

void PxArray::do_fadeToBlackBy(uint8_t fadeBy)
{
  if (fadeBy)
  {
    for (int i = 0; i < _size; ++i)
    {
      fadePixelToBlackBy(i, fadeBy);
    }
  }
}

void PxArray::do_fadeLightBy(uint8_t fadeBy)
{
  if (fadeBy)
  {
    for (int i = 0; i < _size; ++i)
    {
      fadePixelLightBy(i, fadeBy);
    }
  }
}

//--------------------------------------------------------------------------------------------------

/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include <algorithm>

#include "wled.h"
#include "FxUtils1D.h"

//--------------------------------------------------------------------------------------------------

void PxArray::fillBlock(AIndex firstPos, AIndex lastPos, PxColor color)
{
  if (firstPos > lastPos)
    std::swap(firstPos, lastPos);

  if (firstPos >= size())
    return;
  if (lastPos < 0)
    return;

  if (firstPos < 0)
    firstPos = 0;
  if (lastPos >= size())
    lastPos = size() - 1;

  do_fillBlock(firstPos, lastPos, color);
}

void PxArray::copyFrom(const PxArray &other)
{
  if (this != &other)
  {
    const AIndex count = min(_size, other._size);
    for (AIndex pos = 0; pos < count; ++pos)
    {
      setColor(pos, other.getColor(pos));
    }
  }
}

void PxArray::do_fillBlock(AIndex firstPos, AIndex lastPos, PxColor color)
{
  while (firstPos <= lastPos)
    setColor(firstPos++, color);
}

void PxArray::do_fastScale(uint8_t scale)
{
  if (scale == 0)
  {
    fill(0);
  }
  else if (scale == 255)
  {
    // nothing to do
  }
  else
  {
    for (AIndex pos = 0; pos < _size; ++pos)
      setColor(pos, getColor(pos).fastScale(scale));
  }
}

void PxArray::do_fade(uint8_t fadeBy, bool video)
{
  if (fadeBy)
    for (AIndex pos = 0; pos < _size; ++pos)
      setColor(pos, getColor(pos).fade(fadeBy, video));
}

void PxArray::do_fadeToColorBy(const PxColor color, uint8_t fadeBy)
{
  if (fadeBy)
    for (AIndex pos = 0; pos < _size; ++pos)
      setColor(pos, getColor(pos).fadeToColorBy(color, fadeBy));
}

void PxArray::do_addColor(PxColor color, bool preserveCR)
{
  for (AIndex pos = 0; pos < _size; ++pos)
    setColor(pos, getColor(pos).addColor(color, preserveCR));
}

void PxArray::do_blendColor(PxColor color, uint8_t blend)
{
  for (AIndex pos = 0; pos < _size; ++pos)
    setColor(pos, getColor(pos).blendColor(color, blend));
}

void PxArray::do_blur(uint8_t blurAmount, bool smear)
{
  // implement me
  // see Segment::blur()
}

//--------------------------------------------------------------------------------------------------

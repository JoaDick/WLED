/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include <algorithm>

#include "wled.h"
#include "FxUtils1D.h"

//--------------------------------------------------------------------------------------------------

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
  if (constrainRange(*this, firstPos, lastPos))
    while (firstPos <= lastPos)
      setColor(firstPos++, color);
}

void PxArray::do_fastFade(uint8_t fadeBy)
{
  if (fadeBy == 0)
  {
    // nothing to do
  }
  else if (fadeBy == 255)
  {
    clear();
  }
  else
  {
    for (AIndex pos = 0; pos < _size; ++pos)
      setColor(pos, getColor(pos).fastFade(fadeBy));
  }
}

void PxArray::do_fade(uint8_t fadeBy, bool video)
{
  if (fadeBy == 0)
  {
    // nothing to do
  }
  else if (fadeBy == 255)
  {
    clear();
  }
  else
  {
    for (AIndex pos = 0; pos < _size; ++pos)
      setColor(pos, getColor(pos).fade(fadeBy, video));
  }
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

bool constrainRange(const PxArray &pxa, AIndex &firstPos, AIndex &lastPos)
{
  if (firstPos > lastPos)
    std::swap(firstPos, lastPos);

  if (firstPos >= pxa.size())
    return false;
  if (lastPos < 0)
    return false;

  if (firstPos < 0)
    firstPos = 0;
  if (lastPos >= pxa.size())
    lastPos = pxa.size() - 1;

  return true;
}

//--------------------------------------------------------------------------------------------------

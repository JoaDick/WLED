/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include <algorithm>

#include "wled.h"
#include "PxArray.h"

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

// ported from Segment::blur()
void PxArray::do_blur(uint8_t blur_amount, bool smear)
{
  if (blur_amount == 0)
    return; // optimization: 0 means "don't blur"

  const uint8_t keep = smear ? 255 : 255 - blur_amount;
  const uint8_t seep = blur_amount >> 1;

  // handle first pixel to avoid conditional in loop (faster)
  PxColor cur = do_getColor(0);
  PxColor carryover = fastFadeColor(cur, seep);
  do_setColor(0, fastFadeColor(cur, keep));

  for (int i = 1; i < _size; ++i)
  {
    cur = do_getColor(i);
    const PxColor part = fastFadeColor(cur, seep);

    cur.fastFade(keep);
    cur.addColor(carryover);
    do_setColor(i - 1, do_getColor(i - 1).addColor(part)); // previous pixel
    do_setColor(i, cur);                                   // current pixel
    carryover = part;
  }

#if (0) // original from Segment::blur()
  uint8_t keep = smear ? 255 : 255 - blur_amount;
  uint8_t seep = blur_amount >> 1;
  unsigned vlength = vLength();
  // handle first pixel to avoid conditional in loop (faster)
  uint32_t cur = getPixelColorRaw(0);
  uint32_t carryover = fast_color_scale(cur, seep);
  setPixelColorRaw(0, fast_color_scale(cur, keep));
  for (unsigned i = 1; i < vlength; i++)
  {
    cur = getPixelColorRaw(i);
    uint32_t part = fast_color_scale(cur, seep);
    cur = fast_color_scale(cur, keep);
    cur = color_add(cur, carryover);
    setPixelColorRaw(i - 1, color_add(getPixelColorRaw(i - 1), part)); // previous pixel
    setPixelColorRaw(i, cur);                                          // current pixel
    carryover = part;
  }
#endif
}

void PxArray::do_rotate(int delta)
{
  while (delta > 0)
  {
    rotateUp();
    --delta;
  }
  while (delta < 0)
  {
    rotateDown();
    ++delta;
  }
}

void PxArray::rotateUp()
{
  const auto carry = do_getColor(_size - 1);
  AIndex src = _size - 2;
  AIndex dst = _size - 1;
  while (dst > 0)
    do_setColor(dst--, do_getColor(src--));
  do_setColor(0, carry);
}

void PxArray::rotateDown()
{
  const auto carry = do_getColor(0);
  AIndex src = 1;
  AIndex dst = 0;
  while (src < _size)
    do_setColor(dst++, do_getColor(src++));
  do_setColor(_size - 1, carry);
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

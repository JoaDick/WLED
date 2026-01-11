/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "FxUtils2D.h"

//--------------------------------------------------------------------------------------------------

void PxMatrix::do_fill(PxColor color)
{
#if (1)
  auto fct = [&color](PxMatrix &matrix, APoint pos)
  { matrix.setColor(pos, color); };
  applyToAllPixel(*this, fct);
#else
  APoint pos;
  for (pos.x = 0; pos.x < _sizeX; ++pos.x)
    for (pos.y = 0; pos.y < _sizeY; ++pos.y)
      setColor(pos, color);
#endif
}

void PxMatrix::do_fastFade(uint8_t fadeBy)
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
#if (1)
    auto fct = [fadeBy](PxMatrix &matrix, APoint pos)
    { matrix.setColor(pos, matrix.getColor(pos).fastFade(fadeBy)); };
    applyToAllPixel(*this, fct);
#else
    APoint pos;
    for (pos.x = 0; pos.x < _sizeX; ++pos.x)
      for (pos.y = 0; pos.y < _sizeY; ++pos.y)
        setColor(pos, getColor(pos).fastFade(fadeBy));
#endif
  }
}

void PxMatrix::do_fade(uint8_t fadeBy, bool video)
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
#if (1)
    auto fct = [fadeBy, video](PxMatrix &matrix, APoint pos)
    { matrix.setColor(pos, matrix.getColor(pos).fade(fadeBy, video)); };
    applyToAllPixel(*this, fct);
#else
    APoint pos;
    for (pos.x = 0; pos.x < _sizeX; ++pos.x)
      for (pos.y = 0; pos.y < _sizeY; ++pos.y)
        setColor(pos, getColor(pos).fade(fadeBy, video));
#endif
  }
}

void PxMatrix::do_fadeToColorBy(const PxColor color, uint8_t fadeBy)
{
  if (fadeBy)
  {
#if (1)
    auto fct = [color, fadeBy](PxMatrix &matrix, APoint pos)
    { matrix.setColor(pos, matrix.getColor(pos).fadeToColorBy(color, fadeBy)); };
    applyToAllPixel(*this, fct);
#else
    APoint pos;
    for (pos.x = 0; pos.x < _sizeX; ++pos.x)
      for (pos.y = 0; pos.y < _sizeY; ++pos.y)
        setColor(pos, getColor(pos).fadeToColorBy(color, fadeBy));
#endif
  }
}

void PxMatrix::do_addColor(PxColor color, bool preserveCR)
{
#if (1)
  auto fct = [color, preserveCR](PxMatrix &matrix, APoint pos)
  { matrix.setColor(pos, matrix.getColor(pos).addColor(color, preserveCR)); };
  applyToAllPixel(*this, fct);
#else
  APoint pos;
  for (pos.x = 0; pos.x < _sizeX; ++pos.x)
    for (pos.y = 0; pos.y < _sizeY; ++pos.y)
      setColor(pos, getColor(pos).addColor(color, preserveCR));
#endif
}

void PxMatrix::do_blendColor(PxColor color, uint8_t blend)
{
#if (1)
  auto fct = [color, blend](PxMatrix &matrix, APoint pos)
  { matrix.setColor(pos, matrix.getColor(pos).blendColor(color, blend)); };
  applyToAllPixel(*this, fct);
#else
  APoint pos;
  for (pos.x = 0; pos.x < _sizeX; ++pos.x)
    for (pos.y = 0; pos.y < _sizeY; ++pos.y)
      setColor(pos, getColor(pos).blendColor(color, blend));
#endif
}

void PxMatrix::do_blurXY(uint8_t blurAmountX, uint8_t blurAmountY, bool smear)
{
  if (blurAmountX)
    for (AIndex pos = 0; pos < _sizeY; ++pos)
      row(pos).blur(blurAmountX, smear);
  if (blurAmountY)
    for (AIndex pos = 0; pos < _sizeX; ++pos)
      column(pos).blur(blurAmountY, smear);
}

//--------------------------------------------------------------------------------------------------

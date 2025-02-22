/**
 * Some utilities for making 1D WLED effect implementations easier.
 */

#include "FXutils1D.h"

//--------------------------------------------------------------------------------------------------

void AA_Pixel(PxArray &pxa, NIndex pos, PxColor color)
{
  // implement me properly!
  pxa.setPixelColor_n(pos, color);

  // https://github.com/wled/WLED/pull/2465#issuecomment-1237849589
  // https://www.youtube.com/watch?v=RF7GekbNmjU

#ifdef WLED_USE_AA_PIXELS
  // see void Segment::setPixelColor(float i, uint32_t col, bool aa) const
#endif
}

#if (0)
// anti-aliased normalized version of setPixelColor()
void Segment::setPixelColor(float i, uint32_t col, bool aa) const
{
  if (!isActive())
    return;                    // not active
  int vStrip = int(i / 10.0f); // hack to allow running on virtual strips (2D segment columns/rows)
  i -= int(i);

  if (i < 0.0f || i > 1.0f)
    return; // not normalized

  float fC = i * (virtualLength() - 1);
  if (aa)
  {
    unsigned iL = roundf(fC - 0.49f);
    unsigned iR = roundf(fC + 0.49f);
    float dL = (fC - iL) * (fC - iL);
    float dR = (iR - fC) * (iR - fC);
    uint32_t cIL = getPixelColor(iL | (vStrip << 16));
    uint32_t cIR = getPixelColor(iR | (vStrip << 16));
    if (iR != iL)
    {
      // blend L pixel
      cIL = color_blend(col, cIL, uint8_t(dL * 255.0f));
      setPixelColor(iL | (vStrip << 16), cIL);
      // blend R pixel
      cIR = color_blend(col, cIR, uint8_t(dR * 255.0f));
      setPixelColor(iR | (vStrip << 16), cIR);
    }
    else
    {
      // exact match (x & y land on a pixel)
      setPixelColor(iL | (vStrip << 16), col);
    }
  }
  else
  {
    setPixelColor(int(roundf(fC)) | (vStrip << 16), col);
  }
}

// anti-aliased version of setPixelColorXY()
void Segment::setPixelColorXY(float x, float y, uint32_t col, bool aa) const
{
  if (!isActive())
    return; // not active
  if (x < 0.0f || x > 1.0f || y < 0.0f || y > 1.0f)
    return; // not normalized

  float fX = x * (vWidth() - 1);
  float fY = y * (vHeight() - 1);
  if (aa)
  {
    unsigned xL = roundf(fX - 0.49f);
    unsigned xR = roundf(fX + 0.49f);
    unsigned yT = roundf(fY - 0.49f);
    unsigned yB = roundf(fY + 0.49f);
    float dL = (fX - xL) * (fX - xL);
    float dR = (xR - fX) * (xR - fX);
    float dT = (fY - yT) * (fY - yT);
    float dB = (yB - fY) * (yB - fY);
    uint32_t cXLYT = getPixelColorXY(xL, yT);
    uint32_t cXRYT = getPixelColorXY(xR, yT);
    uint32_t cXLYB = getPixelColorXY(xL, yB);
    uint32_t cXRYB = getPixelColorXY(xR, yB);

    if (xL != xR && yT != yB)
    {
      setPixelColorXY(xL, yT, color_blend(col, cXLYT, uint8_t(sqrtf(dL * dT) * 255.0f))); // blend TL pixel
      setPixelColorXY(xR, yT, color_blend(col, cXRYT, uint8_t(sqrtf(dR * dT) * 255.0f))); // blend TR pixel
      setPixelColorXY(xL, yB, color_blend(col, cXLYB, uint8_t(sqrtf(dL * dB) * 255.0f))); // blend BL pixel
      setPixelColorXY(xR, yB, color_blend(col, cXRYB, uint8_t(sqrtf(dR * dB) * 255.0f))); // blend BR pixel
    }
    else if (xR != xL && yT == yB)
    {
      setPixelColorXY(xR, yT, color_blend(col, cXLYT, uint8_t(dL * 255.0f))); // blend L pixel
      setPixelColorXY(xR, yT, color_blend(col, cXRYT, uint8_t(dR * 255.0f))); // blend R pixel
    }
    else if (xR == xL && yT != yB)
    {
      setPixelColorXY(xR, yT, color_blend(col, cXLYT, uint8_t(dT * 255.0f))); // blend T pixel
      setPixelColorXY(xL, yB, color_blend(col, cXLYB, uint8_t(dB * 255.0f))); // blend B pixel
    }
    else
    {
      setPixelColorXY(xL, yT, col); // exact match (x & y land on a pixel)
    }
  }
  else
  {
    setPixelColorXY(uint16_t(roundf(fX)), uint16_t(roundf(fY)), col);
  }
}

#endif

//--------------------------------------------------------------------------------------------------

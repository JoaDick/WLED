/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * @file This file provides utilities for rendering 1D effects.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "FxExtras.h"
#include "PxArray.h"

//--------------------------------------------------------------------------------------------------
// lines

/** Draw a line between absolute positions (direction doesn't matter).
 * @param pxa Draw on that pixel array.
 * @param firstPos First pixel of the line.
 * @param lastPos  Last pixel of the line.
 * @param color The line's color.
 */
inline void line(PxArray &pxa, AIndex firstPos, AIndex lastPos, PxColor color)
{
  pxa.fillBlock(firstPos, lastPos, color);
}

/** Draw a relative line.
 * @param pxa Draw on that pixel array.
 * @param startPos First pixel of the line.
 * @param length Length of the line.
 *               Positive values for draw upward the array, negative values draw in the other direction.
 * @param color The line's color.
 */
inline void line_rel(PxArray &pxa, AIndex startPos, int length, PxColor color)
{
  if (length > 0)
    line(pxa, startPos, startPos + length - 1, color);
  else if (length < 0)
    line(pxa, startPos, startPos + length + 1, color);
}

/// Similar to line_rel() but draws around the given \a centerPos as middle of the line.
inline void line_centered(PxArray &pxa, AIndex centerPos, int length, PxColor color)
{
  line_rel(pxa, centerPos - length / 2, length, color);
}

/// Like line() - but with normalized positions.
inline void line_N(PxArray &pxa, NIndex firstPos, NIndex lastPos, PxColor color)
{
  line(pxa, pxa.toAbs(firstPos), pxa.toAbs(lastPos), color);
}

/// Like line_rel() - but with normalized positions.
inline void line_rel_N(PxArray &pxa, NIndex startPos, float length, PxColor color)
{
  line_rel(pxa, pxa.toAbs(startPos), pxa.toAbs(length), color);
}

/// Like line_centered() - but with normalized positions.
inline void line_centered_N(PxArray &pxa, NIndex centerPos, float length, PxColor color)
{
  line_rel_N(pxa, centerPos - length / 2.0f, length, color);
}

//--------------------------------------------------------------------------------------------------
// colorful lines

/** Draw a line between absolute positions (direction doesn't matter).
 * @param pxa Draw on that pixel array.
 * @param firstPos First pixel of the line.
 * @param lastPos  Last pixel of the line.
 * @param colorSource Get pixel color from there; the index is incremented by one for every pixel.
 */
void colorLine(PxArray &pxa, AIndex firstPos, AIndex lastPos, ColorSource &colorSource);

/** Draw a relative line.
 * @param pxa Draw on that pixel array.
 * @param startPos First pixel of the line.
 * @param length Length of the line.
 *               Positive values for draw upward the array, negative values draw in the other direction.
 * @param colorSource Get pixel color from there; the index is incremented by one for every pixel.
 */
inline void colorLine_rel(PxArray &pxa, AIndex startPos, int length, ColorSource &colorSource)
{
  if (length > 0)
    colorLine(pxa, startPos, startPos + length - 1, colorSource);
  else if (length < 0)
    colorLine(pxa, startPos, startPos + length + 1, colorSource);
}

/// Similar to colorLine_rel() but draws around the given \a centerPos as middle of the line.
inline void colorLine_centered(PxArray &pxa, AIndex centerPos, int length, ColorSource &colorSource)
{
  colorLine_rel(pxa, centerPos - length / 2, length, colorSource);
}

/// Like colorLine() - but with normalized positions.
inline void colorLine_N(PxArray &pxa, NIndex firstPos, NIndex lastPos, ColorSource &colorSource)
{
  colorLine(pxa, pxa.toAbs(firstPos), pxa.toAbs(lastPos), colorSource);
}

/// Like colorLine_rel() - but with normalized positions.
inline void colorLine_rel_N(PxArray &pxa, NIndex startPos, float length, ColorSource &colorSource)
{
  colorLine_rel(pxa, pxa.toAbs(startPos), pxa.toAbs(length), colorSource);
}

/// Like colorLine_centered() - but with normalized positions.
inline void colorLine_centered_N(PxArray &pxa, NIndex centerPos, float length, ColorSource &colorSource)
{
  colorLine_rel_N(pxa, centerPos - length / 2.0f, length, colorSource);
}

//--------------------------------------------------------------------------------------------------

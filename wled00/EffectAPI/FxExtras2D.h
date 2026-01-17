/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * @file This file provides utilities for rendering 2D effects.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "FxExtras1D.h"
#include "PxMatrix.h"

//--------------------------------------------------------------------------------------------------

/** Awesome wu_pixel procedure by reddit u/sutaburosu
 * @param matrix The drawing canvas.
 * @param pos The WU pixel's position.
 * @param color The WU pixel's color.
 * @see https://www.reddit.com/r/FastLED/comments/h7s96r/subpixel_positioning_wu_pixels/
 */
inline void wu_pixel_N(SegmentPxMatrix &matrix, NPoint pos, PxColor color)
{
  const int32_t x = pos.x * 256.0f * matrix.sizeX();
  const int32_t y = pos.y * 256.0f * matrix.sizeY();
  matrix.seg().wu_pixel(x, y, color.raw);
}

/** Draw a raster font character on canvas.
 * If color2 = BLACK then use currently selected palette for gradient, otherwise create gradient
 * from color1 and color2.
 * @param matrix The drawing canvas.
 * @param letter  The character to draw; only ASCII 32-126 supported (0x20-0x7E).
 * @param pos The character's position (left top corner).
 * @param letterWidth   Width of the character (in pixels).
 * @param letterHeight  Width of the character (in pixels).
 * @param color1  TBD
 * @param color2  TBD
 * @note Supported font sizes: 4x6, 5x8, 5x12, 6x8, 7x9=63
 */
inline void drawCharacter(SegmentPxMatrix &matrix, char letter, APoint pos,
                          uint8_t letterWidth, uint8_t letterHeight,
                          PxColor color1, PxColor color2 = PxColor::Black(), int8_t rotate = 0)
{
  matrix.seg().drawCharacter(letter, pos.x, pos.y, letterWidth, letterHeight, color1.raw, color2.raw, rotate);
}

//--------------------------------------------------------------------------------------------------

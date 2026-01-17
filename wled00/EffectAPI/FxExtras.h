/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * @file This file provides utilities (based on FxEnv runtime) for rendering effects.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "FxEnv.h"

//--------------------------------------------------------------------------------------------------

/** Get a color from the currently selected color palette.
 * @param env Effect runtime environment.
 * @param index Palette index: 0 = first color of the palette ... 255 = last color
 * @param brightness Brightness of the color.
 * @param blendType Color interpolation options for the palette:
 * - \c NOBLEND = No interpolation between palette entries (not recommended).
 * - \c LINEARBLEND = Linear interpolation between palette entries, with wrap-around from end to the beginning again.
 * - \c LINEARBLEND_NOWRAP = Linear interpolation between palette entries, but no wrap-around.
 */
inline PxColor paletteColor(FxEnv &env, uint8_t index, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND)
{
  return ColorFromPaletteWLED(env.currentPalette(), index, brightness, blendType);
}

/** Get a color based on a spectrum; either rainbow or from the currently selected palette.
 * When the \e Default palette (0) is selected in the UI, a rainbow color (based on HSV color model)
 * is returned. Otherwise, a color from the currently selected palette is returned.
 * @param env Effect runtime environment.
 * @param hue Rainbow's HSV hue value, or palette index.
 * @param vol Brightness of the color.
 * @param blendType Color interpolation options for the palette:
 * - \c NOBLEND = No interpolation between palette entries (not recommended).
 * - \c LINEARBLEND = Linear interpolation between palette entries, with wrap-around from end to the beginning again.
 * - \c LINEARBLEND_NOWRAP = Linear interpolation between palette entries, but no wrap-around.
 * @note Effect implementations may use this as alternative to \c color_wheel()
 * The difference to that function is that \a vol and \a blendType can be specified by the caller.
 */
inline PxColor rainbowColor(FxEnv &env, uint8_t hue, uint8_t vol = 255, TBlendType blendType = LINEARBLEND)
{
  if (env.ui().paletteNr())
    return paletteColor(env, hue, vol, blendType);
  uint32_t color;
  hsv2rgb(CHSV32(hue, 255, vol), color);
  return color;
}

/** Alias for compatibility with Segment::color_wheel()
 * Get a "rotating" color, based on the given \a pos
 * When the \e Default palette (0) is selected: \n
 * Rotates the color in HSV space, where \a pos is H (0 = 0deg ... 256 = 360deg) with S and V fixed
 * to 255. The colors are a transition red --> green --> blue --> back to red. \n
 * When another palette is selected: \n
 * Returns a color from that palette, where \a pos represents the palette index.
 * @param env Effect runtime environment.
 * @param pos Position in the color wheel.
 * @note Effect implementations shall use this instead of \c SEGMENT.color_wheel()
 */
inline PxColor color_wheel(FxEnv &env, uint8_t pos)
{
  return env.seg().color_wheel(pos);
}

/** Alias for compatibility with Segment::color_from_palette()
 * Get a single color from the currently selected color palette.
 * @param env Effect runtime environment.
 * @param i  Palette index; will wrap around automatically. See \a mapping for its range.
 * @param mapping  \c false = the range of \a i for a full palette cycle is 0 ... 255
 *                 \c true  = the range of \a i for a full palette cycle is 0 ... \c FxEnv::seglen()
 * @param moving  Color palettes can wrap back to the start smoothly.
 *                Set to \c true if you want that wrapping, e.g. when the effect uses a "moving" palette.
 *                Set to \c false to get a hard edge from end to start of the palette.
 * @param mcol  Only when the \e Default palette (0) is selected, return the standard color for 0 (fg), 1 (bg) or 2 (aux) instead.
 *              Ignored if this value is >2 or when another palette is selected.
 * @param pbri  Value to scale down the brightness of the returned color by. Default is 255, meaning full brightness.
 * @note Effect implementations shall use this instead of \c SEGMENT.color_from_palette()
 */
inline PxColor color_from_palette(FxEnv &env, uint16_t i, bool mapping, bool moving, uint8_t mcol, uint8_t pbri = 255)
{
  return env.seg().color_from_palette(i, mapping, moving, mcol, pbri);
}

//--------------------------------------------------------------------------------------------------

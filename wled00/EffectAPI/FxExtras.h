/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * @file This file provides common utilities (based on FxEnv runtime) for rendering effects.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "FxEnv.h"

//--------------------------------------------------------------------------------------------------
// color palette & rainbow

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
// ColorSource

/** Interface for classes that can generate colors based on a given index.
 * Like color palettes on steroids; just more versatile and extensible through custom implementations.
 */
class ColorSource
{
public:
  /** Get color at the given absolute \a index
   * One full range of the color source's spectrum is represented by \c 0<=index<size
   */
  PxColor get(AIndex index) { return do_getColor(index); }

  /** Get color at the given normalized \a index
   * One full range of the color source's spectrum is represented by \c 0.0<=index<=1.0
   */
  PxColor get_N(NIndex index) { return do_getColor(norm2abs(index, size - 1)); }

  /** Size of the color source's spectrum (in pixels).
   * Higher values stretch the spectrum over a larger range for \c index, lower values squeeze it.
   */
  int size;

protected:
  // no impact on child's copy & move policy
  ColorSource(const ColorSource &) = default;
  ColorSource(ColorSource &&) = default;
  ColorSource &operator=(const ColorSource &) = default;
  ColorSource &operator=(ColorSource &&) = default;
  ~ColorSource() = default;

  /** Constructor.
   * @param size Size of the color source's spectrum (in pixels).
   */
  explicit ColorSource(int size_) : size{size_} {}

  /** Get color at the given absolute \a index
   * @see constrainedIndex()
   */
  virtual PxColor do_getColor(AIndex index) = 0;

  /** Helper function for constraining \a index
   * Always returns \c 0...(size-1) - even for negative indices (mathematical modulo).
   */
  AIndex constrainedIndex(AIndex index) const { return ((index % size) + size) % size; }
};

/** A ColorSource that creates colors based on rainbow or from currently selected palette.
 * When the \e Default palette (0) is selected in the UI, a rainbow color (based on HSV color model)
 * is created. Otherwise, the color is created based on the currently selected palette.
 * Like rainbowColor() on steroids.
 */
class RainbowColorSource final : public ColorSource
{
public:
  // no copy & move - this class is intended to be used as temporary object on the stack
  RainbowColorSource(const RainbowColorSource &) = delete;
  RainbowColorSource &operator=(const RainbowColorSource &) = delete;

  /** Constructor.
   * @param env Effect runtime environment.
   * @param size Size of the color spectrum (in pixels).
   *             0 uses the entire segment for one full range of the rainpow (or palette).
   */
  explicit RainbowColorSource(FxEnv &env, int size = 0)
      : ColorSource(size ? size : env.seglen()), _env{env} {}

  /// Brightness of the color.
  uint8_t vol = 255;

  /// Color interpolation option for accessing the currently selected palette.
  TBlendType blendType = LINEARBLEND;

  /// When a color is requested, this offset is added to the user's given index.
  AIndex offset = 0;

  /** Set the \c offset (normalized version).
   * A value of 0.5 for example sets the \c offset to half the spectrum's size.
   */
  void setOffset_N(NIndex offset) { this->offset = norm2abs(offset, size - 1); }

private:
  /// @see ColorSource::do_getColor()
  PxColor do_getColor(AIndex index) override
  {
    const uint8_t hue = map(constrainedIndex(index + offset), 0, size, 0, 255);
    return rainbowColor(_env, hue, vol, blendType);
  }

  FxEnv &_env;
};

//--------------------------------------------------------------------------------------------------

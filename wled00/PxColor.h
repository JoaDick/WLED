/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "colors.h"
#include "pixeltypes.h"

//--------------------------------------------------------------------------------------------------

/** Generic pixel color.
 * This is just a wrapper for one single 32-bit integer value, which provides essential color
 * manipulation features - all in one place.
 * For seamless integration, it supports implicit conversion from \c uint32_t (WW-RR-GG-BB),
 * FastLED's \c CRGB and other color types.
 */
struct PxColor
{
  /// The pixel's raw 32 bit color value (white - red - green - blue).
  uint32_t raw;

  // ----- initialization -----

  /// Make a black PxColor.
  static constexpr PxColor Black() { return {}; }

  /// Default constructor - leaves the color uninitialized!
  PxColor() = default;

  /// Create from raw \c uint32_t (WW-RR-GG-BB).
  constexpr PxColor(uint32_t c) : raw{c} {}

  /// Create from discrete R-G-B (-W) portions.
  constexpr PxColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0)
      : raw{(static_cast<uint32_t>(w) << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b)} {}

  /// Create from FastLED's \c CRGB
  PxColor(CRGB c) : raw{static_cast<uint32_t>(c)} {}

  /// Create from FastLED's \c CHSV
  PxColor(CHSV c) : raw{CRGB{c}} {}

  /// Create from \c CRGBW
  constexpr PxColor(CRGBW c) : raw{c.color32} {}

  /// Create from \c CHSV32
  PxColor(CHSV32 c) { hsv2rgb(c, raw); }

  /// Make this color black.
  void clear() { raw = 0; }

  // ----- RGB access -----

  /// Get the red-only portion of this color.
  constexpr uint8_t r() const { return static_cast<uint8_t>((raw >> 16) & 0xFF); }

  /// Get the green-only portion of this color.
  constexpr uint8_t g() const { return static_cast<uint8_t>((raw >> 8) & 0xFF); }

  /// Get the blue-only portion of this color.
  constexpr uint8_t b() const { return static_cast<uint8_t>(raw & 0xFF); }

  // ----- white-only access -----

  /// Make a white-only PxColor.
  static constexpr PxColor White(uint8_t w) { return PxColor{static_cast<uint32_t>(w) << 24}; }

  /// Get the white part of this color.
  constexpr uint8_t w() const { return static_cast<uint8_t>((raw >> 24) & 0xFF); }

  /// Set only the white part of this color (RGB remains unchanged).
  void set_w(uint8_t w) { raw = (raw & 0x00FFFFFF) | (static_cast<uint32_t>(w) << 24); }

  /// Clear only the white part of this color (RGB remains unchanged).
  void clear_w() { raw = (raw & 0x00FFFFFF); }

  // ----- color manipulations -----

  /** Fast scaling function to reduce the brightness of this color.
   * Preferably use this method when speed more is crucial than accuracy - or when you're unsure
   * which fading algorithm to use.
   * Performs \c color*(255-fadeBy)/256 for all four channels.
   * @param fadeBy 0 = (almost) don't fade ... 255 = instantly black
   */
  PxColor &fastFade(uint8_t fadeBy)
  {
    raw = fast_color_scale(raw, 255 - fadeBy);
    return *this;
  }

  /** Reduce the brightness of this color.
   * When \a video is \c true the color will never become black unless it is already black.
   * Otherwise it will eventually become black.
   */
  PxColor &fade(uint8_t fadeBy, bool video)
  {
    raw = color_fade(raw, 255 - fadeBy, video);
    return *this;
  }

  /** Reduce the brightness of this color until it will eventually fade all the way to black.
   * More accurate than \c fastFade() - but slower.
   * This is just an alias for \c fade() with parameter \c video set to \a false
   */
  PxColor &fadeToBlackBy(uint8_t fadeBy) { return fade(fadeBy, false); }

  /** Reduce the brightness of this color; guaranteed to never fade all the way to black.
   * This is just an alias for \c fade() with parameter \c video set to \a true
   */
  PxColor &fadeLightBy(uint8_t fadeBy) { return fade(fadeBy, true); }

  /** Gradually change this color towards the other color; one step closer with every call.
   * Similar to blendColor() - but ensures that \a color is eventually reached even for small \a fadeBy values.
   * Inspired by Segment::fade_out()
   */
  PxColor &fadeToColorBy(PxColor color, uint8_t fadeBy);

  /** Add \a color into this color.
   * @param preserveCR The color's RGB ratio is preserved when \c true
   */
  PxColor &addColor(PxColor color, bool preserveCR = true)
  {
    raw = color_add(raw, color.raw, preserveCR);
    return *this;
  }

  /** Blend a fraction of \a color into this color.
   * The higher \a blendAmount is, the more of \a color is blended in.
   */
  PxColor &blendColor(PxColor color, uint8_t blendAmount)
  {
    raw = color_blend(raw, color.raw, blendAmount);
    return *this;
  }

  // ----- conversions -----

  // implicit conversion only to other RGB-compatible integer based types
  operator uint32_t &() { return raw; }
  constexpr operator uint32_t() const { return raw; }
  operator CRGB() const { return to_CRGB(); }
  constexpr operator CRGBW() const { return to_CRGBW(); }

  // explicit conversions to other color types
  CRGB to_CRGB() const { return CRGB{raw}; }

  CHSV to_CHSV() const { return static_cast<CHSV>(to_CHSV32()); }

  constexpr CRGBW to_CRGBW() const { return CRGBW{raw}; }

  CHSV32 to_CHSV32() const
  {
    CHSV32 hsv;
    rgb2hsv(raw, hsv);
    return hsv;
  }
};

inline constexpr bool operator==(PxColor c1, PxColor c2) { return c1.raw == c2.raw; }
inline constexpr bool operator!=(PxColor c1, PxColor c2) { return !(c1 == c2); }

/// Like PxColor::fastFade() - but returns a new PxColor object instead of in-place manipulation.
inline PxColor fastFadeColor(PxColor color, uint8_t fadeBy) { return color.fastFade(fadeBy); }

/// Like PxColor::fade() - but returns a new PxColor object instead of in-place manipulation.
inline PxColor fadeColor(PxColor color, uint8_t fadeBy, bool video) { return color.fade(fadeBy, video); }

/// Like PxColor::fadeToBlackBy() - but returns a new PxColor object instead of in-place manipulation.
inline PxColor fadeColorToBlackBy(PxColor color, uint8_t fadeBy) { return fadeColor(color, fadeBy, false); }

/// Like PxColor::fadeLightBy() - but returns a new PxColor object instead of in-place manipulation.
inline PxColor fadeColorLightBy(PxColor color, uint8_t fadeBy) { return fadeColor(color, fadeBy, true); }

/// Like PxColor::fadeToColorBy() - but returns a new PxColor object instead of in-place manipulation.
inline PxColor fadeColorToColorBy(PxColor color1, PxColor color2, uint8_t fadeBy) { return color1.fadeToColorBy(color2, fadeBy); }

/// Like PxColor::addColor() - but returns a new PxColor object instead of in-place manipulation.
inline PxColor addColors(PxColor color1, PxColor color2, bool preserveCR = true) { return color1.addColor(color2, preserveCR); }

/// Like PxColor::blendColor() - but returns a new PxColor object instead of in-place manipulation.
inline PxColor blendColors(PxColor color1, PxColor color2, uint8_t blendBy) { return color1.blendColor(color2, blendBy); }

/// Internal helper function for fadeToColorBy()
uint8_t fadeByte(uint8_t a, uint8_t b, int16_t deltaScaleFactor);

//--------------------------------------------------------------------------------------------------

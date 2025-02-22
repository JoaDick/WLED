/**
 * Essential/generic utilities for making WLED effect implementations easier.
 * @author Joachim Dick, 2025
 */

#pragma once

#ifndef FASTLED_INTERNAL
#define FASTLED_INTERNAL
#endif
#include "FastLED.h"

#define ENABLE_FRACTIONAL_INT 1

//--------------------------------------------------------------------------------------------------

/** Generic pixel color.
 * A very lightweigt 32-bit color object that provides essential color manipulation features. \n
 * It also supports implicit conversion from \c uint32_t (WW-RR-GG-BB) and FastLED's \c CRGB &
 * \c CHSV color types, as well as implicit conversion to \c uint32_t and \c CRGB
 */
struct PxColor
{
  /// Default constructor - leaves the color uninitialized!
  PxColor() = default;

  /// Create from raw \c uint32_t (WW-RR-GG-BB).
  /// @see PxColorWhite() below for making white pixels.
  PxColor(uint32_t c) : wrgb(c) {}

  /// Create from discrete R-G-B (-W) portions.
  PxColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) : wrgb((uint32_t(w) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b)) {}

  /// Create from FastLED's \c CRGB
  PxColor(CRGB c) : wrgb(c) {}

  /// Create from FastLED's \c CHSV
  PxColor(CHSV c) : wrgb(CRGB(c)) {}

  /// The pixel's 32 bit color value (white - red - green - blue).
  uint32_t wrgb;

  uint8_t w() const { return (wrgb >> 24) & 0xFF; }
  uint8_t r() const { return (wrgb >> 16) & 0xFF; }
  uint8_t g() const { return (wrgb >> 8) & 0xFF; }
  uint8_t b() const { return wrgb & 0xFF; }

  operator uint32_t() const { return wrgb; }
  operator CRGB() const { return wrgb; }

  /** Color add function that preserves ratio.
   * Original idea: https://github.com/Aircoookie/WLED/pull/2465 by https://github.com/Proto-molecule
   */
  PxColor &addColor(PxColor color, bool preserveCR = true);

  /** Blend a fraction of \a color into this color.
   * The higher \a blendAmount is, the more of \a color is blended in.
   */
  PxColor &blendColor(PxColor color, uint8_t blendAmount);

  /// Reduce the brightness until it will eventually fade all the way to black.
  PxColor &fadeToBlackBy(uint8_t fadeBy);

  /// Reduce the brightness; guaranteed to never fade all the way to black.
  PxColor &fadeLightBy(uint8_t fadeBy);

  /// Similar to blendColor() - but ensures that \a color is eventually reached even for small \a fadeBy values.
  PxColor &fadeToColorBy(PxColor color, uint8_t fadeBy);
};

/// Makes a white-only PxColor.
inline PxColor PxColorWhite(uint8_t w) { return PxColor(uint32_t(w) << 24); }

inline bool operator==(PxColor c1, PxColor c2) { return c1.wrgb == c2.wrgb; }
inline bool operator!=(PxColor c1, PxColor c2) { return !(c1 == c2); }

/// Like PxColor::addColor() - but returns a new PxColor object instead of in-place manipulation.
inline PxColor addColors(PxColor color1, PxColor color2, bool preserveCR = true) { return color1.addColor(color2, preserveCR); }

/// Like PxColor::blendColor() - but returns a new PxColor object instead of in-place manipulation.
inline PxColor blendColors(PxColor color1, PxColor color2, uint8_t blendBy) { return color1.blendColor(color2, blendBy); }

/// Like PxColor::fadeToBlackBy() - but returns a new PxColor object instead of in-place manipulation.
inline PxColor fadeColorToBlackBy(PxColor color, uint8_t fadeBy) { return color.fadeToBlackBy(fadeBy); }

/// Like PxColor::fadeLightBy() - but returns a new PxColor object instead of in-place manipulation.
inline PxColor fadeColorLightBy(PxColor color, uint8_t fadeBy) { return color.fadeLightBy(fadeBy); }

/// Like PxColor::fadeToColorBy() - but returns a new PxColor object instead of in-place manipulation.
inline PxColor fadeColorToColorBy(PxColor color1, PxColor color2, uint8_t fadeBy) { return color1.fadeToColorBy(color2, fadeBy); }

/** Put more emphasis on the red'ish colors.
 * Intended to be be used for the \c hue parameter of a \c CHSV color.
 */
inline uint8_t redShiftHue(uint8_t hue)
{
  return cos8(128 + hue / 2);
}

//--------------------------------------------------------------------------------------------------

/** DRAFT - Template class for 2D pixel coordinates.
 *
 * http://spiff.rit.edu/classes/phys311.old/lectures/vector/vector.html
 * https://docs.unity3d.com/560/Documentation/Manual/UnderstandingVectorArithmetic.html
 */
template <typename T>
struct Vector2D
{
  using Type = T;

  T x;
  T y;

  Vector2D() = default;
  Vector2D(T xx, T yy) : x(xx), y(yy) {}

  Vector2D &operator+=(const Vector2D &other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  Vector2D &operator-=(const Vector2D &other)
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  Vector2D &operator*=(T factor)
  {
    x *= factor;
    y *= factor;
    return *this;
  }

  Vector2D &operator/=(T factor)
  {
    x /= factor;
    y /= factor;
    return *this;
  }
};

template <typename T>
inline Vector2D<T> operator+(Vector2D<T> a, const Vector2D<T> &b) { return a += b; }

template <typename T>
inline Vector2D<T> operator-(Vector2D<T> a, const Vector2D<T> &b) { return a -= b; }

template <typename T>
inline Vector2D<T> operator*(Vector2D<T> a, T factor) { return a *= factor; }

template <typename T>
inline Vector2D<T> operator/(Vector2D<T> a, T factor) { return a /= factor; }

//--------------------------------------------------------------------------------------------------

#if (ENABLE_FRACTIONAL_INT)

/** DRAFT - Fractional integer value (with 8 bit fraction part).
 *
 * https://en.wikipedia.org/wiki/Fixed-point_arithmetic
 * https://brilliant.org/wiki/factional-part-function/
 * https://en.wikipedia.org/wiki/Fractional_part
 *
 * https://spin.atomicobject.com/simple-fixed-point-math/
 */
struct Fractional8
{
  using RawType = uint32_t;
  using IntType = uint32_t;
  using FractType = uint8_t;

  static constexpr RawType scaleFactor = 256;

  Fractional8() = default;

  explicit Fractional8(RawType r) : raw(r) {}

  Fractional8(IntType intPart, FractType fractPart) : raw(intPart << 8 | fractPart) {}

  /// Get the integer part (magnitude).
  IntType integer() const { return raw / scaleFactor; } // + implicit cast?

  /// Get the fractional part.
  FractType fraction() const { return raw & 0xFF; }

  RawType raw;

  // Operatoren +=, -=, ...
};

// freie Operatoren ...

//--------------------------------------------------------------------------------------------------

#endif

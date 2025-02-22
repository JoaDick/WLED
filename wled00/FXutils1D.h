/**
 * Utilities for making 1D WLED effect implementations easier.
 * @author Joachim Dick, 2025
 */

#pragma once

#include "FXutilsCore.h"

//--------------------------------------------------------------------------------------------------

/// Absolute pixel position (as 1D-index).
using AIndex = int;

/// Normalized pixel position (as 1D-index).
using NIndex = float;

#if (ENABLE_FRACTIONAL_INT)
/// Fractional pixel position (as 1D-index).
using FIndex = Fractional8;

inline FIndex toFract(AIndex pos) { return FIndex(pos, 0); }
#endif

//--------------------------------------------------------------------------------------------------

class ArrayPixelProxy;

/** Interface of a Pixel array for rendering effects.
 * ...
 */
class PxArray
{
public:
  /// Number of pixels in this array.
  AIndex size() const { return _size; }

  /** Get a proxy for the pixel at the given position.
   * Any kind of manipulations can be applied to the returned object, like assigning a new color to
   * that pixel or fading that pixel.
   */
  ArrayPixelProxy px(AIndex pos);
  ArrayPixelProxy px_n(NIndex pos);

  /// Set the pixel at the given position to the given \a color
  void setPixelColor(AIndex pos, PxColor color) { do_setPixelColor(pos, color); }
  void setPixelColor_n(NIndex pos, PxColor color) { do_setPixelColor(toAbs(pos), color); }

  /** Same as setPixelColor_n() but only positive values for \a pos will actually set the color.
   * This means that the (optional) pixel at exactly \a pos == 0.0 will \e not be drawn. \n
   * This may be useful when the Animation wants to implement something like a simple "invalid"
   * or "muted" state of a pixel algorithm.
   */
  void setOptColor_n(NIndex pos, PxColor color)
  {
    if (pos > 0.0)
      setPixelColor_n(pos, color);
  }

  /// Get color of the pixel at the given position.
  PxColor getPixelColor(AIndex pos) const { return do_getPixelColor(pos); }
  PxColor getPixelColor_n(NIndex pos) const { return do_getPixelColor(toAbs(pos)); }

  /// Get the background color of this pixel array.
  PxColor getBackgroundColor() const { return do_getBackgroundColor(); }

  /// Fill the entire array with the given \a color
  void fill(PxColor color) { do_fill(color); }

  /// Set all pixels within the block from \a firstPos to \a lastPos to the given \a color
  void fillBlock(AIndex firstpos, AIndex lastPos, PxColor color)
  {
    constrainPos(firstpos);
    constrainPos(lastPos);
    do_fillBlock(firstpos, lastPos, color);
  }

  // TBD
  /// Fades all pixels to black using nscale8()
  void fadeToBlackBy(uint8_t fadeBy) { do_fadeToBlackBy(fadeBy); }

  // TBD
  void fadeLightBy(uint8_t fadeBy) { do_fadeLightBy(fadeBy); }

  // TBD
  void fadeToBackground(uint8_t fadeBy) { fadeToColorBy(getBackgroundColor(), fadeBy); }

  // TBD
  void fadeToColorBy(PxColor color, uint8_t fadeBy) { do_fadeToColorBy(color, fadeBy); }

  /** Copy all the pixels colors from the \a other array to this array's pixels.
   * The shorter PxArray of them determines the number of copied pixels. \n
   * TIPP: This kind of assignment is also possible with PxMatrixRow or PxMatrixColumn since these
   * are also a PxArray by design.
   */
  void copyFrom(const PxArray &other);

  /// Like copyFrom() - just the other way around.
  void copyTo(PxArray &other) const { other.copyFrom(*this); }

  /** Convert the given normalized position into its corresponding absolute position.
   * @param pos Normalized pixel position
   *     \c 0.0 = first pixel (i.e. start of pixel array) --> absolute position = \c 0
   *     \c 1.0 = last pixel  (i.e. end of pixel array)   --> absolute position = \c size()-1
   */
  AIndex toAbs(NIndex pos) const { return round(pos * (_size - 1)); }

  /// @brief Constrain the given pixel position to be within the range \c 0 .. \c size()-1
  void constrainPos(AIndex &pos) const;

  /** Use the index-operator to access a specific pixel (similar as known from FastLED).
   * This is the same as calling \c px()
   */
  ArrayPixelProxy operator[](AIndex pos);

  /// Just for compatibility - prefer using setPixelColor() with \c PxColor as argument.
  void setPixelColor(AIndex pos, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) { setPixelColor(pos, PxColor(r, g, b, w)); }

protected:
  explicit PxArray(int pixelCount) : _size(pixelCount) {}
  ~PxArray() = default;

  virtual PxColor do_getBackgroundColor() const = 0;
  virtual PxColor do_getPixelColor(AIndex pos) const = 0;
  virtual void do_setPixelColor(AIndex pos, PxColor color) = 0;

  virtual void do_fill(PxColor color);
  virtual void do_fillBlock(AIndex firstpos, AIndex lastPos, PxColor color);
  virtual void do_fadeToBlackBy(uint8_t fadeBy);
  virtual void do_fadeLightBy(uint8_t fadeBy);
  virtual void do_fadeToColorBy(PxColor color, uint8_t fadeBy);
  // // TODO: maybe?
  // // adapt Segment::blur()
  // virtual void do_blur(uint8_t blurAmount, bool smear);

protected:
  /// Size of the array (number of pixels).
  const AIndex _size;
};

/** A proxy object that is representing a specific pixel of a pixel array.
 * @see PxArray::px()
 * @see PxArray::operator[]
 */
class ArrayPixelProxy
{
public:
  ArrayPixelProxy(PxArray &parent, AIndex arrayPos) : pos(arrayPos), _parent(parent) {}

  /// Get the color of this pixel.
  PxColor getColor() const { return _parent.getPixelColor(pos); }

  /// Set this pixel to the given \a color
  void setColor(PxColor color) { _parent.setPixelColor(pos, color); }

  /// @see PxColor::addColor()
  void addColor(PxColor color, bool preserveCR = true) { setColor(getColor().addColor(color, preserveCR)); }

  /// @see PxColor::blendColor()
  void blendColor(PxColor color, uint8_t blend) { setColor(getColor().blendColor(color, blend)); }

  /// @see PxColor::fadeToBlackBy()
  void fadeToBlackBy(uint8_t fadeBy) { setColor(getColor().fadeToBlackBy(fadeBy)); }

  /// @see PxColor::fadeLightBy()
  void fadeLightBy(uint8_t fadeBy) { setColor(getColor().fadeLightBy(fadeBy)); }

  /// @see PxColor::fadeToColorBy()
  void fadeToColorBy(PxColor color, uint8_t fadeBy) { setColor(getColor().fadeToColorBy(color, fadeBy)); }

  /// Assign a new color to this pixel.
  ArrayPixelProxy &operator=(PxColor color)
  {
    setColor(color);
    return *this;
  }

  /// Get the color of this pixel implicitly.
  operator PxColor() const { return getColor(); }

  /// Position of this pixel in the corresponsing pixel array.
  AIndex pos;

private:
  PxArray &_parent;
};

//--------------------------------------------------------------------------------------------------

/** Draw a line in the given \a color, from the given \a firstPos to \a lastPos
 * Direction doesn't matter; \a lastPos may be smaller than \a firstPos
 */
inline void lineAbs(PxArray &pxa, AIndex firstPos, AIndex lastPos, PxColor color) { pxa.fillBlock(firstPos, lastPos, color); }
inline void lineAbs_n(PxArray &pxa, NIndex firstPos, NIndex lastPos, PxColor color) { lineAbs(pxa, pxa.toAbs(firstPos), pxa.toAbs(lastPos), color); }

/** Draw a line in the given \a color, with the given \a length and starting at \a startPos.
 * Positive values for \a length draw upward the array, negative values draw in the other direction.
 */
void lineRel(PxArray &pxa, AIndex startPos, int length, PxColor color);
inline void lineRel_n(PxArray &pxa, NIndex startPos, float length, PxColor color) { lineRel(pxa, pxa.toAbs(startPos), pxa.toAbs(length), color); }

/// Similar to lineRel() but draws around the given \a centerPos as middle of the line.
inline void lineCentered(PxArray &pxa, AIndex centerPos, int length, PxColor color) { lineRel(pxa, centerPos - length / 2.0, length, color); }
inline void lineCentered_n(PxArray &pxa, NIndex centerPos, float length, PxColor color) { lineRel_n(pxa, centerPos - length / 2.0, length, color); }

// TBD
// anti-aliasing-pixel
void AA_Pixel(PxArray &pxa, NIndex pos, PxColor color);

//--------------------------------------------------------------------------------------------------

inline ArrayPixelProxy PxArray::px(AIndex pos) { return ArrayPixelProxy(*this, pos); }

inline ArrayPixelProxy PxArray::px_n(NIndex pos) { return ArrayPixelProxy(*this, toAbs(pos)); }

inline ArrayPixelProxy PxArray::operator[](AIndex pos) { return px(pos); }

inline void PxArray::constrainPos(AIndex &pos) const
{
  if (pos < 0)
    pos = 0;
  else if (pos >= _size)
    pos = _size - 1;
}

inline void PxArray::copyFrom(const PxArray &other)
{
  if (this != &other)
  {
    const AIndex sz = min(_size, other._size);
    for (AIndex i = 0; i < sz; ++i)
    {
      setPixelColor(i, other.getPixelColor(i));
    }
  }
}

inline void PxArray::do_fill(PxColor color) { do_fillBlock(0, _size - 1, color); }

inline void PxArray::do_fillBlock(AIndex firstpos, AIndex lastPos, PxColor color)
{
  while (firstpos < lastPos)
    setPixelColor(firstpos++, color);
  while (firstpos > lastPos)
    setPixelColor(firstpos--, color);
  setPixelColor(lastPos, color);
}

inline void PxArray::do_fadeToBlackBy(uint8_t fadeBy)
{
  if (fadeBy)
    for (AIndex i = 0; i < _size; ++i)
      setPixelColor(i, getPixelColor(i).fadeToBlackBy(fadeBy));
}

inline void PxArray::do_fadeLightBy(uint8_t fadeBy)
{
  if (fadeBy)
    for (AIndex i = 0; i < _size; ++i)
      setPixelColor(i, getPixelColor(i).fadeLightBy(fadeBy));
}

inline void PxArray::do_fadeToColorBy(const PxColor color, uint8_t fadeBy)
{
  if (fadeBy)
    for (AIndex i = 0; i < _size; ++i)
      setPixelColor(i, getPixelColor(i).fadeToColorBy(color, fadeBy));
}

//--------------------------------------------------------------------------------------------------

inline void lineRel(PxArray &pxa, AIndex startPos, int length, PxColor color)
{
  if (length > 0)
    lineAbs(pxa, startPos, startPos + length - 1, color);
  else if (length < 0)
    lineAbs(pxa, startPos, startPos + length + 1, color);
}

//--------------------------------------------------------------------------------------------------

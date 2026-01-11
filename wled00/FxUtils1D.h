/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * Pixel array and utilities for rendering 1D effects.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "PxColor.h"

//--------------------------------------------------------------------------------------------------

/** Absolute pixel position (as 1D-index).
 * Range: 0 = first pixel ... (size-1) = last pixel
 */
using AIndex = int;

/** Normalized pixel position (as 1D-index).
 * Range: 0.0 = first pixel ... 1.0 = last pixel
 */
using NIndex = float;

//--------------------------------------------------------------------------------------------------

class ArrayPixelProxy;

/** Interface of a pixel array for rendering 1D effects.
 * @note This class provides only methods for manipulating single pixels. Higher level features,
 * like drawing lines, etc. have to be implemented as free functions.
 */
class PxArray
{
public:
  // no general copy & move - this class is intended to be passed as reference to other functions
  PxArray(PxArray &&) = delete;
  PxArray &operator=(const PxArray &) = delete;
  PxArray &operator=(PxArray &&) = delete;

  /// Get the background color of this pixel array.
  PxColor getBackgroundColor() const { return do_getBackgroundColor(); }

  // ----- methods using absolute pixel positions -----

  /// Absolute size of this array = number of pixels.
  AIndex size() const { return _size; }

  /// Set the pixel at the given position to the given \a color
  void setColor(AIndex pos, PxColor color) { do_setColor(pos, color); }

  /// Get color of the pixel at the given position.
  PxColor getColor(AIndex pos) const { return do_getColor(pos); }

  /// Set all pixels within the block from \a firstPos to \a lastPos to the given \a color
  void fillBlock(AIndex firstPos, AIndex lastPos, PxColor color);

  /** Get a proxy for the pixel at the given position.
   * Many manipulations can be applied to the returned object, like fading or assigning a new color
   * to the corresponding pixel.
   * @note Be aware that this way implies more performance cost compared to setColor() and getColor()
   */
  ArrayPixelProxy pixel(AIndex pos);

  /** Use the index-operator to access a specific pixel (similar as known from FastLED).
   * This is equivalent to pixel()
   * @note Be aware that this way implies more performance cost compared to setColor() and getColor()
   */
  ArrayPixelProxy operator[](AIndex pos);

  // ----- methods using normalized pixel positions -----

  /** Convert the given normalized position into its corresponding absolute position.
   * @param pos Normalized pixel position
   *     \c 0.0 = first pixel (i.e. start of pixel array) --> absolute position = \c 0
   *     \c 1.0 = last pixel  (i.e. end of pixel array)   --> absolute position = \c size()-1
   */
  AIndex toAbs(NIndex pos) const { return round(pos * (_size - 1)); }

  /// Like setColor() - but with normalized position.
  void setColor_n(NIndex pos, PxColor color) { do_setColor(toAbs(pos), color); }

  /** Same as setColor_n() - but only positive values for \a pos will actually set the color.
   * This means that the (optional) pixel at exactly \a pos == 0.0 will \e not be drawn. \n
   * This may be useful when the Animation wants to implement something like a simple "invalid"
   * or "muted" state of a pixel algorithm.
   */
  void setOptColor_n(NIndex pos, PxColor color)
  {
    if (pos > 0.0f)
      setColor_n(pos, color);
  }

  /// Like getColor() - but with normalized position.
  PxColor getColor_n(NIndex pos) const { return do_getColor(toAbs(pos)); }

  // ----- methods that are manipulating all pixels -----

  /// Fill the entire array with the given \a color
  void fill(PxColor color) { do_fill(color); }

  /// Like PxColor::fastScale() - but for all pixels.
  void fastScale(uint8_t scale) { do_fastScale(scale); }

  /// Like PxColor::fade() - but for all pixels.
  void fade(uint8_t fadeBy, bool video) { do_fade(fadeBy, video); }

  /// Like PxColor::fadeToBlackBy() - but for all pixels.
  void fadeToBlackBy(uint8_t fadeBy) { do_fadeToBlackBy(fadeBy); }

  /// Like PxColor::fadeLightBy() - but for all pixels.
  void fadeLightBy(uint8_t fadeBy) { do_fadeLightBy(fadeBy); }

  /// Like PxColor::fadeToColorBy() - but for all pixels.
  void fadeToColorBy(PxColor color, uint8_t fadeBy) { do_fadeToColorBy(color, fadeBy); }

  // Like fadeToColorBy() - but towards the background color
  void fadeToBackgroundBy(uint8_t fadeBy) { do_fadeToColorBy(getBackgroundColor(), fadeBy); }

  /// Like PxColor::addColor() - but for all pixels.
  void addColor(PxColor color, bool preserveCR = true) { do_addColor(color, preserveCR); }

  /// Like PxColor::blendColor() - but for all pixels.
  void blendColor(PxColor color, uint8_t blendAmount) { do_blendColor(color, blendAmount); }

  /** Blur the pixels of this array.
   * @note For \a blur_amount > 215 this function does not work properly (creates alternating pattern)
   */
  void blur(uint8_t blurAmount, bool smear = false) { do_blur(blurAmount, smear); }

  /** Copy all the pixels colors from the \a other array to this array's pixels.
   * The shorter PxArray of then determines the number of copied pixels.
   */
  void copyFrom(const PxArray &other);

  /// Like copyFrom() - just the other way around.
  void copyTo(PxArray &other) const { other.copyFrom(*this); }

  // ----- aliases for better compatibility with Segment class -----

  /** Alias for compatibility with Segment::setPixelColor()
   * Consider using setColor() instead.
   */
  void setPixelColor(AIndex pos, PxColor color) { setColor(pos, color); }

  /** Alias for compatibility with Segment::setPixelColor()
   * Consider using setColor() with \c PxColor as argument instead.
   */
  void setPixelColor(AIndex pos, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) { setColor(pos, PxColor{r, g, b, w}); }

  /** Alias for compatibility with Segment::getPixelColor()
   * Consider using getColor() instead.
   */
  PxColor getPixelColor(AIndex pos) const { return getColor(pos); }

  /// Don't use this: Alias to support migration from Segment::fade_out()
  [[deprecated("use fadeToBackgroundBy() instead")]] void fade_out(uint8_t rate) { fadeToBackgroundBy(rate); }

protected:
  PxArray(const PxArray &) = default;
  explicit PxArray(AIndex pixelCount) : _size(pixelCount) {}
  ~PxArray() = default;

  /// Call this method when the segment's dimension has changed.
  void updateSize(AIndex newSize) { _size = newSize; }

  /// Get the background color of this pixel array.
  virtual PxColor do_getBackgroundColor() const = 0;

  /** Get color of the pixel at the given position.
   * @note Be aware that \a pos may be outside of the the array bounds.
   * Any color can be returned in that case.
   */
  virtual PxColor do_getColor(AIndex pos) const = 0;

  /** Set the pixel at the given position to the given \a color
   * @note Be aware that \a pos may be outside of the the array bounds.
   */
  virtual void do_setColor(AIndex pos, PxColor color) = 0;

  /// Fill the entire array with the given \a color
  virtual void do_fill(PxColor color);

  /** Set all pixels within the block from \a firstPos to \a lastPos to the given \a color
   * @note It is guaranteed that firstPos <= lastPos and that both are within the array bounds: 0 <= pos <= size()-1
   */
  virtual void do_fillBlock(AIndex firstPos, AIndex lastPos, PxColor color);

  /// Like PxColor::fastScale() - but for all pixels.
  virtual void do_fastScale(uint8_t scale);

  /// Like PxColor::fade() - but for all pixels.
  virtual void do_fade(uint8_t fadeBy, bool video);

  /// Like PxColor::fadeToBlackBy() - but for all pixels.
  virtual void do_fadeToBlackBy(uint8_t fadeBy);

  /// Like PxColor::fadeLightBy() - but for all pixels.
  virtual void do_fadeLightBy(uint8_t fadeBy);

  /// Like PxColor::fadeToColorBy() - but for all pixels.
  virtual void do_fadeToColorBy(PxColor color, uint8_t fadeBy);

  /// Like PxColor::addColor() - but for all pixels.
  virtual void do_addColor(PxColor color, bool preserveCR);

  /// Like PxColor::blendColor() - but for all pixels.
  virtual void do_blendColor(PxColor color, uint8_t blend);

  /// Blur the pixels of this array.
  virtual void do_blur(uint8_t blurAmount, bool smear);

private:
  AIndex _size;
};

//--------------------------------------------------------------------------------------------------

/** A proxy object that is representing a specific pixel of a pixel array.
 * @see PxArray::pixel()
 * @see PxArray::operator[]
 */
class ArrayPixelProxy
{
public:
  /// Only used internally.
  ArrayPixelProxy(PxArray &parent, AIndex arrayPos) : _parent(parent), _pos(arrayPos) {}

  /// Get the color of this pixel.
  PxColor getColor() const { return _parent.getColor(_pos); }

  /// Set this pixel to the given \a color
  void setColor(PxColor color) { _parent.setColor(_pos, color); }

  /// Like PxColor::fastScale()
  void fastScale(uint8_t scale) { setColor(getColor().fastScale(scale)); }

  /// Like PxColor::fade()
  void fade(uint8_t fadeBy, bool video) { setColor(getColor().fade(fadeBy, video)); }

  /// Like PxColor::fadeToBlackBy()
  void fadeToBlackBy(uint8_t fadeBy) { fade(fadeBy, false); }

  /// Like PxColor::fadeLightBy()
  void fadeLightBy(uint8_t fadeBy) { fade(fadeBy, true); }

  /// Like PxColor::fadeToColorBy()
  void fadeToColorBy(PxColor color, uint8_t fadeBy) { setColor(getColor().fadeToColorBy(color, fadeBy)); }

  /// Like PxColor::addColor()
  void addColor(PxColor color, bool preserveCR = true) { setColor(getColor().addColor(color, preserveCR)); }

  /// Like PxColor::blendColor()
  void blendColor(PxColor color, uint8_t blend) { setColor(getColor().blendColor(color, blend)); }

  /// Assign a new color to this pixel.
  ArrayPixelProxy &operator=(PxColor color)
  {
    setColor(color);
    return *this;
  }

  /// Get the color of this pixel implicitly.
  operator PxColor() const { return getColor(); }

  /// Assignment means just assigning the other's color.
  ArrayPixelProxy &operator=(const ArrayPixelProxy &other)
  {
    setColor(other.getColor());
    return *this;
  }

private:
  PxArray &_parent;
  const AIndex _pos;
};

//--------------------------------------------------------------------------------------------------
// line functions
//--------------------------------------------------------------------------------------------------

/** Draw a line in the given \a color, from the given \a firstPos to \a lastPos
 * Direction doesn't matter; \a lastPos may be smaller than \a firstPos
 */
inline void lineAbs(PxArray &pxa, AIndex firstPos, AIndex lastPos, PxColor color) { pxa.fillBlock(firstPos, lastPos, color); }

/** Draw a line in the given \a color, with the given \a length and starting at \a startPos.
 * Positive values for \a length draw upward the array, negative values draw in the other direction.
 */
void lineRel(PxArray &pxa, AIndex startPos, int length, PxColor color);

/// Similar to lineRel() but draws around the given \a centerPos as middle of the line.
inline void lineCentered(PxArray &pxa, AIndex centerPos, int length, PxColor color) { lineRel(pxa, centerPos - length / 2, length, color); }

/// Like lineRel() - but with normalized positions.
inline void lineRel_n(PxArray &pxa, NIndex startPos, float length, PxColor color) { lineRel(pxa, pxa.toAbs(startPos), pxa.toAbs(length), color); }

/// Like lineAbs() - but with normalized positions.
inline void lineAbs_n(PxArray &pxa, NIndex firstPos, NIndex lastPos, PxColor color) { lineAbs(pxa, pxa.toAbs(firstPos), pxa.toAbs(lastPos), color); }

/// Like lineCentered() - but with normalized positions.
inline void lineCentered_n(PxArray &pxa, NIndex centerPos, float length, PxColor color) { lineRel_n(pxa, centerPos - length / 2.0f, length, color); }

//--------------------------------------------------------------------------------------------------
// Just some inline method implementations below - nothing more to see...
//--------------------------------------------------------------------------------------------------

inline ArrayPixelProxy PxArray::pixel(AIndex pos) { return ArrayPixelProxy(*this, pos); }

inline ArrayPixelProxy PxArray::operator[](AIndex pos) { return pixel(pos); }

inline void PxArray::do_fill(PxColor color) { do_fillBlock(0, _size - 1, color); }

inline void PxArray::do_fadeToBlackBy(uint8_t fadeBy) { do_fade(fadeBy, false); }

inline void PxArray::do_fadeLightBy(uint8_t fadeBy) { do_fade(fadeBy, true); }

//--------------------------------------------------------------------------------------------------

inline void lineRel(PxArray &pxa, AIndex startPos, int length, PxColor color)
{
  if (length > 0)
    lineAbs(pxa, startPos, startPos + length - 1, color);
  else if (length < 0)
    lineAbs(pxa, startPos, startPos + length + 1, color);
}

//--------------------------------------------------------------------------------------------------

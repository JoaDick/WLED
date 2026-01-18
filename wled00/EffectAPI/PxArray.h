/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * @file Interface of a pixel array for rendering 1D effects.
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

/** Convert the given normalized position into its corresponding absolute position.
 * @param pos Normalized pixel position to convert
 * @param refIndex Reference index which is returned for \a pos = 1.0
 *                 This is typically \c size-1 for a pixel array.
 * @note This function includes a small margin (1/2 pixel) below 0.0 and above 1.0 which is also
 * mapped into the valid pixel array range.
 */
inline AIndex norm2abs(NIndex pos, AIndex refIndex) { return static_cast<AIndex>(round(pos * refIndex)); }

//--------------------------------------------------------------------------------------------------
class PxArrayPixelProxy;

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
  int size() const { return _size; }

  /// Set the pixel at the given position to the given \a color
  void setColor(AIndex pos, PxColor color) { do_setColor(pos, color); }

  /// Get color of the pixel at the given position.
  PxColor getColor(AIndex pos) const { return do_getColor(pos); }

  /// Set all pixels within the block from \a firstPos to \a lastPos to the given \a color
  void fillBlock(AIndex firstPos, AIndex lastPos, PxColor color) { do_fillBlock(firstPos, lastPos, color); }

  /** Get a proxy for the pixel at the given position.
   * Many manipulations can be applied to the returned object, like fading or assigning a new color
   * to the corresponding pixel.
   * @note Be aware that accessing individual pixels this way might lead to slightly more
   * performance cost compared to setColor() and getColor()
   */
  PxArrayPixelProxy pixel(AIndex pos);

  /** Use the index-operator to access a specific pixel (similar as known from FastLED).
   * This is equivalent to pixel()
   * @note Be aware that accessing individual pixels this way might lead to slightly more
   * performance cost compared to setColor() and getColor()
   */
  PxArrayPixelProxy operator[](AIndex pos);

  // ----- methods using normalized pixel positions -----

  /** Convert the given normalized position into its corresponding absolute position.
   * @param pos Normalized pixel position to convert
   *     \c 0.0 = first pixel (i.e. start of pixel array) --> absolute position = \c 0
   *     \c 1.0 = last pixel  (i.e. end of pixel array)   --> absolute position = \c size()-1
   */
  AIndex toAbs(NIndex pos) const { return norm2abs(pos, _size - 1); }

  /// Like setColor() - but with normalized position.
  void setColor_N(NIndex pos, PxColor color) { do_setColor(toAbs(pos), color); }

  /** Same as setColor_n() - but only positive values for \a pos will actually set the color.
   * This means that the (optional) pixel at exactly \a pos == 0.0 will \e not be drawn. \n
   * This may be useful when the Animation wants to implement something like a simple "invalid"
   * or "muted" state of a pixel algorithm.
   */
  void setOptColor_N(NIndex pos, PxColor color)
  {
    if (pos > 0.0f)
      setColor_N(pos, color);
  }

  /// Like getColor() - but with normalized position.
  PxColor getColor_N(NIndex pos) const { return do_getColor(toAbs(pos)); }

  // ----- methods that are manipulating all pixels -----

  /// Switch all pixels off.
  void clear() { do_fill(PxColor::Black()); }

  /// Set all pixels to the given \a color
  void fill(PxColor color) { do_fill(color); }

  /// Like PxColor::fastFade() - but for all pixels. Preferably use this fading algorithm by default.
  void fastFade(uint8_t fadeBy) { do_fastFade(fadeBy); }

  /// Like PxColor::fade() - but for all pixels.
  void fade(uint8_t fadeBy, bool video) { do_fade(fadeBy, video); }

  /** Like PxColor::fadeToBlackBy() - but for all pixels.
   * @note Be careful when migrating your effect:
   * Segment::fadeToBlackBy() uses the faster (but less accurate) \c fast_color_scale() internally,
   * whereas this method uses the slower (but more accurete) \c color_fade() algorithm. \n
   * So, to exactly preserve your existing behaviour, use fastFade() instead of this method!
   */
  void fadeToBlackBy(uint8_t fadeBy) { do_fadeToBlackBy(fadeBy); }

  /// Like PxColor::fadeLightBy() - but for all pixels.
  void fadeLightBy(uint8_t fadeBy) { do_fadeLightBy(fadeBy); }

  /// Like PxColor::fadeToColorBy() - but for all pixels.
  void fadeToColorBy(PxColor color, uint8_t fadeBy) { do_fadeToColorBy(color, fadeBy); }

  // Like fadeToColorBy() - but towards the background color
  void fadeToBackgroundBy(uint8_t fadeBy) { do_fadeToBackgroundBy(fadeBy); }

  /// Like PxColor::addColor() - but for all pixels.
  void addColor(PxColor color, bool preserveCR = true) { do_addColor(color, preserveCR); }

  /// Like PxColor::blendColor() - but for all pixels.
  void blendColor(PxColor color, uint8_t blendAmount) { do_blendColor(color, blendAmount); }

  /** Blur the pixels of this array.
   * @note For \a blur_amount > 215 this function does not work properly (creates alternating pattern)
   */
  void blur(uint8_t blurAmount, bool smear = false) { do_blur(blurAmount, smear); }

  /// Rotate all pixels of the array by the given \a delta (in pixels).
  void rotate(int delta) { do_rotate(delta); }

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

  /** Alias for compatibility with Segment::fade_out()
   * Consider using fadeToBackgroundBy() instead.
   */
  void fade_out(uint8_t rate) { fadeToBackgroundBy(rate); }

protected:
  PxArray(const PxArray &) = default;
  explicit PxArray(int size) : _size(size) {}
  ~PxArray() = default;

  /// Call this method when the segment's dimension has changed.
  void updateSize(int newSize) { _size = newSize; }

  /// Get the background color.
  virtual PxColor do_getBackgroundColor() const = 0;

  /** Get color of the pixel at the given position.
   * @note Be aware that the position may be outside of the the bounds.
   * Any color can be returned in that case.
   */
  virtual PxColor do_getColor(AIndex pos) const = 0;

  /** Set the pixel at the given position to the given \a color
   * @note Be aware that the position may be outside of the the bounds.
   */
  virtual void do_setColor(AIndex pos, PxColor color) = 0;

  /// Set all pixels to the given \a color
  virtual void do_fill(PxColor color);

  /** Set all pixels within the block from \a firstPos to \a lastPos to the given \a color
   * @note Be aware that the positions may be outside of the the bounds.
   * @see constrainRange()
   */
  virtual void do_fillBlock(AIndex firstPos, AIndex lastPos, PxColor color);

  /// Like PxColor::fastFade() - but for all pixels.
  virtual void do_fastFade(uint8_t fadeBy);

  /// Like PxColor::fade() - but for all pixels.
  virtual void do_fade(uint8_t fadeBy, bool video);

  /// Like PxColor::fadeToBlackBy() - but for all pixels.
  virtual void do_fadeToBlackBy(uint8_t fadeBy);

  /// Like PxColor::fadeLightBy() - but for all pixels.
  virtual void do_fadeLightBy(uint8_t fadeBy);

  /// Like PxColor::fadeToColorBy() - but for all pixels.
  virtual void do_fadeToColorBy(PxColor color, uint8_t fadeBy);

  // Like do_fadeToColorBy() - but towards the background color
  virtual void do_fadeToBackgroundBy(uint8_t fadeBy);

  /// Like PxColor::addColor() - but for all pixels.
  virtual void do_addColor(PxColor color, bool preserveCR);

  /// Like PxColor::blendColor() - but for all pixels.
  virtual void do_blendColor(PxColor color, uint8_t blend);

  /// Blur the pixels of this array.
  virtual void do_blur(uint8_t blurAmount, bool smear);

  /// Rotate all pixels of the array by the given \a delta (in pixels).
  virtual void do_rotate(int delta);

private:
  void rotateUp();
  void rotateDown();

private:
  int _size;
};

/** Constrain (and reorder) the given \a firstPos and \a lastPos to be within the bounds of \a pxa
 * @retval \a true Positions are now within bounds: \c 0<=firstPos<=lastPos<=pxa.size()-1
 * @retval \a false Both positions are out of bounds
 */
bool constrainRange(const PxArray &pxa, AIndex &firstPos, AIndex &lastPos);

//--------------------------------------------------------------------------------------------------

/** A proxy object that is representing a specific pixel of a pixel array.
 * @see PxArray::pixel()
 * @see PxArray::operator[]
 */
class PxArrayPixelProxy
{
public:
  /// Only used internally.
  PxArrayPixelProxy(PxArray &parent, AIndex pos) : _parent{parent}, _pos{pos} {}

  /// Get the color of this pixel.
  PxColor getColor() const { return _parent.getColor(_pos); }

  /// Set this pixel to the given \a color
  void setColor(PxColor color) { _parent.setColor(_pos, color); }

  /// Like PxColor::fastFade()
  void fastFade(uint8_t fadeBy) { setColor(getColor().fastFade(fadeBy)); }

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
  PxArrayPixelProxy &operator=(PxColor color)
  {
    setColor(color);
    return *this;
  }

  /// Get the color of this pixel implicitly.
  operator PxColor() const { return getColor(); }

  /// Assignment means just assigning the other's color.
  PxArrayPixelProxy &operator=(const PxArrayPixelProxy &other)
  {
    setColor(other.getColor());
    return *this;
  }

private:
  PxArray &_parent;
  const AIndex _pos;
};

//--------------------------------------------------------------------------------------------------
// Just some inline method implementations below - nothing more to see...

inline PxArrayPixelProxy PxArray::pixel(AIndex pos) { return PxArrayPixelProxy{*this, pos}; }

inline PxArrayPixelProxy PxArray::operator[](AIndex pos) { return pixel(pos); }

inline void PxArray::do_fill(PxColor color) { do_fillBlock(0, _size - 1, color); }

inline void PxArray::do_fadeToBlackBy(uint8_t fadeBy) { do_fade(fadeBy, false); }

inline void PxArray::do_fadeLightBy(uint8_t fadeBy) { do_fade(fadeBy, true); }

inline void PxArray::do_fadeToBackgroundBy(uint8_t fadeBy) { do_fadeToColorBy(getBackgroundColor(), fadeBy); }

//--------------------------------------------------------------------------------------------------

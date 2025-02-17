/**
 * Utilities for making 1D WLED effect implementations easier.
 * @author Joachim Dick, 2025
 */

#pragma once

#include "FXutils.h"

//--------------------------------------------------------------------------------------------------

class ArrayPixel;

/** Interface of a Pixel array for rendering effects.
 * ...
 */
class PxArray
{
public:
  /// Number of pixels in this array.
  int size() const { return _size; }

  // ----- methods for manipulating a single pixel (by absolute index) -----

  /** Get a proxy for the pixel at the given \a index
   * Any kind of manipulations can be applied to the returned object, like assigning a new color to
   * that pixel or fading that pixel.
   */
  ArrayPixel px(int index);

  /// Get color of the pixel at the given \a index
  PxColor getPixelColor(int index) const { return do_getPixelColor(index); }

  /// Set the pixel at the given \a index to the given \a color
  void setPixelColor(int index, PxColor color) { do_setPixelColor(index, color); }
  void setPixelColor(int index, byte r, byte g, byte b, byte w = 0) { do_setPixelColor(index, PxColor(r, g, b, w)); }

  // ----- methods for manipulating a single pixel (by normalized position) -----

  /// As px() but with a normalized pixel position; see toIndex()
  ArrayPixel n_px(float pos);

  /// As getPixelColor() but with a normalized pixel position; see toIndex()
  PxColor n_getPixelColor(float pos) const { return getPixelColor(toIndex(pos)); }

  /// As setPixelColor() but with a normalized pixel position; see toIndex()
  void n_setPixelColor(float pos, PxColor color) { setPixelColor(toIndex(pos), color); }
  void n_setPixelColor(float pos, byte r, byte g, byte b, byte w = 0) { setPixelColor(pos, PxColor(r, g, b, w)); }

  /** Same as n_setPixelColor() but only positive values for \a pos will actually set the pixel.
   * This means that the (optional) pixel at exactly \a pos == 0.0 will \e not be drawn. \n
   * This may be useful when the Animation wants to implement something like a simple
   * "invalid" or "muted" state of a pixel algorithm.
   */
  void n_setColorOpt(float pos, PxColor color)
  {
    if (pos > 0.0)
      n_setPixelColor(pos, color);
  }
  void n_setColorOpt(float pos, byte r, byte g, byte b, byte w = 0) { n_setColorOpt(pos, PxColor(r, g, b, w)); }

  // ----- methods for manipulating the entire array -----

  /// Get the background color of this array.
  PxColor getBackgroundColor() const { return do_getBackgroundColor(); }

  /// Fill the entire array with the given \a color
  void fill(PxColor color) { do_fill(color); }

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

  // ----- drawing lines (by absolute index) -----

  /** Draw a line in the given \a color, from the given \a firstIndex to \a lastIndex pixel.
   * Direction doesn't matter; \a lastIndex may be smaller than \a firstIndex.
   */
  void lineAbs(int firstIndex, int lastIndex, PxColor color);

  /** Draw a line in the given \a color, with the given \a length and starting at \a startIndex.
   * Positive values for \a length draw upward the array, negative values draw in other direction.
   */
  void lineRel(int startIndex, int length, PxColor color);

  /// Similar to lineRel() but draws around the given \a centerIndex
  void lineCentered(int centerIndex, int length, PxColor color) { lineRel(centerIndex - length / 2.0, length, color); }

  // ----- drawing lines (by normalized position) -----

  /// As lineAbs() but with normalized pixel positions; see toIndex()
  void n_lineAbs(float firstPos, float lastPos, PxColor color) { lineAbs(toIndex(firstPos), toIndex(lastPos), color); }

  /// As lineRel() but with normalized pixel positions; see toIndex()
  void n_lineRel(float startPos, float length, PxColor color) { lineRel(toIndex(startPos), toIndex(length), color); }

  /// Similar to n_lineRel() but draws around the given \a centerPos point.
  void n_lineCentered(float centerPos, float length, PxColor color) { n_lineRel(centerPos - length / 2.0, length, color); }

  // ----- other stuff -----

  /** Convert the given \a pos into its corresponding pixel index.
   * @param pos Pixel position (= normalized pixel index)
   *            0.0 = first pixel (i.e. start of pixel array) --> index = \c 0
   *            1.0 = last pixel (i.e. end of pixel array) --> index = \c size()-1
   */
  int toIndex(float pos) const { return round(pos * (_size - 1)); }

  /** @brief Constrain the given \a index to be within the range \c 0 .. \c size()-1
   * @retval true The given \a index was adjusted
   * @retval false The given \a index was already within the valid range
   */
  bool constrainIndex(int &index) const;

  /** Use the index-operator to access a specific pixel similar as known from FastLED.
   * This is the same as calling \c px()
   */
  ArrayPixel operator[](int index);

protected:
  explicit PxArray(int pixelCount) : _size(pixelCount) {}
  ~PxArray() = default;

  virtual PxColor do_getBackgroundColor() const = 0;
  virtual PxColor do_getPixelColor(int index) const = 0;
  virtual void do_setPixelColor(int index, PxColor color) = 0;

  virtual void do_fill(PxColor color) = 0;
  virtual void do_fadeToBlackBy(uint8_t fadeBy) = 0;
  virtual void do_fadeLightBy(uint8_t fadeBy) = 0;
  virtual void do_fadeToColorBy(PxColor color, uint8_t fadeBy) = 0;

protected:
  /// Size of the array (number of pixels).
  const int _size;
};

/** A proxy object that is representing a specific pixel of a PxArray.
 * @see PxArray::px()
 * @see PxArray::operator[]
 */
class ArrayPixel
{
public:
  ArrayPixel(PxArray &parent, int arrayIndex) : index(arrayIndex), _parent(parent) {}

  /// Get the color of this pixel.
  PxColor getColor() const { return _parent.getPixelColor(index); }

  /// Set this pixel to the given \a color
  void setColor(PxColor color) { _parent.setPixelColor(index, color); }
  void setColor(byte r, byte g, byte b, byte w = 0) { setColor(PxColor(r, g, b, w)); }

  // TBD
  void addColor(PxColor color, bool preserveCR = true) { _parent.setPixelColor(index, _parent.getPixelColor(index).addColor(color, preserveCR)); }

  // TBD
  void blendColor(PxColor color, uint8_t blend) { _parent.setPixelColor(index, _parent.getPixelColor(index).blendColor(color, blend)); }

  // TBD
  /// Fades pixel to black using nscale8()
  void fadeToBlackBy(uint8_t fadeBy) { _parent.setPixelColor(index, _parent.getPixelColor(index).fadeToBlackBy(fadeBy)); }

  // TBD
  // name from FastLED
  void fadeLightBy(uint8_t fadeBy) { _parent.setPixelColor(index, _parent.getPixelColor(index).fadeLightBy(fadeBy)); }

  // TBD
  void fadeToColorBy(PxColor color, uint8_t fadeBy) { _parent.setPixelColor(index, _parent.getPixelColor(index).fadeToColorBy(color, fadeBy)); }

  /// Assign a new color to this pixel.
  ArrayPixel &operator=(PxColor color)
  {
    setColor(color);
    return *this;
  }

  /// Get the color of this pixel implicitly.
  operator PxColor() const { return getColor(); }

  /// Index of this pixel in the corresponsing PxArray.
  int index;

private:
  PxArray &_parent;
};

inline ArrayPixel PxArray::operator[](int index) { return px(index); }
inline ArrayPixel PxArray::px(int index) { return ArrayPixel(*this, index); }
inline ArrayPixel PxArray::n_px(float pos) { return ArrayPixel(*this, toIndex(pos)); }

//--------------------------------------------------------------------------------------------------

/** WLED pixel array for rendering effects (as drawing facade for a Segment).
 * @see PxArray
 */
class WledPxArray : public PxArray
{
public:
  /// Constructor; to be initialized with \c fxs
  explicit WledPxArray(FxSetup &fxs) : WledPxArray(fxs.seg) {}

  /// Constructor; to be initialized with \c SEGMENT
  explicit WledPxArray(Segment &seg) : PxArray(seg.vLength()), _seg(seg) {}

  /** Blur the pixels of the array.
   * @note: For \a blur_amount > 215 this function does not work properly (creates alternating pattern)
   */
  void blur(uint8_t blur_amount, bool smear = false) { _seg.blur(blur_amount, smear); }

  /// Backdoor: Get the underlying Segment.
  Segment &getSegment() { return _seg; }

  /// Convenience backdoor: Access the underlying Segment's members via arrow operator.
  Segment *operator->() { return &_seg; }

  [[deprecated("use fadeToBackground() instead")]] void fade_out(uint8_t rate) { _seg.fade_out(rate); }

private:
  PxColor do_getBackgroundColor() const final { return _seg.getCurrentColor(1); }
  PxColor do_getPixelColor(int index) const final { return _seg.getPixelColor(index); }
  void do_setPixelColor(int index, PxColor color) final { _seg.setPixelColor(index, color.wrgb); }

  void do_fill(PxColor color) final { _seg.fill(color); }
  void do_fadeToBlackBy(uint8_t fadeBy) final { _seg.fadeToBlackBy(fadeBy); }
  void do_fadeLightBy(uint8_t fadeBy) final { PxArray::do_fadeLightBy(fadeBy); }
  void do_fadeToColorBy(PxColor color, uint8_t fadeBy) final { PxArray::do_fadeToColorBy(color, fadeBy); }

private:
  Segment &_seg;
};

//--------------------------------------------------------------------------------------------------

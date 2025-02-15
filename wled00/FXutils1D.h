/**
 * Some utilities for making 1D WLED effect implementations easier.
 */

#pragma once

#include "FXutils.h"

//--------------------------------------------------------------------------------------------------

class PixelProxy;

/** Interface of a Pixel array for rendering effects.
 * ...
 */
class PxArray
{
public:
  /// Number of pixels in this array.
  int size() const { return _size; }

  // ----- methods for manipulating a single pixel (by absolute index) -----

  /// Get color of the pixel at the given \a index
  PxColor getPixelColor(int index) const { return do_getPixelColor(index); }

  /// Set the pixel at the given \a index to the given \a color
  void setPixelColor(int index, PxColor color) { do_setPixelColor(index, color); }
  void setPixelColor(int index, byte r, byte g, byte b, byte w = 0) { do_setPixelColor(index, PxColor(r, g, b, w)); }

  // TBD
  /*
   * color add function that preserves ratio
   * original idea: https://github.com/Aircoookie/WLED/pull/2465 by https://github.com/Proto-molecule
   * speed optimisations by @dedehai
   */
  void addPixelColor(int index, PxColor color, bool preserveCR = true)
  {
    setPixelColor(index, color_add(getPixelColor(index), color, preserveCR));

    // _seg.addPixelColor(index, color, preserveCR);
    // void addPixelColor(int n, uint32_t color, bool preserveCR = true) { setPixelColor(n, color_add(getPixelColor(n), color, preserveCR)); }
  }

  // TBD
  /*
   * color blend function, based on FastLED blend function
   * the calculation for each color is: result = (A*(amountOfA) + A + B*(amountOfB) + B) / 256 with amountOfA = 255 - amountOfB
   */
  void blendPixelColor(int index, PxColor color, uint8_t blend)
  {
    // ToDo...
    setPixelColor(index, color_blend(getPixelColor(index), color, blend));

    // _seg.blendPixelColor(index, color, blend);
    // void blendPixelColor(int n, uint32_t color, uint8_t blend) { setPixelColor(n, color_blend(getPixelColor(n), color, blend)); }
  }

  // TBD
  /*
   * fades color toward black
   * if using "video" method the resulting color will never become black unless it is already black
   */
  /// Fades pixel to black using nscale8()
  void fadePixelToBlackBy(int index, uint8_t fadeBy)
  {
    setPixelColor(index, color_fade(getPixelColor(index), 255 - fadeBy, false));
    // setPixelColor(index, color_fade(getPixelColor(index), fadeBy, false));

    // _seg.fade???(index, fadeBy);

    // FastLED - also fade_raw()
    // nscale8( leds, num_leds, 255 - fadeBy);
  }

  // TBD
  /*
   * fades color toward black
   * if using "video" method the resulting color will never become black unless it is already black
   */
  // name from FastLED
  void fadePixelLightBy(int index, uint8_t fadeBy)
  {
    setPixelColor(index, color_fade(getPixelColor(index), 255 - fadeBy, true));
    // setPixelColor(index, color_fade(getPixelColor(index), fadeBy, true));

    // _seg.fadePixelColor(index, fadeBy);
    // void fadePixelColor  (int index,              uint8_t fade) { setPixelColor  (index, color_fade(getPixelColor  (index), fade, true)); }
    // void fadePixelColorXY(uint16_t x, uint16_t y, uint8_t fade) { setPixelColorXY(x, y,  color_fade(getPixelColorXY(x, y),  fade, true)); }

    // FastLED - also fade_video()
    // nscale8_video( leds, num_leds, 255 - fadeBy);
  }

  // // TODO ???
  // // see fade_out()
  // void fade_pixel_out(uint8_t rate);

  // ----- methods for manipulating a single pixel (by normalized position) -----

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

  void n_addPixelColor(float pos, PxColor color, bool preserveCR = true) { addPixelColor(toIndex(pos), color, preserveCR); }
  void n_blendPixelColor(float pos, PxColor color, uint8_t blend) { blendPixelColor(toIndex(pos), color, blend); }
  void n_fadePixelToBlackBy(float pos, uint8_t fadeBy) { fadePixelToBlackBy(toIndex(pos), fadeBy); }
  void n_fadePixelLightBy(float pos, uint8_t fadeBy) { fadePixelLightBy(toIndex(pos), fadeBy); }

  // ----- methods for manipulating the entire array -----

  /// Fill segment with the given \a color
  void fill(PxColor color) { do_fill(color); }

  // TBD
  /// Fades all pixels to black using nscale8()
  void fadeToBlackBy(uint8_t fadeBy) { do_fadeToBlackBy(fadeBy); }

  // TBD
  void fadeLightBy(uint8_t fadeBy) { do_fadeLightBy(fadeBy); }

  // TBD
  // Does this fade to background color???
  /// Fade out function, higher \a rate = quicker fade.
  void fade_out(uint8_t rate) { do_fade_out(rate); }
  // void fadeToBG(uint8_t rate) { do_fadeToBG(rate); }

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

  /// Use the index-operator to access a specific pixel similar as known from FastLED.
  PixelProxy operator[](int index);
  PixelProxy getPixel(int index);

  /** Copy all the pixel colors from \a other to this array.
   * The shorter PxArray determines the number of copied pixels. \n
   * TIPP: This kind of assignment is also possible with PxMatrixRow or PxMatrixColumn since these
   * are also a PxArray by design.
   */
  PxArray &operator=(const PxArray &other);

  PxArray(const PxArray &) = delete;
  PxArray &operator=(PxArray &&) = delete;

protected:
  explicit PxArray(int pixelCount) : _size(pixelCount) {}
  PxArray(PxArray &&) = default;

  virtual PxColor do_getPixelColor(int index) const = 0;
  virtual void do_setPixelColor(int index, PxColor color) = 0;

  virtual void do_fill(PxColor color) { lineAbs(0, _size - 1, color); }
  virtual void do_fadeToBlackBy(uint8_t fadeBy);
  virtual void do_fadeLightBy(uint8_t fadeBy);
  virtual void do_fade_out(uint8_t rate) = 0;
  // virtual void do_fadeToBG(uint8_t rate) = 0;

protected:
  /// Size of the array (number of pixels).
  const int _size;
};

/** Helper class that is representing a specific pixel.
 * @note This class is rarely used directly. It is returned by PxArray's index operator to emulate
 * the widely known FastLED syntax for manipulating LED strips.
 * @see PxArray::getPixel()
 * @see PxArray::operator[]
 */
class PixelProxy
{
  friend class PxArray;
  PixelProxy(PxArray &array, int index) : _array(array), _index(index) {}

  PxArray &_array;
  const int _index;

public:
  PixelProxy(const PixelProxy &) = delete;
  PixelProxy(PixelProxy &&) = default;
  PixelProxy &operator=(PixelProxy &&) = delete;

  /// Get the color of this pixel.
  PxColor getColor() const { return _array.getPixelColor(_index); }

  /// Set this pixel to the given \a color
  void setColor(PxColor color) { _array.setPixelColor(_index, color); }
  void setColor(byte r, byte g, byte b, byte w = 0) { setColor(PxColor(r, g, b, w)); }

  // TBD
  void addColor(PxColor color, bool preserveCR = true) { _array.addPixelColor(_index, color, preserveCR); }

  // TBD
  void blendColor(PxColor color, uint8_t blend) { _array.blendPixelColor(_index, color, blend); }

  // TBD
  /// Fades pixel to black using nscale8()
  void fadeToBlackBy(uint8_t fadeBy) { _array.fadePixelToBlackBy(_index, fadeBy); }

  // TBD
  // name from FastLED
  void fadeLightBy(uint8_t fadeBy) { _array.fadePixelLightBy(_index, fadeBy); }

  /// Assign a new color to this pixel.
  PixelProxy &operator=(PxColor color)
  {
    setColor(color);
    return *this;
  }

  /// Assign the color of the \a other pixel to this pixel.
  PixelProxy &operator=(const PixelProxy &other)
  {
    setColor(other.getColor());
    return *this;
  }

  /// Get the color of this pixel implicitly.
  operator PxColor() const { return getColor(); }
};

inline PixelProxy PxArray::operator[](int index) { return getPixel(index); }
inline PixelProxy PxArray::getPixel(int index) { return PixelProxy(*this, index); }

//--------------------------------------------------------------------------------------------------

/** Pixel strip for rendering effects (as drawing facade for a Segment).
 * @see PxArray
 */
class PxStrip : public PxArray
{
public:
  /// Constructor; to be initialized with \c fxs
  explicit PxStrip(FxSetup &fxs) : PxStrip(fxs.seg) {}

  /// Constructor; to be initialized with \c SEGMENT
  explicit PxStrip(Segment &seg) : PxArray(seg.vLength()), _seg(seg) {}

  /** Blur pixel strip content.
   * @Note: For \a blur_amount > 215 this function does not work properly (creates alternating pattern)
   */
  void blur(uint8_t blur_amount, bool smear = false) { _seg.blur(blur_amount, smear); }

  /// Backdoor: Get the underlying Segment.
  Segment &getSegment() { return _seg; }

  /// Convenience backdoor: Access the underlying Segment's members via arrow operator.
  Segment *operator->() { return &_seg; }

private:
  PxColor do_getPixelColor(int index) const final { return _seg.getPixelColor(index); }
  void do_setPixelColor(int index, PxColor color) final { _seg.setPixelColor(index, color.wrgb); }

  void do_fill(PxColor color) final { _seg.fill(color); }

  void do_fadeToBlackBy(uint8_t fadeBy) final { _seg.fadeToBlackBy(fadeBy); }

  void do_fade_out(uint8_t rate) final { _seg.fade_out(rate); }
  // void do_fadeToBG(uint8_t rate) final { _seg.fade_out(rate); }

private:
  Segment &_seg;
};

//--------------------------------------------------------------------------------------------------

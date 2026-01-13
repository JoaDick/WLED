/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * Pixel matrix and utilities for rendering 2D effects.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "PxColor.h"
#include "FxUtils1D.h"

//--------------------------------------------------------------------------------------------------

/** Absolute pixel position (as 2D-point).
 * A 2D point, representing an absolute pixel position (with integer coordinates).
 * The \c x and \c y members represent an absolute pixel position in the PxMatrix.
 * Range for x and y: 0 = first pixel ... (size-1) = last pixel
 */
struct APoint
{
  AIndex x;
  AIndex y;
};

/** Normalized pixel position (as 2D-point).
 * A 2D point, representing a normalized pixel position (with floating-point coordinates).
 * The \c x and \c y members represent an absolute pixel position in the PxMatrix.
 * Range for x and y: 0.0 = first pixel ... 1.0 = last pixel
 */
struct NPoint
{
  NIndex x;
  NIndex y;
};

//--------------------------------------------------------------------------------------------------

class PxMatrixPixelProxy;
class PxMatrixRow;
class PxMatrixColumn;

/** Interface of a pixel matrix for rendering 2D effects.
 * @note This class provides only methods for manipulating single pixels. Higher level features,
 * like drawing lines, boxes, etc. have to be implemented as free functions. \n
 * The coordinates [0, 0] represent the pixel at top left corner of the matrix.
 */
class PxMatrix
{
public:
  // no general copy & move - this class is intended to be passed as reference to other functions
  PxMatrix(PxMatrix &&) = delete;
  PxMatrix &operator=(const PxArray &) = delete;
  PxMatrix &operator=(PxMatrix &&) = delete;

  /// Get the background color of this pixel matrix.
  PxColor getBackgroundColor() const { return do_getBackgroundColor(); }

  // ----- methods using absolute pixel positions -----

  /// Absolute width of this matrix (in pixels) = number of columns.
  int sizeX() const { return _sizeX; }

  /// Absolute height of this matrix (in pixels) = number of rows.
  int sizeY() const { return _sizeY; }

  /// Set the pixel at the given position to the given \a color
  void setColor(APoint pos, PxColor color) { do_setColor(pos, color); }

  /// Convenience wrapper with discrete position arguments.
  void setColor(AIndex x, AIndex y, PxColor color) { do_setColor({x, y}, color); }

  /// Get color of the pixel at the given position.
  PxColor getColor(APoint pos) const { return do_getColor(pos); }

  /// Convenience wrapper with discrete position arguments.
  PxColor getColor(AIndex x, AIndex y) const { return do_getColor({x, y}); }

  /** Get a proxy for the pixel at the given position.
   * Many manipulations can be applied to the returned object, like fading or assigning a new color
   * to the corresponding pixel.
   * @note Be aware that accessing individual pixels this way might lead to slightly more
   * performance cost compared to setColor() and getColor()
   */
  PxMatrixPixelProxy pixel(APoint pos);

  /// Convenience wrapper with discrete position arguments.
  PxMatrixPixelProxy pixel(AIndex x, AIndex y);

  /** Use the index-operator to access a specific pixel (similar as known from FastLED).
   * This is equivalent to pixel()
   * @note Be aware that accessing individual pixels this way might lead to slightly more
   * performance cost compared to setColor() and getColor()
   */
  PxMatrixPixelProxy operator[](APoint pos);

  /// Get a specific row of the matrix, to be treated as a PxArray.
  PxMatrixRow row(AIndex rowIndex);

  /// Get a specific column of the matrix, to be treated as a PxArray.
  PxMatrixColumn column(AIndex columnIndex);

  // ----- methods using normalized pixel positions -----

  /// Convert the given normalized position into its corresponding absolute position.
  APoint toAbs(NPoint pos) const
  {
    const NIndex posX_n = round(pos.x * (_sizeX - 1));
    const NIndex posY_n = round(pos.y * (_sizeY - 1));
    return APoint{static_cast<AIndex>(posX_n), static_cast<AIndex>(posY_n)};
  }

  /// Like setColor() - but with normalized position.
  void setColor_N(NPoint pos, PxColor color) { do_setColor(toAbs(pos), color); }

  /// Like getColor() - but with normalized position.
  PxColor getColor_N(NPoint pos) const { return do_getColor(toAbs(pos)); }

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

  /// Blur the pixels of this matrix in both dimensions.
  void blur(uint8_t blurAmount, bool smear = false) { do_blur(blurAmount, smear); }

  /// Blur the pixels of all rows only (= horizontal blurring).
  void blurX(uint8_t blurAmount, bool smear = false) { do_blurXY(blurAmount, 0, smear); }

  /// Blur the pixels of all columns only (= vertical blurring).
  void blurY(uint8_t blurAmount, bool smear = false) { do_blurXY(0, blurAmount, smear); }

  /** 2-dimensional blur function.
   * @param blurAmountX is applied to all rows (= horizontal blurring)
   * @param blurAmountY is applied to all columns (= vertical blurring)
   */
  void blurXY(uint8_t blurAmountX, uint8_t blurAmountY, bool smear = false) { do_blurXY(blurAmountX, blurAmountY, smear); }

  // ----- aliases for better compatibility with Segment class -----

  /** Alias for compatibility with Segment::setPixelColorXY()
   * Consider using setColor() with \c APoint as argument instead.
   */
  void setPixelColorXY(AIndex x, AIndex y, PxColor color) { setColor({x, y}, color); }

  /** Alias for compatibility with Segment::setPixelColorXY()
   * Consider using setColor() with \c APoint and \c PxColor as argument instead.
   */
  void setPixelColorXY(AIndex x, AIndex y, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) { setColor({x, y}, PxColor{r, g, b, w}); }

  /** Alias for compatibility with Segment::getPixelColorXY()
   * Consider using getColor() with \c APoint as argument instead.
   */
  PxColor getPixelColorXY(AIndex x, AIndex y) const { return getColor({x, y}); }

  /** Alias for compatibility with Segment::fade_out()
   * Consider using fadeToBackgroundBy() instead.
   */
  void fade_out(uint8_t rate) { fadeToBackgroundBy(rate); }

protected:
  PxMatrix(const PxMatrix &) = default;
  explicit PxMatrix(AIndex sizeX, AIndex sizeY) : _sizeX(sizeX), _sizeY(sizeY) {}
  ~PxMatrix() = default;

  /// Call this method when the segment's dimension has changed.
  void updateSize(AIndex newSizeX, AIndex newSizeY)
  {
    _sizeX = newSizeX;
    _sizeY = newSizeY;
  }

  /// Get the background color.
  virtual PxColor do_getBackgroundColor() const = 0;

  /** Get color of the pixel at the given position.
   * @note Be aware that the position may be outside of the the bounds.
   * Any color can be returned in that case.
   */
  virtual PxColor do_getColor(APoint pos) const = 0;

  /** Set the pixel at the given position to the given \a color
   * @note Be aware that the position may be outside of the the bounds.
   */
  virtual void do_setColor(APoint pos, PxColor color) = 0;

  /// Set all pixels to the given \a color
  virtual void do_fill(PxColor color);

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

  /// Blur the pixels of this matrix in both dimensions.
  virtual void do_blur(uint8_t blurAmount, bool smear);

  /** 2-dimensional blur function.
   * @param blurAmountX is applied to all rows
   * @param blurAmountY is applied to all columns
   */
  virtual void do_blurXY(uint8_t blurAmountX, uint8_t blurAmountY, bool smear);

private:
  AIndex _sizeX;
  AIndex _sizeY;
};

/// Helper function to iterate over all pixels of a matrix (typically via lambda).
template <typename FUNCTION>
void applyToAllPixel(PxMatrix &matrix, FUNCTION function)
{
  APoint pos;
  for (pos.x = 0; pos.x < matrix.sizeX(); ++pos.x)
    for (pos.y = 0; pos.y < matrix.sizeY(); ++pos.y)
      function(matrix, pos);
}

//--------------------------------------------------------------------------------------------------

/** A proxy object that is representing a specific pixel of a pixel matrix.
 * @see PxMatrix::pixel()
 * @see PxMatrix::operator[]
 */
class PxMatrixPixelProxy
{
public:
  /// Only used internally.
  PxMatrixPixelProxy(PxMatrix &parent, APoint pos) : _parent{parent}, _pos{pos} {}

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
  PxMatrixPixelProxy &operator=(PxColor color)
  {
    setColor(color);
    return *this;
  }

  /// Get the color of this pixel implicitly.
  operator PxColor() const { return getColor(); }

  /// Assignment means just assigning the other's color.
  PxMatrixPixelProxy &operator=(const PxMatrixPixelProxy &other)
  {
    setColor(other.getColor());
    return *this;
  }

private:
  PxMatrix &_parent;
  const APoint _pos;
};

/** A proxy object that is representing one specific row of a PxMatrix.
 * Can be used like a PxArray.
 * @see PxMatrix::getRow()
 */
class PxMatrixRow final : public PxArray
{
public:
  /// Only used internally.
  PxMatrixRow(PxMatrix &parent, int matrixSizeX, int matrixIndexY)
      : PxArray{matrixSizeX}, _posY{matrixIndexY}, _parent{parent} {}

private:
  PxColor do_getBackgroundColor() const { return _parent.getBackgroundColor(); }
  PxColor do_getColor(AIndex pos) const { return _parent.getColor({pos, _posY}); }
  void do_setColor(AIndex pos, PxColor color) { _parent.setColor({pos, _posY}, color); }

private:
  PxMatrix &_parent;
  const AIndex _posY;
};

/** A proxy object that is representing one specific column of a PxMatrix.
 * Can be used like a PxArray.
 * @see PxMatrix::getColumn()
 */
class PxMatrixColumn final : public PxArray
{
public:
  /// Only used internally.
  PxMatrixColumn(PxMatrix &parent, int matrixIndexX, int matrixSizeY)
      : PxArray{matrixSizeY}, _posX{matrixIndexX}, _parent{parent} {}

private:
  PxColor do_getBackgroundColor() const { return _parent.getBackgroundColor(); }
  PxColor do_getColor(AIndex pos) const { return _parent.getColor({_posX, pos}); }
  void do_setColor(AIndex pos, PxColor color) { _parent.setColor({_posX, pos}, color); }

private:
  PxMatrix &_parent;
  const AIndex _posX;
};

//--------------------------------------------------------------------------------------------------
// Just some inline method implementations below - nothing more to see...
//--------------------------------------------------------------------------------------------------

inline PxMatrixPixelProxy PxMatrix::pixel(APoint pos) { return PxMatrixPixelProxy{*this, pos}; }

inline PxMatrixPixelProxy PxMatrix::pixel(AIndex x, AIndex y) { return pixel({x, y}); }

inline PxMatrixPixelProxy PxMatrix::operator[](APoint pos) { return pixel(pos); }

inline PxMatrixRow PxMatrix::row(AIndex rowIndex) { return PxMatrixRow{*this, _sizeX, rowIndex}; }

inline PxMatrixColumn PxMatrix::column(AIndex columnIndex) { return PxMatrixColumn{*this, columnIndex, _sizeY}; }

inline void PxMatrix::do_fadeToBlackBy(uint8_t fadeBy) { do_fade(fadeBy, false); }

inline void PxMatrix::do_fadeLightBy(uint8_t fadeBy) { do_fade(fadeBy, true); }

inline void PxMatrix::do_fadeToBackgroundBy(uint8_t fadeBy) { do_fadeToColorBy(getBackgroundColor(), fadeBy); }

inline void PxMatrix::do_blur(uint8_t blurAmount, bool smear) { do_blurXY(blurAmount, blurAmount, smear); }

//--------------------------------------------------------------------------------------------------

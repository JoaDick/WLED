/**
 * Utilities for making 2D WLED effect implementations easier.
 * @author Joachim Dick, 2025
 */

#pragma once

#include "FXutils1D.h"

//--------------------------------------------------------------------------------------------------

struct FPoint;

/// Template class for 2D pixel coordinates.
template <typename T>
struct TPoint
{
  T x; //< Pixel's X position
  T y; //< Pixel's Y position

  TPoint() = default;
  TPoint(T xx, T yy) : x(xx), y(yy) {}

  TPoint &operator+=(const TPoint &other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  TPoint &operator-=(const TPoint &other)
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  TPoint &operator+=(T value)
  {
    x += value;
    y += value;
    return *this;
  }

  TPoint &operator-=(T value)
  {
    x -= value;
    y -= value;
    return *this;
  }

  TPoint &operator*=(T value)
  {
    x *= value;
    y *= value;
    return *this;
  }

  TPoint &operator/=(T value)
  {
    x /= value;
    y /= value;
    return *this;
  }
};

/** A 2D point, representing an absolute pixel position (with integer coordinates).
 * The \c x and \c y members represent an absolute pixel position in the PxMatrix.
 */
struct APoint : public TPoint<int>
{
  using TPoint::TPoint;

  FPoint toFPoint() const;
};

/** A 2D point, representing an absolute pixel position (with fractional coordinates).
 * Similar to APoint, but with higher accuracy due to fractional indices. The \c x and \c y members
 * are like the ones from APoint, just multiplied with 256. So for example, \c x = 1088 represents
 * a pixel's fractional X position 4.25 (1088 = 1024 + 64 = 4 * 256 + 0.25 * 256).
 */
struct FPoint : public TPoint<int32_t>
{
  using TPoint::TPoint;

  APoint toAPoint() const { return APoint{(x + 128) / 256, (y + 128) / 256}; }
};

inline FPoint APoint::toFPoint() const { return FPoint{int32_t(x) * 256, int32_t(y) * 256}; }

/** A 2D point, representing a normalized pixel position (with floating-point coordinates).
 * The pixel representation of the \c x and \c y members depend on the currently selected mapping
 * policy of the PxMatrix.
 * @see PxMatrix::setMappingNormalized()
 * @see PxMatrix::setMappingProportional()
 * @see PxMatrix::setMappingAbsolute()
 */
using NPoint = TPoint<float>;

class PxMatrixRow;
class PxMatrixColumn;

/** DRAFT
 * ...
 * @note This class provides only methods for manipulating single pixels. Higher level features,
 * like drawing lines, boxes, etc. have to be implemented as free functions.
 */
class PxMatrix
{
public:
  /// Absolute width of the matrix (in pixels).
  int sizeX() const { return _sizeX; }

  /// Absolute height of the matrix (in pixels).
  int sizeY() const { return _sizeY; }

  float n_sizeX() const { /* implement me */ return 0; }
  float n_sizeY() const { /* implement me */ return 0; }

  /// Number of rows of the matrix.
  int rows() const { return sizeY(); }

  /// Number of columns of the matrix.
  int columns() const { return sizeX(); }

  // ----- methods for manipulating a single pixel (by absolute index) -----

  PxColor getPixelColor(int x, int y) const { return do_getPixelColor(x, y); }
  PxColor getPixelColor(APoint point) const { return getPixelColor(point.x, point.y); }

  void setPixelColor(int x, int y, PxColor color) { do_setPixelColor(x, y, color.wrgb); }
  void setPixelColor(APoint point, PxColor color) { setPixelColor(point.x, point.y, color); }

  // // TODO?
  // // https://www.reddit.com/r/FastLED/comments/h7s96r/subpixel_positioning_wu_pixels/
  // // WuPoint?
  // void wu_pixel(uint32_t x, uint32_t y, CRGB c);

  // ----- methods for manipulating a single pixel (by normalized position) -----

  PxColor n_getPixelColor(float x, float y) { return getPixelColor(toIPoint(NPoint(x, y))); }
  PxColor n_getPixelColor(NPoint point) { return getPixelColor(toIPoint(point)); }

  void n_setPixelColor(NPoint point, PxColor color) { setPixelColor(toIPoint(point), color); }

  // ----- methods for manipulating the entire matrix -----

  /// Get the background color of this matrix.
  PxColor getBackgroundColor() const { return do_getBackgroundColor(); }

  /// Fill segment with the given \a color
  void fill(PxColor color) { do_fill(color); }

  // TBD
  /// Fades all pixels to black using nscale8()
  void fadeToBlackBy(uint8_t fadeBy) { do_fadeToBlackBy(fadeBy); }

  // TBD
  void fadeLightBy(uint8_t fadeBy) { do_fadeLightBy(fadeBy); }

  // TBD
  void fadeToBackground(uint8_t fadeBy) { fadeToColorBy(fadeBy, getBackgroundColor()); }

  // TBD
  void fadeToColorBy(PxColor color, uint8_t fadeBy) { do_fadeToColorBy(color, fadeBy); }

  // ----- other stuff -----

  /** TBD
   * This setting is the default behaviour. \n
   * A point's visible range for \c x and \c y is normalized to 0.0 .. 1.0 - which represents the
   * full width and height of the matrix. \n
   * As a consequence, the animations will appear stretched on non-square matrices.
   */
  void setMappingNormalized() { /* implement me */ }

  /** TBD
   * Only relevant for non-square matrices. \n
   * Similar to setMappingNormalized() except that the real aspect ratio is taken into account.
   * The shorter side determines the normalized range 0.0 .. 1.0 - thus the longer side will be
   * represented by a larger range than that. \n
   * With a 32 x 8 matrix for example, the following ranges will represent the full width and height:
   * - \c x : 0.0 .. 4.0
   * - \c y : 0.0 .. 1.0
   */
  void setMappingProportional() { /* implement me */ }

  /** TBD
   * This setting makes NPoint behave like APoint, just with the better floating-point accuracy.
   * A point's \c x and \c y coordinate represent the absolute (index-based) pixel position in the
   * matrix.
   */
  void setMappingAbsolute() { /* implement me */ }

  PxMatrixRow getRow(int rowIndex);
  PxMatrixColumn getColumn(int columnIndex);
  PxMatrixColumn operator[](int columnIndex);

  APoint toIPoint(const NPoint &point) const { /* implement me */ return {0, 0}; }

  bool constrainPoint(APoint &point) const { /* implement me */ return false; }
  bool constrainPoint(NPoint &point) const { /* implement me */ return false; }

protected:
  explicit PxMatrix(int sizeX, int sizeY) : _sizeX(sizeX), _sizeY(sizeY) { setMappingNormalized(); }

  virtual PxColor do_getBackgroundColor() const = 0;
  virtual PxColor do_getPixelColor(int x, int y) const = 0;
  virtual void do_setPixelColor(int x, int y, PxColor color) = 0;

  virtual void do_fill(PxColor color) = 0;
  virtual void do_fadeToBlackBy(uint8_t fadeBy) = 0;
  virtual void do_fadeLightBy(uint8_t fadeBy) = 0;
  virtual void do_fadeToColorBy(PxColor color, uint8_t fadeBy) = 0;

  const int _sizeX;
  const int _sizeY;
};

/// One single row of a PxMatrix. Can be used like a PxArray.
class PxMatrixRow : public PxArray
{
public:
  PxMatrixRow(PxMatrix &parent, int matrixSizeX, int matrixIndexY) : PxArray(matrixSizeX), indexY(matrixIndexY), _parent(parent) {}

  int indexY;

private:
  PxColor do_getBackgroundColor() const final { return _parent.getBackgroundColor(); }
  PxColor do_getPixelColor(int index) const final { return _parent.getPixelColor(index, indexY); }
  void do_setPixelColor(int index, PxColor color) final { _parent.setPixelColor(index, indexY, color.wrgb); }

  void do_fill(PxColor color) final { PxArray::do_fill(color); }
  void do_fadeToBlackBy(uint8_t fadeBy) final { PxArray::do_fadeToBlackBy(fadeBy); }
  void do_fadeLightBy(uint8_t fadeBy) final { PxArray::do_fadeLightBy(fadeBy); }
  void do_fadeToColorBy(PxColor color, uint8_t fadeBy) final { PxArray::do_fadeToColorBy(color, fadeBy); }

private:
  PxMatrix &_parent;
};

inline PxMatrixRow PxMatrix::getRow(int rowIndex) { return PxMatrixRow(*this, _sizeX, rowIndex); }

/// One single column of a PxMatrix. Can be used like a PxArray.
class PxMatrixColumn : public PxArray
{
public:
  PxMatrixColumn(PxMatrix &parent, int matrixIndexX, int matrixSizeY) : PxArray(matrixSizeY), indexX(matrixIndexX), _parent(parent) {}

  int indexX;

private:
  PxColor do_getBackgroundColor() const final { return _parent.getBackgroundColor(); }
  PxColor do_getPixelColor(int index) const final { return _parent.getPixelColor(indexX, index); }
  void do_setPixelColor(int index, PxColor color) final { _parent.setPixelColor(indexX, index, color.wrgb); }

  void do_fill(PxColor color) final { PxArray::do_fill(color); }
  void do_fadeToBlackBy(uint8_t fadeBy) final { PxArray::do_fadeToBlackBy(fadeBy); }
  void do_fadeLightBy(uint8_t fadeBy) final { PxArray::do_fadeLightBy(fadeBy); }
  void do_fadeToColorBy(PxColor color, uint8_t fadeBy) final { PxArray::do_fadeToColorBy(color, fadeBy); }

private:
  PxMatrix &_parent;
};

inline PxMatrixColumn PxMatrix::getColumn(int columnIndex) { return PxMatrixColumn(*this, columnIndex, _sizeY); }
inline PxMatrixColumn PxMatrix::operator[](int columnIndex) { return getColumn(columnIndex); }

//--------------------------------------------------------------------------------------------------

inline void drawBox(PxMatrix &matrix, const NPoint &p1, const NPoint &p2, PxColor color) { /* implement me */ }
inline void drawBoxFilled(PxMatrix &matrix, const NPoint &p1, const NPoint &p2, PxColor color) { /* implement me */ }

//--------------------------------------------------------------------------------------------------

/** TBD
 * ...
 */
class WledPxMatrix
    : public PxMatrix
{
public:
  /// Constructor; to be initialized with \c fxs
  explicit WledPxMatrix(FxSetup &fxs) : WledPxMatrix(fxs.seg) {}

  /// Constructor; to be initialized with \c SEGMENT
  explicit WledPxMatrix(Segment &seg) : PxMatrix(seg.vWidth(), seg.vHeight()), _seg(seg) {}

  /// 2D-blur the pixels of the matrix (can be asymmetrical).
  void blur(uint8_t blurAmountX, uint8_t blurAmountY, bool smear = false) { _seg.blur2D(blurAmountX, blurAmountY, smear); }

  /// Backdoor: Get the underlying Segment.
  Segment &getSegment() { return _seg; }

  /// Convenience backdoor: Access the underlying Segment's members via arrow operator.
  Segment *operator->() { return &_seg; }

  [[deprecated("use fadeToBackground() instead")]] void fade_out(uint8_t rate) { _seg.fade_out(rate); }

private:
  PxColor do_getBackgroundColor() const final { return _seg.getCurrentColor(1); }
  PxColor do_getPixelColor(int x, int y) const final { return _seg.getPixelColorXY(x, y); }
  void do_setPixelColor(int x, int y, PxColor color) final { _seg.setPixelColorXY(x, y, color.wrgb); }

  void do_fill(PxColor color) final { _seg.fill(color); }
  void do_fadeToBlackBy(uint8_t fadeBy) final { _seg.fadeToBlackBy(fadeBy); }
  void do_fadeLightBy(uint8_t fadeBy) final { PxMatrix::do_fadeLightBy(fadeBy); }
  void do_fadeToColorBy(PxColor color, uint8_t fadeBy) final { PxMatrix::do_fadeToColorBy(color, fadeBy); }

private:
  Segment &_seg;
};

//--------------------------------------------------------------------------------------------------

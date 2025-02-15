/**
 * Some utilities for making 2D WLED effect implementations easier.
 */

#pragma once

#include "FXutils1D.h"

//--------------------------------------------------------------------------------------------------

/// Template class for 2D pixel coordinates.
template <typename T>
struct TPoint
{
  T x; //< Pixel's X position
  T y; //< Pixel's Y position

  TPoint() = default;
  TPoint(T xx, T yy) : x(xx), y(yy) {}

  TPoint &operator+=(const TPoint &o)
  {
    x += o.x;
    y += o.y;
    return *this;
  }

  TPoint &operator-=(const TPoint &o)
  {
    x -= o.x;
    y -= o.y;
    return *this;
  }

  TPoint &operator+=(T v)
  {
    x += v;
    y += v;
    return *this;
  }

  TPoint &operator-=(T v)
  {
    x -= v;
    y -= v;
    return *this;
  }

  TPoint &operator*=(T v)
  {
    x *= v;
    y *= v;
    return *this;
  }

  TPoint &operator/=(T v)
  {
    x /= v;
    y /= v;
    return *this;
  }
};

/** A 2D point, representing a pixel position with index based coordinates.
 * The \c x and \c y members represent the absolute pixel positions of the PxMatrix.
 */
using IPoint = TPoint<int>;

/** A 2D point, representing a pixel position with floating-point based coordinates.
 * The pixel representation of the \c x and \c y members depend on the currently selected mapping
 * policy of the PxMatrix.
 * @see PxMatrix::setMappingNormalized()
 * @see PxMatrix::setMappingProportional()
 * @see PxMatrix::setMappingAbsolute()
 */
using FPoint = TPoint<float>;

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
  /// Constructor; to be initialized with \c fxs
  explicit PxMatrix(FxSetup &fxs) : PxMatrix(fxs.seg) {}

  /// Constructor; to be initialized with \c SEGMENT
  explicit PxMatrix(Segment &seg) : _seg(seg), _sizeX(seg.vWidth()), _sizeY(seg.vHeight())
  {
    setMappingNormalized();
  }

  /// Absolute width of the matrix (in pixels).
  int sizeX() const { return _sizeX; }

  /// Absolute height of the matrix (in pixels).
  int sizeY() const { return _sizeY; }

  float f_sizeX() const { /* implement me */ return 0; }
  float f_sizeY() const { /* implement me */ return 0; }

  /// Number of rows of the matrix.
  int rows() const { return sizeY(); }

  /// Number of columns of the matrix.
  int columns() const { return sizeX(); }

  // ----- methods for manipulating a single pixel (by absolute index) -----

  PxColor getPixelColor(int x, int y) const { return _seg.getPixelColorXY(x, y); }
  PxColor getPixelColor(IPoint point) const { return getPixelColor(point.x, point.y); }

  void setPixelColor(int x, int y, PxColor color) { _seg.setPixelColorXY(x, y, color.wrgb); }
  void setPixelColor(IPoint point, PxColor color) { setPixelColor(point.x, point.y, color); }

  // // TODO?
  // // https://www.reddit.com/r/FastLED/comments/h7s96r/subpixel_positioning_wu_pixels/
  // // WuPoint?
  // void wu_pixel(uint32_t x, uint32_t y, CRGB c);

  // ----- methods for manipulating a single pixel (by normalized position) -----

  PxColor f_getPixelColor(float x, float y) { return getPixelColor(toIPoint(FPoint(x, y))); }
  PxColor f_getPixelColor(FPoint point) { return getPixelColor(toIPoint(point)); }

  void f_setPixelColor(FPoint point, PxColor color) { setPixelColor(toIPoint(point), color); }

  // ----- methods for manipulating the entire matrix -----

  // 2D blurring, can be asymmetrical
  void blur(uint8_t blur_amount_x, uint8_t blur_amount_y, bool smear = false)
  {
    _seg.blur2D(blur_amount_x, blur_amount_y, smear);
  }

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
   * This setting makes FPoint behave like IPoint, just with the better floating-point accuracy.
   * A point's \c x and \c y coordinate represent the absolute (index-based) pixel position in the
   * matrix.
   */
  void setMappingAbsolute() { /* implement me */ }

  PxMatrixRow getRow(int rowIndex);
  PxMatrixColumn getColumn(int columnIndex);
  PxMatrixColumn operator[](int columnIndex);

  IPoint toIPoint(const FPoint &point) const { /* implement me */ return {0, 0}; }

  bool constrainPoint(IPoint &point) const { /* implement me */ return false; }
  bool constrainPoint(FPoint &point) const { /* implement me */ return false; }

  /// Backdoor: Get the underlying Segment.
  Segment &getSegment() { return _seg; }

  /// Convenience backdoor: Access the underlying Segment's members via arrow operator.
  Segment *operator->() { return &_seg; }

private:
  Segment &_seg;
  const int _sizeX;
  const int _sizeY;
};

/// One single row of a PxMatrix. Can be used as a PxArray.
class PxMatrixRow : public PxArray
{
  friend class PxMatrix;
  PxMatrixRow(Segment &seg, int size, int y) : PxArray(size), _seg(seg), _y(y) {}

  PxColor do_getPixelColor(int index) const final { return _seg.getPixelColorXY(index, _y); }

  void do_setPixelColor(int index, PxColor color) final { _seg.setPixelColorXY(index, _y, color.wrgb); }

  void do_fade_out(uint8_t rate) final { /* implement me */ }

private:
  Segment &_seg;
  const int _y;
};

inline PxMatrixRow PxMatrix::getRow(int rowIndex) { return PxMatrixRow(_seg, _sizeX, rowIndex); }

class PxMatrixColumn : public PxArray
{
  friend class PxMatrix;
  PxMatrixColumn(Segment &seg, int x, int size) : PxArray(size), _seg(seg), _x(x) {}

  PxColor do_getPixelColor(int index) const final { return _seg.getPixelColorXY(_x, index); }

  void do_setPixelColor(int index, PxColor color) final { _seg.setPixelColorXY(_x, index, color.wrgb); }

  void do_fade_out(uint8_t rate) final { /* implement me */ }

private:
  Segment &_seg;
  const int _x;
};

inline PxMatrixColumn PxMatrix::getColumn(int columnIndex) { return PxMatrixColumn(_seg, columnIndex, _sizeY); }
inline PxMatrixColumn PxMatrix::operator[](int columnIndex) { return getColumn(columnIndex); }

//--------------------------------------------------------------------------------------------------

inline void drawBox(PxMatrix &matrix, const FPoint &p1, const FPoint &p2, PxColor color) { /* implement me */ }
inline void drawBoxFilled(PxMatrix &matrix, const FPoint &p1, const FPoint &p2, PxColor color) { /* implement me */ }

//--------------------------------------------------------------------------------------------------

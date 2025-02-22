/**
 * Utilities for making 2D WLED effect implementations easier.
 * @author Joachim Dick, 2025
 */

#pragma once

#include "FXutils1D.h"

//--------------------------------------------------------------------------------------------------

/// Absolute pixel position (as 2D-point).
/** DRAFT
 * A 2D point, representing an absolute pixel position (with integer coordinates).
 * The \c x and \c y members represent an absolute pixel position in the PxMatrix.
 */
using APoint = Vector2D<AIndex>;
using AVector = Vector2D<AIndex>;

/// Normalized pixel position (as 2D-point).
/** DRAFT
 * A 2D point, representing a normalized pixel position (with floating-point coordinates).
 * The pixel representation of the \c x and \c y members depend on the currently selected mapping
 * policy of the PxMatrix.
 * @see PxMatrix::setMappingNormalized()
 * @see PxMatrix::setMappingProportional()
 * @see PxMatrix::setMappingAbsolute()
 * @note The look and feel of this type of point is similar to the normalized pixel position in the
 * \c PxArray world, just with 2 dimensions. \n
 * Functions that are expecting such coordinates are oftentimes suffixed with \c _n like e.g.
 * \c line_n()
 */
using NPoint = Vector2D<NIndex>;
using NVector = Vector2D<NIndex>;

#if (ENABLE_FRACTIONAL_INT)

/// Fractional pixel position (as 2D-point).
/** DRAFT
 * A 2D point, representing an absolute pixel position (with fractional coordinates).
 * Similar to APoint, but with higher accuracy due to fractional indices. The \c x and \c y members
 * are like the ones from APoint, just multiplied with 256. So for example, \c x = 1088 represents
 * the pixel's fractional X position at 4.25 (1088 = 1024 + 64 = 4 * 256 + 0.25 * 256).
 * @note This type of point is used by only a few specially optimized drawing algorithms.
 * The day-to-day fellow for simple effects will be the \c APoint or the \c NPoint for smoother
 * stuff. \n
 * Functions that are expecting such coordinates are oftentimes suffixed with \c _f like e.g.
 * \c Wu_Pixel_f()
 */
using FPoint = Vector2D<FIndex>;
using FVector = Vector2D<FIndex>;

inline APoint toAbs(const FPoint &pos) { return APoint(pos.x.integer(), pos.y.integer()); }
inline FPoint toFract(const APoint &pos) { return FPoint(toFract(pos.x), toFract(pos.y)); }

#endif

//--------------------------------------------------------------------------------------------------

class PxMatrixRow;
class PxMatrixColumn;

/** DRAFT
 * @note This class provides only methods for manipulating single pixels. Higher level features,
 * like drawing lines, boxes, etc. have to be implemented as free functions. \n
 * The coordinates [0, 0] represent the pixel at top left corner of the matrix.
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

  void setPixelColor(const APoint &pos, PxColor color) { do_setPixelColor(pos, color); }
  void setPixelColor_n(const NPoint &pos, PxColor color) { do_setPixelColor(toAbs(pos), color); }

  PxColor getPixelColor(const APoint &pos) const { return do_getPixelColor(pos); }
  PxColor getPixelColor_n(const NPoint &pos) { return do_getPixelColor(toAbs(pos)); }

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

  PxMatrixRow operator[](int rowIndex);
  PxMatrixRow getRow(int rowIndex);
  PxMatrixColumn getColumn(int columnIndex);

  APoint toAbs(const NPoint &pos) const { /* implement me */ return APoint(round(pos.x * (_sizeX - 1)), round(pos.y * (_sizeY - 1))); }
  bool constrainPos(APoint &pos) const { /* implement me */ return false; }
  bool constrainPos(NPoint &pos) const { /* implement me */ return false; }

#if (ENABLE_FRACTIONAL_INT)
  FPoint toFract(const NPoint &pos) const { /* implement me */ return {}; }
  NPoint toNorm(const FPoint &pos) const { /* implement me */ return {0, 0}; }
  bool constrainPos(FPoint &pos) const { /* implement me */ return false; }
#endif

  /// Just for compatibility - prefer using setPixelColor() with \c APoint and \c PxColor as argument.
  void setPixelColorXY(int x, int y, PxColor color) { setPixelColor(APoint(x, y), color); }

  /// Just for compatibility - prefer using setPixelColor() with \c APoint and \c PxColor as argument.
  void setPixelColorXY(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) { setPixelColor(APoint(x, y), PxColor(r, g, b, w)); }

  /// Just for compatibility - prefer using getPixelColor() with \c APoint as argument.
  PxColor getPixelColorXY(int x, int y) const { return getPixelColor(APoint(x, y)); }

protected:
  explicit PxMatrix(int sizeX, int sizeY) : _sizeX(sizeX), _sizeY(sizeY) { setMappingNormalized(); }

  virtual PxColor do_getBackgroundColor() const = 0;
  virtual PxColor do_getPixelColor(const APoint &pos) const = 0;
  virtual void do_setPixelColor(const APoint &pos, PxColor color) = 0;

  virtual void do_fill(PxColor color);
  virtual void do_fadeToBlackBy(uint8_t fadeBy);
  virtual void do_fadeLightBy(uint8_t fadeBy);
  virtual void do_fadeToColorBy(PxColor color, uint8_t fadeBy);

  const int _sizeX;
  const int _sizeY;
};

/// One single row of a PxMatrix. Can be used like a PxArray.
class PxMatrixRow final : public PxArray
{
public:
  PxMatrixRow(PxMatrix &parent, int matrixSizeX, int matrixIndexY) : PxArray(matrixSizeX), indexY(matrixIndexY), _parent(parent) {}

  int indexY;

private:
  PxColor do_getBackgroundColor() const { return _parent.getBackgroundColor(); }
  PxColor do_getPixelColor(AIndex pos) const { return _parent.getPixelColor(APoint(pos, indexY)); }
  void do_setPixelColor(AIndex pos, PxColor color) { _parent.setPixelColor(APoint(pos, indexY), color); }

private:
  PxMatrix &_parent;
};

/// One single column of a PxMatrix. Can be used like a PxArray.
class PxMatrixColumn final : public PxArray
{
public:
  PxMatrixColumn(PxMatrix &parent, int matrixIndexX, int matrixSizeY) : PxArray(matrixSizeY), indexX(matrixIndexX), _parent(parent) {}

  int indexX;

private:
  PxColor do_getBackgroundColor() const { return _parent.getBackgroundColor(); }
  PxColor do_getPixelColor(AIndex pos) const { return _parent.getPixelColor(APoint(indexX, pos)); }
  void do_setPixelColor(AIndex pos, PxColor color) { _parent.setPixelColor(APoint(indexX, pos), color); }

private:
  PxMatrix &_parent;
};

//--------------------------------------------------------------------------------------------------

// TBD
// https://www.reddit.com/r/FastLED/comments/h7s96r/subpixel_positioning_wu_pixels/
void Wu_Pixel_f(PxMatrix &pxm, const NPoint &pos, PxColor color);

inline void drawBox(PxMatrix &pxm, const APoint &p1, const APoint &p2, PxColor color) { /* implement me */ }

inline void drawBoxFilled(PxMatrix &pxm, const APoint &p1, const APoint &p2, PxColor color) { /* implement me */ }

//--------------------------------------------------------------------------------------------------

inline PxMatrixRow PxMatrix::operator[](int rowIndex) { return getRow(rowIndex); }

inline PxMatrixRow PxMatrix::getRow(int rowIndex) { return PxMatrixRow(*this, _sizeX, rowIndex); }

inline PxMatrixColumn PxMatrix::getColumn(int columnIndex) { return PxMatrixColumn(*this, columnIndex, _sizeY); }

inline void PxMatrix::do_fill(PxColor color)
{
  APoint pos;
  for (pos.x = 0; pos.x < _sizeX; ++pos.x)
    for (pos.y = 0; pos.y < _sizeX; ++pos.y)
      setPixelColor(pos, color);
}

inline void PxMatrix::do_fadeToBlackBy(uint8_t fadeBy)
{
  APoint pos;
  if (fadeBy)
    for (pos.x = 0; pos.x < _sizeX; ++pos.x)
      for (pos.y = 0; pos.y < _sizeX; ++pos.y)
        setPixelColor(pos, getPixelColor(pos).fadeToBlackBy(fadeBy));
}

inline void PxMatrix::do_fadeLightBy(uint8_t fadeBy)
{
  APoint pos;
  if (fadeBy)
    for (pos.x = 0; pos.x < _sizeX; ++pos.x)
      for (pos.y = 0; pos.y < _sizeX; ++pos.y)
        setPixelColor(pos, getPixelColor(pos).fadeLightBy(fadeBy));
}

inline void PxMatrix::do_fadeToColorBy(const PxColor color, uint8_t fadeBy)
{
  APoint pos;
  if (fadeBy)
    for (pos.x = 0; pos.x < _sizeX; ++pos.x)
      for (pos.y = 0; pos.y < _sizeX; ++pos.y)
        setPixelColor(pos, getPixelColor(pos).fadeToColorBy(color, fadeBy));
}

//--------------------------------------------------------------------------------------------------

/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * @file TBD ...
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "FX.h"
#include "PxArray.h"
#include "PxMatrix.h"

//--------------------------------------------------------------------------------------------------

/// PxArray implementation for rendering 1D effects via Segment class.
class SegmentPxArray final : public PxArray
{
public:
  /// Constructor.
  explicit SegmentPxArray(Segment &seg) : PxArray(seg.vLength()), _seg(&seg) {}

  /// Get the underlying Segment (for advanced operations).
  Segment &seg() { return *_seg; }

  /** Call this method when the underlying segment has changed.
   * @param seg The changed segment wo work on from now.
   * @return \c true When the segment's dimension has changed.
   */
  bool updateSegment(Segment &seg)
  {
    const auto old_size = size();
    updateSize(seg.vLength());
    _seg = &seg;
    return old_size != size();
  }

private:
  PxColor do_getBackgroundColor() const override { return _seg->getCurrentColor(1); }

  PxColor do_getColor(AIndex pos) const override { return _seg->getPixelColor(pos); }

  void do_setColor(AIndex pos, PxColor color) override { _seg->setPixelColor(pos, color.raw); }

  // Note: Segment::fadeToBlackBy() already uses fast_color_scale() instead of (the slower) color_fade()
  void do_fastFade(uint8_t fadeBy) override { _seg->fadeToBlackBy(fadeBy); }

  void do_blur(uint8_t blurAmount, bool smear) override { _seg->blur(blurAmount, smear); }

  // Note: Do NOT override do_fadeToBackgroundBy() -- Segment::fade_out() doesn't work properly for
  // very small values of fadeBy. --> To be investigated!

private:
  Segment *_seg;
};

//--------------------------------------------------------------------------------------------------

/// PxMatrix implementation for rendering 2D effects via Segment class.
class SegmentPxMatrix final : public PxMatrix
{
public:
  /// Constructor.
  explicit SegmentPxMatrix(Segment &seg) : PxMatrix(seg.vWidth(), seg.vHeight()), _seg(&seg) {}

  /// Get the underlying Segment (for advanced operations).
  Segment &seg() { return *_seg; }

  /** Call this method when the underlying segment has changed.
   * @param seg The changed segment wo work on from now.
   * @return \c true When the segment's dimension has changed.
   */
  bool updateSegment(Segment &seg)
  {
    const auto old_sizeX = sizeX();
    const auto old_sizeY = sizeY();
    updateSize(seg.vWidth(), seg.vHeight());
    _seg = &seg;
    return (old_sizeX != sizeX()) || (old_sizeY != sizeY());
  }

private:
  PxColor do_getBackgroundColor() const override { return _seg->getCurrentColor(1); }

  PxColor do_getColor(APoint pos) const override { return _seg->getPixelColorXY(pos.x, pos.y); }

  void do_setColor(APoint pos, PxColor color) override { _seg->setPixelColorXY(pos.x, pos.y, color.raw); }

  // Note: Segment::fadeToBlackBy() already uses fast_color_scale() instead of (the slower) color_fade()
  void do_fastFade(uint8_t fadeBy) override { _seg->fadeToBlackBy(fadeBy); }

  void do_blurXY(uint8_t blurAmountX, uint8_t blurAmountY, bool smear) override { _seg->blur2D(blurAmountX, blurAmountY, smear); }

  // Note: Do NOT override do_fadeToBackgroundBy() -- Segment::fade_out() doesn't work properly for
  // very small values of fadeBy. --> To be investigated!

private:
  Segment *_seg;
};

//--------------------------------------------------------------------------------------------------

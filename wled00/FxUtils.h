/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "FX.h"
#include "FxUtils1D.h"

//--------------------------------------------------------------------------------------------------

/** PxArray implementation for rendering 1D effects via Segment class.
 */
class SegmentPxArray final : public PxArray
{
public:
  /// Constructor; to be initialized with \c SEGMENT
  explicit SegmentPxArray(Segment &seg) : PxArray(seg.vLength()), _seg(&seg) {}

  /// Get the underlying Segment.
  Segment &getSegment() { return *_seg; }

  /** Call this method when the segment or its setting has changed.
   * @param seg The changed segment wo work on from now.
   * @return \c true When the segment's dimension has changed.
   */
  bool updateSegment(Segment &seg)
  {
    const auto old_size = size();
#if (1)
    updateSize(seg.virtualLength());
#else
    updateSize(seg.vLength());
#endif
    _seg = &seg;
    return old_size != size();
  }

private:
  PxColor do_getBackgroundColor() const override { return _seg->getCurrentColor(1); }

  PxColor do_getColor(AIndex pos) const override { return _seg->getPixelColor(pos); }

  void do_setColor(AIndex pos, PxColor color) override { _seg->setPixelColor(pos, color.wrgb); }

  // Segment::fadeToBlackBy() uses fast_color_scale() algorithm instead of color_fade()
  void do_fastScale(uint8_t scale) override { _seg->fadeToBlackBy(255 - scale); }

  void do_blur(uint8_t blur_amount, bool smear = false) override { _seg->blur(blur_amount, smear); }

private:
  Segment *_seg;
};

//--------------------------------------------------------------------------------------------------

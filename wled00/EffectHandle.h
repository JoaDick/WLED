/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * This headerfile provides a handle for the Segments to manage their corresponding effect instance.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include <memory>

//--------------------------------------------------------------------------------------------------
class Segment;
class EffectAdapter;
using EffectAdapterPtr = std::unique_ptr<EffectAdapter>;

/** This class represents the one and only interface for Segments to interact with class-based WLED effects.
 * It takes full responsibility over the effect's lifetime (create, destroy, copy/clone, move).
 * @note Be aware that some effects might not be able to handle dimention changes or cannor be
 * copied/cloned. In that case, the contained effect instance is destroyed, resulting in an empty
 * handle. It is then up to the Segment to create a new effect instance at the next frame.
 */
class EffectHandle
{
public:
  /** Constructor.
   * @param newSeg The Segment where this handle lives inside.
   */
  explicit EffectHandle(Segment &seg) : _seg{&seg} {}

  /** Create an instance of the given \a FX_TYPE effect class inside this handle.
   * @param now The current timestamp (in ms).
   */
  template <class FX_TYPE>
  void createEffect(uint32_t now);

  /// Check if this handle does not contain an effect instance.
  bool isEmpty() const { return _fxAdapter.get() != nullptr; }

  /** Render the contained effect instance on the Segment (if any).
   * This method does nothing when the handle empty.
   * @param now The current timestamp (in ms).
   * @return The effect's frametime (in ms), or 0 when empty.
   */
  uint16_t showEffect(uint32_t now);

  /** Notify about any kind of Segment changes.
   * Call this method when any property of the Segment has changed, like UI settings, dimension, ...
   * @retval \c true Success or handle was empty.
   * @retval \c false Failure; a new effect instance has to be created.
   * @note Be aware that some effects might not be able to handle dimention changes. In that case,
   * the effect instance is destroyed, resulting in an empty handle.
   * It is then up to the Segment to create a new effect instance at the next frame.
   */
  bool onSegmentChanges();

  /// Delete this handle's current effect instance (if any).
  void reset() { _fxAdapter.reset(); }

  /** Try to clone the effect instance from \a src into this handle.
   * @note Be aware that cloning might fail, resulting in an empty handle.
   */
  void cloneEffectFrom(const EffectHandle &src);

  /** Try to clone this handle's effect instance into \a dest
   * @note Be aware that cloning might fail, resulting in an empty handle.
   */
  void cloneEffectTo(EffectHandle &dest) const { dest.cloneEffectFrom(*this); }

  /// Transfer the effect instance from \a src to this handle.
  void moveEffectFrom(EffectHandle &src) noexcept;

  /// Transfer this handle's effect instance to \a dest
  void moveEffectTo(EffectHandle &dest) noexcept { dest.moveEffectFrom(*this); }

  /** Perform adjustments after raw copying the Segment via \c memcpy()
   * Call this method directly after \c memcpy() inside the copy operations of the Segment.
   * It must be called on the new (copied) EffectHandle instance, with the new Segment as parameter.
   * @param newSeg The new (copied) Segment.
   * @note The original instance of the effect class is transferred to \a newSeg - and a clone
   * of the original effect is created in the original Segment.
   * If that effect class doesn't support cloning, it is up to the Segment to create a new effect
   * instance at the next frame.
   */
  void adjustAfterRawCopy(Segment &newSeg);

  /** Perform adjustments after raw moving the Segment via \c memcpy()
   * Call this method directly after \c memcpy() inside the move operations of the Segment.
   * It must be called on the moved-to EffectHandle instance, with the moved-to Segment as parameter.
   * @param newSeg The moved-to Segment.
   */
  void adjustAfterRawMove(Segment &newSeg);

  EffectHandle(const EffectHandle &other) { cloneEffectFrom(other); }
  EffectHandle(EffectHandle &&other) noexcept { moveEffectFrom(other); }
  EffectHandle &operator=(const EffectHandle &other)
  {
    cloneEffectFrom(other);
    return *this;
  }
  EffectHandle &operator=(EffectHandle &&other) noexcept
  {
    moveEffectFrom(other);
    return *this;
  }

private:
  void updateSegment(Segment &seg);

  Segment *_seg = nullptr;
  EffectAdapterPtr _fxAdapter;
};

//--------------------------------------------------------------------------------------------------

/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * @file This file provides a handle for the Segment to manage its corresponding effect instance.
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

/// Internal use only.
/// Arguments for creating a new EffectAdapter.
struct EffectInitData
{
  Segment *seg; //< The segment wo work on.
  uint32_t now; //< The current timestamp (in ms).
};

/** This class represents the one and only interface for Segments to interact with class-based WLED effects.
 * It provides the functionality to create, destroy and render the effect; therefore it also claims
 * full responsibility over the effect's lifetime (create, destroy, copy/clone, move).
 * @note Be aware that some effects might not be able to handle dimension changes or cannot be
 * copied/cloned. In that case, the contained effect instance is destroyed, resulting in an empty
 * handle. It is then up to the Segment to create a new effect instance at the next frame.
 */
class EffectHandle
{
public:
  EffectHandle(const EffectHandle &other);
  EffectHandle(EffectHandle &&other) noexcept;
  EffectHandle &operator=(const EffectHandle &other);
  EffectHandle &operator=(EffectHandle &&other) noexcept;
  ~EffectHandle();

  /** Constructor.
   * @param seg The Segment where this handle lives inside.
   */
  explicit EffectHandle(Segment &seg);

  /** Create an instance of effect class type \a FX_CLASS inside this handle.
   * @tparam FX_CLASS Class type of concrete effect implementation. Must be a child of EffectBase.
   * @tparam FX_ARGS Constructor argument pack for FX_CLASS.
   * @param now The current timestamp (in ms).
   * @param fxArgs All these (optional) arguments are forwarded to the constructor of \a FX_CLASS
   * @note To minimize the code size of this templated function, it does not destroy the old effect
   * before the new one is created.
   * So, to reduce heap usage, ensure that this handle is empty -- by calling reset() -- before
   * calling this method.
   */
  template <class FX_CLASS, typename... FX_ARGS>
  void createEffect(uint32_t now, FX_ARGS &&...fxArgs);

  /// Check if this handle does not contain an effect instance.
  bool isEmpty() const { return _fxAdapter.get() == nullptr; }

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
  void reset();

  /** Try to clone the effect instance from \a src into this handle.
   * @note Be aware that cloning might fail, resulting in an empty handle.
   */
  void cloneEffectFrom(const EffectHandle &src);

  /** Try to clone this handle's effect instance into \a dest
   * @note Be aware that cloning might fail, resulting in an empty handle.
   */
  void cloneEffectTo(EffectHandle &dest) const;

  /// Transfer the effect instance from \a src to this handle.
  void moveEffectFrom(EffectHandle &src) noexcept;

  /// Transfer this handle's effect instance to \a dest
  void moveEffectTo(EffectHandle &dest) noexcept;

  /// Swaps this handle's effect instance with the one of \a other
  void swap(EffectHandle &other) noexcept;

  /** Perform adjustments after raw copying the Segment via \c memcpy()
   * Call this method after \c memcpy() inside the copy operations of the Segment.
   * It must be called on the new (copied) EffectHandle instance, with the new Segment as parameter.
   * @param newSeg The new (copied) Segment.
   * @note The original instance of the effect class is transferred to \a newSeg - and a clone
   * of the original effect is created in the original Segment.
   * If that effect class doesn't support cloning, it is up to the Segment to create a new effect
   * instance at the next frame.
   */
  void adjustAfterRawCopy(Segment &newSeg);

  /** Perform adjustments after raw moving the Segment via \c memcpy()
   * Call this method after \c memcpy() inside the move operations of the Segment.
   * It must be called on the moved-to EffectHandle instance, with the moved-to Segment as parameter.
   * @param newSeg The moved-to Segment.
   */
  void adjustAfterRawMove(Segment &newSeg);

private:
  void updateSegment(Segment &seg);

  EffectInitData _fxData;
  EffectAdapterPtr _fxAdapter;
};

/// Swaps the effect instance of \a lhs and \a rhs
inline void swap(EffectHandle &lhs, EffectHandle &rhs) noexcept { lhs.swap(rhs); }

//--------------------------------------------------------------------------------------------------

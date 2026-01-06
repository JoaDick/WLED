/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * This headerfile provides the glue code between the EffectHandle (for the Segments) and the
 * EffectAPI (for the custom effect implementations).
 * All helper classes from here should never be used directly, neither by WLED nor by any effect.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include <type_traits>

#include "EffectAPI.h"
#include "EffectHandle.h"

//--------------------------------------------------------------------------------------------------

/** TBD.
 */
class EffectController : private FxSetup, private FxEnv, private FxProperties
{
public:
  // no copy & move
  EffectController(EffectController &&) = delete;
  EffectController &operator=(const EffectController &) = delete;
  EffectController &operator=(EffectController &&) = delete;

  /** Constructor.
   * @param seg TBD
   * @param now TBD
   * @param effect The effect instance to control.
   * @note Be aware that \a effect is not initialized yet, so don't use it here!
   */
  EffectController(Segment &seg, uint32_t now, RawEffectPtr effect) : FxEnv{seg, now}
  {
    FxEnv::storeEffect(effect);
  }

  /** Pseudo-copy-constructor.
   * @param other The original instance to copy from.
   * @param effect The CLONED effect instance to control; overrides the one from \a other
   * @note Be aware that \a effect is not initialized yet, so don't use it here!
   */
  EffectController(const EffectController &other, RawEffectPtr effect) : EffectController{other}
  {
    FxEnv::storeEffect(effect);
  }

  /// Get the setup data to provide to the effect's constructor.
  FxSetup &getFxSetup() { return *this; }

  /** Render the effect's pixel magic on the segment.
   * @param now The current timestamp (in ms).
   * @return Specific frametime (in ms), or 0 to use WLED's default setting.
   */
  uint16_t showEffect(uint32_t now) { return FxEnv::showEffect(now); }

  /** Call this method when the segment or any of its setting has changed.
   * @param seg The changed segment wo work on from now.
   * @retval \c true Success.
   * @retval \c false Effekt is broken - a new instance has to be created.
   */
  bool updateSegment(Segment &seg);

private:
  EffectController(const EffectController &) = default;
  FxProperties &props() override { return *this; }
  FxEnv &env() override { return *this; }
};

//--------------------------------------------------------------------------------------------------

/** TBD.
 */
class EffectAdapter
{
public:
  /// Arguments for initializing the EffectAdapter.
  struct InitData
  {
    Segment &seg; //< The segment wo work on.
    uint32_t now; //< The current timestamp (in ms).
  };

  // no copy & move - use clone() instead
  EffectAdapter(const EffectAdapter &) = delete;
  EffectAdapter(EffectAdapter &&) = delete;
  EffectAdapter &operator=(const EffectAdapter &) = delete;
  EffectAdapter &operator=(EffectAdapter &&) = delete;
  virtual ~EffectAdapter() = default;

  /** Create an EffectAdapter for the given \a FX_TYPE effect class.
   * @tparam FX_TYPE Class type of concrete effect implementation. Must be a child of EffectBase.
   * @param seg The segment wo work on.
   * @param now The current timestamp (in ms).
   */
  template <class FX_TYPE>
  static EffectAdapterPtr create(Segment &seg, uint32_t now)
  {
    InitData data{seg, now};
    return create<FX_TYPE>(data);
  }

  /// Create an EffectAdapter for the given \a FX_TYPE effect class with the given \a initData
  template <class FX_TYPE>
  static EffectAdapterPtr create(InitData &initData);

  /** Render the effect's pixel magic on the segment.
   * @param now The current timestamp (in ms).
   * @return Specific frametime (in ms), or 0 to use WLED's default setting.
   */
  uint16_t showEffect(uint32_t now) { return _fxController.showEffect(now); }

  /** Call this method when the segment or any of its setting has changed.
   * @param seg The changed segment wo work on from now.
   * @retval \c true Success.
   * @retval \c false Effekt is broken - a new instance has to be created.
   */
  bool updateSegment(Segment &seg) { return _fxController.updateSegment(seg); }

  /** Create a clone of this EffectAdapter instance, including the effect itself.
   * @param seg The (new) segment for the clone wo work on.
   * @note Returns \c nullptr when the effect cannot be cloned.
   */
  EffectAdapterPtr clone(Segment &seg);

protected:
  /** Constructor.
   * @param data Arguments for initializing the EffectAdapter.
   * @param effect The effect instance to control.
   * Be aware that \a effect is not initialized yet, so don't use it here!
   */
  EffectAdapter(InitData &data, RawEffectPtr effect) : _fxController{data.seg, data.now, effect} {}

  /** Pseudo-copy-constructor.
   * @param other The original instance to copy from.
   * @param effect The CLONED effect instance to control; overrides the one from \a other
   * @note Be aware that \a effect is not initialized yet, so don't use it here!
   */
  EffectAdapter(const EffectAdapter &other, RawEffectPtr effect) : _fxController{other._fxController, effect} {}

  /// Get the setup data to provide to the effect's constructor.
  FxSetup &getFxSetup() { return _fxController.getFxSetup(); }

  /** Create a clone of this EffectAdapter instance.
   * Must be implemented by all child classes.
   * @note May return \c nullptr when the child's effect cannot be copied.
   */
  virtual EffectAdapterPtr do_clone() = 0;

private:
  EffectController _fxController;
};

//--------------------------------------------------------------------------------------------------

/** Concrete implementation of an EffectAdapter that is using \a FX_TYPE as effect class.
 * The sole purpose of this class is to hold an instance of the desired effect type, and to make a
 * clone of it when requested (if possible).
 * @tparam FX_TYPE Class type of concrete effect implementation. Must be a child of EffectBase.
 */
template <class FX_TYPE>
class EffectAdapterImpl : public EffectAdapter
{
public:
  static EffectAdapterPtr do_create(InitData &data) { return EffectAdapterPtr{new (std::nothrow) EffectAdapterImpl(data)}; }

private:
  /// Constructor.
  explicit EffectAdapterImpl(InitData &data) : EffectAdapter{data, &_effect}, _effect{getFxSetup()} {}

  /// Copy constructor.
  EffectAdapterImpl(const EffectAdapterImpl &other) : EffectAdapter{other, &_effect}, _effect{other._effect} {}

  /// @copydoc EffectAdapter::do_clone()
  EffectAdapterPtr do_clone() override { return do_clone(std::is_copy_constructible<FX_TYPE>{}); }

  // FX_TYPE has a copy constructor --> great, let's make a copy :-)
  EffectAdapterPtr do_clone(std::true_type) { return EffectAdapterPtr{new (std::nothrow) EffectAdapterImpl(*this)}; }

  // FX_TYPE does not have a copy constructor --> there's nothing we can do :-(
  EffectAdapterPtr do_clone(std::false_type) { return nullptr; }

  FX_TYPE _effect;
};

template <class FX_TYPE>
EffectAdapterPtr EffectAdapter::create(InitData &data) { return EffectAdapterImpl<FX_TYPE>::do_create(data); }

//--------------------------------------------------------------------------------------------------

template <class FX_TYPE>
void EffectHandle::createEffect(uint32_t now)
{
  reset();
  _fxAdapter = EffectAdapter::create<FX_TYPE>(*_seg, now);
}

//--------------------------------------------------------------------------------------------------

/** Mode function for all class-based effects (to be registered at the WLED framework).
 * @tparam FX_TYPE Class type of concrete effect implementation. Must be a child of EffectBase.
 * @see addWledEffect()
 * @note This mode function is abused as factory for the effect class. It does \e not render the
 * effect on the Segment, as the "normal" mode functions do.
 */
template <class FX_TYPE>
uint16_t mode_EffectClass()
{
  extern WS2812FX strip;
  SEGMENT.createEffect<FX_TYPE>(strip.now);
  return 0;
}

/** Register a class-based effect at the WLED framework.
 * @tparam FX_TYPE Class type of concrete effect implementation. Must be a child of EffectBase.
 * @param wled WS2812FX instance representing the WLED framework - a.k.a. \c strip
 * @return The actual ID that is assigned to the effect, or 255 on failure.
 */
template <class FX_TYPE>
uint8_t addEffectClass(WS2812FX &wled, uint8_t FX_id, const char *FX_data)
{
  return wled.addEffect(FX_id, &mode_EffectClass<FX_TYPE>, FX_data);
}

//--------------------------------------------------------------------------------------------------

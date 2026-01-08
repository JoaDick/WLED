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
class EffectController : private SegEnv, private FxSetup, private FxEnv, private FxProperties
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
  EffectController(Segment &seg, uint32_t now, RawEffectPtr effect) : FxEnv{seg, *this, now}
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
  uint16_t showEffect(uint32_t now)
  {
    const uint16_t frametime = FxEnv::showEffect(now);
    SegEnv::next();
    return frametime;
  }

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

  void onSegEnvAllocFailed() override { FxEnv::setBroken(); }
};

//--------------------------------------------------------------------------------------------------

/** TBD.
 */
class EffectAdapter
{
public:
  // no copy & move - use clone() instead
  EffectAdapter(const EffectAdapter &) = delete;
  EffectAdapter(EffectAdapter &&) = delete;
  EffectAdapter &operator=(const EffectAdapter &) = delete;
  EffectAdapter &operator=(EffectAdapter &&) = delete;
  virtual ~EffectAdapter() = default;

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
  EffectAdapter(EffectInitData &data, RawEffectPtr effect) : _fxController{*data.seg, data.now, effect} {}

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

/** Concrete implementation of an EffectAdapter that is using \a FX_CLASS as effect class.
 * The sole purpose of this class is to hold an instance of the desired effect type, and to make a
 * clone of it when requested (if possible).
 * @tparam FX_CLASS Class type of concrete effect implementation. Must be a child of EffectBase.
 */
template <class FX_CLASS>
class EffectAdapterImpl : public EffectAdapter
{
public:
  /// Create an EffectAdapter. All \a fxArgs are forwarded to the constructor of \a FX_CLASS
  template <typename... FX_ARGS>
  static EffectAdapterPtr create(EffectInitData &data, FX_ARGS &&...fxArgs)
  {
    return EffectAdapterPtr{new (std::nothrow) EffectAdapterImpl(data, std::forward<FX_ARGS>(fxArgs)...)};
  }

private:
  /// Constructor. All \a fxArgs are forwarded to the constructor of \a FX_CLASS
  template <typename... FX_ARGS>
  explicit EffectAdapterImpl(EffectInitData &data, FX_ARGS &&...fxArgs)
      : EffectAdapter{data, &_effect}, _effect{getFxSetup(), std::forward<FX_ARGS>(fxArgs)...} {}

  /// Copy constructor.
  EffectAdapterImpl(const EffectAdapterImpl &other) : EffectAdapter{other, &_effect}, _effect{other._effect} {}

  /// @copydoc EffectAdapter::do_clone()
  EffectAdapterPtr do_clone() override { return do_clone(std::is_copy_constructible<FX_CLASS>{}); }

  // FX_CLASS has a copy constructor --> great, let's make a copy :-)
  EffectAdapterPtr do_clone(std::true_type) { return EffectAdapterPtr{new (std::nothrow) EffectAdapterImpl(*this)}; }

  // FX_CLASS does not have a copy constructor --> there's nothing we can do :-(
  EffectAdapterPtr do_clone(std::false_type) { return nullptr; }

  FX_CLASS _effect;
};

//--------------------------------------------------------------------------------------------------

template <class FX_CLASS, typename... FX_ARGS>
void EffectHandle::createEffect(uint32_t now, FX_ARGS &&...fxArgs)
{
  _fxData.now = now;
  _fxAdapter = EffectAdapterImpl<FX_CLASS>::create(_fxData, std::forward<FX_ARGS>(fxArgs)...);
}

//--------------------------------------------------------------------------------------------------

/// Effect class to act as adapter for new style effect-functions into the class-based backend.
class EffectFunctionWrapper : public EffectBase
{
public:
  EffectFunctionWrapper(FxSetup &fxs, EffectFunction fxFct) : EffectBase{fxs}, _fxFct{fxFct} {}

private:
  void showEffect(FxEnv &env) override { _fxFct(env); }
  EffectFunction _fxFct;
};

/// Effect class to act as adapter for the existing mode-functions into the class-based backend.
class ModeFunctionWrapper : public EffectBase
{
public:
  ModeFunctionWrapper(FxSetup &fxs, ModeFunction modeFct) : EffectBase{fxs}, _modeFct{modeFct} {}

private:
  void showEffect(FxEnv &env) override { env.setFrametime(_modeFct()); }
  ModeFunction _modeFct;
};

//--------------------------------------------------------------------------------------------------

/** "Pseudo" mode-function for class-based effects (to be registered at the WLED framework).
 * @tparam FX_CLASS Class type of concrete effect implementation. Must be a child of EffectBase.
 * @see addEffectClass()
 * @note This mode-function is abused as factory for the effect class. It does \e not render the
 * effect on the Segment, as the "normal" mode functions do.
 */
template <class FX_CLASS>
uint16_t mode_EffectClass()
{
  extern WS2812FX strip;
  SEGMENT.createEffect<FX_CLASS>(strip.now);
  return 0;
}

/** "Pseudo" mode-function to use a new style effect-function via wrapper as class-based effect.
 * @tparam FX_FCT The new style effect-function.
 * @see addEffectFunction()
 */
template <EffectFunction FX_FCT> // Yes, the specific function is committed as template argument!
uint16_t mode_EffectFunctionWrapper()
{
  extern WS2812FX strip;
  SEGMENT.createEffect<EffectFunctionWrapper>(strip.now, FX_FCT);
  return 0;
}

/** "Pseudo" mode-function to use an existing mode-function via wrapper as class-based effect.
 * @tparam MODE_FCT The already existing effect's mode-function.
 * @see addNodeFunction()
 */
template <ModeFunction MODE_FCT> // Yes, the specific function is committed as template argument!
uint16_t mode_ModeFunctionWrapper()
{
  extern WS2812FX strip;
  SEGMENT.createEffect<ModeFunctionWrapper>(strip.now, MODE_FCT);
  return 0;
}

//--------------------------------------------------------------------------------------------------

/** Register a class-based effect at the WLED framework.
 * @tparam FX_CLASS Class type of concrete effect implementation. Must be a child of EffectBase.
 * @param wled WS2812FX instance representing the WLED framework - a.k.a. \c strip
 * @return The actual ID that is assigned to the effect, or 255 on failure.
 */
template <class FX_CLASS>
uint8_t addEffectClass(WS2812FX &wled, uint8_t FX_id, const char *FX_data)
{
  return wled.addEffect(FX_id, &mode_EffectClass<FX_CLASS>, FX_data);
}

/** Register a new style effect-function via class-based effect at the WLED framework.
 * @tparam FX_FCT The new style effect-function
 * @param wled WS2812FX instance representing the WLED framework - a.k.a. \c strip
 * @return The actual ID that is assigned to the effect, or 255 on failure.
 */
template <EffectFunction FX_FCT> // Yes, the concrete function is committed as template argument!
uint8_t addEffectFunction(WS2812FX &wled, uint8_t FX_id, const char *FX_data)
{
  return wled.addEffect(FX_id, &mode_EffectFunctionWrapper<FX_FCT>, FX_data);
  return 0;
}

/** Register an existing mode-function via class-based effect at the WLED framework.
 * @tparam MODE_FCT The existing mode-function
 * @param wled WS2812FX instance representing the WLED framework - a.k.a. \c strip
 * @return The actual ID that is assigned to the effect, or 255 on failure.
 */
template <ModeFunction MODE_FCT> // Yes, the concrete function is committed as template argument!
uint8_t addModeFunction(WS2812FX &wled, uint8_t FX_id, const char *FX_data)
{
  return wled.addEffect(FX_id, &mode_ModeFunctionWrapper<MODE_FCT>, FX_data);
  return 0;
}

//--------------------------------------------------------------------------------------------------

/**
 * Interface and helpers for creating class-based WLED effects.
 *
 * (c) 2025 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "FX.h"

// effect cloning is most likely needed by https://github.com/wled/WLED/issues/4550
#define WLED_EFFECT_ENABLE_CLONE 1

//--------------------------------------------------------------------------------------------------

/// Datatype of an Effect-ID.
using EffectID = uint8_t;

/// Special Effect-ID value that lets WLED decide which ID to use eventually for that effect.
constexpr EffectID AutoSelectEffectID = 255;

/** Runtime environment for rendering the effects.
 * The effect implementations shall obtain necessary runtime information for rendering their
 * animation from here.
 * @see WledEffect::showWledEffect()
 */
class FxEnv
{
public:
  // no copy & move - this class is intended to be passed as reference to other functions
  FxEnv(const FxEnv &) = delete;
  FxEnv(FxEnv &&) = delete;
  FxEnv &operator=(const FxEnv &) = delete;
  FxEnv &operator=(FxEnv &&) = delete;
  FxEnv(Segment &seg, uint32_t now) { update(now, &seg); }

  /// The current timestamp - effect implementations shall use this instead of \c strip.now
  uint32_t now() const { return _now; }

  /// Effect implementations shall use this instead of \c SEGMENT
  Segment &seg() { return *_seg; }

  /// Effect implementations shall use this instead of \c SEGLEN
  uint16_t seglen() const { return _seglen; }

  /// Effect implementations shall use this instead of \c SEG_W
  uint16_t segW() const { return _segW; }

  /// Effect implementations shall use this instead of \c SEG_H
  uint16_t segH() const { return _segH; }

  /// Not for public use.
  void update(uint32_t now, Segment *seg = nullptr)
  {
    _now = now;
    if (seg && (seg != _seg))
    {
      _seg = seg;
      _seglen = seg->vLength();
      _segW = seg->vWidth();
      _segH = seg->vHeight();
    }
  }

private:
  uint32_t _now = 0;
  Segment *_seg;
  uint16_t _seglen = 0;
  uint16_t _segW = 0;
  uint16_t _segH = 0;
};

/// Effect setup data.
struct FxSetup
{
  FxEnv &env;
};

//--------------------------------------------------------------------------------------------------

/** Interface for class-based WLED effects.
 * Class-based WLED effects must derive from this class. This requires them to implement the
 * \c showWledEffect() method, where all the fancy pixel magic shall happen.
 * Additionally, all children must provide these public members:
 * @code
 * class Fx_MyEffectClass : public WledFxBase
 * {
 * public:
 *   static const EffectID FX_id = AutoSelectEffectID; // or the effect's specific ID
 *   static const char     FX_data[];
 *   // ...
 * };
 * const char Fx_MyEffectClass::FX_data[] PROGMEM = "My Name@...";
 * @endcode
 * @note For the format of \c FX_data see https://kno.wled.ge/interfaces/json-api/#effect-metadata
 */
class WledEffect
{
public:
  // no copy & move
#if (!WLED_EFFECT_ENABLE_CLONE)
  WledEffect(const WledEffect &) = delete;
#endif
  WledEffect(WledEffect &&) = delete;
  WledEffect &operator=(const WledEffect &) = delete;
  WledEffect &operator=(WledEffect &&) = delete;
  virtual ~WledEffect() = default;

  /** Create a new WLED effect instance.
   * @param fxs Is passed to the effect's constructor.
   * @tparam FX_TYPE Class type of concrete effect implementation. Must be a child of WledEffect.
   */
  template <class FX_TYPE>
  static WledEffectPtr create(FxSetup &fxs)
  {
    return WledEffectPtr(new (std::nothrow) FX_TYPE(fxs));
  }

  /** Render the custom WLED effect.
   * @param env Runtime environment for rendering the effect.
   * @return The Effect's frametime (in ms), or 0 to use default frametime.
   */
  uint16_t show(FxEnv &env) { return showWledEffect(env); }

#if (WLED_EFFECT_ENABLE_CLONE)
  /** Clone this WLED effect instance.
   * @note Be aware that this method will return \c nullptr when cloning is not possible!
   */
  WledEffectPtr clone() { return cloneWledEffect(); }
#endif

protected:
#if (WLED_EFFECT_ENABLE_CLONE)
  /// Copy constructor (just for cloning).
  WledEffect(const WledEffect &) = default;
#endif

  /// Constructor; child classes shall just pass \a fxs to their base class.
  explicit WledEffect(FxSetup &fxs) {}

  /** Rendering function for the custom WLED effect.
   * Must be implemented by all child classes to show their specific animation on the Segment.
   * @param env Runtime environment for rendering the effect.
   * @return The Effect's frametime (in ms), or 0 to use default frametime.
   */
  virtual uint16_t showWledEffect(FxEnv &env) = 0;

  /** Fallback rendering function.
   * Can be called as fallback by a child when it cannot render its own stuff, e.g. when something
   * like allocating additional effect memory went wrong.
   * @param env Runtime environment for rendering the effect.
   * @return Always 0 --> default frametime shall be used.
   */
  uint16_t showFallbackEffect(FxEnv &env)
  {
    env.seg().fill(env.seg().getCurrentColor(0));
    return 0;
  }

  /** Emergency back-out function.
   * Can be called by a child when it is in an unrecoverable state, e.g. when the segment's
   * dimensions have changed and the internal data structures are now completely messed up. It will
   * destroy the current effect instance in order to create a new one upon the next frame.
   * @param env Runtime environment for rendering the effect.
   * @return Always 0 --> default frametime shall be used.
   */
  uint16_t pleaseKillMe(FxEnv &env)
  {
    env.seg().effect.reset();
    return 0;
  }

#if (WLED_EFFECT_ENABLE_CLONE)
  /** Make a clone of the custom WLED effect instance.
   * Must be implemented by all child classes. Oftentimes it can look just like this:
   * @code
   * WledEffectPtr cloneWledEffect() override { return makeClone(this); }
   * @endcode
   * @note For more complex effects, it might be necessary for the child to implement a copy
   * constructor that performs a deep copy. \n
   * Or just return \c nullptr if cloning is not possible or doesn't work as expected.
   */
  virtual WledEffectPtr cloneWledEffect() = 0;

  /// Helper for \c cloneWledEffect()
  template <class FX_TYPE>
  WledEffectPtr makeClone(FX_TYPE *effect)
  {
    return effect ? WledEffectPtr(new (std::nothrow) FX_TYPE(*effect)) : nullptr;
  }
#endif
};

//--------------------------------------------------------------------------------------------------

/// Internal helper function.
inline uint16_t render_WledEffect(FxEnv &env, uint16_t defaultFrametime)
{
  const uint16_t frametime = env.seg().effect->show(env);
  return frametime ? frametime : defaultFrametime;
}

/** Mode function for all class-based effects (to be registered at the WLED framework).
 * @tparam FX_TYPE Class type of concrete effect implementation. Must be a child of WledEffect.
 * @see addWledEffect()
 */
template <class FX_TYPE>
uint16_t mode_WledEffect()
{
  extern WS2812FX strip;
  Segment &seg = strip._segments[strip.getCurrSegmentId()]; /* SEGMENT */
  FxEnv env(seg, strip.now);
  if (!seg.effect)
  {
    FxSetup fxs{env};
    seg.effect = WledEffect::create<FX_TYPE>(fxs);
  }
  return render_WledEffect(env, strip.getFrameTime() /* FRAMETIME */);
}

/** Register a class-based effect at the WLED framework.
 * @tparam FX_TYPE Class type of concrete effect implementation. Must be a child of WledEffect.
 * @param wled WS2812FX instance representing the WLED framework - a.k.a. \c strip
 * @return The actual ID that is assigned to the effect, or 255 on failure.
 */
template <class FX_TYPE>
uint8_t addWledEffect(WS2812FX &wled,
                      EffectID FX_id = FX_TYPE::FX_id, const char *FX_data = FX_TYPE::FX_data)
{
  return wled.addEffect(FX_id, &mode_WledEffect<FX_TYPE>, FX_data);
}

//--------------------------------------------------------------------------------------------------

/** Simple base class which should be suitable for most class-based effects.
 * Child classes must implement the \c showEffect() method. Additionally, they can implement the
 * \c initEffect() method, which may be used for one-time initialization of internal state. \n
 * This base class also checks if the Segment's dimensions have changed. If so, the current effect
 * instance is destroyed (and recreated automatically upon the next frame).
 */
class WledFxBase : public WledEffect
{
protected:
#if (WLED_EFFECT_ENABLE_CLONE)
  /// Copy constructor (just for cloning).
  WledFxBase(const WledFxBase &) = default;
#endif

  /// Constructor; child classes shall just pass \a fxs to their base class.
  explicit WledFxBase(FxSetup &fxs) : WledEffect(fxs),
                                      _seglen(fxs.env.seg().vLength()),
                                      _segW(fxs.env.seg().vWidth()),
                                      _segH(fxs.env.seg().vHeight()) {}

  /** Initialization function for the custom WLED effect.
   * Can optionally be implemented by child classes to initialize their internal data.
   * This method is called once before \c showEffect() is going to be called for the first time.
   * @param env Runtime environment for initializing the effect.
   * @note Implementing that method in the child class is the alternative for this handmade check:
   * @code
   * if (SEGENV.call == 0) {
   *   // do effect specific init stuff
   * }
   * @endcode
   */
  virtual void initEffect(FxEnv &env);

  /** Rendering function for the custom WLED effect.
   * Must be implemented by all child classes to show their specific animation on the Segment.
   * @param env Runtime environment for rendering the effect.
   * @return The Effect's frametime (in ms), or 0 to use default frametime.
   */
  virtual uint16_t showEffect(FxEnv &env) = 0;

private:
  uint16_t showWledEffect(FxEnv &env) final;
  bool mustRecreate(const FxEnv &env) const;

  const uint16_t _seglen;
  const uint16_t _segW;
  const uint16_t _segH;
};

//--------------------------------------------------------------------------------------------------

// DRAFT: Create effects via factory.

class FxFactory;
using FxFactoryPtr = std::unique_ptr<FxFactory>;

class FxFactory
{
public:
  FxFactory(const FxFactory &) = delete;
  FxFactory(FxFactory &&) = delete;
  FxFactory &operator=(const FxFactory &) = delete;
  FxFactory &operator=(FxFactory &&) = delete;
  virtual ~FxFactory() = default;

  EffectID FX_id() const { return _FX_id; }
  const char *FX_data() const { return _FX_data; }
  WledEffectPtr makeEffect(FxSetup &fxs) { return do_makeEffect(fxs); }

protected:
  FxFactory(EffectID FX_id, const char *FX_data) : _FX_id(FX_id), _FX_data(FX_data) {}
  virtual WledEffectPtr do_makeEffect(FxSetup &fxs) = 0;

private:
  EffectID _FX_id;
  const char *_FX_data;
};

template <class FX_TYPE>
class DefaultFxFactory : public FxFactory
{
public:
  // used by makeFactory() below - when the factory shall be stored in a std::vector<FxFactoryPtr>
  static FxFactoryPtr create(EffectID FX_id, const char *FX_data)
  {
    return FxFactoryPtr(new (std::nothrow) DefaultFxFactory<FX_TYPE>(FX_id, FX_data));
  }

  // used by REGISTER_EFFECT(MyEffectClass, id, data) below - to be preferred
  DefaultFxFactory(EffectID FX_id, const char *FX_data) : FxFactory(FX_id, FX_data) {}

private:
  WledEffectPtr do_makeEffect(FxSetup &fxs) override { return FX_TYPE::create(fxs); }
};

template <class FX_TYPE>
FxFactoryPtr makeFactory(EffectID FX_id, const char *FX_data)
{
  return DefaultFxFactory<FX_TYPE>::create(FX_id, FX_data);
}

template <class FX_TYPE>
FxFactoryPtr makeFactory()
{
  return makeFactory<FX_TYPE>(FX_TYPE::FX_id, FX_TYPE::FX_data);
}

// same linker magic as for REGISTER_USERMOD() from there: https://github.com/wled/WLED/pull/4480
// see also "thoughts 1" in this comment: https://github.com/wled/WLED/pull/4549#issuecomment-2695943205
// #define REGISTER_USERMOD(x) Usermod* const um_##x __attribute__((__section__(".dtors.tbl.usermods.1"), used)) = &x
#define REGISTER_EFFECT(FX_CLASS, FX_ID, FX_DATA)     \
  static FX_CLASS factory_##FX_CLASS(FX_ID, FX_DATA); \
  FxFactory *const fx_factory_##FX_CLASS __attribute__((__section__(".dtors.tbl.effects.1"), used)) = &factory_##FX_CLASS

//--------------------------------------------------------------------------------------------------

// DRAFT: Encapsulate existing mode_XYZ() functions into WledEffect class.

using FxModeFct = uint16_t (*)();

class ModeFctFxFactory : public FxFactory
{
public:
  // used by makeFactory() below - when the factory shall be stored in a std::vector<FxFactoryPtr>
  static FxFactoryPtr create(FxModeFct FX_fct, EffectID FX_id, const char *FX_data)
  {
    return FxFactoryPtr(new (std::nothrow) ModeFctFxFactory(FX_fct, FX_id, FX_data));
  }

  // used by REGISTER_MODE_FCT(mode_fct, id, data) below - to be preferred
  ModeFctFxFactory(FxModeFct FX_fct, EffectID FX_id, const char *FX_data)
      : FxFactory(FX_id, FX_data), _FX_fct(FX_fct) {}

private:
  class FctWrapper : public WledEffect
  {
    FxModeFct _FX_fct;
#if (WLED_EFFECT_ENABLE_CLONE)
    WledEffectPtr cloneWledEffect() override { return makeClone(this); }
#endif
    uint16_t showWledEffect(FxEnv &) override { return _FX_fct(); }
    FctWrapper(FxSetup &fxs, FxModeFct FX_fct) : WledEffect(fxs), _FX_fct(FX_fct) {}

  public:
    static WledEffectPtr create(FxSetup &fxs, FxModeFct FX_fct)
    {
      return WledEffectPtr(new (std::nothrow) FctWrapper(fxs, FX_fct));
    }
  };

  WledEffectPtr do_makeEffect(FxSetup &fxs) override { return FctWrapper::create(fxs, _FX_fct); }
  FxModeFct _FX_fct;
};

inline FxFactoryPtr makeFactory(FxModeFct FX_fct, EffectID FX_id, const char *FX_data)
{
  return ModeFctFxFactory::create(FX_fct, FX_id, FX_data);
}

// same linker magic as for REGISTER_USERMOD() from there: https://github.com/wled/WLED/pull/4480
// see also "thoughts 2" in this comment: https://github.com/wled/WLED/pull/4549#issuecomment-2695943205
// #define REGISTER_USERMOD(x) Usermod* const um_##x __attribute__((__section__(".dtors.tbl.usermods.1"), used)) = &x
#define REGISTER_MODE_FCT(MODE_FCT, FX_ID, FX_DATA)                      \
  static ModeFctFxFactory factory_##MODE_FCT(&MODE_FCT, FX_ID, FX_DATA); \
  FxFactory *const fx_factory_##MODE_FCT __attribute__((__section__(".dtors.tbl.effects.1"), used)) = &factory_##MODE_FCT

//--------------------------------------------------------------------------------------------------

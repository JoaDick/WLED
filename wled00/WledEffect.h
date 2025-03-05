/**
 * Interface and helpers for creating class-based WLED effects.
 *
 * (c) 2025 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "FX.h"

#define WLED_EFFECT_ENABLE_CLONE 1

//--------------------------------------------------------------------------------------------------

/// Datatype of an Effect-ID.
using EffectID = uint8_t;

/// Special Effect-ID value that lets WLED decide which ID to use eventually for that effect.
constexpr EffectID AutoSelectEffectID = 255;

/** Runtime environment for rendering the effects.
 * The effect implementations shall obtain necessary runtime information for rendering their
 * animation from here.
 */
class FxEnv
{
public:
  // no copy & move - this class is intended to be passed as reference to other functions
  FxEnv(const FxEnv &) = delete;
  FxEnv(FxEnv &&) = delete;
  FxEnv &operator=(const FxEnv &) = delete;
  FxEnv &operator=(FxEnv &&) = delete;
  FxEnv(Segment &seg, uint32_t now) { update(seg, now); }

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

private:
  friend class WledEffect;
  void update(Segment &seg, uint32_t now)
  {
    _seg = &seg;
    _now = now;
    _seglen = seg.vLength();
    _segW = seg.vWidth();
    _segH = seg.vHeight();
  }

  Segment *_seg;
  uint32_t _now = 0;
  uint16_t _seglen = 0;
  uint16_t _segW = 0;
  uint16_t _segH = 0;
};

//--------------------------------------------------------------------------------------------------

/// Effect setup data (for the constructors).
struct FxSetup
{
  FxEnv &env;
};

/** Interface for class-based WLED effects.
 * All class-based WLED effects must derive from this class. This requires them to implement the
 * \c showWledEffect() method, which is the replacement for a \c mode_XXX() function. All fancy
 * pixel magic that is rendered on the LED segment shall be done there.
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

  /// Destructor.
  virtual ~WledEffect() = default;

  /** Effect's mode function (to be registered at the WLED framework).
   * @tparam EFFECT_TYPE Class type of concrete effect implementation. Must be a child of WledEffect.
   * @see addWledEffect()
   */
  template <class EFFECT_TYPE>
  static uint16_t mode_function()
  {
    extern WS2812FX strip;
    const uint32_t now = strip.now;
    Segment &seg = strip._segments[strip.getCurrSegmentId()];
    if (!seg.effect)
    {
      FxEnv env(seg, now);
      FxSetup fxs{env};
      seg.effect.reset(new (std::nothrow) EFFECT_TYPE(fxs));
    }
    return render_function(seg, now, strip.getFrameTime());
  }

  /** Render the WLED effect.
   * Currently used only internally; not relevant for custom effect implementations.
   * @param seg The Segment to work on - a.k.a. \c SEGMENT
   * @param now The current timestamp - a.k.a. \c strip.now
   * @return The effect's frametime (in ms)
   */
  uint16_t render(Segment &seg, uint32_t now)
  {
    _env.update(seg, now);
    return showWledEffect(_env);
  }

  /** Fallback rendering function.
   * Can be called as fallback by an effect implementation when it cannot render its own stuff,
   * e.g. when something like allocating additional effect memory went wrong.
   * @param now The current timestamp
   * @return Always 0 --> default frametime shall be used
   */
  static uint16_t showFallbackEffect(FxEnv &env)
  {
    env.seg().fill(env.seg().getCurrentColor(0));
    return 0;
  }

#if (WLED_EFFECT_ENABLE_CLONE)
  WledEffectPtr clone() { return cloneWledEffect(); }
#endif

protected:
#if (WLED_EFFECT_ENABLE_CLONE)
  WledEffect(const WledEffect &other) : _env(*other._env._seg, other._env._now) {}
#endif

  /// Constructor; child classes shall just pass \a fxs to their base class.
  explicit WledEffect(FxSetup &fxs) : _env(fxs.env.seg(), fxs.env.now()) {}

  /** Rendering function for the custom WLED effect.
   * Must be implemented by all child classes to show their specific animation on the Segment.
   * @param env Runtime environment for rendering the effects.
   * @return The Effect's frametime (in ms), or 0 to use default frametime - a.k.a \c FRAMETIME
   */
  virtual uint16_t showWledEffect(FxEnv &env) = 0;

#if (WLED_EFFECT_ENABLE_CLONE)
  virtual WledEffectPtr cloneWledEffect() = 0;

  template <class EFFECT_TYPE>
  static WledEffectPtr makeClone(EFFECT_TYPE *effect)
  {
    return effect ? WledEffectPtr(new EFFECT_TYPE(*effect)) : nullptr;
  }
#endif

private:
  static uint16_t render_function(Segment &seg, uint32_t now, uint16_t defaultFrametime);

private:
  FxEnv _env;
};

/** Register an effect's mode function at the WLED framework.
 * For format of \a FX_data see https://kno.wled.ge/interfaces/json-api/#effect-metadata
 * @tparam EFFECT_TYPE Class type of concrete effect implementation. Must be a child of WledEffect.
 * @param strip WS2812FX instance, representing the WLED framework.
 * @return The actual id used for the effect, or 255 if the add failed.
 */
template <class EFFECT_TYPE>
uint8_t addWledEffect(WS2812FX &wled, EffectID FX_id = EFFECT_TYPE::FX_id, const char *FX_data = EFFECT_TYPE::FX_data)
{
  return wled.addEffect(FX_id, &WledEffect::mode_function<EFFECT_TYPE>, FX_data);
}

//--------------------------------------------------------------------------------------------------

/** Simple base class for WLED effects.
 * Child classes must implement the \c showEffect() method. Additionally, they can implement the
 * \c initEffect() method, which can be used for one-time initialization of internal state. \n
 * This base also checks if the Segment's dimensions have changed. If so, the effect instance is
 * destroyed and rebuilt automatically.
 */
class WledFxBase : public WledEffect
{
protected:
#if (WLED_EFFECT_ENABLE_CLONE)
  WledFxBase(const WledFxBase &) = default;
#endif

  /// Constructor; child classes shall just pass \a fxs to their base class.
  explicit WledFxBase(FxSetup &fxs) : WledEffect(fxs),
                                      _seglen(fxs.env.seg().vLength()),
                                      _segW(fxs.env.seg().vWidth()),
                                      _segH(fxs.env.seg().vHeight()) {}

  /** Initialization function for the custom WLED effect.
   * This method is called once before \c showEffect() is going to be called for the first time.
   * Can optionally be implemented by child classes to initialize their internal data.
   * @param env Runtime environment for initializing the effects.
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
   * @param env Runtime environment for rendering the effects.
   * @return The Effect's frametime (in ms), or 0 to use default frametime - a.k.a \c FRAMETIME
   */
  virtual uint16_t showEffect(FxEnv &env) = 0;

private:
  uint16_t showWledEffect(FxEnv &env) final;
  bool mustRecreate(const FxEnv &env) const;

  uint16_t _seglen;
  uint16_t _segW;
  uint16_t _segH;
};

//--------------------------------------------------------------------------------------------------

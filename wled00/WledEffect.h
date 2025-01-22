/**
 * Interfaces and utilities for creating class-based WLED effects.
 * @author Joachim Dick
 */

#pragma once

#include "FX.h"

/// Datatype of an Effect-ID.
using EffectID = std::uint8_t;

/// Special Effect-ID value that lets WLED decide which ID to use eventually for that effect.
constexpr EffectID AutoSelectID = 255;

/** Internal setup data for the effects.
 * @note Not intended to be used by the effect implementations (because it's likely to be changed).
 * These shall just pass that argument to their base class constructor.
 */
class FxSetup
{
public:
  /// The effect's corresponding Segment to work on (a.k.a. 'SEGMENT').
  Segment &seg;
};

/** Convenience interface for the effect's user configuration settings (from the UI).
 * Use this instead of accessing the Segement's members directly, e.g. via 'SEGMENT.speed'.
 */
class FxConfig
{
public:
  /** Constructor.
   * @param seg The effect's corresponding Segment to work on (a.k.a. 'SEGMENT').
   */
  explicit FxConfig(const Segment &seg) : _seg{seg} {}

  /// Current setting of the 'Speed" slider.
  uint8_t speed() const { return _seg.speed; }

  /// Current setting of the 'Intensity" slider.
  uint8_t intensity() const { return _seg.intensity; }

  /// Current setting of custom slider 1.
  uint8_t custom1() const { return _seg.custom1; }

  /// Current setting of custom slider 2.
  uint8_t custom2() const { return _seg.custom2; }

  /// Current setting of custom slider 3 (reduced range 0-31).
  uint8_t custom3() const { return _seg.custom3; }

  /// Current setting of checkbox 1 (with Palette icon).
  bool check1() const { return _seg.check1; }

  /// Current setting of checkbox 2 (with Overlay icon).
  bool check2() const { return _seg.check2; }

  /// Current setting of checkbox 3 (with Heart icon).
  bool check3() const { return _seg.check3; }

  /// Returns \c true when this effect is configured as overlay.
  bool isOverlay() const { return check2(); }

private:
  const Segment &_seg;
};

/** Runtime environment for rendering the effects.
 * The effect implementations shall obtain necessary runtime information for rendering their
 * animation from here, like current timestamp, settings for speed / intensity / ...
 */
class FxEnv
{
public:
  /// The current timestamp (a.k.a. 'strip.now').
  const uint32_t now;

  /// The Effect's current configuration settings (a.k.a. 'SEGMENT.speed' etc.) from the UI.
  const FxConfig &cfg;
};

/** Interface for WLED effects.
 * All class-based WLED effects must derive from this class. This requires them to override the
 * renderEffect() method, which is the replacement for a mode_XXX() function. All fancy pixel magic
 * that is rendered on the LED segment shall be done there.
 */
class WledEffect
{
public:
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
    Segment &seg = strip._segments[strip.getCurrSegmentId()];
    if (!seg.effect)
    {
      FxSetup fxs{seg};
      seg.effect = new (std::nothrow) EFFECT_TYPE(fxs);
    }
    return render_function(seg, strip.now, strip.getFrameTime());
  }

  /** Fallback rendering function.
   * Can be called as fallback by any effect's showEffect() method, when it cannot render its own
   * stuff; e.g. when something like allocating additional effect memory went wrong.
   */
  static uint16_t showFallbackEffect(Segment &seg, FxEnv &env);

  /** Render the WLED effect.
   * Currently used only internally; not relevant for custom effect implementations.
   * @param seg The Segment to work on (a.k.a. 'SEGMENT').
   * @param now The current timestamp (a.k.a. 'strip.now').
   * @return The effect's frametime (in ms), or 0 to use default frametime (a.k.a 'FRAMETIME').
   */
  uint16_t renderEffect(Segment &seg, uint32_t now);

protected:
  WledEffect(const WledEffect &) = default;

  /** Constructor.
   * @param fxs Internal setup data.
   */
  explicit WledEffect(FxSetup &fxs) : _fxs(fxs)
  {
  }

  /** Rendering function for the custom WLED effect.
   * Must be implemented by all child classes to show their specific animation on the Segment.
   * @param seg The Segment to work on (a.k.a. 'SEGMENT').
   * @param env Runtime environment for rendering the effect.
   * @return The Effect's frametime (in ms), or 0 to use default frametime (a.k.a 'FRAMETIME').
   */
  virtual uint16_t showEffect(Segment &seg, FxEnv &env) = 0;

private:
  static uint16_t render_function(Segment &seg, uint32_t now, uint16_t defaultFrametime)
  {
    const uint16_t frametime = seg.effect->renderEffect(seg, now);
    return frametime ? frametime : defaultFrametime;
  }

private:
  FxSetup _fxs;
};

/** Register an effect's mode function at the WLED framework.
 * @tparam EFFECT_TYPE Class type of concrete effect implementation. Must be a child of WledEffect.
 * @param strip WS2812FX instance, representing the WLED framework.
 * @return The actual id used for the effect, or 255 if the add failed.
 */
template <class EFFECT_TYPE>
uint8_t addWledEffect(WS2812FX &wled, uint8_t FX_id = EFFECT_TYPE::FX_id, const char *FX_data = EFFECT_TYPE::FX_data)
{
  return wled.addEffect(FX_id, &WledEffect::mode_function<EFFECT_TYPE>, FX_data);
}

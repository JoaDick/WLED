/**
 * Interfaces and utilities for creating class-based WLED effects.
 * @author Joachim Dick
 */

#pragma once

#include "FX.h"

class WledEffectConfig;

/** Interface for WLED effects.
 * All class-based WLED Effects must derive from this class. This requires them to override the 
 * renderEffect() method, which is the replacement for a mode_XXX() function. All fancy pixel magic
 * that is rendered on the LED segment shall be done there.
 */
class WledEffect {
 public:
  /// Destructor.
  virtual ~WledEffect() = default;

  /** Render the WLED Effect.
   * Must be implemented by all child classes to render their specific animation on the Segment.
   * @param seg The Segment to work on (a.k.a. 'SEGMENT').
   * @param now The current timestamp (a.k.a. 'strip.now').
   * @param cfg The Effect's current configuration settings (a.k.a. 'SEGMENT.speed' etc.) from the UI.
   * @return The Effect's frametime (in ms), or 0 to use default frametime (a.k.a 'FRAMETIME').
   */
  virtual uint16_t renderEffect(Segment& seg, uint32_t now, const WledEffectConfig& cfg) = 0;

  /** Effect's mode function (to be registered at the WLED framework).
   * @tparam EFFECT_TYPE Class type of concrete Effect implementation. Must be a child of WledEffect.
   * @see addWledEffect()
   */
  template<class EFFECT_TYPE>
  static uint16_t mode_function()
  {
    extern WS2812FX strip;
    Segment& seg = strip._segments[strip.getCurrSegmentId()];
    if (!seg.effect) {
      seg.effect = new(std::nothrow) EFFECT_TYPE(seg);
    }
    return render_function(seg, strip.now, strip.getFrameTime());
  }

 protected:
  /** Constructor. 
   * @param seg The Effect's corresponding Segment to work on (a.k.a. 'SEGMENT').
   */
  explicit WledEffect(Segment& seg);

 private:
  static uint16_t render_function(Segment& seg, uint32_t now, uint16_t defaultFrametime);
};


/** Register an Effect's mode function at the WLED framework.
 * @tparam EFFECT_TYPE Class type of concrete Effect implementation. Must be a child of WledEffect.
 * @param strip WS2812FX instance, representing the WLED framework.
 * @return The actual id used for the effect, or 255 if the add failed.
 */
template<class EFFECT_TYPE>
uint8_t addWledEffect(WS2812FX& strip, uint8_t FX_id = EFFECT_TYPE::FX_id, const char* FX_data = EFFECT_TYPE::FX_data) {
  return strip.addEffect(FX_id, &WledEffect::mode_function<EFFECT_TYPE>, FX_data);
}


/** Convenience interface for the Effect's user configuration settings (from the UI).
 * Use this instead of accessing the Segement's members directly, e.g. via 'SEGMENT.speed'.
 */
class WledEffectConfig {
 public:
  /** Constructor. 
   * @param seg The Effect's corresponding Segment to work on (a.k.a. 'SEGMENT').
   */
  explicit WledEffectConfig(const Segment& seg) : _seg{seg} {}

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

  /// Returns \c true when this Effect is configured as overlay.
  bool isOverlay() const { return check2(); }

 private:
  const Segment& _seg;
};

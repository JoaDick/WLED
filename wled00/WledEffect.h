/**
 * Interfaces and utilities for creating class-based WLED effects.
 * @author Joachim Dick
 */

#pragma once

#include "FX.h"


/// Interface for WLED effects.
class WledEffect {
 public:
  /// Destructor.
  virtual ~WledEffect() = default;

  /** Render the WLED Effect.
   * Must be implemented by all child classes to render their specific animation on the Segment.
   * @param seg The Segment to work on (a.k.a. 'SEGMENT').
   * @param now The current timestamp (a.k.a. 'strip.now').
   * @return The Effect's frametime (in ms), or 0 to use default frametime (a.k.a 'FRAMETIME').
   */
  virtual uint16_t renderEffect(Segment& seg, uint32_t now) = 0;

  /** Effect's mode function (to be registered at the WLED framework).
   * @tparam EFFECT_TYPE Class type of concrete Effect implementation. Must be a child of WledEffect.
   * @see addWledEffect()
   */
  template<class EFFECT_TYPE>
  static uint16_t mode_function()
  {
    extern WS2812FX strip;
    Segment& seg = SEGMENT;
    if(!seg.effect) {
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
 * @tparam strip WS2812FX instance, representing the WLED framework.
 * @return The actual id used for the effect, or 255 if the add failed.
 */
template<class EFFECT_TYPE>
uint8_t addWledEffect(WS2812FX& strip, uint8_t FX_id = EFFECT_TYPE::FX_id, const char* FX_data = EFFECT_TYPE::FX_data) {
  return strip.addEffect(FX_id, &WledEffect::mode_function<EFFECT_TYPE>, FX_data);
}

/**
 * Interfaces and utilities for creating class-based WLED effects.
 * @author Joachim Dick
 */

#pragma once

#include <memory>
#include "FX.h"
#include "FastLED.h"

void addExperimentalEffects(WS2812FX &wled);

//--------------------------------------------------------------------------------------------------

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

  /// Current setting of the 'Speed" slider (with Clock icon) -- 'SEGMENT.speed'
  uint8_t speed() const { return _seg.speed; }

  /// Current setting of the 'Intensity" slider (with Fire icon) -- 'SEGMENT.intensity'
  uint8_t intensity() const { return _seg.intensity; }

  /// Current setting of custom slider 1 (with Star icon) -- 'SEGMENT.custom1'
  uint8_t custom1() const { return _seg.custom1; }

  /// Current setting of custom slider 2 (with Gear icon) -- 'SEGMENT.custom2'
  uint8_t custom2() const { return _seg.custom2; }

  /// Current setting of custom slider 3 (with Eye icon; reduced range 0-31) -- 'SEGMENT.custom3'
  uint8_t custom3() const { return _seg.custom3; }

  /// Current setting of checkbox 1 (with Palette icon) -- 'SEGMENT.check1'
  bool check1() const { return _seg.check1; }

  /// Current setting of checkbox 2 (with Overlay icon) -- 'SEGMENT.check2'
  bool check2() const { return _seg.check2; }

  /// Current setting of checkbox 3 (with Heart icon) -- 'SEGMENT.check3'
  bool check3() const { return _seg.check3; }

  /// Get current effect/foreground color -- 'SEGCOLOR(0)'
  uint32_t fxColor() const { return color(0); }

  /// Get current background color -- 'SEGCOLOR(1)'
  uint32_t bgColor() const { return color(1); }

  /// Get current extra color -- 'SEGCOLOR(2)'
  uint32_t auxColor() const { return color(2); }

  /// Get the desired color \a x -- 'SEGCOLOR(x)'
  uint32_t color(unsigned x) const { return _seg.getCurrentColor(x); }

  /// Get currently selected color palette -- 'SEGPALETTE'
  const CRGBPalette16 &palette() { return _seg.getCurrentPalette(); }

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

//--------------------------------------------------------------------------------------------------

/** Interface for WLED effects.
 * All class-based WLED effects must derive from this class. This requires them to override the
 * showEffect() method, which is the replacement for a mode_XXX() function. All fancy pixel magic
 * that is rendered on the LED segment shall be done there.
 */
class WledFx
{
public:
  WledFx(WledFx &&) = delete;
  WledFx &operator=(const WledFx &) = delete;
  WledFx &operator=(WledFx &&) = delete;

  /// Destructor.
  virtual ~WledFx() = default;

  /** Effect's mode function (to be registered at the WLED framework).
   * @tparam EFFECT_TYPE Class type of concrete effect implementation. Must be a child of WledFx.
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
  uint16_t render(Segment &seg, uint32_t now);

protected:
  WledFx(const WledFx &) = default;

  /** Constructor.
   * @param fxs Internal setup data.
   */
  explicit WledFx(FxSetup &fxs) : _fxs(fxs) {}

  /** Rendering function for the custom WLED effect.
   * Must be implemented by all child classes to show their specific animation on the Segment.
   * @param seg The Segment to work on (a.k.a. 'SEGMENT').
   * @param env Runtime environment for rendering the effect.
   * @return The Effect's frametime (in ms), or 0 to use default frametime (a.k.a 'FRAMETIME').
   */
  virtual uint16_t showEffect(Segment &seg, FxEnv &env) = 0;

  // // Only for development.
  // Segment &getSegment() { return _fxs.seg; }

private:
  static uint16_t render_function(Segment &seg, uint32_t now, uint16_t defaultFrametime)
  {
    const uint16_t frametime = seg.effect->render(seg, now);
    return frametime ? frametime : defaultFrametime;
  }

private:
  FxSetup _fxs;
};

/** Register an effect's mode function at the WLED framework.
 * For format of \a FX_data see https://kno.wled.ge/interfaces/json-api/#effect-metadata
 * @tparam EFFECT_TYPE Class type of concrete effect implementation. Must be a child of WledFx.
 * @param strip WS2812FX instance, representing the WLED framework.
 * @return The actual id used for the effect, or 255 if the add failed.
 */
template <class EFFECT_TYPE>
uint8_t addWledEffect(WS2812FX &wled, uint8_t FX_id = EFFECT_TYPE::FX_id, const char *FX_data = EFFECT_TYPE::FX_data)
{
  return wled.addEffect(FX_id, &WledFx::mode_function<EFFECT_TYPE>, FX_data);
}

//--------------------------------------------------------------------------------------------------

/** Internal helper class for emulating a FastLED CRGB pixel, which is tied to a WLED pixel.
 * Upon construction, it loads the current color from the corresponding WLED Segment's pixel.
 * During lifetime, it "feels and behaves" like a FastLED CRGB pixel; meaning it can be manipulated
 * just as known from FastLED animations.
 * Upon destruction, it writes back its new color to the WLED Segment's pixel.
 */
struct ProxyCRGB : public CRGB
{
  ProxyCRGB(const ProxyCRGB &) = delete;
  ProxyCRGB(ProxyCRGB &&other) : CRGB(other), _seg(other._seg), _index(other._index), _muted(other._muted)
  {
    other._muted = true;
  }
  ProxyCRGB &operator=(const ProxyCRGB &) = delete;
  ProxyCRGB &operator=(ProxyCRGB &&) = delete;

  ProxyCRGB(Segment &seg, int index) : CRGB(seg.getPixelColor(index)), _seg(seg), _index(index) {}

  ~ProxyCRGB()
  {
    if (!_muted)
    {
      _seg.setPixelColor(_index, *this);
    }
  }

  ProxyCRGB &operator=(const CRGB &color)
  {
    static_cast<CRGB &>(*this).operator=(color);
    return *this;
  }

private:
  Segment &_seg;
  const int _index;
  bool _muted = false;
};

/** Internal helper class for emulating a FastLED LED array.
 * Use the index-operator to access an individual LED.
 */
class EmulatedFastLedArray
{
public:
  explicit EmulatedFastLedArray(Segment &seg) : _seg(seg) {}
  ProxyCRGB operator[](int index) { return ProxyCRGB(_seg, index); }
  uint16_t size() const { return _seg.vLength(); }
  Segment &getSegment() { return _seg; }

private:
  Segment &_seg;
};

inline void fill_solid(EmulatedFastLedArray &leds, const CRGB &color) { leds.getSegment().fill_solid(color); }

inline void fadeToBlackBy(EmulatedFastLedArray &leds, uint8_t fadeBy) { leds.getSegment().fadeToBlackBy(fadeBy); }

/** Base class for simple FastLED based effects (which emulates the LED array without buffering).
 * ... usage ...
 * ... limitations ...
 */
class EmulatedFastLedFxBase : public WledFx
{
protected:
  EmulatedFastLedFxBase(const EmulatedFastLedFxBase &) = default;

  /** Constructor.
   * @param fxs Internal setup data.
   */
  explicit EmulatedFastLedFxBase(FxSetup &fxs) : WledFx(fxs) {}

  /** Rendering function for the custom FastLED effect.
   * Must be implemented by all child classes to show their specific animation.
   * Use parameter \a leds for rendering the animation. It feels and behaves mostly like the well known
   * \code
   * CRGB leds[numleds];
   * \endcode
   *
   * @param leds Emulated LED array (with limited functionality).
   * @param num_leds Number of LEDs in the array.
   * @param env Runtime environment for rendering the effect.
   * @return The effect's frametime (in ms), or 0 to use default frametime.
   */
  virtual uint16_t showFastLed(EmulatedFastLedArray &leds, uint16_t num_leds, FxEnv &env) = 0;

  /// @see WledFx::showEffect()
  uint16_t showEffect(Segment &seg, FxEnv &env) override
  {
    EmulatedFastLedArray leds(seg);
    return showFastLed(leds, seg.vLength(), env);
  }
};

//--------------------------------------------------------------------------------------------------

/** Base class for fully featured FastLED based effects (with an internally buffered FastLED array).
 * ... usage ...
 * ... limitations ...
 */
class BufferedFastLedFxBase : public WledFx
{
protected:
  BufferedFastLedFxBase(const BufferedFastLedFxBase &) = delete; // should clone _leds

  /** Constructor.
   * @param fxs Internal setup data.
   */
  explicit BufferedFastLedFxBase(FxSetup &fxs)
      : WledFx(fxs), _numLeds{static_cast<uint16_t>(fxs.seg.vLength())}, _leds{new CRGB[_numLeds]}
  {
    fill_solid(_leds.get(), _numLeds, CRGB::Black);
  }

  /** Rendering function for the custom FastLED effect.
   * Must be implemented by all child classes to show their specific animation.
   * @param leds Real FastLED array (with full functionality).
   * @param num_leds Number of LEDs in the array.
   * @param env Runtime environment for rendering the effect.
   * @return The effect's frametime (in ms), or 0 to use default frametime.
   */
  virtual uint16_t showFastLed(CRGB leds[], uint16_t num_leds, FxEnv &env) = 0;

  /// @see WledFx::showEffect()
  uint16_t showEffect(Segment &seg, FxEnv &env) override;

private:
  const uint16_t _numLeds;
  std::unique_ptr<CRGB[]> _leds;
};

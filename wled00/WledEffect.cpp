/**
 * Interfaces and utilities for creating class-based WLED effects.
 * @author Joachim Dick
 */

#include "wled.h"
#include "WledEffect.h"

//--------------------------------------------------------------------------------------------------

void checkIfThisCompiles(Segment &seg)
{
  EmulatedFastLedArray leds{seg};

  leds[1] = CRGB(12, 34, 56);
}

//--------------------------------------------------------------------------------------------------

uint16_t WledEffect::renderEffect(Segment &seg, uint32_t now)
{
  FxConfig configuration(seg);
  FxEnv runtimeEnvironment{now, configuration};

  // just a sanity check (out of pure curiosity)
  if (&seg != &_fxs.seg)
  {
    // somehow purple
    seg.fill(0x440022);
    return 0;
  }

  return showEffect(seg, runtimeEnvironment);
}

uint16_t WledEffect::showFallbackEffect(Segment &seg, FxEnv &env)
{
  // somehow red
  seg.fill(0x440000);
  return 0;
}

//--------------------------------------------------------------------------------------------------

class EmulatedFastLedExample : public EmulatedFastLedEffectBase
{
public:
  static constexpr EffectID FX_id = AutoSelectID;
  static constexpr char FX_data[] PROGMEM = "FastLED Emulation FX@Speed,Tail,,,,,Overlay;;;;sx=60,ix=160";

  /// Constructor.
  explicit EmulatedFastLedExample(FxSetup &fxs) : EmulatedFastLedEffectBase(fxs) {}

  /// @see EmulatedFastLedEffectBase::showFastLed()
  uint16_t showFastLed(EmulatedFastLedArray &leds, uint16_t numleds, FxEnv &env) override
  {
    if (!env.cfg.check2())
    {
      fadeToBlackBy(leds, 128 - (env.cfg.intensity() / 2));
    }

    const int32_t newPos = beatsin16(1 + env.cfg.speed() / 4, 0, numleds - 1);
    const CRGB color = env.cfg.fxColor();
    leds[newPos] = color;

    if (_lastPos >= 0)
    {
      while (_lastPos < newPos)
      {
        leds[_lastPos] = color;
        ++_lastPos;
      }
      while (_lastPos > newPos)
      {
        leds[_lastPos] = color;
        --_lastPos;
      }
    }
    else
    {
      _lastPos = newPos;
    }

    return 0;
  }

private:
  int32_t _lastPos = -1;
};

//--------------------------------------------------------------------------------------------------

void addExperimentalEffects(WS2812FX &wled)
{
  addWledEffect<EmulatedFastLedExample>(wled);
}

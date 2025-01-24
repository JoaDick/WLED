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

uint16_t WledFx::render(Segment &seg, uint32_t now)
{
  FxConfig configuration(seg);
  FxEnv runtimeEnvironment{now, configuration};

  // just a sanity check (out of pure curiosity) if the Segment remains the same all the time
  if (&seg != &_fxs.seg)
  {
    for (unsigned i = 0; i < seg.vLength(); ++i)
    {
      unsigned mask = (1 << 3);
      uint32_t color = 0;
      if (i & mask)
      {
        color |= 0xFF0000;
      }
      mask <<= 1;
      if (i & mask)
      {
        color |= 0x00FF00;
      }
      mask <<= 1;
      if (i & mask)
      {
        color |= 0x0000FF;
      }
      seg.setPixelColor(i, color);
    }
    return 0;
  }

  return showEffect(seg, runtimeEnvironment);
}

uint16_t WledFx::showFallbackEffect(Segment &seg, FxEnv &env)
{
  seg.fill(env.cfg.fxColor());
  for (unsigned i = 0; i < seg.vLength(); ++i)
  {
    if ((i & 0xC) == 0xC)
    {
      seg.setPixelColor(i, 0xFF0000);
    }
  }
  return 0;
}

//--------------------------------------------------------------------------------------------------

uint16_t BufferedFastLedFxBase::showEffect(Segment &seg, FxEnv &env)
{
  const uint16_t retval = showFastLed(_leds.get(), _numLeds, env);
  CRGB *begin = _leds.get();
  CRGB *end = begin + _numLeds;
  int index = 0;
  while (begin < end)
  {
    seg.setPixelColor(index++, *(begin++));
  }
  return retval;
}

//--------------------------------------------------------------------------------------------------

/// Simple example that uses the emulated FastLED feature.
class FxExample_EmulatedFastLed : public EmulatedFastLedFxBase
{
public:
  static constexpr EffectID FX_id = AutoSelectID;
  static constexpr char FX_data[] PROGMEM = "FastLED Emulation FX@Speed,Tail,,,,,Overlay;;;;sx=60,ix=160";

  /// Constructor.
  explicit FxExample_EmulatedFastLed(FxSetup &fxs) : EmulatedFastLedFxBase(fxs) {}

  /// @see EmulatedFastLedFxBase::showFastLed()
  uint16_t showFastLed(EmulatedFastLedArray &leds, uint16_t num_leds, FxEnv &env) override
  {
    if (!env.cfg.check2())
    {
      fadeToBlackBy(leds, 128 - (env.cfg.intensity() / 2));
    }

    const int newPos = beatsin16(1 + env.cfg.speed() / 4, 0, num_leds - 1);
    const CRGB color = env.cfg.fxColor();
    leds[newPos] = color;

    if (_lastPos >= 0)
    {
      while (_lastPos < newPos)
      {
        leds[_lastPos++] = color;
      }
      while (_lastPos > newPos)
      {
        leds[_lastPos--] = color;
      }
    }
    else
    {
      _lastPos = newPos;
    }

    return 0;
  }

private:
  int _lastPos = -1;
};

//--------------------------------------------------------------------------------------------------

/// Same effect as FxExample_EmulatedFastLed, but with a real FastLED array - thus without overlay.
class FxExample_BufferedFastLed : public BufferedFastLedFxBase
{
public:
  static constexpr EffectID FX_id = AutoSelectID;
  static constexpr char FX_data[] PROGMEM = "FastLED Buffering FX@Speed,Tail;;;;sx=60,ix=160";

  /// Constructor.
  explicit FxExample_BufferedFastLed(FxSetup &fxs) : BufferedFastLedFxBase(fxs) {}

  /// @see BufferedFastLedFxBase::showFastLed()
  uint16_t showFastLed(CRGB leds[], uint16_t num_leds, FxEnv &env) override
  {
    fadeToBlackBy(leds, num_leds, 128 - (env.cfg.intensity() / 2));

    const int newPos = beatsin16(1 + env.cfg.speed() / 4, 0, num_leds - 1);
    const CRGB color = env.cfg.fxColor();
    leds[newPos] = color;

    if (_lastPos >= 0)
    {
      while (_lastPos < newPos)
      {
        leds[_lastPos++] = color;
      }
      while (_lastPos > newPos)
      {
        leds[_lastPos--] = color;
      }
    }
    else
    {
      _lastPos = newPos;
    }

    return 0;
  }

private:
  int _lastPos = -1;
};

//--------------------------------------------------------------------------------------------------

void addExperimentalEffects(WS2812FX &wled)
{
  addWledEffect<FxExample_EmulatedFastLed>(wled);
  addWledEffect<FxExample_BufferedFastLed>(wled);
}

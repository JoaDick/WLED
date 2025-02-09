/**
 * Blablabla.
 */

#pragma once

#include "FXutils.h"

// #define USERMOD_ID_EFFECT_RUNNER         57     //Usermod "usermod_EffectRunner.h"

//--------------------------------------------------------------------------------------------------

/// TBD ...
class UmEffectRunner : public Usermod
{
public:
  // uint16_t getId() override { return USERMOD_ID_EFFECT_RUNNER; /* USERMOD_ID_UNSPECIFIED */ }

  void setup() override { addEffects(strip); }

  void loop() override {}

private:
  void addEffects(WS2812FX &wled);
};

//--------------------------------------------------------------------------------------------------

/// Example for the PxArray and FxConfig helper classes.
uint16_t mode_PxArray_example()
{
  static int index = 0;

  PxArray pixels(SEGMENT);
  FxConfig config(pixels.getSegment());

  pixels->fadeToBlackBy(128 - (config.intensity() / 2));
  pixels.setColor(index, config.fxColor());

  if (++index >= pixels.size())
  {
    index = 0;
  }

  return FRAMETIME;
}
static const char _data_FX_mode_FOO[] PROGMEM = "A PxArray@,Tail";

// just to check if it compiles
void audioReactiveFoo(Segment &seg)
{
  AudioReactiveUmData data(seg);

  data.samplePeak();
}

//--------------------------------------------------------------------------------------------------

/// TBD ...
class FallbackEffect : public EffectRunner
{
public:
  static const EffectID FX_id = AutoSelectID;
  static const char FX_data[];

  explicit FallbackEffect(FxSetup &fxs) : EffectRunner(fxs) {}

private:
  uint16_t showEffect(uint32_t now) override { return showFallbackEffect(now); }
};
const char FallbackEffect::FX_data[] PROGMEM = "A FallbackEffect@!";

//--------------------------------------------------------------------------------------------------

/// TBD ...
struct SoundmeterEffect : public EffectRunner
{
  static const EffectID FX_id = AutoSelectID;
  static const char FX_data[];
  explicit SoundmeterEffect(FxSetup &fxs) : EffectRunner(fxs) {}

private:
  /// Data to be preserved between calls to showEffect()
  struct FxData
  {
    float lastPosSmth = 0.5;
    float peakPosSmth = 0.0;

    float lastPosRaw = 0.5;
    float peakPosRaw = 0.0;
  };

  uint16_t showEffect(uint32_t now) override
  {
    FxData *fxData;
    return segenv.getFxData(fxData) ? showEffect(now, *fxData) : showFallbackEffect(now);
  }

  uint16_t showEffect(uint32_t now, FxData &fxData)
  {
    PxArray pixels(seg);
    AudioReactiveUmData audioData(seg);

    pixels->fadeToBlackBy(1 + config.intensity() / 2);
    // pixels.setColor(segenv.call() % seglen, config.auxColor());

    const float posSmth = audioData.n_volumeSmth();
    const float posRaw = audioData.volumeRaw() / 255.0;
    if (posSmth > fxData.peakPosSmth)
    {
      fxData.peakPosSmth = posSmth;
    }
    if (posRaw > fxData.peakPosRaw)
    {
      fxData.peakPosRaw = posRaw;
    }

    pixels.n_lineAbs(1.0 - fxData.lastPosSmth, 1.0 - posSmth, config.fxColor());
    if (fxData.peakPosSmth > 0.0)
    {
      pixels.n_setColor(1.0 - fxData.peakPosSmth, config.auxColor());
      fxData.peakPosSmth -= 0.002;
    }

    pixels.n_lineAbs(fxData.lastPosRaw, posRaw, config.bgColor());
    if (fxData.peakPosRaw > 0.0)
    {
      pixels.n_setColor(fxData.peakPosRaw, config.auxColor());
      fxData.peakPosRaw -= 0.002;
    }

    fxData.lastPosSmth = posSmth;
    fxData.lastPosRaw = posRaw;

    return 0;
  }
};
const char SoundmeterEffect::FX_data[] PROGMEM = "A Soundmeter@,Fading;Smth,Raw,Peak;;1v";

//--------------------------------------------------------------------------------------------------

/// more effect class examples ...

//--------------------------------------------------------------------------------------------------

void UmEffectRunner::addEffects(WS2812FX &wled)
{
  wled.addEffect(AutoSelectID, &mode_PxArray_example, _data_FX_mode_FOO);
  addEffectRunner<FallbackEffect>(wled);
  addEffectRunner<SoundmeterEffect>(wled);
}

//--------------------------------------------------------------------------------------------------

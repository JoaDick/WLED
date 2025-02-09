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
const char FallbackEffect::FX_data[] PROGMEM = "A FallbackEffect@";

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

/// Inline Equalizer.
struct InlineEqEffect : public EffectRunner
{
  static const EffectID FX_id = AutoSelectID;
  static const char FX_data[];
  explicit InlineEqEffect(FxSetup &fxs) : EffectRunner(fxs) {}

private:
  uint16_t showEffect(uint32_t now) override
  {
    const uint8_t numBlobs = AudioReactiveUmData::fftResult_size() / 3;
    std::array<float, numBlobs> blobs;

    AudioReactiveUmData audioData(seg);
    // take the average of 3 FFT bins as one blob's size (ignoring the last bin)
    for (uint8_t i = 0; i < audioData.fftResult_size(); i += 3)
    {
      float fftAvg = audioData.fftResult(i);
      fftAvg += audioData.fftResult(i + 1);
      fftAvg += audioData.fftResult(i + 2);
      fftAvg /= 3;
      blobs[i / 3] = fftAvg / 255.0;
    }

    PxArray pixels(seg);
    pixels->fadeToBlackBy(config.intensity());
    for (uint8_t i = 0; i < numBlobs; ++i)
    {
      const float blobSize = blobs[i] / numBlobs;
      const float maxBlobSize = 1.0 / numBlobs;
      const float centerPos = maxBlobSize / 2.0 + i * maxBlobSize;
      // TODO: somehow use a color palette 
      pixels.n_lineCentered(centerPos, blobSize, config.fxColor());
    }

    return 0;
  }
};
const char InlineEqEffect::FX_data[] PROGMEM = "A Inline EQ@,Fading;!;;1f;ix=96";

/// Inline Equalizer 2. Doesn't turn out good - too twitchy. :-(
struct InlineEq2Effect : public EffectRunner
{
  static const EffectID FX_id = AutoSelectID;
  static const char FX_data[];
  explicit InlineEq2Effect(FxSetup &fxs) : EffectRunner(fxs) {}

private:
  uint16_t showEffect(uint32_t now) override
  {
    const uint8_t arrayLen = 1 + 2 * (AudioReactiveUmData::fftResult_size() / 4);
    std::array<float, arrayLen> blobSizes;
    for (auto &blobSize : blobSizes)
    {
      blobSize = 0.33;
    }

    AudioReactiveUmData audioData(seg);
    for (uint8_t i = 0; i < audioData.fftResult_size(); i += 4)
    {
      float fftAvg = audioData.fftResult(i);
      fftAvg += audioData.fftResult(i + 1);
      fftAvg += audioData.fftResult(i + 2);
      fftAvg += audioData.fftResult(i + 3);
      fftAvg /= 4;
      blobSizes[1 + 2 * (i / 4)] = fftAvg / 255.0;
    }

    std::array<float, arrayLen> blobOffsets;
    float totalSize = 0.0;
    for (uint8_t i = 0; i < arrayLen; ++i)
    {
      blobOffsets[i] = totalSize;
      totalSize += blobSizes[i];
    }
    const float scaleFactor = 1.0 / totalSize;

    PxArray pixels(seg);
    pixels.fill(0);
    for (uint8_t i = 1; i < arrayLen; i += 2)
    {
      pixels.n_lineRel(blobOffsets[i] * scaleFactor, blobSizes[i] * scaleFactor, config.fxColor());
    }

    return 0;
  }
};
const char InlineEq2Effect::FX_data[] PROGMEM = "A Inline EQ 2@;!;;1f";

//--------------------------------------------------------------------------------------------------

/// more effect class examples ...

//--------------------------------------------------------------------------------------------------

void UmEffectRunner::addEffects(WS2812FX &wled)
{
  wled.addEffect(AutoSelectID, &mode_PxArray_example, _data_FX_mode_FOO);
  addEffectRunner<FallbackEffect>(wled);
  addEffectRunner<InlineEqEffect>(wled);
  addEffectRunner<InlineEq2Effect>(wled);
  addEffectRunner<SoundmeterEffect>(wled);
}

//--------------------------------------------------------------------------------------------------

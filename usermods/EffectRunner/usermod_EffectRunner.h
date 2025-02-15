/**
 * Blablabla.
 */

#pragma once

#include "FXutils2D.h"

// #define USERMOD_ID_EFFECT_RUNNER         57     //Usermod "usermod_EffectRunner.h"

//--------------------------------------------------------------------------------------------------

/// TBD ...
class UmEffectRunner : public Usermod
{
public:
  // uint16_t getId() override { return USERMOD_ID_EFFECT_RUNNER; /* USERMOD_ID_UNSPECIFIED */ }

  void setup() override { addEffects(strip); }

  void loop() override {}

  // void addToJsonInfo(JsonObject &root) override
  // {
  //   if (0)
  //   {
  //     JsonObject user = root["u"];
  //     if (user.isNull())
  //     {
  //       user = root.createNestedObject("u");
  //     }

  //     FxConfig config(SEGMENT);
  //     // ... dump config into info ...
  //     // palette...?

  //     JsonArray infoArr = user.createNestedArray(F("???"));
  //     infoArr.add(F("receiving v"));
  //     infoArr.add(F("1"));
  //     // infoArr.add(F("<i>(unconnected)</i>"));
  //   }
  // }

private:
  void addEffects(WS2812FX &wled);
};

//--------------------------------------------------------------------------------------------------

/// Example for the PxStrip and FxConfig helper classes.
uint16_t mode_PxStrip_example()
{
  static int index = 0;

  PxStrip leds(SEGMENT);
  FxConfig config(leds.getSegment());

  leds.fadeToBlackBy(128 - (config.intensity() / 2));
#if (1)
  // FastLED style
  leds[index] = config.fxColor();
#else
  // WLED style
  leds.setPixelColor(index, config.fxColor());
#endif

  if (++index >= leds.size())
  {
    index = 0;
  }

  return FRAMETIME;
}
static const char _data_FX_mode_PxStrip[] PROGMEM = "A PxStrip@,Tail";

//--------------------------------------------------------------------------------------------------

/// Example that shows the Fallback-Effect (which usually indicates a problem).
class FxFallback : public EffectRunner
{
public:
  static const EffectID FX_id = AutoSelectID;
  static const char FX_data[];

  explicit FxFallback(FxSetup &fxs) : EffectRunner(fxs) {}

private:
  uint16_t showEffect(uint32_t now) override { return showFallbackEffect(now); }
};
const char FxFallback::FX_data[] PROGMEM = "A Fallback@";

//--------------------------------------------------------------------------------------------------

/// Yet another Larson Scanner.
class FxLarson : public EffectRunner
{
  PxStrip pixels;

public:
  static const EffectID FX_id = AutoSelectID;
  static const char FX_data[];

  explicit FxLarson(FxSetup &fxs) : EffectRunner(fxs), pixels(fxs), _lastPos(segenv.aux0()) {}

private:
  uint16_t getPosition() { return beatsin16(1 + config.speed() / 4, 0, seglen - 1); }

  void initEffect(uint32_t now) override { _lastPos = getPosition(); }

  uint16_t showEffect(uint32_t now) override
  {
    if (!config.check2())
    {
      pixels.fadeToBlackBy(128 - (config.intensity() / 2));
    }

    const uint16_t newPos = getPosition();
    pixels.lineAbs(_lastPos, newPos, config.fxColor());
    _lastPos = newPos;

    return 0;
  }

private:
  uint16_t &_lastPos; // this is a reference (!) which is bound to segment's custom var 'aux0'
};
const char FxLarson::FX_data[] PROGMEM = "A Larson@Speed,Tail,,,,,Overlay;;;;sx=60,ix=160";

//--------------------------------------------------------------------------------------------------

/// Simple VU meter that shows smoothed and raw audio level, including a peak dot.
class FxSoundmeter : public EffectRunner
{
  /// Data to be preserved between calls to showEffect()
  struct FxData
  {
    float lastPosSmth = 0.5;
    float peakPosSmth = 0.0;

    float lastPosRaw = 0.5;
    float peakPosRaw = 0.0;
  };

  PxStrip pixels;
  AudioReactiveUmData audioData;

public:
  static const EffectID FX_id = AutoSelectID;
  static const char FX_data[];

  explicit FxSoundmeter(FxSetup &fxs) : EffectRunner(fxs), pixels(fxs), audioData(fxs) {}

private:
  uint16_t showEffect(uint32_t now) override
  {
    FxData *fxData;
    return segenv.getFxData(fxData) ? showEffect(now, *fxData) : showFallbackEffect(now);
  }

  uint16_t showEffect(uint32_t now, FxData &fxData)
  {
    pixels.fadeToBlackBy(1 + config.intensity() / 2);

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
      pixels.n_setPixelColor(1.0 - fxData.peakPosSmth, config.auxColor());
      fxData.peakPosSmth -= 0.002;
    }

    pixels.n_lineAbs(fxData.lastPosRaw, posRaw, config.bgColor());
    if (fxData.peakPosRaw > 0.0)
    {
      pixels.n_setPixelColor(fxData.peakPosRaw, config.auxColor());
      fxData.peakPosRaw -= 0.002;
    }

    fxData.lastPosSmth = posSmth;
    fxData.lastPosRaw = posRaw;

    return 0;
  }
};
const char FxSoundmeter::FX_data[] PROGMEM = "A Soundmeter@,Fading;Smth,Raw,Peak;;1v";

//--------------------------------------------------------------------------------------------------

/// Inline 5-band graphic equalizer.
class FxInlineEq : public EffectRunner
{
  PxStrip pixels;
  AudioReactiveUmData audioData;

public:
  static const EffectID FX_id = AutoSelectID;
  static const char FX_data[];

  explicit FxInlineEq(FxSetup &fxs) : EffectRunner(fxs), pixels(fxs), audioData(fxs) {}

private:
  uint16_t showEffect(uint32_t now) override
  {
    const uint8_t numBlobs = AudioReactiveUmData::fftResult_size() / 3;
    std::array<float, numBlobs> blobs;

    // take the average of 3 FFT bins as one blob's size (ignoring the last bin)
    for (uint8_t i = 0; i < audioData.fftResult_size(); i += 3)
    {
      float fftAvg = audioData.fftResult(i);
      fftAvg += audioData.fftResult(i + 1);
      fftAvg += audioData.fftResult(i + 2);
      fftAvg /= 3;
      blobs[i / 3] = fftAvg / 255.0;
    }

    pixels.fadeToBlackBy(config.intensity());
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
const char FxInlineEq::FX_data[] PROGMEM = "A Inline EQ@,Fading;!;;1f;ix=96";

/// Inline Equalizer 2. Doesn't turn out good - too twitchy. :-(
class FxInlineEq2 : public EffectRunner
{
  PxStrip pixels;
  AudioReactiveUmData audioData;

public:
  static const EffectID FX_id = AutoSelectID;
  static const char FX_data[];

  explicit FxInlineEq2(FxSetup &fxs) : EffectRunner(fxs), pixels(fxs), audioData(fxs) {}

private:
  uint16_t showEffect(uint32_t now) override
  {
    const uint8_t arrayLen = 1 + 2 * (AudioReactiveUmData::fftResult_size() / 4);
    std::array<float, arrayLen> blobSizes;
    for (auto &blobSize : blobSizes)
    {
      blobSize = 0.33;
    }

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

    pixels.fill(0);
    for (uint8_t i = 1; i < arrayLen; i += 2)
    {
      pixels.n_lineRel(blobOffsets[i] * scaleFactor, blobSizes[i] * scaleFactor, config.fxColor());
    }

    return 0;
  }
};
const char FxInlineEq2::FX_data[] PROGMEM = "A Inline EQ 2@;!;;1f";

//--------------------------------------------------------------------------------------------------

/** Softly floating colorful clouds.
 * Ported to WLED from https://github.com/JoaDick/EyeCandy/blob/master/ColorClouds.h
 */
class ColorCloudsBase : public EffectRunner
{
protected:
  PxStrip pixels;

  explicit ColorCloudsBase(FxSetup &fxs) : EffectRunner(fxs), pixels(fxs) { setNormal(); }

  uint16_t showClouds(uint32_t currentMillis)
  {
    const auto ledCount = seglen;
    const uint8_t hueOffset = beat88(64) >> 8;

    for (uint32_t i = 0; i < ledCount; i++)
    {
      const uint32_t hueX = i * hueSqueeze * 16;
      const uint32_t hueT = currentMillis * (1 + hueSpeed) / 4;
      uint8_t hue = inoise16(hueX, hueT) >> 7;
      hue += hueOffset;
      if (moreRed)
      {
        hue = PxColor::redShift(hue);
      }

      const uint32_t volX = i * volSqueeze * 64;
      const uint32_t volT = currentMillis * (1 + volSpeed) / 8;
      long vol = inoise16(volX, volT);
      vol = map(vol, 25000, 47500, 0, 255);
      vol = constrain(vol, 0, 255);

      CRGB pixel = CHSV(hue, 255, vol);
      if (int(pixel.r) + pixel.g + pixel.b <= 1)
      {
        pixel = CRGB::Black;
      }

      seg.setPixelColor(i, pixel);
    }

    return 0;
  }

  /// Higher values make the color change faster.
  uint8_t hueSpeed;

  /// Higher values "squeeze" more color gradients on the LED strip.
  uint8_t hueSqueeze;

  /// Higher values make the clouds change faster.
  uint8_t volSpeed;

  /// Higher values make more clouds (but smaller ones).
  uint8_t volSqueeze;

  /// Put more emphasis on the red'ish colors when true.
  bool moreRed = false;

  void setNormal()
  {
    hueSpeed = 64;
    hueSqueeze = 64;
    volSpeed = 64;
    volSqueeze = 64;
  }

  void setAmbient()
  {
    hueSpeed = 3;
    hueSqueeze = 25;
    volSpeed = 25;
    volSqueeze = 45;
  }

  void setExtraSlow()
  {
    hueSpeed = 1;
    hueSqueeze = 35;
    volSpeed = 5;
    volSqueeze = 40;
  }
};

class FxColorClouds : public ColorCloudsBase
{
public:
  static const EffectID FX_id = AutoSelectID;
  static const char FX_data[];

  explicit FxColorClouds(FxSetup &fxs) : ColorCloudsBase(fxs)
  {
    // hueSpeed = 255;
    // hueSpeed = 128;
    // hueSpeed = 3;
    // hueSpeed = 0;

    // hueSqueeze = 255;
    // hueSqueeze = 128;
    // hueSqueeze = 25;
    // hueSqueeze = 0;

    // volSpeed = 255;
    // volSpeed = 128;
    // volSpeed = 25;
    // volSpeed = 0;

    // volSqueeze = 255;
    // volSqueeze = 128;
    // volSqueeze = 45;
    // volSqueeze = 0;
  }

private:
  uint16_t showEffect(uint32_t now) override
  {
    moreRed = config.check3();
    return showClouds(now);
  }
};
const char FxColorClouds::FX_data[] PROGMEM = "Color Clouds@,,,,,,,More red;;;;o3=1";
// const char FxColorClouds::FX_data[] PROGMEM = "Color Clouds@Speed,Clouds,,,,,,,More red;;;;o3=1";

class FxColorCloudsAmbient : public ColorCloudsBase
{
public:
  static const EffectID FX_id = AutoSelectID;
  static const char FX_data[];

  explicit FxColorCloudsAmbient(FxSetup &fxs) : ColorCloudsBase(fxs) { setAmbient(); }

private:
  uint16_t showEffect(uint32_t now) override
  {
    moreRed = config.check3();
    return showClouds(now);
  }
};
const char FxColorCloudsAmbient::FX_data[] PROGMEM = "Color Clouds Ambient@,,,,,,,More red";

class FxColorCloudsExtraSlow : public ColorCloudsBase
{
public:
  static const EffectID FX_id = AutoSelectID;
  static const char FX_data[];

  explicit FxColorCloudsExtraSlow(FxSetup &fxs) : ColorCloudsBase(fxs) { setExtraSlow(); }

private:
  uint16_t showEffect(uint32_t now) override
  {
    moreRed = config.check3();
    return showClouds(now);
  }
};
const char FxColorCloudsExtraSlow::FX_data[] PROGMEM = "Color Clouds Turtle@,,,,,,,More red;;;;o3=1";

//--------------------------------------------------------------------------------------------------

/// more effect class examples ...

//--------------------------------------------------------------------------------------------------

void UmEffectRunner::addEffects(WS2812FX &wled)
{
  wled.addEffect(AutoSelectID, &mode_PxStrip_example, _data_FX_mode_PxStrip);
  addEffectRunner<FxColorClouds>(wled);
  addEffectRunner<FxColorCloudsAmbient>(wled);
  addEffectRunner<FxColorCloudsExtraSlow>(wled);
  addEffectRunner<FxFallback>(wled);
  addEffectRunner<FxInlineEq>(wled);
  addEffectRunner<FxInlineEq2>(wled);
  addEffectRunner<FxSoundmeter>(wled);
  addEffectRunner<FxLarson>(wled);
}

//--------------------------------------------------------------------------------------------------

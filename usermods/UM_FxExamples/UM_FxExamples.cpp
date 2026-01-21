/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"

//--------------------------------------------------------------------------------------------------

/** This is just for my dev setup.
 * @see https://github.com/wled/WLED/pull/5268 ;-)
 */
uint16_t mode_ColorClouds()
{
  // Set random start points for clouds and color.
  if (SEGENV.call == 0)
  {
    SEGENV.aux0 = hw_random16();
    SEGENV.aux1 = hw_random16();
  }
  const uint32_t volX0 = SEGENV.aux0;
  const uint32_t hueX0 = SEGENV.aux1;
  const uint8_t hueOffset0 = volX0 + hueX0;

  // Put more emphasis on the red'ish colors when true (or begin & end of palette).
  const bool moreRed = SEGMENT.check3;

  // Higher values make the clouds move faster.
  const uint32_t volSpeed = 1 + SEGMENT.speed;

  // Higher values make the color change faster.
  const uint32_t hueSpeed = 1 + SEGMENT.intensity;

  // Higher values make more clouds (but smaller ones).
  const uint32_t volSqueeze = 8 + SEGMENT.custom1;

  // Higher values make the clouds more colorful.
  const uint32_t hueSqueeze = SEGMENT.custom2;

  // Higher values make larger gaps between the clouds.
  const long volCutoff = 12500 + SEGMENT.custom3 * 900;
  const long volSaturate = 52000;
  // Note: When adjusting these calculations, ensure that volCutoff is always smaller than volSaturate.

  const uint32_t now = strip.now;
  const uint32_t volT = now * volSpeed / 8;
  const uint32_t hueT = now * hueSpeed / 8;
  const uint8_t hueOffset = beat88(64) >> 8;

  for (int i = 0; i < SEGLEN; i++)
  {
    const uint32_t volX = i * volSqueeze * 64;
    long vol = perlin16(volX0 + volX, volT);
    vol = map(vol, volCutoff, volSaturate, 0, 255);
    vol = constrain(vol, 0, 255);

    const uint32_t hueX = i * hueSqueeze * 8;
    uint8_t hue = perlin16(hueX0 + hueX, hueT) >> 7;
    hue += hueOffset0;
    hue += hueOffset;
    if (moreRed)
    {
      hue = cos8_t(128 + hue / 2);
    }

    uint32_t pixel;
    if (SEGMENT.palette)
    {
      pixel = SEGMENT.color_from_palette(hue, false, true, 0, vol);
    }
    else
    {
      hsv2rgb(CHSV32(hue, 255, vol), pixel);
    }

    // Suppress extremely dark pixels to avoid flickering of plain r/g/b.
    // Unfortunately this doesn't always work properly when gamma correction for color is enabled.
    // So, when using this effect standalone, also try it without color gamma correction.
    if (int(R(pixel)) + G(pixel) + B(pixel) <= 2)
    {
      pixel = 0;
    }

    SEGMENT.setPixelColor(i, pixel);
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_COLORCLOUDS[] PROGMEM = "Color Clouds@Cloud speed,Color speed,Clouds,Colors,Distance,,,More red;;!;;sx=24,ix=32,c1=48,c2=64,c3=12,pal=0";

//--------------------------------------------------------------------------------------------------

/** A usermod for effect examples.
 */
class UM_FxExamples : public Usermod
{
  void setup() override
  {
    registerEffects();
  }

  void loop() override
  {
  }

  void registerEffects()
  {
    strip.addEffect(218, &mode_ColorClouds, _data_FX_MODE_COLORCLOUDS);
  }
};

//--------------------------------------------------------------------------------------------------

static UM_FxExamples um_FxExamples;
REGISTER_USERMOD(um_FxExamples);

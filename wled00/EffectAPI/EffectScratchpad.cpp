/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "EffectAPI.h"

//--------------------------------------------------------------------------------------------------

void fx_Scratchpad(FxEnv &env)
{
  auto &ui = env.ui();
  auto &matrix = env.pxMatrix();

  const uint8_t glow = ui.custom3_reduced();
  if (glow == 0)
    matrix.clear();
  else
    matrix.fastFade(32 - glow);

  const NIndex x0 = beatsinF(ui.speed() / 2.0f);
  const NIndex y0 = beatsinF(ui.intensity() / 2.0f, 0, uint16_max / 2);
  const NIndex x1 = beatsinF(ui.custom1() / 2.0f, 0, uint16_max / 4);
  const NIndex y1 = beatsinF(ui.custom2() / 2.0f);

  const PxColor color = rainbowColor(env, (env.now() >> 4) & 0xFF);
  line_N(matrix, {x0, y0}, {x1, y1}, color, ui.check3());
  // matrix.setColor_N({x0, y0}, 0x880000);
  // matrix.setColor_N({x1, y1}, 0x008800);
}
static const char _data_FX_SCRATCHPAD_FCT[] PROGMEM = "! Scratchpad Fct@X0,Y0,X1,Y1,Glow,,,Soft;;!;2;sx=0,ix=0,c1=64,c2=64,c3=16,o3=1,pal=0";
// Keep this as backup!
// static const char _data_FX_SCRATCHPAD_FCT[] PROGMEM = "! Scratchpad Fct@speed,intensity,custom1,custom2,custom3,check1,check2,check3;fx,bg,cs;!;;sx=98,ix=76,c1=54,c2=32,c3=10,o1=1,o2=1,o3=1,pal=11";

//--------------------------------------------------------------------------------------------------

class FX_Scratchpad : public EffectBase
{

public:
  explicit FX_Scratchpad(FxSetup &fxs) : EffectBase(fxs) {}

private:
  void showEffect(FxEnv &env) override
  {
    auto &ui = env.ui();
    auto &matrix = env.pxMatrix();

    const uint8_t glow = ui.custom3_reduced();
    if (glow == 0)
      matrix.clear();
    else if (mustFade(env))
      matrix.fastFade(32 - glow);

    const NIndex x0 = beatsinF(ui.speed() / 2.0f);
    const NIndex y0 = beatsinF(ui.intensity() / 2.0f, 0, uint16_max / 2);
    const NIndex x1 = beatsinF(ui.custom1() / 2.0f, 0, uint16_max / 4);
    const NIndex y1 = beatsinF(ui.custom2() / 2.0f);

    const PxColor color = rainbowColor(env, (env.now() >> 4) & 0xFF);
    line_N(matrix, {x0, y0}, {x1, y1}, color, ui.check3());
    // matrix.setColor_N({x0, y0}, 0x880000);
    // matrix.setColor_N({x1, y1}, 0x008800);
  }
};
static const char _data_FX_SCRATCHPAD_CLASS[] PROGMEM = "! Scratchpad Class@X0,Y0,X1,Y1,Glow,,,Soft;;!;2;sx=0,ix=0,c1=64,c2=64,c3=16,o3=1,pal=0";
// Keep this as backup!
// static const char _data_FX_SCRATCHPAD_CLASS[] PROGMEM = "! Scratchpad Class@speed,intensity,custom1,custom2,custom3,check1,check2,check3;fx,bg,cs;!;;sx=98,ix=76,c1=54,c2=32,c3=10,o1=1,o2=1,o3=1,pal=11";

//--------------------------------------------------------------------------------------------------

void addEffectScratchpad(WS2812FX &wled)
{
  uint8_t fctID = 255;
  uint8_t classID = 255;
  // fctID = 219;
  classID = 219;
  addEffectFunction<fx_Scratchpad>(wled, fctID, _data_FX_SCRATCHPAD_FCT);
  addEffectClass<FX_Scratchpad>(wled, classID, _data_FX_SCRATCHPAD_CLASS);
}

//--------------------------------------------------------------------------------------------------

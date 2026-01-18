/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "EffectBase.h"
#include "FxEnv.h"
#include "FxExtras.h"
#include "FxExtras1D.h"
#include "FxExtras2D.h"

//--------------------------------------------------------------------------------------------------

uint16_t FxEnv::showEffect(uint32_t now)
{
  updateTime(now);
  if (_effect)
  {
    _effect->show(*this);
  }
  else
  {
    showFallbackEffect();
  }
  return _frametime;
}

void FxEnv::updateTime(uint32_t now)
{
  _deltaT = now - _now;
  _age += _deltaT;
  _now = now;
}

void FxEnv::showFallbackEffect()
{
  fx_broken(*this);
  setBroken();
}

bool FxEnv::updateSegment(Segment &seg, Segenv &segenv)
{
  const bool new_is2D = seg.is2D();

  const bool dimensionChanged = _pxArray.updateSegment(seg) ||
                                _pxMatrix.updateSegment(seg) ||
                                (_is2D != new_is2D);

  _is2D = new_is2D;
  _config._seg = &seg;
  _segenv = &segenv;

#ifndef WLED_DISABLE_PARTICLESYSTEM1D
  if (_partSys_1D)
  {
    _partSys_1D = reinterpret_cast<ParticleSystem1D *>(seg.data);
  }
#endif
#ifndef WLED_DISABLE_PARTICLESYSTEM2D
  if (_partSys_2D)
  {
    _partSys_2D = reinterpret_cast<ParticleSystem2D *>(seg.data);
  }
#endif

  return dimensionChanged;
}

#ifndef WLED_DISABLE_PARTICLESYSTEM1D
ParticleSystem1D *FxEnv::initPS_1D(const uint32_t requestedsources,
                                   const uint8_t fractionofparticles,
                                   const uint32_t additionalbytes,
                                   const bool advanced)
{
  if (!initParticleSystem1D(_partSys_1D, requestedsources, fractionofparticles, additionalbytes, advanced) || !_partSys_1D)
  {
    onFxEnvAllocFailed();
    return nullptr;
  }
  return _partSys_1D;
}
#endif

#ifndef WLED_DISABLE_PARTICLESYSTEM2D
ParticleSystem2D *FxEnv::initPS_2D(const uint32_t requestedsources,
                                   const uint32_t additionalbytes,
                                   const bool advanced,
                                   const bool sizecontrol)
{
  if (!initParticleSystem2D(_partSys_2D, requestedsources, additionalbytes, advanced, sizecontrol) || !_partSys_2D)
  {
    onFxEnvAllocFailed();
    return nullptr;
  }
  return _partSys_2D;
}
#endif

//--------------------------------------------------------------------------------------------------

void fx_broken(FxEnv &env)
{
  PxArray &leds = env.pxArray();
  leds.clear();

#if (1)
  const auto p1 = beatsin16_t(13 << 6) / 65535.0f * 1.2f - 0.1f;
  const auto p2 = beatsin16_t(11 << 6) / 65535.0f * 1.2f - 0.1f;
  RainbowColorSource colorSource{env};
  colorSource.setOffset_N(p1);
  colorLine_N(leds, p1, p2, colorSource);
#else
  const int p1 = beatsin16_t(13 << 6, 0, env.seglen() - 1);
  const int p2 = beatsin16_t(11 << 6, 0, env.seglen() - 1);

  const PxColor c1 = env.ui().fxColor();
  line(leds, p1, p2, c1);

  const PxColor c2 = ~c1.raw & 0x00FFFFFF;
  leds[p1] = c2;
  leds[p2] = c2;
#endif
}

//--------------------------------------------------------------------------------------------------

void fx_Scratchpad(FxEnv &env)
{
  auto &ui = env.ui();
  auto &matrix = env.pxMatrix();

  const NIndex x0 = beatsinF(ui.speed() / 2.0f);
  const NIndex y0 = beatsinF(ui.intensity() / 2.0f, 0, uint16_max / 2);
  const NIndex x1 = beatsinF(ui.custom1() / 2.0f, 0, uint16_max / 4);
  const NIndex y1 = beatsinF(ui.custom2() / 2.0f);

  matrix.clear();
  const auto color = rainbowColor(env, (env.now() >> 4) & 0xFF);
  line_N(matrix, {x0, y0}, {x1, y1}, color, ui.check3());
  // matrix.setColor_N({x0, y0}, 0x880000);
  // matrix.setColor_N({x1, y1}, 0x008800);
}
const char _data_FX_SCRATCHPAD_FCT[] PROGMEM = "! Scratchpad Fct@X0,Y0,X1,Y1,,,,Soft;;!;;sx=0,ix=0,c1=64,c2=64,o3=1,pal=0";
// const char _data_FX_SCRATCHPAD_FCT[] PROGMEM = "! Scratchpad Fct@speed,intensity,custom1,custom2,custom3,check1,check2,check3;fx,bg,cs;!;;sx=98,ix=76,c1=54,c2=32,c3=10,o1=1,o2=1,o3=1,pal=11";

//--------------------------------------------------------------------------------------------------

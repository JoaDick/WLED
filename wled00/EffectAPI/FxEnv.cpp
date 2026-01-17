/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "EffectBase.h"
#include "FxEnv.h"

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
  const uint16_t new_seglen = seg.vLength();
  const uint16_t new_segW = seg.vWidth();
  const uint16_t new_segH = seg.vHeight();
  const bool new_is2D = seg.is2D();

  const bool dimensionChanged = (_seglen != new_seglen) ||
                                (_segW != new_segW) ||
                                (_segH != new_segH) ||
                                (_is2D != new_is2D);

  _seglen = new_seglen;
  _segW = new_segW;
  _segH = new_segH;
  _is2D = new_is2D;
  _seg = &seg;
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
  Segment &seg = env.seg();
  seg.clear();

  const int p1 = beatsin16_t(13 << 6, 0, env.seglen() - 1);
  const int p2 = beatsin16_t(11 << 6, 0, env.seglen() - 1);

  const PxColor c1 = env.ui().fxColor();
  auto tmp = p1;
  while (tmp < p2)
    seg.setPixelColor(tmp++, c1);
  while (tmp > p2)
    seg.setPixelColor(tmp--, c1);

  const PxColor c2 = ~c1.raw & 0x00FFFFFF;
  seg.setPixelColor(p1, c2);
  seg.setPixelColor(p2, c2);
}

//--------------------------------------------------------------------------------------------------

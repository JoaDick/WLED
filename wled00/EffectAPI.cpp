/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "EffectAPI.h"
#include "FXparticleSystem.h"

//--------------------------------------------------------------------------------------------------
// class FxEnv

void FxEnv::showFallbackEffect()
{
  fx_broken(*this);
  setBroken();
}

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

bool FxEnv::updateSegment(Segment &seg, SegEnv &segenv)
{
#if (1)
  const uint16_t new_seglen = seg.virtualLength();
  const uint16_t new_segW = seg.virtualWidth();
  const uint16_t new_segH = seg.virtualHeight();
#else
  const uint16_t new_seglen = seg.vLength();
  const uint16_t new_segW = seg.vWidth();
  const uint16_t new_segH = seg.vHeight();
#endif
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
  _segenv = &segenv;

  return dimensionChanged;
}

void FxEnv::updateTime(uint32_t now)
{
  _deltaT = now - _now;
  _age += _deltaT;
  _now = now;
}

void fx_broken(FxEnv &env)
{
  Segment &seg = env.seg();

  const uint32_t c1 = seg.getCurrentColor(0);
  const uint32_t c2 = ~c1 & 0x00FFFFFF;
  const uint16_t p1 = beatsin16_t(13 << 6, 0, env.seglen() - 1);
  const uint16_t p2 = beatsin16_t(11 << 6, 0, env.seglen() - 1);

  seg.fill(0);
  auto tmp = p1;
  while (tmp < p2)
    seg.setPixelColor(tmp++, c1);
  while (tmp > p2)
    seg.setPixelColor(tmp--, c1);
  seg.setPixelColor(p1, c2);
  seg.setPixelColor(p2, c2);
}

//--------------------------------------------------------------------------------------------------
// class EffectBase

EffectBase::EffectBase(FxSetup &fxs)
{
  FxEnv &env = fxs.env();
  Segment &seg = env.seg();
  seg.fill(0);
}

void EffectBase::show(FxEnv &env)
{
  if (env.isBroken())
  {
    env.showFallbackEffect();
  }
  else
  {
    showEffect(env);
  }
}

//--------------------------------------------------------------------------------------------------
// class SegEnv

size_t SegEnv::_allDataSize = 0;

SegEnv::SegEnv(const SegEnv &other)
    : _call{other._call}, step{other.step}, aux0{other.aux0}, aux1{other.aux1},
      _partSys_1D{other._partSys_1D}, _partSys_2D{other._partSys_2D}
{
  if (this != &other)
  {
    if (other._dataSize)
    {
      if (allocateData(other._dataSize))
      {
        memcpy(_data, other._data, _dataSize);
      }
    }
  }
}

void SegEnv::updateSegment(Segment &seg)
{
  if (_partSys_1D)
  {
    _partSys_1D = reinterpret_cast<ParticleSystem1D *>(seg.data);
  }
  if (_partSys_2D)
  {
    _partSys_2D = reinterpret_cast<ParticleSystem2D *>(seg.data);
  }
}

bool SegEnv::allocateData(size_t size)
{
  // Note: Checking for size == 0 is intentionally omitted (no pointless paranoia checks).
  // --> We can always successfully allocate zero bytes :-)

  // not enough memory allocated yet?
  if (_dataSize < size)
  {
    deallocateData();

#ifndef BOARD_HAS_PSRAM
    // limit to MAX_SEGMENT_DATA if there is no PSRAM, otherwise prefer functionality over speed
    if (Segment::getUsedSegmentData() + _allDataSize + size > MAX_SEGMENT_DATA)
    {
      DEBUG_PRINTF_P(PSTR("SegEnv: SegmentData limit reached [%d/%d/%d]\n"), size, Segment::getUsedSegmentData(), _allDataSize);
      errorFlag = ERR_NORAM;
      onSegEnvAllocFailed();
      return false;
    }
#endif

    // prefer DRAM over PSRAM for speed
    _data = allocate_buffer(size, BFRALLOC_PREFER_DRAM | BFRALLOC_CLEAR);
    if (_data == nullptr)
    {
      DEBUG_PRINTF_P(PSTR("SegEnv %p: Allocate failed [%d/%d/%d]\n"), this, size, Segment::getUsedSegmentData(), _allDataSize);
      errorFlag = ERR_NORAM;
      DEBUG_PRINTLN(F("!!! SegEnv: Allocation failed. !!!"));
      onSegEnvAllocFailed();
      return false;
    }

    _dataSize = size;
    _allDataSize += _dataSize;
    // DEBUG_PRINTF_P(PSTR("SegEnv %p: Allocated [%d] @ %p\n"), this, _dataSize, _data);
  }

  return true;
}

void SegEnv::deallocateData()
{
  if (_dataSize)
  {
    // DEBUG_PRINTF_P(PSTR("SegEnv %p: Deallocate [%d] @ %p\n"), this, _dataSize, _data);
    d_free(_data);
    _allDataSize -= _dataSize;
    _dataSize = 0;
    _data = nullptr;
  }
}

void SegEnv::reset()
{
  deallocateData();
  _call = 0;
  step = 0;
  aux0 = 0;
  aux1 = 0;
}

ParticleSystem1D *SegEnv::initPS_1D(const uint32_t requestedsources,
                                    const uint8_t fractionofparticles,
                                    const uint32_t additionalbytes,
                                    const bool advanced)
{
  if (!initParticleSystem1D(_partSys_1D, requestedsources, fractionofparticles, additionalbytes, advanced) || !_partSys_1D)
  {
    onSegEnvAllocFailed();
    return nullptr;
  }
  return _partSys_1D;
}

ParticleSystem2D *SegEnv::initPS_2D(const uint32_t requestedsources,
                                    const uint32_t additionalbytes,
                                    const bool advanced,
                                    const bool sizecontrol)
{
  if (!initParticleSystem2D(_partSys_2D, requestedsources, additionalbytes, advanced, sizecontrol) || !_partSys_2D)
  {
    onSegEnvAllocFailed();
    return nullptr;
  }
  return _partSys_2D;
}

//--------------------------------------------------------------------------------------------------

/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * @file This file provides the runtime environment for rendering the effects.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "FX.h"
#include "FXparticleSystem.h"
#include "FxConfig.h"
#include "PxColor.h"
#include "PxHelper.h"
#include "Segenv.h"

//--------------------------------------------------------------------------------------------------
class EffectBase;
using RawEffectPtr = EffectBase *;

/** Runtime environment for rendering the effects.
 * Concrete effect implementations obtain all the necessary runtime information and interfaces for
 * rendering their animation from here.
 */
class FxEnv
{
public:
  // no general copy & move - this class is intended to be passed as reference to other functions
  FxEnv(FxEnv &&) = delete;
  FxEnv &operator=(const FxEnv &) = delete;
  FxEnv &operator=(FxEnv &&) = delete;

  // ----- time related methods -----

  /** The current timestamp (in ms).
   * @note Effect implementations shall use this instead of \c strip.now
   */
  uint32_t now() const { return _now; }

  /** Age of the effect (in ms) since it was instantiated.
   * Alternative to now(), which always starts counting at 0.
   */
  uint32_t age() const { return _age; }

  /// Duration (in ms) since the effect was rendered the last time.
  uint32_t deltaT() const { return _deltaT; }

  /** Returns \c true only for the during the very first frame.
   * @note Try to avoid using this method. Prefer putting initialization stuff into the constructor
   * of your effect class.
   */
  bool isFistFrame() const { return _segenv->call == 0; }

  // ----- rendering related methods -----

  /** Get the segment on which the effect shall be rendered.
   * @note Effect implementations shall use this instead of \c SEGMENT
   */
  Segment &seg() { return _pxArray.seg(); }

  /** Get the length of the segment.
   * @note Effect implementations shall use this instead of \c SEGLEN
   */
  int seglen() const { return _pxArray.size(); }

  /** Get the width of the segment.
   * @note Effect implementations shall use this instead of \c SEG_W
   */
  int segW() const { return _pxMatrix.sizeX(); }

  /** Get the height of the segment.
   * @note Effect implementations shall use this instead of \c SEG_H
   */
  int segH() const { return _pxMatrix.sizeY(); }

  /** Check if the segment is configured as a 2D setup.
   * @note Effect implementations shall use this instead of \c SEGMENT.is2D()
   */
  bool is2D() const { return _is2D; }

  /// Get the 1D canvas for rendering the pixel magic.
  SegmentPxArray &pxArray() { return _pxArray; }

  /// Get the 2D canvas for rendering the pixel magic.
  SegmentPxMatrix &pxMatrix() { return _pxMatrix; }

  // ----- methods for accessing resources  -----

  /// Get user configuration data (settings from the UI).
  FxConfig &ui() { return _config; }

  /** Get persistent effect data.
   * @note Effect implementations shall use this instead of \c SEGENV
   * Nevertheless, prefer your own effect class member variables over this.
   */
  Segenv &segenv() { return *_segenv; }

  /** Get currently selected color palette.
   * @note Effect implementations shall use this instead of \c SEGPALETTE
   */
  const CRGBPalette16 &currentPalette() { return seg().getCurrentPalette(); }

#ifndef WLED_DISABLE_PARTICLESYSTEM1D
  /** EXPERIMENTAL
   * ...
   * The effect is marked as broken when the allocation failed, so its rendering function won't be
   * called anymore.
   * @param PartSys Pointer to ParticleSystem (which will be redirected).
   * @retval \c true Success; \a PartSys is now pointing to a valid ParticleSystem1D instance.
   * @retval \c false Allocation failed; do \e not use \a PartSys
   * @note ParticleSystem allocates memory via Segment, thus completely independent from \c allocateData()
   */
  bool getParticleSystem(ParticleSystem1D *&PartSys,
                         const uint32_t requestedsources,
                         const uint8_t fractionofparticles = 255,
                         const uint32_t additionalbytes = 0,
                         const bool advanced = false)

  {
    return getParticleSystem([](ParticleSystem1D *) {}, PartSys, requestedsources, fractionofparticles, additionalbytes, advanced);
  }

  template <typename PS_INIT_FCT>
  bool getParticleSystem(PS_INIT_FCT initFct,
                         ParticleSystem1D *&PartSys,
                         const uint32_t requestedsources,
                         const uint8_t fractionofparticles = 255,
                         const uint32_t additionalbytes = 0,
                         const bool advanced = false)
  {
    PartSys = _partSys_1D;
    if (!PartSys)
    {
      PartSys = initPS_1D(requestedsources, fractionofparticles, additionalbytes, advanced);
      if (!PartSys)
      {
        return false;
      }
      initFct(PartSys);
    }
    return true;
  }
#endif

#ifndef WLED_DISABLE_PARTICLESYSTEM2D
  /** EXPERIMENTAL
   * ...
   * The effect is marked as broken when the allocation failed, so its rendering function won't be
   * called anymore.
   * @param PartSys Pointer to ParticleSystem (which will be redirected).
   * @retval \c true Success; \a PartSys is now pointing to a valid ParticleSystem1D instance.
   * @retval \c false Allocation failed; do \e not use \a PartSys
   * @note ParticleSystem allocates memory via Segment, thus completely independent from \c allocateData()
   */
  bool getParticleSystem(ParticleSystem2D *&PartSys,
                         const uint32_t requestedsources,
                         const uint32_t additionalbytes = 0,
                         const bool advanced = false,
                         const bool sizecontrol = false)
  {
    return getParticleSystem([](ParticleSystem2D *) {}, PartSys, requestedsources, additionalbytes, advanced, sizecontrol);
  }

  template <typename PS_INIT_FCT>
  bool getParticleSystem(PS_INIT_FCT initFct,
                         ParticleSystem2D *&PartSys,
                         const uint32_t requestedsources,
                         const uint32_t additionalbytes = 0,
                         const bool advanced = false,
                         const bool sizecontrol = false)
  {
    PartSys = _partSys_2D;
    if (!PartSys)
    {
      PartSys = initPS_2D(requestedsources, additionalbytes, advanced, sizecontrol);
      if (!PartSys)
      {
        return false;
      }
      initFct(PartSys);
    }
    return true;
  }
#endif

  // ----- effect related methods -----

  /** Mark the effect as non-functional.
   * Can be called when something went terribly wrong, and the effect is no more working.
   * @note The effect's rendering function will no more be called after this!
   */
  void setBroken() { _effect = nullptr; }

  /// Check if the effect is broken.
  bool isBroken() const { return _effect == nullptr; }

  /** Set the effect's frametime.
   * @param ms Delay between calling the effect's rendering function (in ms); 0 means use default.
   * @note Try to avoid using this feature. Set a specific frametime only if the effect cannot adapt
   * to WLED's default frametime.
   */
  void setFrametime(uint16_t ms) { _frametime = ms; }

  /** Fallback rendering function.
   * Can be called as fallback by an effect when it cannot render its own stuff, e.g. when something
   * went terribly wrong.
   * @note Calling this function implies setBroken() - which means that the effect's rendering
   * function will no more be called!
   */
  void showFallbackEffect();

protected:
  FxEnv(const FxEnv &) = default;
  FxEnv(Segment &seg, Segenv &segenv, uint32_t now)
      : _pxArray{seg}, _pxMatrix{seg}, _config{seg}, _now{now} { updateSegment(seg, segenv); }
  ~FxEnv() = default;

  /** Render the effect's pixel magic on the segment.
   * @param now The current timestamp (in ms).
   * @return Specific frametime (in ms), or 0 to use WLED's default setting.
   */
  uint16_t showEffect(uint32_t now);

  /** Call this method when the segment or its setting has changed.
   * @param seg The changed segment wo work on from now.
   * @return \c true When the segment's dimension has changed.
   */
  bool updateSegment(Segment &seg, Segenv &segenv);

  /** Store the given \a effect to be controlled.
   * @note Be aware that \a effect is not initialized yet, so don't use it here!
   */
  void storeEffect(RawEffectPtr effect) { _effect = effect; }

  /** This method is called when an allocation has failed.
   * Child class shall mark the effect as broken.
   */
  virtual void onFxEnvAllocFailed() = 0;

private:
  void updateTime(uint32_t now);

#ifndef WLED_DISABLE_PARTICLESYSTEM1D
  ParticleSystem1D *initPS_1D(const uint32_t requestedsources,
                              const uint8_t fractionofparticles,
                              const uint32_t additionalbytes,
                              const bool advanced);
#endif

#ifndef WLED_DISABLE_PARTICLESYSTEM2D
  ParticleSystem2D *initPS_2D(const uint32_t requestedsources,
                              const uint32_t additionalbytes,
                              const bool advanced,
                              const bool sizecontrol);
#endif

private:
  FxConfig _config;
  SegmentPxArray _pxArray;
  SegmentPxMatrix _pxMatrix;
  RawEffectPtr _effect = nullptr;
  Segenv *_segenv = nullptr;
#ifndef WLED_DISABLE_PARTICLESYSTEM1D
  ParticleSystem1D *_partSys_1D = nullptr;
#endif
#ifndef WLED_DISABLE_PARTICLESYSTEM2D
  ParticleSystem2D *_partSys_2D = nullptr;
#endif
  uint32_t _now = 0;
  uint32_t _age = 0;
  uint32_t _deltaT = 0;
  uint16_t _frametime = 0;
  bool _is2D = false;
};

/// The worm of fail.
void fx_broken(FxEnv &env);

//--------------------------------------------------------------------------------------------------

void fx_Scratchpad(FxEnv &env);
extern const char _data_FX_SCRATCHPAD_FCT[];

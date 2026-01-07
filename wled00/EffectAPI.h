/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * This headerfile provides all utilities for custom class-based WLED effect implementations.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "FX.h"

//--------------------------------------------------------------------------------------------------
class EffectBase;

/// Non-owning pointer to an effect.
using RawEffectPtr = EffectBase *;

class FxConfig; // not implemented yet
class SegEnv;   // not implemented yet

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

  /** Get the segment on which the effect shall be rendered.
   * @note Effect implementations shall use this instead of \c SEGMENT
   */
  Segment &seg() { return *_seg; }

  /** Get the length of the segment.
   * @note Effect implementations shall use this instead of \c SEGLEN
   */
  uint16_t seglen() const { return _seglen; }

  /** Get the width of the segment.
   * @note Effect implementations shall use this instead of \c SEG_W
   */
  uint16_t segW() const { return _segW; }

  /** Get the height of the segment.
   * @note Effect implementations shall use this instead of \c SEG_H
   */
  uint16_t segH() const { return _segH; }

  /** Check if the segment is configured as a 2D setup.
   * @note Effect implementations shall use this instead of \c SEGMENT.is2D()
   */
  bool is2D() const { return _is2D; }

  /// Get user configuration data (from the UI).
  FxConfig &config(); // not implemented yet

  /** Fallback rendering function.
   * Can be called as fallback by an effect when it cannot render its own stuff, e.g. when something
   * like allocating additional effect memory went wrong.
   * @note Calling this function implies setBroken(), meaning that the concrete effect's rendering
   * function will no more be called!
   */
  void showFallbackEffect();

  /** Mark the effect as non-functional.
   * Can be called when something went terribly wrong (e.g. like allocating additional effect memory),
   * and the effect is no more working.
   * @note The concrete effect's rendering function will no more be called after this!
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

  /** Returns \c true only for the during the very first frame.
   * @note Try to avoid using this method. Prefer putting initialization stuff into the constructor.
   */
  bool isFistFrame() const { return _seg->call == 0; }

  /** Get persistent effect data from the segment (for legacy compatibility).
   * @note Effect implementations shall use this instead of \c SEGENV
   * Nevertheless, prefer your own effect class member variables over this.
   */
  SegEnv &segenv(); // not implemented yet

protected:
  FxEnv(const FxEnv &) = default;
  FxEnv(Segment &seg, uint32_t now) : _now{now} { updateSegment(seg); }
  ~FxEnv() = default;

  /** Render the effect's pixel magic on the segment.
   * @param now The current timestamp (in ms).
   * @return Specific frametime (in ms), or 0 to use WLED's default setting.
   */
  uint16_t showEffect(uint32_t now);

  /** Call this method when the segment or any of its setting has changed.
   * @param seg The changed segment wo work on from now.
   * @return \c true When the segment's dimension has changed.
   */
  bool updateSegment(Segment &seg);

  /** Store the given \a effect to be controlled.
   * @note Be aware that \a effect is not initialized yet, so don't use it here!
   */
  void storeEffect(RawEffectPtr effect) { _effect = effect; }

private:
  void updateTime(uint32_t now);

private:
  RawEffectPtr _effect = nullptr;
  Segment *_seg = nullptr;
  uint32_t _now = 0;
  uint32_t _age = 0;
  uint32_t _deltaT = 0;
  uint16_t _seglen = 0;
  uint16_t _segW = 0;
  uint16_t _segH = 0;
  uint16_t _frametime = 0;
  bool _is2D = false;
};

//--------------------------------------------------------------------------------------------------

/** Properties of an effect.
 * These properties can be specified by the constructor of a concrete effect implementation.
 */
class FxProperties
{
public:
  // no general copy & move - this class is intended to be passed as reference to other functions
  FxProperties(FxProperties &&) = delete;
  FxProperties &operator=(const FxProperties &) = delete;
  FxProperties &operator=(FxProperties &&) = delete;

  /// Indicate that the effect can handle cnahges of the segment on the fly without being recreated.
  void setResizeSupportEnabled() { _isResizeSupportEnabled = true; }

  /// Indicate that the effect can \a only be used with a 2D setup.
  void setRequires2D() { _isRequired2D = true; }

protected:
  FxProperties(const FxProperties &) = default;
  FxProperties() = default;
  ~FxProperties() = default;

protected:
  bool _isResizeSupportEnabled = false;
  bool _isRequired2D = false;
};

//--------------------------------------------------------------------------------------------------

/** Interface to access setup data for creating effect instances.
 * This interface is passed to the constructor of all concrete effect implementations.
 */
class FxSetup
{
public:
  /// Use this to specify specific properties of the effect (optional).
  virtual FxProperties &props() = 0;

  /// Use this to initialize the LED strip (optional).
  /// @note All LEDs of the segment are already switched off automatically.
  virtual FxEnv &env() = 0;

protected:
  ~FxSetup() = default;
};

//--------------------------------------------------------------------------------------------------

/** Base class for all effects.
 * Class-based effects must derive from this base class. This requires them to implement the
 * \c showEffect() method, where all the fancy pixel magic happens.
 */
class EffectBase
{
public:
  // no general copy & move - only child classes may be copy-constructed
  EffectBase(EffectBase &&) = delete;
  EffectBase &operator=(const EffectBase &) = delete;
  EffectBase &operator=(EffectBase &&) = delete;

  /** Render the effect.
   * @param env Runtime environment for rendering the effect.
   */
  void show(FxEnv &env);

protected:
  EffectBase(const EffectBase &) = default;
  ~EffectBase() = default;

  /** Constructor; child classes have to pass \a fxs to this base class.
   * @note All LEDs of the segment are switched off implicitly.
   */
  explicit EffectBase(FxSetup &fxs);

  /** Child's effect rendering function.
   * Must be implemented by all child classes to show their specific pixel magic.
   * @param env Runtime environment for rendering the effect.
   * @note This method won't be called anymore when the effect is broken!
   */
  virtual void showEffect(FxEnv &env) = 0;
};

//--------------------------------------------------------------------------------------------------

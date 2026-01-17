/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * @file This file provides the base class for custom effects.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "FxEnv.h"

//--------------------------------------------------------------------------------------------------
class FxProperties;
class FxSetup;

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

  /** Indicate that the effect can handle changes of the segment's dimensions on the fly without
   * having to be recreated.
   */
  void setSupported_segmentResize() { _isSupported_segmentResize = true; }

  /// Validate the minimum length of the segment.
  bool validate_minSeglen(uint16_t minSeglen)
  {
    _required_minSeglen = minSeglen;
    return validateOrSetBroken(env().seglen() >= minSeglen);
  }

  /// Validate that the segment is a 2D setup.
  bool validate_is2D()
  {
    _required_is2D = true;
    return validateOrSetBroken(env().is2D() == true);
  }

protected:
  FxProperties(const FxProperties &) = default;
  FxProperties() = default;
  ~FxProperties() = default;
  virtual FxEnv &getFxEnv() = 0;

private:
  FxEnv &env() { return getFxEnv(); }

  bool validateOrSetBroken(bool checkResult)
  {
    if (checkResult == false)
      getFxEnv().setBroken();
    return checkResult;
  }

protected:
  bool _isSupported_segmentResize = false;
  uint16_t _required_minSeglen = 0;
  bool _required_is2D = false;
};

//--------------------------------------------------------------------------------------------------

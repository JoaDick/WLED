/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * @file This file provides all utilities for custom effect implementations.
 * It is the only one that needs to be included in your own sourcefiles.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "FX.h"
#include "EffectAdapter.h"
#include "EffectBase.h"
#include "FxExtras.h"
#include "FxExtras1D.h"
#include "FxExtras2D.h"

//--------------------------------------------------------------------------------------------------

/** Pointer to a new style of free effect-function (as alternative for the existing mode-function).
 * The essential difference is that it gets an \c FxEnv as argument (for rendering), and doesn't
 * have a returnvalue. As a consequence (but only if really needed), a specific frametime can be
 * announced via \a env.setFrametime()
 */
using EffectFunction = void (*)(FxEnv &env);

//--------------------------------------------------------------------------------------------------

/// Effect class to act as adapter for new style effect-functions into the class-based backend.
class EffectFunctionWrapper : public EffectBase
{
public:
  EffectFunctionWrapper(FxSetup &fxs, EffectFunction fxFct) : EffectBase{fxs}, _fxFct{fxFct} {}

private:
  void showEffect(FxEnv &env) override { _fxFct(env); }
  EffectFunction _fxFct;
};

/// Effect class to act as adapter for the existing mode-functions into the class-based backend.
class ModeFunctionWrapper : public EffectBase
{
public:
  ModeFunctionWrapper(FxSetup &fxs, ModeFunction modeFct) : EffectBase{fxs}, _modeFct{modeFct} {}

private:
  void showEffect(FxEnv &env) override { env.setFrametime(_modeFct()); }
  ModeFunction _modeFct;
};

//--------------------------------------------------------------------------------------------------

/** "Pseudo" mode-function for class-based effects (to be registered at the WLED framework).
 * @tparam FX_CLASS Class type of concrete effect implementation. Must be a child of EffectBase.
 * @see addEffectClass()
 * @note This mode-function is abused as factory for the effect class. It does \e not render the
 * effect on the Segment, as the "normal" mode functions do.
 */
template <class FX_CLASS>
uint16_t mode_EffectClass()
{
  SEGMENT.createEffect<FX_CLASS>(strip.now);
  return 0;
}

/** "Pseudo" mode-function to use a new style effect-function via wrapper as class-based effect.
 * @tparam FX_FCT The new style effect-function.
 * @see addEffectFunction()
 */
template <EffectFunction FX_FCT> // Yes, the specific function is committed as template argument!
uint16_t mode_EffectFunctionWrapper()
{
  SEGMENT.createEffect<EffectFunctionWrapper>(strip.now, FX_FCT);
  return 0;
}

/** "Pseudo" mode-function to use an existing mode-function via wrapper as class-based effect.
 * @tparam MODE_FCT The already existing effect's mode-function.
 * @see addNodeFunction()
 */
template <ModeFunction MODE_FCT> // Yes, the specific function is committed as template argument!
uint16_t mode_ModeFunctionWrapper()
{
  SEGMENT.createEffect<ModeFunctionWrapper>(strip.now, MODE_FCT);
  return 0;
}

//--------------------------------------------------------------------------------------------------

/** Register a class-based effect at the WLED framework.
 * @tparam FX_CLASS Class type of concrete effect implementation. Must be a child of EffectBase.
 * @param wled WS2812FX instance representing the WLED framework - a.k.a. \c strip
 * @return The actual ID that is assigned to the effect, or 255 on failure.
 */
template <class FX_CLASS>
uint8_t addEffectClass(WS2812FX &wled, uint8_t FX_id, const char *FX_data)
{
  return wled.addEffect(FX_id, &mode_EffectClass<FX_CLASS>, FX_data);
}

/** Register a new style effect-function via class-based effect at the WLED framework.
 * @tparam FX_FCT The new style effect-function
 * @param wled WS2812FX instance representing the WLED framework - a.k.a. \c strip
 * @return The actual ID that is assigned to the effect, or 255 on failure.
 */
template <EffectFunction FX_FCT> // Yes, the concrete function is committed as template argument!
uint8_t addEffectFunction(WS2812FX &wled, uint8_t FX_id, const char *FX_data)
{
  return wled.addEffect(FX_id, &mode_EffectFunctionWrapper<FX_FCT>, FX_data);
  return 0;
}

/** Register an existing mode-function via class-based effect at the WLED framework.
 * @tparam MODE_FCT The existing mode-function
 * @param wled WS2812FX instance representing the WLED framework - a.k.a. \c strip
 * @return The actual ID that is assigned to the effect, or 255 on failure.
 */
template <ModeFunction MODE_FCT> // Yes, the concrete function is committed as template argument!
uint8_t addModeFunction(WS2812FX &wled, uint8_t FX_id, const char *FX_data)
{
  return wled.addEffect(FX_id, &mode_ModeFunctionWrapper<MODE_FCT>, FX_data);
  return 0;
}

//--------------------------------------------------------------------------------------------------

void addEffectScratchpad(WS2812FX &wled);

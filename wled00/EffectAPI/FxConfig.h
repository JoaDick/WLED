/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * @file This file provides the interface for reading UI settings.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "FX.h"
#include "PxColor.h"

//--------------------------------------------------------------------------------------------------

/** Interface for retrieving the effect's user configuration settings (from the UI).
 * Example metadata-string (as template for your convenience; with palette and without flags):
 * \c "MyEffect@speed,intensity,custom1,custom2,custom3,check1,check2,check3;fx,bg,cs;!;;sx=98,ix=76,c1=54,c2=32,c3=10,o1=1,o2=1,o3=1,pal=11"
 * @see https://kno.wled.ge/interfaces/json-api/#effect-metadata
 */
class FxConfig
{
public:
  // no general copy & move - this class is intended to be passed as reference to other functions
  FxConfig(FxConfig &&) = delete;
  FxConfig &operator=(const FxConfig &) = delete;
  FxConfig &operator=(FxConfig &&) = delete;

  // ----- slider -----

  /** Get current setting of the 'Speed" slider (with Stopwatch icon).
   * metadata-string: \c sx=0-255
   * @note Effect implementations shall use this instead of \c SEGMENT.speed
   */
  uint8_t speed() const { return _seg->speed; }

  /** Get current setting of the 'Intensity" slider (with Fire icon).
   * metadata-string: \c ix=0-255
   * @note Effect implementations shall use this instead of \c SEGMENT.intensity
   */
  uint8_t intensity() const { return _seg->intensity; }

  /** Get current setting of custom slider 1 (with Star icon).
   * metadata-string: \c c1=0-255
   * @note Effect implementations shall use this instead of \c SEGMENT.custom1
   */
  uint8_t custom1() const { return _seg->custom1; }

  /** Get current setting of custom slider 2 (with Gear icon).
   * metadata-string: \c c2=0-255
   * @note Effect implementations shall use this instead of \c SEGMENT.custom2
   */
  uint8_t custom2() const { return _seg->custom2; }

  /** Get current setting of custom slider 3 (with Eye icon; reduced range 0-31).
   * metadata-string: \c c3=0-31
   * @note Effect implementations shall use this instead of \c SEGMENT.custom3
   */
  uint8_t custom3_reduced() const { return _seg->custom3; }

  /// Current setting of custom slider 3 (with Eye icon; upscaled to almost full range 0-248).
  uint8_t custom3() const { return custom3_reduced() << 3; }

  // ----- checkbox -----

  /** Get current setting of checkbox 1 (with Palette icon).
   * metadata-string: \c o1=0|1
   * @note Effect implementations shall use this instead of \c SEGMENT.check1
   */
  bool check1() const { return _seg->check1; }

  /** Get current setting of checkbox 2 (with Overlay icon).
   * metadata-string: \c o2=0|1
   * @note Effect implementations shall use this instead of \c SEGMENT.check2
   */
  bool check2() const { return _seg->check2; }

  /** Get current setting of checkbox 3 (with Heart icon).
   * metadata-string: \c o3=0|1
   * @note Effect implementations shall use this instead of \c SEGMENT.check3
   */
  bool check3() const { return _seg->check3; }

  // ----- color -----

  /** Get currently selected effect/foreground color.
   * @note Effect implementations shall use this instead of \c SEGCOLOR(0)
   */
  PxColor fxColor() const { return color(0); }

  /** Get currently selected background color.
   * @note Effect implementations shall use this instead of \c SEGCOLOR(1)
   */
  PxColor bgColor() const { return color(1); }

  /** Get currently selected extra color.
   * @note Effect implementations shall use this instead of \c SEGCOLOR(2)
   */
  PxColor csColor() const { return color(2); }

  /** Get the desired color \a x
   * 0=fg / 1=bg / 2=aux / other=black
   * @note Effect implementations shall use this instead of \c SEGCOLOR(n)
   */
  PxColor color(unsigned x) const { return _seg->getCurrentColor(x); }

  /** Get number of currently selected color palette.
   * metadata-string: \c pal=0-255
   * See palettes.cpp for palette numbers (or popup in UI), e.g.
   * -  0 = Default
   * -  1 = Random Cycle
   * -  2 = Color 1
   * -  3 = Colors 1&2
   * -  4 = Color Gradient
   * -  5 = Colors only
   * -  6 = Party
   * - 11 = Rainbow
   * @note Effect implementations shall use this instead of \c SEGMENT.palette
   */
  uint8_t paletteNr() const { return _seg->palette; }

  /** Get the "Palette wrapping" setting from the "LED Preferences" page in the UI.
   * - 0 = Linear (wrap when moving)
   * - 1 = Linear (always wrap)
   * - 2 = Linear (never wrap)
   * - 3 = None (not recommended)
   */
  uint8_t paletteBlend() const;

private:
  friend class FxEnv;
  FxConfig(const FxConfig &) = default;
  explicit FxConfig(const Segment &seg) : _seg(&seg) {}
  const Segment *_seg;
};

//--------------------------------------------------------------------------------------------------

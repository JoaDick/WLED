/**
 * Some utilities for making WLED effect implementations easier.
 */

#pragma once

#include "wled.h"
#include "FX.h"
#include "fcn_declare.h"
#include <array>

//--------------------------------------------------------------------------------------------------

/// Datatype of an Effect-ID.
using EffectID = std::uint8_t;

/// Special Effect-ID value that lets WLED decide which ID to use eventually for that effect.
constexpr EffectID AutoSelectID = 255;

/** Generic pixel color.
 * Supports implicit conversion from \c uint32_t (WW-RR-GG-BB) and FastLED's \c CRGB & \c CHSV
 * as well as implicit conversion to \c uint32_t and \c CRGB
 */
struct PxColor
{
  /// Default constructor - leaves the color uninitialized!
  PxColor() = default;

  /// Create from discrete R-G-B (-W) portions.
  PxColor(byte r, byte g, byte b, byte w = 0) : wrgb(RGBW32(r, g, b, w)) {}

  PxColor(uint32_t c) : wrgb(c) {}
  PxColor(CRGB c) : wrgb(c) {}
  PxColor(CHSV c) : wrgb(CRGB(c)) {}

  operator uint32_t() const { return wrgb; }
  operator CRGB() const { return wrgb; }

  /// The pixel's 32 bit color value (white - red - green - blue).
  uint32_t wrgb;

  /** Put more emphasis on the red'ish colors.
   * Can be used for the \c hue parameter of a \c CHSV color.
   */
  static uint8_t redShift(uint8_t hue)
  {
    return cos8(128 + hue / 2);
  }

  PxColor &operator=(PxColor other)
  {
    wrgb = other.wrgb;
    return *this;
  }
};

/** Internal setup data for the effects.
 * @note Not intended to be used by the effect implementations (because it's likely to be changed).
 * These shall just pass that argument to their base class constructor.
 */
class FxSetup
{
public:
  explicit FxSetup(Segment &s) : seg(s) {}
  Segment &seg;
};

//--------------------------------------------------------------------------------------------------

/** Convenience interface for the effect's user configuration settings (from the UI).
 * Use this facade as replacement for accessing the Segement's effect configuration related members
 * directly, e.g. \c speed() instead if \c SEGMENT.speed
 */
class FxConfig
{
public:
  /// Constructor; to be initialized with \c fxs
  explicit FxConfig(const FxSetup &fxs) : FxConfig(fxs.seg) {}

  /// Constructor; to be initialized with \c SEGMENT
  /// Whenever feasible, prefer the other constructor with FxSetup.
  explicit FxConfig(const Segment &seg) : _seg(seg) {}

  /// Current setting of the 'Speed" slider (with Clock icon) -- \c SEGMENT.speed
  uint8_t speed() const { return _seg.speed; }

  /// Current setting of the 'Intensity" slider (with Fire icon) -- \c SEGMENT.intensity
  uint8_t intensity() const { return _seg.intensity; }

  /// Current setting of custom slider 1 (with Star icon) -- \c SEGMENT.custom1
  uint8_t custom1() const { return _seg.custom1; }

  /// Current setting of custom slider 2 (with Gear icon) -- \c SEGMENT.custom2
  uint8_t custom2() const { return _seg.custom2; }

  /// Current setting of custom slider 3 (with Eye icon; reduced range 0-31) -- \c SEGMENT.custom3
  uint8_t custom3() const { return _seg.custom3; }

  /// Current setting of checkbox 1 (with Palette icon) -- \c SEGMENT.check1
  bool check1() const { return _seg.check1; }

  /// Current setting of checkbox 2 (with Overlay icon) -- \c SEGMENT.check2
  bool check2() const { return _seg.check2; }

  /// Current setting of checkbox 3 (with Heart icon) -- \c SEGMENT.check3
  bool check3() const { return _seg.check3; }

  /// Get current effect/foreground color -- \c SEGCOLOR(0)
  PxColor fxColor() const { return color(0); }

  /// Get current background color -- \c SEGCOLOR(1)
  PxColor bgColor() const { return color(1); }

  /// Get current extra color -- \c SEGCOLOR(2)
  PxColor auxColor() const { return color(2); }

  /// Get the desired color \a x -- 'SEGCOLOR(x)'
  PxColor color(unsigned x) const { return _seg.getCurrentColor(x); }

  /// DRAFT
  uint8_t paletteIndex() const { return _seg.palette; }

  /// Get currently selected color palette -- \c SEGPALETTE
  const CRGBPalette16 &palette() { return _seg.getCurrentPalette(); }

  /** Get a single color from the currently selected palette.
   * @param i Palette Index (if mapping is true, the full palette will be as long as the Segment, if false then 255). Will wrap around automatically.
   * @param mapping if true, LED position in segment is considered for color
   * @param wrap FastLED palettes will usually wrap back to the start smoothly. Set false to get a hard edge
   * @param mcol If the default palette 0 is selected, return the standard color 0, 1 or 2 instead. If >2, Party palette is used instead
   * @param pbri Value to scale the brightness of the returned color by. Default is 255. (no scaling)
   */
  PxColor color_from_palette(uint16_t i, bool mapping, bool wrap, uint8_t mcol, uint8_t pbri = 255) const
  {
    return _seg.color_from_palette(i, mapping, wrap, mcol, pbri);
  }

private:
  const Segment &_seg;
};

/** Convenience interface for   TBD...
 * Use this facade as replacement for accessing the Segement's custom variables directly, e.g.
 * \c step() instead if \c SEGENV.step
 */
class SegEnv
{
public:
  /// Constructor; to be initialized with \c fxs
  explicit SegEnv(const FxSetup &fxs) : _seg(fxs.seg) {}

  /// Constructor; to be initialized with \c SEGMENT
  /// Whenever feasible, prefer the other constructor with FxSetup.
  explicit SegEnv(Segment &seg) : _seg(seg) {}

  /// Call counter (starts with 0 and is incremented by one with every frame) -- \c SEGENV.call
  uint32_t call() const { return _seg.call; }

  /// Custom variable -- \c SEGENV.step
  uint32_t &step() { return _seg.step; }

  /// Custom variable -- \c SEGENV.aux0
  uint16_t &aux0() { return _seg.aux0; }

  /// Custom variable -- \c SEGENV.aux1
  uint16_t &aux1() { return _seg.aux1; }

  /** TBD
   * ...
   * @tparam FX_DATA Type of custom effect data
   * @param dataPtr Pointer to effect data (which shall be redirected)
   * @retval true Success; \a dataPtr is now pointing to a valid instance of \a FX_DATA
   * @retval false Allocation failed; do \e not use \a dataPtr
   */
  template <typename FX_DATA>
  bool getFxData(FX_DATA *&dataPtr)
  {
    if (!_seg.allocateData(sizeof(FX_DATA)))
    {
      dataPtr = nullptr;
      return false;
    }
    // hopefully the alignment matches...
    dataPtr = static_cast<FX_DATA *>(static_cast<void *>(_seg.data));
    if (call() == 0)
    {
      // this 'placement new' calls the constructor
      dataPtr = new (dataPtr) FX_DATA;
      // *dataPtr = FX_DATA();
    }
    return true;
  }

  // DRAFT
  template <typename FX_DATA>
  bool getFxDataArray(FX_DATA *&dataPtr, uint32_t arrayLength)
  {
    if (!_seg.allocateData(sizeof(FX_DATA) * arrayLength))
    {
      dataPtr = nullptr;
      return false;
    }
    // hopefully the alignment matches...
    dataPtr = static_cast<FX_DATA *>(static_cast<void *>(_seg.data));
    if (call() == 0)
    {
      // this 'placement new' calls the constructors
      dataPtr = new (dataPtr) FX_DATA[arrayLength];
      // for (size_t i = 0; i < arrayLength; ++i)
      // {
      //   dataPtr[i] = FX_DATA();
      // }
    }
    return true;
  }

private:
  Segment &_seg;
};

//--------------------------------------------------------------------------------------------------

/** Base class for WLED effects.
 * All class-based WLED effects must derive from this class. This requires them to override the
 * \c showEffect() method, which is the replacement for a \c mode_XXX() function. All fancy pixel
 * magic that is rendered on the LED segment shall be done there.
 * @note Be aware that ... TBD
 */
class EffectRunner
{
public:
  // no copy & move
  EffectRunner(const EffectRunner &) = delete;
  EffectRunner(EffectRunner &&) = delete;
  EffectRunner &operator=(const EffectRunner &) = delete;
  EffectRunner &operator=(EffectRunner &&) = delete;

  /** Effect's mode function (to be registered at the WLED framework).
   * @tparam EFFECT_TYPE Class type of user effect implementation. Must be a child of EffectRunner.
   * @see addEffectRunner()
   */
  template <class EFFECT_TYPE>
  static uint16_t mode_function()
  {
    // extern WS2812FX strip;
    FxSetup fxs{strip._segments[strip.getCurrSegmentId()]};
    EFFECT_TYPE effect(fxs);
    return effect.renderEffect(strip.now, strip.getFrameTime());
  }

  /** Render the WLED effect.
   * Currently only internally used; not relevant for custom effect implementations.
   * @param now The current timestamp (a.k.a. \c strip.now ).
   * @param defaultFrametime The effect's default frametime (a.k.a \c FRAMETIME ).
   */
  uint16_t renderEffect(uint32_t now, uint16_t defaultFrametime)
  {
    if (segenv.call() == 0)
    {
      initEffect(now);
    }
    const uint16_t frametime = showEffect(now);
    return frametime ? frametime : defaultFrametime;
  }

protected:
  ~EffectRunner() = default;

  /// Constructor; child classes shall just pass \a fxs to their base class.
  explicit EffectRunner(FxSetup &fxs)
      : seg(fxs.seg), seglen(seg.vLength()), segenv(seg), config(seg) {}

  /** Rendering function of the WLED effect.
   * Must be implemented by all child classes to show their specific animation on the Segment.
   * @param now The current timestamp (a.k.a. \c strip.now ).
   * @return The Effect's frametime (in ms), or 0 to use default frametime (a.k.a \c FRAMETIME ).
   */
  virtual uint16_t showEffect(uint32_t now) = 0;

  /** Initialization function of the WLED effect.
   * This method is called exactly once right before the first call to showEffect()
   * Implementation in child classes is optional; can be used as replacement for
   * \code
   * if (SEGENV.call == 0) {
   *   // ... effect specific one-time initialization stuff ...
   * }
   * \endcode
   * @param now The current timestamp (a.k.a. \c strip.now ).
   */
  virtual void initEffect(uint32_t now) {}

  /// Use this as replacement for \c SEGMENT in effect implementations.
  Segment &seg;

  /// Use this as replacement for \c SEGLEN in effect implementations.
  const uint16_t seglen;

  /// Use this as replacement for \c SEGENV in effect implementations.
  SegEnv segenv;

  /** Use this facade as replacement for reading the Segement's effect configuration directly.
   * For example, replace \c SEGMENT.speed with \c config.speed()
   */
  const FxConfig config;

  /** Fallback rendering function.
   * Can be called as fallback by any child's showEffect() method, when it cannot render its own
   * stuff; e.g. when something like allocating additional effect memory went wrong.
   */
  uint16_t showFallbackEffect(uint32_t now);
};

/** Register an EffectRunner implementation at the WLED framework.
 * For format of \a FX_data see https://kno.wled.ge/interfaces/json-api/#effect-metadata
 * @tparam EFFECT_TYPE Class type of the effect implementation. Must be a child of EffectRunner.
 * @param wled WS2812FX instance, representing the WLED framework (a.k.a. \c strip )
 * @return The actual id used for the effect, or 255 if the add failed.
 */
template <class EFFECT_TYPE>
uint8_t addEffectRunner(WS2812FX &wled, uint8_t FX_id = EFFECT_TYPE::FX_id, const char *FX_data = EFFECT_TYPE::FX_data)
{
  return wled.addEffect(FX_id, &EffectRunner::mode_function<EFFECT_TYPE>, FX_data);
}

//--------------------------------------------------------------------------------------------------

/** AudioReactive Usermod Data (as facade for the handmade data conversions).
 * This helper is a code manifestation of the textual description about how AudioReactive's generic
 * Usermod Data shall be dissected & converted.
 * The neat thing about this facade is that it doesn't introduce any runtime overhead :-)
 */
class AudioReactiveUmData
{
  UM_Exchange_Data *um_data;

public:
  // no copy & move - this class is intended to be passed as reference to other functions
  AudioReactiveUmData(const AudioReactiveUmData &) = delete;
  AudioReactiveUmData(AudioReactiveUmData &&) = delete;
  AudioReactiveUmData &operator=(const AudioReactiveUmData &) = delete;
  AudioReactiveUmData &operator=(AudioReactiveUmData &&) = delete;

  /// Constructor; to be initialized with \c fxs
  explicit AudioReactiveUmData(FxSetup &fxs) : AudioReactiveUmData(fxs.seg) {}

  /// Constructor; to be initialized with \c SEGMENT
  /// Whenever feasible, prefer the other constructor with FxSetup.
  explicit AudioReactiveUmData(Segment &seg)
  {
    if (!UsermodManager::getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE))
    {
      um_data = simulateSound(seg.soundSim); // support for no audio
    }
  }

  // either sampleAvg or sampleAgc depending on soundAgc; smoothed sample
  // Range: 0.0 .. 255.0
  float volumeSmth() const { return *static_cast<const float *>(um_data->u_data[0]); }

  /// @brief As volumeSmth() but with a normalized range of 0.0 .. 1.0
  float n_volumeSmth() const { return volumeSmth() / 255.0; }

  // either sampleRaw or rawSampleAgc depending on soundAgc
  // Range: 0 .. 255
  uint16_t volumeRaw() const { return *static_cast<const uint16_t *>(um_data->u_data[1]); }

  // number of FFT bins provided by fftResult()
  static constexpr uint8_t fftResult_size() { return 16; /* NUM_GEQ_CHANNELS */ }

  // Our calculated freq. channel result table to be used by effects
  const uint8_t *fftResult() const { return static_cast<const uint8_t *>(um_data->u_data[2]); }
  uint8_t fftResult(uint8_t index) const { return index < fftResult_size() ? fftResult()[index] : 0; }

  // Boolean flag for peak - used in effects. Responding routine may reset this flag. Auto-reset after strip.getFrameTime()
  bool samplePeak() const { return *static_cast<const bool *>(um_data->u_data[3]); }

  // FFT: strongest (peak) frequency
  float FFT_MajorPeak() const { return *static_cast<const float *>(um_data->u_data[4]); }

  // FFT_Magnitude, scaled by multAgc
  float my_magnitude() const { return *static_cast<const float *>(um_data->u_data[5]); }

  // (was 10) Reasonable value for constant volume for 'peak detector', as it won't always trigger  (deprecated)
  // requires UI element (SEGMENT.customX?), changes source element
  // This is a setter!
  uint8_t &maxVol() { return *static_cast<uint8_t *>(um_data->u_data[6]); }

  // Used to select the bin for FFT based beat detection  (deprecated)
  // requires UI element (SEGMENT.customX?), changes source element
  // This is a setter!
  uint8_t &binNum() { return *static_cast<uint8_t *>(um_data->u_data[7]); }
};

//--------------------------------------------------------------------------------------------------

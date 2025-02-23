/**
 * Utilities for making WLED effect implementations easier.
 *
 * All of these classes are very lightweight abstractions on top of WLED's API, mainly for the
 * \c Segment class to simplify its overwhelming interface. Most of these classes contain not even
 * a handful of integers and references. The majority of their methods are inline oneliners to give
 * the compiler maximum opportunity for optimizations, including even de-virtualization.
 *
 * The functionality of the \c Segment class is segregated into separated and well documented
 * interfaces, depending on the particular purpose:
 * - \c FxConfig - effect configuration settings from the UI, like speed, intensity, ...
 * - \c SegEnv - for accessing effect internal data, which shall be preserved between frames.
 * - \c AudioReactiveUmData - read the AudioReactive Usermod output without complicated typecasts.
 *
 * Plus some additional support classes:
 * - \c PxColor - as general abstraction for manipulating the color of an LED in many ways.
 * - \c EffectRunner - as base class for implementing class based, object-oriented WLED effects.
 * - \c FxSetup - internal structure for initializing some of the other helper classes.
 *
 * And finally the good stuff for drawing the effects:
 * - \c PxArray & related methods - generic interface for making 1D effects.
 * - \c ArrayPixelProxy - a proxy object for manipulating a specific pixel of a \c PxArray
 * - \c WledPxArray - to access WLED's \c Segment class through the \c PxArray interface.
 * - \c PxMatrix & related methods - generic interface for making 2D effects.
 * - \c PxMatrixRow - to access a specific row of a \c PxMatrix through the \c PxArray interface.
 * - \c PxMatrixColumn - to access a specific column of a \c PxMatrix through the \c PxArray interface.
 * - \c WledPxMatrix - to access WLED's \c Segment class through the \c PxMatrix interface.
 *
 * @author Joachim Dick, 2025
 */

#pragma once

#include "wled.h"
#include "FX.h"
#include "fcn_declare.h"
#include "FXutilsCore.h"
#include "FXutils1D.h"
#include "FXutils2D.h"
#include "../usermods/EffectProfiler/EffectProfilerTrigger.h"

//--------------------------------------------------------------------------------------------------

/// Datatype of an Effect-ID.
using EffectID = std::uint8_t;

/// Special Effect-ID value that lets WLED decide which ID to use eventually for that effect.
constexpr EffectID AutoSelectID = 255;

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

/** WLED pixel array for rendering effects (as drawing facade for a Segment).
 * @see PxArray
 */
class WledPxArray final : public PxArray
{
public:
  /// Constructor; to be initialized with \c fxs
  explicit WledPxArray(FxSetup &fxs) : WledPxArray(fxs.seg) {}

  /// Constructor; to be initialized with \c SEGMENT
  explicit WledPxArray(Segment &seg) : PxArray(seg.vLength()), _seg(seg) {}

  /** Blur the pixels of the array.
   * @note: For \a blur_amount > 215 this function does not work properly (creates alternating pattern)
   */
  void blur(uint8_t blur_amount, bool smear = false) { _seg.blur(blur_amount, smear); }

  /// Backdoor: Get the underlying Segment.
  Segment &getSegment() { return _seg; }

  /// Convenience backdoor: Access the underlying Segment's members via arrow operator.
  Segment *operator->() { return &_seg; }

  [[deprecated("use fadeToBackground() instead")]] void fade_out(uint8_t rate) { _seg.fade_out(rate); }

private:
  PxColor do_getBackgroundColor() const { return _seg.getCurrentColor(1); }
  PxColor do_getPixelColor(AIndex pos) const { return _seg.getPixelColor(pos); }
  void do_setPixelColor(AIndex pos, PxColor color) { _seg.setPixelColor(pos, color.wrgb); }
  void do_fade(AIndex pos, PxColor color) { _seg.setPixelColor(pos, color.wrgb); }
#if (1)
  void do_fadeToBlackBy(uint8_t fadeBy) override { _seg.fadeToBlackBy(fadeBy); }
#else
  void do_fadeToBlackBy(uint8_t fadeBy) override
  {
    EffectProfilerTrigger profiler;
    if (profiler.mustRun_A())
    {
      profiler.startSingle_A();
      PxArray::do_fadeToBlackBy(fadeBy);
      profiler.stop();
    }
    else
    {
      profiler.startSingle_B();
      _seg.fadeToBlackBy(fadeBy);
      profiler.stop();
    }
  }
#endif

private:
  Segment &_seg;
};

//--------------------------------------------------------------------------------------------------

/** TBD
 * ...
 */
class WledPxMatrix final : public PxMatrix
{
public:
  /// Constructor; to be initialized with \c fxs
  explicit WledPxMatrix(FxSetup &fxs) : WledPxMatrix(fxs.seg) {}

  /// Constructor; to be initialized with \c SEGMENT
  explicit WledPxMatrix(Segment &seg) : PxMatrix(seg.vWidth(), seg.vHeight()), _seg(seg) {}

  /// 2D-blur the pixels of the matrix (can be asymmetrical).
  void blur(uint8_t blurAmountX, uint8_t blurAmountY, bool smear = false) { _seg.blur2D(blurAmountX, blurAmountY, smear); }

  /// Backdoor: Get the underlying Segment.
  Segment &getSegment() { return _seg; }

  /// Convenience backdoor: Access the underlying Segment's members via arrow operator.
  Segment *operator->() { return &_seg; }

  [[deprecated("use fadeToBackground() instead")]] void fade_out(uint8_t rate) { _seg.fade_out(rate); }

private:
  PxColor do_getBackgroundColor() const { return _seg.getCurrentColor(1); }
  PxColor do_getPixelColor(const APoint &pos) const { return _seg.getPixelColorXY(pos.x, pos.y); }
  void do_setPixelColor(const APoint &pos, PxColor color) { _seg.setPixelColorXY(pos.x, pos.y, color.wrgb); }
#if (1)
  void do_fadeToBlackBy(uint8_t fadeBy) override { _seg.fadeToBlackBy(fadeBy); }
#else
  void do_fadeToBlackBy(uint8_t fadeBy) override
  {
    EffectProfilerTrigger profiler;
    if (profiler.mustRun_A())
    {
      profiler.startSingle_A();
      PxMatrix::do_fadeToBlackBy(fadeBy);
      profiler.stop();
    }
    else
    {
      profiler.startSingle_B();
      _seg.fadeToBlackBy(fadeBy);
      profiler.stop();
    }
  }
#endif

private:
  Segment &_seg;
};

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

  /** Smoothed sample.
   * Range: 0.0 .. 255.0
   */
  float volumeSmth() const { return raw_volumeSmth(); }

  /// @brief As volumeSmth() but with a normalized range of 0.0 .. 1.0
  float n_volumeSmth() const { return volumeSmth() / 255.0; }

  /** Current sample.
   * Range: 0 .. 255
   */
  uint16_t volumeRaw() const { return raw_volumeRaw(); }

  /// @brief As volumeRaw() but with a normalized range of 0.0 .. 1.0
  float n_volumeRaw() const { return volumeRaw() / 255.0; }

  /// Result of the FFT bin with the given \a index
  uint8_t fftResult(uint8_t index) const { return index < fftResult_size() ? fftResult()[index] : 0; }

  /// Our calculated frequency channel result table to be used by effects (FFT bins).
  const uint8_t *fftResult() const { return static_cast<const uint8_t *>(um_data->u_data[2]); }

  /// Number of FFT bins provided by fftResult()
  static constexpr uint8_t fftResult_size() { return 16; /* NUM_GEQ_CHANNELS */ }

  /** 0 no peak; >= 1 peak detected.
   * In future, this will also provide peak Magnitude.
   */
  uint8_t samplePeak() const { return raw_samplePeak(); }

  /// Frequency (Hz) of largest FFT result.
  float fftMajorPeak() const { return raw_FFT_MajorPeak(); }

  // Largest FFT result from a single run (raw value, can go up to 4096).
  float fftMagnitude() const { return raw_my_magnitude(); }

  /** Setter for reasonable value for constant volume for 'peak detector', as it won't always trigger.
   * Assign your desired value to the returned reference.
   * @note This method is deprecated.
   */
  uint8_t &maxVol() { return raw_maxVol(); }

  /** Setter to select the bin for FFT based beat detection.
   * Assign your desired value to the returned reference.
   * @note This method is deprecated.
   */
  uint8_t &binNum() { return raw_binNum(); }

private:
  // Copies of all the comments and other breadcrumbs that are spread throughout audio_reactive.h

  // static uint8_t soundAgc = 0;    // Automagic gain control: 0 - none, 1 - normal, 2 - vivid, 3 - lazy (config value)
  // static float sampleAvg = 0.0f;  // Smoothed Average sample - sampleAvg < 1 means "quiet" (simple noise gate)
  // static float sampleAgc = 0.0f;  // Smoothed AGC sample
  // int16_t sampleRaw = 0;          // Current sample. Must only be updated ONCE!!! (amplified mic value by sampleGain and inputLevel)
  // int16_t rawSampleAgc = 0;       // not smoothed AGC sample

  // float volumeSmth = 0.0f;                           // either sampleAvg or sampleAgc depending on soundAgc; smoothed sample
  // um_data->u_data[0] = &volumeSmth;                  //*used (New)
  // um_data->u_type[0] = UMT_FLOAT;
  // volumeSmth = *(float *)um_data->u_data[0];
  // [WLED_MM SyncPacket::] float sampleSmth;           //  04 Bytes  offset 12 - either "sampleAvg" or "sampleAgc" depending on soundAgc setting
  float raw_volumeSmth() const { return *static_cast<const float *>(um_data->u_data[0]); }

  // int16_t volumeRaw = 0;                             // either sampleRaw or rawSampleAgc depending on soundAgc
  // um_data->u_data[1] = &volumeRaw;                   // used (New)
  // um_data->u_type[1] = UMT_UINT16;
  // volumeRaw = *(float *)um_data->u_data[1];
  // [WLED_MM SyncPacket::] float sampleRaw;            //  04 Bytes  offset 8  - either "sampleRaw" or "rawSampleAgc" depending on soundAgc setting
  uint16_t raw_volumeRaw() const { return *static_cast<const uint16_t *>(um_data->u_data[1]); }

  // static uint8_t fftResult[NUM_GEQ_CHANNELS] = {0};  // Our calculated freq. channel result table to be used by effects
  // um_data->u_data[2] = fftResult;                    //*used (Blurz, DJ Light, Noisemove, GEQ_base, 2D Funky Plank, Akemi)
  // um_data->u_type[2] = UMT_BYTE_ARR;
  // uint8_t *fftResult = nullptr;
  // fftResult = (uint8_t *)um_data->u_data[2];
  // [WLED_MM SyncPacket::] uint8_t fftResult[16];      //  16 Bytes  offset 18 - 16 GEQ channels, each channel has one byte (uint8_t)
  const uint8_t *raw_fftResult() const { return static_cast<const uint8_t *>(um_data->u_data[2]); }

  // static bool samplePeak = false;                    // Boolean flag for peak - used in effects. Responding routine may reset this flag. Auto-reset after strip.getFrameTime()
  // um_data->u_data[3] = &samplePeak;                  //*used (Puddlepeak, Ripplepeak, Waterfall)
  // um_data->u_type[3] = UMT_BYTE;
  // bool samplePeak = false;
  // samplePeak = *(uint8_t *)um_data->u_data[3];
  // [WLED_MM SyncPacket::] uint8_t samplePeak;         //  01 Bytes  offset 16 - 0 no peak; >=1 peak detected. In future, this will also provide peak Magnitude
  uint8_t raw_samplePeak() const { return *static_cast<const uint8_t *>(um_data->u_data[3]); }

  // static float FFT_MajorPeak = 1.0f;                 // FFT: strongest (peak) frequency
  // um_data->u_data[4] = &FFT_MajorPeak;               //*used (Ripplepeak, Freqmap, Freqmatrix, Freqpixels, Freqwave, Gravfreq, Rocktaves, Waterfall)
  // um_data->u_type[4] = UMT_FLOAT;
  // float FFT_MajorPeak = 1.0;
  // FFT_MajorPeak = *(float *)um_data->u_data[4];
  // [WLED_MM SyncPacket::] float FFT_MajorPeak;        //  04 Bytes  offset 40 - frequency (Hz) of largest FFT result
  float raw_FFT_MajorPeak() const { return *static_cast<const float *>(um_data->u_data[4]); }

  // float my_magnitude = 0.0f;                         // FFT_Magnitude, scaled by multAgc
  // um_data->u_data[5] = &my_magnitude;                // used (New)
  // um_data->u_type[5] = UMT_FLOAT;
  // my_magnitude = *(float *)um_data->u_data[5];
  // [WLED_MM SyncPacket::] float FFT_Magnitude;        //  04 Bytes  offset 36 - largest FFT result from a single run (raw value, can go up to 4096)
  float raw_my_magnitude() const { return *static_cast<const float *>(um_data->u_data[5]); }

  // static uint8_t maxVol = 31;                        // (was 10) Reasonable value for constant volume for 'peak detector', as it won't always trigger  (deprecated)
  // um_data->u_data[6] = &maxVol;                      // assigned in effect function from UI element!!! (Puddlepeak, Ripplepeak, Waterfall)
  // um_data->u_type[6] = UMT_BYTE;
  // uint8_t *maxVol = (uint8_t *)(&SEGENV.aux1 + 1);   // just in case assignment
  // maxVol = (uint8_t *)um_data->u_data[6];            // requires UI element (SEGMENT.customX?), changes source element
  uint8_t &raw_maxVol() { return *static_cast<uint8_t *>(um_data->u_data[6]); }

  // static uint8_t binNum = 8;                         // Used to select the bin for FFT based beat detection  (deprecated)
  // um_data->u_data[7] = &binNum;                      // assigned in effect function from UI element!!! (Puddlepeak, Ripplepeak, Waterfall)
  // um_data->u_type[7] = UMT_BYTE;
  // uint8_t *binNum = (uint8_t *)&SEGENV.aux1;         // just in case assignment
  // binNum = (uint8_t *)um_data->u_data[7];            // requires UI element (SEGMENT.customX?), changes source element
  uint8_t &raw_binNum() { return *static_cast<uint8_t *>(um_data->u_data[7]); }

#if (0) // new in WLED_MM
#ifdef ARDUINO_ARCH_ESP32
  // static float FFT_MajPeakSmth = 1.0f;               // FFT: (peak) frequency, smooth
  // um_data->u_data[8] = &FFT_MajPeakSmth;             // new
  // um_data->u_type[8] = UMT_FLOAT;
  // FFT_MajPeakSmth = *(float *)um_data->u_data[8];    // FFT Majorpeak smoothed
  float raw_FFT_MajorPeakSmth() const { return *static_cast<const float *>(um_data->u_data[8]); }
#else
  // static float FFT_MajorPeak = 1.0f;                 // FFT: strongest (peak) frequency
  // um_data->u_data[8] = &FFT_MajorPeak;               // substitute for 8266
  // um_data->u_type[8] = UMT_FLOAT;
#endif

  // float soundPressure = 0;                           // Sound Pressure estimation, based on microphone raw readings. 0 ->5db, 255 ->105db
  // um_data->u_data[9] = &soundPressure;               // used (New)
  // um_data->u_type[9] = UMT_FLOAT;
  // soundPressure = *(float *)um_data->u_data[9];      // sound pressure ( = logarithmic scale microphone input). Range 0...255
  // [WLED_MM SyncPacket::] uint8_t pressure[2];        //  02 Bytes, offset 6  - sound pressure as fixed point (8bit integer,  8bit fraction)
  float raw_soundPressure() const { return *static_cast<const float *>(um_data->u_data[9]); }

  // // TODO: probably best not used by receive nodes
  // static float agcSensitivity = 128;                 // AGC sensitivity estimation, based on agc gain (multAgc). calculated by getSensitivity(). range 0..255
  // um_data->u_data[10] = &agcSensitivity;             // used (New) - dummy value on 8266
  // um_data->u_type[10] = UMT_FLOAT;
  // agcSensitivity = *(float *)um_data->u_data[10];    // current AGC gain, scaled to 0...255. use "255.0f - agcSensitivity" to get MIC input level
  float raw_agcSensitivity() const { return *static_cast<const float *>(um_data->u_data[10]); }

  // static uint16_t zeroCrossingCount = 0;             // number of zero crossings in the current batch of 512 samples
  // um_data->u_data[11] = &zeroCrossingCount;          // for auto playlist usermod
  // um_data->u_type[11] = UMT_UINT16;
  // [WLED_MM SyncPacket::] uint16_t zeroCrossingCount; //  02 Bytes, offset 34 - number of zero crossings seen in 23ms
  uint16_t raw_zeroCrossingCount() const { return *static_cast<const uint16_t *>(um_data->u_data[11]); }

  // [WLED_MM SyncPacket::] uint8_t frameCounter;       //  01 Bytes  offset 17 - rolling counter to track duplicate/out of order packets
#endif
};

//--------------------------------------------------------------------------------------------------

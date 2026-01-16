/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * This headerfile provides all utilities for custom class-based WLED effect implementations.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include <type_traits>

#include "FX.h"
#include "FxUtils.h"
#include "PxColor.h"

//--------------------------------------------------------------------------------------------------
class EffectBase;
using RawEffectPtr = EffectBase *;
class SegEnv;
#ifndef WLED_DISABLE_PARTICLESYSTEM1D
class ParticleSystem1D;
#endif
#ifndef WLED_DISABLE_PARTICLESYSTEM2D
class ParticleSystem2D;
#endif

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

  /** Get current setting of the 'Speed" slider (with Clock icon).
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
  bool isFistFrame() const;

  // ----- rendering related methods -----

  /** Get the segment on which the effect shall be rendered.
   * @note Effect implementations shall use this instead of \c SEGMENT
   */
  Segment &seg() { return _pxArray.seg(); }

  /** Get the length of the segment.
   * @note Effect implementations shall use this instead of \c SEGLEN
   */
  uint16_t seglen() const { return _pxArray.size(); }

  /** Get the width of the segment.
   * @note Effect implementations shall use this instead of \c SEG_W
   */
  uint16_t segW() const { return _pxMatrix.sizeX(); }

  /** Get the height of the segment.
   * @note Effect implementations shall use this instead of \c SEG_H
   */
  uint16_t segH() const { return _pxMatrix.sizeY(); }

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
  SegEnv &segenv() { return *_segenv; }

  /** Get currently selected color palette.
   * @param env Effect runtime environment.
   * @note Effect implementations shall use this instead of \c SEGPALETTE
   */
  const CRGBPalette16 &currentPalette() { return seg().getCurrentPalette(); }

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
  FxEnv(Segment &seg, SegEnv &segenv, uint32_t now)
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
  bool updateSegment(Segment &seg, SegEnv &segenv);

  /** Store the given \a effect to be controlled.
   * @note Be aware that \a effect is not initialized yet, so don't use it here!
   */
  void storeEffect(RawEffectPtr effect) { _effect = effect; }

private:
  void updateTime(uint32_t now);

private:
  FxConfig _config;
  SegmentPxArray _pxArray;
  SegmentPxMatrix _pxMatrix;
  RawEffectPtr _effect = nullptr;
  SegEnv *_segenv = nullptr;
  uint32_t _now = 0;
  uint32_t _age = 0;
  uint32_t _deltaT = 0;
  uint16_t _frametime = 0;
  bool _is2D = false;
};

/** Pointer to a new style of free effect-function (as alternative for the existing mode-function).
 * The essential difference is that it gets an \c FxEnv as argument (for rendering), and doesn't
 * have a returnvalue. As a consequence (but only if really needed), a specific frametime has to be
 * announced via \a env.setFrametime()
 */
using EffectFunction = void (*)(FxEnv &env);

/** Get a color from the currently selected color palette.
 * @param env Effect runtime environment.
 * @param index Palette index: 0 = first color of the palette ... 255 = last color
 * @param brightness Brightness of the color.
 * @param blendType Color interpolation options for the palette:
 * - \c NOBLEND = No interpolation between palette entries (not recommended).
 * - \c LINEARBLEND = Linear interpolation between palette entries, with wrap-around from end to the beginning again.
 * - \c LINEARBLEND_NOWRAP = Linear interpolation between palette entries, but no wrap-around.
 */
inline PxColor paletteColor(FxEnv &env, uint8_t index, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND)
{
  return ColorFromPaletteWLED(env.currentPalette(), index, brightness, blendType);
}

/** Get a color based on a spectrum; either rainbow or from the currently selected palette.
 * When the \e Default palette (0) is selected in the UI, a rainbow color (based on HSV color model)
 * is returned. Otherwise, a color from the currently selected palette is returned.
 * @param env Effect runtime environment.
 * @param hue Rainbow's HSV hue value, or palette index.
 * @param vol Brightness of the color.
 * @param blendType Color interpolation options for the palette:
 * - \c NOBLEND = No interpolation between palette entries (not recommended).
 * - \c LINEARBLEND = Linear interpolation between palette entries, with wrap-around from end to the beginning again.
 * - \c LINEARBLEND_NOWRAP = Linear interpolation between palette entries, but no wrap-around.
 * @note Effect implementations may use this as alternative to \c color_wheel()
 * The difference to that function is that \a vol and \a blendType can be specified by the caller.
 */
inline PxColor rainbowColor(FxEnv &env, uint8_t hue, uint8_t vol = 255, TBlendType blendType = LINEARBLEND)
{
  if (env.ui().paletteNr())
    return paletteColor(env, hue, vol, blendType);
  uint32_t color;
  hsv2rgb(CHSV32(hue, 255, vol), color);
  return color;
}

/** Alias for compatibility with Segment::color_wheel()
 * Get a "rotating" color, based on the given \a pos
 * When the \e Default palette (0) is selected: \n
 * Rotates the color in HSV space, where \a pos is H (0 = 0deg ... 256 = 360deg) with S and V fixed
 * to 255. The colors are a transition red --> green --> blue --> back to red. \n
 * When another palette is selected: \n
 * Returns a color from that palette, where \a pos represents the palette index.
 * @param env Effect runtime environment.
 * @param pos Position in the color wheel.
 * @note Effect implementations shall use this instead of \c SEGMENT.color_wheel()
 */
inline PxColor color_wheel(FxEnv &env, uint8_t pos)
{
  return env.seg().color_wheel(pos);
}

/** Alias for compatibility with Segment::color_from_palette()
 * Get a single color from the currently selected color palette.
 * @param env Effect runtime environment.
 * @param i  Palette index; will wrap around automatically. See \a mapping for its range.
 * @param mapping  \c false = the range of \a i for a full palette cycle is 0 ... 255
 *                 \c true  = the range of \a i for a full palette cycle is 0 ... \c FxEnv::seglen()
 * @param moving  Color palettes can wrap back to the start smoothly.
 *                Set to \c true if you want that wrapping, e.g. when the effect uses a "moving" palette.
 *                Set to \c false to get a hard edge from end to start of the palette.
 * @param mcol  Only when the \e Default palette (0) is selected, return the standard color for 0 (fg), 1 (bg) or 2 (aux) instead.
 *              Ignored if this value is >2 or when another palette is selected.
 * @param pbri  Value to scale down the brightness of the returned color by. Default is 255, meaning full brightness.
 * @note Effect implementations shall use this instead of \c SEGMENT.color_from_palette()
 */
inline PxColor color_from_palette(FxEnv &env, uint16_t i, bool mapping, bool moving, uint8_t mcol, uint8_t pbri = 255)
{
  return env.seg().color_from_palette(i, mapping, moving, mcol, pbri);
}

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

/** Persistent effect data.
 * Effect implementations shall use this instead of \c SEGENV
 * @note This helper class emulates the same allocation functionality as the Segment class provides.
 * Nevertheless, try to avoid using that feature. Prefer implementing your own class-based effect
 * with member variables, so that allocation isn't needed at all.
 *
 * Dveleoper's note: This class re-implements the allocation stuff from the Segment class instead
 * of forwarding the method calls.
 * That decision was intentional: Although it is bad for code size, this simplifies possible future
 * refactorings of the Segment class, so that the allocation stuff can be eliminated there.
 */
class SegEnv
{
public:
  // no general copy & move - this class is intended to be passed as reference to other functions
  SegEnv(SegEnv &&) = delete;
  FxEnv &operator=(const SegEnv &) = delete;
  SegEnv &operator=(SegEnv &&) = delete;

  /** Call counter (starts with 0 and is incremented by one after every frame).
   * @note Effect implementations shall use this instead of \c SEGENV.call
   * The value of this counter cannot be manipulated by the effects.
   * If your effect has the need to start all over again, call \c reset() instead.
   */
  const uint32_t &call = _call;

  /** Custom "step" variable.
   * @note Effect implementations shall use this instead of \c SEGENV.step
   */
  uint32_t step = 0;

  /** Custom variable.
   * @note Effect implementations shall use this instead of \c SEGENV.aux0
   */
  uint16_t aux0 = 0;

  /** Custom variable.
   * @note Effect implementations shall use this instead of \c SEGENV.aux1
   */
  uint16_t aux1 = 0;

  /** EXPERIMENTAL
   * ...
   * The effect is marked as broken when the allocation failed, so its rendering function won't be
   * called anymore.
   * @tparam FX_DATA Type of custom effect data.
   * @param dataPtr Pointer to effect data (which will be redirected).
   * @retval \c true Success; \a dataPtr is now pointing to a valid instance of \a FX_DATA
   * @retval \c false Allocation failed; do \e not use \a dataPtr
   * @note Try to avoid using this method. Prefer implementing your own class-based effect with
   * member variables, so that allocation isn't needed at all.
   */
  template <typename FX_DATA>
  bool getFxData(FX_DATA *&dataPtr)
  {
    static_assert(std::is_default_constructible<FX_DATA>::value,
                  "Incompatible FX_DATA: must have a default constructor");
    // https://en.cppreference.com/w/cpp/language/destructor.html#Trivial_destructor
    static_assert(std::is_trivially_destructible<FX_DATA>::value,
                  "Incompatible FX_DATA: must not have a custom destructor");
    // https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable.html
    // https://en.cppreference.com/w/cpp/language/classes.html#Trivially_copyable_class
    static_assert(std::is_trivially_copyable<FX_DATA>::value,
                  "Incompatible FX_DATA: must be trivially copyable");

    const size_t size = sizeof(FX_DATA);
    // memory already allocated? --> just cast the pointer
    if (_dataSize >= size)
    {
      dataPtr = static_cast<FX_DATA *>(_data);
    }
    // must allocate memory
    else
    {
      if (!allocateData(size))
      {
        dataPtr = nullptr;
        return false;
      }

      dataPtr = new (_data) FX_DATA;
      if (dataPtr != _data)
      {
        // DEBUG_PRINTF_P(PSTR("SegEnv %p: Alignment failure [%p/%p]\n"), this, dataPtr, _data);
        onSegEnvAllocFailed();
        return false;
      }
    }

    return true;
  }

  /** EXPERIMENTAL
   * ...
   * The effect is marked as broken when the allocation failed, so its rendering function won't be
   * called anymore.
   * @tparam FX_DATA Type of custom effect data.
   * @param dataPtr Pointer to effect data array (which will be redirected).
   * @param arrayLength Number of elements in the array.
   * @retval \c true Success; \a dataPtr is now pointing to a valid array of \a FX_DATA
   * @retval \c false Allocation failed; do \e not use \a dataPtr
   * @note Try to avoid using this method. Prefer implementing your own class-based effect with
   * member variables, so that allocation isn't needed at all.
   */
  template <typename FX_DATA>
  bool getFxDataArray(FX_DATA *&dataPtr, uint32_t arrayLength)
  {
    static_assert(std::is_default_constructible<FX_DATA>::value,
                  "Incompatible FX_DATA: must have a default constructor");
    // https://en.cppreference.com/w/cpp/language/destructor.html#Trivial_destructor
    static_assert(std::is_trivially_destructible<FX_DATA>::value,
                  "Incompatible FX_DATA: must not have a custom destructor");
    // https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable.html
    // https://en.cppreference.com/w/cpp/language/classes.html#Trivially_copyable_class
    static_assert(std::is_trivially_copyable<FX_DATA>::value,
                  "Incompatible FX_DATA: must be trivially copyable");

    const size_t size = sizeof(FX_DATA) * arrayLength;
    // memory already allocated? --> just cast the pointer
    if (_dataSize >= size)
    {
      dataPtr = static_cast<FX_DATA *>(_data);
    }
    // must allocate memory
    else
    {
      if (!allocateData(size))
      {
        dataPtr = nullptr;
        return false;
      }

      dataPtr = new (_data) FX_DATA[arrayLength];
      if (dataPtr != _data)
      {
        // DEBUG_PRINTF_P(PSTR("SegEnv %p: Alignment failure [%p/%p]\n"), this, dataPtr, _data);
        onSegEnvAllocFailed();
        return false;
      }
    }

    return true;
  }

  /** Allocate raw effect data buffer (and set all bytes to 0).
   * The effect is marked as broken when the allocation failed, so its rendering function won't be
   * called anymore.
   * @retval \c true Success; data() will provide the allocated buffer.
   * @retval \c false Failed; the effect is broken now. The caller should return immediately.
   * @note Effect implementations shall use this instead of \c SEGENV.allocateData()
   * Nevertheless, try to avoid using this method. Prefer implementing your own class-based effect
   * with member variables, so that allocation isn't needed at all.
   */
  bool allocateData(size_t size);
  void deallocateData();

  /** Get pointer to the raw effect data.
   * Buffer must have been allocated before via \c allocateData()
   * @note Effect implementations shall use this instead of \c SEGENV.data
   * Tipp: Consider using \c getFxData() or \c getFxDataArray() instead of this method.
   */
  byte *data() { return static_cast<byte *>(_data); }

  /// Reset (and deallocate) all data - just as if it were the very first frame.
  void reset();

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

protected:
  SegEnv() = default;
  SegEnv(const SegEnv &other);
  ~SegEnv() { deallocateData(); }

  /// Call this method \e after every rendered frame.
  void next() { ++_call; }

  /** Call this method when the segment has changed.
   * @param seg The changed segment wo work on from now.
   */
  void updateSegment(Segment &seg);

  /** This method is called when an allocation has failed.
   * Child class shall mark the effect as broken.
   */
  virtual void onSegEnvAllocFailed() = 0;

private:
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
  static size_t _allDataSize;
  size_t _dataSize = 0;
  void *_data = nullptr;
#ifndef WLED_DISABLE_PARTICLESYSTEM1D
  ParticleSystem1D *_partSys_1D = nullptr;
#endif
#ifndef WLED_DISABLE_PARTICLESYSTEM2D
  ParticleSystem2D *_partSys_2D = nullptr;
#endif
  uint32_t _call = 0;
};

//--------------------------------------------------------------------------------------------------

/** Interface for classes that can generate colors based on a given index.
 * Like color palettes on steroids; just more versatile and extensible through custom implementations.
 */
class ColorSource
{
public:
  /** Get color at the given absolute \a index
   * One full range of the color source's spectrum is represented by \c 0<=index<size
   */
  PxColor get(AIndex index) { return do_getColor(index); }

  /** Get color at the given normalized \a index
   * One full range of the color source's spectrum is represented by \c 0.0<=index<=1.0
   */
  PxColor get_N(NIndex index) { return do_getColor(norm2abs(index, size)); }

  /** Size of the color source's spectrum (in pixels).
   * Higher values stretch the spectrum over a larger range for \c index, lower values squeeze it.
   */
  AIndex size;

protected:
  // no impact on child's copy & move policy
  ColorSource(const ColorSource &) = default;
  ColorSource(ColorSource &&) = default;
  ColorSource &operator=(const ColorSource &) = default;
  ColorSource &operator=(ColorSource &&) = default;
  ~ColorSource() = default;

  /** Constructor.
   * @param size Size of the color source's spectrum (in pixels).
   */
  explicit ColorSource(AIndex size_) : size{size_} {}

  /** Get color at the given absolute \a index
   * @see constrainedIndex()
   */
  virtual PxColor do_getColor(AIndex index) = 0;

  /** Helper function for constraining \a index
   * Always returns \c 0...(size-1) - even for negative indices (mathematical modulo).
   */
  AIndex constrainedIndex(AIndex index) const { return ((index % size) + size) % size; }
};

/** A ColorSource that creates colors based on rainbow or from currently selected palette.
 * When the \e Default palette (0) is selected in the UI, a rainbow color (based on HSV color model)
 * is created. Otherwise, the color is created based on the currently selected palette.
 * Like rainbowColor() on steroids.
 */
class RainbowColorSource final : public ColorSource
{
public:
  // no copy & move - this class is intended to be used as temporary object on the stack
  RainbowColorSource(const RainbowColorSource &) = delete;
  RainbowColorSource &operator=(const RainbowColorSource &) = delete;

  /** Constructor.
   * @param env Effect runtime environment.
   * @param size Size of the color spectrum (in pixels).
   *             0 uses the entire segment for one full range of the rainpow (or palette).
   */
  explicit RainbowColorSource(FxEnv &env, AIndex size = 0)
      : ColorSource(size ? size : env.seglen()), _env{env} {}

  /// Brightness of the color.
  uint8_t vol = 255;

  /// Color interpolation option for accessing the currently selected palette.
  TBlendType blendType = LINEARBLEND;

  /// When a color is requested, this offset is added to the user's given index.
  AIndex offset = 0;

  /** Set the \c offset (normalized version).
   * A value of 0.5 for example sets the \c offset to half the spectrum's size.
   */
  void setOffset_N(NIndex offset) { this->offset = norm2abs(offset, size); }

private:
  /// @see ColorSource::do_getColor()
  PxColor do_getColor(AIndex index) override
  {
    const uint8_t hue = map(constrainedIndex(index + offset), 0, size, 0, 255);
    return rainbowColor(_env, hue, vol, blendType);
  }

  FxEnv &_env;
};

//--------------------------------------------------------------------------------------------------

/** Draw a line between absolute positions (direction doesn't matter).
 * @param pxa Draw on that pixel array.
 * @param firstPos First pixel of the line.
 * @param lastPos  Last pixel of the line.
 * @param colorSource Get pixel color from there; the index is incremented by one for every pixel.
 */
void colorLine_abs(PxArray &pxa, AIndex firstPos, AIndex lastPos, ColorSource &colorSource);

/** Draw a relative line.
 * @param pxa Draw on that pixel array.
 * @param startPos First pixel of the line.
 * @param length Length of the line.
 *               Positive values for draw upward the array, negative values draw in the other direction.
 * @param colorSource Get pixel color from there; the index is incremented by one for every pixel.
 */
inline void colorLine_rel(PxArray &pxa, AIndex startPos, int length, ColorSource &colorSource)
{
  if (length > 0)
    colorLine_abs(pxa, startPos, startPos + length - 1, colorSource);
  else if (length < 0)
    colorLine_abs(pxa, startPos, startPos + length + 1, colorSource);
}

/// Similar to colorLine_rel() but draws around the given \a centerPos as middle of the line.
inline void colorLine_centered(PxArray &pxa, AIndex centerPos, int length, ColorSource &colorSource)
{
  colorLine_rel(pxa, centerPos - length / 2, length, colorSource);
}

/// Like colorLine_rel() - but with normalized positions.
inline void colorLine_rel_N(PxArray &pxa, NIndex startPos, float length, ColorSource &colorSource)
{
  colorLine_rel(pxa, pxa.toAbs(startPos), pxa.toAbs(length), colorSource);
}

/// Like colorLine_abs() - but with normalized positions.
inline void colorLine_abs_N(PxArray &pxa, NIndex firstPos, NIndex lastPos, ColorSource &colorSource)
{
  colorLine_abs(pxa, pxa.toAbs(firstPos), pxa.toAbs(lastPos), colorSource);
}

/// Like colorLine_centered() - but with normalized positions.
inline void colorLine_centered_N(PxArray &pxa, NIndex centerPos, float length, ColorSource &colorSource)
{
  colorLine_rel_N(pxa, centerPos - length / 2.0f, length, colorSource);
}

//--------------------------------------------------------------------------------------------------

inline bool FxEnv::isFistFrame() const { return _segenv->call == 0; }

/// The worm of fail.
void fx_broken(FxEnv &env);

//--------------------------------------------------------------------------------------------------

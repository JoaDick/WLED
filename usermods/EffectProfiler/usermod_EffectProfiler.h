/**
 * Blablabla.
 */

#pragma once

#include "wled.h"
#include "EffectProfilerTrigger.h"

//--------------------------------------------------------------------------------------------------

uint16_t mode_fire_2012();
uint16_t mode_lake();
uint16_t mode_meteor();
uint16_t mode_perlinmove();

/// Just a simple effect example.
uint16_t mode_ExampleRainbow()
{
  static uint8_t starthue = 0;
  uint8_t hue = ++starthue;
  for (unsigned i = 0; i < SEGLEN; ++i)
  {
    SEGMENT.setPixelColor(i, CHSV(++hue, 255, 128));
  }
  return FRAMETIME;
}

/// Just another simple effect example.
uint16_t mode_ExampleRandom()
{
  for (unsigned i = 0; i < SEGLEN; ++i)
  {
    SEGMENT.setPixelColor(i, hw_random8() / 16, hw_random8() / 16, hw_random8() / 16);
  }
  return FRAMETIME;
}

/** Example ... TBD
 *
 */
uint16_t mode_EffectProfiler_Auto()
{
  EffectProfilerTrigger profiler;

  mode_ExampleRainbow();
  // mode_ExampleRandom();

  return FRAMETIME;
}
static const char _data_FX_mode_EffectProfiler_Auto[] PROGMEM = "A Profiler: auto@;";

/** Example for comparing 2 existing effects.
 * @note Important: Only one of them may use SEGENV - otherwise they will interfere with each other.
 */
uint16_t mode_EffectProfiler_FxCompare()
{
  EffectProfilerTrigger profiler;
  uint16_t retval;

  if (profiler.mustRun_A())
  {
    profiler.startSingle_A();
    // retval = mode_lake();
    retval = mode_perlinmove();
    profiler.stop();
  }
  else
  {
    profiler.startSingle_B();
    // retval = mode_fire_2012();
    retval = mode_meteor();
    profiler.stop();
  }

  return retval;
}
static const char _data_FX_mode_EffectProfiler_FxComp[] PROGMEM = "A Profiler: compare@!,!,!,!,!,!,!,!;!,!,!;!";

// ... slow ...
void complicated_algorithm()
{
  uint32_t lastColor = SEGMENT.getPixelColor(SEGLEN - 1);
  for (unsigned i = 0; i < SEGLEN; ++i)
  {
    uint32_t temp = SEGMENT.getPixelColor(i);
    SEGMENT.setPixelColor(i, lastColor);
    lastColor = temp;
  }
}

// ... fast ...
void optimized_algorithm()
{
  Segment &seg = SEGMENT;
  const unsigned size = SEGLEN;
  uint32_t lastColor = seg.getPixelColor(size - 1);
  for (unsigned i = 0; i < size; ++i)
  {
    uint32_t temp = seg.getPixelColor(i);
    seg.setPixelColor(i, lastColor);
    lastColor = temp;
  }
}

uint16_t mode_EffectProfiler_example()
{
  EffectProfilerTrigger profiler;

  mode_ExampleRandom();

  if (profiler.mustRun_A())
  {
    const uint32_t iterations = profiler.startMulti_A();
    for (uint32_t i = 0; i < iterations; ++i)
    {
      // the new _A_dvanced stuff
      optimized_algorithm();
    }
    profiler.stop();
  }
  else
  {
    const uint32_t iterations = profiler.startMulti_B();
    for (uint32_t i = 0; i < iterations; ++i)
    {
      // the old stuff as reference _B_aseline
      complicated_algorithm();
    }
    profiler.stop();
  }

  return FRAMETIME;
}
static const char _data_FX_mode_EffectProfiler[] PROGMEM = "A Profiler: A-B@;";

//--------------------------------------------------------------------------------------------------

/// Effect runtime statistics, calculated by the profiler.
class EffectProfilerStats
{
public:
  EffectProfilerStats() { reset(); }

  bool isValid() const { return totalIterations() > 0; }

  uint32_t frames() const { return _frames; }
  uint32_t totalIterations() const { return _totalIterations; }
  uint32_t totalDuration_us() const { return _totalDuration_us; }

  uint32_t avgDuration_us() const { return _totalDuration_us / _totalIterations; }
  uint32_t minDuration_us() const { return _minDuration_us; }
  uint32_t maxDuration_us() const { return _maxDuration_us; }

  uint32_t maxIterationsPerSec() const { return (1000 * 1000) / avgDuration_us(); }
  uint32_t maxIterationsPerFrame() const;

  void addSample(uint32_t iterations, uint32_t duration_us)
  {
    ++_frames;
    _totalIterations += iterations;
    _totalDuration_us += duration_us;
    const uint32_t currentDuration_us = duration_us / iterations;
    if (currentDuration_us < _minDuration_us)
    {
      _minDuration_us = currentDuration_us;
    }
    if (currentDuration_us > _maxDuration_us)
    {
      _maxDuration_us = currentDuration_us;
    }
  }

  void reset()
  {
    _frames = 0;
    _totalIterations = 0;
    _totalDuration_us = 0;
    _minDuration_us = 999999;
    _maxDuration_us = 0;
  }

private:
  uint32_t _frames;
  uint32_t _totalIterations;
  uint32_t _totalDuration_us;
  uint32_t _minDuration_us;
  uint32_t _maxDuration_us;
};

/// The effect profiler implementation which is doing the real math.
class EffectProfilerEngine : public EffectProfilerBackend
{
public:
  /// Constructor
  EffectProfilerEngine() { reset(); }

  /// Checks if a profiling session is currently in progress.
  bool isActive() const { return millis() - _timestamp_lastTestRun < 1000; }

  /// Get the mode that is currently processed for profiling.
  uint8_t currentMode() const { return _currentMode; }

  /// Get profiling statistics.
  const EffectProfilerStats &stats_A() const { return _stats_A; }

  /// Get profiling statistics for option B (only valid in case of A-B testing).
  const EffectProfilerStats &stats_B() const { return _stats_B; }

  /// @see EffectProfilerBackend::isSelected_A()
  bool isSelected_A() override { return _isSelected_A; }

  /// @see EffectProfilerBackend::getIterations_A()
  uint32_t getIterations_A() override { return _iterations_A; }

  /// @see EffectProfilerBackend::getIterations_B()
  uint32_t getIterations_B() override { return _iterations_B; }

  /// @see EffectProfilerBackend::addTestRun()
  void addTestRun(uint32_t duration_us, uint32_t iterations, bool is_A, Segment &seg) override
  {
    if (seg.isInTransition())
    {
      return;
    }

    const uint8_t mode = seg.mode;
    if (mode != _currentMode)
      reset(mode);

    if (is_A)
      _stats_A.addSample(iterations, duration_us);
    else
      _stats_B.addSample(iterations, duration_us);

    drawStats(seg);
    prepareNextTestRun();
    _timestamp_lastTestRun = millis();
  }

private:
  void reset(uint8_t newMode = 0)
  {
    _currentMode = newMode;
    _timestamp_lastTestRun = 0;
    _timestamp_lastToggle = 0;
    _isSelected_A = false;
    _iterations_A = 5;
    _iterations_B = 5;
    _stats_A.reset();
    _stats_B.reset();
    prepareNextTestRun();
  }

  void drawStats(Segment &seg)
  {
    seg.setPixelColor(0, 0x000000);
    seg.setPixelColor(1, 0x000000);
    seg.setPixelColor(2, 0x000000);
    if (_isSelected_A)
    {
      seg.setPixelColor(0, 0x00FF00);
    }
    else
    {
      if (_stats_B.isValid())
      {
        seg.setPixelColor(1, 0x0000FF);
      }
    }

    if (_stats_A.isValid() && _stats_B.isValid())
    {
      const uint32_t duration_A = _stats_A.avgDuration_us();
      const uint32_t duration_B = _stats_B.avgDuration_us();
      if (duration_A < duration_B)
      {
        const float ratio = float(duration_A) / float(duration_B);
        const int pos = seg.vLength() * ratio;
        seg.setPixelColor(pos - 1, 0);
        seg.setPixelColor(pos, 0x00FF00);
        seg.setPixelColor(pos + 1, 0);
      }
      if (duration_A > duration_B)
      {
        const float ratio = float(duration_B) / float(duration_A);
        const int pos = seg.vLength() * ratio;
        seg.setPixelColor(pos - 1, 0);
        seg.setPixelColor(pos, 0x0000FF);
        seg.setPixelColor(pos + 1, 0);
      }
    }
  }

  void prepareNextTestRun()
  {
    const uint32_t now = millis();
    if (now - _timestamp_lastToggle > 3000)
    {
      _timestamp_lastToggle = now;
      _isSelected_A ^= true;
    }
    // TODO: calculate new optimized values for _iterations_A & _iterations_B
  }

private:
  uint8_t _currentMode;
  uint32_t _timestamp_lastTestRun;
  uint32_t _timestamp_lastToggle;
  bool _isSelected_A;
  uint32_t _iterations_A;
  uint32_t _iterations_B;
  EffectProfilerStats _stats_A;
  EffectProfilerStats _stats_B;
};

/// The EffectProfiler usermod to be registered at WLED.
class UmEffectProfiler : public Usermod
{
public:
  uint16_t getId() override { return USERMOD_ID_EFFECT_PROFILER; }

  void setup() override
  {
    strip.addEffect(255, &mode_EffectProfiler_Auto, _data_FX_mode_EffectProfiler_Auto);
    strip.addEffect(255, &mode_EffectProfiler_example, _data_FX_mode_EffectProfiler);
    strip.addEffect(255, &mode_EffectProfiler_FxCompare, _data_FX_mode_EffectProfiler_FxComp);

    if (!initDone())
    {
      um_data.u_size = 1;
      um_data.u_data = new void *[um_data.u_size];
      um_data.u_type = new um_types_t[um_data.u_size];
      um_data.u_data[0] = &_profiler;
      um_data.u_type[0] = UMT_BYTE_ARR; // type UMT_VOID_PTR would be great...
    }
  }

  void loop() override {}

  bool getUMData(um_data_t **data) override
  {
    if (!data)
      return false; // no pointer provided by caller or not enabled -> exit
    *data = &um_data;
    return true;
  }

  void addToJsonInfo(JsonObject &root) override
  {
    // TODO: add a "Reset"-button

    JsonObject user = root["u"];
    if (user.isNull())
    {
      user = root.createNestedObject("u");
    }

    const EffectProfilerStats &stats_A = _profiler.stats_A();
    const EffectProfilerStats &stats_B = _profiler.stats_B();

    String uiDomString;
    JsonArray infoArr = user.createNestedArray(F("Effect Profiler"));
    if (_profiler.isActive())
    {
      uiDomString = F("Mode ");
      uiDomString += _profiler.currentMode();
      if (stats_B.isValid())
      {
        uiDomString += _profiler.isSelected_A() ? F(": <font color=\"#00FF00\";><b>A</b></font> | B")
                                                : F(": A | <font color=\"#00BFFF\";><b>B</b></font>");
      }
    }
    else
    {
      const auto lastMode = _profiler.currentMode();
      if (lastMode)
      {
        uiDomString = F("<i>last</i> Mode ");
        uiDomString += lastMode;
        if (stats_B.isValid())
        {
          uiDomString += F(": A | B");
        }
      }
      else
      {
        uiDomString = F("<i>(idle)</i>");
      }
    }
    infoArr.add(uiDomString);

    if (stats_A.isValid() || stats_B.isValid())
    {
      const int32_t duration_A = stats_A.avgDuration_us();
      const int32_t duration_B = stats_B.avgDuration_us();

      if (stats_A.isValid() && stats_B.isValid())
      {
        const int32_t ratio = (1000 * duration_A) / duration_B;
        const int32_t delta = duration_A - duration_B;

        infoArr = user.createNestedArray(F("Delta A - B"));
        uiDomString.clear();

        if (delta > 0)
        {
          uiDomString += F("+");
        }
        uiDomString += delta;
        uiDomString += F("µs (");
        if (delta > 0)
        {
          uiDomString += F("+");
        }
        if (delta < 0)
        {
          uiDomString += F("-");
        }
        uiDomString += abs(ratio - 1000) / 10;
        uiDomString += F(".");
        uiDomString += abs(ratio - 1000) % 10;
        uiDomString += F("%)<br>");

        uiDomString += F("A:B = <font color=\"");
        uiDomString += delta < 0 ? F("#90EE90") : F("#FF8C00");
        uiDomString += F("\";>");
        uiDomString += ratio / 10;
        uiDomString += F(".");
        uiDomString += ratio % 10;
        uiDomString += F("%</font>");

        infoArr.add(uiDomString);
      }

#if (0)
      if (stats_A.isValid() && stats_B.isValid())
      {
        const int32_t ratio = (1000 * duration_A) / duration_B;
        const int32_t delta = duration_A - duration_B;

        infoArr = user.createNestedArray(F("Delta A - B (optimization)"));
        uiDomString.clear();
        if (delta > 0)
        {
          uiDomString += F("+");
        }
        uiDomString += delta;
        uiDomString += F("µs (");
        if (delta > 0)
        {
          uiDomString += F("+");
        }
        if (delta < 0)
        {
          uiDomString += F("-");
        }
        uiDomString += abs(ratio - 1000) / 10;
        uiDomString += F(".");
        uiDomString += abs(ratio - 1000) % 10;
        uiDomString += F("%)");
        infoArr.add(uiDomString);

#if (1)
        infoArr = user.createNestedArray(F("Average duration ratio"));
        uiDomString.clear();
        uiDomString += F("A : B = ");
        uiDomString += ratio / 10;
        uiDomString += F(".");
        uiDomString += ratio % 10;
        uiDomString += F("%");
        infoArr.add(uiDomString);
#endif
      }
#endif

      infoArr = user.createNestedArray(F("Average duration"));
      uiDomString.clear();
      if (stats_A.isValid())
      {
        uiDomString += duration_A;
        uiDomString += F("µs");
      }
      if (stats_B.isValid())
      {
        uiDomString += F(" | ");
        uiDomString += duration_B;
        uiDomString += F("µs");
      }
      infoArr.add(uiDomString);

      infoArr = user.createNestedArray(F("Max. duration"));
      uiDomString.clear();
      if (stats_A.isValid())
      {
        uiDomString += stats_A.maxDuration_us();
        uiDomString += F("µs");
      }
      if (stats_B.isValid())
      {
        uiDomString += F(" | ");
        uiDomString += stats_B.maxDuration_us();
        uiDomString += F("µs");
      }
      infoArr.add(uiDomString);

      infoArr = user.createNestedArray(F("Min. duration"));
      uiDomString.clear();
      if (stats_A.isValid())
      {
        uiDomString += stats_A.minDuration_us();
        uiDomString += F("µs");
      }
      if (stats_B.isValid())
      {
        uiDomString += F(" | ");
        uiDomString += stats_B.minDuration_us();
        uiDomString += F("µs");
      }
      infoArr.add(uiDomString);

      infoArr = user.createNestedArray(F("Iterations total (per Frame)"));
      uiDomString.clear();
      if (stats_A.isValid())
      {
        const uint32_t iterations = stats_A.totalIterations();
        const uint32_t iterations_pf = 100 * iterations / stats_A.frames();
        uiDomString += iterations;
        uiDomString += F(" (");
        uiDomString += iterations_pf / 100;
        uiDomString += F(".");
        uiDomString += iterations_pf % 100;
        uiDomString += F(")");
      }
      if (stats_B.isValid())
      {
        uiDomString += F(" | ");
        const uint32_t iterations = stats_B.totalIterations();
        const uint32_t iterations_pf = 100 * iterations / stats_B.frames();
        uiDomString += iterations;
        uiDomString += F(" (");
        uiDomString += iterations_pf / 100;
        uiDomString += F(".");
        uiDomString += iterations_pf % 100;
        uiDomString += F(")");
      }
      infoArr.add(uiDomString);
    }
  }

private:
  bool initDone() const { return um_data.u_size != 0; }

private:
  EffectProfilerEngine _profiler;
  um_data_t um_data;
};

//--------------------------------------------------------------------------------------------------

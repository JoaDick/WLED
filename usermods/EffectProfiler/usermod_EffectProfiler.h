/**
 * EffectProfiler usermod.
 *
 * The purpose of this usermod is to show the runtime of a WLED effect implementation in the "Info"
 * section of the web UI. A direct comparison of 2 implementation alternatives is also possible. \n
 * Any effect that shall be profiled just has to use the class \c EffectProfilerTrigger for starting
 * and stopping the measurements. Everything else is done in the background by this usermod.
 *
 * (c) 2025 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include <array>
#include "wled.h"
#include "EffectProfilerTrigger.h"

//--------------------------------------------------------------------------------------------------

/// Minimalistic example that shows how to measure the runtime of a function.
uint16_t mode_EffectProfiler_auto()
{
  // 1. just add this line at the very beginning of the function
  EffectProfilerTrigger profiler(0); // uses slot 0

  // 2. then comes the code
  SEGMENT.fadeToBlackBy(128 - (SEGMENT.intensity / 2));
  const uint16_t pos = beatsin16(1 + SEGMENT.speed / 4, 0, SEGLEN - 1);
  SEGMENT.setPixelColor(pos, SEGCOLOR(0));
  if (SEGENV.call == 0)
    SEGENV.aux0 = pos;
  while (SEGENV.aux0 < pos)
    SEGMENT.setPixelColor(SEGENV.aux0++, SEGCOLOR(0));
  while (SEGENV.aux0 > pos)
    SEGMENT.setPixelColor(SEGENV.aux0--, SEGCOLOR(0));

  // 3. nothing more :-)
  return FRAMETIME;
}
static const char _data_FX_mode_EffectProfiler_auto[] PROGMEM = "Profiler: auto@!,!;!;;;sx=120,ix=64";

/// Example that shows how to measure multiple slots (same effect as before).
uint16_t mode_EffectProfiler_slots()
{
  EffectProfilerTrigger profiler;

  profiler.start(0); // uses slot 0
  SEGMENT.fadeToBlackBy(128 - (SEGMENT.intensity / 2));
  profiler.stop();

  profiler.start(1); // uses slot 1
  const uint16_t pos = beatsin16(1 + SEGMENT.speed / 4, 0, SEGLEN - 1);
  profiler.stop();

  profiler.start(2); // uses slot 2
  SEGMENT.setPixelColor(pos, SEGCOLOR(0));
  if (SEGENV.call == 0)
    SEGENV.aux0 = pos;
  while (SEGENV.aux0 < pos)
    SEGMENT.setPixelColor(SEGENV.aux0++, SEGCOLOR(0));
  while (SEGENV.aux0 > pos)
    SEGMENT.setPixelColor(SEGENV.aux0--, SEGCOLOR(0));
  profiler.stop();

  return FRAMETIME;
}
static const char _data_FX_mode_EffectProfiler_slots[] PROGMEM = "Profiler: slots@!,!;!;;;sx=120,ix=64";

//--------------------------------------------------------------------------------------------------

/// _B_aseline rainbow function.
void makeColorWheelRainbow(uint8_t startHue, uint8_t deltaHue)
{
  for (unsigned i = 0; i < SEGLEN; ++i)
  {
    SEGMENT.setPixelColor(i, SEGMENT.color_wheel(startHue));
    startHue += deltaHue;
  }
}

/// _A_lternative rainbow function.
void makeFastLedRainbow(uint8_t startHue, uint8_t deltaHue)
{
  for (unsigned i = 0; i < SEGLEN; ++i)
  {
    SEGMENT.setPixelColor(i, CHSV(startHue, 255, 255));
    startHue += deltaHue;
  }
}

/** A simple example to show how to perform A-B testing.
 * Imagine we're having 2 different options for making a simple rainbow effect: We could use WLED's
 * \c color_wheel() function for creating the rainbow, or FastLED's \c CHSV class as alternative.
 * Both are very easy to use, and have their own unique features. But which one is faster? \n
 * Let's find out: run this effect, and have a look at the Info section of the web UI.
 */
uint16_t mode_EffectProfiler_AB()
{
  static uint16_t startHue = 0;
  static uint8_t deltaHue = 3;

  EffectProfilerTrigger profiler;
  if (profiler.mustRun_A())
  {
    profiler.start_A();
    makeFastLedRainbow(startHue / 256, deltaHue); // the new _A_lternative stuff
    profiler.stop();
  }
  else
  {
    profiler.start_B();
    makeColorWheelRainbow(startHue / 256, deltaHue); // the existing stuff as reference _B_aseline
    profiler.stop();
  }

  startHue += 64;

  return FRAMETIME;
}
static const char _data_FX_mode_EffectProfiler_AB[] PROGMEM = "Profiler: A-B@;";

//--------------------------------------------------------------------------------------------------

// Experimental: considered slow.
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

// Experimental: hopefully faster.
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

// Experimental.
uint16_t mode_EffectProfiler_multi()
{
  EffectProfilerTrigger profiler;

  profiler.start(0);
  for (unsigned i = 0; i < SEGLEN; ++i)
    SEGMENT.setPixelColor(i, hw_random8() / 16, hw_random8() / 16, hw_random8() / 16);
  profiler.stop();

  if (profiler.mustRun_A())
  {
    const uint32_t iterations = profiler.startMulti_A();
    for (uint32_t i = 0; i < iterations; ++i)
      optimized_algorithm();
    profiler.stop();
  }
  else
  {
    const uint32_t iterations = profiler.startMulti_B();
    for (uint32_t i = 0; i < iterations; ++i)
      complicated_algorithm();
    profiler.stop();
  }

  return FRAMETIME;
}
static const char _data_FX_mode_EffectProfiler_multi[] PROGMEM = "Profiler: multi@;";

//--------------------------------------------------------------------------------------------------

/// Effect runtime statistics, calculated by the profiler.
class EffectProfilerStats
{
public:
  EffectProfilerStats() { reset(); }

  bool isValid() const { return _avgDuration_us > 0; }

  uint32_t frames() const { return _frames; }
  uint32_t totalIterations() const { return _totalIterations; }
  uint32_t totalDuration_us() const { return _totalDuration_us; }

  uint32_t avgDuration_us() const { return _avgDuration_us; }
  uint32_t minDuration_us() const { return _minDuration_us; }
  uint32_t maxDuration_us() const { return _maxDuration_us; }

  void addSample(uint32_t iterations, uint32_t duration_us)
  {
    ++_frames;
    _totalIterations += iterations;
    _totalDuration_us += duration_us;
    _avgDuration_us = _totalDuration_us / _totalIterations;
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
    _avgDuration_us = 0;
    _minDuration_us = 999999;
    _maxDuration_us = 0;
  }

  void resetStats()
  {
    _frames = 1;
    _totalIterations = 1;
    _totalDuration_us = _avgDuration_us;
    _minDuration_us = _avgDuration_us;
    _maxDuration_us = _avgDuration_us;
  }

private:
  uint32_t _frames;
  uint32_t _totalIterations;
  uint32_t _totalDuration_us;
  uint32_t _avgDuration_us;
  uint32_t _minDuration_us;
  uint32_t _maxDuration_us;
};

/// The effect profiler implementation which is doing the real math.
class EffectProfilerEngine : public EffectProfilerBackend
{
public:
  /// Constructor
  EffectProfilerEngine() { reset(); }

  /// Check if a profiling session is currently in progress.
  bool isActive() const { return millis() - _timestamp_lastTestRun < 1000; }

  /// Get the mode that is currently processed for profiling.
  uint8_t currentMode() const { return _currentMode; }

  /// Get profiling statistics.
  const EffectProfilerStats &stats(uint8_t slot) const
  {
    if (slot >= _stats.size())
      slot = 0;
    return _stats[slot];
  }

  /// Get profiling statistics for option A (only valid in case of A-B testing).
  const EffectProfilerStats &stats_A() const { return stats(EffectProfilerTrigger::slot_A); }

  /// Get profiling statistics for option B (only valid in case of A-B testing).
  const EffectProfilerStats &stats_B() const { return stats(EffectProfilerTrigger::slot_B); }

  /// Get profiling statistics for frame.
  const EffectProfilerStats &stats_F() const { return stats(EffectProfilerTrigger::slot_F); }

  /// @see EffectProfilerBackend::isSelected_A()
  bool isSelected_A() override { return _isSelected_A; }

  /// @see EffectProfilerBackend::getIterations_A()
  uint32_t getIterations_A() override { return _iterations_A; }

  /// @see EffectProfilerBackend::getIterations_B()
  uint32_t getIterations_B() override { return _iterations_B; }

  /// @see EffectProfilerBackend::addTestRun()
  void addTestRun(uint32_t duration_us, uint32_t iterations, uint8_t slot, Segment &seg) override
  {
    if (seg.isInTransition())
      return;

    if (slot >= _stats.size())
      return;

    const uint8_t mode = seg.mode;
    if (mode != _currentMode)
      reset(mode);

    _stats[slot].addSample(iterations, duration_us);

    drawStats(seg);
    prepareNextTestRun();
    _timestamp_lastTestRun = millis();
  }

  void resetStats()
  {
    for (auto &stat : _stats)
      stat.resetStats();
  }

private:
  void reset(uint8_t newMode = 0)
  {
    _currentMode = newMode;
    _timestamp_lastTestRun = 0;
    _timestamp_lastToggle = 0;
    _isSelected_A = false;
    _iterations_A = 10;
    _iterations_B = 10;
    for (auto &stat : _stats)
      stat.reset();
    prepareNextTestRun();
  }

  void drawStats(Segment &seg)
  {
    if (!stats_A().isValid() || !stats_B().isValid())
      return;

    seg.setPixelColor(0, 0x000000);
    seg.setPixelColor(1, 0x000000);
    seg.setPixelColor(2, 0x000000);
    seg.setPixelColor(3, 0x000000);
    if (_isSelected_A)
      seg.setPixelColor(1, 0x00FF00);
    else
      seg.setPixelColor(2, 0x0000FF);

    const uint32_t duration_A = stats_A().avgDuration_us();
    const uint32_t duration_B = stats_B().avgDuration_us();
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

  void prepareNextTestRun()
  {
    const uint32_t now = millis();
    if (now - _timestamp_lastToggle > 3000)
    {
      _timestamp_lastToggle = now;
      _isSelected_A ^= true;
    }
    // TODO: Calculate new optimized values for _iterations_A and _iterations_B so they don't pull
    // down the FPS too much.
  }

private:
  uint8_t _currentMode;
  uint32_t _timestamp_lastTestRun;
  uint32_t _timestamp_lastToggle;
  bool _isSelected_A;
  uint32_t _iterations_A;
  uint32_t _iterations_B;
  std::array<EffectProfilerStats, EffectProfilerTrigger::maxSlots + 3> _stats;
};

/// The EffectProfiler usermod, which is presenting the measurement statistics on the UI.
class UmEffectProfiler : public Usermod
{
public:
  uint16_t getId() override { return USERMOD_ID_EFFECT_PROFILER; }

  void setup() override
  {
    strip.addEffect(255, &mode_EffectProfiler_auto, _data_FX_mode_EffectProfiler_auto);
    strip.addEffect(255, &mode_EffectProfiler_slots, _data_FX_mode_EffectProfiler_slots);
    strip.addEffect(255, &mode_EffectProfiler_AB, _data_FX_mode_EffectProfiler_AB);
    strip.addEffect(255, &mode_EffectProfiler_multi, _data_FX_mode_EffectProfiler_multi);

    if (!initDone())
    {
      um_data.u_size = 1;
      um_data.u_data = new void *[um_data.u_size];
      um_data.u_type = nullptr;
      um_data.u_data[0] = &_profiler;
    }
  }

  void loop() override {}

  bool getUMData(um_data_t **data) override
  {
    if (!data)
      return false;
    *data = &um_data;
    return true;
  }

  void readFromJsonState(JsonObject &root) override
  {
    if (!initDone())
      return; // prevent crash on boot applyPreset()

    const JsonObject usermod = root[F("EffectProfiler")];
    if (usermod.isNull())
      return;

    const auto cmd = usermod[F("cmd")];
    if (cmd.is<int>())
    {
      switch (cmd.as<int>())
      {
      case 1:
        _profiler.resetStats();
        break;

      case 0:
      // nothing; just updating info page
      // [[fallthrough]]
      default:
        break;
      }
    }
  }

  void addToJsonInfo(JsonObject &root) override
  {
    JsonObject user = root["u"];
    if (user.isNull())
      user = root.createNestedObject("u");

    const EffectProfilerStats &stats_A = _profiler.stats_A();
    const EffectProfilerStats &stats_B = _profiler.stats_B();
    const EffectProfilerStats &stats_F = _profiler.stats_F();

    JsonArray infoArr;
    String arrayEntry;
    String uiDomString;

    // --- header & buttons ---

    arrayEntry = F("<button onclick=\"requestJson({EffectProfiler:{cmd:0}});\"><b>Effect Profiler</b></button>");
    if (_profiler.isActive())
    {
      arrayEntry += F(" <small>Mode ");
      arrayEntry += _profiler.currentMode();
      arrayEntry += F("</small>");
    }
    else
    {
      const auto lastMode = _profiler.currentMode();
      if (lastMode)
      {
        arrayEntry += F(" <small>last Mode: ");
        arrayEntry += lastMode;
        arrayEntry += F("</small>");
      }
    }
    infoArr = user.createNestedArray(arrayEntry);
    infoArr.add(_profiler.isActive()
                    ? F("<button onclick=\"requestJson({EffectProfiler:{cmd:1}});\">Reset</button>")
                    : F("<i>(idle)</i>"));

    infoArr = user.createNestedArray(F("Frames"));
    uiDomString = int(stats_F.frames());
    infoArr.add(uiDomString);

    // --- duration table ---

    infoArr = user.createNestedArray(F("Duration (µs)"));
    uiDomString.clear();
    uiDomString += F("<table>");
    uiDomString += F("<tr><th></th><th>min.</th><th>avg.</th><th>max.</th></tr>");
    uiDomString += makeDurationRow(F("Frame"), stats_F);
    for (uint8_t i = 0; i < EffectProfilerTrigger::maxSlots; ++i)
      uiDomString += makeDurationRow(String(F("Slot ")) + int(i), _profiler.stats(i));
    uiDomString += makeDurationRow(F("A"), stats_A);
    uiDomString += makeDurationRow(F("B"), stats_B);
    uiDomString += F("</table>");
    infoArr.add(uiDomString);

    if (stats_A.isValid() && stats_B.isValid())
    {
      const int32_t duration_A = stats_A.avgDuration_us();
      const int32_t duration_B = stats_B.avgDuration_us();

      const int32_t delta = duration_A - duration_B;
      const int32_t ratio = (1000 * duration_A) / duration_B;

      // --- A-B comparison ---
      infoArr = user.createNestedArray(F("Delta A-B"));
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

      // --- A-B iterations ---
      infoArr = user.createNestedArray(F("Iterations"));
      uiDomString.clear();
      if (_profiler.isSelected_A())
      {
        uiDomString += F("<font color=\"#00FF00\";><b>A</b></font> = ");
        uiDomString += stats_A.totalIterations();
        uiDomString += F(" | B = ");
        uiDomString += stats_B.totalIterations();
      }
      else
      {
        uiDomString += F("A = ");
        uiDomString += stats_A.totalIterations();
        uiDomString += F(" | <font color=\"#00BFFF\";><b>B</b></font> = ");
        uiDomString += stats_B.totalIterations();
      }
      infoArr.add(uiDomString);
    }
  }

private:
  bool initDone() const { return um_data.u_size != 0; }

  String makeDurationRow(const String &slot, const EffectProfilerStats &stats)
  {
    String uiDomString;
    if (stats.isValid())
    {
      uiDomString += F("<tr><td><b>");
      uiDomString += slot;
      uiDomString += F("</b></td><td>");
      uiDomString += stats.minDuration_us();
      uiDomString += F("</td><td>");
      uiDomString += stats.avgDuration_us();
      uiDomString += F("</td><td>");
      uiDomString += stats.maxDuration_us();
      uiDomString += F("</td></tr>");
    }
    return uiDomString;
  }

private:
  EffectProfilerEngine _profiler;
  um_data_t um_data;
};

//--------------------------------------------------------------------------------------------------

/**
 * Frontend of the EffectProfiler usermod.
 *
 * Class \c EffectProfilerTrigger must be used by the effect-under-test for starting and stopping
 * the measurement. See examples in usermod_EffectProfiler.h
 *
 * (c) 2025 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include "wled.h"

//--------------------------------------------------------------------------------------------------

/** Internal interface of the EffectProfiler usermod.
 * Not to be used directly.
 */
class EffectProfilerBackend
{
public:
  EffectProfilerBackend(const EffectProfilerBackend &) = delete;
  EffectProfilerBackend &operator=(const EffectProfilerBackend &) = delete;
  virtual bool isSelected_A() = 0;
  virtual uint32_t getIterations_A() = 0;
  virtual uint32_t getIterations_B() = 0;
  virtual void addTestRun(uint32_t duration_us, uint32_t iterations, bool is_A, Segment &seg) = 0;

protected:
  EffectProfilerBackend() = default;
};

/** Trigger interface for the effects to control the measurements.
 * Automatic measurement:
 * - Just place an instance of this class at the very beginning of your \c mode_XYZ() function.
 * - That's all.
 *
 * Selective measurement:
 * - Place an instance of this class anywhere in your \c mode_XYZ() function.
 * - Call \c mustRun_A() just before the code section that shall be measured.
 * - Call \c stop() immediately after the code section that shall be measured.
 *
 * A-B testing:
 * - Similar to selective measurement, but with 2 alternative code sections that shall be compared.
 * - Check which option shall be executed --> \c mustRun_A()
 * - A is the _A_lternative or _A_dvanced new stuff --> \c start_A()
 * - B is the existing _B_aseline to compare against --> \c start_B()
 * - Stop the measurement --> \c stop()
 */
class EffectProfilerTrigger : private EffectProfilerBackend
{
public:
  /** Constructor.
   * Starts measurement (of option A) implicitly.
   * Measuring can be restarted by explicitly calling \c start_A() or \c start_B()
   */
  explicit EffectProfilerTrigger()
  {
    UM_Exchange_Data *um_data;
    if (UsermodManager::getUMData(&um_data, USERMOD_ID_EFFECT_PROFILER))
      _backend = static_cast<EffectProfilerBackend *>(um_data->u_data[0]);
    else
      _backend = this;
    start_A();
  }

  /** Destructor.
   * Stops a running measurement implicitly via \c stop()
   */
  ~EffectProfilerTrigger() { stop(); }

  /** Start simple measurement (or measurement of option A in case of A-B testing).
   * @see mustRun_A()
   */
  void start_A()
  {
    _is_A = true;
    _iterations = 1;
    _startTime = micros();
  };

  /// Stop a running measurement and add result to the profiling statistics.
  void stop()
  {
    const uint32_t endTime = micros();
    if (_startTime)
    {
      _backend->addTestRun(endTime - _startTime, _iterations, _is_A, SEGMENT);
      _startTime = 0;
    }
  };

  /** Determine which option (A or B) shall be measured.
   * Only for A-B testing:
   * - A is the _A_lternative or _A_dvanced (new) stuff
   * - B is the existing _B_aseline to compare against
   */
  bool mustRun_A() { return _backend->isSelected_A(); }

  /// Start measurement of option B.
  void start_B()
  {
    _is_A = false;
    _iterations = 1;
    _startTime = micros();
  };

  /// Experimental.
  uint32_t startMulti_A()
  {
    _is_A = true;
    _iterations = _backend->getIterations_A();
    _startTime = micros();
    return _iterations;
  };

  /// Experimental.
  uint32_t startMulti_B()
  {
    _is_A = false;
    _iterations = _backend->getIterations_B();
    _startTime = micros();
    return _iterations;
  };

private:
  // In case the usermod is not available, we are acting as null-object (mocking the real backend).
  bool isSelected_A() override { return true; }
  uint32_t getIterations_A() override { return 1; }
  uint32_t getIterations_B() override { return 1; }
  void addTestRun(uint32_t, uint32_t, bool, Segment &) override {}

private:
  EffectProfilerBackend *_backend;
  bool _is_A;
  uint32_t _iterations;
  uint32_t _startTime;
};

//--------------------------------------------------------------------------------------------------

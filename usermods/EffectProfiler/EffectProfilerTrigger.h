/**
 * Blablabla.
 */

#pragma once

#include "wled.h"

//--------------------------------------------------------------------------------------------------

/// Internal interface to the EffectProfiler usermod.
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

class EffectProfilerTrigger : private EffectProfilerBackend
{
public:
  explicit EffectProfilerTrigger()
  {
    UM_Exchange_Data *um_data;
    if (UsermodManager::getUMData(&um_data, USERMOD_ID_EFFECT_PROFILER))
    {
      _backend = static_cast<EffectProfilerBackend *>(um_data->u_data[0]);
    }
    else
    {
      _backend = this;
    }
    startSingle_A();
  }

  ~EffectProfilerTrigger() { stop(); }

  /// For A-B testing: A is the _A_dvanced new stuff, B is the existing _B_aseline to compare against.
  bool mustRun_A() { return _backend->isSelected_A(); }

  void startSingle_A()
  {
    _is_A = true;
    _iterations = 1;
    _startTime = micros();
  };

  void startSingle_B()
  {
    _is_A = false;
    _iterations = 1;
    _startTime = micros();
  };

  uint32_t startMulti_A()
  {
    _is_A = true;
    _iterations = _backend->getIterations_A();
    _startTime = micros();
    return _iterations;
  };

  uint32_t startMulti_B()
  {
    _is_A = false;
    _iterations = _backend->getIterations_B();
    _startTime = micros();
    return _iterations;
  };

  void stop()
  {
    const uint32_t endTime = micros();
    if (_startTime)
    {
      _backend->addTestRun(endTime - _startTime, _iterations, _is_A, SEGMENT);
      _startTime = 0;
    }
  };

private:
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

/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"

//--------------------------------------------------------------------------------------------------

/** Dummy usermod implementation that simulates random sensor readings.
 * @note This usermod doesn't override getId() because it doesn't interact with the outside world.
 */
class UM_DummySensor : public Usermod, public TemperatureSensor
{
  void setup() override
  {
    // register sensor plugin
    UsermodManager::registerTemperatureSensor(*this, "Dummy");
  }

  void loop() override {}

  float temperatureC() override { return readTemp(); }

  float readTemp()
  {
    const int32_t temp_raw = perlin16(strip.now * 64) - 0x8000;
    // simulate some random temperature around 20°C
    return 20.0f + temp_raw / 65535.0f * 20.0f;
  }
};

//--------------------------------------------------------------------------------------------------

static UM_DummySensor _DummySensor;
REGISTER_USERMOD(_DummySensor);

/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"

//--------------------------------------------------------------------------------------------------

/** Dummy usermod implementation that simulates random sensor readings.
 * @note This usermod doesn't override getId() because it doesn't directly interact with the outside world.
 */
class UM_DummySensor : public Usermod, public TemperatureSensor, public HumiditySensor
{
  void setup() override
  {
    // register sensor plugins
    pluginManager.registerTemperatureSensor(*this, "Dummy");
    pluginManager.registerHumiditySensor(*this, "Dummy");
  }

  void loop() override {}

  /// @copydoc TemperatureSensor::do_getTemperatureC()
  float do_getTemperatureC() override { return readTemperature(); }

  /// @copydoc HumiditySensor::do_getHumidityC()
  float do_getHumidity() override { return readHumidity(); }

  /// The dummy implementation to simulate temperature values (based on perlin noise).
  float readTemperature()
  {
    const int32_t raw = perlin16(strip.now * 8) - 0x8000;
    // simulate some random temperature around 20°C
    return 20.0f + raw / 65535.0f * 30.0f;
  }

  /// The dummy implementation to simulate humidity values (a sine wave).
  float readHumidity()
  {
    const int32_t raw = beatsin16_t(1);
    // simulate some random humidity between 10% and 90%
    return 10.0f + raw / 65535.0f * 80.0f;
  }
};

//--------------------------------------------------------------------------------------------------

static UM_DummySensor um_DummySensor;
REGISTER_USERMOD(um_DummySensor);

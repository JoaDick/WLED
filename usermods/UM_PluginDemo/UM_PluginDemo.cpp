/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"

// TODO(feature) Make this configurable via UI
#define DEFAULT_PIN 8 // Pin number of a relay or LED.

extern uint16_t mode_static(void);

//--------------------------------------------------------------------------------------------------

/** Example effect for processing the readings of a TemperatureSensor and a HumiditySensor plugin.
 */
uint16_t mode_UmExampleThermometer()
{
  // try to get a TemperatureSensor - and bail out if that fails
  TemperatureSensor *tempSensor = pluginManager.getTemperatureSensor();
  if (!tempSensor)
    return mode_static();

  SEGMENT.clear();

  // read the temperature value from the plugin
  const float temp = tempSensor->temperatureC();
  // and draw a representing bar in the fx color
  int tempPos = temp * (SEGLEN - 1) / 40.0f; // 20° shall be in the middle
  while (tempPos >= 0)
    SEGMENT.setPixelColor(tempPos--, fast_color_scale(SEGCOLOR(0), 64));

  // draw some dots as scale
  SEGMENT.setPixelColor(SEGLEN / 4, 0x8080F8);     // 10°C
  SEGMENT.setPixelColor(SEGLEN / 2, 0x808080);     // 20°C
  SEGMENT.setPixelColor(SEGLEN * 3 / 4, 0xF88080); // 30°C

  // try to get a HumiditySensor
  HumiditySensor *humSensor = pluginManager.getHumiditySensor();
  if (humSensor)
  {
    // read the humidity value from the plugin (if one exists)
    const float hum = humSensor->humidity();
    // and draw a representing dot in magenta
    const int humPos = hum * (SEGLEN - 1) / 100.0f;
    SEGMENT.setPixelColor(humPos, 0xFF00FF);
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_EX_UM_THERMOMETER[] PROGMEM = "Ex: Thermometer@;!";

//--------------------------------------------------------------------------------------------------

/** A usermod for plugin examples.
 */
class UM_PluginDemo : public Usermod, public PinUser
{
  // ----- usermod functions -----

  void setup() override
  {
    registerEffects();
    registerPins();
  }

  void loop() override
  {
    processFanControl();
  }

  // ----- initialization helper functions -----

  void registerEffects()
  {
    strip.addEffect(255, &mode_UmExampleThermometer, _data_FX_MODE_EX_UM_THERMOMETER);
  }

  void registerPins()
  {
    _pinConfig.pinNr = DEFAULT_PIN;
    if (pluginManager.registerPinUser(*this, _pinConfig, "PluginDemo"))
    {
      pinMode(_pinConfig.pinNr, OUTPUT);
    }
  }

  // ----- processing functions -----

  /// Just an example for custom plugin sensor data processing.
  void processFanControl()
  {
    // bail out if we didn't get a GPIO
    if (!_pinConfig.isPinValid())
      return;

    // try to get a TemperatureSensor - and bail out if that fails
    TemperatureSensor *tempSensor = pluginManager.getTemperatureSensor();
    if (!tempSensor)
      return;

    // read the temperature value from the other plugin
    const float temp = tempSensor->temperatureC();
    // control a connected fan
    const bool isHot = temp > 20.0f;
    digitalWrite(_pinConfig.pinNr, isHot ? HIGH : LOW);
  }

  // ----- member variables -----

  PinConfig _pinConfig{PinType::Digital_out, "Relay"};
};

//--------------------------------------------------------------------------------------------------

static UM_PluginDemo um_PluginDemo;
REGISTER_USERMOD(um_PluginDemo);

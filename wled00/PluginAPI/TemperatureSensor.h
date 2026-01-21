/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

//--------------------------------------------------------------------------------------------------

/// Interface of a sensor that can provide tempreature readings.
class TemperatureSensor
{
public:
  /** Determines the unit for \c temperature() - default is °C
   * @see PluginManager::setUseFahrenheit()
   */
  static bool useFahrenheit() { return _useFahrenheit; }

  /// Get the temperature, with the unit according to the global setting of \a useFahrenheit()
  float temperature() { return useFahrenheit() ? temperatureF() : temperatureC(); }

  /// Get the temperature in °C
  float temperatureC() { return do_getTemperatureC(); }

  /// Get the temperature in °F
  float temperatureF() { return do_getTemperatureC() * 9.0f / 5.0f + 32.0f; }

protected:
  /// Get the plugin's temperature reading in °C
  virtual float do_getTemperatureC() = 0;

private:
  friend class PluginManager;
  static bool _useFahrenheit;
};

//--------------------------------------------------------------------------------------------------

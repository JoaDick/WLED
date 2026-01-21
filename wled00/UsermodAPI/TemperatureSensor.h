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
  /// Determines the unit for \c temperature()
  bool useFahrenheit = false;

  /// Get the temperature, with the unit selected via \a useFahrenheit
  float temperature() { return useFahrenheit ? temperatureF() : temperatureC(); }

  /// Get the temperature in °F
  float temperatureF() { return temperatureC() * 1.8f + 32.0f; }

  /// Get the temperature in °C
  virtual float temperatureC() = 0;
};

//--------------------------------------------------------------------------------------------------

/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

//--------------------------------------------------------------------------------------------------

/// Interface of a sensor that can provide humidity readings.
class HumiditySensor
{
public:
  /// Get the humidity in % rel.
  float humidity() { return do_getHumidity(); }

protected:
  /// Get the plugin's humidity reading in % rel.
  virtual float do_getHumidity() = 0;
};

//--------------------------------------------------------------------------------------------------

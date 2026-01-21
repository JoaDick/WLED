/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include <map>

#include "PinUser.h"
#include "HumiditySensor.h"
#include "TemperatureSensor.h"

//--------------------------------------------------------------------------------------------------

/** The central component that orchestrates all plugins.
 * @note All strings (names) that are provided from the plugins are only pointer-copied.
 * They must remain valid as long as the plugin is registered!
 */
class PluginManager
{
public:
  // no copy & move
  PluginManager(const PluginManager &) = delete;
  PluginManager &operator=(const PluginManager &) = delete;
  PluginManager() = default;

  // ----- PinUser plugins -----

  /** Try to register the desired pins of a plugin.
   * @note Use this method also fur updating the pin configuration (even if already registered).
   */
  bool registerPinUser(PinUser &user, uint8_t pinCount, PinConfig *pinConfig, const char *pluginName);

  /// Convenience method for registering one single pin.
  bool registerPinUser(PinUser &user, PinConfig &pinConfig, const char *pluginName)
  {
    return registerPinUser(user, 1, &pinConfig, pluginName);
  }

  /// Revoke all of a plugin's registered pins.
  void unregisterPinUser(PinUser &user);

  // ----- TemperatureSensor plugins -----

  /// Register a plugin as a TemperatureSensor.
  void registerTemperatureSensor(TemperatureSensor &sensor, const char *pluginName);

  /// Revoke a plugin's registration as TemperatureSensor.
  void unregisterTemperatureSensor(TemperatureSensor &sensor);

  /// Get a registered TemperatureSensor or \c nullptr if there are none.
  TemperatureSensor *getTemperatureSensor();

  /** Globally set the unit for \c TemperatureSensor::temperature() - default is °C
   * @see This setting should only be configured via UI.
   */
  static void setUseFahrenheit(bool enabled);

  // ----- HumiditySensor plugins -----

  /// Register a plugin as a HumiditySensor.
  void registerHumiditySensor(HumiditySensor &sensor, const char *pluginName);

  /// Revoke a plugin's registration as HumiditySensor.
  void unregisterHumiditySensor(HumiditySensor &sensor);

  /// Get a registered HumiditySensor or \c nullptr if there are none.
  HumiditySensor *getHumiditySensor();

private:
  bool rollbackPinRegistration(PinUser &user, uint8_t pinCount, PinConfig *pinConfig);

  // TODO(optimization) To save precious DRAM, use a std::pmr::map/multimap with a memory resource
  // that allocates PSRAM instead. Unfortunately, that is a C++17 feature...
  std::map<PinUser *, const char *> _pinUsers;
  std::multimap<PinUser *, PinConfig> _pinUserConfigs;

  std::map<TemperatureSensor *, const char *> _temperatureSensors;

  std::map<HumiditySensor *, const char *> _humiditySensors;
};

/// The global PluginManager instance.
extern PluginManager pluginManager;

//--------------------------------------------------------------------------------------------------

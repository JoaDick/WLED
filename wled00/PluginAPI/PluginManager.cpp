/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "PluginManager.h"

//--------------------------------------------------------------------------------------------------

const char *getPinName(PinType pinType)
{
  switch (pinType)
  {
  case PinType::Digital_in:
    return "Digital in";
  case PinType::Digital_out:
    return "Digital out";
  case PinType::Analog_in:
    return "Analog in";
  case PinType::PWM_out:
    return "PWM out";
  case PinType::I2C_scl:
    return "I2C SCL";
  case PinType::I2C_sda:
    return "I2C SDA";
  case PinType::SPI_sclk:
    return "SPI SCLK";
  case PinType::SPI_mosi:
    return "SPI MOSI";
  case PinType::SPI_miso:
    return "SPI MISO";
  case PinType::OneWire:
    return "OneWire";
  default:
    return "[???]";
  }
}

bool isOutputPin(PinType pinType)
{
  switch (pinType)
  {
  // case PinType::Digital_in:
  case PinType::Digital_out:
  // case PinType::Analog_in:
  case PinType::PWM_out:
  case PinType::I2C_scl:
  case PinType::I2C_sda:
  case PinType::SPI_sclk:
  case PinType::SPI_mosi:
  // case PinType::SPI_miso:
  case PinType::OneWire:
    return true;
  default:
    return false;
  }
}

//--------------------------------------------------------------------------------------------------

void PluginManager::setUseFahrenheit(bool enabled) { TemperatureSensor::_useFahrenheit = enabled; }

bool PluginManager::registerPinUser(PinUser &user, uint8_t pinCount, PinConfig *pinConfig, const char *pluginName)
{
  unregisterPinUser(user); // always remove previous registration

  auto begin = pinConfig;
  auto end = begin + pinCount;
  for (auto itr = begin; itr != end; ++itr)
  {
    if (itr->pinName == nullptr)
      itr->pinName = getPinName(itr->pinType);
    if (!itr->isPinValid())
      return rollbackPinRegistration(user, pinCount, pinConfig);
    if (PinManager::allocatePin(itr->pinNr, isOutputPin(itr->pinType), PinOwner::PluginMgr) == false)
      return rollbackPinRegistration(user, pinCount, pinConfig);
    _pinUserConfigs.insert({&user, *itr});
  }

  _pinUsers[&user] = pluginName;
  return true;
}

bool PluginManager::rollbackPinRegistration(PinUser &user, uint8_t pinCount, PinConfig *pinConfig)
{
  unregisterPinUser(user);
  auto begin = pinConfig;
  auto end = begin + pinCount;
  for (auto itr = begin; itr != end; ++itr)
    itr->invalidatePin();
  return false;
}

void PluginManager::unregisterPinUser(PinUser &user)
{
  const auto configs = _pinUserConfigs.equal_range(&user);
  for (auto itr = configs.first; itr != configs.second; ++itr)
  {
    const auto &pinConfig = itr->second;
    PinManager::deallocatePin(pinConfig.pinNr, PinOwner::PluginMgr);
  }
  _pinUserConfigs.erase(&user);
  _pinUsers.erase(&user);
}

void PluginManager::registerTemperatureSensor(TemperatureSensor &sensor, const char *pluginName)
{
  _temperatureSensors[&sensor] = pluginName;
}

void PluginManager::unregisterTemperatureSensor(TemperatureSensor &sensor)
{
  _temperatureSensors.erase(&sensor);
}

TemperatureSensor *PluginManager::getTemperatureSensor()
{
  // TODO(feature) Pick a UI selectable default sensor from the map
  return _temperatureSensors.empty() ? nullptr : _temperatureSensors.begin()->first;
}

void PluginManager::registerHumiditySensor(HumiditySensor &sensor, const char *pluginName)
{
  _humiditySensors[&sensor] = pluginName;
}

void PluginManager::unregisterHumiditySensor(HumiditySensor &sensor)
{
  _humiditySensors.erase(&sensor);
}

HumiditySensor *PluginManager::getHumiditySensor()
{
  // TODO(feature) Pick a UI selectable default sensor from the map
  return _humiditySensors.empty() ? nullptr : _humiditySensors.begin()->first;
}

//--------------------------------------------------------------------------------------------------

bool TemperatureSensor::_useFahrenheit = false;

PluginManager pluginManager;

//--------------------------------------------------------------------------------------------------

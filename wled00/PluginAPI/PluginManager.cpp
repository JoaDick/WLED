/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include <algorithm>
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
    if (!PinManager::allocatePin(itr->pinNr, isOutputPin(itr->pinType), PinOwner::PluginMgr))
      return rollbackPinRegistration(user, pinCount, pinConfig);
    _pinUserConfigs.emplace_back(&user, *itr);
  }

  _pinUsers.emplace_back(&user, pluginName);
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
  for (const auto &entry : _pinUserConfigs)
  {
    const auto &pinConfig = entry.second;
    PinManager::deallocatePin(pinConfig.pinNr, PinOwner::PluginMgr);
  }

  auto pred1 = [&user](const PinUserConfigs::value_type &entry)
  { return entry.first == &user; };
  std::remove_if(_pinUserConfigs.begin(), _pinUserConfigs.end(), pred1);

  auto pred2 = [&user](const PinUsers::value_type &entry)
  { return entry.first == &user; };
  std::remove_if(_pinUsers.begin(), _pinUsers.end(), pred2);
}

void PluginManager::registerTemperatureSensor(TemperatureSensor &sensor, const char *pluginName)
{
  unregisterTemperatureSensor(sensor);
  _temperatureSensors.emplace_back(&sensor, pluginName);
}

void PluginManager::unregisterTemperatureSensor(TemperatureSensor &sensor)
{
  auto pred = [&sensor](const TemperatureSensors::value_type &entry)
  { return entry.first == &sensor; };
  std::remove_if(_temperatureSensors.begin(), _temperatureSensors.end(), pred);
}

TemperatureSensor *PluginManager::getTemperatureSensor()
{
  // TODO(feature) Select a default sensor via UI and return that one.
  return _temperatureSensors.empty() ? nullptr : _temperatureSensors.front().first;
}

void PluginManager::registerHumiditySensor(HumiditySensor &sensor, const char *pluginName)
{
  unregisterHumiditySensor(sensor);
  _humiditySensors.emplace_back(&sensor, pluginName);
}

void PluginManager::unregisterHumiditySensor(HumiditySensor &sensor)
{
  auto pred = [&sensor](const HumiditySensors::value_type &entry)
  { return entry.first == &sensor; };
  std::remove_if(_humiditySensors.begin(), _humiditySensors.end(), pred);
}

HumiditySensor *PluginManager::getHumiditySensor()
{
  // TODO(feature) Select a default sensor via UI and return that one.
  return _humiditySensors.empty() ? nullptr : _humiditySensors.front().first;
}

//--------------------------------------------------------------------------------------------------

bool TemperatureSensor::_useFahrenheit = false;

PluginManager pluginManager;

//--------------------------------------------------------------------------------------------------

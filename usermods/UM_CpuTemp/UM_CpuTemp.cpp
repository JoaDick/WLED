/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"

/*

https://github.com/PelicanHu/ESPCPUTemp

260203: Not working with ESP32-C3

---- Opened the serial port COM4 ----
ESP-ROM:esp32c3-api1-20210207
Build:Feb  7 2021
rst:0x3 (RTC_SW_SYS_RST),boot:0xd (SPI_FAST_FLASH_BOOT)
Saved PC:0x40383334
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fcd5810,len:0x48
load:0x403cc710,len:0x650
load:0x403ce710,len:0x2228
entry 0x403cc710
New driver not available for ESP32-S2/S3/C3/C6/H2
ESP CPU temperature sensor initialization failed
Ada

*/

class UM_CpuTemp : public Usermod
{
public:
  void setup() override { _tempSensor.begin(); }

  void loop() override
  {
    const auto now = millis();
    if (now < _nextUpdateTime)
      return;
    readSensor();
    _nextUpdateTime = now + 1000;
  }

  uint8_t getSensorCount() override { return 1; }
  Sensor *getSensor(uint8_t index) override { return &_sensorValue; }

private:
  void readSensor()
  {
    if (!_tempSensor.tempAvailable())
    {
      // _sensorValue = -1.1f;
      _sensorValue.suspendSensor();
      return;
    }

    const float temp = _tempSensor.getTemp();
    if (isnan(temp))
    {
      // _sensorValue = -2.2f;
      _sensorValue.suspendSensor();
      return;
    }

    _sensorValue = temp;
  }

private:
  EasySensor _sensorValue{ESP.getChipModel(), makeChannelProps_Temperature("CPU-Temp", 0.0f, 80.0f)};
  ESPCPUTemp _tempSensor;
  uint32_t _nextUpdateTime = 0;
};

static UM_CpuTemp um_CpuTemp;
REGISTER_USERMOD(um_CpuTemp);

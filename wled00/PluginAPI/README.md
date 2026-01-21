# Interfaces of WLED's usermod plugin framework.

These interfaces are very high-level by intentional design choice - and they are defined the WLED
framework only. <br>
This here is **not** the place for specialized custom stuff!

Potential additional interfaces:
- HumiditySensor with `float humidity()` in %
- BatterySensor with `uint16_t batteryLevel()` with range 0 ... 1000 representing 0.0 ... 100.0%
- TimeProvider with year/month/day & hour/minute/second (localtime; without DST and timezone)
  - With usermod implementations based on I2C or OneWire RTC or NTP or DCF77 or ...
- AudioSensor with ...
    - ... be careful to stay generic and flexible!
- ... ?

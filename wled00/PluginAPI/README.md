# Plugin framework for WLED's usermods.

These interfaces here are very high-level by explicit design choice - and they are administrated
by the WLED framework only. <br>
This directory here is **not** the place for specialized custom interfaces - use the subdirectory
`custom` for that.

Potential additional interfaces:
- BatterySensor with `uint16_t batteryLevel()`, with range 0 ... 1000 representing 0.0 ... 100.0%
- TimeProvider with `getTime()`, returning s struct of `year/month/day` & `hour/minute/second`
  (localtime; without DST and timezone).
  - With usermod implementations, based on I2C or OneWire RTC or NTP or DCF77 or ...
- AudioSensor with ...
    - ... be careful to stay generic with these interfaces!
- ... ?

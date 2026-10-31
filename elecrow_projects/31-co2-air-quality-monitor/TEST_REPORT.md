# Test Report: Project 31 - CO2 Indoor Air Quality Monitor

## System Information
- **Target Hardware**: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
- **Target Channel**: IamNotIntoTech (`@IamNotIntoTech`)
- **Author**: Tushar Aggarwal (tushar2704)
- **Date**: October 31, 2026

## Touch Controller & Coordinate Verification
- **Touch Controller**: XPT2046 over dedicated VSPI (`MOSI=32`, `MISO=39`, `CLK=25`, `CS=33`)
- **Calibration Mapping**:
  - `screenX = map(p.x, 240, 3800, 0, 320)`
  - `screenY = map(p.y, 240, 3800, 0, 240)` (Non-inverted Y axis)
- **Debounce Threshold**: 180 ms minimum lockout with pressure detection `p.z > 50`
- **Bottom Touch Bar (screenY >= 205)**:
  - Unit Toggle (`UNIT: C / F`): `screenX = 10..100`
  - Alarm Mute (`ALARM: ON / MUTED`): `screenX = 115..205`
  - Manual Refresh (`REFRESH`): `screenX = 220..310`

## Sensor & Bus Verification
- Hardware I2C configured on GPIO 27 (SDA) and GPIO 22 (SCL) at 100 kHz
- CRC-8 checksum validation for 3-byte Sensirion SCD4x telemetry frames
- Graceful automatic failover to dynamic ambient simulation mode upon missing I2C ACK
- Visual/acoustic alert dispatch with 400ms flash cadence on critical thresholds

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Build Status**: PlatformIO build clean (Success code 0)


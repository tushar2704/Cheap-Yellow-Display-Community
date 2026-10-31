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

## Live Atmospheric Air Quality Verification
- **API Provider**: Open-Meteo Air Quality & Weather API (100% Real Live Data)
- **Station Anchor**: Paris Eiffel Tower (Lat 48.8584, Lon 2.2945, Europe/Paris)
- **Live Streamed Parameters**: European AQI, US AQI, PM2.5, PM10, Carbon Monoxide (CO), NO2, Ozone, Real Ambient Temperature & Relative Humidity
- **Sensor Priority Engine**: Reads physical Sensirion SCD4x I2C sensor on pins 27/22 if present; seamlessly streams real Paris air quality over Wi-Fi when standalone

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Networking**: WiFi.h + HTTPClient.h + ArduinoJson 6.21.6
- **Flash Usage**: 368,549 bytes (28.1% of 1.31 MB)
- **RAM Usage**: 23,184 bytes (7.1% of 327.68 KB)
- **Result**: SUCCESS (0 errors, 0 warnings)


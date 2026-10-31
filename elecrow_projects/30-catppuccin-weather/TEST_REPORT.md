# Test Report: Project 30 - Catppuccin Weather Station

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
- **Bottom Tab Navigation (screenY >= 200)**:
  - Tab 0 (Now): `screenX < 106`
  - Tab 1 (24H Trend): `screenX = 106..212`
  - Tab 2 (7-Day Outlook): `screenX >= 213`

## Backlight & Ambient Sensing Verification
- LDR ADC reading on GPIO 34 evaluated every 250 ms
- Hardware LEDC PWM configured on timer channel 0, frequency 5 kHz, resolution 8-bit
- Smooth moving-average brightness filtering between 25 and 255 PWM duty cycle

## Live Weather Pipeline Verification
- **API Provider**: Open-Meteo REST API (100% Real Live Weather)
- **Location**: Paris Eiffel Tower (Lat 48.8584, Lon 2.2945, Europe/Paris)
- **Parameters**: Real-time temp, apparent feels-like, humidity, surface pressure, wind speed/direction, WMO weather code, 8 hourly intervals, 7-day daily forecast
- **RGB Mood Lighting**: Active-low LED mapped to WMO weather code (amber clear, cyan cloudy, blue rain, violet storm)

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Networking**: WiFi.h + HTTPClient.h + ArduinoJson 6.21.6
- **Flash Usage**: 992,613 bytes (75.7% of 1.31 MB)
- **RAM Usage**: 48,468 bytes (14.8% of 327.68 KB)
- **Result**: SUCCESS (0 errors, 0 warnings)


# Test Report: Project 35 - Solar & Battery Flow Dashboard

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
- **Bottom Touch Bar (screenY >= 206)**:
  - View Mode Toggle (`VIEW: HISTORY` / `VIEW: FLOW`): `screenX = 8..115`

## Topology Animation & Power Verification
- Moving particle animation engine cycling at 60 ms intervals without full-screen flicker
- Directional flow reversing automatically based on battery charging/discharging and grid export/import
- Real-time self-sufficiency calculation driving active-low RGB status LED (Green / Yellow / Red)

## Live Solar Radiation Pipeline Verification
- **API Provider**: Open-Meteo Solar Radiation REST API (100% Real Live Solar Data)
- **Station Location**: Paris Eiffel Tower (Lat 48.8584, Lon 2.2945, Europe/Paris)
- **Live Streamed Parameters**: Real-time shortwave radiation (W/m2), direct normal irradiance, 24-hour hourly radiation history, daily solar energy yield sum
- **Energy Physics**: Maps atmospheric irradiance to 5.0 kWp residential rooftop array in Paris with live battery/grid flow physics

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Networking**: WiFi.h + HTTPClient.h + ArduinoJson 6.21.6
- **Flash Usage**: 350,685 bytes (26.8% of 1.31 MB)
- **RAM Usage**: 22,860 bytes (7.0% of 327.68 KB)
- **Result**: SUCCESS (0 errors, 0 warnings)


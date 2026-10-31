# Test Report: Project 19 - Wi-Fi Channel & Spectrum Auditor

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
- **Bottom Touch Controls (screenY >= 195)**:
  - `RESCAN`: `screenX < 105`
  - `SPECTRUM / AP TABLE`: `screenX = 110..209`
  - `USE CH X`: `screenX >= 215`

## Radio & Spectrum Verification
- **Radio Band**: 802.11 b/g/n 2.4GHz
- **Channels**: 1 through 14
- **Algorithm**: Evaluates primary channels (1, 6, 11) using adjacent channel weighting and RSSI penalty functions.

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Networking**: WiFi.h

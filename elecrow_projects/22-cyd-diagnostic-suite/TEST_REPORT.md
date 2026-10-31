# Test Report: Project 22 - CYD Hardware Factory Diagnostic Suite

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
- **Bottom Navigation Tabs (screenY >= 200)**:
  - `SYSTEM`: `screenX = 8..85`
  - `PIXELS`: `screenX = 86..163`
  - `TOUCH`: `screenX = 164..241`
  - `BENCH`: `screenX = 242..319`

## Bench Controls (Bench Screen)
- Audio Sweep (300Hz–3.2kHz): `screenX = 8..156, screenY = 92..142`
- RGB LED Test: `screenX = 164..312, screenY = 92..142`

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)


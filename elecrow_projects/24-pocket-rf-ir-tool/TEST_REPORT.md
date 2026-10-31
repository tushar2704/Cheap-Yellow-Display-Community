# Test Report: Project 24 - Pocket RF & IR Universal Multi-Tool

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
  - `IR REMOTE`: `screenX = 8..111`
  - `IR SNIFFER`: `screenX = 112..215`
  - `WI-FI QR`: `screenX = 216..319`

## Remote & Sniffer Matrix
- Keypad Grid (Mode 0): 3 columns x 2 rows (`screenX = 10..316, screenY = 35..143`)
- Sniffer Action Buttons (Mode 1):
  - Learn/Sniff: `screenX < 156, screenY = 166..200`
  - Replay Signal: `screenX >= 164, screenY = 166..200`

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **QR Engine**: qrcode library


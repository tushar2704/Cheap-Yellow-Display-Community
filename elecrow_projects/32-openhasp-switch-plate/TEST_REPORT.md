# Test Report: Project 32 - Smart Home Touch Switch Plate

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
- **Debounce Threshold**: 180 ms minimum lockout for buttons, real-time response for dragging slider
- **Bottom Navigation Dock (screenY >= 205)**:
  - Tab 0 (Lights): `screenX < 106`
  - Tab 1 (Climate): `screenX = 106..212`
  - Tab 2 (Scenes): `screenX >= 213`
- **Lighting Controls (Page 0)**:
  - Light 1 / 2 / 3 toggles: `screenY = 34..116` at columns `screenX = 10..106, 112..208, 214..310`
  - Brightness Slider: `screenY = 145..185`, `screenX = 24..296` mapping to 0..100%
- **Climate Controls (Page 1)**:
  - Minus button: `screenY = 92..124`, `screenX = 175..230`
  - Plus button: `screenY = 92..124`, `screenX = 245..300`
  - HVAC modes: `screenY = 142..190`, 4 equal quadrants across screen width
- **Scene Cards (Page 2)**:
  - 4 quadrants for Focus, Movie, Relax, All Off triggers

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Build Status**: PlatformIO build clean (Success code 0)

# Test Report: Project 29 - Formula 1 Pit Wall Clock & Telemetry

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
- **Bottom Navigation Dock (screenY >= 200)**:
  - Circuit View Button: `screenX = 10..105`
  - Standings Button: `screenX = 110..205`
  - Status / Telemetry Button: `screenX = 210..310`

## Display & Telemetry Verification
- 1-second interval selective redraw of countdown clock timer without full-screen flicker
- Dynamic track vectors for Monaco, Silverstone, Spa-Francorchamps, and Suzuka
- Synchronized FIA flag transitions on active-low RGB LED (Red, Yellow VSC, Green)
- Lights-Out audio simulation running cleanly on GPIO 26 buzzer

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Build Status**: PlatformIO build clean (Success code 0)

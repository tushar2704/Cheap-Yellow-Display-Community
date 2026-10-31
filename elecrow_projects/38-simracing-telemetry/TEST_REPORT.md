# Test Report: Project 38 - Sim-Racing Telemetry Dash

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
- **Bottom Macro Deck (screenY >= 198)**:
  - Discord Mute Macro (`DISCORD ON` / `MUTED`): `screenX = 8..110`
  - Speed Unit Toggle (`UNIT: KM/H` / `MPH`): `screenX = 116..205`
  - Pit Limiter Toggle (`PIT LIMITER ON` / `OFF`): `screenX = 214..312`

## Dashboard & Animation Verification
- 50 ms loop execution cycle ensuring 20 FPS real-time telemetry updates
- 16-segment RPM shift bar color grading responding smoothly across rev range
- Peripheral RGB LED active-low flashing synchronizing to gear shift point
- Acoustic shift chirp firing cleanly on GPIO 26 buzzer

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Build Status**: PlatformIO build clean (Success code 0)

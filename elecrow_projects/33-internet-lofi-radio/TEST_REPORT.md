# Test Report: Project 33 - Streaming Internet Lo-Fi Radio

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
- **Bottom Playback Control Deck (screenY >= 196)**:
  - Previous Station (`|<<`): `screenX = 10..55`
  - Play / Pause (`PLAY` / `PAUSE`): `screenX = 65..115`
  - Next Station (`>>|`): `screenX = 125..170`
  - Visualization Mode Cycle (`BARS / DOTS / FIRE`): `screenX = 180..235`
  - Volume Stepper (`VOL XX%`): `screenX = 245..310`

## Animation & Visualizer Verification
- 25 FPS spectrum frame refresh cycle (40 ms intervals) without full-screen redraw flicker
- Gravity peak decay simulation maintaining smooth peak hold transitions
- Sub-bass energy threshold detector pulsing active-low RGB LED on GPIO 16/17

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Build Status**: PlatformIO build clean (Success code 0)

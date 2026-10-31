# Test Report: Project 28 - Spotify Album Art & Media Remote

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
- **Bottom Media Buttons (screenY >= 195)**:
  - Previous (`|<<`): `screenX < 65`
  - Play / Pause: `screenX = 68..148`
  - Next (`>>|`): `screenX = 150..210`
  - Volume Down (`-`): `screenX = 212..262`
  - Volume Up (`+`): `screenX >= 264`

## Display & Flicker-Free Verification
- Selective dirty rectangle redraw on progress bar every 1s
- Zero full-screen redraw artifacts during steady-state playback

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)


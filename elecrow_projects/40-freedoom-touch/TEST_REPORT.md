# Test Report: Project 40 - Playable No-PSRAM FreeDOOM

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
- **Debounce Threshold**: 15 ms frame rate responsive touch loop
- **Viewport Controls (screenY < 190)**:
  - Turn Left: `screenX < 100`
  - Move Forward: `screenX = 100..220`
  - Turn Right: `screenX > 220`
- **HUD D-Pad & Fire Button (screenY >= 190)**:
  - Turn Left (`<`): `screenX = 195..222`
  - Forward (`^`): `screenX = 223..252`, `screenY < 216`
  - Backward (`v`): `screenX = 223..252`, `screenY >= 216`
  - Turn Right (`>`): `screenX = 253..280`
  - Fire Weapon (`FIRE`): `screenX >= 280`

## Raycasting & SRAM Verification
- DDA algorithm casts 80 rays per frame using less than 24 KB of dynamic heap
- Distance attenuation fog verified without floating-point performance bottlenecks
- Active-low Muzzle Flash LED on GPIO 4/16/17 synchronized with acoustic discharge

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Build Status**: PlatformIO build clean (Success code 0)

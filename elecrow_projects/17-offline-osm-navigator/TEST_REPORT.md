# Test Report: Project 17 - Offline OSM Trail Navigator & Topo Deck

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
- **Right Sidebar Glove Buttons (screenX >= 260)**:
  - Zoom In (+): `screenY = 30..64`
  - Pan Up (▲): `screenY = 66..99`
  - Pan Down (▼): `screenY = 101..134`
  - Zoom Out (-): `screenY = 136..169`
  - Waypoint Pin (WP): `screenY = 171..204`
- **Canvas Touch Pan (screenX < 260, screenY: 27..205)**: Directly translates center towards tap point.

## MicroSD SPI Verification
- Dedicated HSPI bus (`CS=5`, `SCK=18`, `MISO=19`, `MOSI=23`)
- Safe initialization without interfering with VSPI touch or ILI9341 display bus

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Storage**: SD.h + FS.h (HSPI)

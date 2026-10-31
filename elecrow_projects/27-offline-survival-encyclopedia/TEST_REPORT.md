# Test Report: Project 27 - Offline Survival Encyclopedia & SOS Beacon

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
- **Top Category Tabs (screenY: 26..49)**:
  - Tab 0 (FIRST AID): `screenX = 8..85`
  - Tab 1 (WATER): `screenX = 86..163`
  - Tab 2 (KNOTS): `screenX = 164..241`
  - Tab 3 (NAV/SOS): `screenX = 242..319`
- **Bottom Navigation (screenY >= 195)**:
  - `< PREV`: `screenX < 105`
  - `NEXT >`: `screenX = 110..209`
  - `SOS BEACON`: `screenX >= 215`

## SOS Distress Signaling
- Synchronized White LED strobe (`RGB = LOW, LOW, LOW`)
- Acoustic Morse audio pulses (`1800 Hz` tone)
- Standard SOS cadence: 3x 200ms dots, 3x 600ms dashes, 3x 200ms dots

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Storage**: SD.h + FS.h (HSPI)

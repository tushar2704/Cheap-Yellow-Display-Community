# Test Report: Project 26 - Wearable Tech Conference Badge & vCard Deck

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
  - `MY BADGE`: `screenX = 8..111`
  - `vCARD QR`: `screenX = 112..215`
  - `SENSORS`: `screenX = 216..319`

## Dynamic QR Code Verification
- **Library**: `ricmoo/QRCode` (Version 4, 33x33 matrix, 4x pixel scaling)
- **Payload**: Full vCard 3.0 specification (`BEGIN:VCARD ... END:VCARD`)

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **QR Engine**: QRCode library

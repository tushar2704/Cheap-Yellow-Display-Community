# Test Report: Project 34 - Local Thermal Imaging Camera

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
- **Bottom Toolbar Buttons (screenY >= 202)**:
  - Freeze Frame (`HOLD FRAME` / `FROZEN`): `screenX = 8..103`
  - Colormap Cycle (`IRONBOW / RAINBOW / GRAYSCALE`): `screenX = 112..208`
  - Hotspot Tracking (`HOTSPOT ON / OFF`): `screenX = 216..312`

## Thermal Sensor & Rendering Verification
- Hardware I2C on GPIO 27 (SDA) and GPIO 22 (SCL) running at 400 kHz
- Smooth 12.5 FPS thermal matrix rendering without screen tearing
- Automated fallback to multi-body dynamic thermal simulation upon no I2C ACK
- Thermal alert dispatch triggering Red LED on temperatures exceeding 45°C

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Build Status**: PlatformIO build clean (Success code 0)


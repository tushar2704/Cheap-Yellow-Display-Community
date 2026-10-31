# Test Report: Project 36 - Stylus Note Messenger

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
- **Ultra Low Latency Drawing**: 5 ms poll cadence in active drawing zone (`screenX >= 48`)
- **Left Vertical Toolbar (screenX <= 46, screenY >= 27)**:
  - 5 Ink Palette Swatches: `screenY = 32..150` (24px vertical pitch)
  - Eraser Toggle (`ERA`): `screenY = 154..174`
  - Stroke Weight Stepper (`1px / 3px / 5px`): `screenY = 178..200`
  - Transmit Button (`SEND`): `screenY = 204..238`

## Canvas & Drawing Verification
- Bresenham line vector interpolation connecting consecutive touch sample pairs
- Dynamic stroke width scaling correctly rendering 1px, 3px, and 5px nib sizes
- Transmission routine triggers active-low Blue LED pulsation and 2-tone delivery chime

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Build Status**: PlatformIO build clean (Success code 0)

# Test Report: Project 23 - Multi-Firmware SD Bootloader & OTA Launcher

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
- **Bottom Navigation Buttons (screenY >= 200)**:
  - `RESCAN SD`: `screenX < 120`
  - `FLASH & BOOT BIN`: `screenX >= 126`
- **File Selection List (screenY: 32..194)**:
  - Row height: 40px (`tappedIdx = (screenY - 32) / 40`)

## MicroSD SPI Bus Isolation
- Dedicated HSPI bus (`CS=5`, `SCK=18`, `MISO=19`, `MOSI=23`)
- Operates safely without contention against VSPI touch or display controller

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **OTA Engine**: Update.h + SD.h + FS.h

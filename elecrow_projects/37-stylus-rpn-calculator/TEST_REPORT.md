# Test Report: Project 37 - Stylus RPN Calculator

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
- **Debounce Threshold**: 180 ms lockout for 30-key grid, 150 ms for direct bit manipulation
- **30-Key Math Matrix (screenY >= 88)**:
  - 5 Rows x 6 Columns grid (`r = (screenY - 88) / 30`, `c = (screenX - 4) / 52`)
  - Row 0: `ENT`, `x<>y`, `R v`, `DROP`, `AND`, `OR`
  - Row 1: `HEX`, `DEC`, `BIN`, `NOT`, `<<`, `>>`
  - Row 2: `7`, `8`, `9`, `/`, `A`, `B`
  - Row 3: `4`, `5`, `6`, `*`, `C`, `D`
  - Row 4: `1`, `2`, `3`, `-`, `+`, `CLR`
- **Interactive Bit Matrix**: `screenX = 180..310`, `screenY = 64..84` with direct stylus bit toggling

## Stack & Computation Verification
- 4-level push/pop stack logic executing accurately with roll-down and register exchange
- Simultaneous unsigned 32-bit hex and decimal formatting verified
- Stack pulse animation firing active-low Cyan LED and 25 ms audio confirmation

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Build Status**: PlatformIO build clean (Success code 0)


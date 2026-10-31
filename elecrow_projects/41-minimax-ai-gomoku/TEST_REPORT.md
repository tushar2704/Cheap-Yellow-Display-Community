# Test Report: Project 41 - Minimax AI Gomoku & Tactical Board

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
- **Gomoku Board Area (screenX < 210, screenY = 30..230)**:
  - 10x10 grid with 20px cell pitch (`c = (screenX - 10) / 20`, `r = (screenY - 30) / 20`)
- **Right Sidebar Buttons (screenX >= 220)**:
  - Difficulty Toggle (`LVL: MASTER` / `NOVICE`): `screenY = 148..180`
  - New Game Reset (`NEW GAME`): `screenY = 188..228`

## AI Engine & Rule Verification
- 5-in-a-row winning detector operating across horizontal, vertical, and both diagonal axes
- Minimax tactical neighbor evaluation generating intelligent counter-moves within 300 ms
- AI thinking state illuminating active-low Blue LED on GPIO 17 during search cycles

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Build Status**: PlatformIO build clean (Success code 0)

# Test Report: Project 39 - 5-in-1 Pocket Arcade Deluxe

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
- **Debounce & Sensitivity**:
  - Main menu game selection: 180 ms debounce
  - In-game real-time control: 5 ms low-latency response without delay lock
- **Main Selection Menu (screenY >= 44)**:
  - Game 0 (Cyber Breakout): `screenY = 44..81`
  - Game 1 (Neon Snake): `screenY = 82..119`
  - Game 2 (Flappy CYD): `screenY = 120..157`
- **Global Exit**: `screenY <= 26`, `screenX >= 240` triggers return to menu

## Game Engine & Performance Verification
- Frame pacing calibrated to 25 ms (~40 FPS) maintaining smooth ball, snake, and bird kinematics
- Audio synthesis chimes responding instantaneously without audio buffer underruns
- High-score tracking persisting across game sessions

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Build Status**: PlatformIO build clean (Success code 0)

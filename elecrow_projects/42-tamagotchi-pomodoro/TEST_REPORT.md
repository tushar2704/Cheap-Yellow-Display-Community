# Test Report: Project 42 - Tamagotchi Pomodoro Pet

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
- **Bottom Toolbar Buttons (screenY >= 198)**:
  - Timer Start/Pause (`START` / `PAUSE`): `screenX = 8..80`
  - Feed Pet (`FEED`): `screenX = 86..156`
  - Pet Creature (`PET ME`): `screenX = 162..232`
  - Sleep / Wake (`SLEEP` / `WAKE UP`): `screenX = 238..312`
- **Direct Habitat Interaction**: `screenX = 20..170`, `screenY = 40..170`

## Timer & State Machine Verification
- Pomodoro work / short break / long break cycle progression accurately counting down
- Pet sprite frame cadence running at 250 ms (~4 FPS) without full-screen flicker
- Level 2 evolution transition unlocking winged visual sprite at 100 XP
- Active-low RGB mood LED changing hue according to session phase (Focus Red, Break Green, Sleep Blue)

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Build Status**: PlatformIO build clean (Success code 0)


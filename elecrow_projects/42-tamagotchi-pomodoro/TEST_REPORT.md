# Test Report: Project 42 - Tamagotchi Pomodoro Pet

## System Information
- **Target Hardware**: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
- **Target Channel**: IamNotIntoTech (`@IamNotIntoTech`)
- **Author**: Tushar Aggarwal (tushar2704)
- **Station Anchor**: Paris (Eiffel Tower - 48.8584° N, 2.2945° E)
- **Date**: October 31, 2026

## Pomodoro Engine & Virtual Pet Logic
- **Pomodoro Timer Engine**:
  - 25-minute Work Focus sprint (`MODE_WORK`).
  - 5-minute Short Break (`MODE_BREAK`).
  - 15-minute Long Break after 4 completed focus sprints (`MODE_LONG_BREAK`).
- **Pet Evolution & Dynamics**:
  - Level 1: Ground chick form with hopping, blinking, and sleeping Zzz states.
  - Level 2: Evolved celestial form with wings upon reaching 100 XP from completed work sprints.
  - 4 Dynamic Vitals: Health (HP), Happiness (JOY), Hunger/Satiety (FOOD), and XP level.
  - Periodic ambient decay of happiness and hunger during awake state.

## Touch Controller & Coordinate Verification
- **Touch Controller**: XPT2046 over dedicated VSPI (`MOSI=32`, `MISO=39`, `CLK=25`, `CS=33`)
- **Calibration Mapping**:
  - `screenX = map(p.x, 240, 3800, 0, 320)`
  - `screenY = map(p.y, 240, 3800, 0, 240)` (Non-inverted Y axis)
- **Interactive Touch Regions**:
  - **Start / Pause Button (`screenX in [8, 80], screenY >= 198`)**:
    - Starts or pauses Pomodoro timer, toggles mood lighting, produces 2600 Hz / 1600 Hz chime.
  - **Feed Snack Button (`screenX in [86, 156], screenY >= 198`)**:
    - Reduces hunger (-25), increases happiness (+10), plays 3200 Hz tone.
  - **Pet Creature Button (`screenX in [162, 232], screenY >= 198`)**:
    - Increments happiness (+15) and plays chromatic frequency sweep from 1600 to 2800 Hz.
  - **Sleep / Wake Toggle (`screenX in [238, 312], screenY >= 198`)**:
    - Toggles sleep state, stops vital decay, switches LED to Calm Blue.
  - **Direct Pet Touch (`screenX in [20, 170], screenY in [40, 170]`)**:
    - Touching creature directly in habitat increases happiness (+8) with chirping feedback.

## Chromatic Mood Lighting & Acoustic Feedback
- **Active-Low RGB LED (GPIOs 4/16/17)**:
  - Focus Mode: Coral Red
  - Break Mode: Mint Green
  - Sleep Mode: Calm Blue
- **Buzzer (GPIO 26)**:
  - Multi-tone completion fanfares and tactile feedback clicks.

## Wi-Fi & Station Anchor Integration
- **Connection Mode**: 2.5-second non-blocking station attempt to local AP.
- **NTP Time Sync**: Configures Paris timezone (`CET-1CEST,M3.5.0,M10.5.0/3`).
- **Telemetry Header**: Displays "PARIS | #X" streak counter when connected.

## Compilation & Memory Footprint
- **Platform**: Espressif 32 (v7.0.1) / ESP32 Dev Module
- **RAM Usage**: 46,288 bytes (14.1% of 327,680 bytes)
- **Flash Usage**: 812,681 bytes (62.0% of 1,310,720 bytes)
- **Build Status**: 0 errors, 0 warnings (SUCCESS)

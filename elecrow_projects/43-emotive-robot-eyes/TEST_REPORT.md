# Test Report: Project 43 - Procedural Emotive Robot Eyes

## System Information
- **Target Hardware**: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
- **Target Channel**: IamNotIntoTech (`@IamNotIntoTech`)
- **Author**: Tushar Aggarwal (tushar2704)
- **Station Anchor**: Paris (Eiffel Tower - 48.8584° N, 2.2945° E)
- **Date**: October 31, 2026

## Procedural Animation & Emotion Architecture
- **Rendering Performance**: 60 FPS continuous procedural kinematics rendering dual animatronic eyes on ILI9341 display.
- **Micro-Saccades & Autonomous Wander**:
  - Gaze vector with smooth exponential decay interpolation (`0.28f` factor).
  - Autonomous micro-saccades trigger every 2.4 seconds when idle.
- **Natural Blinking Dynamics**:
  - Blinking interval randomized between 2.0 and 5.0 seconds.
  - Smooth eyelid closure/opening progression curve with slit minimum state.
- **6 Expressive Emotional States**:
  1. **CALM / NEUTRAL**: Cyan rounded eyes with steady gaze.
  2. **HAPPY**: Emerald Green eyes with clipped crescent lower smiles.
  3. **CURIOUS**: Lavender / Purple eyes with asymmetrical raised eyebrow.
  4. **ANGRY**: Fiery Red eyes with sharp angled aggressive eyebrows.
  5. **SHOCKED**: Electric Yellow oversized circular dilated pupils.
  6. **SLEEPY**: Dim Cyan-Blue half-lidded heavy eyelids.

## Touch Controller & Coordinate Verification
- **Touch Controller**: XPT2046 over dedicated VSPI (`MOSI=32`, `MISO=39`, `CLK=25`, `CS=33`)
- **Calibration Mapping**:
  - `screenX = map(p.x, 240, 3800, 0, 320)`
  - `screenY = map(p.y, 240, 3800, 0, 240)` (Non-inverted Y axis)
- **Interactive Touch Regions**:
  - **Mood Selection Toolbar (`screenY >= 202`)**:
    - `tappedIdx = (screenX - 4) / 52` selects corresponding mood [0..5].
    - Debounced at 200 ms to prevent accidental repeat toggles.
  - **Dynamic Gaze Following (`screenY < 202`)**:
    - Pupil center targets touch coordinates in real time.
    - Direct touch on eyeball centers triggers reflex blink response with audio chirp.

## Acoustic & Visual Hardware Feedback
- **Acoustic Synthesizer (GPIO 26)**:
  - Happy/Curious: Ascending frequency chirp (1200 Hz to 2800 Hz).
  - Surprised: Dual high-pitched burst (3200 Hz / 4200 Hz).
  - Angry: Descending growl sweep (450 Hz to 180 Hz).
  - Blink: Crisp 2600 Hz tick.
- **Active-Low RGB LED (GPIOs 4/16/17)**:
  - Synchronized with emotional mood (Cyan, Green, Magenta, Red, Yellow, Blue).

## Wi-Fi & Station Anchor Integration
- **Connection Mode**: 2.5-second non-blocking station attempt to local AP.
- **Header Badge**: Displays "@IamNotIntoTech | Paris" upon successful connection.

## Compilation & Memory Footprint
- **Platform**: Espressif 32 (v7.0.1) / ESP32 Dev Module
- **RAM Usage**: 44,712 bytes (13.6% of 327,680 bytes)
- **Flash Usage**: 790,173 bytes (60.3% of 1,310,720 bytes)
- **Build Status**: 0 errors, 0 warnings (SUCCESS)

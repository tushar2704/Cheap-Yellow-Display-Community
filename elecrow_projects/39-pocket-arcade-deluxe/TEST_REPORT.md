# Test Report: Project 39 - 5-in-1 Pocket Arcade Deluxe

## System Information
- **Target Hardware**: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
- **Target Channel**: IamNotIntoTech (`@IamNotIntoTech`)
- **Author**: Tushar Aggarwal (tushar2704)
- **Station Anchor**: Paris (Eiffel Tower - 48.8584° N, 2.2945° E)
- **Date**: October 31, 2026

## 5-in-1 Game Engines & Interactive Test
1. **Cyber Breakout**:
   - Touch paddle dragging on horizontal axis with angle deflection dynamics.
   - 4x8 color brick matrix with ball destruction and sound chimes.
2. **Neon Snake**:
   - Directional touch vector steering (Delta X / Delta Y tracking).
   - Food generation, segment growth, and boundary collision handling.
3. **Flappy CYD**:
   - Tap-to-flap impulse physics (-4.5 velocity impulse, 0.55 gravity acceleration).
   - Random gap pipe obstacles with score increment and bounding box collision.
4. **Space Defender**:
   - Touch-guided spaceship with dual laser firing rate control.
   - 2x6 animated alien fleet descending in stepped intervals with laser blast detection.
5. **Memory Matrix**:
   - Simon-style 4-quadrant musical audio-visual color sequence memory challenge.
   - Audio tone generation on GPIO 26 buzzer (880 Hz, 1175 Hz, 1397 Hz, 1760 Hz).

## Touch Controller & Coordinate Verification
- **Touch Controller**: XPT2046 over dedicated VSPI (`MOSI=32`, `MISO=39`, `CLK=25`, `CS=33`)
- **Calibration Mapping**:
  - `screenX = map(p.x, 240, 3800, 0, 320)`
  - `screenY = map(p.y, 240, 3800, 0, 240)` (Non-inverted Y axis)
- **Debounce & Sensitivity**:
  - Main menu game selection: 200 ms debounce
  - In-game real-time control: low-latency response without delay lock
- **Main Selection Menu (screenY >= 42)**:
  - Game 0 (Cyber Breakout): `(touchY - 42) / 39 == 0`
  - Game 1 (Neon Snake): `(touchY - 42) / 39 == 1`
  - Game 2 (Flappy CYD): `(touchY - 42) / 39 == 2`
  - Game 3 (Space Defender): `(touchY - 42) / 39 == 3`
  - Game 4 (Memory Matrix): `(touchY - 42) / 39 == 4`
- **Global Exit**: `screenY <= 26`, `screenX >= 240` triggers clean return to arcade menu.

## Wi-Fi & Paris NTP Synchronization
- **Connection Mode**: 3-second non-blocking station attempt to local AP.
- **NTP Server**: `pool.ntp.org` / `time.google.com` (Paris CET/CEST time zone sync).
- **Status Indicator**: Dynamic "PARIS ONLINE" green status or offline indicator in header.

## Compilation & Memory Footprint
- **Platform**: Espressif 32 (v7.0.1) / ESP32 Dev Module
- **RAM Usage**: 47,288 bytes (14.4% of 327,680 bytes)
- **Flash Usage**: 816,389 bytes (62.3% of 1,310,720 bytes)
- **Build Status**: 0 errors, 0 warnings (SUCCESS)

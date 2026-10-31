# Test Report: Project 40 - Playable No-PSRAM FreeDOOM

## System Information
- **Target Hardware**: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
- **Target Channel**: IamNotIntoTech (`@IamNotIntoTech`)
- **Author**: Tushar Aggarwal (tushar2704)
- **Station Anchor**: Paris (Eiffel Tower - 48.8584° N, 2.2945° E)
- **Date**: October 31, 2026

## 3D Raycasting Engine Architecture
- **Rendering Model**: DDA (Digital Differential Analysis) raycaster casting 80 rays across 320 px screen (4 px column slice width) within internal 520 KB SRAM without external PSRAM.
- **Viewport**: 320x188 px high-speed raycasted perspective viewport with depth-fog attenuation and directional wall shading.
- **Map Representation**: 16x16 dungeon array with varying wall textures and solid collision boundaries.
- **Sprite Synthesis**: Procedural double-barrel shotgun sprite with dynamic recoil displacement and muzzle flash lighting.

## Touch Controller & Coordinate Verification
- **Touch Controller**: XPT2046 over dedicated VSPI (`MOSI=32`, `MISO=39`, `CLK=25`, `CS=33`)
- **Calibration Mapping**:
  - `screenX = map(p.x, 240, 3800, 0, 320)`
  - `screenY = map(p.y, 240, 3800, 0, 240)` (Non-inverted Y axis)
- **Interactive Control Schemes**:
  1. **Direct Viewport Drag (`screenY < 190`)**:
     - `screenX < 100`: Smooth left turn rotation
     - `screenX > 220`: Smooth right turn rotation
     - `100 <= screenX <= 220`: Forward movement with wall collision bounds
  2. **Virtual HUD D-Pad (`screenY >= 190`)**:
     - Left turn (`<`): `screenX in [195, 222]`
     - Forward (`^`): `screenX in [223, 252], screenY < 216`
     - Backward (`v`): `screenX in [223, 252], screenY >= 216`
     - Right turn (`>`): `screenX in [253, 280]`
     - Fire Shotgun (`FIRE`): `screenX >= 280`

## Acoustic & Visual Hardware Feedback
- **Acoustic Synthesizer**: Rapid descending sweep from 1800 Hz down to 300 Hz on GPIO 26 buzzer for shotgun muzzle report.
- **Visual Feedback**: Yellow/Red muzzle flash rendered on screen and active-low RGB LED flash on GPIOs 4/16/17.

## Wi-Fi & Station Anchor Integration
- **Connection Mode**: Safe non-blocking connection check to local station AP.
- **Watermark & Telemetry**: Dynamic "PARIS NET | IamNotIntoTech" header indicator.

## Compilation & Memory Footprint
- **Platform**: Espressif 32 (v7.0.1) / ESP32 Dev Module
- **RAM Usage**: 44,572 bytes (13.6% of 327,680 bytes)
- **Flash Usage**: 794,329 bytes (60.6% of 1,310,720 bytes)
- **Build Status**: 0 errors, 0 warnings (SUCCESS)

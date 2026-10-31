# Project 40: Playable No-PSRAM FreeDOOM

A 3D first-person raycasting engine running inside internal ESP32 SRAM engineered for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
Experience 90s-style pseudo-3D first-person action on embedded silicon without requiring external PSRAM chips. Running purely within the ESP32's 520KB internal memory, this custom DDA raycaster delivers textured depth-shaded walls, distance fogging, animated shotgun weapon sprites with muzzle blast recoil, an interactive touch D-pad, and authentic audio feedback.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM, Zero PSRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Muzzle Flash LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue) synchronized with gun discharges
- **Buzzer**: Piezo buzzer on GPIO 26 for shotgun blast frequency sweep audio

## Features
- **DDA Raycasting Graphics Pipeline**:
  - Casts 80 independent rays with 4-pixel column scaling at 188px vertical field of view.
  - Multi-material wall rendering (Stone, Brick/Wood, Slime) with shadow-side darkening and distance fog attenuation.
- **Shotgun Weapon Sprite & Muzzle Recoil**:
  - Center-aligned double-barrel shotgun sprite with dynamic recoil offset and yellow/red muzzle flash spark during firing.
- **Dual Touch Movement Interfaces**:
  - Direct 3D Viewport Touch: Tap left to rotate counter-clockwise, right to rotate clockwise, and center to move forward.
  - Classic On-Screen Virtual D-Pad: Dedicated directional touch targets (`<`, `^`, `v`, `>`).
  - Giant `FIRE` Trigger Button: Fires weapon with synthesized audio sweep and RGB flash.
- **Classic Retro Status HUD**:
  - Health percentage readout (Red).
  - Armor percentage readout (Blue).
  - Shotgun shell ammunition counter (Gold).

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups

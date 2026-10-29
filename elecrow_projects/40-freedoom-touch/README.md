# Project 40: Playable No-PSRAM FreeDOOM

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

**Playable No-PSRAM FreeDOOM** is a bare-metal 3D first-person shooter engine built for the standard ESP32-WROOM without external PSRAM. Running entirely within the chip's internal 520KB SRAM, it utilizes an optimized Digital Differential Analyzer (DDA) raycasting pipeline to deliver 35+ FPS first-person perspective dungeon exploration directly on the Elecrow 2.8" CYD.

The engine features real-time depth shading, distance fog, animated weapon recoil with muzzle flashes, on-screen touch D-Pad controls, an instant "HIT" trigger button, and classic retro health/armor/ammo HUD metrics.

---

## Features

- **No-PSRAM 3D Raycasting Engine**:
  - Full first-person 3D perspective rendering at 80 rays with 4-pixel column interpolation across a 320x188 viewport.
  - Textured and depth-attenuated walls with dynamic distance atmospheric fog.
- **Weapon System & Recoil Animation**:
  - Center-mounted double-barrel shotgun with animated recoil kickback and spark muzzle flash.
- **Classic Retro Status HUD**:
  - Live **HEALTH %**, **ARMOR %**, and **AMMO** counter displays styled after legendary 90s FPS titles.
- **Intuitive Touch Controls**:
  - Direct touch on-screen D-Pad: `[<]` Turn Left, `[^]` Forward, `[v]` Backward, `[>]` Turn Right.
  - Dedicated tactile `[HIT]` firing trigger button.
  - Direct viewport touch drag to pan or advance through corridors.
- **Acoustic Audio & Muzzle Flash LED**:
  - Generates authentic 8-bit weapon blast audio sweeps on `GPIO 26`.
  - Tri-color RGB LED (`IO4`, `IO16`) flashes brilliant amber on weapon discharge and red on incoming damage.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **Muzzle Flash RGB LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW weapon flash indicator |
| **Weapon Audio Buzzer** | `GPIO 26` | Piezo sound blast synthesizer |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/40-freedoom-touch
   pio run --target upload
   ```
3. Use the on-screen touch controls to navigate the dungeon, clear corridors, and fire your weapon.

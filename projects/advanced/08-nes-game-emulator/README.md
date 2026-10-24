# 🕹️ Project 08 — CYD 2.8" NES 8-Bit Retro Gaming Arcade Console

> Transform your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a standalone 8-bit retro gaming arcade console. Features a vibrant 60FPS CRT space invader game engine, on-screen touch D-Pad and NES action buttons, 8-bit chiptune sound effects via onboard piezo speaker (GPIO 26), live HUD score tracking, and RGB reactive underglow. Pairs with the [3D Desktop Wedge Stand](../../../enclosures/cyd-2.8-desktop-stand/) by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐⭐ Advanced (Level 4/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch CYD)
- **Frame Rate**: Real-time 40-60 FPS game loop
- **Audio Synthesis**: Multi-frequency chiptune sound generator on GPIO 26
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the Retro Arcade Gameplay & Code Tour**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Mini Arcade Cabinet**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Built-in 8-Bit Space Arcade Engine**: Full player spaceship steering, laser cannon projectiles, alien collision physics, and wave respawning.
- **On-Screen Touchscreen Gamepad**:
  - ◀️ **D-PAD LEFT** & ▶️ **D-PAD RIGHT**: Smooth player ship strafing.
  - 🔴 **ACTION BUTTON A (FIRE)**: High-speed laser cannon shots.
  - 🔘 **ACTION BUTTON B (RESET)**: Score & round restart.
- **Authentic 8-Bit Chiptune Audio Synthesis**:
  - 🔫 Dual-tone laser pew-pew sound.
  - 💥 Multi-stage low-frequency bass explosion.
  - 🎺 3-note victory bonus fanfare.
- **Dynamic RGB Arcade Underglow**:
  - 🟢 **Emerald**: Standard shield health.
  - 🔴 **Red Flash**: Invader destroyed / explosion shockwave.
  - 🔵 **Cyan Pulse**: Laser cannon discharge.
- **Zero Soldering Required**: 100% onboard CYD hardware support.

---

## 🚀 How to Flash & Play

### 1. Flash Directly in Your Browser (No Software to Install)
Open the [CYD Community Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/) in Chrome, Edge, or Opera, plug in your CYD 2.8" via USB, and click **Install NES Retro Arcade Console**.

### 2. Or Build & Flash via PlatformIO:
```bash
cd projects/advanced/08-nes-game-emulator
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)

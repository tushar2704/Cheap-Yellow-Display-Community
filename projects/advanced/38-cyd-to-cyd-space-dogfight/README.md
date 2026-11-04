# 🚀 Project 38: CYD-to-CYD Space Dogfight Duel

> A high-energy, retro arcade two-player space dogfight running across two Cheap Yellow Displays (ESP32-2432S028R). Ships face each other across physical space, passing plasma laser bolts, EMP shockwaves, and energy shields over sub-5ms ESP-NOW radio packets!

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Build%20Passing-brightgreen.svg)](platformio.ini)
[![Hardware](https://img.shields.io/badge/Hardware-CYD%202.8%22%20Resistive-orange.svg)](#hardware-requirements)
[![Protocol](https://img.shields.io/badge/Protocol-ESP--NOW%202.4GHz-blue.svg)](#how-it-works)
[![License: GNU AGPLv3](https://img.shields.io/badge/License-AGPLv3-blue.svg)](../../../LICENSE)

---

## 📺 Community Channels

| Channel Handle | Channel Name | Focus |
|---------------|-------------|-------|
| `@IamNotIntoTech` | [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) | Tech projects, ESP32 tutorials & CYD builds |
| `@Makerrdesigns` | [Makerrdesigns](https://youtube.com/@Makerrdesigns) | 3D design, enclosures & hardware mods |

---

## 🌟 Highlights & Features

- **Cross-Screen Projectile Physics**: Fire a plasma bolt on your screen, and watch it exit the top of your display and immediately materialize on your opponent's display traveling down toward their ship!
- **Zero-Latency ESP-NOW Radio**: Direct peer-to-peer 2.4GHz packets at 60Hz. No WiFi router, zero internet lag, and instant reaction time.
- **Player Roles & Ship Identity**:
  - **Player 1**: Blue Falcon fighter (Cyan plasma lasers & blue hull).
  - **Player 2**: Red Phoenix fighter (Red plasma lasers & red hull).
  - Tap the top-left player badge to switch roles; saved to NVS flash memory.
- **Tactical Abilities**:
  - `[FIRE 🚀]`: Rapid-fire dual plasma cannons with 8-bit audio zaps.
  - `[SHIELD 🛡️]`: Deploys a spherical energy forcefield that absorbs 1 hit (3s active duration, 9s cooldown).
  - `[EMP ⚡]`: Launches an electric EMP orb that temporarily disables opponent thrusters and weapons for 2.5s!
- **Haptic & Acoustic Feedback**:
  - Synthesized retro laser zaps, shield deflection hums, and low rumble explosions on GPIO 26.
  - RGB LED pulses with your ship's color and flashes red when taking damage.
  - Screen shake and damage flashes when hull integrity is breached.
- **Calibrated Touch Digitizer**: Dedicated VSPI bus (`XPT2046` on `CLK=25, MISO=39, MOSI=32, CS=33`) with responsive touch arrows and drag lane steering.

---

## 🛠️ Hardware Requirements

| Component | Specification |
|-----------|---------------|
| **Device** | 2× Cheap Yellow Display (CYD 2.8" ESP32-2432S028R) |
| **Display** | ILI9341 320×240 TFT LCD |
| **Touch Controller** | XPT2046 Resistive Touchscreen (dedicated VSPI) |
| **Audio** | Built-in Piezo buzzer on GPIO 26 |
| **RGB LED** | On-board Tricolor LED (GPIO 4: Red, GPIO 16: Green, GPIO 17: Blue) |
| **Power** | 5V via Micro-USB or Type-C |

---

## 📌 Pinout Reference

| Peripheral | ESP32 GPIO Pin | Description |
|------------|----------------|-------------|
| **TFT MOSI** | GPIO 13 | HSPI Data to Screen |
| **TFT MISO** | GPIO 12 | HSPI Data from Screen |
| **TFT SCLK** | GPIO 14 | HSPI Screen Clock |
| **TFT CS** | GPIO 15 | Screen Chip Select |
| **TFT DC** | GPIO 2 | Screen Data / Command |
| **TFT BL** | GPIO 21 | Screen Backlight (Active HIGH) |
| **Touch CLK** | GPIO 25 | VSPI Touch Clock |
| **Touch MISO**| GPIO 39 | VSPI Touch MISO |
| **Touch MOSI**| GPIO 32 | VSPI Touch MOSI |
| **Touch CS**  | GPIO 33 | VSPI Touch Chip Select |
| **Touch IRQ** | GPIO 36 | Touch Interrupt Detect |
| **Speaker**   | GPIO 26 | Tone / Audio Output |
| **LED Red**   | GPIO 4  | Damage Alert LED (Active LOW) |
| **LED Green** | GPIO 16 | Shield Status LED (Active LOW) |
| **LED Blue**  | GPIO 17 | P1 Falcon LED (Active LOW) |

---

## 🚀 Getting Started

### Option 1: PlatformIO (Recommended)

1. Open this directory in VS Code with PlatformIO installed:
   ```bash
   cd projects/advanced/38-cyd-to-cyd-space-dogfight
   ```
2. Connect your first CYD via USB cable.
3. Build and upload firmware:
   ```bash
   pio run --target upload
   ```
4. Connect your second CYD and flash the same firmware.

### Option 2: Arduino IDE

1. Install the ESP32 board package (`esp32` by Espressif, version 2.0.x or 3.x).
2. Install the required libraries via the Library Manager:
   - `TFT_eSPI` (v2.5.43+)
   - `XPT2046_Touchscreen`
3. Configure `User_Setup.h` in `TFT_eSPI` for the ESP32-2432S028R display (`ILI9341_2_DRIVER`).
4. Open [38-cyd-to-cyd-space-dogfight.ino](38-cyd-to-cyd-space-dogfight.ino).
5. Select board **ESP32 Dev Module**, choose the USB COM port, and click **Upload**.

---

## 🎮 How to Play

1. **Place CYDs Head-to-Head**: Position two CYD displays facing each other on a table or desk.
2. **Assign Roles**:
   - On the first CYD, verify the header displays **`[P1: BLUE FALCON]`**.
   - On the second CYD, tap the top-left badge to switch to **`[P2: RED PHOENIX]`**.
3. **Steering**:
   - Tap `[ < ]` and `[ > ]` buttons or drag your finger along the lower ship lane to dodge incoming fire.
4. **Attacking**:
   - Tap `[FIRE]` to launch plasma bolts upwards. When they pass off the top of your display, they shoot out of the top of your opponent's display!
5. **Defense & Special Tactics**:
   - Tap `[SHIELD]` to deflect an incoming lethal strike.
   - Tap `[EMP]` to launch an electromagnetic pulse bolt that stuns your rival.
6. **Victory & Rematch**:
   - Each hit reduces hull HP by 20%.
   - When a ship reaches 0% HP, the victor's screen displays **`VICTORY!!!`** and the loser sees **`DEFEATED!`**.
   - Tap the screen on either device to trigger an instant synchronized rematch!

---

## 📜 License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPLv3)**.

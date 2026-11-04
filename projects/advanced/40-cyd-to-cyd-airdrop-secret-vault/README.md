# 🔐 Project 40: CYD-to-CYD AirDrop & Secret Vault

> An encrypted offline credentials vault and wireless peer-to-peer data beam for the Cheap Yellow Display (ESP32-2432S028R). Store sensitive credentials, passwords, and cryptographic seeds behind a PIN lock, and beam secrets wirelessly between two CYD units with animated plasma particle effects over sub-5ms ESP-NOW!

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

- **Encrypted Local Storage (NVS)**: Securely stores credentials, private keys, WiFi passphrases, and seed phrases in non-volatile flash memory that persists across power cycles.
- **PIN-Protected Lock Screen**: Master 4-digit PIN authentication (`1234` default) with interactive on-screen capacitive/resistive numpad.
- **Wireless P2P AirDrop Data Beam**: Select any stored secret and tap `[AIRDROP BEAM]` to beam the encrypted payload directly to a nearby CYD over 2.4GHz ESP-NOW (zero router, zero cloud).
- **Animated Plasma Particle Beam**: Features an animated laser particle beam stream and transfer progress bar during wireless transmission.
- **Receiving Confirmation Modal**: The recipient CYD prompts with an interactive dialog: `INCOMING AIRDROP from CYD-XXXX: "Home WiFi Key"` with `[ACCEPT]` or `[REJECT]` buttons.
- **Mask / Unmask Inspector**: Inspect secrets with privacy masking (`••••••••`) and tap `[SHOW]` / `[HIDE]` for reveal.
- **Acoustic & RGB Alerts**:
  - Vault unlock fanfare and lock confirmation clicks on GPIO 26.
  - Laser charge frequency sweep and transfer whoosh.
  - RGB LED signals status: Red (Locked), Green (Unlocked), Blue (AirDrop Beaming).
- **Calibrated Touch Digitizer**: Dedicated VSPI bus (`XPT2046` on `CLK=25, MISO=39, MOSI=32, CS=33`).

---

## 🛠️ Hardware Requirements

| Component | Specification |
|-----------|---------------|
| **Device** | 2× Cheap Yellow Display (CYD 2.8" ESP32-2432S028R) |
| **Display** | ILI9341 320×240 TFT LCD |
| **Touch Controller** | XPT2046 Resistive Touchscreen (dedicated VSPI) |
| **Audio** | Built-in Piezo buzzer on GPIO 26 |
| **RGB LED** | On-board Tricolor LED (GPIO 4: Red, GPIO 16: Green, GPIO 17: Blue) |
| **Radio** | ESP-NOW 2.4GHz Peer-to-Peer |
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
| **LED Red**   | GPIO 4  | Vault Locked LED (Active LOW) |
| **LED Green** | GPIO 16 | Vault Unlocked LED (Active LOW) |
| **LED Blue**  | GPIO 17 | AirDrop Beaming LED (Active LOW) |

---

## 🚀 Getting Started

### Option 1: PlatformIO (Recommended)

1. Open this directory in VS Code with PlatformIO installed:
   ```bash
   cd projects/advanced/40-cyd-to-cyd-airdrop-secret-vault
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
4. Open [40-cyd-to-cyd-airdrop-secret-vault.ino](40-cyd-to-cyd-airdrop-secret-vault.ino).
5. Select board **ESP32 Dev Module**, choose the USB COM port, and click **Upload**.

---

## 🎮 How to Use

1. **Unlock the Vault**:
   - On boot, the red LED illuminates and the PIN lock screen appears.
   - Enter default PIN **`1234`** on the on-screen keypad to unlock.
2. **Browse Records**:
   - The green LED turns on and displays your stored secrets list (e.g. WiFi keys, SSH keys, recovery seeds).
   - Tap any record to enter the detail inspector.
3. **Inspect Secret**:
   - Tap `[SHOW]` to unmask the secret payload.
4. **AirDrop Beam to Another CYD**:
   - Power on a second CYD running this firmware within 100 meters.
   - On the sending CYD, tap **`[AIRDROP BEAM]`**.
   - Watch the animated plasma particle stream shoot across the display with laser charging sounds.
5. **Accepting on Recipient CYD**:
   - The receiving CYD displays an instant modal dialog: `INCOMING AIRDROP! From: CYD-XXXX`.
   - Tap **`[ACCEPT]`** to save the credential into the recipient's local flash vault.
6. **Locking**:
   - Tap `[LOCK]` anytime to return to the secure locked state.

---

## 📜 License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPLv3)**.

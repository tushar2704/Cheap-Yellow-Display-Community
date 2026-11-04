# 📱 Project 36: CYD-to-CYD BlackBerry Messenger (CYD-BBM)

> An authentic, retro-styled off-grid peer-to-peer communicator for two or more Cheap Yellow Displays (ESP32-2432S028R), bringing back the iconic BlackBerry Messenger (BBM) experience with auto-discovery PINs, chiclet QWERTY keyboard, delivery receipts, and the legendary **`PING!!!`** buzz!

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

- **Iconic BlackBerry BBM PIN**: Deterministically generates an 8-character hex PIN (e.g., `PIN: 2B84D91A`) derived from the ESP32 hardware MAC address.
- **Zero-Config Off-Grid Radio**: Powered by direct 2.4GHz ESP-NOW packets — no WiFi routers, no internet, and no cloud servers required (up to 200m line of sight).
- **Auto-Discovery Presence Beacons**: Displays broadcast periodic heartbeat beacons every 4 seconds. Nearby buddies automatically appear in the active header count.
- **The Famous "PING!!!" (Nudge / Buzz)**: Pressing `[PING!]` triggers a triple high-contrast screen strobe, rapid red RGB LED flashes, and a high-energy alert buzz on the recipient's display.
- **BBM Delivery `[D]` & Read `[R]` Receipts**:
  - `[D]` appears when the peer ESP-NOW radio successfully acknowledges packet reception.
  - `[R]` appears when the peer opens/reads the incoming message stream.
- **Blinking Red Notification LED**: The signature BlackBerry flashing red LED on GPIO 4 pulses gently whenever an unread message or PING awaits.
- **On-Screen Chiclet QWERTY Keyboard**: Beveled retro chiclet key layout with tactile click tones on GPIO 26, quick emoji/symbol cycling (`[SYM]`), clear (`[CLR]`), backspace (`[DEL]`), and `[SEND]`.
- **Tested & Calibrated Touch Mapping**: Tuned specifically for the 2.8" resistive touch digitizer (`XPT2046` on SPI `CLK=25, MISO=39, MOSI=32, CS=33`).

---

## 🛠️ Hardware Requirements

| Component | Specification |
|-----------|---------------|
| **Device** | 2× (or more) Cheap Yellow Display (CYD 2.8" ESP32-2432S028R) |
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
| **Touch MISO**| GPIO 39 | VSPI Touch MISO (Hardware IRQ/MISO) |
| **Touch MOSI**| GPIO 32 | VSPI Touch MOSI |
| **Touch CS**  | GPIO 33 | VSPI Touch Chip Select |
| **Touch IRQ** | GPIO 36 | Touch Interrupt Detect |
| **Speaker**   | GPIO 26 | Tone / Audio Output |
| **LED Red**   | GPIO 4  | Notification LED (Active LOW) |
| **LED Green** | GPIO 16 | Status LED (Active LOW) |
| **LED Blue**  | GPIO 17 | Radio Activity (Active LOW) |

---

## 🚀 Getting Started

### Option 1: PlatformIO (Recommended)

1. Open this directory in VS Code with PlatformIO installed:
   ```bash
   cd projects/advanced/36-cyd-to-cyd-blackberry-messenger
   ```
2. Connect your CYD via USB cable.
3. Build and upload firmware:
   ```bash
   pio run --target upload
   ```
4. Flash the second CYD using the same command.

### Option 2: Arduino IDE

1. Install the ESP32 board package (`esp32` by Espressif, version 2.0.x or 3.x).
2. Install the required libraries via the Library Manager:
   - `TFT_eSPI` (v2.5.43+)
   - `XPT2046_Touchscreen`
3. Configure `User_Setup.h` in `TFT_eSPI` for the ESP32-2432S028R display (`ILI9341_2_DRIVER`).
4. Open [36-cyd-to-cyd-blackberry-messenger.ino](36-cyd-to-cyd-blackberry-messenger.ino).
5. Select board **ESP32 Dev Module**, choose the USB COM port, and click **Upload**.

---

## 🎮 How to Use

1. **Power On**: Power two CYD boards within 100 meters of each other.
2. **Instant Pairing**: Look at the top bar — both devices will show their unique 8-character BBM PIN and the buddy count will switch to `Buddies: 1` as presence beacons exchange.
3. **Typing**: Tap letters on the on-screen chiclet keyboard. Each touch emits a crisp tactile feedback click.
4. **Sending**: Tap `[SEND]` to transmit over ESP-NOW. Notice the `[D]` delivery confirmation tick appear immediately on the message bubble.
5. **The PING Buzz**: Tap `[PING!]` to send an instant high-priority alert. The other CYD's screen will strobe in red/white with a triple buzzer siren!
6. **Blinking Red Notification LED**: If a message arrives while you are not looking, the classic BlackBerry red LED pulses every 2 seconds until you tap the screen.

---

## 📜 License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPLv3)**.

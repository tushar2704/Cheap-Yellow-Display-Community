# 📡 Project 39: CYD Stealth WiFi Radar Hunter

> A standalone 2.4GHz RF privacy scanner and stealth beacon detector for the Cheap Yellow Display (ESP32-2432S028R). Features an animated circular CRT phosphor radar sweep, hidden/cloaked SSID detection, proximity distance estimation, 13-channel spectrum histogram, and acoustic Geiger counter audio feedback!

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Build%20Passing-brightgreen.svg)](platformio.ini)
[![Hardware](https://img.shields.io/badge/Hardware-CYD%202.8%22%20Resistive-orange.svg)](#hardware-requirements)
[![RF Scanner](https://img.shields.io/badge/RF-2.4GHz%20Promiscuous%20Scan-blue.svg)](#how-it-works)
[![License: GNU AGPLv3](https://img.shields.io/badge/License-AGPLv3-blue.svg)](../../../LICENSE)

---

## 📺 Community Channels

| Channel Handle | Channel Name | Focus |
|---------------|-------------|-------|
| `@IamNotIntoTech` | [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) | Tech projects, ESP32 tutorials & CYD builds |
| `@Makerrdesigns` | [Makerrdesigns](https://youtube.com/@Makerrdesigns) | 3D design, enclosures & hardware mods |

---

## 🌟 Highlights & Features

- **Animated Phosphor CRT Radar Scope**: 360-degree rotating radar beam with concentric distance rings (-50dBm, -70dBm, -90dBm) and phosphor blip fading.
- **Hidden / Cloaked SSID Discovery**: Detects access points broadcasting null/zero-length SSIDs or hiding beacon identifiers, highlighting them with pulsing red hazard blips `[STEALTH AP]`.
- **Proximity Distance Estimation**: Translates raw RSSI dBm signal values to estimated physical distance (in meters) using the Log-Distance Path Loss model.
- **Three Touch-Switchable Views**:
  1. **Radar Scope**: Real-time rotating radar display with side target intel panel.
  2. **Stealth Log**: Detailed tabular inventory with BSSID MAC addresses, channels, RSSI, and distance metrics.
  3. **2.4GHz Spectrum Histogram**: Visual congestion graph across all 13 channels with hidden transmitters highlighted in red atop channel bars.
- **Geiger Proximity Audio Feedback**: Emits simulated Geiger counter clicks that increase in frequency as you move closer to hidden wireless transmitters.
- **Hardware-Tested Touch Mapping**: Calibrated non-inverted resistive touch coordinates (`XPT2046` on dedicated VSPI `CLK=25, MISO=39, MOSI=32, CS=33`).
- **Tricolor Visual Alerting**: Flashes red on GPIO 4 when covert or hidden networks are detected in range, and illuminates green when the airspace is clean.

---

## 🛠️ Hardware Requirements

| Component | Specification |
|-----------|---------------|
| **Device** | Cheap Yellow Display (CYD 2.8" ESP32-2432S028R) |
| **Display** | ILI9341 320×240 TFT LCD |
| **Touch Controller** | XPT2046 Resistive Touchscreen (dedicated VSPI) |
| **Audio** | Built-in Piezo buzzer on GPIO 26 |
| **RGB LED** | On-board Tricolor LED (GPIO 4: Red Alert, GPIO 16: Green Clean, GPIO 17: Blue Scan) |
| **RF Radio** | 2.4GHz 802.11b/g/n receiver |
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
| **LED Red**   | GPIO 4  | Stealth AP Alert LED (Active LOW) |
| **LED Green** | GPIO 16 | Clean Airspace LED (Active LOW) |
| **LED Blue**  | GPIO 17 | Active Scan LED (Active LOW) |

---

## 🚀 Getting Started

### Option 1: PlatformIO (Recommended)

1. Open this directory in VS Code with PlatformIO installed:
   ```bash
   cd projects/advanced/39-cyd-stealth-wifi-radar-hunter
   ```
2. Connect your CYD via USB cable.
3. Build and upload firmware:
   ```bash
   pio run --target upload
   ```

### Option 2: Arduino IDE

1. Install the ESP32 board package (`esp32` by Espressif, version 2.0.x or 3.x).
2. Install the required libraries via the Library Manager:
   - `TFT_eSPI` (v2.5.43+)
   - `XPT2046_Touchscreen`
3. Configure `User_Setup.h` in `TFT_eSPI` for the ESP32-2432S028R display (`ILI9341_2_DRIVER`).
4. Open [39-cyd-stealth-wifi-radar-hunter.ino](39-cyd-stealth-wifi-radar-hunter.ino).
5. Select board **ESP32 Dev Module**, choose the USB COM port, and click **Upload**.

---

## 🎮 How to Use

1. **Power On**: Flash and boot the device. The unit emits an initial sonar chirp and performs an immediate background RF scan.
2. **Radar Scope**:
   - The green beam sweeps clockwise continuously across the circular range rings.
   - Regular public networks appear as cyan dots.
   - **Hidden networks** appear as **pulsing red diamond targets** with hazard rings.
   - When the radar sweep crosses a hidden blip, an acoustic sonar ping sounds!
3. **Intel Panel (Right Side)**:
   - Shows total detected networks, stealth count, and the BSSID, RSSI, channel, and estimated distance of the strongest hidden transmitter.
   - Tap `[RE-SCAN]` to force an immediate refresh.
   - Tap `[GEIGER: ON]` to activate audio proximity clicking. As you walk closer to the hidden transmitter, clicking accelerates!
4. **Switching Views**:
   - Tap `[LOG]` in the top header to inspect the raw inventory table.
   - Tap `[SPECTRUM]` to view the 13-channel RF histogram.
   - Tap `[BEEP]` / `[MUTE]` to toggle audio effects.

---

## 📜 License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPLv3)**.


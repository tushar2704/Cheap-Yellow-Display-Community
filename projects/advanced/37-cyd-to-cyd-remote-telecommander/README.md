# 🕹️ Project 37: CYD-to-CYD Remote Tele-Commander (KVM & Actuator Deck)

> A dual-display handheld wireless controller and appliance system for two Cheap Yellow Displays (ESP32-2432S028R). Use one CYD as a precision touch trackpad, remote backlight dimmer, RGB palette picker, and soundboard, while the other CYD acts as a target HUD appliance and robotic actuator controller with live telemetry feedback over sub-5ms ESP-NOW!

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

- **Single Unified Firmware**: The exact same binary runs on both CYDs. Easily switch roles between `[ROLE: MASTER]` and `[ROLE: SLAVE]` by tapping the top-right role button (persisted in NVS flash memory).
- **Sub-5ms Ultra-Low Latency ESP-NOW**: Zero-configuration, off-grid 2.4GHz wireless link streaming at 40Hz with no router or internet required.
- **Precision Virtual Touchpad**: Dragging your finger across Master's trackpad moves a live emerald green crosshair cursor `+` across Slave's canvas in real time.
- **Remote Backlight Dimmer**: Touch-slider on Master controls Slave's display backlight PWM brightness (10% to 100%) on GPIO 21.
- **Remote Audio Soundboard**: Tap 4 sound presets (`SIREN`, `LASER`, `HORN`, `CHIRP`) on Master to synthesize instant tones through Slave's speaker (GPIO 26).
- **Remote RGB LED Palette**: Select `RED`, `GREEN`, `BLUE`, or `OFF` to actuate Slave's on-board tri-color LED (GPIO 4, 16, 17).
- **Remote Actuator Relay Switch**: Master toggle activates/deactivates Slave's GPIO 22 output pin for physical relays, solenoid valves, or robotics.
- **Bi-Directional Sensor Telemetry**: Slave continuously samples its ambient light (LDR sensor GPIO 34), ESP32 CPU die temperature, and packet counters, streaming them back to Master's live telemetry HUD.
- **Hardware-Tested Touch Mapping**: Non-inverted 2.8" resistive digitizer calibration (`XPT2046` on VSPI `CLK=25, MISO=39, MOSI=32, CS=33`).

---

## 🛠️ Hardware Requirements

| Component | Specification |
|-----------|---------------|
| **Device** | 2× Cheap Yellow Display (CYD 2.8" ESP32-2432S028R) |
| **Display** | ILI9341 320×240 TFT LCD |
| **Touch Controller** | XPT2046 Resistive Touchscreen (dedicated VSPI) |
| **Audio** | Built-in Piezo buzzer on GPIO 26 |
| **Backlight PWM** | GPIO 21 (PWM Channel 0, 5kHz) |
| **RGB LED** | On-board Tricolor LED (GPIO 4: Red, GPIO 16: Green, GPIO 17: Blue) |
| **Light Sensor** | LDR Photoresistor on GPIO 34 |
| **Aux Relay Pin** | GPIO 22 (Digital Output for Relay / Transistor) |
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
| **TFT BL** | GPIO 21 | Screen Backlight (PWM dimming) |
| **Touch CLK** | GPIO 25 | VSPI Touch Clock |
| **Touch MISO**| GPIO 39 | VSPI Touch MISO |
| **Touch MOSI**| GPIO 32 | VSPI Touch MOSI |
| **Touch CS**  | GPIO 33 | VSPI Touch Chip Select |
| **Touch IRQ** | GPIO 36 | Touch Interrupt Detect |
| **Speaker**   | GPIO 26 | Tone / Audio Output |
| **LED Red**   | GPIO 4  | Status LED (Active LOW) |
| **LED Green** | GPIO 16 | Status LED (Active LOW) |
| **LED Blue**  | GPIO 17 | Radio Activity (Active LOW) |
| **LDR Sensor**| GPIO 34 | Ambient Light Sensor (Analog ADC) |
| **Aux Relay** | GPIO 22 | External Relay Actuator Trigger |

---

## 🚀 Getting Started

### Option 1: PlatformIO (Recommended)

1. Open this directory in VS Code with PlatformIO installed:
   ```bash
   cd projects/advanced/37-cyd-to-cyd-remote-telecommander
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
4. Open [37-cyd-to-cyd-remote-telecommander.ino](37-cyd-to-cyd-remote-telecommander.ino).
5. Select board **ESP32 Dev Module**, choose the USB COM port, and click **Upload**.

---

## 🎮 How to Use

1. **Power On**: Power up both CYD units.
2. **Assign Roles**:
   - On the CYD you wish to hold as the remote controller, tap the top-right button until it displays **`ROLE: MTR`**.
   - On the other CYD (target display/actuator), tap the top-right button until it displays **`ROLE: SLV`**.
   - Both devices save this setting in non-volatile flash memory.
3. **Wireless Cursor Control**:
   - Slide your finger on Master's left Trackpad box.
   - Watch the emerald green crosshair cursor `+` track your finger on Slave in real time.
4. **Remote Backlight**:
   - Tap or slide along the Master's brightness bar. Slave's screen backlight dims or brightens instantly.
5. **Remote Soundboard**:
   - Tap `SIREN`, `LASER`, `HORN`, or `CHIRP` on Master to trigger loud synthesized audio alerts on Slave.
6. **Remote RGB & Relay**:
   - Tap the color tiles (`RED`, `GREEN`, `BLUE`, `OFF`) to toggle Slave's physical RGB LED.
   - Tap `ACTUATOR RELAY` to toggle Slave's GPIO 22 on and off.
7. **Telemetry Return**:
   - Cover Slave's light sensor (LDR on GPIO 34) with your hand and watch the live ADC value update on Master's footer!

---

## 📜 License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPLv3)**.

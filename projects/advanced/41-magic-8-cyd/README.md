# 🎱 Project 41: Magic 8 CYD

> An interactive mystical Magic 8-Ball oracle, fluid shake synthesizer, and digital axis accelerometer station for the Cheap Yellow Display (ESP32-2432S028R). Features 32 deep prophecies across classic and tech categories, 3D glossy ball rendering, blue liquid vortex animations, floating icosahedron die reveals, and auto-detecting support for external digital axis sensors and acoustic feedback!

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Build%20Passing-brightgreen.svg)](platformio.ini)
[![Hardware](https://img.shields.io/badge/Hardware-CYD%202.8%22%20Resistive-orange.svg)](#hardware-requirements)
[![Sensors](https://img.shields.io/badge/Sensors-Touch%20%2B%20I2C%20Axis-blue.svg)](#how-it-works)
[![License: GNU AGPLv3](https://img.shields.io/badge/License-AGPLv3-blue.svg)](../../../LICENSE)

---

## 📺 Community Channels

| Channel Handle | Channel Name | Focus |
|---------------|-------------|-------|
| `@IamNotIntoTech` | [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) | Tech projects, ESP32 tutorials & CYD builds |
| `@Makerrdesigns` | [Makerrdesigns](https://youtube.com/@Makerrdesigns) | 3D design, enclosures & hardware mods |

---

## 🌟 Highlights & Features

- **32 Deep Prophecies (Far Beyond the Standard 8)**:
  - **Affirmative (10)**: *"IT IS CERTAIN"*, *"WITHOUT A DOUBT"*, *"SIGNS POINT TO YES"*, *"YOU MAY RELY ON IT"*...
  - **Non-Committal / Hesitant (6)**: *"REPLY HAZY, TRY AGAIN"*, *"MISTS HIDE THE TRUTH"*, *"CONCENTRATE & ASK"*...
  - **Negative (6)**: *"DON'T COUNT ON IT"*, *"MY REPLY IS NO"*, *"OUTLOOK NOT SO GOOD"*, *"VERY DOUBTFUL"*...
  - **Tech & Maker Wisdom (10)**: *"COMPILE SUCCESSFUL"*, *"SEGFAULT: TRY AGAIN"*, *"404: ANSWER NOT FOUND"*, *"COMMIT & PUSH TO GIT"*, *"MAGIC SMOKE DETECTED: NO"*, *"IT WORKS ON MY CYD!"*...
- **Dual Triggering Engine (Touch + Physical Shake)**:
  - **Touch & Drag**: Tap or flick the on-screen Magic 8 Ball to shake it virtually.
  - **Digital Axis Accelerometer Ready**: Auto-probes I2C on the CYD CN1 expansion port (`SDA=GPIO 27, SCL=GPIO 22`). If an MPU-6050 or ADXL345 is attached, shaking the physical CYD triggers the prophecy! If absent, it runs seamlessly via touch with zero freeze.
- **Dynamic 3D Shading & Fluid Vortex Animation**:
  - Glossy 3D ray-shaded black billiard sphere with realistic specular light highlights.
  - White circular numeral "8" faceplate.
  - When shaken, the white faceplate dissolves into a swirling dark blue fluid vortex with floating bubbles and RGB LED color mixing.
  - Floating blue icosahedron die triangle surfaces from the liquid with glowing prophecy text.
- **Acoustic & Sentiment Lighting Feedback**:
  - Synthesized fluid swirling and rattle sounds on GPIO 26 speaker.
  - Distinct musical reveal chimes tailored to answer sentiment: triumphant fanfare (Affirmative), ambient wobble (Neutral), low double buzz (Negative), and 8-bit cyber trills (Tech).
  - Multi-color RGB LED indicator (GPIO 4, 16, 17): Emerald Green, Amber, Ruby Red, or Electric Cyan.
- **Oracle Category Filtering**: Cycle between `[ALL (32)]`, `[CLASSIC (20)]`, and `[TECH (10)]`.

---

## 🛠️ Hardware Requirements

| Component | Specification |
|-----------|---------------|
| **Device** | Cheap Yellow Display (CYD 2.8" ESP32-2432S028R) |
| **Display** | ILI9341 320×240 TFT LCD |
| **Touch Controller** | XPT2046 Resistive Touchscreen (dedicated VSPI) |
| **Audio** | Built-in Piezo buzzer on GPIO 26 |
| **RGB LED** | On-board Tricolor LED (GPIO 4: Red, GPIO 16: Green, GPIO 17: Blue) |
| **Sensor Expansion** | Optional MPU-6050 or ADXL345 digital axis sensor on CN1/P3 connector |
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
| **Speaker**   | GPIO 26 | Tone / Acoustic Output |
| **LED Red**   | GPIO 4  | Negative / Alert LED (Active LOW) |
| **LED Green** | GPIO 16 | Affirmative LED (Active LOW) |
| **LED Blue**  | GPIO 17 | Idle / Fluid LED (Active LOW) |
| **I2C SDA**   | GPIO 27 | Optional Accelerometer Data (CN1) |
| **I2C SCL**   | GPIO 22 | Optional Accelerometer Clock (CN1) |

---

## 🚀 Getting Started

### Option 1: PlatformIO (Recommended)

1. Open this directory in VS Code with PlatformIO installed:
   ```bash
   cd projects/advanced/41-magic-8-cyd
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
4. Open [41-magic-8-cyd.ino](41-magic-8-cyd.ino).
5. Select board **ESP32 Dev Module**, choose the USB COM port, and click **Upload**.

---

## 🎮 How to Use

1. **Power On**: The Magic 8 Ball appears rendered in 3D shading with a glowing numeral 8.
2. **Ask a Question**: Think of a yes-or-no question in your mind.
3. **Shake the Ball**:
   - **Via Touch**: Tap anywhere on the Magic 8 Ball or press `[SHAKE >]` in the top header.
   - **Via Motion (Optional)**: If you plugged an MPU-6050 accelerometer module into CN1 (`SDA=27, SCL=22`), simply shake the physical CYD unit! The top bar will show `[AXIS: DETECT]`.
4. **Watch the Mystical Reveal**:
   - The ball swirls with dark blue fluid vortex particles while acoustic rattles play.
   - The blue triangle die surfaces with your answer.
   - The RGB LED reflects the verdict (Green for Yes, Red for No, Amber for Maybe, Cyan for Tech).
5. **Ask Another Question**: Tap the screen anywhere to submerge the die and return to the classic '8' faceplate.
6. **Configure Mode**: Tap `[MODE: ...]` in the header to switch between All 32 answers, Classic 20, or Tech/Maker 10.

---

## 📜 License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPLv3)**.

# 📻 34 - CYD-to-CYD Walkie-Talkie & Acoustic Voice Intercom

> Half-duplex peer-to-peer walkie-talkie for two or more Cheap Yellow Displays. Press-to-Talk (PTT) capacitive interface, multi-frequency acoustic transmissions, live spectrum visualizer, and instant chime dispatch over ESP-NOW!

[![Web Flasher](https://img.shields.io/badge/Web%20Flasher-Ready%20to%20Flash-brightgreen.svg)](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/)
[![License: GNU AGPLv3](https://img.shields.io/badge/License-AGPLv3-blue.svg)](../../LICENSE)

---

## 📺 Community Channels
- 🎬 **Tech, Coding & Build Videos**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printing, Enclosures & CAD**: [@Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🎯 Features

- **Push-to-Talk (PTT)**: Hold down the main tactile green PTT button to transmit; turns bright RED with `TRANSMITTING` feedback and streams acoustic tone packets over ESP-NOW.
- **Audio Output**: Plays directly through the onboard FM8002A class-AB audio amplifier on GPIO 26.
- **Spectrum Visualizer**: Real-time 16-bar graphic equalizer animation reacts dynamically to incoming audio and outbound transmissions.
- **Tactical Presets**:
  - `[CALL CHIME]`: Sends a melodic two-tone door/pager chime to alert the receiver.
  - `[ROGER BEEP]`: Sends a sharp 2.2kHz radio mic confirmation chirp.
  - `[SOS SIREN]`: Sends a multi-tone wailing siren burst in emergency scenarios.
- **Frequency Selector**: Choose carrier tones from 400Hz, 800Hz, 1200Hz, or 2000Hz.

---

## 🛠️ Hardware Requirements

- **2x or more ESP32-2432S028R** (Resistive) or **ESP32-2432S028C** (Capacitive)
- Pair with the [CYD Handheld Cyberdeck Shell](../../../enclosures/cyd-2.8-cyberdeck-handheld/) for a complete walkie-talkie radio experience!

---

## 🚀 Quick Start (Web Flasher)

1. Open the [CYD Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/) in Chrome or Edge.
2. Select **34 — CYD-to-CYD Walkie-Talkie**.
3. Flash both units in 1-click!

---

## 💻 Building with PlatformIO

```bash
git clone https://github.com/tushar2704/Cheap-Yellow-Display-Community.git
cd Cheap-Yellow-Display-Community/projects/advanced/34-cyd-to-cyd-walkie-talkie
pio run --target upload
```

---

## 📜 License
Licensed under GNU AGPLv3. See [LICENSE](../../LICENSE) for details.

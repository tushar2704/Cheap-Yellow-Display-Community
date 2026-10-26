# 💬 31 - CYD-to-CYD Tactical P2P Messenger & Pager

> Off-grid decentralized wireless text communicator for two or more Cheap Yellow Displays. Communicate peer-to-peer via ESP-NOW with full on-screen touch QWERTY keyboard, automatic beacon discovery, and double-piezo chime alerts.

[![Web Flasher](https://img.shields.io/badge/Web%20Flasher-Ready%20to%20Flash-brightgreen.svg)](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/)
[![License: GNU AGPLv3](https://img.shields.io/badge/License-AGPLv3-blue.svg)](../../LICENSE)

---

## 📺 Community Channels
- 🎬 **Tech, Coding & Build Videos**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printing, Enclosures & CAD**: [@Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🎯 Features

- **Zero Routers, Zero Cloud**: Uses native 2.4GHz ESP-NOW raw MAC frames for instant off-grid communications.
- **On-Screen Touch QWERTY Keyboard**: Full 4-row touch keyboard with letter keys, number/symbol toggle (`123` / `ABC`), spacebar, and backspace.
- **Rolling Chat History**: Real-time bubble view displaying sent and received messages with sender prefixes and auto-scroll.
- **Node Beacon & Auto-Discovery**: Periodically emits presence beacons; discovers peers within range automatically and displays their unique node ID (e.g. `CYD-B4A1`).
- **Acoustic & Optical Receipts**: Tactile double-piezo chime and blue LED pulse when messages arrive.

---

## 🛠️ Hardware Requirements

- **2x or more ESP32-2432S028R** (Resistive) or **ESP32-2432S028C** (Capacitive)
- Pair with the [CYD Handheld Cyberdeck Shell](../../../enclosures/cyd-2.8-cyberdeck-handheld/) for a complete portable off-grid terminal!

---

## 🚀 Quick Start (Web Flasher)

1. Open the [CYD Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/) in Chrome or Edge.
2. Select **31 — CYD-to-CYD Tactical Messenger**.
3. Flash both units in 1-click and start chatting!

---

## 💻 Building with PlatformIO

```bash
git clone https://github.com/tushar2704/Cheap-Yellow-Display-Community.git
cd Cheap-Yellow-Display-Community/projects/advanced/31-cyd-to-cyd-tactical-messenger
pio run --target upload
```

---

## 📜 License
Licensed under GNU AGPLv3. See [LICENSE](../../LICENSE) for details.

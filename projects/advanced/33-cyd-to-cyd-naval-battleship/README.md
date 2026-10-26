# ⚓ 33 - CYD-to-CYD Naval Battleship Tactical Combat

> Turn-based two-player naval warfare for two Cheap Yellow Displays. Secretly place your naval armada, track enemy radar coordinates, and engage in off-grid artillery duels over ESP-NOW!

[![Web Flasher](https://img.shields.io/badge/Web%20Flasher-Ready%20to%20Flash-brightgreen.svg)](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/)
[![License: GNU AGPLv3](https://img.shields.io/badge/License-AGPLv3-blue.svg)](../../LICENSE)

---

## 📺 Community Channels
- 🎬 **Tech, Coding & Build Videos**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printing, Enclosures & CAD**: [@Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🎯 Game Features

- **Phase 1: Tactical Deployment**:
  - 8x8 Naval Grid (Carrier, Battleship, Cruiser, Submarine).
  - Tap cells or click **`[AUTO DEPLOY]`** for strategic dispersion.
  - Tap **`[LOCK & READY]`** to synchronize over ESP-NOW.
- **Phase 2: Combat & Sonar Radar**:
  - Live Enemy Radar on left screen: tap grid coordinates to launch artillery.
  - Mini Status Radar on right side displaying your own ship health.
  - Instant Hit (`X` / Red) and Miss (`O` / Cyan) telemetry over sub-5ms packets.
  - Acoustic explosions and LED visual alerts on direct hits.
- **Victory / Defeat Celebrations**:
  - Dynamic victory fanfare when all enemy ships are sunk.
  - Instant rematch with the tap of a button.

---

## 🛠️ Hardware Requirements

- **2x ESP32-2432S028R** (Resistive) or **ESP32-2432S028C** (Capacitive)
- **Zero setup**: Automatic peer pairing over raw ESP-NOW broadcast frames.

---

## 🚀 Quick Start (Web Flasher)

1. Open the [CYD Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/) in Chrome or Edge.
2. Select **33 — CYD-to-CYD Naval Battleship**.
3. Flash both devices in 1-click!

---

## 💻 Building with PlatformIO

```bash
git clone https://github.com/tushar2704/Cheap-Yellow-Display-Community.git
cd Cheap-Yellow-Display-Community/projects/advanced/33-cyd-to-cyd-naval-battleship
pio run --target upload
```

---

## 📜 License
Licensed under GNU AGPLv3. See [LICENSE](../../LICENSE) for details.

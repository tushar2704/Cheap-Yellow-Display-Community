# 🎛️ 35 - CYD-to-CYD Master-Worker Dual-Screen Extended Display & Cockpit HUD

> Transform two Cheap Yellow Displays into a wireless multi-monitor rig! One CYD serves as a tactical master control surface with sliders, joysticks, and illuminated toggle switches, while the second CYD operates as an extended real-time HUD instrument cluster at 40Hz over ESP-NOW.

[![Web Flasher](https://img.shields.io/badge/Web%20Flasher-Ready%20to%20Flash-brightgreen.svg)](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/)
[![License: GNU AGPLv3](https://img.shields.io/badge/License-AGPLv3-blue.svg)](../../LICENSE)

---

## 📺 Community Channels
- 🎬 **Tech, Coding & Build Videos**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printing, Enclosures & CAD**: [@Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🎯 Architecture

```
┌─────────────────────────┐             ┌─────────────────────────┐
│     MASTER CYD          │  ESP-NOW    │       SLAVE CYD         │
│  (CONTROL SURFACE)      │ ──────────> │   (COCKPIT / HUD VIEW)  │
├─────────────────────────┤   40 Hz     ├─────────────────────────┤
│ • Vertical Throttle     │  Packets    │ • Airspeed & Alt Tapes  │
│ • Pitch/Roll Trackpad   │             │ • Artificial Horizon    │
│ • 3x Latching Toggles   │             │ • Live Tactical Status  │
└─────────────────────────┘             └─────────────────────────┘
```

- **Master Screen**:
  - Vertical throttle slider (0–100%) with linear response.
  - Interactive flight pitch/roll trackpad ($-45^\circ$ to $+45^\circ$).
  - 3 Aircraft illuminated switches: `[GEAR]`, `[RADAR]`, and `[AFTERBURNER]`.
- **Slave Screen**:
  - Full-screen fighter jet Heads-Up Display (HUD) with moving pitch ladders and roll horizon line.
  - Vertical tape gauges for airspeed (knots) and barometric altitude.
  - Tactical status banners (`GEAR DN`, `RADAR LOCK`, `BURNER ACTIVE`).
- **One-Tap Role Switching**: Toggle either display between `MASTER` and `SLAVE` dynamically.

---

## 🛠️ Hardware Requirements

- **2x ESP32-2432S028R** (Resistive) or **ESP32-2432S028C** (Capacitive)
- Pair with the [Dual Desktop Wedge Stand](../../../enclosures/cyd-2.8-desktop-stand/) for the ultimate cockpit battlestation!

---

## 🚀 Quick Start (Web Flasher)

1. Open the [CYD Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/) in Chrome or Edge.
2. Select **35 — CYD-to-CYD Extended Display & HUD**.
3. Flash both units in 1-click!

---

## 💻 Building with PlatformIO

```bash
git clone https://github.com/tushar2704/Cheap-Yellow-Display-Community.git
cd Cheap-Yellow-Display-Community/projects/advanced/35-cyd-to-cyd-extended-display
pio run --target upload
```

---

## 📜 License
Licensed under GNU AGPLv3. See [LICENSE](../../LICENSE) for details.

# 📡 Project 05 — CYD 2.8" BLE Proximity Radar & iBeacon Tracker

> Turn your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a military-grade circular BLE Proximity Radar and iBeacon Tracker. Scans nearby Bluetooth Low Energy peripherals, smart tags (Apple AirTags, Tile, Chipolo), smartwatches, and beacons. Plots targets on a sweeping green radar scope with concentric distance rings (-40 dBm to -100 dBm), shows interactive touch-selectable target telemetry, and chirps upon detection via the onboard piezo. Pairs with the [3D Desktop Wedge Stand](../../../enclosures/cyd-2.8-desktop-stand/) by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐⭐ Advanced (Level 4/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch CYD)
- **Radio Protocol**: Bluetooth Low Energy (BLE 4.2 / 5.0 Advertisements)
- **Libraries Used**: `BLEDevice`, `BLEScan`, `BLEAdvertisedDevice`, `TFT_eSPI`, `XPT2046_Touchscreen`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the BLE Radar Walkthrough & Tracking Test**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Ergonomic Radar Stand**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Circular Sweeping Radar Display**: Smooth 360-degree rotating beam with trailing phosphor glow simulation.
- **Concentric Range Rings**: Accurate visual distance categorization:
  - **-40 dBm**: Immediate Proximity (< 0.5 meter)
  - **-60 dBm**: Near Proximity (< 2.0 meters)
  - **-80 dBm**: Mid Range (2 to 5 meters)
  - **-100 dBm**: Peripheral Horizon (> 5 meters)
- **Live Target Drawer**: Scrollable sidebar showing tracked device names, MAC IDs, live RSSI, and proximity status.
- **Interactive Touch Target Lock**: Tap any target in the list or blip on screen to lock tracking, trigger sound ping, and illuminate indicator.
- **Audio Sonar Feedback**: Piezo chirps (GPIO 26) when new targets enter the scan perimeter.
- **Multi-Color RGB LED Status**:
  - 🟢 **Green Sweep**: Radar active and sweeping.
  - 🔵 **Blue Lock**: Apple AirTag / iBeacon target locked.
  - 🟡 **Yellow Alert**: High RSSI immediate proximity tag detected.
- **100% Zero External Components**: Runs purely on the onboard ESP32 radio, display, piezo, and touch controller.

---

## 🚀 How to Flash & Run

### 1. 1-Click Flash in Your Browser (No Software to Install)
Open the [CYD Community Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/) in Chrome, Edge, or Opera, plug in your CYD 2.8" via USB, and click **Install BLE Proximity Radar**.

### 2. Or Build & Flash via PlatformIO:
```bash
cd projects/advanced/05-ble-ibeacon-radar
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)

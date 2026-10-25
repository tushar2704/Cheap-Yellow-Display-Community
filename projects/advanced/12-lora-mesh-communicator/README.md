# 📡 Project 12 — CYD 2.8" LoRa Meshtastic Terminal & Long-Range Communicator

> Transform your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a dedicated off-grid LoRa communicator and Meshtastic handheld terminal. Connects via UART or SPI to SX1262 / SX1276 LoRa transceivers (433MHz, 868MHz, 915MHz) or operates as an autonomous mesh terminal. Features a live chat viewport with RSSI/SNR signal diagnostics, touch message dispatcher, quick emergency presets (SOS, Location, Acknowledgment), and channel switcher. Pairs with the [3D Desktop Wedge Stand](../../../enclosures/cyd-2.8-desktop-stand/) by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐⭐ Advanced (Level 4/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch CYD)
- **Radio Protocol**: LoRa Chirp Spread Spectrum / Meshtastic Protocol
- **Frequencies**: 433 MHz / 868 MHz / 915 MHz (module dependent)
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the LoRa Mesh Communicator Range Test**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Handheld Field Communicator Case**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Off-Grid Long-Range Mesh**: Send and receive encrypted packets across multi-hop peer nodes without cellular, WiFi, or internet infrastructure.
- **Real-Time Chat Viewport**: Displays recent conversations color-coded by node with live RSSI dBm signal strength annotations.
- **Interactive Touch Dispatcher**:
  - `[SEND]`: Transmit currently buffered message payload.
  - `[OK / ACK]`: Quick 1-tap confirmation token.
  - `[SEND SOS]`: Priority distress alert with emergency acoustic beacon warble.
  - `[LOC CHECK]`: GPS coordinates and telemetry broadcast.
  - `[CH SWITCH]`: Cycle through `#LongFast`, `#Emergency`, and `#Admin` channels.
- **Multi-State RGB LED Telemetry**:
  - 🔵 **Blue**: LoRa packet transmission (TX).
  - 🟢 **Green**: Standby & mesh carrier listening.
  - 🔴 **Red**: Emergency broadcast active.
- **Audio Feedback**: Frequency-modulated acoustic chirps via onboard speaker (GPIO 26).

---

## 🚀 How to Flash & Deploy

### 1. 1-Click Flash in Your Browser (No Software to Install)
Open the [CYD Community Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/) in Chrome, Edge, or Opera, plug in your CYD 2.8" via USB, and click **Install LoRa Meshtastic Terminal**.

### 2. Or Build & Flash via PlatformIO:
```bash
cd projects/advanced/12-lora-mesh-communicator
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)

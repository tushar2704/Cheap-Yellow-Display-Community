# 🌐 Project 06 — CYD 2.8" ESP-NOW Mesh Intercom & Touch Node

> Transform your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a decentralized, zero-router ESP-NOW mesh touch intercom and node controller. Operates completely independently of home WiFi routers, internet connections, or cloud servers. Broadcasts instant emergency alerts, doorbells, acknowledge pings, and telemetric signals with sub-10ms packet latency. Pairs with the [3D Desktop Wedge Stand](../../../enclosures/cyd-2.8-desktop-stand/) by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐⭐ Advanced (Level 4/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch CYD)
- **Wireless Protocol**: Espressif ESP-NOW (2.4GHz Peer-to-Peer, Channel 1)
- **Infrastructure Requirement**: **Zero WiFi routers or cloud needed**
- **Libraries Used**: `esp_now`, `WiFi`, `TFT_eSPI`, `XPT2046_Touchscreen`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the ESP-NOW Mesh Demonstration & Range Test**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Mesh Desk Enclosure**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Decentralized Zero-Router Mesh**: Operates anywhere off-grid using native 2.4GHz ESP-NOW raw MAC frame broadcast.
- **Sub-10 Millisecond Packet Latency**: Near-instantaneous transmission and acknowledgment.
- **Interactive Preset Dispatcher**:
  - 🔔 **DOORBELL**: Sends instant remote wireless chime alert.
  - 📡 **PING MESH**: Sends a ping heartbeat probe across all listening nodes.
  - ✅ **ACK / OK**: Quick receipt confirmation for team communications.
  - 🚨 **SOS ALERT**: High-priority emergency trigger with high-frequency acoustic alarm.
- **Live Mesh Intercom Feed**: Real-time rolling activity log tracking transmissions (TX>) and peer receipts (RX<).
- **Acoustic & Optical Dual Telemetry**:
  - 🟢 **Green Confirmation**: Successful packet transmission delivery.
  - 🔵 **Blue Flash**: Incoming mesh message packet received.
  - 🔴 **Red Alert**: Transmission failed or emergency broadcast active.
  - 🔊 **Double Piezo Chime**: Tactile tone notifications on message receipt via GPIO 26.

---

## 🚀 How to Flash & Deploy

### 1. Flash Directly in Your Browser (1-Click, No IDE)
Open the [CYD Community Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/) in Chrome, Edge, or Opera, plug in your CYD 2.8" via USB, and click **Install ESP-NOW Mesh Node**.

### 2. Or Build & Flash via PlatformIO:
```bash
cd projects/advanced/06-esp-now-mesh-node
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)

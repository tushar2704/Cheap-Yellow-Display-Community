# 59 — CYD Marauder WiFi & BLE Security Auditor

> ⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A portable, standalone **RF cybersecurity diagnostic terminal & wireless security auditor** running on the ESP32 Cheap Yellow Display. Features hardware-accelerated 802.11 promiscuous packet sniffing across 2.4 GHz channels 1–13, real-time Deauthentication & Disassociation attack intrusion detection with audible alarms, WiFi Access Point discovery, BLE beacon and AirTag tracking, and a live RF channel spectrum waterfall gauge.

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 🛡️ 4 Diagnostic HUD Modes
1. **Mode 0 — NETWORKS (WiFi AP Scanner)**:
   - Scans 2.4 GHz channels 1 through 13.
   - Discovers nearby access points, SSID target names, operating channel, RSSI signal level (-dBm), and encryption standard (`OPEN`, `WPA2`, `WPA3`).
   - Touch screen to trigger an instant active rescan.
2. **Mode 1 — SNIFFER (Promiscuous Packet Monitor)**:
   - Real-time packet telemetry engine capturing all ambient 802.11 frames.
   - Independent telemetry counters for Total Packets, Management Frames, Beacon Frames, and Deauth Frames.
   - **2.4 GHz 13-Channel RF Waterfall**: Live bar graph displaying packet traffic density across channels 1–13 with current active channel highlight.
   - **Acoustic Geiger Counter**: Subtle audio click on frame reception via onboard piezo buzzer.
3. **Mode 2 — DEAUTH (Intrusion Defense Shield)**:
   - Active intrusion detection monitor scanning for malicious 802.11 deauthentication and disassociation frames.
   - Triggers bright crimson on-screen alert banner, red alert RGB LED pulse, and high-frequency piezo alarm on attack detection.
   - Logs recent intrusion events with source MAC, target MAC, and frame reason code.
4. **Mode 3 — BLE (Bluetooth Beacon & Tag Sniffer)**:
   - Monitors 2.4 GHz Bluetooth Low Energy advertising packets.
   - Detects nearby BLE beacons, Apple AirTags, Galaxy SmartTags, smart fitness bands, and HID devices with RSSI distance estimates.

### 💡 Multi-Color Status Indicators & Audio Haptics
- **Geiger Audio Tick**: Toggleable acoustic feedback mimicking a Geiger counter on packet bursts.
- **Intrusion Alarm**: High-frequency alarm siren on detected deauth floods.
- **Tricolor LED Aura**: Electric Cyan during passive sniffing, Emerald Green during active AP scanning, and Crimson Red during alert state.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Onboard Piezo Buzzer on **GPIO 26** for geiger ticks and alarm siren
- Onboard RGB LED on **GPIO 4, 16, 17**
- Micro-USB or USB-C power source (5V, 1A)
- Cyberdeck / Handheld Auditor 3D Enclosure by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Navigate to the project directory:
   ```bash
   cd projects/advanced/59-cyd-marauder-wifi-ble-security-auditor
   ```
2. Build and flash via PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```
3. Use the bottom navigation bar to switch between Networks, Sniffer, Deauth Shield, and BLE scanner modes!

---

## 📌 Pinout Reference

| Component | Pin | Function |
|---|---|---|
| TFT LCD (ILI9341) | MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21 | 320×240 Cybersecurity HUD |
| Resistive Touch (XPT2046) | CS=33, CLK=25, MOSI=32, MISO=39, IRQ=36 | Mode Tabs, Geiger Toggle, Rescan |
| RGB LED | R=4, G=16, B=17 (Active LOW) | Status & Intrusion Alert Visualizer |
| Buzzer | GPIO 26 | Geiger Ticks & Intrusion Alarm Siren |

---

## 📺 Community & Enclosures

- 📺 **IamNotIntoTech YouTube**: [Explore more CYD Tutorials](https://youtube.com/@IamNotIntoTech)
- 🖨️ **Makerrdesigns 3D Files**: [Download Cyberdeck Auditor Case](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community


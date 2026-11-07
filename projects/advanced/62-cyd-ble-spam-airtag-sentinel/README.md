# 62 — CYD Apple Juice & BLE Spam Sentinel

> ⭐⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A dedicated **cybersecurity RF defense monitor & Bluetooth Low Energy intrusion sentinel** running on the ESP32 Cheap Yellow Display. Features real-time promiscuous BLE packet inspection to identify and alert on Apple Juice popup crashes (AirPods pairing popups, AppleTV setup spam), Android Google Fast Pair floods, Windows Swift Pair spam, and unauthorized Apple FindMy AirTag or Tile tracking beacons with acoustic alarm sirens and red alert RGB flashes.

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 🛡️ 3 Cybersecurity Defense Views
1. **View 0 — DEFENSE SHIELD (Real-Time Airspace Monitor)**:
   - High-visibility central radar shield displaying instantaneous threat level (`RF AIRSPACE SECURE` vs `BLE SPOOFING ATTACK DETECTED!`).
   - Live traffic density monitor showing BLE frames per second and total frames analyzed.
   - 4 independent threat category counters:
     - `Apple Juice`: iOS Action popup spam (AirPods, AppleTV, Transfer Number).
     - `Fast Pair`: Android Google Fast Pair flood frames.
     - `Swift Pair`: Windows Microsoft Swift Pair flood frames.
     - `AirTags`: Apple FindMy offline tracking beacons.
2. **View 1 — ATTACK LOGS (Intrusion Incident Recorder)**:
   - Live scrolling chronological log capturing recent spoofing attacks.
   - Records Attack Vector, Spoofed Source MAC address, and Signal Strength (-dBm).
3. **View 2 — AIRTAG RADAR (Stalker & Beacon Finder)**:
   - Dedicated tracker radar identifying nearby Apple AirTags, Samsung SmartTags, and Tile beacons.
   - Dynamic signal level bars to assist in physical direction finding and proximity estimation.

### 💡 Acoustic Intrusion Siren & Red Alert Strobe
- **Intrusion Alarm**: High-frequency dual-tone piezo siren (`2800 Hz` / `3400 Hz`) triggering immediately when malicious spam bursts or rogue AirTags are detected.
- **Tricolor LED Alert**: Emerald Green during clean scanning, Electric Cyan during passive reception, and flashing Crimson Red during intrusion alerts.
- **Mute Toggle**: One-touch on-screen toggle to silence the acoustic alarm while preserving visual alerts.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Onboard Piezo Buzzer on **GPIO 26** for intrusion siren
- Onboard RGB LED on **GPIO 4, 16, 17** for threat visualizer
- Micro-USB or USB-C power source (5V, 1A)
- Cyberdeck / Handheld Sentinel 3D Enclosure by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Navigate to the project directory:
   ```bash
   cd projects/advanced/62-cyd-ble-spam-airtag-sentinel
   ```
2. Build and flash via PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```
3. Power up the device. Tap the center shield to run a simulated attack demo or use the bottom tabs to inspect live logs!

---

## 📌 Pinout Reference

| Component | Pin | Function |
|---|---|---|
| TFT LCD (ILI9341) | MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21 | 320×240 Cybersecurity Terminal |
| Resistive Touch (XPT2046) | CS=33, CLK=25, MOSI=32, MISO=39, IRQ=36 | Tab Navigation & Alarm Mute Toggle |
| RGB LED | R=4, G=16, B=17 (Active LOW) | Intrusion Threat Warning Strobe |
| Buzzer | GPIO 26 | Dual-Tone Acoustic Intrusion Siren |

---

## 📺 Community & Enclosures

- 📺 **IamNotIntoTech YouTube**: [Explore more CYD Tutorials](https://youtube.com/@IamNotIntoTech)
- 🖨️ **Makerrdesigns 3D Files**: [Download Handheld Sentinel Cases](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community

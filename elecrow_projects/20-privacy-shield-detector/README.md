# 🛡️ Project 20: Privacy Shield BLE Tracker Detector

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Passive BLE Surveillance Tag Sniffer**:
   - Continuously scans 2.4GHz BLE advertisement payloads for manufacturer signatures corresponding to Apple Find My (AirTag), Samsung Galaxy SmartTag, and Tile trackers.
   - Operates 100% passively with zero cloud dependencies or smartphone pairing required.
2. **Proximity Radar & Distance Estimation**:
   - Computes distance from measured RSSI signal attenuation using logarithmic RF path-loss models.
   - Real-time rotating radar beam displaying color-coded blips for detected tags across Immediate (<1.5m), Near (<5m), and Far (>5m) rings.
3. **Persistent Following / Stalker Alert Engine**:
   - Tracks duration and continuity of unidentified tags. If an unknown tracker stays within proximity across multiple scans (>45s), an on-screen warning banner and red safety alert are triggered.
4. **Geiger-Counter Style Audio Clicks**:
   - Onboard piezo buzzer produces auditory clicks that accelerate in frequency as you approach an active tracker, enabling rapid physical locating.
5. **Hardware Safety Indicator**:
   - Active-low RGB LED glows Green when surroundings are clear, Yellow when a tracker is in vicinity, and Red during persistent following alerts.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — Threat alert |
| **GPIO 26** | Audio / Buzzer | Geiger-counter proximity clicks |
| **BLE Radio** | 2.4GHz Bluetooth LE | Promiscuous Advertisement Sniffer |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/20-privacy-shield-detector
pio run -t upload
```

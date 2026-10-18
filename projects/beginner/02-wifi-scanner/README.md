# 📶 Project 02 — WiFi Scanner & Network Analyzer

> Turn your Cheap Yellow Display (CYD) into a portable handheld 2.4 GHz WiFi spectrum analyzer with signal strength bar graphs, encryption detection, and an interactive touch re-scan button.

---

## 📋 Overview

- **Difficulty**: ⭐ Beginner (Level 1/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **External Components Required**: **None** (100% onboard ESP32 WiFi & display)
- **Libraries Used**: `WiFi`, `TFT_eSPI`, `XPT2046_Touchscreen`

---

## 📺 Video Tutorials

- 🎬 **Watch the Build & Code Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Handheld Cyberdeck Case**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Real-Time 2.4 GHz Channel Scan**: Discovers all active SSIDs, channels (1–14), and security protocols (Open, WPA, WPA2, WPA3).
- **Color-Coded Signal Strength (RSSI)**:
  - 🟢 **Green**: > -60 dBm (Excellent signal)
  - 🟡 **Yellow**: -60 to -75 dBm (Moderate signal)
  - 🔴 **Red**: < -75 dBm (Weak signal)
- **Interactive Touch "SCAN NOW"**: Tap the top right button on the screen to trigger an immediate live re-scan.
- **Audio & Visual Feedback**: Tone beeps and onboard RGB LED status indication during scan.

---

## 🚀 How to Flash & Run

### PlatformIO:
```bash
cd projects/beginner/02-wifi-scanner
pio run -t upload
```

### Arduino IDE:
1. Ensure `TFT_eSPI` is configured for your CYD variant.
2. Select **ESP32 Dev Module**.
3. Upload `02-wifi-scanner.ino`.

---

## 📜 License
MIT License — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)

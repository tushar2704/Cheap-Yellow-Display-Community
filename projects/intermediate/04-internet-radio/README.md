# 📻 Project 04 — CYD 2.8" WiFi Internet Radio Streamer

> Turn your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a standalone web radio player. Streams live MP3 web radio stations over WiFi directly to the onboard speaker using the ESP32 internal DAC on GPIO 26 with an interactive touchscreen GUI.

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐ Intermediate (Level 3/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **Audio Output**: Onboard Speaker (GPIO 26 Internal DAC) or external amplifier via P1 JST port
- **Libraries Used**: `WiFi`, `TFT_eSPI`, `XPT2046_Touchscreen`, `ESP32-audioI2S`

---

## 📺 Video Tutorials

- 🎬 **Watch the Build & Code Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Vintage Tabletop Radio Housing**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Live WiFi Audio Streaming**: Streams live Shoutcast/Icecast MP3 stations directly over WiFi.
- **Onboard DAC Audio Playback**: Utilizes ESP32 Channel 2 8-bit DAC on GPIO 26 connected to the onboard amplifier — no external I2S DAC breakout board required!
- **Touch Station Switcher**: Tap `< PREV` and `NEXT >` to cycle through streaming presets.
- **Touch Volume Slider**: Drag or tap the interactive volume bar on screen.
- **Live Stream Title Readout**: Displays the currently playing song metadata.

---

## 🚀 How to Flash & Run

### 1. Configure WiFi Credentials
In `04-internet-radio.ino`:
```cpp
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

### 2. Upload via PlatformIO:
```bash
cd projects/intermediate/04-internet-radio
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)


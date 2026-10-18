# ⏱️ Project 03 — CYD 2.8" Rolling Digit NTP Desk Clock

> A retro-modern digital desk clock for the 2.8" Cheap Yellow Display (ESP32-2432S028R) inspired by mechanical split-flap and rolling clocks. Synchronizes over WiFi with atomic NTP time servers.

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐ Intermediate (Level 3/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **External Components Required**: **None** (100% onboard ESP32 WiFi & TFT)
- **Libraries Used**: `WiFi`, `TFT_eSPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the Build & Code Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Retro Clock Housing**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Atomic NTP Synchronization**: Automatically updates time and date from `pool.ntp.org`.
- **Split-Flap Digit Styling**: High-contrast split-plate clock display.
- **Hourly Chime**: Gentle audio beep on the hour via onboard speaker pin 26.
- **WiFi Status LED**: Onboard green LED turns solid when WiFi clock is synchronized.

---

## 🚀 How to Flash & Run

### 1. Configure WiFi & Timezone
In `03-rolling-clock.ino`:
```cpp
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Adjust for your UTC offset in seconds (e.g. UTC+1 = 3600, UTC-5 = -18000)
const long GMT_OFFSET_SEC = 0;
```

### 2. Upload via PlatformIO:
```bash
cd projects/intermediate/03-rolling-clock
pio run -t upload
```

---

## 📜 License
Dual License: AGPL-3.0 for code & CC BY-NC-ND 4.0 for assets | Author: [@tushar2704](https://github.com/tushar2704)

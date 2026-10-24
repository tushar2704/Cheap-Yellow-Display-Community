# 🌦️ Project 01 — CYD Smart Weather Station & NTP Desk Clock

> A connected desktop weather dashboard and NTP synchronized clock for your Cheap Yellow Display (CYD). Fetches live meteorological data using the 100% free **Open-Meteo API** (zero API key required!).

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐ Intermediate (Level 3/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **External Components Required**: **None** (100% onboard ESP32 WiFi & TFT)
- **Libraries Used**: `WiFi`, `HTTPClient`, `ArduinoJson`, `TFT_eSPI`, `XPT2046_Touchscreen`

---

## 📺 Video Tutorials

- 🎬 **Watch the Full Code Walkthrough & Setup**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Angled Desk Stand Enclosure**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Live Weather via Open-Meteo**: Current temperature (°C), relative humidity, wind speed, weather condition descriptions, and daily high/low forecast.
- **NTP Synchronized Real-Time Clock**: Synchronizes with global atomic time servers (`pool.ntp.org`) for precision down to the second.
- **Zero API Keys**: Uses public Open-Meteo endpoints — no accounts or credit cards required.
- **Modern UI Design**: Dark background with high-contrast colored cards for optimal viewing from across the room.

---

## 💰 Premium Upgrade Edition (Gumroad)

While this repository provides the full, complete open-source weather station for free, we also offer a **Premium Weather Pro Bundle** on Gumroad ($5.00) including:
- 📊 7-Day interactive graphical forecast with temperature trend curves.
- 🎨 3 custom UI themes (Cyberpunk Neon, Clean Minimalist Light, and Retro CRT).
- 🖨️ Complete STL & STEP source files for the magnetic snap-fit desktop enclosure by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).
- 📍 Multi-city touch switcher with weather alerts & air quality index (AQI).

---

## 🚀 How to Flash & Run

### 1. Configure WiFi & Location
Edit `01-weather-station.ino`:
```cpp
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Enter your Latitude and Longitude
const float LATITUDE  = 40.7128;
const float LONGITUDE = -74.0060;
const char* CITY_NAME = "New York, USA";
```

### 2. Upload via PlatformIO:
```bash
cd projects/intermediate/01-weather-station
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)


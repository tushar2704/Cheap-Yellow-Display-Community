# 🏠 Project 02 — CYD 2.8" Home Assistant Smart Touch Controller

> Transform your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a dedicated wall-mounted smart home command center. Controls Home Assistant lights, smart switches, fans, and climate thermostats via the Home Assistant REST API. Pairs with the [3D Flush Wall Mount](../../../enclosures/cyd-2.8-wall-mount/) by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐⭐ Advanced (Level 4/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **Required Backend**: Home Assistant server (OS, Supervised, or Container)
- **Libraries Used**: `WiFi`, `HTTPClient`, `ArduinoJson`, `TFT_eSPI`, `XPT2046_Touchscreen`

---

## 📺 Video Tutorials

- 🎬 **Watch the Full Home Assistant Integration Guide**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Flush Wall Bracket Case**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **4 Device Control Tiles**: Instant status toggles for Living Room Light, Desk Lamp, Ceiling Fan, and Entertainment Center.
- **Thermostat Climate Adjuster**: Interactive `[-]` and `[+]` touch buttons to set heating/cooling setpoints with 0.5°C step precision.
- **Visual Glow Indicators**: Active tiles glow in high-contrast amber/yellow; off devices remain dark slate.
- **Audible Chimes**: Tactile audio click feedback on every wall panel interaction.
- **Onboard RGB Status LED**: Solid green when connected to WiFi and authenticated; flashes blue during active API calls.

---

## 🚀 How to Configure & Flash

### 1. Generate Home Assistant Token
1. Open your Home Assistant web dashboard.
2. Click your profile icon (bottom left) ➔ scroll to **Long-Lived Access Tokens**.
3. Click **Create Token**, name it `CYD Wall Panel`, and copy the token string.

### 2. Enter Credentials in `02-home-assistant-touch.ino`
```cpp
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* HA_SERVER_URL = "http://homeassistant.local:8123";
const char* HA_BEARER_TOKEN = "paste_your_long_lived_token_here";
```

### 3. Upload via PlatformIO:
```bash
cd projects/advanced/02-home-assistant-touch
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)


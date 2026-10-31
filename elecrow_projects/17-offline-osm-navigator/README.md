# 🗺️ Project 17: Offline OSM Trail Navigator & Topo Deck

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  
> **Location Anchor**: **Paris Eiffel Tower Sector (JN18eu)**

---

## 🚀 Key Features

1. **Offline Paris Topographic Vector Map**:
   - Centered on Paris Eiffel Tower coordinates (`48.8584° N, 2.2945° E, Alt: 36m MSL`).
   - Procedural vector topographic map renderer depicting the Seine River waterway, elevation contours, and Champ de Mars / Pont d'Iéna pedestrian trails.
   - User beacon marker with red forward compass heading vector needle.

2. **Glove-Optimized Touch Navigation**:
   - Right sidebar high-target touch buttons:
     - `+` : Zoom In (up to Level 18)
     - `▲` : Pan North / Up
     - `▼` : Pan South / Down
     - `-` : Zoom Out (down to Level 10)
     - `WP` : Drop interactive GPS Waypoint
   - Direct canvas touch: tap anywhere on the map to smoothly pan toward the touched coordinate with acoustic feedback.

3. **Storage & Multi-Bus Architecture**:
   - Dual-SPI bus architecture:
     - Display on HSPI (`MOSI: 13, MISO: 12, SCLK: 14, CS: 15, DC: 2`)
     - Touch Controller on VSPI (`MOSI: 32, MISO: 39, CLK: 25, CS: 33`)
     - MicroSD on HSPI (`CS: 5, MOSI: 23, MISO: 19, SCK: 18`)
   - MicroSD Auto-Detection with seamless fallback to internal vector storage.

4. **Hardware Indicators**:
   - Active-Low RGB LED: Green (SD Active), Yellow (Internal vector memory mode), Amber flash (Waypoint registered).
   - Audio buzzer feedback on GPIO 26.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC (HSPI) |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 25, 39, 32** | XPT2046 Touch SPI | CS, CLK, MISO, MOSI (Dedicated VSPI, Polling mode) |
| **GPIO 5, 23, 19, 18** | MicroSD Slot SPI | CS, MOSI, MISO, SCK (HSPI) |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone generation |

---

## ⚙️ Configuration (`secrets.h`)

Create a local `secrets.h` file (git-ignored for security):

```cpp
#pragma once

#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Station Reference Coordinates (Paris Eiffel Tower)
#define STATION_LAT   48.8584
#define STATION_LON   2.2945
#define STATION_ALT   36
```

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/17-offline-osm-navigator
pio run -t upload --upload-port COM5
```

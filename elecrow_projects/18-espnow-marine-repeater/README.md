# ⛵ Project 18: ESP-NOW Marine Instrument Repeater

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  
> **Networking**: **2.4 GHz Peer-to-Peer ESP-NOW Wireless Mesh** (Zero router required)

---

## 🚀 Key Features

1. **Zero-Router Marine Telemetry**:
   - Ultra-low-latency 2.4 GHz ESP-NOW protocol connects masthead and keel transducers directly to the cockpit display.
   - Dynamic packet parsing (`struct_marine_packet`): Wind Speed, Wind Angle, Barometric Pressure, Water Depth, Water Temp, and Transmitter Battery Voltage.

2. **360° Polar Wind Rose & Sailing Direction**:
   - High-contrast apparent wind angle dial with color-coded port (Red) and starboard (Green) needles.
   - Central yacht outline and digital speed readout in Knots or Meters/sec.

3. **Multi-Sensor Instrument Telemetry Stack**:
   - **Water Depth**: Sonar depth in feet or meters with automatic shallow water (< 2.5m) visual and acoustic alarm.
   - **Barometer / Squall**: Barometric pressure in hPa with storm trend detection.
   - **Water Temperature**: Surface temperature in °F or °C.

4. **Tactical Lighting & Unit Conversion**:
   - **Night Mode Toggle**: Preserves night vision with deep red illumination palette for offshore night watches.
   - **Unit Toggle**: Instantly switch between Nautical (Knots/Feet/°F) and Metric (m/s/Meters/°C).

5. **Hardware Alarm & Indicators**:
   - **Red LED**: Critical shallow water warning (< 2.5m)
   - **Yellow LED**: Gale force wind alert (> 22 knots)
   - **Green LED**: Safe cruising
   - Dedicated buzzer alarm on GPIO 26.

6. **Calibrated Touch Controls**:
   - Dedicated buttons: `NAUTICAL / METRIC`, `ALARM MUTE`, `DAY / NIGHT MODE`.
   - Dedicated VSPI hardware touch bus (`MOSI: 32, MISO: 39, CLK: 25, CS: 33`) with polling mode and calibrated non-inverted Y mapping.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC (HSPI) |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 25, 39, 32** | XPT2046 Touch SPI | CS, CLK, MISO, MOSI (Dedicated VSPI, Polling mode) |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Shallow depth alarm and button tone generation |

---

## ⚙️ Configuration (`secrets.h`)

Create a local `secrets.h` file (git-ignored for security):

```cpp
#pragma once

#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Station Reference Coordinates (Paris Seine / Marine Sector)
#define STATION_LAT   48.8584
#define STATION_LON   2.2945
```

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/18-espnow-marine-repeater
pio run -t upload --upload-port COM5
```

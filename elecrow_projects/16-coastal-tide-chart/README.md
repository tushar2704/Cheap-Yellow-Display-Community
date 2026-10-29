# 🌊 Project 16: Live Coastal Tide Chart & Lunar Tracker

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Continuous Harmonic Tide Curve Engine**:
   - Computes mathematical semi-diurnal harmonic tide curves (M2 / S2 tidal constituents) across a continuous 24-hour diurnal timeline.
   - Smoothly plots high-resolution water elevation curves directly on the ILI9341 display.
2. **Real-Time Water Elevation Cursor**:
   - Real-time time indicator cursor with dynamic water height (meters / feet) and trend direction (Rising ▲ / Falling ▼).
   - High Tide and Low Tide peak height markers and mean sea level datum line.
3. **Lunar Phase & Spring / Neap Tide Calculations**:
   - Displays real-time moon illumination percentage (e.g. `84% GIBBOUS`) and classifies the astronomical tidal regime (Spring Tide vs Neap Tide).
4. **Golden Hour & Solar Astronomy Markers**:
   - Exact sunrise and sunset temporal points calculated for maritime navigation and photography.
5. **Interactive Coastal Harbor Switcher**:
   - One-tap touchscreen buttons across the display base to rapidly switch between monitored harbor stations (SF Bay, Dover, Sydney, Mumbai).
6. **Hardware Multicolored Tidal State Feedback**:
   - Active-low RGB LED glows Cyan for rising tide (flood), Blue for falling tide (ebb), and Amber for slack water peaks.
   - Acoustic frequency chimes on station switching and peak tidal events.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — Tidal state |
| **GPIO 26** | Audio / Buzzer | Frequency tone alert |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/16-coastal-tide-chart
pio run -t upload
```

# Project 29: F1 Pitwall Dashboard

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **F1 Pitwall Dashboard** transforms the Elecrow 2.8" CYD into a high-octane Formula 1 race engineering telemetry monitor. It provides live race sector delta intervals, tyre wear simulation and pit strategy timers, real-time championship driver standings, and dynamic track weather diagnostics.

The on-board on-board RGB LED provides instant FIA track status flag alerts (Green Flag, Safety Car, Full Course Yellow, Red Flag, and Blue Flag cautions), while the touch bottom tab bar allows seamless switching between pit telemetry views.

---

## Features

- **Live Leaderboard & Sector Gaps**: Displays top driver positions, interval gaps to leader, sector 1/2/3 mini-sector timing bars, and DRS detection zones.
- **FIA Track Status & RGB Indicator**:
  - Green Flag: Solid Green (`IO16`)
  - Full Course Yellow / SC: Solid Yellow (`IO4` + `IO16`)
  - Red Flag / Session Stoppage: Solid Red (`IO4`)
  - Blue Flag Caution: Flashing Blue (`IO17`)
- **Tyre Strategy & Pit Window Matrix**:
  - Compound wear degradation curve calculation (Soft, Medium, Hard compounds).
  - Pit stop turnaround countdown timer and delta pit-loss calculator.
- **Championship Standings**: Fast-scrolling constructor and driver points table with official team accent styling.
- **Weather Radar & Microclimate Telemetry**:
  - Track temperature, ambient air temperature, track surface wetness %, and barometer pressure trend.
- **Audible Pit Alarm**: Generates quick frequency chirps via on-board buzzer (`IO26`) on safety car deploys or pit window opens.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **RGB Status LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW race flag warning indicator |
| **Telemetry Audio Buzzer** | `GPIO 26` | Piezo audio alarm for radio pit cues |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD board via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/29-f1-pitwall-dashboard
   pio run --target upload
   ```
3. Set your local Wi-Fi SSID and credentials in `29-f1-pitwall-dashboard.ino` if connecting to live OpenF1 / Ergast telemetry webhooks, or let the firmware run the realistic telemetry simulation engine out of the box.

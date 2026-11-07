# 🛰️ Project 70 — CYD ISS & Satellite Orbit Tracker

> **A real-time International Space Station (ISS) and Low Earth Orbit (LEO) satellite tracking terminal for the Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch). Features a world map ground track projection with sinusoidal orbit trajectory, subsolar daytime illumination, polar Azimuth/Elevation antenna rotator pointing compass, Expedition crew manifest, overhead ground pass predictions with live countdown timer, and downlink telemetry audio chimes!**

[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/)
[![Display: ILI9341](https://img.shields.io/badge/Display-ILI9341_2.8%22-yellow.svg)](https://platformio.org/)
[![Touch: XPT2046](https://img.shields.io/badge/Touch-XPT2046_Resistive-red.svg)](https://platformio.org/)
[![Brand: IamNotIntoTech](https://img.shields.io/badge/Branded-IamNotIntoTech-FEA000.svg)](https://youtube.com/@IamNotIntoTech)

---

### 📺 Watch the Masterclass & 3D Enclosure
- 🎬 **Tutorial & Code Walkthrough**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printable Desktop Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🌟 Key Features

1. **4 Touch-Navigated Space Views**:
   - **View 1: Ground Track & World Map** — Simplified continental world map with real-time sinusoidal orbital ground track, current ISS position icon with solar array wings, live latitude/longitude, altitude (418 km), and orbital velocity (27,600 km/h).
   - **View 2: Antenna Az/El Compass** — Polar antenna pointing dome showing Azimuth (0–360° bearing) and Elevation angle (0–90° above horizon) for aiming amateur radio Yagi antennas and satellite tracking dishes, including Doppler shift telemetry (-2.4 kHz @ 145.800 MHz).
   - **View 3: Expedition Crew Manifest** — Roster of active astronauts on board the station, mission agency affiliations (NASA, Roscosmos), and real-time station life support telemetry (cabin pressure 101.3 kPa, solar array power 118 kW, oxygen concentration).
   - **View 4: Overhead Pass Schedule** — Table of next 3 visible horizon passes with Acquisition of Signal (AOS), Loss of Signal (LOS), maximum elevation angle, and live ticking countdown timer (`T - 01:14:22`).
2. **Synchronized Hardware Alerts**:
   - Active-low RGB LED pulses deep cosmic blue every 2 seconds as an orbital radio beacon, and flashes green when an overhead pass is active.
   - Piezo buzzer on `GPIO 26` synthesizes satellite downlink telemetry beeps and frequency chirps.
3. **LDR Sensor**:
   - Reads ambient room lighting on `GPIO 34` for automatic day/night screen contrast.

---

## 🔌 Hardware Pinout (ESP32-2432S028R)

| Component | Pin Function | ESP32 GPIO |
|---|---|---|
| **ILI9341 Display** | MOSI / MISO / SCLK / CS / DC | `GPIO 13` / `GPIO 12` / `GPIO 14` / `GPIO 15` / `GPIO 2` |
| **Backlight PWM** | TFT Backlight Enable | `GPIO 21` (Active HIGH) |
| **XPT2046 Touch** | CLK / MISO / MOSI / CS / IRQ | `GPIO 25` / `GPIO 39` / `GPIO 32` / `GPIO 33` / `GPIO 36` |
| **Orbital Beacon LED** | Blue (Orbit) / Green (Pass) | `GPIO 4` / `GPIO 16` / `GPIO 17` (Active LOW) |
| **Telemetry Speaker** | Downlink Chirps | `GPIO 26` |
| **LDR Sensor** | Ambient Light ADC | `GPIO 34` |

---

## 🛠️ How to Compile & Flash

### PlatformIO (Recommended)
```bash
cd projects/advanced/70-cyd-iss-satellite-tracker
pio run --target upload
```

### Arduino IDE
1. Open `70-cyd-iss-satellite-tracker.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and configure `User_Setup.h` for ILI9341.
4. Install **XPT2046_Touchscreen**.
5. Click **Upload**.

---

<p align="center">
  <b>Built with 🟡 by <a href="https://youtube.com/@IamNotIntoTech">IamNotIntoTech</a></b> &bull; 
  <b>3D Enclosures by <a href="https://youtube.com/@Makerrdesigns">Makerrdesigns</a></b>
</p>

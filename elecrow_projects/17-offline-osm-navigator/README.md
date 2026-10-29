# 🗺️ Project 17: Offline OSM Trail Navigator & Topo Deck

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Glove-Friendly Touch Navigation**:
   - Oversized resistive touch buttons designed for cold outdoor hiking with winter gloves (+ Zoom In, - Zoom Out, ▲ Pan Up, ▼ Pan Down, WP Drop Waypoint).
   - High-contrast outdoor-visible vector topological rendering with contour elevation lines, waterways, and hiking trails.
2. **MicroSD Offline Tile & Track Engine**:
   - Native integration with the onboard MicroSD card slot (IO5 CS, IO18 SCK, IO19 MISO, IO23 MOSI) for loading offline map tiles and logging GPX track files.
   - Built-in fallback topological vector generation if SD card is omitted.
3. **Breadcrumb GPX Waypoint Logging**:
   - Dynamic waypoint marker engine allowing users to mark campsites, trail forks, and summit landmarks directly on the screen.
4. **Real-Time Hiking Telemetry**:
   - Continuous readout of Latitude/Longitude coordinates, Barometric/GPS Altitude (meters), Trip Distance (km), and Heading Rose (degrees).
5. **Hardware Multi-Sensory Feedback**:
   - Active-low RGB LED glows Green for active track recording and SD mount, Amber during waypoint drop, and Red on hardware fault.
   - Distinct acoustic feedback clicks for tactile verification while wearing heavy gloves.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 5, 18, 19, 23** | MicroSD Card SPI | CS, SCK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — Track status |
| **GPIO 26** | Audio / Buzzer | Frequency tactile feedback tone |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/17-offline-osm-navigator
pio run -t upload
```

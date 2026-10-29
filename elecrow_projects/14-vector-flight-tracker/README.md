# ✈️ Project 14: High-Contrast Vector Flight Tracker

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **10-Foot Distant High-Contrast Display**:
   - Giant callsign typography and high-contrast flight card optimized for desk or wall viewing across the room.
   - Airline brand color highlighting (e.g. United Cyan, British Airways Blue, Lufthansa Amber, Emirates Crimson).
2. **Flight Progress & Route Vector**:
   - Displays Origin and Destination airport IATA codes (e.g. `LHR` ➔ `ORD`) and metropolitan city names.
   - Visual flight journey progress bar with dynamic aircraft waypoint position.
3. **Aviation Vector Telemetry Matrix**:
   - Real-time altitude indicator (`38K FT`) with climb / descent / level flight trend.
   - Ground speed readout (`492 KTS` / Mach 0.81).
   - Cardinal true heading readout (`285° WNW`).
4. **Interactive Touchpad Controls**:
   - Tap `< PREV` or `NEXT >` buttons to manually flip between monitored overhead aircraft.
   - Toggle `AUTO ON / PAUSED` 8-second automatic aircraft carousel mode.
5. **Multi-Color Status Indicator & Audio Alert**:
   - Active-low RGB LED glows Blue for cruising, Green for climbing, Amber for descending, and flashes Red for emergency squawk events.
   - Piezo buzzer emits audible chimes when switching aircraft or when a flight enters sector airspace.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — Flight trend |
| **GPIO 26** | Audio / Buzzer | Frequency tone alert |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/14-vector-flight-tracker
pio run -t upload
```

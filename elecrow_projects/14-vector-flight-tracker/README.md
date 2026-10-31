# ✈️ Project 14: High-Contrast Vector Flight Tracker

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 🚀 Key Features

1. **Glanceable Vector Flight Telemetry**:
   - High-contrast visual flight deck formatted for long-distance readability across a room.
   - Live route visualization with departure and destination IATA badges, cities, and animated aircraft progress bar.
2. **Kinematic Metrics Grid**:
   - Digital readouts for Altitude (thousands of feet), Ground Speed (knots / Mach), Vertical Speed trend (+CLIMB / -DESC / CRUISE), and 360° Heading Vector.
3. **Calibrated Touchpad Controls**:
   - Dedicated buttons: `< PREV`, `AUTO ON / PAUSED`, and `NEXT >` for browsing overhead aircraft.
   - Direct-tap card cycling with audio tone feedback.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 25, 39, 32** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI (Hardware Calibrated) |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone generation |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/14-vector-flight-tracker
pio run -t upload
```

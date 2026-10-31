# ✈️ Project 09: ATC Rotating Sweep Flight Radar

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 🚀 Key Features

1. **Simulated & Live ADS-B Airspace Radar**:
   - Dual-core rendering architecture running a continuous 360-degree phosphor green CRT sweep line across a calibrated radar scope.
   - Plots aircraft blips with polar coordinates (range and azimuth bearing).
2. **Tracked Target Data Card**:
   - Displays real-time callsign, aircraft type (e.g. `B738`, `A321`, `B77W`, `EC35`), altitude in feet, speed in knots, distance in nautical miles, and heading angle.
   - Highlights nearest targets and identifies emergency Squawk 7700 beacons.
3. **Interactive Touch Controls**:
   - Dynamic range selector: tap `[10NM]`, `[25NM]`, or `[50NM]` to rescale the radar scope instantly.
   - Target cycle button: tap `>> NEXT TARGET <<` to inspect individual aircraft telemetry.
   - Direct scope tap: tap any aircraft blip directly on the radar scope to lock onto it.
4. **Hardware Visual & Audio Alerts**:
   - Audible proximity chirp on GPIO 26 whenever an aircraft breaches within 5 NM.
   - Onboard active-low RGB LED pulses on each 360° sweep rotation and flashes Red on Squawk 7700 emergency.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 25, 39, 32** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI (Hardware Calibrated) |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone generation |
| **Wi-Fi** | ADS-B Telemetry | 802.11 REST Client |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/09-atc-flight-radar
pio run -t upload
```

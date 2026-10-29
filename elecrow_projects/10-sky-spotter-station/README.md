# 🔭 Project 10: Sky-Spotter Flight Deck

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Airport-Style Live Flight Board**:
   - Displays real-time airline flight numbers (e.g. `DL 482`, `UA 915`, `AA 104`, `LH 430`), origin/destination routes, and aircraft types.
   - Shows altitude in feet, ground speed in knots, and distance in nautical miles.
2. **3D Spotter Line Vector Calculation**:
   - Computes the exact compass bearing (0° to 360°) and sky elevation incline angle (0° horizon to 90° zenith overhead).
   - Graphic visual compass needle and elevation arc guide binoculars or cameras directly to passing overhead aircraft.
3. **Dual-Mode Touch Interface**:
   - Top tabs toggle between the multi-flight schedule board and dedicated single-flight 3D Spotter Vector views.
4. **Hardware Visual & Audio Feedback**:
   - Audible frequency chimes on flight selection and tab swaps.
   - Onboard active-low RGB status signaling.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone generation |
| **Wi-Fi** | Flight Telemetry | 802.11 REST Client |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/10-sky-spotter-station
pio run -t upload
```

# 🚀 Project 11: Orbital Rocket & ISS Tracker

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Global Rocket Launch Countdown Engine**:
   - Live real-time T-minus countdown clock (Days, Hours, Minutes, Seconds) ticking down to next global space missions (e.g. SpaceX Falcon 9, NASA SLS, Rocket Lab Electron).
   - Mission payload summary, launch pad coordinates, and weather go/no-go status.
2. **Real-Time ISS Orbital Tracking**:
   - Displays live International Space Station latitude, longitude, altitude (km), and orbital speed (27,580 km/h).
   - Visual trajectory tracker showing progressive orbital flight path.
3. **Backyard Visible Pass Predictor**:
   - Calculates next visible naked-eye flyover pass for your exact geographic coordinates.
   - Computes maximum elevation angle (e.g. 74° zenith) and expected optical magnitude (-3.4 brightness).
4. **Hardware Visual & Audio Alerts**:
   - Tricolor active-low RGB LED glows Cyan/Purple during orbital tracking and pulses Amber at T-minus countdown milestones.
   - Tone alerts sound on launch status changes.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone generation |
| **Wi-Fi** | Space Telemetry | 802.11 REST Client |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/11-iss-rocket-tracker
pio run -t upload
```

# 🖨️ Project 07: Bambu Lab Dual-Printer Telemetry Desk Monitor

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Dual-Printer MQTT Telemetry**:
   - Subscribes to local Bambu Lab 3D printer MQTT telemetry (supporting X1-Carbon, P1P, P1S, and A1 Mini).
   - Real-time Nozzle temperature, Heated Bed temperature, Chamber temperature, and Target values.
2. **Comprehensive Job Metrics**:
   - Graphical print progress percentage with ETA remaining time countdown.
   - Current layer tracking vs. total layer count (e.g. `Layer 216 / 300`).
   - Integrated filament consumption tracker in grams and estimated print material cost.
3. **Multi-View UI Touch Switcher**:
   - `[1] X1-CARBON`: Dedicated full-screen diagnostic card for Printer 1.
   - `[2] P1S COMBO`: Dedicated full-screen diagnostic card for Printer 2.
   - `[3] DUAL SPLIT`: Side-by-side dual-printer fleet view for monitoring two machines simultaneously.
4. **Hardware Visual & Audio Alerts**:
   - Onboard active-low RGB LED signals printer state: Red on HMS failure / filament jam, Orange while active printing, Green on completion.
   - Speaker chime plays upon job completion.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone generation |
| **Wi-Fi** | MQTT Port 1883/8883 | Bambu telemetry subscriber |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/07-bambu-telemetry-monitor
pio run -t upload
```

# 🚀 Project 11: Orbital Rocket Launch & ISS Tracker

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 🚀 Key Features

1. **Live Rocket Launch Countdown**:
   - T-minus countdown timer tracking upcoming orbital missions (SpaceX Falcon 9, Starship, Electron, Ariane 6).
   - Mission status, launch pad location, booster model, payload inventory, and weather go/no-go probability.
2. **Real-Time ISS Orbital Tracker**:
   - Computes live International Space Station coordinates (Latitude, Longitude, Altitude, Orbital Velocity).
   - Calculates next visible backyard naked-eye flyover pass with elevation angle, azimuth, and brightness magnitude.
3. **Calibrated Touch Controls**:
   - Tap top tabs `[1] ROCKET COUNTDOWN` and `[2] ISS ORBIT & PASS` to switch telemetry displays.
   - Tap Webcast / Payload buttons or ISS pass card for instant acoustic confirmation and details.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 25, 39, 32** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI (Hardware Calibrated) |
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


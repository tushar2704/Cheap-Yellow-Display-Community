# 📻 Project 13: HamClock Mini Dashboard

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 🚀 Key Features

1. **Space Weather & Geomagnetic Conditions**:
   - Real-time Solar Flux Index (SFI), Sunspot Number (SSN), A-Index, and K-Index monitoring.
   - Dynamic RGB LED color alert based on geomagnetic storm warning thresholds (Green = Quiet, Yellow = Unsettled, Red = Storm).
2. **HF Band Propagation Matrix**:
   - Live Day & Night condition indicators across 80m, 40m, 20m, 15m, and 10m amateur bands.
3. **POTA / SOTA Live DX Spots**:
   - Real-time activator list with callsign, operating frequency, mode (SSB, CW, FT8), park reference, distance in kilometers, and direct beam heading.
4. **Calibrated Touch Controls**:
   - Quick bottom tab switcher between Solar Conditions and POTA/SOTA live spots.
   - Tap any spot card for acoustic feedback and detailed bearing telemetry.

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
cd elecrow_projects/13-hamclock-mini
pio run -t upload
```

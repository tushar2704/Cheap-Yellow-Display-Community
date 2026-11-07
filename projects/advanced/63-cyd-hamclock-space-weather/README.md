# 63 — CYD HamClock Space Weather Station

> ⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

An amateur radio and astronomical **desktop Space Weather & HamClock terminal** running on the ESP32 Cheap Yellow Display. Features a live world map with real-time day/night solar terminator (greyline projection), subsolar point and auroral oval tracking, dual UTC / Local timezone clocks, Maidenhead grid locator, Solar Flux Index (SFI), Sunspot Number (SSN), geomagnetic Kp-index gauge, X-ray solar flare class, and HF band propagation indicators with hourly Morse Code chimes.

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 🌍 World Map & Solar Terminator (Greyline)
- **Real-Time Day / Night Projection**: Sinusoidal solar terminator dividing daylight and darkness across global continents.
- **Subsolar Point Tracker**: Real-time position marker for the sun and auroral oval ring across polar latitudes.
- **Continuous Orbit**: Sun longitude advances smoothly in real time.

### 🕒 Dual Timezone Clocks & Grid Locator
- **UTC / Zulu Clock**: Prominent high-contrast UTC master time display (`14:28:45 UTC`).
- **Local Time**: Synchronized local time clock (`09:28:45 EST`).
- **Solar Ephemeris**: Calculates local solar noon and solar midnight markers.
- **Maidenhead Grid**: Displays station QTH locator (e.g. `FN31pr`).

### ☀️ Space Weather & HF Propagation Gauges
1. **Solar Flux Index (SFI)**: 10.7cm solar radio emission flux (`168 sfu` - DX condition).
2. **Sunspot Number (SSN)**: Solar Cycle 25 active sunspot counter (`142 spots`).
3. **Geomagnetic Kp-Index**: Real-time planetary geomagnetic disturbance gauge (`Kp 2.3` - Quiet / Unsettled / Storm alert).
4. **X-Ray Solar Flare Flux**: Live flare classification (`C 1.4` Class C normal background).
5. **HF Band Propagation Matrix**: Multi-band ionospheric condition ratings across `10m`, `15m`, `20m`, and `40m` amateur bands.

### 💡 Multi-Color Geomagnetic Glow & Morse Audio
- **Tricolor LED Aura**: Real-time geomagnetic field reflection (Emerald Green for quiet $Kp < 3$, Yellow for unsettled $Kp = 4$, and Warning Crimson for geomagnetic storm $Kp \ge 5$).
- **Hourly Morse Code Chime**: Plays acoustic Morse Code `"DX"` (`-.. -..-`) on the top of every hour via the onboard piezo buzzer.
- **Touch Interaction**: Tap clocks to preview Morse Code audio; tap map or weather cards to force telemetry sync.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Onboard Piezo Buzzer on **GPIO 26** for Morse Code hourly chimes
- Onboard RGB LED on **GPIO 4, 16, 17** for geomagnetic status glow
- Micro-USB or USB-C power source (5V, 1A)
- Ham Radio Shack Desk Stand 3D Enclosure by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Navigate to the project directory:
   ```bash
   cd projects/advanced/63-cyd-hamclock-space-weather
   ```
2. Build and flash via PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```
3. Set on your radio shack desk and monitor global solar propagation!

---

## 📌 Pinout Reference

| Component | Pin | Function |
|---|---|---|
| TFT LCD (ILI9341) | MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21 | 320×240 Greyline Map & Space Weather HUD |
| Resistive Touch (XPT2046) | CS=33, CLK=25, MOSI=32, MISO=39, IRQ=36 | Morse Audio Toggle & Force Sync |
| RGB LED | R=4, G=16, B=17 (Active LOW) | Geomagnetic Kp Storm Status Indicator |
| Buzzer | GPIO 26 | Hourly Morse Code "DX" Chimes |

---

## 📺 Community & Enclosures

- 📺 **IamNotIntoTech YouTube**: [Explore more CYD Tutorials](https://youtube.com/@IamNotIntoTech)
- 🖨️ **Makerrdesigns 3D Files**: [Download Radio Shack Desktop Stand](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community

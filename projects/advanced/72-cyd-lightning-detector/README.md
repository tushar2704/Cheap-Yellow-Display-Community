# ⚡ Project 72 — CYD Lightning Strike & Storm Warning Radar

> **A severe thunderstorm and lightning strike early warning radar terminal for the Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch). Features a polar strike scope (5, 15, 30, 40 km rings), strike age tracking, 60-minute frequency histogram, severe thunderstorm advisory with ETA and energy readouts, low-frequency thunderclap audio simulation on GPIO 26, flash-bang white strobe RGB LED, and optical verification via GPIO 34 LDR!**

[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/)
[![Display: ILI9341](https://img.shields.io/badge/Display-ILI9341_2.8%22-yellow.svg)](https://platformio.org/)
[![Touch: XPT2046](https://img.shields.io/badge/Touch-XPT2046_Resistive-red.svg)](https://platformio.org/)
[![Brand: IamNotIntoTech](https://img.shields.io/badge/Branded-IamNotIntoTech-FEA000.svg)](https://youtube.com/@IamNotIntoTech)

---

### 📺 Watch the Masterclass & 3D Enclosure
- 🎬 **Tutorial & Code Walkthrough**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printable Weather Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🌟 Key Features

1. **4 Touch-Navigated Weather Threat Views**:
   - **View 1: Storm Radar Scope** — 40 km polar range rings with 10 km danger perimeter, cardinal compass bearings (N, E, S, W), and color-coded strikes (Red = <30s fresh strike, Yellow = mid-age). Displays approaching front velocity and distance.
   - **View 2: Activity Histogram** — Rolling 60-minute bar chart of strike frequency per interval with current strike velocity gauge (strikes / minute).
   - **View 3: Severe Weather Advisory** — Immediate safety warnings, estimated time to arrival (ETA), peak kilo-ampere (kA) discharge energy, and indoor shelter directives.
   - **View 4: Sensor Diagnostics** — RF noise floor threshold, spike rejection levels, and interactive `[SIMULATE TEST STRIKE]` touch button.
2. **Audio & Lightning Simulation**:
   - Low-frequency thunderclap acoustic rumble on piezo buzzer `GPIO 26`.
   - Proximity warning sirens when strikes occur within the 10 km radius.
3. **Hardware Strobe & Optical Verification**:
   - Bright white flash-bang strobe on active-low RGB LED (`GPIO 4, 16, 17`) upon strike detection.
   - LDR light sensor on `GPIO 34` acts as an optical verification photodiode channel.

---

## 🔌 Hardware Pinout (ESP32-2432S028R)

| Component | Pin Function | ESP32 GPIO |
|---|---|---|
| **ILI9341 Display** | MOSI / MISO / SCLK / CS / DC | `GPIO 13` / `GPIO 12` / `GPIO 14` / `GPIO 15` / `GPIO 2` |
| **Backlight PWM** | TFT Backlight Enable | `GPIO 21` (Active HIGH) |
| **XPT2046 Touch** | CLK / MISO / MOSI / CS / IRQ | `GPIO 25` / `GPIO 39` / `GPIO 32` / `GPIO 33` / `GPIO 36` |
| **Lightning Strobe LED** | Red / Green / Blue | `GPIO 4` / `GPIO 16` / `GPIO 17` (Active LOW) |
| **Thunder Speaker** | Acoustic Thunderclaps | `GPIO 26` |
| **Optical LDR** | Flash Verification ADC | `GPIO 34` |

---

## 🛠️ How to Compile & Flash

### PlatformIO (Recommended)
```bash
cd projects/advanced/72-cyd-lightning-detector
pio run --target upload
```

### Arduino IDE
1. Open `72-cyd-lightning-detector.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and configure `User_Setup.h` for ILI9341.
4. Install **XPT2046_Touchscreen**.
5. Click **Upload**.

---

<p align="center">
  <b>Built with 🟡 by <a href="https://youtube.com/@IamNotIntoTech">IamNotIntoTech</a></b> &bull; 
  <b>3D Enclosures by <a href="https://youtube.com/@Makerrdesigns">Makerrdesigns</a></b>
</p>


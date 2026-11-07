# 🏎️ Project 66 — CYD F1 Pit Wall Telemetry Dashboard

> **A professional Formula 1 race strategy and telemetry command console for the Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch). Features real-time driver gap leaderboards, tire degradation matrices, 4-corner thermal heatmaps, circuit weather radar, and interactive FIA Race Control flag management with synchronized audio sirens and RGB warning strobes!**

[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/)
[![Display: ILI9341](https://img.shields.io/badge/Display-ILI9341_2.8%22-yellow.svg)](https://platformio.org/)
[![Touch: XPT2046](https://img.shields.io/badge/Touch-XPT2046_Resistive-red.svg)](https://platformio.org/)
[![Brand: IamNotIntoTech](https://img.shields.io/badge/Branded-IamNotIntoTech-FEA000.svg)](https://youtube.com/@IamNotIntoTech)

---

### 📺 Watch the Masterclass & 3D Enclosure
- 🎬 **Tutorial & Code Walkthrough**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printable Desktop Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🌟 Key Features

1. **4 Touch-Navigated Pit Wall Views**:
   - **View 1: Timing Leaderboard** — Top 6 driver gaps (VER, NOR, LEC, HAM, PIA, RUS), tire compound badges (`[S]`, `[M]`, `[H]`), sector 1/2/3 speed blocks, and purple fastest lap highlights.
   - **View 2: Tire Strategy & Degradation Matrix** — 4-corner tire thermal heatmap (°C) on chassis silhouette (FL, FR, RL, RR), stint lap age counter (18/32 laps), and live pit strategy delta banner (`BOX THIS LAP: DELTA -22.4s TO P4`).
   - **View 3: Weather Radar & Track Telemetry** — Track temperature (38.4°C), ambient air temp (24.1°C), wind velocity (14 km/h), and precipitation countdown gauge (`RAIN IN 12 MIN`).
   - **View 4: FIA Race Control Simulator** — Interactive touchscreen triggers for Green Flag, Virtual Safety Car (VSC), Full Safety Car (SC), and Red Flag.
2. **Synchronized Hardware Warning Alerts**:
   - Onboard active-low RGB LED projects live race status: Green (Track Clear), Yellow Pulsing (VSC), Orange Flashing (Safety Car), Crimson Strobe (Red Flag).
   - Piezo buzzer on `GPIO 26` synthesizes acoustic team radio notification chimes and two-tone emergency track sirens.
3. **Dynamic Stint Simulation**:
   - Real-time thermal drift and wear degradation calculations running in the background.
4. **LDR Auto-Brightness Sensor**:
   - Reads ambient room lighting on `GPIO 34` for automatic contrast control.

---

## 🔌 Hardware Pinout (ESP32-2432S028R)

| Component | Pin Function | ESP32 GPIO |
|---|---|---|
| **ILI9341 Display** | MOSI / MISO / SCLK / CS / DC | `GPIO 13` / `GPIO 12` / `GPIO 14` / `GPIO 15` / `GPIO 2` |
| **Backlight PWM** | TFT Backlight Enable | `GPIO 21` (Active HIGH) |
| **XPT2046 Touch** | CLK / MISO / MOSI / CS / IRQ | `GPIO 25` / `GPIO 39` / `GPIO 32` / `GPIO 33` / `GPIO 36` |
| **Race Flag RGB LED** | Red / Green / Blue | `GPIO 4` / `GPIO 16` / `GPIO 17` (Active LOW) |
| **Pit Radio Speaker** | Audio Chimes & Siren | `GPIO 26` |
| **LDR Sensor** | Ambient Light ADC | `GPIO 34` |

---

## 🛠️ How to Compile & Flash

### PlatformIO (Recommended)
```bash
cd projects/advanced/66-cyd-f1-pitwall-dashboard
pio run --target upload
```

### Arduino IDE
1. Open `66-cyd-f1-pitwall-dashboard.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and configure `User_Setup.h` for ILI9341.
4. Install **XPT2046_Touchscreen**.
5. Click **Upload**.

---

<p align="center">
  <b>Built with 🟡 by <a href="https://youtube.com/@IamNotIntoTech">IamNotIntoTech</a></b> &bull; 
  <b>3D Enclosures by <a href="https://youtube.com/@Makerrdesigns">Makerrdesigns</a></b>
</p>


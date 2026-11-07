# ✈️ Project 67 — CYD ADS-B Flight Radar & Aircraft Tracker

> **A real-time aviation virtual radar scope and ADS-B mode-S flight tracking terminal for the Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch). Features a 360-degree rotating phosphor radar sweep, concentric 10/25/50 NM range rings, multi-aircraft target velocity vectors, interactive touch flight telemetry cards, transponder emergency alerts (Squawk 7700), acoustic radar pings, and synchronized alert strobes!**

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

1. **360° Real-Time Rotating Radar Scope**:
   - High-contrast green phosphor radar screen with trigonometric beam sweeping at 30+ FPS.
   - 10 NM, 25 NM, and 50 NM range rings with cardinal compass bearings (N, E, S, W) and center station origin.
2. **Multi-Aircraft Vector Tracking**:
   - Renders live aircraft targets positioned by range and relative bearing.
   - Directional heading vectors indicate ground flight path and airspeed.
3. **Interactive Touch Avionics Telemetry Card**:
   - Tap any aircraft blip on the radar screen or use the `[NEXT TARGET >>]` button to inspect live flight statistics:
     - **Callsign**: Airline & flight identifier (e.g. `UAL842`, `BAW117`, `DLH400`, `AFR066`, `QFA009`, `EMR202`).
     - **Aircraft Model**: Heavy & wide-body airframes (B789, A359, B748, A388, B77W).
     - **Origin & Destination**: Flight routes (e.g. `LHR -> JFK`, `DXB -> LAX`).
     - **Altitude & Ground Speed**: Flight level in feet (e.g. `34,000 FT`) and speed in knots (`482 KTS`).
     - **Transponder Squawk Code**: Normal VFR (`1200`) and emergency code (`7700`).
4. **Emergency Transponder Detection (Squawk 7700)**:
   - Built-in emergency detector triggers two-tone rising frequency sirens on `GPIO 26` and flashes crimson LEDs on `GPIO 4`.
5. **Acoustic Radar Feedback**:
   - Synthesizes 1800 Hz sweep ping chimes on every full 360° radar revolution.
6. **Hardware LDR Sensor**:
   - Reads ambient illumination on `GPIO 34` for automatic day/night radar contrast.

---

## 🔌 Hardware Pinout (ESP32-2432S028R)

| Component | Pin Function | ESP32 GPIO |
|---|---|---|
| **ILI9341 Display** | MOSI / MISO / SCLK / CS / DC | `GPIO 13` / `GPIO 12` / `GPIO 14` / `GPIO 15` / `GPIO 2` |
| **Backlight PWM** | TFT Backlight Enable | `GPIO 21` (Active HIGH) |
| **XPT2046 Touch** | CLK / MISO / MOSI / CS / IRQ | `GPIO 25` / `GPIO 39` / `GPIO 32` / `GPIO 33` / `GPIO 36` |
| **Status RGB LED** | Red / Green / Blue | `GPIO 4` / `GPIO 16` / `GPIO 17` (Active LOW) |
| **Radar Speaker** | Sweep Pings & Sirens | `GPIO 26` |
| **LDR Sensor** | Ambient Light ADC | `GPIO 34` |

---

## 🛠️ How to Compile & Flash

### PlatformIO (Recommended)
```bash
cd projects/advanced/67-cyd-adsb-flight-radar
pio run --target upload
```

### Arduino IDE
1. Open `67-cyd-adsb-flight-radar.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and configure `User_Setup.h` for ILI9341.
4. Install **XPT2046_Touchscreen**.
5. Click **Upload**.

---

<p align="center">
  <b>Built with 🟡 by <a href="https://youtube.com/@IamNotIntoTech">IamNotIntoTech</a></b> &bull; 
  <b>3D Enclosures by <a href="https://youtube.com/@Makerrdesigns">Makerrdesigns</a></b>
</p>


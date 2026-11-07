# 60 — CYD SimHub Racing Cockpit Telemetry HUD

> ⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A high-visibility, professional **racing simulator cockpit telemetry display & shift-light indicator** for SimHub (Assetto Corsa, iRacing, F1 23/24, Forza Motorsport, Le Mans Ultimate). Features a 16-LED sequential rev-limiter shift light bar across the top with redline blue strobe, giant current gear box (`1`–`8`, `N`, `R`), digital speedometer, real-time delta lap time calculator, 4-corner tire temperature thermal heatmap, fuel remaining, and ERS/DRS battery status.

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 🏎️ Race Cockpit Instrumentation
- **16-LED Sequential Shift Bar**:
  - Green (Stage 1) -> Yellow (Stage 2) -> Red (Stage 3) -> Flashing Strobe Blue on redline shift point (`>12,600 RPM`).
  - Hardware shift strobe on onboard tricolor RGB LED + high-pitched shift chirp audio.
- **Giant Gear Display**: Ultra-bold centered gear indicator with dynamic RPM readout and DRS activation state banner.
- **Speedometer**: Digital speed gauge with instant toggle between `KM/H` and `MPH`.
- **Live Delta Lap Timer**: Real-time delta against personal best lap (`-0.342s` in green when faster, `+0.184s` in crimson when slower).
- **4-Corner Thermal Tire Heatmap**: Real-time tire carcass temperatures for `FL`, `FR`, `RL`, and `RR` with color-coded thermal states (Cold Blue, Optimum Green, Hot Orange, Overheating Red).
- **Energy & Fuel**: Fuel load in liters with remaining lap estimate, and ERS hybrid battery deployment bar.

### 🎛️ Touch Interactions
- **Tap Gear Box**: Manual gear cycle override.
- **Tap Speedometer**: Toggle between KM/H and MPH units.
- **Tap Footer Bar**: Toggle redline shift buzzer audio on/off.

### 🌐 Standalone Simulation & SimHub Ready
- Includes built-in dynamic race lap simulation with realistic engine rev curves, downshift braking, tire heat progression, and lap delta drift.
- Ready for live PC SimHub telemetry streaming via UDP broadcast or USB serial bridge.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Onboard Piezo Buzzer on **GPIO 26** for shift point acoustic beeps
- Onboard RGB LED on **GPIO 4, 16, 17** for shift strobe
- Micro-USB or USB-C power source (5V, 1A)
- Sim-Racing Steering Wheel / Cockpit Mount 3D Enclosure by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Navigate to the project directory:
   ```bash
   cd projects/advanced/60-cyd-simhub-racing-telemetry
   ```
2. Build and flash via PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```
3. Mount to your racing wheel rig and start hot-lapping!

---

## 📌 Pinout Reference

| Component | Pin | Function |
|---|---|---|
| TFT LCD (ILI9341) | MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21 | 320×240 High-Contrast Telemetry Screen |
| Resistive Touch (XPT2046) | CS=33, CLK=25, MOSI=32, MISO=39, IRQ=36 | Speed Unit & Audio Toggle |
| RGB LED | R=4, G=16, B=17 (Active LOW) | High-RPM Shift Point Strobe |
| Buzzer | GPIO 26 | Redline Acoustic Shift Beep |

---

## 📺 Community & Enclosures

- 📺 **IamNotIntoTech YouTube**: [Explore more CYD Tutorials](https://youtube.com/@IamNotIntoTech)
- 🖨️ **Makerrdesigns 3D Files**: [Download Sim Racing Mounts](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community

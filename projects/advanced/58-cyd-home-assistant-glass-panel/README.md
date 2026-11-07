# 58 — CYD Home Assistant Quick-Tiles Glass Panel

> ⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A contemporary, high-performance **wall-mount glassmorphic smart home control panel** running on the ESP32 Cheap Yellow Display. Features 4 interactive navigation tabs (`HOME`, `LIGHTS`, `CLIMATE`, `ENERGY`), a circular thermostat dial with target setpoint adjustment, 2×2 lighting dimmer tiles, home security mode switcher, and solar / grid / battery energy telemetry.

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 📱 4 Glassmorphic Dashboard Views
1. **Tab 0 — HOME Overview**:
   - Thermostat quick dial showing current temperature, target setpoint, and quick `+0.5` / `-0.5` adjustment buttons.
   - Home Security Alarm card: One-touch toggle between `ARMED HOME`, `ARMED AWAY`, and `DISARMED`.
   - 4 Instant Scene buttons: `Relax`, `Movie`, `Bright`, and `Off`.
2. **Tab 1 — LIGHTS Quick-Tiles (2×2 Grid)**:
   - 4 independent smart lighting tiles (`Living Room`, `Desk Strip`, `Kitchen Pod`, `Porch Light`).
   - One-tap toggle for on/off state with instant color-coded visual indicator and brightness percentage bar.
3. **Tab 2 — CLIMATE Master Thermostat**:
   - Large central circular thermostat dial with dual arc temperature indicators.
   - Large touch setpoint increment/decrement buttons.
   - 4 HVAC Mode Selector buttons: `HEAT`, `COOL`, `AUTO`, `ECO`.
4. **Tab 3 — ENERGY & Solar Dashboard**:
   - Real-time rooftop solar generation meter (kW) with peak tracking.
   - Whole-home power consumption and net grid export monitor.
   - Home battery storage state-of-charge (SOC %) gauge and charging status.

### 💡 Multi-Color Status Aura & Audio Haptics
- **Tricolor LED Aura**: Real-time hardware status reflection (Warm Amber when heating, Cool Blue when air conditioning, Green when secure, Red alert when armed away).
- **Acoustic Haptics**: Subtle piezo buzzer click tones on every button tap.

### 🌐 Home Assistant Ready & Standalone Demo
- Works immediately out of the box with high-precision procedural simulation and realistic room temperature fluctuations.
- Structured with JSON entity state models (`ArduinoJson`) ready for local WiFi Home Assistant REST / WebSocket integration.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Onboard Piezo Buzzer on **GPIO 26** for touch click haptics
- Onboard RGB LED on **GPIO 4, 16, 17**
- Micro-USB or USB-C power source (5V, 1A)
- Wall-Mount Glass Panel 3D Enclosure by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Navigate to the project directory:
   ```bash
   cd projects/advanced/58-cyd-home-assistant-glass-panel
   ```
2. Build and flash via PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```
3. Use the bottom navigation bar to switch between Home, Lights, Climate, and Energy dashboards!

---

## 📌 Pinout Reference

| Component | Pin | Function |
|---|---|---|
| TFT LCD (ILI9341) | MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21 | 320×240 Glassmorphic Smart Display |
| Resistive Touch (XPT2046) | CS=33, CLK=25, MOSI=32, MISO=39, IRQ=36 | Glass Tiles, Tabs, Thermostat Buttons |
| RGB LED | R=4, G=16, B=17 (Active LOW) | Climate & Alarm Security State Glow |
| Buzzer | GPIO 26 | Physical Touch Haptic Clicks |

---

## 📺 Community & Enclosures

- 📺 **IamNotIntoTech YouTube**: [Explore more CYD Tutorials](https://youtube.com/@IamNotIntoTech)
- 🖨️ **Makerrdesigns 3D Files**: [Download Wall Mount Smart Panel Enclosure](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community

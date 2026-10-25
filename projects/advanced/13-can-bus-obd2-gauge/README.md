# 🏎️ Project 13 — CYD 2.8" Automotive CAN Bus OBD-II Live Digital Gauge Cluster

> Transform your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a high-performance sports car digital instrument cluster and OBD-II engine monitor. Displays an analog needle tachometer (0-8000 RPM) with 6500+ RPM redline shift alerts, oversized digital speedometer (MPH / KM/H touch-toggle), coolant temperature (°C), alternator battery voltage (14.2V), throttle percentage, and DTC trouble code status. Pairs with the [3D Desktop Wedge Stand](../../../enclosures/cyd-2.8-desktop-stand/) by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐⭐ Advanced (Level 4/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch CYD)
- **Bus Interface**: CAN Bus (ISO 15765-4, 500kbps) / OBD-II PID polling
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the CAN Bus OBD-II Live Vehicle Test**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Dashboard Gauge Pod Mount**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Circular Analog Tachometer**: 0 to 8000 RPM gauge with high-contrast colored needle, tick graduation, and glowing redline limit zone.
- **Digital Speedometer**: Glanceable numerical display with instant 1-touch unit conversion between **MPH** and **KM/H**.
- **Auxiliary Telemetry**: Real-time readouts for Engine Coolant Temperature, Alternator Voltage, and Throttle Position (TPS %).
- **Shift Light Indicator (RGB LED)**:
  - 🟢 **Green**: Low idle / cruise.
  - 🔵 **Cyan**: Mid-band power delivery.
  - 🟡 **Yellow**: Approaching shift point (> 5000 RPM).
  - 🔴 **Flashing Red**: Redline limit (> 6500 RPM) with audio rev warning buzzer.
- **Acoustic Redline Alarm**: Audible tone alerts via onboard piezo (GPIO 26).
- **Interactive Touch Revving**: Tap the speedometer to toggle speed units, or tap the tachometer dial to simulate throttle burst.

---

## 🚀 How to Flash & Run

### 1. 1-Click Flash in Your Browser (No Coding Required)
Open the [CYD Community Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/) in Chrome, Edge, or Opera, plug in your CYD 2.8" via USB, and click **Install CAN Bus OBD-II Gauge**.

### 2. Or Build & Flash via PlatformIO:
```bash
cd projects/advanced/13-can-bus-obd2-gauge
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)

#  Project 03 — CYD 2.8" Apple HomeKit Touch Terminal & Controller

> Turn your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a dedicated Apple HomeKit touch terminal. Features sleek Cupertino-inspired dark mode UI, interactive accessory controls (Ceiling Lights, Desk Lamp, Exhaust Fan, Smart Outlet), live room climate monitoring, onboard RGB LED ambient status, audio feedback, and HomeKit pairing setup card. Pairs seamlessly with the [3D Desktop Stand](../../../enclosures/cyd-2.8-desktop-stand/) by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐⭐ Advanced (Level 4/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch CYD)
- **HomeKit Setup Code**: `466-37-726`
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`, `WiFi`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the Full Build & Firmware Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Desktop Enclosure & Stand**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Cupertino Dark Theme UI**: High-contrast modern tiles with custom glyph badges and state glow.
- **4 Interactive Accessory Tiles**:
  - 💡 **Ceiling Light**: Amber gold active glow, toggleable with touch feedback.
  - 🛋️ **Desk Lamp**: Cyan luminescence, live state readouts.
  - 🌀 **Exhaust Fan**: Emerald breeze mode with PWM fan simulation.
  - 🔌 **Smart Outlet**: High-visibility orange power state.
- **HAP Climate & Sensor Panel**: Real-time room temperature and humidity monitoring with touch-triggered sync.
- **Onboard RGB LED Integration**: Active color mapping based on smart home state (Yellow for primary lighting, Cyan for climate ventilation, Green when synced).
- **Tactile Audio Feedback**: Dedicated piezo feedback on screen presses via onboard speaker (GPIO 26).
- **100% Zero Soldering Required**: All features utilize onboard CYD hardware.

---

## 🚀 How to Flash & Configure

### 1. Flash Directly in Your Browser (No Coding Required)
Launch the [CYD Community Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/) in Chrome, Edge, or Opera, plug in your CYD 2.8" via USB, and click **Install Apple HomeKit Terminal**.

### 2. Or Build & Flash via PlatformIO:
```bash
cd projects/advanced/03-homekit-terminal
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)

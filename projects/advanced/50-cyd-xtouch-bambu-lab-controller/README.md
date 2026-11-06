# 50 — CYD X-Touch Bambu Lab 3D Printer Controller

> ⭐⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A standalone wireless touchscreen controller and live telemetry heads-up display for **Bambu Lab 3D printers** (P1P, P1S, X1C, A1, A1 Mini). Connects directly to the printer's onboard local MQTT broker over WiFi with real-time temperature monitoring, layer tracking, AMS 4-slot filament bay controls, and interactive touch navigation.

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 📊 Screen 0 — Main Print HUD
- **Real-Time Hotend & Bed Gauges**: Live current and target temperature readouts with color-coded thermal indicators.
- **Progress Gauge & Countdown**: Animated print progress bar with percentage, remaining hours/minutes, and current layer vs. total layer count.
- **Speed Profiles**: Touch button to cycle between `SILENT (50%)`, `STANDARD (100%)`, `SPORT (124%)`, and `LUDICROUS (166%)`.
- **Direct Controls**: One-touch Pause and Resume buttons with status badge.

### 🎨 Screen 1 — AMS Filament Bay (4 Spools)
- Displays all 4 AMS filament slots with custom color swatches (e.g. PLA Black, PETG White, Silk Cyan, TPU Red).
- Material type tags (`PLA Basic`, `PETG HF`, `TPU 95A`).
- Visual remaining filament capacity gauges per slot.
- Active feeding extruder highlighted with a glowing pulse.

### 🎛️ Screen 2 — Chamber & Hardware Controls
- Chamber internal temperature telemetry.
- Chamber LED light toggle.
- Part cooling fan and auxiliary fan speed percentages.
- `IamNotIntoTech` branding watermark.

### 💡 RGB LED & Acoustic Feedback
- **Cyan Breathe**: Printing normally.
- **Green Solid**: Print completed successfully.
- **Amber / Red Blink**: Print paused or thermal warning.
- **Acoustic Touch**: Haptic click chimes on every touch event.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Micro-USB or USB-C power cable
- 3D-printed desk stand or printer-mounted enclosure by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Open `src/main.cpp` and enter your WiFi credentials:
   ```cpp
   const char* WIFI_SSID     = "YOUR_WIFI_SSID";
   const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
   ```
2. Enter your Bambu Lab printer local credentials (found on printer screen under `Settings > Network`):
   ```cpp
   const char* PRINTER_IP          = "192.168.1.150";
   const char* PRINTER_SERIAL      = "01P00A123456789";
   const char* PRINTER_ACCESS_CODE = "12345678";
   ```
   *(If left unchanged or printer is offline, the firmware automatically launches into **Simulation Demo Mode** with live animated data!)*
3. Compile and flash using PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```

---

## 📋 Pinout Reference

| Function | Pin | Notes |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | HSPI bus dedicated to XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Active LOW logic |
| Buzzer | GPIO 26 | Haptic click & chimes |

---

## 📺 Credits

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community

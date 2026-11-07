# 51 — CYD Cydintosh 1984 Retro Mac Emulator

> ⭐⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A faithful recreation of the iconic **1984 Macintosh GUI and desktop experience** running directly on the ESP32 Cheap Yellow Display. Features an authentic 1-bit monochrome dithered interface, classic Motorola 68000 desktop environment, working MacPaint canvas, interactive four-function Calculator, and the legendary 1984 F-Major ascending startup chime.

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 🖥️ Authentic System 6 Desktop Environment
- **Classic 50% Checkerboard Dither**: Pixel-accurate Macintosh desktop background stipple.
- **Top System Menu Bar**: ` File  Edit  View  Special  IamNotIntoTech` with active pull-down logic.
- **Desktop Icons**:
  - `Macintosh HD` (opens System Information)
  - `MacPaint` (opens touch drawing studio)
  - `Calculator` (opens working 1984 calculator)
  - `Trash` (classic corrugated metal can with sound effect)
- **Classic Windowing System**:
  - Striped horizontal titlebars
  - Functional square close box (top-left)
  - Drop shadow window frames

### 🎨 Integrated MacPaint Drawing Studio
- Interactive 1-bit drawing canvas (`240×150` pixels) responding smoothly to fingers or styluses.
- **Tools**:
  - **Pencil**: Precise single-pixel vector sketching with continuous stroke interpolation.
  - **Brush**: Soft round 3px ink strokes.
  - **Eraser**: Precision canvas eraser.
  - **Clear**: Instantly wipes the canvas.

### 🔢 1984 Desk Calculator
- Fully working four-function desk calculator with recessed LCD-style display.
- Tactile beveled buttons (`0-9`, `+`, `-`, `*`, `/`, `=`, `C`).
- Audio click chime on every keypress.

### 🎵 Iconic Hardware & Audio
- **1984 Startup Chime**: Multi-frequency major chord arpeggio (`F4 → A4 → C5 → F5`) played on boot via the onboard buzzer (GPIO 26).
- **Beige Power LED**: Warm amber glow matching the original vintage 1984 Macintosh case.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Micro-USB or USB-C power cable
- 3D-printed miniature 1984 Macintosh enclosure by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Clone the repository:
   ```bash
   git clone https://github.com/tushar2704/Cheap-Yellow-Display-Community.git
   cd Cheap-Yellow-Display-Community/projects/advanced/51-cydintosh-1984-retro-mac
   ```
2. Build and upload with PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```
3. Power on and listen for the classic startup chime!

---

## 📋 Pinout Reference

| Function | Pin | Notes |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | HSPI bus dedicated to XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Warm amber vintage power glow |
| Buzzer | GPIO 26 | Startup chime & tactile clicks |

---

## 📺 Credits

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community


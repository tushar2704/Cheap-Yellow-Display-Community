# 🎨 Project 03 — CYD Touch Paint (Stylus Drawing Pad)

> Turn your Cheap Yellow Display (CYD) into an interactive touchscreen drawing canvas with an 8-color palette, smooth stroke interpolation, a canvas wipe button, and synchronized RGB LED lighting.

---

## 📋 Overview

- **Difficulty**: ⭐ Beginner (Level 1/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **External Components Required**: **None** (use your finger or any plastic stylus!)
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`

---

## 📺 Video Tutorials

- 🎬 **Watch the Build & Code Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the CYD Snap-on Stylus Holder & Case**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **8-Color Swatch Palette**: Red, Green, Blue, Yellow, Cyan, Magenta, White, and Black (Eraser).
- **RGB LED Sync**: The onboard RGB LED lights up in the exact color of the currently selected brush tool!
- **Smooth Line Interpolation**: Connects consecutive touch readings via Bresenham line drawing so fast sketching doesn't leave broken gaps.
- **Audio Sound Effects**: Audio chimes when selecting swatches, wiping the canvas, or booting.
- **Instant Clear Button**: Dedicated tactile button in the toolbar to clear the entire canvas.

---

## 🚀 How to Flash & Run

### PlatformIO:
```bash
cd projects/beginner/03-touch-paint
pio run -t upload
```

### Arduino IDE:
1. Ensure `TFT_eSPI` is configured for your CYD variant.
2. Select **ESP32 Dev Module**.
3. Upload `03-touch-paint.ino`.

---

## 📜 License
MIT License — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)

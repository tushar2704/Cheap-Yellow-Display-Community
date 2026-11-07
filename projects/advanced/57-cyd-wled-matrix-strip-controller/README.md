# 57 — CYD WLED Matrix & Lightstrip Network Controller

> ⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A dedicated **touchscreen remote and master controller** for WLED smart addressable lighting (NeoPixel WS2812B strips, rings, and LED matrices) on the ESP32 Cheap Yellow Display. Features a live 16×5 virtual LED matrix preview canvas, full-spectrum color picker with swatches, brightness / speed / intensity sliders, effect presets, and synchronized onboard RGB LED hardware mirroring.

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 🌈 Virtual LED Canvas & Real-Time Preview
- **Live 16×5 LED Matrix Canvas**: Simulates real-time lighting animations (Rainbow, Fire, Cyberpunk, Aurora, Scanner, Solid) directly on the CYD screen at 30 FPS.
- **Hardware Mirroring**: The onboard tricolor RGB LED mirrors the center matrix LED color in real-time.
- **Master Power Control**: Smooth toggle between live illumination and dark standby mode.

### 🎨 Color Palette & Continuous Spectrum
- **8 Instant Swatches**: Crimson Red, Amber Orange, Neon Yellow, Matrix Green, Cyan, Electric Blue, Hot Pink, and Warm White.
- **Continuous Hue Spectrum Bar**: Slide across the gradient bar to select any exact RGB tone with live HEX code readout (e.g. `#00FFC8`).

### 🎛️ Precision Lighting Sliders
- **Brightness**: 0% to 100% master intensity scaling.
- **Speed**: Adjusts animation wave cycle velocity.
- **Intensity**: Modulates pattern density and particle diffusion.

### ✨ 6 Effect Presets
1. `Solid`: Uniform ambient wash with chosen color.
2. `Rainbow`: Dynamic HSV traveling gradient waves.
3. `Fire`: Realistic stochastic flickering flame simulation.
4. `Cyber`: Dual-color cyan and neon magenta chasing pulses.
5. `Aurora`: Ethereal green-blue oscillating curtain.
6. `Scanner`: Retro bouncing laser beam with phosphor trail.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Onboard Piezo Buzzer on **GPIO 26** for touch haptics
- Onboard RGB LED on **GPIO 4, 16, 17**
- Micro-USB or USB-C power cable (5V, 1A)
- Desktop Console 3D Enclosure by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Navigate to the project directory:
   ```bash
   cd projects/advanced/57-cyd-wled-matrix-strip-controller
   ```
2. Build and flash via PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```
3. Tap swatches or drag the spectrum slider to choose colors, adjust speed and brightness, or switch effects!

---

## 📌 Pinout Reference

| Component | Pin | Function |
|---|---|---|
| TFT LCD (ILI9341) | MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21 | 320×240 WLED Control Panel |
| Resistive Touch (XPT2046) | CS=33, CLK=25, MOSI=32, MISO=39, IRQ=36 | Sliders, Swatches, Effect Buttons |
| RGB LED | R=4, G=16, B=17 (Active LOW) | Synchronized Live Lighting Mirror |
| Buzzer | GPIO 26 | Haptic Feedback Audio Clicks |

---

## 📺 Community & Enclosures

- 📺 **IamNotIntoTech YouTube**: [Explore more CYD Tutorials](https://youtube.com/@IamNotIntoTech)
- 🖨️ **Makerrdesigns 3D Files**: [Download WLED Desk Console Enclosure](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community


# 55 — CYD PeanutGB Classic Game Boy Handheld

> ⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A nostalgic, standalone **Classic Game Boy Handheld experience** on the ESP32 Cheap Yellow Display. Features a native 160×144 LCD centered inside a vintage handheld shell, on-screen touch D-Pad and A/B action buttons, the iconic scrolling boot sequence with dual-tone startup chime, 4 authentic color palettes, and 3 built-in playable retro games.

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 🕹️ Authentic Handheld Aesthetic
- **Classic Game Boy Boot Sequence**:
  - Trademark scrolling `IamNotIntoTech` logo descending into the LCD screen.
  - Pixel-perfect `(R)` registration badge pop.
  - Iconic ascending two-tone startup chime (`1046 Hz` -> `2093 Hz`) on **GPIO 26**.
- **Handheld Shell Interface**:
  - Left-hand tactile 4-way D-Pad (Up, Down, Left, Right).
  - Right-hand classic angled burgundy A and B buttons.
  - Slanted rubber Select & Start buttons.
  - Red power / battery LED indicator and classic dual accent stripes.

### 🎨 4 Retro Color Palettes
Switchable on the fly with the on-screen `PALETTE` button:
1. **DMG-01 Classic**: The original 1989 monochrome olive/pea-soup green LCD matrix.
2. **Pocket Silver**: High-contrast crisp black & white monochrome LCD.
3. **Cyberpunk Neon**: High-energy electric cyan, deep navy, and hot magenta.
4. **Solar Amber**: Warm vintage amber phosphor CRT handheld glow.

### 🎮 3 Built-in Playable Retro Games
- **Game 1 — Space Brick Breaker**:
  - Full paddle deflection physics, ball speed dynamics, and multi-tier brick field.
  - Real-time score counter and 3-life heart tracker with audio bounces.
- **Game 2 — Pixel Runner / Flappy Cyber**:
  - Gravity jump impulse physics via 'A' button or tap.
  - Procedural obstacle pipes, gap clearance detection, and local high score tracking.
- **Game 3 — Chiptune PSG Synthesizer**:
  - 8-key interactive retro keyboard (C4 through C5).
  - Square wave sound synthesis with real-time waveform visualization.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Onboard Piezo Buzzer on **GPIO 26**
- Onboard RGB LED on **GPIO 4, 16, 17**
- Micro-USB or USB-C power cable (5V, 1A)
- Game Boy Classic 3D Handheld Shell by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Navigate to the project directory:
   ```bash
   cd projects/advanced/55-cyd-peanutgb-gameboy-emulator
   ```
2. Build and flash via PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```
3. Use the on-screen D-Pad and A/B buttons to play, or tap `PALETTE` and `GAME` to switch modes!

---

## 📌 Pinout Reference

| Component | Pin | Function |
|---|---|---|
| TFT LCD (ILI9341) | MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21 | 320×240 Display (160×144 Active GB LCD) |
| Resistive Touch (XPT2046) | CS=33, CLK=25, MOSI=32, MISO=39, IRQ=36 | D-Pad, A/B Buttons, Select/Start |
| RGB LED | R=4, G=16, B=17 (Active LOW) | Battery / Power Indicator LED |
| Buzzer | GPIO 26 | Authentic Startup Chime & Game Sound Effects |

---

## 📺 Community & Enclosures

- 📺 **IamNotIntoTech YouTube**: [Explore more CYD Tutorials](https://youtube.com/@IamNotIntoTech)
- 🖨️ **Makerrdesigns 3D Files**: [Download Handheld 3D Case](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community

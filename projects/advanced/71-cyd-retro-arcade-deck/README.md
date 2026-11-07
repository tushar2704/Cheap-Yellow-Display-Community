# 🕹️ Project 71 — CYD 8-Bit Retro Mini-Arcade Deck

> **A standalone retro 8-bit multi-game arcade handheld console for the Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch). Features 3 fully playable arcade classics (*Cyber Snake*, *Paddle Pong*, *Neon Brick Breaker*), on-screen tactile D-Pad and direct touch dragging, 8-bit chiptune sound effect synthesizer on GPIO 26, scoring RGB LED alerts, and LDR ambient dimming!**

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

1. **3 Iconic Playable Retro Games**:
   - **Game 1: Cyber Snake 2088** — Directional grid navigation, apple spawning, growing snake tail, wall and self-collision physics, with touch D-Pad controls.
   - **Game 2: Paddle Pong (VS CPU)** — 60 FPS 2D physics table tennis against an intelligent AI paddle, dynamic deflection angles, and rally counter.
   - **Game 3: Neon Brick Breaker** — Breakout arcade classic with color-coded brick grid (Red, Gold, Green), ball bounce physics, and paddle dragging.
2. **Arcade Touch Interface**:
   - Main menu game carousel selector card view.
   - On-screen touch D-Pad (`^`, `v`, `<`, `>`) for Cyber Snake with haptic click tones.
   - Direct touch paddle dragging along Y-axis for Pong and X-axis for Breakout.
   - Dedicated `[< MENU]` return button to switch cartridges anytime.
3. **8-Bit Arcade Synthesizer**:
   - Synthesizes jump bleeps, paddle clinks, score point chimes, and game over sounds on onboard piezo `GPIO 26`.
4. **Scoring RGB LED Alerts**:
   - Green strobe on points scored.
   - Red flash on ball drop or snake wall crash.
5. **LDR Sensor**:
   - Reads ambient room lighting on `GPIO 34`.

---

## 🔌 Hardware Pinout (ESP32-2432S028R)

| Component | Pin Function | ESP32 GPIO |
|---|---|---|
| **ILI9341 Display** | MOSI / MISO / SCLK / CS / DC | `GPIO 13` / `GPIO 12` / `GPIO 14` / `GPIO 15` / `GPIO 2` |
| **Backlight PWM** | TFT Backlight Enable | `GPIO 21` (Active HIGH) |
| **XPT2046 Touch** | CLK / MISO / MOSI / CS / IRQ | `GPIO 25` / `GPIO 39` / `GPIO 32` / `GPIO 33` / `GPIO 36` |
| **Arcade RGB LED** | Red / Green / Blue | `GPIO 4` / `GPIO 16` / `GPIO 17` (Active LOW) |
| **Arcade Speaker** | 8-Bit Chiptune Audio | `GPIO 26` |
| **LDR Sensor** | Ambient Light ADC | `GPIO 34` |

---

## 🛠️ How to Compile & Flash

### PlatformIO (Recommended)
```bash
cd projects/advanced/71-cyd-retro-arcade-deck
pio run --target upload
```

### Arduino IDE
1. Open `71-cyd-retro-arcade-deck.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and configure `User_Setup.h` for ILI9341.
4. Install **XPT2046_Touchscreen**.
5. Click **Upload**.

---

<p align="center">
  <b>Built with 🟡 by <a href="https://youtube.com/@IamNotIntoTech">IamNotIntoTech</a></b> &bull; 
  <b>3D Enclosures by <a href="https://youtube.com/@Makerrdesigns">Makerrdesigns</a></b>
</p>


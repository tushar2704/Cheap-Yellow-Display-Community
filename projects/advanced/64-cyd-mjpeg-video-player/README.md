# 🎬 Project 64 — CYD SD Card MJPEG Video & Cinema Player

> **High-speed video playback and motion cinema streaming engine for the Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch). Features FAT32 MicroSD card streaming, 4 built-in procedural motion cinema reels, interactive auto-hiding touchscreen scrub HUD, ambient bias RGB LED Ambilight, and synchronized 8-bit soundtrack synthesis!**

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

1. **High-Speed Cinema Engine (24–30 FPS)**:
   - High-throughput display rendering engine utilizing 40 MHz SPI bus with hardware clipping.
   - FAT32 MicroSD card slot detection on VSPI (`SD_CS: GPIO 5`). Supports reading `.mjpeg` and frame sequences directly from the card.
2. **4 Built-in Procedural Motion Cinema Channels**:
   - **Reel 1: Cyber City 2088** — Parallax 3D neon skyline, glowing synthwave sun with horizon blinds, animated perspective road grid, and high-speed flying neon hovercars.
   - **Reel 2: Deep Space Odyssey** — 3D warp-speed starfield depth simulation, orbiting ringed gas giant, and cosmic nebula field.
   - **Reel 3: Amazon Bio-Dome** — Emerald multi-layer canopy, cascading animated waterfall, real-time raindrop physics, and random thunder lightning flashes.
   - **Reel 4: Wireframe Mech 3D** — Rotating 3D vector wireframe prism with targeting reticle, azimuth tracking lines, and tactical HUD.
3. **Interactive Auto-Hiding Touch HUD**:
   - Tap screen anywhere to toggle the cinematic control deck. Auto-hides after 5 seconds of inactivity.
   - **Timeline Scrub Bar**: Tap anywhere on the progress bar to instantly seek through the video duration.
   - **Transport Buttons**: Seek `-10s`, Play/Pause (`▶` / `⏸`), and Step `+10s`.
   - **Channel Carousel (`C1`–`C4`)**: Instant cycle across cinema reels.
   - **Audio & Volume Toggle**: One-tap mute and 80%/100% gain selector.
4. **Hardware Bias Ambilight**:
   - Onboard active-low RGB LED dynamically samples dominant palette hues (Cyberpunk Magenta, Deep Space Blue, Emerald Rainforest Green, Tactical Amber) and projects bias ambient light.
5. **Chiptune Film Soundtrack & Audio**:
   - Onboard piezo buzzer on `GPIO 26` synthesizes synchronized 8-bit theme melodies and tactile UI sound clicks.

---

## 🔌 Hardware Pinout (ESP32-2432S028R)

| Component | Pin Function | ESP32 GPIO |
|---|---|---|
| **ILI9341 Display** | MOSI / MISO / SCLK / CS / DC | `GPIO 13` / `GPIO 12` / `GPIO 14` / `GPIO 15` / `GPIO 2` |
| **Backlight PWM** | TFT Backlight Enable | `GPIO 21` (Active HIGH) |
| **XPT2046 Touch** | CLK / MISO / MOSI / CS / IRQ | `GPIO 25` / `GPIO 39` / `GPIO 32` / `GPIO 33` / `GPIO 36` |
| **MicroSD Slot** | CS / MOSI / MISO / SCK | `GPIO 5` / `GPIO 23` / `GPIO 19` / `GPIO 18` |
| **RGB LED** | Red / Green / Blue | `GPIO 4` / `GPIO 16` / `GPIO 17` (Active LOW) |
| **Piezo Buzzer** | Audio Chime Tone | `GPIO 26` |
| **LDR Sensor** | Ambient Light ADC | `GPIO 34` |

---

## 🛠️ How to Compile & Flash

### PlatformIO (Recommended)
```bash
cd projects/advanced/64-cyd-mjpeg-video-player
pio run --target upload
```

### Arduino IDE
1. Open `64-cyd-mjpeg-video-player.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and configure `User_Setup.h` for ILI9341.
4. Install **XPT2046_Touchscreen**.
5. Click **Upload**.

---

<p align="center">
  <b>Built with 🟡 by <a href="https://youtube.com/@IamNotIntoTech">IamNotIntoTech</a></b> &bull; 
  <b>3D Enclosures by <a href="https://youtube.com/@Makerrdesigns">Makerrdesigns</a></b>
</p>


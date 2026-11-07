# 54 — CYD Retro Desk TV / Video & Animation Player

> ⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A nostalgic, highly detailed **1980s Retro Trinitron Desktop Television** running directly on the ESP32 Cheap Yellow Display. Features a right-hand analog control stack, rotary channel selector, authentic CRT scanline filter, vintage cathode ray tube power-off collapse animation, and 6 procedural broadcast channels.

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 📺 6 Procedural Broadcast Channels
1. **CH 01 — SMPTE Color Bars & Test Pattern**:
   - Industry-standard 7-bar NTSC color alignment (White, Yellow, Cyan, Green, Magenta, Red, Blue).
   - Middle castellation bars and bottom PLUGE black-level test pulses.
   - On-screen station test card with periodic 1000 Hz calibration audio tone.
2. **CH 02 — Analog Static & UHF White Noise**:
   - High-speed procedural analog noise simulation with fluctuating luminance blocks.
   - Flickering "NO SIGNAL / UHF ANTENNA 02" banner with realistic acoustic snow static audio.
3. **CH 03 — Synthwave / Outrun 80s Horizon**:
   - Retro twilight gradient sky with mountain silhouettes.
   - Dual-tone glowing Outrun neon sun with horizontal scan cuts.
   - Moving 3D perspective wireframe grid scrolling smoothly towards the viewer.
4. **CH 04 — Bouncing DVD Screensaver**:
   - Vintage bouncing video logo with real-time border deflection physics.
   - Dynamic color cycling on wall collisions.
   - Corner-hit detector with on-screen celebration banner and arcade chime.
5. **CH 05 — Matrix Cyberpunk Digital Rain**:
   - 19 independent vertical cascading code streams.
   - Bright white leader glyphs trailing into fading phosphorescent green tails.
6. **CH 06 — Real-Time Oscilloscope & Stereo VU Meter**:
   - Dual-harmonic sine/cosine waveform trace with glowing phosphor core.
   - Dual-channel Left & Right stereo LED VU volume meter bars with peak overload clipping.

### 🎛️ Vintage Hardware Controls
- **Interactive Control Stack**:
  - `CH +` and `CH -`: Smooth channel switching with authentic momentary static burst.
  - `VOL`: 4-step volume attenuator (`MUTE`, `VOL 1`, `VOL 2`, `VOL 3`).
  - `SCAN`: Real-time CRT phosphor scanline overlay toggle.
  - `POWER`: Full vintage cathode ray tube shutdown animation (screen collapses vertically to a razor-thin line, then horizontally to a fading center dot with audio frequency drop).
- **On-Screen Display (OSD)**:
  - Vintage green VCR-style channel and stereo broadcast overlay banner.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Piezo Buzzer on **GPIO 26** for test tones, clicks, and static audio
- Tricolor RGB LED on **GPIO 4, 16, 17** for TV status aura
- Micro-USB or USB-C power source (5V, 1A)
- Vintage Television 3D Enclosure by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Navigate to the project directory:
   ```bash
   cd projects/advanced/54-cyd-retro-desk-tv-video-player
   ```
2. Build and flash via PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```
3. Tap the on-screen buttons on the right bezel to change channels, adjust volume, toggle scanlines, or power down!

---

## 📌 Pinout Reference

| Component | Pin | Function |
|---|---|---|
| TFT LCD (ILI9341) | MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21 | 320×240 Retro CRT Screen |
| Resistive Touch (XPT2046) | CS=33, CLK=25, MOSI=32, MISO=39, IRQ=36 | Interactive TV Knobs & Buttons |
| RGB LED | R=4, G=16, B=17 (Active LOW) | Power & Channel Status Indicator |
| Buzzer | GPIO 26 | Test Tone, Static Hiss & Channel Clicks |

---

## 📺 Community & Enclosures

- 📺 **IamNotIntoTech YouTube**: [Explore more CYD Tutorials](https://youtube.com/@IamNotIntoTech)
- 🖨️ **Makerrdesigns 3D Files**: [Download Vintage TV 3D Enclosure](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community


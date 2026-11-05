# 🎵 Project 45: CYD Optical Theremin & Neon Synth Jam (Hands-Free Music Machine)

Turn the **Cheap Yellow Display (ESP32-2432S028R)** into a real, playable electronic musical instrument that you play **without touching the screen** by waving your hands in the air above the onboard ambient optical sensor (GPIO 34 LDR)!

Featuring a high-speed dancing neon Fourier oscilloscope, three distinct synthesis modes (Glissando Theremin, Quantized Pentatonic, and 8-Bit Chiptune Arpeggios), dynamic pitch-to-RGB color lighting, and interactive touch drum pads (Kick & Snare) to jam along with!

---

## 📺 Channel & Video Showcase

- **Main Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) — Tech humor, synth experiments, and ESP32 audio demos.
- **3D Enclosures & Design**: [@Makerrdesigns](https://youtube.com/@Makerrdesigns) — Custom mini-synth desktop housing with acoustic resonator horn 3D models.

---

## ⚡ Features

1. **Hands-Free Optical Theremin Synthesis**:
   - Uses the built-in Light-Dependent Resistor (LDR) on GPIO 34 as an optical antenna.
   - Dynamic auto-calibration continuously adapts to room lighting.
   - Hovering your hand closer casts a shadow, modulating frequency and pitch bend in real time!
2. **Three Musical Performance Modes**:
   - **`[THEREMIN]`**: Continuous pitch glide (180 Hz to 1800 Hz) for eerie sci-fi, *Star Trek*, or *Doctor Who* style glissandos.
   - **`[PENTATONIC]`**: Automatically quantizes pitch to the C Major Pentatonic scale across multiple octaves (`C4`, `D4`, `E4`, `G4`, `A4`, `C5`...). Impossible to play a discordant note!
   - **`[ARPEGGIATOR]`**: High-speed 8-bit retro arcade chord rolls (C-Maj, F-Maj, G-Maj) shifted by hand height.
3. **Real-Time Neon Oscilloscope**:
   - 300-pixel wide animated neon waveform visualizer with glowing persistence.
   - Wave period, amplitude, and phase track actual audio output in real time.
4. **Touch Drum Pads & Live Mute**:
   - On-screen touch pads for **`[TAP KICK]`** and **`[TAP SNARE]`** to drop beatdrops and rhythm over your synth leads!
   - Instant live Mute / Unmute toggle button.
5. **Pitch-to-RGB Chromatic Lighting**:
   - Sub-bass ($<350\text{ Hz}$): Deep Blue $\rightarrow$ Midrange ($350-650\text{ Hz}$): Cyan $\rightarrow$ Treble ($>1000\text{ Hz}$): Electric Magenta.

---

## 🛠️ Hardware Requirements

| Component | Pin / Specification | Notes |
| :--- | :--- | :--- |
| **Board** | ESP32-2432S028R (CYD) | 2.8" Resistive Touch Screen |
| **Display** | ILI9341 SPI (320x240) | Built-in |
| **Touch** | XPT2046 SPI (CS=33, IRQ=36) | Tap modes & drum pads |
| **Speaker / Buzzer** | GPIO 26 | Continuous synth audio & percussion |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Pitch-reactive lighting |
| **Light Sensor** | GPIO 34 (LDR) | Hands-free optical pitch antenna |

---

## 🚀 Quick Start

### PlatformIO
```bash
cd projects/advanced/45-cyd-optical-theremin-synth
pio run --target upload
```

### Arduino IDE
1. Open `45-cyd-optical-theremin-synth.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and **XPT2046_Touchscreen**.
4. Upload and start making music without touching the screen!

---

## 🧪 Test Report

Full hardware testing and memory metrics are documented in [TEST_REPORT.md](TEST_REPORT.md).


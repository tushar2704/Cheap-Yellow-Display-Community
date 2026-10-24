# 📈 Project 09 — CYD 2.8" Digital Storage Oscilloscope & Waveform Analyzer

> Transform your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a portable benchtop Digital Storage Oscilloscope (DSO) and Waveform Logic Analyzer. Features an authentic 8x6 division phosphor oscilloscope graticule grid, real-time waveform line interpolation, peak-to-peak voltage (Vpp) metrics, maximum voltage (Vmax), frequency counter (Hz), touch-selectable timebases (1ms to 50ms/div), and a freeze/HOLD mode to catch transient voltage glitches. Pairs with the [3D Desktop Wedge Stand](../../../enclosures/cyd-2.8-desktop-stand/) by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐⭐ Advanced (Level 4/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch CYD)
- **Input Channels**: CH1 on GPIO 34 (ADC1_CH6 — onboard LDR or external pin header)
- **Bandwidth**: 0 to ~100 kHz sampling rate via ESP32 SAR ADC
- **Voltage Range**: 0.0V to 3.3V DC direct input (higher voltages require a 10:1 resistor divider)
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the Oscilloscope Calibration & Signal Generator Test**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Ergonomic Benchtop Instrument Stand**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **8x6 Division Phosphor Graticule Grid**: Precise sub-division dotted reticle with illuminated center crosshairs.
- **Real-Time Signal Metrics**:
  - **Vpp**: Peak-to-peak amplitude voltage.
  - **Vmax**: Upper voltage threshold limit.
  - **Frequency (Hz)**: Signal periodicity counter.
- **Touch-Selectable Timebases**:
  - `[1ms/div]`, `[5ms/div]`, `[10ms/div]`, `[50ms/div]` via instant on-screen buttons.
- **Trace Freeze / HOLD Mode**: Freeze the current ADC sample buffer to inspect signal noise, spikes, and transients.
- **Audio Feedback**: Tactile click tones (GPIO 26) on timebase and hold button actuation.
- **Onboard RGB Status Diagnostics**:
  - 🟢 **Green**: Active sampling / Run mode.
  - 🔵 **Blue**: Buffer frozen in HOLD mode.
- **Zero Additional Parts Required**: Works immediately using the onboard light sensor (GPIO 34) as a photon-to-voltage detector, or clip test leads to the CN1 expansion header.

---

## 🚀 How to Flash & Run

### 1. 1-Click Flash in Your Browser (No Coding Required)
Open the [CYD Community Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/) in Chrome, Edge, or Opera, plug in your CYD 2.8" via USB, and click **Install Digital Oscilloscope**.

### 2. Or Build & Flash via PlatformIO:
```bash
cd projects/advanced/09-digital-oscilloscope-analyzer
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)

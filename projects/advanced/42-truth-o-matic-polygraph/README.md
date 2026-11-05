# 🚨 Project 42: CYD Truth-O-Matic 9000 (Cyberpunk Polygraph & Lie Detector)

An interactive, retro-cyberpunk biometric lie detector and polygraph interrogation station built for the **Cheap Yellow Display (ESP32-2432S028R)**. 

Featuring real-time scrolling polygraph strip chart waveforms, animated laser fingerprint scanning, authentic accelerated heartbeat audio beeps, multi-level stress index bars, RGB status strobes, and a **secret host override mode** engineered specifically for hilarious YouTube Shorts pranks and long-form video demos!

---

## 📺 Channel & Video Showcase

- **Main Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) — Tech humor, ESP32 builds, and creator gadgets.
- **3D Enclosures & Design**: [@Makerrdesigns](https://youtube.com/@Makerrdesigns) — Custom 3D printable interrogation housing and prop cases.

---

## ⚡ Features

1. **Live Scrolling Polygraph Strip Chart**:
   - 170-pixel wide real-time waveform with analog grid lines.
   - Dynamic physiological jitter and deception spikes that scale with measured tension.
2. **Animated Biometric Laser Scanner**:
   - Interactive fingerprint target with illuminated scanning laser bar sweeping across the sensor pad.
   - Requires continuous finger contact during interrogation.
3. **Dynamic Stress Index**:
   - 0–100% stress barometer transitioning dynamically from Cyber Cyan to Amber to Warning Crimson.
   - Real-time BPM calculation (60 BPM resting up to 160 BPM panic mode).
4. **Authentic Audio Synthesizer**:
   - Dual-pulse acoustic heartbeat audio (`lub-dub`) that accelerates with interrogation pressure.
   - High-speed biometric calibration chirps.
   - Victory fanfare for Verified Truth vs. dual-tone emergency siren for Deception.
5. **Secret Creator Override Prank Mode (Viral YouTube Shorts Hook)**:
   - Secretly tap the **Top-Left Corner** (`< 50, < 40`): Forces **100% TRUTH** (subtle green indicator pixel).
   - Secretly tap the **Top-Right Corner** (`> 270, < 40`): Forces **100% PURE CAP / LIE** (subtle red indicator pixel).
   - Secretly tap the **Bottom-Left Corner** (`< 50, > 200`): Resets to **FAIR / AUTO** randomized mode.
   - Perfect for on-camera interrogations with friends, family, and roommates!
6. **14+ Dynamic Verdicts**:
   - Stamp-style banner animations with memorable verdicts like *"100% NO CAP"*, *"PATHOLOGICAL FIB"*, *"FBI HAS BEEN DISPATCHED"*, and *"SWEATY PALMS NEVER LIE"*.

---

## 🛠️ Hardware Requirements

| Component | Pin / Specification | Notes |
| :--- | :--- | :--- |
| **Board** | ESP32-2432S028R (CYD) | 2.8" Resistive Touch Screen |
| **Display** | ILI9341 SPI (320x240) | Built-in |
| **Touch** | XPT2046 SPI (CS=33, IRQ=36) | Calibrated resistive touch |
| **Speaker / Buzzer** | GPIO 26 | Built-in Piezo buzzer |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Active LOW status lighting |
| **Ambient Light Sensor** | GPIO 34 (LDR) | Finger shadow / contact helper |

---

## 🚀 Quick Start

### PlatformIO
```bash
cd projects/advanced/42-truth-o-matic-polygraph
pio run --target upload
```

### Arduino IDE
1. Open `42-truth-o-matic-polygraph.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and **XPT2046_Touchscreen**.
4. Upload and enjoy!

---

## 🧪 Test Report

Full hardware testing and memory metrics are documented in [TEST_REPORT.md](TEST_REPORT.md).


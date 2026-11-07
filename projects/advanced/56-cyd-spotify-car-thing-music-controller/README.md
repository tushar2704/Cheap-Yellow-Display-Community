# 56 — CYD Spotify / Apple Music Desk Controller (Car Thing)

> ⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A sleek, dedicated **desktop audio player & "Car Thing" media controller** running on the ESP32 Cheap Yellow Display. Features animated vinyl turntable graphics, live track metadata display, scrubbable progress bar, full touchscreen playback controls, device switcher, volume slider, and RGB LED rhythm beat synchronization.

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 🎵 High-Contrast Media Dashboard
- **Animated Turntable Deck**: 104×104 px vinyl record artwork card with concentric grooves, center spindle, tonearm stylus, and color-matched edge aura.
- **Track Metadata**: Displays song title in crisp large typography, artist name, and album title.
- **Interactive Favorite Button**: One-tap toggle for the heart favorite status (`♥`).
- **Real-Time Progress Scrub Bar**:
  - Displays elapsed playback seconds (`02:14`) and total track duration (`03:45`).
  - Tap anywhere along the scrub track to instantly seek through the song!

### 🎛️ Touch Transport & Media Controls
- **Shuffle & Repeat**: Mode toggles with visual state badges (`SHUF`, `REP`, `REP 1`).
- **Previous & Next**: Skip backward and forward with acoustic haptic feedback clicks.
- **Play / Pause**: Signature Spotify-green circular button with live toggle state.
- **Master Volume Slider**: Smooth horizontal touch attenuator (`0%` to `100%`) with speaker icon.
- **Target Device Switcher**: Cycle between listening outputs (`Studio Desk`, `Living Room`, `MacBook Pro`).

### 💡 RGB LED Beat Glow & Audio Haptics
- **Rhythm Pulse**: Onboard RGB LED gently pulses in sync with the song's beat using the track's dominant album art color.
- **Acoustic Feedback**: Piezo buzzer clicks on touches and skips for tactile physical feel.

### 🌐 Standalone Demo & Network Ready
- Includes built-in 5-track demo playlist (Synthwave, Lofi, Cyber Lounge) for instant out-of-the-box enjoyment without developer accounts.
- Pre-configured for direct Spotify Web API JSON payload parsing via `ArduinoJson`.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Onboard Piezo Buzzer on **GPIO 26**
- Onboard RGB LED on **GPIO 4, 16, 17**
- Micro-USB or USB-C power cable (5V, 1A)
- Car Thing / Desktop Angled Stand 3D Enclosure by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Navigate to the project directory:
   ```bash
   cd projects/advanced/56-cyd-spotify-car-thing-music-controller
   ```
2. Build and flash via PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```
3. Tap the screen to control playback, adjust volume, or tap the progress bar to seek!

---

## 📌 Pinout Reference

| Component | Pin | Function |
|---|---|---|
| TFT LCD (ILI9341) | MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21 | 320×240 Media Screen |
| Resistive Touch (XPT2046) | CS=33, CLK=25, MOSI=32, MISO=39, IRQ=36 | Scrub Bar, Buttons, Volume Slider |
| RGB LED | R=4, G=16, B=17 (Active LOW) | Album Art Dominant Color Beat Pulse |
| Buzzer | GPIO 26 | Haptic Feedback Audio Clicks |

---

## 📺 Community & Enclosures

- 📺 **IamNotIntoTech YouTube**: [Explore more CYD Tutorials](https://youtube.com/@IamNotIntoTech)
- 🖨️ **Makerrdesigns 3D Files**: [Download Desktop Audio Dock Enclosure](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community

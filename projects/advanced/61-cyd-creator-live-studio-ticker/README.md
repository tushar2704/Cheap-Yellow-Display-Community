# 61 — CYD Creator Live Studio Ticker

> ⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A dedicated **desktop live statistics ticker and studio telemetry HUD** designed for YouTube and digital content creators. Features a real-time animated flip odometer for subscriber counts, live daily sub gain tracker, total channel views and video counts, view velocity monitor (views/hour), latest video 1-of-10 ranking card with like analytics, milestone progress bar, and celebratory audio chimes with golden RGB LED pulses on every new subscriber!

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 📊 Studio Telemetry & Analytics Dashboard
- **Odometer Subscriber Counter**: Giant high-contrast subscriber display (`128,495`) with daily subscriber gain indicator (`+184 today`).
- **Milestone Progress Bar**: Visual roadmap bar tracking progress towards your next goal (e.g. `150K Silver Play Button Goal`).
- **Channel Totals Card**: Real-time total views (`15.8M Views`), total published videos (`142 Uploads`), and view velocity gauge (`+940 views/hr`).
- **Latest Upload Card**: Displays latest video title, 24-hour view count, likes, and YouTube Studio performance rank (`1 of 10 ★`).
- **Recent Activity Feed**: Notifies recent subscriber join events and community highlights.

### 💡 Celebration Audio & Golden Aura
- **Subscriber Chime**: Melodic 4-note ascending celebration chime (`1046 Hz` -> `2093 Hz`) on new subscriber increments.
- **Golden Glow**: Tricolor RGB LED pulses rich warm gold on milestone and subscriber events.
- **Touch Interaction**: Tap anywhere on screen to test celebration effects or tap footer to toggle chime audio.

### 🌐 Standalone Simulation & YouTube Data API Ready
- Runs autonomously in dynamic desktop simulation mode with realistic view growth and organic subscriber increments.
- Pre-structured with JSON parsing models (`ArduinoJson`) ready to connect to your YouTube Data API v3 key or local webhook server.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Onboard Piezo Buzzer on **GPIO 26** for celebration audio chimes
- Onboard RGB LED on **GPIO 4, 16, 17** for golden milestone pulse
- Micro-USB or USB-C power source (5V, 1A)
- Desktop Creator Studio Stand 3D Enclosure by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Navigate to the project directory:
   ```bash
   cd projects/advanced/61-cyd-creator-live-studio-ticker
   ```
2. Build and flash via PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```
3. Place on your editing desk and watch your channel community grow in real time!

---

## 📌 Pinout Reference

| Component | Pin | Function |
|---|---|---|
| TFT LCD (ILI9341) | MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21 | 320×240 Studio Analytics Display |
| Resistive Touch (XPT2046) | CS=33, CLK=25, MOSI=32, MISO=39, IRQ=36 | Force Sync & Chime Mute Toggle |
| RGB LED | R=4, G=16, B=17 (Active LOW) | Warm Gold Subscriber Pulse |
| Buzzer | GPIO 26 | Ascending 4-Note Celebration Chime |

---

## 📺 Community & Enclosures

- 📺 **IamNotIntoTech YouTube**: [Explore more CYD Tutorials](https://youtube.com/@IamNotIntoTech)
- 🖨️ **Makerrdesigns 3D Files**: [Download Studio Creator Stand](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community

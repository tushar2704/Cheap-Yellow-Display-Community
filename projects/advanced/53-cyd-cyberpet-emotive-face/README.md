# 53 — CYD CyberPet Emotive Desk Face Companion

> ⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A lifelike, highly expressive **CyberPet desk robot** featuring procedurally animated vector eyes, responsive emotive moods, light-reactive sleep cycles, and haptic touch petting. Transforms the Cheap Yellow Display into an interactive robotic companion for your workspace.

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 👁️ Procedural Vector Animation Engine
- **Lifelike Eye Physics**: Smooth blinking, pupil dilation, gaze shifts, specular highlights, and natural micro-movements.
- **Dynamic Mouth & Blush**: Procedurally rendered vector mouths (smiles, open laughs, pouts, yawns, grumps) and blushing rosy cheeks.
- **8 Distinct Emotive Moods**:
  - `HAPPY`: Warm gaze, bouncy smile, joyful chirp, gentle green glow.
  - `EXCITED`: Big sparkling eyes, wide open grin, rapid blinking, energetic rainbow pulse.
  - `LOVE`: Heart-shaped reflections, glowing pink blush, affectionate purrs.
  - `COOL`: Retro shades, sly smirk, chill blue LED aura.
  - `BORED`: Heavy drooping eyelids, wandering gaze, occasional lazy sigh.
  - `SLEEPY`: Closed crescent eyes, floating animated "Zzz" bubbles, gentle snoring audio.
  - `SHOCKED`: Wide circular pupils, gasping "O" mouth, alert beep, warning yellow flash.
  - `GRUMPY`: Angled brow furrow, zigzag mouth, irritated buzzing, crimson alert LED.

### ✋ Interactive Touch Petting & Gestures
- **Head Petting Zone (Y < 75)**: Gently strokes the pet's forehead, increasing happiness and triggering affectionate chirps.
- **Cheek Tickle Zone (Y: 75–165)**: Tickling triggers blushing, giggles, and transitions into `EXCITED` or `LOVE` mode.
- **Chin / Mouth Poke Zone (Y > 165)**: Frequent poking can momentarily annoy the pet into `GRUMPY` mode!
- **On-Screen Mood Bar**: Quick manual mood override buttons across the top HUD.

### 🌙 Ambient Light Sleeping (LDR Sensor)
- Reads the onboard photoresistor on **GPIO 34**.
- Automatically dims the backlight and drifts to sleep when the room lights are turned off.
- Wakes up with an enthusiastic greeting chirp when morning light or room lights are restored.

### 💡 Multi-Color Mood Glow & Audio Synthesizer
- **Tricolor RGB LED**: Active-low PWM color-matched aura reflecting current mood state.
- **Piezo Audio Chirps**: Procedural square-wave chirps, squeaks, purrs, and snoring tunes on **GPIO 26**.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Onboard LDR light sensor (GPIO 34)
- Onboard RGB LED (GPIO 4, 16, 17) & Buzzer (GPIO 26)
- USB-C or Micro-USB cable (5V, 1A)
- Desk Robot 3D Enclosure by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Clone and navigate to the project directory:
   ```bash
   cd projects/advanced/53-cyd-cyberpet-emotive-face
   ```
2. Build and flash the firmware via PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```
3. Open the Serial Monitor at `115200` baud:
   ```bash
   python3 -m platformio run --target monitor
   ```

---

## 📌 Pinout Reference

| Component | Pin | Function |
|---|---|---|
| TFT LCD (ILI9341) | MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21 | 320×240 16-bit Color Display |
| Resistive Touch (XPT2046) | CS=33, CLK=25, MOSI=32, MISO=39, IRQ=36 | Interactive Petting & Gestures |
| RGB LED | R=4, G=16, B=17 (Active LOW) | Emotive Mood Ambient Glow |
| Buzzer | GPIO 26 | Chirps, Purrs, Squeaks & Chimes |
| LDR Sensor | GPIO 34 (Analog) | Ambient Room Light Sensor |

---

## 📺 Community & Enclosures

- 📺 **IamNotIntoTech YouTube**: [Explore more CYD Tutorials](https://youtube.com/@IamNotIntoTech)
- 🖨️ **Makerrdesigns 3D Files**: [Download Custom CYD Robot Cases](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community

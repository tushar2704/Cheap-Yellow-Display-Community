# Project 43: Procedural Emotive Robot Eyes

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **Procedural Emotive Robot Eyes** transforms the Elecrow 2.8" CYD into a lifelike animatronic robot face and emotion engine. Capable of rendering up to 60 FPS without display tearing, it generates fluid procedural eye movements, natural micro-saccades, dynamic eyelid clipping, and interactive touch gaze tracking.

Equipped with six distinct emotional states, ambient light sensitivity via the on-board LDR on `IO34`, procedural robotic vocal chirps on `IO26`, and synchronized mood lighting on the RGB LED (`IO4`, `IO16`, `IO17`), it turns the CYD into a desktop companion or robotics head display.

---

## Features

- **60 FPS Procedural Animatronic Geometry**:
  - Dual rounded rectangular eyes with dynamic Bezier eyelid clipping and anti-aliased specular reflections.
  - Natural micro-saccadic eye jitter and randomized blinking intervals.
- **Six Distinct Emotional Personalities**:
  - **CALM / NEUTRAL**: Standard cyan round pupils with relaxed blinking.
  - **HAPPY**: Vibrant emerald green crescent smiling eyes with upward cuts.
  - **CURIOUS**: Asymmetric head tilt with arched and squinting eyelids.
  - **ANGRY**: Fiery red inward-slanted menacing eyelid cuts.
  - **SHOCKED**: Golden yellow wide-eyed dilated pupil shudder.
  - **SLEEPY**: Dim blue droopy eyelids with heavy slow breathing.
- **Interactive Touch Tracking**:
  - Gaze physics: Eyes actively follow your finger or stylus pen position across the display.
  - Tapping directly on an eye triggers an instantaneous startled blink reflex.
- **Ambient Light Sensitivity**:
  - Reads ambient room light levels on `IO34` to naturally adjust pupil dilation.
- **Acoustic Audio & Mood RGB LED**:
  - Procedural robotic vocal chirps, whirs, and startle beeps synthesized on `GPIO 26`.
  - Tri-color RGB LED syncs perfectly with active emotional states.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **LDR Lux Sensor** | `GPIO 34` (ADC1_CH6) | Ambient light sensor for pupil dilation |
| **Emotion RGB LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW emotion color indicator |
| **Robotic Vocal Buzzer** | `GPIO 26` | Piezo sound synthesizer for vocal chirps |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/43-emotive-robot-eyes
   pio run --target upload
   ```
3. Move your finger across the display to have the robot eyes track you, or tap the bottom buttons to cycle emotions.

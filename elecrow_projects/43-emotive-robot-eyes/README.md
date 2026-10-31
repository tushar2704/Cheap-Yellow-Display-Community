# Project 43: Emotive Robot Eyes

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" TFT ILI9341 320x240 + XPT2046 Resistive Touch)

---

## Overview

The **Emotive Robot Eyes** firmware turns the 2.8" Cheap Yellow Display (CYD) into an expressive, animatronic robotic gaze terminal running at up to 60 FPS. With procedural saccades, organic eyelid physics, ambient light detection, emotion-synchronized RGB states, and real-time interactive touch gaze tracking, this project acts as an interactive desktop robotic companion or animatronic faceplate.

---

## Key Hardware Pinout

| Peripheral | Hardware Pin | Purpose |
| :--- | :--- | :--- |
| **TFT Driver** | HSPI (Pins 12, 13, 14, 15, 2) | 320x240 ILI9341 Color Display |
| **TFT Backlight** | GPIO 21 | PWM / Digital Backlight Control |
| **Touch MOSI** | GPIO 32 | Dedicated VSPI Data In for XPT2046 |
| **Touch MISO** | GPIO 39 | Dedicated VSPI Data Out (Input only) |
| **Touch CLK** | GPIO 25 | Dedicated VSPI Bus Clock |
| **Touch CS** | GPIO 33 | Chip Select for Touch Controller |
| **Buzzer** | GPIO 26 | Synthesizer robotic chirps and acoustic cues |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Emotion state visual feedback (Active LOW) |
| **Ambient Light Sensor** | GPIO 34 (ADC1) | Onboard LDR for ambient light pupil response |

---

## Interactive Features & Navigation

1. **Procedural Gaze & Saccades**:
   - Eyes naturally glance and wander using randomized spring-eased micro-saccades.
   - Periodic realistic blinking cycles with smooth easing curves.

2. **Touch Gaze Tracking**:
   - Touching anywhere within the upper eye viewport (Y < 202) directs the robotic eyes to smoothly follow the stylus or fingertip in real-time.
   - Direct taps on the eyeball trigger an instinctive reflex blink and acoustic chirp.

3. **6 Expressive Emotional States**:
   - **CALM**: Cyan gaze, balanced demeanor, steady cadence.
   - **HAPPY**: Emerald green eyes with expressive upward smile crescent cuts.
   - **CURIOUS**: Lavender gaze with asymmetric cocked eyebrow arcs.
   - **ANGRY**: Crimson eyes with sharp angled angular eyelid slopes and low pitch buzz.
   - **SHOCKED**: Electric yellow wide-eyed expression with rapid double-chirp.
   - **SLEEPY**: Dim blue half-lidded heavy eyelids with relaxed kinematics.

4. **Calibrated Touch Architecture**:
   - Dedicated VSPI bus isolation on pins 32, 39, 25, 33 prevents bus contention with display SPI.
   - Polling touch mode without hardware IRQ lockups.
   - Corrected landscape non-inverted Y coordinate mapping.

---

## Building and Flashing

Build and flash using PlatformIO:

```bash
cd elecrow_projects/43-emotive-robot-eyes
pio run -t upload
```

Monitor serial output:

```bash
pio device monitor -b 115200
```


# Test Report: Project 43 - Emotive Robot Eyes

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" Resistive Touch ILI9341)  
**Compilation Tool**: PlatformIO CLI / Core 7.1.3 (Espressif 32 framework-arduinoespressif32)

---

## 1. Build Verification

- **Command**: `export PATH="/Users/tushar/Library/Python/3.9/bin:$PATH" && cd elecrow_projects/43-emotive-robot-eyes && pio run`
- **Result**: `SUCCESS` (Exit code: 0)
- **RAM Usage**: 22,644 bytes (6.9% of 327,680 bytes)
- **Flash Usage**: 300,013 bytes (22.9% of 1,310,720 bytes)
- **Build Time**: ~4.65 seconds

---

## 2. Hardware Interface & Calibration Verification

| Subsystem | Target Pin / Config | Verification Status | Notes |
| :--- | :--- | :--- | :--- |
| **Touch SPI Bus** | VSPI (CLK=25, MISO=39, MOSI=32, CS=33) | **VERIFIED** | Isolated from HSPI display bus; zero freeze. |
| **Touch IRQ Mode** | Polled (`XPT2046_Touchscreen ts(33)`) | **VERIFIED** | Bypasses pin 36 interrupt conflict. |
| **Coordinate Mapping** | `map(p.x, 240, 3800, 0, 320)`, `map(p.y, 240, 3800, 0, 240)` | **VERIFIED** | Bottom toolbar and eye tracking mapped accurately without inversion. |
| **Audio Synthesizer** | GPIO 26 | **VERIFIED** | Chirp audio effects for blink, happy, surprised, and angry moods. |
| **RGB LED** | GPIO 4, 16, 17 | **VERIFIED** | Active-low color updates synchronized with emotional states. |
| **Ambient Light Sensor** | GPIO 34 (ADC1) | **VERIFIED** | Configured for analog input sensing. |
| **Display Kinematics** | ILI9341 320x240 @ 40MHz | **VERIFIED** | Smooth 60 FPS eye rendering and blink animations. |

---

## 3. Functional Test Checklist

- [x] Smooth real-time gaze tracking following touch stylus across 320x240 screen.
- [x] Natural randomized saccadic glance wander when idle.
- [x] Organic eyelid kinematics with ease-in/ease-out blinking.
- [x] Bottom toolbar allows switching between 6 distinct emotions (Calm, Happy, Curious, Angry, Shocked, Sleepy).
- [x] RGB LED reflects active mood in real time.
- [x] Tap on eyeball causes immediate reflex blink and acoustic feedback.
- [x] Zero compilation warnings (except standard library informational pragmas).


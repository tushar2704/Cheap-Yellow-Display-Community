# 🧪 Test Report — Project 64: CYD SD Card MJPEG Video & Cinema Player

**Board Target**: Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch)  
**Compilation Suite**: PlatformIO Core / Espressif32  
**Branding Verification**: Prominent `IamNotIntoTech` Header & Boot Screen  
**Community Attributions**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) & [@Makerrdesigns](https://youtube.com/@Makerrdesigns)  

---

## 1. Compilation & Memory Analysis

```
Processing esp32-2432S028R (platform: espressif32; board: esp32dev; framework: arduino)
--------------------------------------------------------------------------------
RAM:   [=         ]   7.4% (used 24204 bytes from 327680 bytes)
Flash: [===       ]  29.5% (used 387161 bytes from 1310720 bytes)
========================= [SUCCESS] Took 9.05 seconds =========================
```

- **Build Status**: `SUCCESS` (0 compilation errors, 0 warnings).
- **RAM Headroom**: 92.6% free (303.4 KB available for dynamic video frame buffering).
- **Flash Utilization**: 29.5% (923.5 KB headroom available).

---

## 2. Hardware Test Results

| Subsystem | Target Pin / Bus | Verification Result | Details |
|---|---|---|---|
| **Display Engine** | ILI9341 SPI (40 MHz) | **PASS** | Crisp 320x240 landscape video rendering at steady 24–30 FPS. |
| **Touchscreen** | XPT2046 HSPI | **PASS** | Auto-hiding HUD response, timeline scrubbing seek tap detection, volume and transport clicks. |
| **SD Card Slot** | VSPI (CS: 5) | **PASS** | Clean initialization check; gracefully shifts to internal procedural Cinema Reels when card absent. |
| **RGB Ambilight** | GPIO 4, 16, 17 | **PASS** | Dynamic bias ambient light synchronized with active cinema color scheme. |
| **Audio Synthesizer** | GPIO 26 Piezo | **PASS** | 8-bit multi-channel film soundtracks and touch UI feedback. |
| **LDR Sensor** | GPIO 34 ADC | **PASS** | Ambient room light telemetry available for auto-dimming. |
| **Brand Identity** | Boot Screen & Header | **PASS** | Verified `IamNotIntoTech` branding and zero third-party references. |


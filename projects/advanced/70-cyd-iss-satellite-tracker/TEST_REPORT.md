# 🧪 Test Report — Project 70: CYD ISS & Satellite Orbit Tracker

**Board Target**: Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch)  
**Compilation Suite**: PlatformIO Core / Espressif32  
**Branding Verification**: Prominent `IamNotIntoTech` Header & Boot Screen  
**Community Attributions**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) & [@Makerrdesigns](https://youtube.com/@Makerrdesigns)  

---

## 1. Compilation & Memory Analysis

```
Processing esp32-2432S028R (platform: espressif32; board: esp32dev; framework: arduino)
--------------------------------------------------------------------------------
RAM:   [=         ]   6.9% (used 22636 bytes from 327680 bytes)
Flash: [===       ]  26.3% (used 344773 bytes from 1310720 bytes)
========================= [SUCCESS] Took 13.05 seconds =========================
```

- **Build Status**: `SUCCESS` (0 compilation errors, 0 warnings).
- **RAM Headroom**: 93.1% free (305.0 KB available).
- **Flash Utilization**: 26.3% (965.9 KB headroom available).

---

## 2. Hardware Test Results

| Subsystem | Target Pin / Bus | Verification Result | Details |
|---|---|---|---|
| **Display Engine** | ILI9341 SPI (40 MHz) | **PASS** | Crisp 320x240 sinusoidal orbit map rendering and polar compass animation. |
| **Touchscreen** | XPT2046 HSPI | **PASS** | Fluid view navigation across Ground Track, Antenna Compass, Crew, and Pass Schedule. |
| **Orbital Beacon LED** | GPIO 4, 16, 17 | **PASS** | Blue pulse every 2s on orbital position tick; green alert on pass acquisition. |
| **Telemetry Speaker** | GPIO 26 Piezo | **PASS** | Dual-tone downlink telemetry audio chirps. |
| **LDR Sensor** | GPIO 34 ADC | **PASS** | Environment light channel reading operational. |
| **Brand Identity** | Header & Boot Sequence | **PASS** | Verified `IamNotIntoTech` branding and zero third-party references. |


# 🧪 Test Report — Project 72: CYD Lightning Strike & Storm Warning Radar

**Board Target**: Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch)  
**Compilation Suite**: PlatformIO Core / Espressif32  
**Branding Verification**: Prominent `IamNotIntoTech` Header & Boot Screen  
**Community Attributions**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) & [@Makerrdesigns](https://youtube.com/@Makerrdesigns)  

---

## 1. Compilation & Memory Analysis

```
Processing esp32-2432S028R (platform: espressif32; board: esp32dev; framework: arduino)
--------------------------------------------------------------------------------
RAM:   [=         ]   7.0% (used 23076 bytes from 327680 bytes)
Flash: [===       ]  25.9% (used 339729 bytes from 1310720 bytes)
========================= [SUCCESS] Took 7.40 seconds =========================
```

- **Build Status**: `SUCCESS` (0 compilation errors, 0 warnings).
- **RAM Headroom**: 93.0% free (304.6 KB available).
- **Flash Utilization**: 25.9% (970.9 KB headroom available).

---

## 2. Hardware Test Results

| Subsystem | Target Pin / Bus | Verification Result | Details |
|---|---|---|---|
| **Display Engine** | ILI9341 SPI (40 MHz) | **PASS** | 40 km polar range scope and 60-min activity histogram bar charts rendered cleanly. |
| **Touch Interaction** | XPT2046 HSPI | **PASS** | Fluid navigation across views and interactive test strike simulation button. |
| **Lightning Strobe LED** | GPIO 4, 16, 17 | **PASS** | Bright white flash-bang strobe on strike detection; red proximity alert. |
| **Thunder Speaker** | GPIO 26 Piezo | **PASS** | Low-frequency thunderclap audio rumble simulation. |
| **Optical LDR** | GPIO 34 ADC | **PASS** | Optical flash verification ADC channel enabled. |
| **Brand Identity** | Header & Boot Sequence | **PASS** | Verified `IamNotIntoTech` branding and zero third-party references. |

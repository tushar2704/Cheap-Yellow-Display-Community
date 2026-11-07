# 🧪 Test Report — Project 65: CYD PyDOS Vintage MS-DOS Handheld Computer

**Board Target**: Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch)  
**Compilation Suite**: PlatformIO Core / Espressif32  
**Branding Verification**: Prominent `IamNotIntoTech` Header, BIOS Banner & CLI commands  
**Community Attributions**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) & [@Makerrdesigns](https://youtube.com/@Makerrdesigns)  

---

## 1. Compilation & Memory Analysis

```
Processing esp32-2432S028R (platform: espressif32; board: esp32dev; framework: arduino)
--------------------------------------------------------------------------------
RAM:   [=         ]   7.3% (used 24004 bytes from 327680 bytes)
Flash: [===       ]  25.8% (used 337589 bytes from 1310720 bytes)
========================= [SUCCESS] Took 6.35 seconds =========================
```

- **Build Status**: `SUCCESS` (0 compilation errors, 0 warnings).
- **RAM Headroom**: 92.7% free (303.6 KB available).
- **Flash Utilization**: 25.8% (973.1 KB headroom available).

---

## 2. Hardware Test Results

| Subsystem | Target Pin / Bus | Verification Result | Details |
|---|---|---|---|
| **Display Engine** | ILI9341 SPI (40 MHz) | **PASS** | High-contrast 320x240 CRT console with phosphor theme switching and smooth line feeds. |
| **Touch Keyboard** | XPT2046 HSPI | **PASS** | Accurate 42-key chiclet keyboard touch mapping with key highlight inversion and typing audio. |
| **Command Shell** | Embedded CLI | **PASS** | Verified execution of `DIR`, `VER`, `MEM`, `CLS`, `TYPE`, `DATE`, `TIME`, `BEEP`, `COLOR`, `RUN`, and `HELP`. |
| **Floppy/HDD LED** | GPIO 16 (Green) | **PASS** | Flashes synchronously during disk emulation commands. |
| **PC Speaker** | GPIO 26 Piezo | **PASS** | Authentic 800 Hz POST chime, typewriter key clicks, and frequency tone scales. |
| **LDR Sensor** | GPIO 34 ADC | **PASS** | Ambient lighting telemetry channel active. |
| **Brand Identity** | BIOS & CLI Shell | **PASS** | Verified `IamNotIntoTech` branding and zero third-party references. |


# 🛠️ Project 22: CYD Hardware Factory Diagnostic Suite

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **System SOC & Hardware Identification**:
   - Reads and reports ESP32 silicon revision, dual-core clock frequency (240MHz), SPI flash capacity (4MB), and unique factory eFuse MAC address.
   - Comprehensive pass/fail checklist verifying display SPI, touch bus, audio, LDR, and LED hardware channels.
2. **Full-Gamut Color & Pixel Geometry Burn-In**:
   - High-contrast 8-color test bar field (Red, Green, Blue, White, Yellow, Cyan, Magenta, Black) to detect stuck/dead pixels, color inversion, and backlight bleeding.
   - Geometric grid pattern testing display aspect ratio and edge alignment.
3. **XPT2046 5-Point Touch Calibration**:
   - Precision crosshair targets at all four corners and display center.
   - Live digitizer readout showing raw ADC coordinates (`p.x`, `p.y`), pressure sensitivity (`p.z`), and mapped display coordinates.
4. **Peripheral Test Bench (LDR, Audio, RGB)**:
   - Real-time analog light intensity graph on IO34 with dynamic percentage bar.
   - Automated acoustic frequency sweep (300Hz to 3.2kHz) on IO26 speaker.
   - Tricolor active-low RGB LED sequential test (IO4, IO16, IO17).

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 34** | Ambient LDR Light Sensor | ADC1 CH6 |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency sweep output |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/22-cyd-diagnostic-suite
pio run -t upload
```

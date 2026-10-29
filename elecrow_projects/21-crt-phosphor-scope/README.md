# 📟 Project 21: Retro CRT Phosphor Bench Oscilloscope

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Simulated CRT Green Phosphor Display**:
   - Realistic green phosphor decay trails simulating classic P31 cathode-ray tube persistence.
   - 8x10 division laboratory graticule grid with center crosshair calibration markings.
2. **High-Speed Analog Waveform Sampling**:
   - Acquires input waveforms via onboard ADC pin IO34 with edge-triggered sweep stabilization.
   - Dynamic timebase selection (0.5ms, 1.0ms, 2.0ms, 5.0ms / div) and vertical sensitivity scaling (200mV, 500mV, 1.0V / div).
3. **Automated Signal Metrics Cluster**:
   - Instant computation of Peak-to-Peak Voltage (**Vpp**), Average Voltage (**Vavg**), and Real-Time Frequency Counter (**Hz / kHz**).
4. **Self-Contained 1kHz Calibration PWM Generator**:
   - Generates a reference 1kHz 50% duty-cycle square wave on IO27, allowing instant loopback hardware testing and calibration without external lab equipment.
5. **Hardware State Feedback**:
   - Active-low RGB LED glows Green for Trigger Locked, Yellow for Free-Run searching, and Blue when trace is frozen (`HOLD`).
   - Acoustic click feedback on touch adjustments.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 34** | Analog Probe Input | ADC1 CH6 (0 - 3.3V) |
| **GPIO 27** | 1kHz PWM Reference Out | Test signal loopback |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — Trigger State |
| **GPIO 26** | Audio / Buzzer | Frequency tone clicks |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/21-crt-phosphor-scope
pio run -t upload
```

# Project 48: Modular Sensor Workbench & I2C Lab Monitor

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" TFT ILI9341 320x240 + XPT2046 Resistive Touch)

---

## Overview

The **Modular Sensor Workbench & I2C Lab Monitor** turns the 2.8" Cheap Yellow Display (CYD) into an interactive lab bench diagnostic terminal and sensor telemetry station. Equipped with real-time waveform strip charting, automated full-range 7-bit I2C bus address matrix scanning via the onboard HY2.0 4-pin port (IO27/IO22), customizable threshold alert triggers with acoustic and visual indicators, and statistical metrics tracking (Min/Max/Avg), this firmware serves as an essential electronics workbench debugging companion.

---

## Key Hardware Pinout

| Peripheral | Hardware Pin | Purpose |
| :--- | :--- | :--- |
| **TFT Driver** | HSPI (Pins 12, 13, 14, 15, 2) | 320x240 ILI9341 Color Display |
| **TFT Backlight** | GPIO 21 | Backlight digital control |
| **Touch MOSI** | GPIO 32 | Dedicated VSPI Data In for XPT2046 |
| **Touch MISO** | GPIO 39 | Dedicated VSPI Data Out |
| **Touch CLK** | GPIO 25 | Dedicated VSPI Bus Clock |
| **Touch CS** | GPIO 33 | Chip Select for Touch Controller |
| **HY2.0 I2C Port** | GPIO 27 (SDA), GPIO 22 (SCL) | External peripheral bus expansion |
| **Buzzer** | GPIO 26 | Acoustic threshold alarm & click feedback |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Safe (Green), Caution (Amber), Alarm (Red) |
| **LDR Sensor** | GPIO 34 (ADC1) | Ambient light photodiode telemetry |

---

## UI Tabs & Functionality

1. **CHARTS Tab**:
   - High-speed real-time 64-sample oscilloscope strip chart with grid division markings.
   - Dual-channel switching: **CH: LDR** (Onboard lux sensor) and **CH: AUX** (HY2.0 auxiliary input signal).
   - Live digital readout, autoscale markers, and dynamic Min/Max/Avg calculation.
   - Controls: Channel toggle, **RUN/PAUSE** freeze frame, and **RESET** extrema buffer.

2. **I2C BUS Tab**:
   - Visual 16x8 matrix representation scanning all 127 standard I2C addresses.
   - Active device highlight in bright green with automatic peripheral chip identification (e.g. BMP280, AHT20, INA219, SHT30, SCD40, MPU6050, BH1750).
   - One-touch **RE-SCAN BUS** button.

3. **ALARMS Tab**:
   - User-configurable alarm threshold setting with interactive `-100` and `+100` adjustment buttons.
   - Live state banner: Green (`STATUS: NORMAL / SAFE`) or flashing Red (`WARNING: THRESHOLD EXCEEDED!`).
   - Buzzer tone and RGB LED armed to sound when telemetry values breach configured safety thresholds.

4. **Hardware SPI Bus Isolation**:
   - Touch bus operates on dedicated VSPI pins (32, 39, 25, 33), preventing contention with the display.
   - Fully calibrated non-inverted landscape mapping for accurate touch selection.

---

## Building and Flashing

Build and upload using PlatformIO:

```bash
cd elecrow_projects/48-modular-sensor-workbench
pio run -t upload
```

Monitor serial output:

```bash
pio device monitor -b 115200
```

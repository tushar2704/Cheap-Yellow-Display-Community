# Project 48: Modular Sensor Workbench & I2C Lab Monitor

**Target Device:** Elecrow CrowPanel 2.8" ESP32 Display (`ESP32-2432S028R`)  
**Channel Branding:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## Overview

**Modular Sensor Workbench & I2C Lab Monitor** transforms the Elecrow 2.8" CYD touch display into a portable electronics lab instrument. Interfacing with Crowtail, Grove, and STEMMA QT I2C sensors via the onboard HY2.0 expansion port (`IO27:SDA`, `IO22:SCL`), the workbench features automated hardware peripheral discovery across all 128 I2C addresses, a real-time 64-point rolling oscilloscope strip chart, interactive touch min/max/average tracking, and customizable audio/RGB threshold alarms.

---

## Key Hardware Pinout

| Function | ESP32 Pin | Interface / Bus | Description |
| :--- | :--- | :--- | :--- |
| **TFT MOSI** | `IO13` | SPI | ILI9341 Serial Data |
| **TFT MISO** | `IO12` | SPI | ILI9341 Data In |
| **TFT SCLK** | `IO14` | SPI | ILI9341 Clock |
| **TFT CS** | `IO15` | GPIO / SPI | Display Chip Select |
| **TFT DC** | `IO2` | GPIO | Display Command / Data |
| **TFT Backlight** | `IO21` | GPIO | Active HIGH Backlight control |
| **Touch CS** | `IO33` | VSPI | XPT2046 Touch Controller Chip Select |
| **Touch IRQ** | `IO36` | GPIO | XPT2046 Touch Interrupt |
| **HY2.0 SDA** | `IO27` | I2C Data | External Crowtail / Grove I2C Data line |
| **HY2.0 SCL** | `IO22` | I2C Clock | External Crowtail / Grove I2C Clock line |
| **RGB LED (Red)** | `IO4` | GPIO | Active LOW Alarm Exceeded Indicator |
| **RGB LED (Green)** | `IO16` | GPIO | Active LOW Nominal Status Indicator |
| **RGB LED (Blue)** | `IO17` | GPIO | Active LOW I2C Bus Scan Activity |
| **Piezo Speaker** | `IO26` | DAC / PWM | Threshold alert beeper and touch tone |
| **LDR Photocell** | `IO34` | ADC1_CH6 | Onboard ambient light sensor |

---

## Features

1. **Automated I2C Bus Scanner & Chip Identification**:
   - Scans full I2C address range (`0x01` to `0x7E`).
   - Visual 16x8 address matrix highlighting active peripherals in bright green.
   - Built-in identification for popular sensors: SHT3x, BMP280, BME280, AHT10/AHT20, BH1750, MPU6050, INA219, and SCD40.
2. **Real-Time Rolling Oscilloscope Strip Chart**:
   - 64-point historical ring buffer with auto-scaling dynamic range.
   - Sub-division grid lines with live min, max, peak-to-peak, and moving-average calculations.
   - Run/Pause capture mode to freeze waveforms for analysis.
3. **Threshold Alarm System**:
   - User-adjustable trigger limits with touch `+100` / `-100` step buttons.
   - Multi-tiered alerting: Green (Nominal), Amber (Warning at 85%), Red (Critical threshold exceeded).
   - High-pitch acoustic beeper alert on IO26.
4. **Dual Channel Input**:
   - Seamless switching between internal hardware telemetry (LDR lux photocell) and external HY2.0 I2C analog/digital sensor streams.

---

## Quick Start & Build Instructions

### PlatformIO
1. Open this directory in VS Code or Terminal:
   ```bash
   cd elecrow_projects/48-modular-sensor-workbench
   ```
2. Build firmware:
   ```bash
   pio run
   ```
3. Upload to CrowPanel 2.8" CYD via USB-C:
   ```bash
   pio run --target upload
   ```

---

## Usage Guide

1. **Tab Navigation**: Tap the top tabs (**CHARTS**, **I2C BUS**, **ALARMS**) with a stylus or finger.
2. **Charts View**:
   - Tap **CH: LDR / CH: AUX** to switch active data channel.
   - Tap **PAUSE** to freeze the live oscilloscope trace for detailed inspection.
   - Tap **RESET** to reset historical minimum and maximum metrics.
3. **I2C Bus View**:
   - Plug any 3.3V I2C Crowtail or Grove sensor into the HY2.0 connector.
   - Tap **RE-SCAN BUS** to detect the address and query the identification table.
4. **Alarms View**:
   - Tap **-100** or **+100** to set your warning threshold. When values exceed this limit, the unit sounds a loud alarm and flashes the red RGB LED.

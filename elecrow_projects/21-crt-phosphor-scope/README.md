# Project 21: Retro CRT Phosphor Bench Oscilloscope

A portable analog waveform visualizer with simulated decaying phosphor trails built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
This compact bench instrument transforms the ESP32 into a laboratory oscilloscope with classic green phosphor CRT tube aesthetics. Featuring high-speed ADC sampling on GPIO 34, an edge-triggering engine, decaying persistence buffers, and an onboard 1kHz square-wave calibration reference on GPIO 27, it calculates peak-to-peak voltage (Vpp), average voltage (Vavg), and signal frequency in real time.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **Analog Input**: GPIO 34 (12-bit ADC, 0–3.3V input range)
- **Calibration PWM Output**: GPIO 27 (1 kHz square wave, 50% duty cycle)
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for touch click confirmation

## Features
- **Simulated CRT Phosphor Display**: Glowing electric-green trace with simulated decaying afterglow trails plotted against an 8x6 division laboratory graticule.
- **Hardware Edge Trigger Lock**: Rising-edge threshold detector locks periodic waveforms firmly in place without horizontal jitter.
- **Automated Metric Suite**: Continuously computes Vpp (peak-to-peak), Vavg (DC average), and signal frequency from zero-crossings.
- **Touch-Adjustable Timebase & Amplitude**:
  - Timebase: 0.5ms, 1.0ms, 2.0ms, 5.0ms / division
  - Amplitude: 200mV, 500mV, 1.0V / division
- **Run / Hold Trace Freeze**: Single-tap trace capture freeze allowing static waveform inspection.
- **Hardware State LED**:
  - Green: Trigger locked and running
  - Yellow: Free-running / searching trigger
  - Blue: Trace frozen / stopped

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups


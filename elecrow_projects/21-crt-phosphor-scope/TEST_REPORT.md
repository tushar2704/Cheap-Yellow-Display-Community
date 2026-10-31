# Test Report: Project 21 - Retro CRT Phosphor Bench Oscilloscope

## System Information
- **Target Hardware**: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
- **Target Channel**: IamNotIntoTech (`@IamNotIntoTech`)
- **Author**: Tushar Aggarwal (tushar2704)
- **Date**: October 31, 2026

## Touch Controller & Coordinate Verification
- **Touch Controller**: XPT2046 over dedicated VSPI (`MOSI=32`, `MISO=39`, `CLK=25`, `CS=33`)
- **Calibration Mapping**:
  - `screenX = map(p.x, 240, 3800, 0, 320)`
  - `screenY = map(p.y, 240, 3800, 0, 240)` (Non-inverted Y axis)
- **Debounce Threshold**: 180 ms minimum lockout with pressure detection `p.z > 50`
- **Bottom Touch Controls (screenY >= 195)**:
  - `TIME/DIV`: `screenX < 80`
  - `VOLT/DIV`: `screenX = 80..159`
  - `RUN / HOLD`: `screenX = 160..239`
  - `1kHz CAL`: `screenX >= 240`

## Signal Acquisition & Calibration Generator
- **ADC Pin**: GPIO 34 (12-bit, 0–4095)
- **Calibration PWM**: GPIO 27 (1000 Hz, 50% duty cycle on LEDC channel 0)
- **Buffer Size**: 240 samples with dual-frame decay trail

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)

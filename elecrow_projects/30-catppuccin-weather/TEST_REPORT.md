# Test Report: Project 30 - Catppuccin Weather Station

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
- **Bottom Tab Navigation (screenY >= 200)**:
  - Tab 0 (Now): `screenX < 106`
  - Tab 1 (24H Trend): `screenX = 106..212`
  - Tab 2 (7-Day Outlook): `screenX >= 213`

## Backlight & Ambient Sensing Verification
- LDR ADC reading on GPIO 34 evaluated every 250 ms
- Hardware LEDC PWM configured on timer channel 0, frequency 5 kHz, resolution 8-bit
- Smooth moving-average brightness filtering between 25 and 255 PWM duty cycle

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Build Status**: PlatformIO build clean (Success code 0)

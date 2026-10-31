# Test Report: Project 18 - ESP-NOW Marine Instrument Repeater

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
- **Bottom Touch Buttons (screenY >= 195)**:
  - Metric / Nautical toggle: `screenX < 105`
  - Alarm Mute: `screenX = 110..209`
  - Night / Day Palette: `screenX >= 215`

## Protocol & Radio Verification
- **Protocol**: ESP-NOW peer-to-peer 2.4GHz
- **Payload**: Custom `MarinePacket` struct (24 bytes) containing wind speed, angle, pressure, depth, water temperature, and battery telemetry.
- **Failover**: Automatic simulation loop active when physical transducer nodes are out of range.

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Networking**: WiFi.h + esp_now.h

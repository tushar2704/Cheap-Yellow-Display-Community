# Test Report: Project 20 - Privacy Shield BLE Tracker Detector

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
  - `SCAN`: `screenX < 105`
  - `RADAR / TAG LIST`: `screenX = 110..209`
  - `GEIGER ON / MUTED`: `screenX >= 215`

## BLE Radio & Detection Verification
- **Radio**: BLE Passive Scanning (`BLEScan`)
- **Manufacturer Signatures**:
  - Apple Find My / AirTag (0x004C, Type 0x12)
  - Samsung Galaxy SmartTag (0x0075)
  - Tile Tracker (0x00D6 / UUID 0xFEED)
- **Partition Scheme**: `min_spiffs.csv` (1.9MB app space)

## Compilation Verification
- **Framework**: Arduino ESP32 core
- **Display Driver**: TFT_eSPI (ILI9341_2)
- **Touch Driver**: XPT2046_Touchscreen (polling mode)
- **Bluetooth**: BLEDevice, BLEScan


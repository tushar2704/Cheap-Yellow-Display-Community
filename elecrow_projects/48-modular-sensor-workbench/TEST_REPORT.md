# Test Report: Project 48 - Modular Sensor Workbench & I2C Lab Monitor

## System Information
- **Target Hardware**: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
- **Target Channel**: IamNotIntoTech (`@IamNotIntoTech`)
- **Author**: Tushar Aggarwal (tushar2704)
- **Station Anchor**: Paris (Eiffel Tower - 48.8584° N, 2.2945° E)
- **Date**: October 31, 2026

## Modular Lab Instruments & Bus Monitor
1. **Real-time Strip Chart Oscilloscope (`CHARTS` Tab)**:
   - High-speed 64-sample rolling FIFO waveform visualizer with dynamic auto-scaling.
   - Dual-channel switching:
     - Channel 0: Real onboard LDR Photocell (`IO34` ADC1) measuring ambient illuminance.
     - Channel 1: HY2.0 external auxiliary analog/digital input signal.
   - Live telemetry: Current value, Min / Max markers, and rolling average readout.
   - Run / Pause freeze-frame toggle and hardware min/max reset.
2. **I2C Bus Address Analyzer (`I2C BUS` Tab)**:
   - Complete 128-address matrix scan (0x00 to 0x7F) over HY2.0 I2C bus (`IO27`: SDA, `IO22`: SCL).
   - Real-time address decoding and identification (BH1750, AHT10/20, INA219, SHT3x, SCD4x, MPU6050, BMP280, BME680).
   - Visual 16x8 green / slate grid status matrix with on-demand bus rescan.
3. **Alarm Threshold Engine (`ALARMS` Tab)**:
   - Configurable alert trigger thresholds with +/-100 step buttons.
   - Dual-state visual status banner (Normal vs. Threshold Exceeded).

## Touch Controller & Coordinate Verification
- **Touch Controller**: XPT2046 over dedicated VSPI (`MOSI=32`, `MISO=39`, `CLK=25`, `CS=33`)
- **Calibration Mapping**:
  - `screenX = map(p.x, 240, 3800, 0, 320)`
  - `screenY = map(p.y, 240, 3800, 0, 240)` (Non-inverted Y axis)
- **Interactive Touch Regions**:
  - Top Tabs (`screenY <= 26`): `CHARTS` (`screenX in [6, 100]`), `I2C BUS` (`screenX in [106, 200]`), `PARIS LAB / ALARMS` (`screenX in [206, 314]`).
  - Chart Controls (`screenY >= 214`): Channel toggle (`[6, 96]`), Pause/Run (`[102, 182]`), Reset (`[188, 248]`).
  - Alarm Controls: Step `-100` (`[200, 246]`), Step `+100` (`[254, 300]`).

## Acoustic & Visual Hardware Feedback
- **Acoustic Synthesizer (GPIO 26)**:
  - 1800 Hz tab transition chirp, 2100 Hz bus scan chime, 2600 Hz threshold breach alarm tone.
- **Active-Low RGB LED (GPIOs 4/16/17)**:
  - Normal state: Green.
  - Warning margin (>= 85% limit): Yellow/Amber.
  - Alarm threshold breach: Pulsating Red.

## Wi-Fi & Station Anchor Integration
- **Connection Mode**: 2.5-second non-blocking station attempt to local AP.
- **Tab Header**: Dynamically labels Tab 3 as "PARIS LAB" when connected to network.

## Compilation & Memory Footprint
- **Platform**: Espressif 32 (v7.0.1) / ESP32 Dev Module
- **RAM Usage**: 45,772 bytes (14.0% of 327,680 bytes)
- **Flash Usage**: 816,525 bytes (62.3% of 1,310,720 bytes)
- **Build Status**: 0 errors, 0 warnings (SUCCESS)

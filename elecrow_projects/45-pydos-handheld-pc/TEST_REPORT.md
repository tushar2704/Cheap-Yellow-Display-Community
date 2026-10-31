# Test Report: Project 45 - PyDOS Handheld Computer

## System Information
- **Target Hardware**: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
- **Target Channel**: IamNotIntoTech (`@IamNotIntoTech`)
- **Author**: Tushar Aggarwal (tushar2704)
- **Station Anchor**: Paris (Eiffel Tower - 48.8584° N, 2.2945° E)
- **Date**: October 31, 2026

## PyDOS Shell & Virtual Keyboard Architecture
- **Console Display**: 9-line retro amber phosphor typography (`#FD20`) over deep black background with automatic line scrolling and blinking block cursor.
- **Virtual Keyboard**: 4-row touch QWERTY layout:
  - Row 0: Numbers `0-9`, `-`, `=`
  - Row 1: `Q` through `P`
  - Row 2: `A` through `L` + dedicated `CLS` screen clear key
  - Row 3: `Z` through `M`, `SPC` spacebar, `BK` backspace, and `ENT` return key
  - Row 4: Instant preset command buttons (`DIR`, `MEM`, `NET`, `HELP`)
- **Interactive Command Suite**:
  - `DIR` / `LS`: Displays root volume file directory with sizes.
  - `VER`: Displays operating system version, author, and Paris edition badge.
  - `MEM`: Queries real-time ESP32 heap metrics (`ESP.getFreeHeap()`, `getMinFreeHeap()`).
  - `TIME` / `DATE`: Reports real-time Paris NTP clock timestamp.
  - `NET` / `IP`: Reports live Wi-Fi SSID, RSSI signal strength, local IP address, and Paris coordinates (`48.8584° N, 2.2945° E`).
  - `SCAN`: Executes 802.11 b/g/n RF scan listing nearby access points.
  - `CLS`: Clears the terminal screen buffer.
  - `BEEP`: Emits acoustic test beep on GPIO 26.
  - `LED <R|G|B|OFF>`: Actuates RGB hardware on GPIOs 4/16/17.
  - `ECHO <text>`: Echoes user string into terminal buffer.
  - `PY <expr>`: MicroPython evaluation prompt placeholder.

## Touch Controller & Coordinate Verification
- **Touch Controller**: XPT2046 over dedicated VSPI (`MOSI=32`, `MISO=39`, `CLK=25`, `CS=33`)
- **Calibration Mapping**:
  - `screenX = map(p.x, 240, 3800, 0, 320)`
  - `screenY = map(p.y, 240, 3800, 0, 240)` (Non-inverted Y axis)
- **Debounce & Response**:
  - 180 ms debounce filter per keystroke with immediate acoustic keyclick.

## Acoustic & Visual Hardware Feedback
- **Acoustic Synthesizer (GPIO 26)**:
  - 2600 Hz keyclick sound per key touch.
  - 2400 Hz beep command response.
- **Active-Low RGB LED (GPIOs 4/16/17)**:
  - Green command execution flash on return key.
  - Controllable via `LED` shell command.

## Wi-Fi & Station Anchor Integration
- **Connection Mode**: 2.5-second non-blocking station attempt to local AP.
- **NTP Time Sync**: Paris timezone (`CET-1CEST,M3.5.0,M10.5.0/3`).

## Compilation & Memory Footprint
- **Platform**: Espressif 32 (v7.0.1) / ESP32 Dev Module
- **RAM Usage**: 47,564 bytes (14.5% of 327,680 bytes)
- **Flash Usage**: 823,609 bytes (62.8% of 1,310,720 bytes)
- **Build Status**: 0 errors, 0 warnings (SUCCESS)

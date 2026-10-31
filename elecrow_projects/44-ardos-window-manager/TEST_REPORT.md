# Test Report: Project 44 - ArdOS Retro Window Manager

## System Information
- **Target Hardware**: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
- **Target Channel**: IamNotIntoTech (`@IamNotIntoTech`)
- **Author**: Tushar Aggarwal (tushar2704)
- **Station Anchor**: Paris (Eiffel Tower - 48.8584° N, 2.2945° E)
- **Date**: October 31, 2026

## ArdOS Retro Desktop Architecture
- **Desktop Environment**: Classic Win95 / 90s OS aesthetic featuring Win95 teal desktop (`#008080`), 3D beveled silver-gray chrome, navy active titlebars, and dark slate inactive titlebars.
- **Window Management Engine**:
  - Overlapping multi-window z-ordering (active window rendered cleanly on top).
  - Smooth titlebar dragging with clamped bounds inside the 320x216 desktop workspace.
  - Close button `[X]` on titlebar right edge.
- **Interactive Taskbar**:
  - 3D sunken/raised `[Start]` menu button with popup menu hierarchy.
  - Active application task tabs.
  - Live system tray clock synchronized to Paris NTP time (`HH:MM`).
- **3 Built-in Desktop Applications**:
  1. **Notepad (`MEMO.TXT`)**: Displays system architecture, memory footprint, channel branding, and Paris Wi-Fi network status.
  2. **Calculator**: Interactive 4-function touch keypad (`0-9`, `+`, `-`, `*`, `/`, `C`, `=`) with live LCD display.
  3. **Minesweeper**: Classic 6x6 minefield with smiley face reset button, safe neighbor count reveals, and mine detonation alerts.

## Touch Controller & Coordinate Verification
- **Touch Controller**: XPT2046 over dedicated VSPI (`MOSI=32`, `MISO=39`, `CLK=25`, `CS=33`)
- **Calibration Mapping**:
  - `screenX = map(p.x, 240, 3800, 0, 320)`
  - `screenY = map(p.y, 240, 3800, 0, 240)` (Non-inverted Y axis)
- **Interactive Touch Regions**:
  - **Taskbar & Start Menu (`screenY >= 216`)**:
    - Start button: `screenX in [3, 55]`
    - App tabs: `screenX >= 60`, 68 px per tab
  - **Start Menu Popup (`screenX in [3, 113], screenY in [104, 214]`)**:
    - Item 0: Notepad, Item 1: Calculator, Item 2: Minesweeper, Item 3: About
  - **Desktop Icons (`screenX in [10, 45]`)**:
    - Double/single tap opens corresponding application.
  - **Window Titlebar Dragging**:
    - Tap and drag on titlebar moves window dynamically across desktop.

## Acoustic & Visual Hardware Feedback
- **Acoustic Synthesizer (GPIO 26)**:
  - 2200 Hz click on Start button, 2000 Hz / 2400 Hz app window launch, 2600 Hz safe mine reveal, 800 Hz mine detonation.
- **Active-Low RGB LED (GPIOs 4/16/17)**:
  - Green LED illuminates when Start Menu is active.
  - Red LED pulses upon Minesweeper explosion.

## Wi-Fi & Station Anchor Integration
- **Connection Mode**: 2.5-second non-blocking station attempt to local AP.
- **Clock Sync**: Synchronizes to Paris time (`pool.ntp.org` / `time.google.com`) and updates taskbar tray.
- **Notepad Telemetry**: Reports "Anchor: Paris (Eiffel Tower)" and live network status.

## Compilation & Memory Footprint
- **Platform**: Espressif 32 (v7.0.1) / ESP32 Dev Module
- **RAM Usage**: 46,544 bytes (14.2% of 327,680 bytes)
- **Flash Usage**: 813,769 bytes (62.1% of 1,310,720 bytes)
- **Build Status**: 0 errors, 0 warnings (SUCCESS)

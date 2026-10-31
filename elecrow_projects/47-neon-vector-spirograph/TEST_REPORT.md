# Test Report: Project 47 - Neon Vector Spirograph & Mandala Generator

## System Information
- **Target Hardware**: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
- **Target Channel**: IamNotIntoTech (`@IamNotIntoTech`)
- **Author**: Tushar Aggarwal (tushar2704)
- **Station Anchor**: Paris (Eiffel Tower - 48.8584° N, 2.2945° E)
- **Date**: October 31, 2026

## Mathematical Vector Engines
1. **Hypotrochoid Spirograph Engine**:
   - Parametric formula:
     $$\begin{cases} x(\theta) = (R - r)\cos(\theta) + d\cos\left(\frac{R - r}{r}\theta\right) \\ y(\theta) = (R - r)\sin(\theta) - d\sin\left(\frac{R - r}{r}\theta\right) \end{cases}$$
   - Continuous vector tracing with 4 steps per frame for smooth kinematics.
   - 5 built-in mathematical presets (Hypotrochoid 8P, Complex Star 13, Harmonic Rose, Double Bloom, Classic Trifoil).
   - 4 Dynamic Chroma Palettes: Neon Rainbow, Cyberpunk, Fire, Emerald.
2. **Rotational Symmetry Mandala Touch Canvas**:
   - User draws with stylus / finger directly on touchscreen.
   - Real-time n-fold rotational reflection matrix (4x, 6x, 8x, 12x symmetry).
   - Dynamic time-progressed chroma shifting along user stroke vectors.

## Touch Controller & Coordinate Verification
- **Touch Controller**: XPT2046 over dedicated VSPI (`MOSI=32`, `MISO=39`, `CLK=25`, `CS=33`)
- **Calibration Mapping**:
  - `screenX = map(p.x, 240, 3800, 0, 320)`
  - `screenY = map(p.y, 240, 3800, 0, 240)` (Non-inverted Y axis)
- **Interactive Controls**:
  - Mode Switch (`screenX >= 195, screenY <= 24`): Toggles between Auto Spirograph and Mandala Mode.
  - Preset Selector (`screenX in [6, 70], screenY >= 215`): Cycles through 5 mathematical presets.
  - Palette Selector (`screenX in [74, 142], screenY >= 215`): Cycles color maps.
  - Clear Button (`screenX in [146, 206], screenY >= 215`): Flushes vector canvas.
  - Symmetry Fold (`screenX in [6, 76], screenY >= 215` in Mandala Mode): Cycles 4x, 6x, 8x, 12x folds.

## Acoustic & Visual Hardware Feedback
- **Acoustic Synthesizer (GPIO 26)**:
  - 1750 Hz startup chime, 2200 Hz mode toggle, 1400 Hz preset step, 1800 Hz palette cycle, 800 Hz canvas clear.
- **Active-Low RGB LED (GPIOs 4/16/17)**:
  - Continuous HSV chromatic cycle synchronized with vector pen progress.

## Wi-Fi & Station Anchor Integration
- **Connection Mode**: 2.5-second non-blocking station attempt to local AP.
- **Header Display**: Displays "@IamNotIntoTech | Paris" upon successful connection.

## Compilation & Memory Footprint
- **Platform**: Espressif 32 (v7.0.1) / ESP32 Dev Module
- **RAM Usage**: 44,592 bytes (13.6% of 327,680 bytes)
- **Flash Usage**: 793,421 bytes (60.5% of 1,310,720 bytes)
- **Build Status**: 0 errors, 0 warnings (SUCCESS)

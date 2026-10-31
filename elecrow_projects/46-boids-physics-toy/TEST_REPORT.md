# Test Report: Project 46 - Boids Artificial Life & 4D Tesseract Sandbox

## System Information
- **Target Hardware**: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
- **Target Channel**: IamNotIntoTech (`@IamNotIntoTech`)
- **Author**: Tushar Aggarwal (tushar2704)
- **Station Anchor**: Paris (Eiffel Tower - 48.8584° N, 2.2945° E)
- **Date**: October 31, 2026

## Physics & Simulation Engines
1. **Boids Artificial Flocking Life**:
   - 42 autonomous boids computing Craig Reynolds' flocking principles: Separation, Alignment, and Cohesion in real-time.
   - Dynamic boundary soft-steering avoiding display edges.
   - Interactive Touch Beacons:
     - `FOOD (ATTRACT)`: Boids accelerate toward touch target with green beacon indicator.
     - `HAWK (REPEL)`: Boids flee from touch target like a predator with red beacon indicator.
     - `SCATTER`: Re-scatters boid positions across workspace.
2. **4D Hypercube (Tesseract) Mathematical Projection**:
   - 16 4D vertices undergoing continuous rotational transformations across XW, YZ, and XY hyper-planes.
   - Dual-stage perspective transformation: 4D to 3D, then 3D to 2D screen coordinates.
   - 32 projected wireframe edges rendered with multi-color depth shading.
   - Interactive touch drag dynamically tilts the hyper-rotation angles.
   - Speed + / - controls and rotational axis jump features.
   - Real-time hardware FPS performance monitor.

## Touch Controller & Coordinate Verification
- **Touch Controller**: XPT2046 over dedicated VSPI (`MOSI=32`, `MISO=39`, `CLK=25`, `CS=33`)
- **Calibration Mapping**:
  - `screenX = map(p.x, 240, 3800, 0, 320)`
  - `screenY = map(p.y, 240, 3800, 0, 240)` (Non-inverted Y axis)
- **Interactive Regions**:
  - Top-Right Switch (`screenX >= 195, screenY <= 24`): Toggles between Boids and 4D Tesseract.
  - Bottom Bar (`screenY >= 215`): Mode-specific controls.
  - Simulation Canvas (`24 < screenY < 215`): Touch beacon placement / rotation tilting.

## Acoustic & Visual Hardware Feedback
- **Acoustic Synthesizer (GPIO 26)**:
  - 1800 Hz food beacon chime, 800 Hz predator alarm, 1200 Hz scatter sound, 2400 Hz mode toggle chirp.
- **Active-Low RGB LED (GPIOs 4/16/17)**:
  - Green for Attract Mode, Red for Predator Mode, pulsating Blue/Red during Tesseract rotation.

## Wi-Fi & Station Anchor Integration
- **Connection Mode**: 2.5-second non-blocking station attempt to local AP.
- **Header Display**: Displays "@IamNotIntoTech | Paris" upon successful connection.

## Compilation & Memory Footprint
- **Platform**: Espressif 32 (v7.0.1) / ESP32 Dev Module
- **RAM Usage**: 46,100 bytes (14.1% of 327,680 bytes)
- **Flash Usage**: 795,533 bytes (60.7% of 1,310,720 bytes)
- **Build Status**: 0 errors, 0 warnings (SUCCESS)

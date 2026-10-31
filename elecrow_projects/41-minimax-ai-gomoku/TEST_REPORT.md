# Test Report: Project 41 - Minimax AI Gomoku & Tactical Board

## System Information
- **Target Hardware**: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
- **Target Channel**: IamNotIntoTech (`@IamNotIntoTech`)
- **Author**: Tushar Aggarwal (tushar2704)
- **Station Anchor**: Paris (Eiffel Tower - 48.8584° N, 2.2945° E)
- **Date**: October 31, 2026

## Minimax AI Heuristic & Board Engine
- **Board Grid**: 10x10 bamboo-wood tactile board with precision grid lines.
- **Ruleset**: Standard Gomoku (Five-in-a-Row).
- **Search Engine**:
  - Embedded heuristic search with dynamic depth level (Novice: 2-ply, Master: 3-ply).
  - Immediate win detection for AI (5-in-a-row completion).
  - Immediate defensive block detection against human player winning threats.
  - Neighbor-guided candidate move pruning for low-latency response.
  - Position advantage evaluation meter reflecting tactical board dominance (-100 to +100).

## Touch Controller & Coordinate Verification
- **Touch Controller**: XPT2046 over dedicated VSPI (`MOSI=32`, `MISO=39`, `CLK=25`, `CS=33`)
- **Calibration Mapping**:
  - `screenX = map(p.x, 240, 3800, 0, 320)`
  - `screenY = map(p.y, 240, 3800, 0, 240)` (Non-inverted Y axis)
- **Interactive Touch Regions**:
  - **Board Stone Placement (`screenX < 210, screenY in [30, 230]`)**:
    - `c = (screenX - BOARD_X) / CELL_SIZE`
    - `r = (screenY - BOARD_Y) / CELL_SIZE`
    - Validates grid cell bounds [0..9] and rejects occupied cells.
  - **Difficulty Toggle (`screenX in [224, 310], screenY in [146, 178]`)**:
    - Toggles between Novice (2-ply) and Master (3-ply).
  - **New Game Button (`screenX in [224, 310], screenY in [184, 228]`)**:
    - Clears board, resets last move indicators, resets turn order.

## Acoustic & Visual Hardware Feedback
- **Acoustic Synthesizer**: 1800 Hz click on stone drop, 2800 Hz chime on AI response, and 3200 Hz victory fanfare on GPIO 26 buzzer.
- **Visual Feedback**:
  - Active-low Blue LED pulse during AI thinking phase on GPIOs 4/16/17.
  - Green LED for Human victory, Red LED for AI victory.
  - Red ring highlighting the most recent move on the board.

## Wi-Fi & Station Anchor Integration
- **Connection Mode**: 2.5-second non-blocking station attempt to local AP.
- **Status Display**: Real-time "PARIS ONLINE" or "PARIS (OFFLINE)" status badge on the AI telemetry card.

## Compilation & Memory Footprint
- **Platform**: Espressif 32 (v7.0.1) / ESP32 Dev Module
- **RAM Usage**: 44,660 bytes (13.6% of 327,680 bytes)
- **Flash Usage**: 790,573 bytes (60.3% of 1,310,720 bytes)
- **Build Status**: 0 errors, 0 warnings (SUCCESS)

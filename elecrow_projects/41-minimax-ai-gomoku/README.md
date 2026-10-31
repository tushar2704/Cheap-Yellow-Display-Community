# Project 41: Minimax AI Gomoku & Tactical Board

A strategic desktop AI tabletop terminal running an embedded Minimax search engine engineered for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
Challenge an embedded AI opponent to the ancient board game of Gomoku (Five-in-a-Row) directly on the CYD screen. Featuring a realistic bamboo wood board, precision stylus stone placement, real-time AI thought telemetry, an advantage evaluation bar, and tactical search with automated defense blocking, this project demonstrates real-time algorithmic game theory running on local ESP32 hardware.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB AI Thought LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue) glowing blue while AI calculates tactical responses
- **Buzzer**: Piezo buzzer on GPIO 26 for acoustic stone snap audio and victory chimes

## Features
- **10x10 Tactical Board**:
  - High-contrast bamboo wood grid rendering with distinct black (human) and white (AI) stones.
  - Red ring highlighting the opponent's most recent stone placement.
- **Embedded Minimax Intelligence**:
  - Pattern evaluation engine with immediate 4-in-a-row defense and lethal attack execution.
  - Center-board proximity scoring for strong positional opening plays.
- **Real-Time AI Telemetry Sidebar**:
  - Search Depth: Shows active search ply depth (Novice vs Master).
  - Node Counter: Real-time counter of board state branches analyzed per turn.
  - Eval Meter: Bipolar evaluation bar showing match advantage (+AI Red / +Human Green).
- **Interactive Touch Deck**:
  - Direct Board Cell Touch: Accurate stylus or finger touch to drop stones.
  - Difficulty Toggle: Switch between fast Novice and deep Master AI heuristics.
  - New Game Trigger: Clears the board and resets search statistics.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups


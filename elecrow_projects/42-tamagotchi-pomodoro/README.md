# Project 42: Tamagotchi Pomodoro Pet

An interactive virtual desktop companion and productivity timer engineered for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
Keep your productivity on track with a virtual desk companion whose growth and well-being are directly linked to your study sessions. Featuring the classic 25/5/15 Pomodoro interval framework, this appliance rewards completed focus sprints with XP points, allowing your pixel companion to evolve from a baby creature into an advanced winged form while providing playful tactile touch interactions.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Mood LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue) reflecting focus state (Focus Red, Break Green, Sleep Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for purring sounds, feeding clicks, and session fanfares

## Features
- **Integrated Pomodoro Productivity Engine**:
  - 25-minute Work Focus sprint countdown.
  - 5-minute Short Break recovery.
  - 15-minute Long Break interval automatically scheduled every 4 completed sessions.
  - Streak tracking counter.
- **Animated Cyber Pet Life Cycle**:
  - Animated pixel sprite featuring blinking, floor hopping, speech bubble reactions, and sleeping curled animations.
  - Level 2 Evolution: Unlocks golden horns and wings upon reaching 100 XP from focus sessions.
- **Pet Vitality Dashboard**:
  - Health Points (HP)
  - Joy / Happiness Meter
  - Food / Satiety Level
- **Tactile Touch Toolbar**:
  - `START / PAUSE`: Toggle countdown timer.
  - `FEED`: Offer snacks to replenish satiety and happiness.
  - `PET ME`: Pet your companion to trigger happy purr audio sweeps.
  - `SLEEP / WAKE UP`: Put your pet to sleep with calm blue LED mood lighting.
  - Direct Touch: Tap inside the habitat to pat the creature.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups


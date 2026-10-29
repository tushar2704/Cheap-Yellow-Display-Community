# Project 42: Tamagotchi Pomodoro Pet

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **Tamagotchi Pomodoro Pet** turns the Elecrow 2.8" CYD into an adorable productivity desk companion whose health, happiness, and evolution are linked to your daily focus habits. Designed around the classic Pomodoro Technique (25-minute Work Focus, 5-minute Short Break, and 15-minute Long Break after 4 completed sprints), it rewards focused work sprints with creature XP, evolution stages, and playful interactions.

Equipped with animated pixel creature behaviors (blinking, hopping, eating, and sleeping), touch feeding and petting controls, acoustic audio feedback, and ambient Pomodoro state RGB lighting, it makes deep study and coding sessions fun and rewarding.

---

## Features

- **Pomodoro Productivity State Machine**:
  - **Work Focus (25 min)**: Intense coral red timer card with do-not-disturb blue RGB mood lighting.
  - **Short Break (5 min)**: Refreshing emerald green timer card encouraging creature stretching.
  - **Long Break (15 min)**: Extended rest reward awarded after 4 completed Pomodoro rounds.
- **Animated Pixel Creature Simulation**:
  - Real-time idle hopping, procedural eye blinking, eating animations, and curling up into sleeping poses.
  - 3 Evolution Life Tiers: Baby Creature -> Cyber Dino -> Cyber Dragon (unlocked through cumulative study XP).
- **Four Real-Time Vital Stat Gauges**:
  - **Health %**: Maintained by completing Pomodoro sprints without early cancellation.
  - **Joy / Happiness %**: Enhanced by petting and feeding.
  - **Food / Hunger %**: Gradually increases during work sessions; replenished by feeding virtual snacks.
  - **Experience (XP) %**: Progress towards the next creature evolution milestone.
- **Interactive Touch Actions**:
  - **START / PAUSE**: Controls the Pomodoro countdown timer.
  - **FEED**: Drops snacks to satisfy creature hunger with audio crunch chirps.
  - **PET**: Triggers happy purr frequency sweeps on `GPIO 26`.
  - **SLEEP / WAKE**: Toggles rest mode for nightstand placement.
- **RGB Mood Synchronization**:
  - Solid Blue for Focus mode, Emerald Green for Break time, and flashing Red for neglected pet alerts.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **Pomodoro Mood RGB LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW focus/break/distress indicator |
| **Pet Acoustic Buzzer** | `GPIO 26` | Piezo sound synthesizer for purrs, bites, and chimes |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/42-tamagotchi-pomodoro
   pio run --target upload
   ```
3. Tap `[START]` to begin your first 25-minute Pomodoro study block and watch your companion grow.

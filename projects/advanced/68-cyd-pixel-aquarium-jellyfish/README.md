# 🪼 Project 68 — CYD Denki-Kurage Pixel Jellyfish & Desktop Aquarium

> **A bioluminescent Japanese *Denki-Kurage* (electric jellyfish) desktop digital marine aquarium for the Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch). Features procedural marine locomotion physics, 6 flowing elastic tentacles with inverse kinematics, interactive touch water ripples, sinking plankton food feeding, 5 selectable bioluminescent chroma palettes, Day vs. Abyss lighting modes, aerator bubble simulation, and hydrophone audio chimes!**

[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/)
[![Display: ILI9341](https://img.shields.io/badge/Display-ILI9341_2.8%22-yellow.svg)](https://platformio.org/)
[![Touch: XPT2046](https://img.shields.io/badge/Touch-XPT2046_Resistive-red.svg)](https://platformio.org/)
[![Brand: IamNotIntoTech](https://img.shields.io/badge/Branded-IamNotIntoTech-FEA000.svg)](https://youtube.com/@IamNotIntoTech)

---

### 📺 Watch the Masterclass & 3D Enclosure
- 🎬 **Tutorial & Code Walkthrough**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printable Desktop Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🌟 Key Features

1. **Procedural Jellyfish Marine Locomotion**:
   - Realistic rhythmic bell undulation: contracts to generate upward hydrodynamic thrust, then slowly drifts downwards.
   - 6 Multi-joint elastic tentacles trailing behind with inverse spring physics.
   - Central electric organ core pulsing with rhythmic bio-energy.
2. **Interactive Touch Ecosystem**:
   - **Water Shockwaves**: Tap anywhere in the tank to trigger circular shockwaves that physically push the jellyfish away.
   - **Feeding System**: Tap `[FEED]` to release floating bioluminescent plankton pellets. The jellyfish senses the food and consumes it with ascending harp chimes!
   - **5 Bioluminescent Chroma Palettes**: Tap `[CHROMA]` to cycle through:
     1. *Electric Cyan* (Deep ocean electric blue)
     2. *Neon Bio-Pink* (Luminous radioactive magenta)
     3. *Emerald Phosphor* (Bioluminescent green algae glow)
     4. *Deep-Sea Amber* (Warm benthic volcanic vent gold)
     5. *Ghost Spectral White* (Ethereal translucent deep abyss)
   - **Lighting Mode Selector**: Tap `[LIGHTS]` to switch between Azure Ocean Day mode (sunlight caustics) and Midnight Abyss mode (pitch-black abyss where only the jellyfish glow).
3. **Ambient Marine Physics**:
   - Continuous rising aerator bubbles with procedural wobble from the bottom gravel bed.
   - Deep-water sunlight caustic rays penetrating through the water surface.
4. **Hardware Audio & Lighting**:
   - Piezo buzzer on `GPIO 26` synthesizes hydrophone bubble pops, jet pulses, and feeding chimes.
   - Active-low RGB LED on `GPIO 4, 16, 17` pulses ambient bias lighting in sync with the jellyfish's bell contractions.
   - LDR light sensor on `GPIO 34` detects dark rooms for auto night-mode transitions.

---

## 🔌 Hardware Pinout (ESP32-2432S028R)

| Component | Pin Function | ESP32 GPIO |
|---|---|---|
| **ILI9341 Display** | MOSI / MISO / SCLK / CS / DC | `GPIO 13` / `GPIO 12` / `GPIO 14` / `GPIO 15` / `GPIO 2` |
| **Backlight PWM** | TFT Backlight Enable | `GPIO 21` (Active HIGH) |
| **XPT2046 Touch** | CLK / MISO / MOSI / CS / IRQ | `GPIO 25` / `GPIO 39` / `GPIO 32` / `GPIO 33` / `GPIO 36` |
| **RGB LED** | Ambient Bio-Pulse | `GPIO 4` / `GPIO 16` / `GPIO 17` (Active LOW) |
| **Speaker** | Hydrophone Audio | `GPIO 26` |
| **LDR Sensor** | Ambient Light ADC | `GPIO 34` |

---

## 🛠️ How to Compile & Flash

### PlatformIO (Recommended)
```bash
cd projects/advanced/68-cyd-pixel-aquarium-jellyfish
pio run --target upload
```

### Arduino IDE
1. Open `68-cyd-pixel-aquarium-jellyfish.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and configure `User_Setup.h` for ILI9341.
4. Install **XPT2046_Touchscreen**.
5. Click **Upload**.

---

<p align="center">
  <b>Built with 🟡 by <a href="https://youtube.com/@IamNotIntoTech">IamNotIntoTech</a></b> &bull; 
  <b>3D Enclosures by <a href="https://youtube.com/@Makerrdesigns">Makerrdesigns</a></b>
</p>

# 💾 Project 23: Multi-Firmware SD Bootloader & OTA Launcher

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **MicroSD Multi-Bootloader Architecture**:
   - Automatically inspects the MicroSD card root directory and detects compiled ESP32 firmware binaries (`.bin`).
   - Enables carrying dozens of full touch projects on a single card and switching between them on-the-go without a computer.
2. **Touchscreen Firmware Selection Menu**:
   - Clean, high-contrast touch list showing binary filenames, byte size in KB, and partition target.
   - Interactive selection highlights and dynamic status reporting.
3. **On-Device OTA Partition Flasher**:
   - Executes ESP32 Over-The-Air (`Update.h`) partition writes directly from SD card SPI blocks.
   - Animated visual progress bar tracking flash block execution percentage (`0%` -> `100%`).
4. **Hardware Multi-Sensory Feedback**:
   - Active-low RGB LED glows Green when ready, pulses Amber during active partition flashing, and triggers Red on verification failure.
   - Ascending audio frequency sweep sounds while flashing, concluding with a celebratory reboot fanfare.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 5, 18, 19, 23** | MicroSD Card SPI | CS, SCK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — Flashing status |
| **GPIO 26** | Audio / Buzzer | Bootloader audio fanfare |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/23-sd-firmware-launcher
pio run -t upload
```

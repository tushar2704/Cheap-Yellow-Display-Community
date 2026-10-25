# Project 25: Smart Electric Meter Optical Pulse Counter & Cost Estimator

A real-time home energy monitor and utility electricity tariff cost estimator for the Cheap Yellow Display (ESP32-2432S028R). Interfacing with DIN-rail electric meters or utility grid meter flashing LEDs (1000 imp/kWh standard) via the onboard LDR or photodiode sensor, it computes active power draw (kW), cumulative energy consumed today (kWh), real-time electricity bills ($/€), and triggers peak-demand overload alarms.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **Real-Time Active Power (kW)**: High-resolution power calculation derived from optical LED pulse intervals ($3600 / \Delta t$).
- **Daily Energy Accumulator (kWh)**: Cumulative meter billing telemetry and pulse verification.
- **Utility Cost Projection**: Instantaneous daily and projected monthly utility electricity costs based on configurable tariff rates.
- **Peak Demand Warning**: Acoustic alarm and red visual alert when household wattage breaches contracted supply limits.
- **Touch Interaction**: Configurable tariff pricing selector ($/kWh) and energy reset controls.

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| Optical Pulse Sensor | 34 | Onboard LDR phototransistor or CN1 pulse probe |
| RGB LED | 4 (R), 16 (G), 17 (B) | Optical pulse flash & overload warning |
| Audio Buzzer | 26 | Peak demand warning buzzer |
| Ambient Light (LDR) | 34 | Night mode auto-dimming |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)

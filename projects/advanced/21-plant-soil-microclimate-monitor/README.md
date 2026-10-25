# Project 21: Plant & Soil Microclimate Greenhouse Station

A precision horticulture telemetry and automated irrigation monitor for the Cheap Yellow Display (ESP32-2432S028R). Features multi-zone capacitive soil moisture bars, real-time Vapor Pressure Deficit ($VPD$ kPa) microclimate modeling, ambient solar illuminance lux estimation via onboard LDR, dry-soil acoustic alarms, and manual irrigation pump override buttons.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **4-Zone Soil Moisture**: Independent percentage fill bars and health indicators for up to four plants.
- **VPD Calculations**: Computes atmospheric Vapor Pressure Deficit ($SVP - AVP$) for greenhouse transpiration management.
- **Microclimate Readouts**: Ambient temperature (°C), relative humidity (% RH), and light intensity (Lux).
- **Irrigation Pump Control**: Touch toggle to activate automated water pump relay for parched zones.
- **Dry Soil Warning**: Low moisture triggers a piezo chime and flashing red alert LED.

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| Soil Sensor Bus / ADC | 35 / 34 | Analog soil probe inputs (CN1 connector) |
| RGB LED | 4 (R), 16 (G), 17 (B) | Plant health status indicator |
| Audio Buzzer | 26 | Parched soil acoustic alert |
| Ambient Light (LDR) | 34 | Light intensity (Lux) / Auto-dimming |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)

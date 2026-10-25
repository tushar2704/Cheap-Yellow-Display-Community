# Project 24: Live Doppler Rain Radar & Animated Satellite Weather Map

A dedicated desktop NEXRAD weather radar terminal and animated precipitation loop station for the Cheap Yellow Display (ESP32-2432S028R). Features multi-frame historical Doppler radar loops (Play/Pause/Step), standard National Weather Service dBZ precipitation reflectivity coloring (Green light rain, Yellow moderate, Red severe, Violet hail core), geographical range rings, and severe cell RGB acoustic alarms.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **NEXRAD Doppler Loops**: 6-frame animated precipitation timeline showing storm movement and cell development.
- **Meteorological Color Scale**: Standard dBZ reflectivity scale (20 to 70 dBZ) for rain rates and thunderstorm detection.
- **Geographical Range Rings**: 45 km and 90 km distance markers centered on regional observation stations.
- **Severe Weather Alarms**: Optical Red LED and buzzer alert when severe storm cores (>55 dBZ) enter the perimeter.
- **Touch Controls**: Interactive Play/Pause loop toggle, frame advance, and map zoom.

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| RGB LED | 4 (R), 16 (G), 17 (B) | Severe storm alert / rain indicator |
| Audio Buzzer | 26 | Thunderstorm proximity alarm |
| Ambient Light (LDR) | 34 | Auto-dimming night mode |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)

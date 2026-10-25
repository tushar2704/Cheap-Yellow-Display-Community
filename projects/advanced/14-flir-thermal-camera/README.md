# Project 14: FLIR / MLX90640 Thermal Imager & Heat Map Scanner

A portable infrared thermal imaging camera station for the Cheap Yellow Display (ESP32-2432S028R). Visualizes real-time thermal matrices, auto-tracks hottest and coldest points with crosshairs, provides selectable false-color palettes (Ironbow, Rainbow, Hot/Cold), and raises acoustic and RGB LED alarms on overheating threshold breaches.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **Thermal Heatmap Matrix**: 32x24 grid rendering with real-time temperature scale normalization.
- **Multiple Color Palettes**: Ironbow (FLIR standard), Rainbow, and High-Contrast Hot/Cold.
- **Min/Max Spot Crosshair**: Dynamic tracking of hottest component and coldest zone.
- **Acoustic & Optical Alarm**: Piezo buzzer warning and RGB status LED indicator.
- **Touch Controls**: Toggle palettes, silence/enable alarms, and recalibrate zero point.

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| I2C SDA / SCL (CN1) | 22 / 27 | External MLX90640 Sensor Interface |
| RGB LED | 4 (R), 16 (G), 17 (B) | Thermal status warning lights |
| Audio Buzzer | 26 | Over-temperature alert |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)

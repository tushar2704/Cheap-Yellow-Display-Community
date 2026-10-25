# Project 17: Solar & Battery Storage MPPT Dashboard

An off-grid renewable energy telemetry terminal and solar charge controller dashboard for the Cheap Yellow Display (ESP32-2432S028R). Features real-time animated power flow (Solar PV -> MPPT -> LiFePO4 Storage -> AC Inverter Load), battery state-of-charge gauge, daily kWh yield tracker, and interactive inverter toggles.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **Live Energy Flow Architecture**: Visual schematic linking Solar Arrays, MPPT converter, LiFePO4 Battery, and AC Inverter.
- **Battery Management Metrics**: Real-time State of Charge (SOC %), pack voltage, net charge/discharge current, and battery temperature.
- **PV Generation Statistics**: Instantaneous solar wattage, array voltage, string amperage, and cumulative daily yield ($kWh$).
- **Touch Interaction**: Inverter relay power switch and daily energy counter reset.
- **Charging Indicator**: Tri-color RGB LED indicates bulk charging (Green), float/discharging (Yellow), or low battery alert (Red).

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| RGB LED | 4 (R), 16 (G), 17 (B) | Solar/Battery charge state |
| Audio Buzzer | 26 | Touch chirp and low-battery warning |
| Ambient Light (LDR) | 34 | Auto-dimming night mode |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)

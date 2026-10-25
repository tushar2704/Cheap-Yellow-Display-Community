# Project 19: GRBL CNC Milling & Laser Engraver Touch Pendant

A handheld touchscreen pendant and jog controller for GRBL / FluidNC CNC milling routers and diode laser engravers using the Cheap Yellow Display (ESP32-2432S028R). Features real-time X/Y/Z coordinate readouts, tactile jog D-Pad with configurable step resolution (0.1mm, 1.0mm, 10mm, 50mm), spindle/laser control, coordinate zeroing, and an emergency hardware stop button.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **3-Axis Jogging**: Interactive on-screen directional buttons for precise X, Y, and Z positioning.
- **Selectable Step Distance**: Quick toggle between 0.1 mm (fine tramming), 1.0 mm, 10 mm, and 50 mm (rapid transit).
- **Work Coordinate Readout**: Live coordinate values for X, Y, and Z relative to work zero ($G54$).
- **One-Touch Axis Zeroing**: Set current position as workpiece origin ($X0, Y0, Z0$).
- **Emergency Stop (E-STOP)**: Immediate feed hold and alarm trigger with acoustic buzzer and red LED warning.

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| GRBL UART TX / RX (CN1) | 22 / 27 | Serial communication to CNC controller |
| RGB LED | 4 (R), 16 (G), 17 (B) | Machine motion / alarm status |
| Audio Buzzer | 26 | Step jog click and emergency alarm |
| Ambient Light (LDR) | 34 | Auto-dimming night mode |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)

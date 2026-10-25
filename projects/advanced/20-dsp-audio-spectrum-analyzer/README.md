# Project 20: Real-Time DSP Audio Spectrum Analyzer

A 16-band real-time digital signal processing audio spectrum analyzer and graphic equalizer visualizer for the Cheap Yellow Display (ESP32-2432S028R). Features peak-hold gravity decay pips, selectable dynamic color themes (Cyberpunk Neon, Inferno Flame, Retro Green Phosphor), digital gain sensitivity toggling, and RGB LED beat synchronization.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **16-Band Graphic Equalizer**: Fast discrete frequency bin decomposition with segmented visual LED-style bars.
- **Peak Hold Decay**: Suspended floating peak markers with realistic gravity drop.
- **Multiple Color Themes**: Cyberpunk Magenta/Cyan, Inferno Red/Yellow, and Studio Green Phosphor.
- **Gain Sensitivity Presets**: 1x, 2x, and 4x digital preamp boost for quiet or loud audio environments.
- **Beat Sync Lighting**: Tri-color RGB LED pulses on low-end bass transients.

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| Audio Input / ADC | 34 | Analog audio line input (CN1 connector) |
| RGB LED | 4 (R), 16 (G), 17 (B) | Bass beat pulse light |
| Audio Buzzer | 26 | Touch navigation feedback chime |
| Ambient Light (LDR) | 34 | Auto-dimming night mode |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)

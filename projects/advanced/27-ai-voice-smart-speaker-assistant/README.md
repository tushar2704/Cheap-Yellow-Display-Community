# Project 27: Local Voice AI & LLM Smart Speaker Terminal

A smart voice AI interface and local LLM conversation terminal for the Cheap Yellow Display (ESP32-2432S028R). Designed to integrate with Home Assistant Assist, local Whisper speech-to-text, and Ollama / LLaMA-3 / OpenAI compatible APIs. Features a Siri-style fluid animated voice orb with dynamic state machine transitions (Idle, Listening, Thinking, Speaking), transcript and intent display, push-to-talk touch button, and tri-color RGB light feedback.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **Dynamic Animated Voice Orb**: Fluid glowing neon sphere with distinct visual signatures for Listening, Reasoning, and Speaking states.
- **Natural Language Intent Cards**: Displays real-time voice query transcriptions alongside structured LLM responses and Home Assistant smart device actions.
- **Push-To-Talk Interaction**: Touch the bottom screen to activate microphone listening or interrupt spoken responses.
- **Tri-Color State Indicator**: Green for Standby, Red for Listening, Yellow for Processing, and Blue for Speaking.
- **Audio Feedback**: Pleasant acoustic wake chime and completion chimes on the onboard buzzer.

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| I2S / PDM Microphone (CN1) | 22 / 27 | External I2S microphone (INMP441) |
| RGB LED | 4 (R), 16 (G), 17 (B) | Voice AI state lighting |
| Audio Buzzer / DAC Out | 26 | Wake chime and voice prompts |
| Ambient Light (LDR) | 34 | Auto-dimming night mode |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)

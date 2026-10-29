# 🎵 Project 28: Spotify Album Art & Media Remote

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Desktop Media Remote & Album Showcase**:
   - High-resolution visual media dashboard displaying current track title, performing artist, and album name.
   - Generative vinyl disc artwork with dominant color center labels.
2. **Dynamic Mood Ambient Backlighting**:
   - Dominant color extraction algorithm determining the hue of the currently playing album (e.g. Purple for M83, Crimson for The Weeknd, Gold for Daft Punk, Cyan for HOME).
   - Tricolor active-low RGB LED dynamically mirrors the album's ambient aesthetic across your workspace.
3. **Real-Time Playback Progress Timeline**:
   - Animated track progression bar with elapsed minutes/seconds vs total track duration.
4. **Touchpad Playback Controls**:
   - Five dedicated tactile touch controls: Previous (`|<<`), Play / Pause (`> / ||`), Next (`>>|`), Volume Down (`-`), and Volume Up (`+`).
   - Spotify Connect status badge and audio chirp confirmation.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — Ambient album mood |
| **GPIO 26** | Audio / Buzzer | Tactile playback click tones |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/28-spotify-album-remote
pio run -t upload
```

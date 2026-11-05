# 💣 Project 44: CYD Keep Talking & Nobody Explodes (Two-Player Wireless Bomb Defusal)

A high-intensity, cooperative two-player bomb defusal arcade game inspired by the legendary *Keep Talking and Nobody Explodes*, engineered specifically for the **Cheap Yellow Display (ESP32-2432S028R)**.

Using sub-5ms peer-to-peer **ESP-NOW wireless networking**, one player acts as the frantic **Bomb Defuser** touching physical digital wires and cryptic keypads, while the other player holds the classified **Defusal Manual** shouting tactical instructions!

---

## 📺 Channel & Video Showcase

- **Main Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) — Tech humor, ESP32 builds, and creator video challenges.
- **3D Enclosures & Design**: [@Makerrdesigns](https://youtube.com/@Makerrdesigns) — Custom tactical military briefcase and bomb casing 3D models.

---

## ⚡ Features

1. **Instant Dual-Role Selection**:
   - Choose **`[PLAY AS BOMB]`** or **`[DEFUSAL MANUAL]`** directly on the touch screen on startup.
   - Fully playable standalone on a single CYD or synced wirelessly across two CYD boards over ESP-NOW.
2. **Dynamic Bomb Modules**:
   - **Ticking 7-Segment Digital Clock**: Counts down from 60 seconds with acoustic audio ticks that accelerate into panic mode during the final 10 seconds.
   - **Strike System (3 Strikes & Detonate)**: Visual strike indicators and harsh error buzzes when a wrong action is taken.
   - **4-Color Wire Cutting Module**: Red, Blue, Yellow, and White wires rendered on-screen. Touch to cut!
   - **Cryptic Greek Keypad Module**: 4 cryptic buttons (`Ω`, `π`, `λ`, `Ψ`) that must be tapped in a strict procedural sequence.
3. **Interactive Defusal Manual**:
   - Synced live telemetry displaying the bomb's active serial code (e.g. `CYD-7B9`), remaining seconds, and strike tally.
   - **Page 1: Wire Cutting Protocol** logic tree based on the serial parity and present colors.
   - **Page 2: Keypad Precedence Table** detailing the exact execution order.
4. **Cinematic Game Over Outcomes**:
   - **Detonation**: Shaking screen, loud explosive static rumble on GPIO 26, and flashing emergency red lighting.
   - **Disarmed**: Tactical victory fanfare, celebratory green LED illumination, and disarm banner.

---

## 🛠️ Hardware Requirements

| Component | Pin / Specification | Notes |
| :--- | :--- | :--- |
| **Board** | ESP32-2432S028R (CYD) | 1 or 2 boards supported |
| **Display** | ILI9341 SPI (320x240) | Built-in |
| **Touch** | XPT2046 SPI (CS=33, IRQ=36) | Touch to cut wires & press keys |
| **Speaker / Buzzer** | GPIO 26 | Ticks, buzzer strikes, detonation sound |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Panic strobe & disarm confirmation |
| **Wireless** | ESP-NOW (2.4GHz) | Sub-5ms latency peer-to-peer |

---

## 🚀 Quick Start

### PlatformIO
```bash
cd projects/advanced/44-cyd-bomb-defusal-keep-talking
pio run --target upload
```

### Arduino IDE
1. Open `44-cyd-bomb-defusal-keep-talking.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and **XPT2046_Touchscreen**.
4. Flash two CYDs (one as Bomb, one as Manual) and defuse!

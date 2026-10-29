# ⚡ Project 01: DevOps Command Vault (Docker, Git & Exit Codes)

> **Elecrow 2.8" CYD Series**  
> **Channel Branding**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  
> **Open-Source Inspiration**: [lepczynski-cloud/crowpanel-Admin-Pocket-Toolkit](https://github.com/lepczynski-cloud/crowpanel-Admin-Pocket-Toolkit)

---

## 📺 YouTube Presentation & Angles

* **Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
* **Video Title**: *"The $15 Cheat Code Screen That Saved My Coding Job"*
* **Hook**: Stop tab-switching and scouring StackOverflow for arcane Docker commands, Git detached HEAD rescues, or Linux kill signals. This dedicated touch panel gives you an instant offline cheat sheet on your desk with one-touch USB Serial output!

---

## 🚀 Key Features

1. **Docker Command Matrix**: Quick access to interactive run, container shell exec, live tail logs, system prune, and compose up commands.
2. **Git Disaster Recovery**: One-touch cheat sheet for soft commit resets, reflog recovery, detached HEAD branches, safe force pushes, and stash operations.
3. **Linux Signals Lookup**: Quick reference for `SIGTERM (15)`, `SIGKILL (9)`, `SIGHUP (1)`, `SIGINT (2)`, and `SIGUSR1 (10)`.
4. **DNS & Networking Records**: Fast overview of `A`, `AAAA`, `CNAME`, `MX`, and `TXT` records with syntax examples.
5. **HTTP Status Code Reference**: Instant explanations for 401, 403, 404, 500, and 502 codes.
6. **Direct USB Serial Bridge**: Tap the `>> COPY TO USB SERIAL <<` button to stream the highlighted command over the 115200 baud USB connection directly to your workstation clipboard or terminal bridge.
7. **Audio & RGB Feedback**: Acoustic confirmation tones on GPIO 26 and tricolor active-low status LEDs (Green: Ready, Blue: Command Dispatched).

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | DAC / PWM tone generation |
| **GPIO 34** | LDR Light Sensor | Ambient light ADC input |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/01-devops-command-vault
pio run -t upload
```

Or open the folder in VS Code with the PlatformIO extension installed and click **Upload**.

---

## 🖨️ 3D Enclosure Concept
* **Keycap-Angled Desk Wedge**: 35-degree mechanical-keyboard-matching tilt stand with an integrated top trough to dock the resistive stylus pen.

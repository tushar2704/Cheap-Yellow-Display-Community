# 🎛️ Project 04: USB Serial PC Telemetry & Macro Deck

> **Elecrow 2.8" CYD Series**  
> **Channel Branding**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  
> **Open-Source Base**: [lepczynski-cloud/ESP32-PC-Control-Deck](https://github.com/lepczynski-cloud/ESP32-PC-Control-Deck)

---

## 📺 YouTube Presentation & Angles

* **Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
* **Video Title**: *"Why Pay $150 for a Stream Deck When This $15 Screen Exists?"*
* **Hook**: Commercial stream decks and PC sensor panels cost an arm and a leg. By using the built-in CH340 USB-Serial port on the Elecrow 2.8" CYD paired with a lightweight Python bridge daemon, you get live CPU/GPU/VRAM bar graphs and 6 physical touch macro buttons right under your monitor!

---

## 🚀 Key Features

1. **Real-Time PC Hardware Telemetry**:
   - Bar gauges and numerical readouts for CPU %, GPU %, RAM %, VRAM %, and core temperatures.
   - Automatic fallback to animated simulated metrics when unplugged from the host PC.
2. **6-Tile Interactive Touch Macro Pad**:
   - `MUTE MIC`: Dispatches quick mute action for Discord / system input.
   - `TERMINAL`: Opens development shell or IDE workspace.
   - `GIT PULL`: Triggers repository synchronization scripts.
   - `RUN BUILD`: Fires off project compilation or test runs.
   - `AI PROMPT`: Brings up your favorite local or cloud AI assistant.
   - `LOCK PC`: Locks your desktop when walking away.
3. **Cross-Platform Host Bridge Daemon (`bridge.py`)**:
   - Python 3 daemon using `psutil` and `pyserial` to stream host hardware stats and listen for touchscreen triggers.
4. **Active Hardware Feedback**:
   - Acoustic click tones on GPIO 26 and active-low status LEDs (Green: Nominal, Blue: Macro Dispatched).

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone generation |
| **USB Serial** | CH340 UART @ 115200 | Bidirectional JSON packets |

---

## 🛠️ How to Run & Connect

1. **Flash the Firmware**:
   ```bash
   cd elecrow_projects/04-usb-pc-telemetry-deck
   pio run -t upload
   ```
2. **Run the Host Python Bridge**:
   ```bash
   pip install pyserial psutil
   python bridge.py
   ```

---

## 🖨️ 3D Enclosure Concept
* **Under-Monitor Clip Pod**: Low-profile clamp that suspends the 2.8" display right beneath the bottom bezel of your computer monitor.

# 🤖 Project 05: Local-First Private AI Desk Terminal (Ollama Client)

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Private Local AI Integration**:
   - Queries your local home network Ollama server (`http://<local-ip>:11434/api/generate`) via Wi-Fi HTTP requests.
   - 100% private: no data ever leaves your local network.
2. **One-Touch Prompt Cards**:
   - `[1. SUMMARIZE]`: Generates a 3-bullet executive summary.
   - `[2. FIX CODE]`: Diagnoses software bugs and outputs corrected syntax.
   - `[3. EXPLAIN REGEX]`: Deconstructs complex regular expressions step-by-step.
   - `[4. DRAFT REPLY]`: Generates polite professional email and message drafts.
3. **On-Device Model Selector**:
   - Tap the top-right model badge to toggle between `llama3`, `mistral`, and `phi3`.
4. **Streaming Terminal Canvas**:
   - Token output streams in real-time onto an on-screen scrolling terminal window.
   - Offline demonstration fallback with simulated LLM tokens when disconnected from LAN.
5. **Acoustic & Tricolor LED Signaling**:
   - Status LED glows Cyan during active token generation, transitions to Green when query finishes.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone generation |
| **Wi-Fi** | 802.11 b/g/n | HTTP REST client to Ollama |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/05-local-ai-desk-terminal
pio run -t upload
```

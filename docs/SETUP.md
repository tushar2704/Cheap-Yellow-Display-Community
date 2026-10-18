# ⚡ Getting Started — Setting Up Your Cheap Yellow Display (CYD)

This guide will help you set up your development environment and flash your first program to any CYD variant.

---

## 📋 Prerequisites

- A Cheap Yellow Display (any variant)
- A USB cable (micro-USB or USB-C depending on your CYD model)
- A computer (Windows, macOS, or Linux)

---

## 🅰️ Option 1: Arduino IDE Setup

### Step 1: Install Arduino IDE

Download and install [Arduino IDE 2.x](https://www.arduino.cc/en/software) (recommended) or Arduino IDE 1.8.x.

### Step 2: Add ESP32 Board Package

1. Open Arduino IDE
2. Go to **File → Preferences**
3. In "Additional Board Manager URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Board Manager**
5. Search for **"ESP32"** and install **"esp32 by Espressif Systems"**

### Step 3: Select Your Board

| CYD Model | Board Selection |
|-----------|----------------|
| 2.4" / 2.8" / 3.2" / 3.5" (ESP32) | `ESP32 Dev Module` |
| 4.0" / 5.0" / 7.0" (ESP32-S3) | `ESP32S3 Dev Module` |

### Step 4: Install Required Libraries

Go to **Tools → Manage Libraries** and install:

| Library | Purpose | Required For |
|---------|---------|-------------|
| **TFT_eSPI** | Display driver | All CYD variants |
| **XPT2046_Touchscreen** | Resistive touch | 2.4", 2.8"R, 3.2", 3.5"R |
| **FT6236** (or TAMC_GT911) | Capacitive touch | 2.8"C, 3.5"C, 4.0", 5.0", 7.0" |
| **ArduinoJson** | JSON parsing | WiFi/API projects |
| **WiFiManager** | WiFi setup portal | WiFi projects |

### Step 5: Configure TFT_eSPI

**This is the most important step!** TFT_eSPI needs to be configured for your specific CYD.

1. Find the TFT_eSPI library folder:
   - Windows: `C:\Users\{user}\Documents\Arduino\libraries\TFT_eSPI\`
   - macOS: `~/Documents/Arduino/libraries/TFT_eSPI/`
   - Linux: `~/Arduino/libraries/TFT_eSPI/`
2. Open `User_Setup.h`
3. Comment out ALL existing driver selections
4. Add the configuration for your CYD variant (see [Display Config](DISPLAY_CONFIG.md))

**For the most common CYD (2.8" Resistive — ESP32-2432S028R):**

```cpp
#define ILI9341_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_BL   21
#define TFT_BACKLIGHT_ON HIGH

#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1

#define TOUCH_CS 33

#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
```

### Step 6: Upload Your First Sketch

1. Connect your CYD via USB
2. Select the correct **COM port** (Tools → Port)
3. Open one of the example projects from this repo
4. Click **Upload** (→ button)

---

## 🅱️ Option 2: PlatformIO Setup

### Step 1: Install VSCode + PlatformIO

1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Install the [PlatformIO extension](https://platformio.org/install/ide?install=vscode)

### Step 2: Open a Project

Each project in this repo includes a `platformio.ini` file. Simply:

1. Open VSCode
2. Open the project folder
3. PlatformIO will automatically detect the configuration
4. Click the **Upload** button (→) in the PlatformIO toolbar

### Example `platformio.ini` for CYD 2.8" Resistive:

```ini
[env:cyd-28-resistive]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200

lib_deps =
    bodmer/TFT_eSPI@^2.5.0
    paulstoffregen/XPT2046_Touchscreen@^1.4

build_flags =
    -DUSER_SETUP_LOADED=1
    -DILI9341_DRIVER=1
    -DTFT_WIDTH=240
    -DTFT_HEIGHT=320
    -DTFT_BL=21
    -DTFT_BACKLIGHT_ON=HIGH
    -DTFT_MISO=12
    -DTFT_MOSI=13
    -DTFT_SCLK=14
    -DTFT_CS=15
    -DTFT_DC=2
    -DTFT_RST=-1
    -DTOUCH_CS=33
    -DSPI_FREQUENCY=40000000
    -DSPI_READ_FREQUENCY=20000000
    -DSPI_TOUCH_FREQUENCY=2500000
```

---

## 🅲 Option 3: ESPHome (for Home Assistant Users)

If you're integrating your CYD with Home Assistant, ESPHome is a great option:

1. Install ESPHome in Home Assistant
2. Create a new device configuration
3. See our Home Assistant project for a complete ESPHome config

---

## ✅ Verify Your Setup

After uploading, your CYD should:
- 🖥️ Display text or graphics on the screen
- 👆 Respond to touch input (if the project uses touch)
- 📡 Connect to WiFi (if the project uses WiFi)

---

## ❓ Having Trouble?

See our [Troubleshooting Guide](TROUBLESHOOTING.md) for solutions to common issues.

---

📺 **Video walkthrough**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)

---

*[← Back to main README](../README.md)*

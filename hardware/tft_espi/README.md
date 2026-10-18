# 🖥️ TFT_eSPI Setup Guides for Cheap Yellow Display (CYD)

> Pre-configured header files for Bodmer's popular `TFT_eSPI` library across all CYD variants.

---

## 🛠️ How to Use with Arduino IDE

1. Install **TFT_eSPI** via the Arduino Library Manager.
2. Navigate to your Arduino libraries folder:
   - **Windows**: `Documents/Arduino/libraries/TFT_eSPI/`
   - **macOS**: `~/Documents/Arduino/libraries/TFT_eSPI/`
   - **Linux**: `~/Arduino/libraries/TFT_eSPI/`
3. Replace `User_Setup.h` with the matching setup file for your CYD variant from this folder:
   - For 2.8" Resistive: Copy `User_Setup_CYD_28R.h` to `User_Setup.h`
   - For 3.5" Resistive: Copy `User_Setup_CYD_35R.h` to `User_Setup.h`

---

## ⚡ How to Use with PlatformIO (Zero File Swapping!)

In PlatformIO, you **do not** need to edit library files. Simply define the build flags directly in your `platformio.ini`:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    bodmer/TFT_eSPI @ ^2.5.43
    https://github.com/PaulStoffregen/XPT2046_Touchscreen.git

build_flags =
    -D USER_SETUP_LOADED=1
    -D ILI9341_2_DRIVER=1
    -D TFT_WIDTH=240
    -D TFT_HEIGHT=320
    -D TFT_MISO=12
    -D TFT_MOSI=13
    -D TFT_SCLK=14
    -D TFT_CS=15
    -D TFT_DC=2
    -D TFT_RST=-1
    -D TFT_BL=21
    -D TFT_BACKLIGHT_ON=HIGH
    -D LOAD_GLCD=1
    -D LOAD_FONT2=1
    -D LOAD_FONT4=1
    -D LOAD_FONT6=1
    -D LOAD_FONT7=1
    -D LOAD_FONT8=1
    -D LOAD_GFXFF=1
    -D SMOOTH_FONT=1
    -D SPI_FREQUENCY=40000000
    -D SPI_READ_FREQUENCY=20000000
    -D SPI_TOUCH_FREQUENCY=2500000
```

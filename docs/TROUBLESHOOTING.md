# 🔧 Troubleshooting — Cheap Yellow Display (CYD) Common Issues

> Having trouble with your CYD? Check these common issues and solutions.

---

## 🖥️ Display Issues

### White/Blank Screen After Upload

**Symptoms**: Code uploads successfully but screen stays white or blank.

**Solutions**:
1. **Wrong display driver** — Make sure your TFT_eSPI config matches your CYD:
   - 2.4", 2.8", 3.2": `ILI9341_DRIVER`
   - 3.5": `ST7796_DRIVER`
   - 4.0"+: Use LovyanGFX or Arduino_GFX instead of TFT_eSPI
2. **Wrong pin configuration** — Double-check all SPI pins match your variant (see [Pinout](PINOUT.md))
3. **Backlight not enabled** — Add `TFT_BL` pin and set `TFT_BACKLIGHT_ON HIGH` in your config
4. **Wrong SPI frequency** — Try reducing `SPI_FREQUENCY` to `27000000`

### Display Shows Garbled/Wrong Colors

**Solutions**:
1. **Wrong color order** — Try adding `#define TFT_RGB_ORDER TFT_BGR` to your config
2. **Wrong resolution** — Verify `TFT_WIDTH` and `TFT_HEIGHT` match your screen
3. **Wrong rotation** — Try different values for `tft.setRotation(0-3)`

### Flickering Display

**Solutions**:
1. **Power supply issue** — Use a good quality USB cable and power source (min 500mA)
2. **SPI frequency too high** — Reduce `SPI_FREQUENCY`
3. **Missing decoupling capacitor** — Some cheap USB cables cause noise

---

## 👆 Touch Issues

### Touch Not Working

**Solutions**:
1. **Wrong touch library** — Resistive: use `XPT2046_Touchscreen`, Capacitive: use `FT6236` or `TAMC_GT911`
2. **Wrong TOUCH_CS pin** — For 2.8" resistive: `TOUCH_CS = GPIO 33`
3. **Touch not initialized** — Make sure you call touch initialization in `setup()`
4. **Shared SPI bus** — Touch and display share SPI on most CYDs; some libraries need explicit handling

### Touch Coordinates Are Wrong/Inverted

**Solutions**:
1. **Calibration needed** — Run a touch calibration sketch and save calibration values
2. **Wrong rotation** — Touch rotation must match display rotation
3. **X/Y swapped** — Try swapping X and Y in your touch mapping code

### Touch is Inaccurate (Resistive)

**Solutions**:
1. **Recalibrate** — Touch calibration values vary between individual boards
2. **Use a stylus** — Resistive screens are more accurate with a stylus
3. **SPI touch frequency** — Try `SPI_TOUCH_FREQUENCY 2500000`

---

## 📤 Upload Issues

### Can't Upload — "Failed to connect to ESP32"

**Solutions**:
1. **Hold BOOT button** — On some CYDs, hold the BOOT button while uploading
2. **Try different USB cable** — Many USB cables are charge-only (no data)
3. **Install CH340 driver** — Most CYDs use the CH340 USB-to-serial chip:
   - [CH340 Driver Download](https://www.wch-ic.com/downloads/CH341SER_ZIP.html)
4. **Install CP2102 driver** — Some variants use CP2102:
   - [CP2102 Driver Download](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
5. **Check COM port** — Make sure the correct port is selected
6. **Reduce upload speed** — Set upload speed to `115200` instead of `921600`

### Upload Speed is Very Slow

**Solutions**:
1. Increase upload speed in settings to `921600`
2. Use PlatformIO instead of Arduino IDE (generally faster)
3. Check your USB cable — some cables have high resistance

### "Sketch too large" Error

**Solutions**:
1. Change partition scheme to **"Huge APP (3MB No OTA/1MB SPIFFS)"**
2. In PlatformIO, add: `board_build.partitions = huge_app.csv`
3. Optimize your code — remove unused libraries

---

## 📡 WiFi Issues

### WiFi Won't Connect

**Solutions**:
1. **2.4GHz only** — ESP32 only supports 2.4GHz WiFi, not 5GHz
2. **Check credentials** — SSID and password are case-sensitive
3. **Router compatibility** — Some routers have issues with ESP32; try disabling band steering
4. **Antenna** — The CYD's PCB antenna is small; keep it close to your router

### WiFi Disconnects Frequently

**Solutions**:
1. **Power supply** — WiFi draws extra current; use a good USB power source
2. **WiFi reconnect logic** — Add automatic reconnection in your code
3. **Reduce WiFi power** — Try `WiFi.setTxPower(WIFI_POWER_8_5dBm)` to reduce interference

---

## 💾 SD Card Issues

### SD Card Not Detected

**Solutions**:
1. **Format as FAT32** — ESP32 SD library requires FAT32 (not exFAT)
2. **Max 32GB** — Use SD cards 32GB or smaller
3. **Check wiring** — SD card uses a separate SPI bus (see [Pinout](PINOUT.md))
4. **Insert before power** — Some boards need the SD card inserted before powering on

---

## 🔊 Audio Issues

### No Sound from Speaker

**Solutions**:
1. **Speaker not connected** — Some CYDs don't include a speaker; you may need to solder one
2. **Wrong pin** — Speaker/DAC is typically on GPIO 26
3. **Use `dacWrite()`** — For simple tones: `dacWrite(26, volume)`

---

## ⚡ Power Issues

### Board Keeps Resetting

**Solutions**:
1. **Insufficient power** — Use a USB port that provides at least 500mA
2. **Bad USB cable** — Try a different, shorter cable
3. **Brownout detection** — Disable brownout detection (not recommended for production):
   ```cpp
   #include "soc/soc.h"
   #include "soc/rtc_cntl_reg.h"
   // In setup():
   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
   ```

---

## ❓ Still Stuck?

1. Check the [FAQ](FAQ.md)
2. Open an [Issue](https://github.com/tushar2704/Cheap-Yellow-Display-Community/issues)
3. Watch troubleshooting videos on [IamNotIntoTech](https://youtube.com/@IamNotIntoTech)

---

*[← Back to main README](../README.md)*

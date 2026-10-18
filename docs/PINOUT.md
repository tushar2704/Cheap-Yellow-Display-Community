# 🔌 CYD Pinout Reference — Cheap Yellow Display Pin Maps

> Complete pin mapping for every Cheap Yellow Display (CYD) variant.

---

## 📌 ESP32-2432S028R — 2.8" Resistive Touch (Most Common CYD)

This is the **most popular** CYD variant and the one most projects in this repo target by default.

### Display Pins (ILI9341 via SPI)

| Function | GPIO | Notes |
|----------|------|-------|
| TFT_MISO | GPIO 12 | SPI MISO |
| TFT_MOSI | GPIO 13 | SPI MOSI |
| TFT_SCLK | GPIO 14 | SPI Clock |
| TFT_CS | GPIO 15 | Chip Select |
| TFT_DC | GPIO 2 | Data/Command |
| TFT_RST | N/A | Connected to EN (reset) |
| TFT_BL | GPIO 21 | Backlight (active HIGH) |

### Touch Pins (XPT2046 — Resistive)

| Function | GPIO | Notes |
|----------|------|-------|
| TOUCH_CS | GPIO 33 | Touch chip select |
| TOUCH_IRQ | GPIO 36 | Touch interrupt (input only) |
| TOUCH_MISO | GPIO 12 | Shared with display SPI |
| TOUCH_MOSI | GPIO 13 | Shared with display SPI |
| TOUCH_CLK | GPIO 14 | Shared with display SPI |

### SD Card Pins

| Function | GPIO | Notes |
|----------|------|-------|
| SD_CS | GPIO 5 | SD card chip select |
| SD_MISO | GPIO 19 | SPI MISO (separate SPI bus) |
| SD_MOSI | GPIO 23 | SPI MOSI (separate SPI bus) |
| SD_SCK | GPIO 18 | SPI Clock (separate SPI bus) |

### Other Pins

| Function | GPIO | Notes |
|----------|------|-------|
| LED (RGB Red) | GPIO 4 | Active LOW |
| LED (RGB Green) | GPIO 16 | Active LOW |
| LED (RGB Blue) | GPIO 17 | Active LOW |
| LDR | GPIO 34 | Light sensor (ADC, input only) |
| Speaker | GPIO 26 | DAC output (if speaker connected) |

### Expansion Header (P3)

| Pin | GPIO | Notes |
|-----|------|-------|
| 1 | GND | Ground |
| 2 | GPIO 35 | Input only, ADC |
| 3 | GPIO 22 | I2C SCL |
| 4 | GPIO 27 | General purpose |

### Expansion Header (CN1)

| Pin | GPIO | Notes |
|-----|------|-------|
| 1 | GND | Ground |
| 2 | GPIO 22 | I2C SCL (shared with P3) |
| 3 | GPIO 27 | General purpose (shared with P3) |
| 4 | 3.3V | Power output |

---

## 📌 ESP32-2432S028C — 2.8" Capacitive Touch

Same as 2.8" Resistive **except** for touch:

### Touch Pins (FT6236 — Capacitive via I2C)

| Function | GPIO | Notes |
|----------|------|-------|
| TOUCH_SDA | GPIO 33 | I2C Data |
| TOUCH_SCL | GPIO 32 | I2C Clock |
| TOUCH_IRQ | GPIO 21 | Touch interrupt |
| TOUCH_RST | GPIO 25 | Touch reset |

> ⚠️ **Note**: On the capacitive version, GPIO 21 is used for touch interrupt instead of backlight.

---

## 📌 ESP32-3248S035R/C — 3.5" Variants

### Display Pins (ST7796 via SPI)

| Function | GPIO | Notes |
|----------|------|-------|
| TFT_MISO | GPIO 12 | SPI MISO |
| TFT_MOSI | GPIO 13 | SPI MOSI |
| TFT_SCLK | GPIO 14 | SPI Clock |
| TFT_CS | GPIO 15 | Chip Select |
| TFT_DC | GPIO 2 | Data/Command |
| TFT_RST | N/A | Connected to EN |
| TFT_BL | GPIO 27 | Backlight |

> ⚠️ **Note**: The 3.5" uses the **ST7796** driver, not ILI9341. Make sure your library config reflects this!

---

## 📌 ESP32-S3 Variants (4.0", 5.0", 7.0")

The larger CYD variants use **ESP32-S3** with **RGB parallel** display interface — these have significantly different pin configurations.

> 🚧 **Detailed pin maps for ESP32-S3 variants coming in Phase 2!**

### General Notes for ESP32-S3 CYDs:
- Use **RGB parallel** display interface (not SPI)
- Require the **Arduino_GFX** or **LovyanGFX** library (not TFT_eSPI)
- All use **capacitive touch** (GT911 controller via I2C)
- Have **USB-C** connector
- Support **USB OTG** (native USB on ESP32-S3)

---

## 🔗 Useful Resources

- 📺 [Pin explanation video — IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 📄 [Full variant comparison](VARIANTS.md)
- 🔧 [Display configuration guide](SETUP.md#step-5-configure-tft_espi)

---

*[← Back to main README](../README.md)*

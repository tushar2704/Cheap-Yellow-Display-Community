# 📐 CYD Variant Guide — Identifying Your Cheap Yellow Display

> Not sure which Cheap Yellow Display you have? This guide covers every CYD variant with photos, specs, and identification tips.

---

## 🔍 Quick Identification

### Step 1: Check the Model Number

Look at the **back of your board** for a printed model number. Common formats:

- `ESP32-2432S028R` → 2.8" Resistive
- `ESP32-2432S028C` → 2.8" Capacitive
- `ESP32-3248S035R` → 3.5" Resistive
- etc.

### Decoding the Model Number

```
ESP32-XXYYS0ZZT

XX   = Horizontal resolution prefix (24 = 240px, 32 = 320px, 80 = 800px)
YY   = Vertical resolution prefix (32 = 320px, 48 = 480px)
S0   = Sunton board identifier
ZZ   = Screen size in tenths of inches (28 = 2.8", 35 = 3.5", 40 = 4.0")
T    = Touch type (R = Resistive, C = Capacitive, N = No touch)
```

### Step 2: Check Screen Size

Measure the **visible display area** diagonally:
- ~2.4" (6 cm) → 2.4" variant
- ~2.8" (7 cm) → 2.8" variant (most common)
- ~3.2" (8 cm) → 3.2" variant
- ~3.5" (9 cm) → 3.5" variant
- ~4.0" (10 cm) → 4.0" variant
- ~5.0" (13 cm) → 5.0" variant
- ~7.0" (18 cm) → 7.0" variant

### Step 3: Check Touch Type

- **Resistive**: Needs pressure to register touch (works with stylus or fingernail)
- **Capacitive**: Works with light finger touch (like your phone)

---

## 📊 Complete Variant Comparison

| Feature | 2.4" | 2.8" (R) | 2.8" (C) | 3.2" | 3.5" (R) | 3.5" (C) | 4.0" | 5.0" | 7.0" |
|---------|------|----------|----------|------|----------|----------|------|------|------|
| **Model** | ESP32-2432S024 | ESP32-2432S028R | ESP32-2432S028C | ESP32-3248S032 | ESP32-3248S035R | ESP32-3248S035C | ESP32-4048S040 | ESP32-8048S050 | ESP32-8048S070 |
| **Resolution** | 320×240 | 320×240 | 320×240 | 320×240 | 480×320 | 480×320 | 480×480 | 800×480 | 800×480 |
| **Touch** | Resistive | Resistive | Capacitive | Resistive | Resistive | Capacitive | Capacitive | Capacitive | Capacitive |
| **ESP32** | WROOM-32 | WROOM-32 | WROOM-32 | WROOM-32 | WROOM-32 | WROOM-32 | S3 | S3 | S3 |
| **Display Driver** | ILI9341 | ILI9341 | ILI9341 | ILI9341 | ST7796 | ST7796 | ST7701 | RGB | RGB |
| **Interface** | SPI | SPI | SPI | SPI | SPI | SPI | RGB | RGB | RGB |
| **USB** | Micro-USB | Micro-USB | Micro-USB | Micro-USB | Micro-USB | Micro-USB | USB-C | USB-C | USB-C |
| **SD Card** | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes |
| **RGB LED** | Yes | Yes | Yes | Yes | Yes | Yes | No | No | No |
| **Speaker** | No | Optional | Optional | No | Optional | Optional | Built-in | Built-in | Built-in |
| **PSRAM** | No | No | No | No | No | No | Yes (8MB) | Yes (8MB) | Yes (8MB) |
| **Flash** | 4MB | 4MB | 4MB | 4MB | 4MB | 4MB | 16MB | 16MB | 16MB |
| **Price (approx)** | $10 | $12-15 | $13-16 | $13 | $14-17 | $15-18 | $18-22 | $20-25 | $22-28 |

---

## 🏷️ Which CYD Should I Buy?

### For Beginners
→ **ESP32-2432S028R (2.8" Resistive)** — Best documented, most projects support it, cheapest.

### For Better Touch Experience
→ **ESP32-2432S028C (2.8" Capacitive)** — Same as above but with smooth capacitive touch.

### For Larger Dashboards
→ **ESP32-3248S035C (3.5" Capacitive)** — Good balance of size and price.

### For Maximum Screen Real Estate
→ **ESP32-8048S050 (5.0")** or **ESP32-8048S070 (7.0")** — ESP32-S3 based, great for complex UIs.

---

## ⚠️ Important Compatibility Notes

1. **ESP32 vs ESP32-S3**: The 4.0", 5.0", and 7.0" variants use **ESP32-S3**, which requires different board selection and library configuration.
2. **SPI vs RGB Interface**: Smaller CYDs (≤3.5") use SPI displays; larger ones (≥4.0") use RGB parallel interface and need different display libraries.
3. **Touch Libraries**: Resistive models use **XPT2046_Touchscreen**, capacitive models use **FT6236** or **TAMC_GT911**.
4. **Pin Differences**: Even within the same size, resistive and capacitive versions may have different pin assignments. Always check the [Pinout Reference](PINOUT.md).

---

## 🛒 Where to Buy

- **AliExpress** — Search the exact model number for best prices
- **Makerfabs** — [makerfabs.com](https://www.makerfabs.com/) — Tested boards, good quality
- **Amazon** — Search "ESP32 TFT touch display" — faster shipping in US/EU

---

📺 **CYD buying guide video**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
🖨️ **Which CYD for 3D printing projects**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

*[← Back to main README](../README.md)*

# ❓ FAQ — Frequently Asked Questions about Cheap Yellow Display (CYD)

---

## General Questions

### What is a Cheap Yellow Display (CYD)?
A CYD is a low-cost ESP32-based development board with a built-in TFT LCD touchscreen. They're called "Cheap Yellow Display" because of their distinctive yellow PCB and affordable price point ($10–$25). The boards are manufactured by Sunton and sold under model numbers like ESP32-2432S028R.

### Why "Cheap Yellow Display"?
The original model number "ESP32-2432S028R" doesn't exactly roll off the tongue. The community adopted "Cheap Yellow Display" (CYD) as a friendly nickname because:
- They're **cheap** ($10–$25)
- The PCB is **yellow**
- They're a **display** module

### How many CYD variants exist?
There are currently **9+ known variants**, ranging from 2.4" to 7.0" screens. See our [Variant Guide](VARIANTS.md) for the complete list.

### Which CYD should I buy first?
The **ESP32-2432S028R** (2.8" Resistive) is the best starting point — it's the cheapest, most documented, and most widely supported by community projects.

---

## Technical Questions

### What's the difference between Resistive and Capacitive touch?
- **Resistive**: Requires physical pressure. Works with any object (finger, stylus, gloved hand). Less accurate but more versatile.
- **Capacitive**: Detects finger proximity (like your phone). Smoother, more precise, but doesn't work with gloves or non-conductive styluses.

### Which display library should I use?
| CYD Size | Recommended Library |
|----------|-------------------|
| 2.4" – 3.5" (SPI) | **TFT_eSPI** by Bodmer |
| 4.0" – 7.0" (RGB) | **LovyanGFX** or **Arduino_GFX** |

### Can I use LVGL with CYD?
**Yes!** LVGL (Light and Versatile Graphics Library) works great with CYDs. It provides professional-looking widget-based UIs. We have LVGL-based projects in the intermediate and advanced sections.

### Can I use ESPHome / Home Assistant with CYD?
**Yes!** ESPHome supports many CYD variants, making them great as Home Assistant control panels and displays.

### What programming languages/frameworks are supported?
- **Arduino C++** (most common, recommended for beginners)
- **PlatformIO** (recommended for experienced developers)
- **ESP-IDF** (advanced, maximum control)
- **ESPHome** (YAML-based, great for Home Assistant)
- **MicroPython** (possible but limited display support)

### Does CYD support OTA (Over-The-Air) updates?
**Yes!** ESP32 supports OTA updates. You can update your CYD firmware over WiFi without physically connecting a USB cable.

### How much RAM/Flash does a CYD have?
- **ESP32-WROOM** (2.4"–3.5"): 520KB SRAM, 4MB Flash
- **ESP32-S3** (4.0"–7.0"): 512KB SRAM, 8MB PSRAM, 16MB Flash

---

## Buying Questions

### Where can I buy a CYD?
- **AliExpress** — Cheapest, search by model number
- **Makerfabs** — Good quality, tested boards
- **Amazon** — Faster shipping, slightly higher price

### How much does a CYD cost?
Prices range from **$10** (2.4") to **$28** (7.0"), depending on size and seller. The most popular 2.8" model typically costs **$12–$15** shipped.

### Are all CYDs the same quality?
Quality can vary between sellers, but most CYDs are manufactured by **Sunton** and share similar quality. Buying from Makerfabs tends to have more consistent quality.

---

## Project Questions

### Can I sell projects I build using code from this repo?
**Yes!** This repository is MIT licensed. You're free to use, modify, and sell anything you build.

### How do I contribute a project?
See our [Contributing Guide](../CONTRIBUTING.md) or use the [Project Submission template](https://github.com/tushar2704/Cheap-Yellow-Display-Community/issues/new?template=project_submission.md).

### Do I need to solder anything?
**No!** CYDs come fully assembled. Just plug in a USB cable and start programming. External sensors/modules may require soldering to the expansion headers.

---

## Troubleshooting Quick Reference

| Problem | Quick Fix |
|---------|-----------|
| White screen | Check display driver config |
| Touch not working | Check touch library (resistive vs capacitive) |
| Can't upload | Install CH340/CP2102 driver, hold BOOT button |
| WiFi won't connect | ESP32 only supports 2.4GHz |
| SD card not detected | Format as FAT32, max 32GB |

For detailed troubleshooting, see our [Troubleshooting Guide](TROUBLESHOOTING.md).

---

📺 **More answers on**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) | [Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

*[← Back to main README](../README.md)*

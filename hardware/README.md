# ⚙️ Hardware Bible — Cheap Yellow Display (CYD)

> Comprehensive hardware documentation, schematics, pinout references, and drop-in library configurations for every CYD variant.

---

## 📂 Contents

| Directory / File | Description |
|---|---|
| [`cyd_pins.h`](cyd_pins.h) | **Universal pin mapping header** — select any CYD board with one `#define` |
| [`tft_espi/`](tft_espi/) | Pre-configured `User_Setup.h` files and PlatformIO configurations |
| [`lovyangfx/`](lovyangfx/) | Fast DMA LovyanGFX display & touch class definitions |
| [`../docs/PINOUT.md`](../docs/PINOUT.md) | Exhaustive GPIO tables for all display sizes |
| [`../docs/VARIANTS.md`](../docs/VARIANTS.md) | Visual guide & serial number decoder to identify your exact board |
| [`../docs/ADDONS.md`](../docs/ADDONS.md) | How to wire sensors, rotary encoders, speakers, and external buttons |
| [`../docs/MODS.md`](../docs/MODS.md) | Hardware modifications: USB-C CC fix, LDR auto-brightness, audio whine fix |

---

## ⚡ Quick Start: Universal Pin Header

Drop `cyd_pins.h` into your Arduino sketch or PlatformIO `include/` directory:

```cpp
#define CYD_28_RESISTIVE // Choose your board
#include "cyd_pins.h"

void setup() {
  pinMode(CYD_LED_RED, OUTPUT);
  CYD_RGB_ON(CYD_LED_RED); // Turn on red LED
}
```

---

## 📺 Community Videos
- 🎬 Hardware breakdowns: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ 3D printable brackets and housings: [@Makerrdesigns](https://youtube.com/@Makerrdesigns)

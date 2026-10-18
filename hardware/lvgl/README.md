# 🎨 LVGL (Light and Versatile Graphics Library) for CYD

> Complete configuration headers and zero-boilerplate drivers to build high-performance smartphone-like touch interfaces on your Cheap Yellow Display.

---

## 🚀 Quick Start in 3 Lines of Code

Include [`LVGL_CYD_Setup.h`](LVGL_CYD_Setup.h) in your sketch:

```cpp
#include "LVGL_CYD_Setup.h"

void setup() {
  initLVGL(); // Initializes display buffer, driver, and touch input

  // Create your LVGL widgets
  lv_obj_t *btn = lv_btn_create(lv_scr_act());
  lv_obj_center(btn);
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, "Hello LVGL!");
}

void loop() {
  lv_timer_handler(); // Handles animations, rendering, and touch input
  delay(5);
}
```

---

## 📂 Files Included

- [`lv_conf.h`](lv_conf.h): Tuned for 16-bit color, 48kB RAM allocation, fonts, and smooth animations.
- [`LVGL_CYD_Setup.h`](LVGL_CYD_Setup.h): Complete display flushing and touchscreen input mapping with zero boilerplate.

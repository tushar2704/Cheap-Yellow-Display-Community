/**
 * @file LVGL_CYD_Setup.h
 * @brief Zero-boilerplate LVGL v8 Display & Touch initialization for CYD 2.8"
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 */

#ifndef LVGL_CYD_SETUP_H
#define LVGL_CYD_SETUP_H

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#define CYD_SCREEN_WIDTH  320
#define CYD_SCREEN_HEIGHT 240

// Touch Pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

static TFT_eSPI tft = TFT_eSPI();
static SPIClass touchSPI(VSPI);
static XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// LVGL Draw Buffers (1/10th screen height buffer for optimal RAM usage)
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[CYD_SCREEN_WIDTH * 24];

/* Display flushing callback */
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

/* Touchpad read callback */
static void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, CYD_SCREEN_WIDTH);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, CYD_SCREEN_HEIGHT);
    x = constrain(x, 0, CYD_SCREEN_WIDTH - 1);
    y = constrain(y, 0, CYD_SCREEN_HEIGHT - 1);

    data->state = LV_INDEV_STATE_PR;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

/* One-call complete LVGL setup */
static inline void initLVGL() {
  lv_init();

  tft.init();
  tft.setRotation(1); // Landscape
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, CYD_SCREEN_WIDTH * 24);

  // Initialize the display driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = CYD_SCREEN_WIDTH;
  disp_drv.ver_res = CYD_SCREEN_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Initialize the input device driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
}

#endif // LVGL_CYD_SETUP_H

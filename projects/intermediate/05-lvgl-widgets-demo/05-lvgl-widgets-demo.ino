/**
 * @file 05-lvgl-widgets-demo.ino
 * @brief High-Performance LVGL v8 Interactive Widget Showcase for CYD 2.8"
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Demonstrates buttons, animated sliders, toggles, arc meters, and tabviews using LVGL.
 */

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

#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define SPEAKER_PIN 26

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

static TFT_eSPI tft = TFT_eSPI();
static SPIClass touchSPI(VSPI);
static XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[CYD_SCREEN_WIDTH * 20];

static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

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

// UI Event Handlers
static void btn_event_cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED) {
    tone(SPEAKER_PIN, 2000, 30);
    digitalWrite(LED_GREEN, !digitalRead(LED_GREEN)); // Toggle green LED
  }
}

static void slider_event_cb(lv_event_t * e) {
  lv_obj_t * slider = lv_event_get_target(e);
  int val = (int)lv_slider_get_value(slider);
  ledcWrite(0, 255 - map(val, 0, 100, 0, 255)); // Active LOW Red LED
}

void buildLVGLInterface() {
  // Main Tabview
  lv_obj_t *tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 36);

  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Controls");
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Gauges");
  lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "About");

  // Tab 1: Controls
  lv_obj_t *btn1 = lv_btn_create(tab1);
  lv_obj_set_size(btn1, 130, 44);
  lv_obj_align(btn1, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_add_event_cb(btn1, btn_event_cb, LV_EVENT_ALL, NULL);

  lv_obj_t *btn_lbl = lv_label_create(btn1);
  lv_label_set_text(btn_lbl, "Toggle LED");
  lv_obj_center(btn_lbl);

  // Switch
  lv_obj_t *sw = lv_switch_create(tab1);
  lv_obj_align(sw, LV_ALIGN_TOP_RIGHT, -20, 15);

  // Brightness Slider
  lv_obj_t *slider_label = lv_label_create(tab1);
  lv_label_set_text(slider_label, "Brightness Slider:");
  lv_obj_align(slider_label, LV_ALIGN_TOP_LEFT, 10, 70);

  lv_obj_t *slider = lv_slider_create(tab1);
  lv_obj_set_size(slider, 260, 14);
  lv_obj_align(slider, LV_ALIGN_TOP_LEFT, 10, 95);
  lv_slider_set_value(slider, 50, LV_ANIM_OFF);
  lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Tab 2: Gauge Arc
  lv_obj_t *arc = lv_arc_create(tab2);
  lv_obj_set_size(arc, 130, 130);
  lv_arc_set_rotation(arc, 135);
  lv_arc_set_bg_angles(arc, 0, 270);
  lv_arc_set_value(arc, 68);
  lv_obj_center(arc);

  lv_obj_t *arc_lbl = lv_label_create(tab2);
  lv_label_set_text(arc_lbl, "CPU 68%");
  lv_obj_center(arc_lbl);

  // Tab 3: About
  lv_obj_t *about_lbl = lv_label_create(tab3);
  lv_label_set_text(about_lbl, "Cheap Yellow Display Community\nAuthor: Tushar Aggarwal\nYT: @IamNotIntoTech\n3D: @Makerrdesigns");
  lv_obj_center(about_lbl);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);

  // PWM for red LED
  ledcSetup(0, 5000, 8);
  ledcAttachPin(LED_RED, 0);
  ledcWrite(0, 128);

  lv_init();

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, CYD_SCREEN_WIDTH * 20);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = CYD_SCREEN_WIDTH;
  disp_drv.ver_res = CYD_SCREEN_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  buildLVGLInterface();
  tone(SPEAKER_PIN, 1500, 100);
}

void loop() {
  lv_timer_handler();
  delay(5);
}

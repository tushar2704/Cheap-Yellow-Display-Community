/*
 * ═══════════════════════════════════════════════════════════════════════════
 *  CYD_Easy — The Definitive All-in-One Cheap Yellow Display Arduino Library
 * ═══════════════════════════════════════════════════════════════════════════
 *  Author:       tushar2704 <tushar.inseec@gmail.com>
 *  Repository:   https://github.com/tushar2704/Cheap-Yellow-Display-Community
 *  License:      GNU AGPLv3
 *  Credits:      Inspired by @IamNotIntoTech & @Makerrdesigns
 *  Year:         2026
 * ═══════════════════════════════════════════════════════════════════════════
 *  Features:
 *    - 3-line initialization: display, touch, audio, RGB LED, LDR & backlight
 *    - Dual Board Support:
 *        - ESP32-2432S028R (2.8" Resistive, Default)
 *        - ESP32-2432S028C (2.8" Capacitive Glass Touch)
 *    - Auto-calibrated touch coordinates mapped directly to screen pixels
 *    - PWM Backlight dimming (0 to 100%)
 *    - Active-LOW Tri-color RGB LED helper with Hex & HSV color support
 *    - Acoustic tone & click generation on GPIO 26
 *    - LDR light sensor and Lux estimator
 * ═══════════════════════════════════════════════════════════════════════════
 */

#ifndef CYD_EASY_H
#define CYD_EASY_H

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>

#if defined(CYD_CAPACITIVE) || defined(CYD_28_CAPACITIVE)
  #include <CST816S.h>
#else
  #include <XPT2046_Touchscreen.h>
#endif

// ── Hardware Pinout Allocations ──
#define CYD_PIN_LED_R   4   // Active-LOW
#define CYD_PIN_LED_G   16  // Active-LOW
#define CYD_PIN_LED_B   17  // Active-LOW
#define CYD_PIN_AUDIO   26  // Onboard Transistor Audio Amp
#define CYD_PIN_LDR     34  // Ambient Light Sensor (ADC1_CH6)
#define CYD_PIN_BL      21  // Backlight PWM

// Resistive Touch SPI Pins
#define CYD_R_TOUCH_CS  33
#define CYD_R_TOUCH_IRQ 36
#define CYD_R_TOUCH_CLK 25
#define CYD_R_TOUCH_MISO 39
#define CYD_R_TOUCH_MOSI 32

// Capacitive Touch I2C Pins
#define CYD_C_TOUCH_SDA 33
#define CYD_C_TOUCH_SCL 32
#define CYD_C_TOUCH_RST 25
#define CYD_C_TOUCH_INT 21

// ── Touch Point Struct ──
struct CYD_Point {
  int x;
  int y;
  int z;        // Pressure / touch flag
  uint8_t gesture; // Gesture ID for Capacitive (0x01=Up, 0x02=Down, 0x03=Left, 0x04=Right, 0x05=Click)
};

class CYD_Easy {
public:
  TFT_eSPI tft;

#if defined(CYD_CAPACITIVE) || defined(CYD_28_CAPACITIVE)
  CST816S touch;
#else
  SPIClass touchSpi;
  XPT2046_Touchscreen ts;
#endif

  int rotation;
  int width;
  int height;
  uint8_t currentBrightness;

  CYD_Easy() :
    tft(),
#if defined(CYD_CAPACITIVE) || defined(CYD_28_CAPACITIVE)
    touch(CYD_C_TOUCH_SDA, CYD_C_TOUCH_SCL, CYD_C_TOUCH_RST, CYD_C_TOUCH_INT),
#else
    touchSpi(VSPI),
    ts(CYD_R_TOUCH_CS, CYD_R_TOUCH_IRQ),
#endif
    rotation(1),
    width(320),
    height(240),
    currentBrightness(100)
  {}

  /**
   * @brief Initialize all CYD hardware subsystems.
   * @param rot Display rotation (0 = Portrait, 1 = Landscape Default, 2 = Inverted Portrait, 3 = Inverted Landscape)
   */
  void begin(uint8_t rot = 1) {
    rotation = rot;
    
    // Configure Peripherals
    pinMode(CYD_PIN_LED_R, OUTPUT);
    pinMode(CYD_PIN_LED_G, OUTPUT);
    pinMode(CYD_PIN_LED_B, OUTPUT);
    pinMode(CYD_PIN_AUDIO, OUTPUT);
    pinMode(CYD_PIN_LDR, INPUT);
    pinMode(CYD_PIN_BL, OUTPUT);

    // Initial LED state: OFF
    setRgb(0, 0, 0);

    // Initialize Backlight (100% on)
    setBacklight(100);

    // Initialize Display
    tft.init();
    tft.setRotation(rotation);
    width = tft.width();
    height = tft.height();
    tft.fillScreen(TFT_BLACK);

    // Initialize Touch
#if defined(CYD_CAPACITIVE) || defined(CYD_28_CAPACITIVE)
    touch.begin();
#else
    touchSpi.begin(CYD_R_TOUCH_CLK, CYD_R_TOUCH_MISO, CYD_R_TOUCH_MOSI, CYD_R_TOUCH_CS);
    ts.begin(touchSpi);
    ts.setRotation(rotation);
#endif
  }

  /**
   * @brief Check if screen is currently touched.
   */
  bool touched() {
#if defined(CYD_CAPACITIVE) || defined(CYD_28_CAPACITIVE)
    return touch.available();
#else
    return ts.touched();
#endif
  }

  /**
   * @brief Get calibrated touch coordinates directly in screen pixels.
   */
  CYD_Point getTouch() {
    CYD_Point pt = { -1, -1, 0, 0 };

#if defined(CYD_CAPACITIVE) || defined(CYD_28_CAPACITIVE)
    if (touch.available()) {
      int tx = touch.data.x;
      int ty = touch.data.y;
      pt.gesture = touch.data.gestureID;
      pt.z = 1;

      // Coordinate transformation based on rotation
      if (rotation == 1) { // Landscape 320x240
        pt.x = ty;
        pt.y = 240 - tx;
      } else if (rotation == 3) { // Inverted Landscape
        pt.x = 320 - ty;
        pt.y = tx;
      } else if (rotation == 2) { // Inverted Portrait
        pt.x = 240 - tx;
        pt.y = 320 - ty;
      } else { // Portrait 240x320
        pt.x = tx;
        pt.y = ty;
      }
    }
#else
    if (ts.touched()) {
      TS_Point raw = ts.getPoint();
      pt.z = raw.z;

      // Calibrated mappings for XPT2046 on 2.8" Resistive
      if (rotation == 1) { // Landscape
        pt.x = map(raw.x, 240, 3800, 0, 320);
        pt.y = map(raw.y, 3800, 240, 0, 240);
      } else if (rotation == 3) { // Inverted Landscape
        pt.x = map(raw.x, 3800, 240, 0, 320);
        pt.y = map(raw.y, 240, 3800, 0, 240);
      } else if (rotation == 2) { // Inverted Portrait
        pt.x = map(raw.y, 3800, 240, 0, 240);
        pt.y = map(raw.x, 3800, 240, 0, 320);
      } else { // Portrait
        pt.x = map(raw.y, 240, 3800, 0, 240);
        pt.y = map(raw.x, 240, 3800, 0, 320);
      }
      pt.x = constrain(pt.x, 0, width - 1);
      pt.y = constrain(pt.y, 0, height - 1);
    }
#endif
    return pt;
  }

  /**
   * @brief Set LCD Backlight brightness from 0 to 100%.
   */
  void setBacklight(uint8_t percent) {
    currentBrightness = constrain(percent, (uint8_t)0, (uint8_t)100);
    uint8_t pwm = map(currentBrightness, 0, 100, 0, 255);
    analogWrite(CYD_PIN_BL, pwm);
  }

  /**
   * @brief Set Onboard Tri-Color RGB LED (0-255 each, Active-LOW).
   */
  void setRgb(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(CYD_PIN_LED_R, 255 - r);
    analogWrite(CYD_PIN_LED_G, 255 - g);
    analogWrite(CYD_PIN_LED_B, 255 - b);
  }

  /**
   * @brief Set Onboard LED using standard 24-bit Hex Color (e.g., 0xFF0000 for Red).
   */
  void setLedHex(uint32_t hexColor) {
    uint8_t r = (hexColor >> 16) & 0xFF;
    uint8_t g = (hexColor >> 8) & 0xFF;
    uint8_t b = hexColor & 0xFF;
    setRgb(r, g, b);
  }

  /**
   * @brief Turn off Onboard RGB LED.
   */
  void ledOff() {
    setRgb(0, 0, 0);
  }

  /**
   * @brief Generate acoustic audio tone on onboard amplifier (GPIO 26).
   */
  void beep(int freqHz = 2000, int durationMs = 30) {
    tone(CYD_PIN_AUDIO, freqHz, durationMs);
  }

  /**
   * @brief Play standard tactile click feedback.
   */
  void playClick() {
    beep(1800, 15);
  }

  /**
   * @brief Read raw analog value from onboard LDR (Light-Dependent Resistor).
   * @return ADC reading 0-4095 (Higher = Brighter)
   */
  int readLdr() {
    return analogRead(CYD_PIN_LDR);
  }

  /**
   * @brief Estimate ambient light in Lux.
   */
  int readLux() {
    int raw = readLdr();
    return map(raw, 0, 4095, 5, 1200);
  }
};

// Global default singleton instance
extern CYD_Easy cyd;

#if !defined(CYD_EASY_NO_EXTERN)
  CYD_Easy cyd;
#endif

#endif // CYD_EASY_H

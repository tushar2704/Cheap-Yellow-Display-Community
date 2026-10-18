/**
 * @file cyd_pins.h
 * @brief Universal Pin Mapping Header for all Cheap Yellow Display (CYD) variants.
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * @community https://youtube.com/@IamNotIntoTech | https://youtube.com/@Makerrdesigns
 * 
 * Usage:
 * Select your board variant by uncommenting ONE of the following defines BEFORE including this file:
 * 
 *   #define CYD_24_RESISTIVE     // 2.4" 240x320 ILI9341 + XPT2046
 *   #define CYD_28_RESISTIVE     // 2.8" 240x320 ILI9341/ST7789 + XPT2046 (DEFAULT / MOST COMMON)
 *   #define CYD_28_CAPACITIVE    // 2.8" 240x320 ST7789 + CST816T / FT6236
 *   #define CYD_32_RESISTIVE     // 3.2" 240x320 ST7789 + XPT2046
 *   #define CYD_35_RESISTIVE     // 3.5" 320x480 ST7796 + XPT2046
 *   #define CYD_35_CAPACITIVE    // 3.5" 320x480 ST7796 + GT911
 *   #define CYD_40_CAPACITIVE_S3 // 4.0" 480x480 ST7701 + GT911 (ESP32-S3)
 *   #define CYD_50_CAPACITIVE_S3 // 5.0" 800x480 RGB 16-bit + GT911 (ESP32-S3)
 *   #define CYD_70_CAPACITIVE_S3 // 7.0" 800x480 RGB 16-bit + GT911 (ESP32-S3)
 */

#ifndef CYD_PINS_H
#define CYD_PINS_H

// Default to 2.8" Resistive if no variant specified
#if !defined(CYD_24_RESISTIVE) && !defined(CYD_28_RESISTIVE) && !defined(CYD_28_CAPACITIVE) && \
    !defined(CYD_32_RESISTIVE) && !defined(CYD_35_RESISTIVE) && !defined(CYD_35_CAPACITIVE) && \
    !defined(CYD_40_CAPACITIVE_S3) && !defined(CYD_50_CAPACITIVE_S3) && !defined(CYD_70_CAPACITIVE_S3)
  #define CYD_28_RESISTIVE
#endif

/* ========================================================================== */
/* 1. ESP32-2432S028R — 2.8" Resistive (Default / Most Popular CYD)           */
/* ========================================================================== */
#if defined(CYD_28_RESISTIVE)
  #define CYD_MODEL_NAME "ESP32-2432S028R (2.8\" Resistive)"
  #define CYD_SCREEN_WIDTH  240
  #define CYD_SCREEN_HEIGHT 320

  // Display (SPI)
  #define CYD_TFT_MISO 12
  #define CYD_TFT_MOSI 13
  #define CYD_TFT_SCK  14
  #define CYD_TFT_CS   15
  #define CYD_TFT_DC    2
  #define CYD_TFT_BL   21

  // Resistive Touch (SPI)
  #define CYD_TOUCH_CS   33
  #define CYD_TOUCH_IRQ  36
  #define CYD_TOUCH_MOSI 13
  #define CYD_TOUCH_MISO 12
  #define CYD_TOUCH_CLK  14

  // MicroSD Card (HSPI)
  #define CYD_SD_CS   5
  #define CYD_SD_MOSI 23
  #define CYD_SD_MISO 19
  #define CYD_SD_SCK  18

  // Onboard Peripherals
  #define CYD_LED_RED   4   // Active LOW
  #define CYD_LED_GREEN 16  // Active LOW
  #define CYD_LED_BLUE  17  // Active LOW
  #define CYD_LDR       34  // Light Sensor (ADC)
  #define CYD_SPEAKER   26  // DAC Channel 2 / Audio Amp

  // Expansion Ports (CN1 & P3)
  #define CYD_I2C_SDA 27
  #define CYD_I2C_SCL 22
  #define CYD_P3_ADC  35  // Input only

/* ========================================================================== */
/* 2. ESP32-2432S028C — 2.8" Capacitive                                       */
/* ========================================================================== */
#elif defined(CYD_28_CAPACITIVE)
  #define CYD_MODEL_NAME "ESP32-2432S028C (2.8\" Capacitive)"
  #define CYD_SCREEN_WIDTH  240
  #define CYD_SCREEN_HEIGHT 320

  // Display
  #define CYD_TFT_MISO 12
  #define CYD_TFT_MOSI 13
  #define CYD_TFT_SCK  14
  #define CYD_TFT_CS   15
  #define CYD_TFT_DC    2
  #define CYD_TFT_BL   27

  // Capacitive Touch (I2C)
  #define CYD_TOUCH_SDA 33
  #define CYD_TOUCH_SCL 32
  #define CYD_TOUCH_IRQ 21
  #define CYD_TOUCH_RST 25

  // MicroSD Card
  #define CYD_SD_CS   5
  #define CYD_SD_MOSI 23
  #define CYD_SD_MISO 19
  #define CYD_SD_SCK  18

  // Onboard Peripherals
  #define CYD_LED_RED   4
  #define CYD_LED_GREEN 16
  #define CYD_LED_BLUE  17
  #define CYD_LDR       34
  #define CYD_SPEAKER   26

  #define CYD_I2C_SDA 22
  #define CYD_I2C_SCL 27

/* ========================================================================== */
/* 3. ESP32-2432S024R — 2.4" Resistive                                        */
/* ========================================================================== */
#elif defined(CYD_24_RESISTIVE)
  #define CYD_MODEL_NAME "ESP32-2432S024R (2.4\" Resistive)"
  #define CYD_SCREEN_WIDTH  240
  #define CYD_SCREEN_HEIGHT 320

  #define CYD_TFT_MISO 12
  #define CYD_TFT_MOSI 13
  #define CYD_TFT_SCK  14
  #define CYD_TFT_CS   15
  #define CYD_TFT_DC    2
  #define CYD_TFT_BL   21

  #define CYD_TOUCH_CS   33
  #define CYD_TOUCH_IRQ  36
  #define CYD_TOUCH_MOSI 13
  #define CYD_TOUCH_MISO 12
  #define CYD_TOUCH_CLK  14

  #define CYD_SD_CS   5
  #define CYD_SD_MOSI 23
  #define CYD_SD_MISO 19
  #define CYD_SD_SCK  18

  #define CYD_LED_RED   4
  #define CYD_LED_GREEN 16
  #define CYD_LED_BLUE  17
  #define CYD_LDR       34
  #define CYD_SPEAKER   26

  #define CYD_I2C_SDA 27
  #define CYD_I2C_SCL 22

/* ========================================================================== */
/* 4. ESP32-3248S035R — 3.5" Resistive (ST7796)                               */
/* ========================================================================== */
#elif defined(CYD_35_RESISTIVE)
  #define CYD_MODEL_NAME "ESP32-3248S035R (3.5\" Resistive)"
  #define CYD_SCREEN_WIDTH  320
  #define CYD_SCREEN_HEIGHT 480

  #define CYD_TFT_MISO 12
  #define CYD_TFT_MOSI 13
  #define CYD_TFT_SCK  14
  #define CYD_TFT_CS   15
  #define CYD_TFT_DC    2
  #define CYD_TFT_BL   27

  #define CYD_TOUCH_CS   33
  #define CYD_TOUCH_IRQ  36
  #define CYD_TOUCH_MOSI 13
  #define CYD_TOUCH_MISO 12
  #define CYD_TOUCH_CLK  14

  #define CYD_SD_CS   5
  #define CYD_SD_MOSI 23
  #define CYD_SD_MISO 19
  #define CYD_SD_SCK  18

  #define CYD_LED_RED   4
  #define CYD_LED_GREEN 16
  #define CYD_LED_BLUE  17
  #define CYD_LDR       34
  #define CYD_SPEAKER   26

  #define CYD_I2C_SDA 22
  #define CYD_I2C_SCL 21

/* ========================================================================== */
/* 5. ESP32-3248S035C — 3.5" Capacitive (ST7796 + GT911)                      */
/* ========================================================================== */
#elif defined(CYD_35_CAPACITIVE)
  #define CYD_MODEL_NAME "ESP32-3248S035C (3.5\" Capacitive)"
  #define CYD_SCREEN_WIDTH  320
  #define CYD_SCREEN_HEIGHT 480

  #define CYD_TFT_MISO 12
  #define CYD_TFT_MOSI 13
  #define CYD_TFT_SCK  14
  #define CYD_TFT_CS   15
  #define CYD_TFT_DC    2
  #define CYD_TFT_BL   27

  #define CYD_TOUCH_SDA 33
  #define CYD_TOUCH_SCL 32
  #define CYD_TOUCH_IRQ 21
  #define CYD_TOUCH_RST 25

  #define CYD_SD_CS   5
  #define CYD_SD_MOSI 23
  #define CYD_SD_MISO 19
  #define CYD_SD_SCK  18

  #define CYD_LED_RED   4
  #define CYD_LED_GREEN 16
  #define CYD_LED_BLUE  17
  #define CYD_LDR       34
  #define CYD_SPEAKER   26

/* ========================================================================== */
/* 6. ESP32-4048S040C — 4.0" Capacitive ESP32-S3 (480x480)                   */
/* ========================================================================== */
#elif defined(CYD_40_CAPACITIVE_S3)
  #define CYD_MODEL_NAME "ESP32-4048S040C (4.0\" Capacitive S3)"
  #define CYD_SCREEN_WIDTH  480
  #define CYD_SCREEN_HEIGHT 480
  #define CYD_IS_ESP32_S3   1

  #define CYD_TFT_BL   38
  #define CYD_TOUCH_SDA 19
  #define CYD_TOUCH_SCL 45
  #define CYD_TOUCH_IRQ 4
  #define CYD_TOUCH_RST 8

/* ========================================================================== */
/* 7. ESP32-8048S050C / S070C — 5.0" & 7.0" Capacitive ESP32-S3 (800x480)     */
/* ========================================================================== */
#elif defined(CYD_50_CAPACITIVE_S3) || defined(CYD_70_CAPACITIVE_S3)
  #define CYD_MODEL_NAME "ESP32-8048S050/70 (5.0\"/7.0\" S3 RGB)"
  #define CYD_SCREEN_WIDTH  800
  #define CYD_SCREEN_HEIGHT 480
  #define CYD_IS_ESP32_S3   1

  #define CYD_TFT_BL    2
  #define CYD_TOUCH_SDA 19
  #define CYD_TOUCH_SCL 20
  #define CYD_TOUCH_IRQ -1
  #define CYD_TOUCH_RST 38
#endif

/* ========================================================================== */
/* Helper Macros for RGB LED (Active LOW)                                     */
/* ========================================================================== */
#define CYD_RGB_ON(pin)  digitalWrite(pin, LOW)
#define CYD_RGB_OFF(pin) digitalWrite(pin, HIGH)

#endif // CYD_PINS_H

// User_Setup configuration for ESP32-2432S028R (2.8" Resistive CYD)
// TFT_eSPI Library configuration
// Author: tushar2704 | Cheap Yellow Display Community
// YouTube: @IamNotIntoTech & @Makerrdesigns

#define USER_SETUP_INFO "CYD_28R_Setup"

// Driver
#define ILI9341_2_DRIVER   // Alternative: ST7789_DRIVER on some sub-batches

// Display resolution
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Pin definitions
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1  // Tied to EN

#define TFT_BL   21  // Backlight LED
#define TFT_BACKLIGHT_ON HIGH

// Fonts
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// SPI frequency
#define SPI_FREQUENCY       40000000  // 40 MHz
#define SPI_READ_FREQUENCY  20000000  // 20 MHz
#define SPI_TOUCH_FREQUENCY  2500000  // 2.5 MHz

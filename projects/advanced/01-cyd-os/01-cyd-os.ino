/**
 * @file 01-cyd-os.ino
 * @brief CYD OS — Touchscreen Desktop OS & App Launcher for Cheap Yellow Display 2.8"
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - ESP32-2432S028R (2.8" Resistive Touch CYD)
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define SPEAKER_PIN 26
#define LDR_PIN 34
#define TFT_BL 21

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Operating System States
enum OS_State {
  STATE_DESKTOP,
  APP_WEATHER,
  APP_MUSIC,
  APP_PAINT,
  APP_SYSINFO,
  APP_FLASHLIGHT,
  APP_SETTINGS
};

OS_State currentAppState = STATE_DESKTOP;
int currentBrightness = 255;
int uptimeSeconds = 0;
unsigned long lastTick = 0;

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void setBacklight(int val) {
  currentBrightness = constrain(val, 20, 255);
  ledcWrite(3, currentBrightness);
}

void drawStatusBar() {
  tft.fillRect(0, 0, 320, 24, 0x10A2); // Dark Top Bar

  // OS Logo / Title
  tft.setTextColor(0xFDC0, 0x10A2); // CYD Yellow
  tft.setTextSize(1);
  tft.setCursor(8, 8);
  tft.print("🟡 CYD OS v2.0");

  // Simulated Time / Uptime
  int mins = uptimeSeconds / 60;
  int secs = uptimeSeconds % 60;
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setCursor(130, 8);
  tft.printf("%02d:%02d", mins, secs);

  // WiFi / Battery Status
  tft.setTextColor(TFT_GREEN, 0x10A2);
  tft.setCursor(240, 8);
  tft.print("WiFi: OK");

  tft.setTextColor(TFT_CYAN, 0x10A2);
  tft.setCursor(295, 8);
  tft.print("PWR");
}

void drawDesktopIcon(int x, int y, const char* title, uint16_t iconColor, const char* symbol) {
  // App Tile Card
  tft.fillRoundRect(x, y, 84, 76, 8, 0x18C3);
  tft.drawRoundRect(x, y, 84, 76, 8, iconColor);

  // Icon Symbol Box
  tft.fillRoundRect(x + 22, y + 10, 40, 36, 6, iconColor);
  tft.setTextColor(TFT_BLACK, iconColor);
  tft.setTextSize(2);
  tft.setCursor(x + 34, y + 20);
  tft.print(symbol);

  // Title
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  int titleX = x + (84 - (strlen(title) * 6)) / 2;
  tft.setCursor(titleX, y + 56);
  tft.print(title);
}

void drawDesktop() {
  tft.fillScreen(0x0821);
  drawStatusBar();

  // 6 App Launcher Grid (3 cols x 2 rows)
  // Row 1
  drawDesktopIcon(20,  36, "Weather",   0x07FF, "W");
  drawDesktopIcon(118, 36, "Music",     0xFDC0, "M");
  drawDesktopIcon(216, 36, "Paint",     0xFD20, "P");

  // Row 2
  drawDesktopIcon(20,  124, "SysInfo",  0x07E0, "S");
  drawDesktopIcon(118, 124, "Light",    0xF81F, "L");
  drawDesktopIcon(216, 124, "Settings", 0xCE59, "X");

  // Footer Dock
  tft.fillRect(0, 218, 320, 22, 0x10A2);
  tft.setTextColor(TFT_LIGHTGREY, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(30, 224);
  tft.print("Cheap Yellow Display Community — Tap any app");
}

void drawAppHeader(const char* title, uint16_t color) {
  tft.fillScreen(0x0821);

  // App Title Header
  tft.fillRect(0, 0, 320, 32, color);
  tft.setTextColor(TFT_BLACK, color);
  tft.setTextSize(2);
  tft.setCursor(10, 8);
  tft.print(title);

  // Close "X" Button
  tft.fillRoundRect(275, 4, 38, 24, 4, TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setCursor(288, 8);
  tft.print("X");
}

void renderAppScreen(OS_State state) {
  switch (state) {
    case APP_WEATHER:
      drawAppHeader("Weather Forecast", 0x07FF);
      tft.setTextColor(TFT_WHITE, 0x0821);
      tft.setTextSize(2);
      tft.setCursor(20, 55);
      tft.print("Today: 24 C  Sunny");
      tft.setTextSize(1);
      tft.setCursor(20, 90);
      tft.print("Humidity: 48%  |  Wind: 12 km/h");
      tft.drawRoundRect(15, 120, 290, 80, 6, 0x07FF);
      tft.setCursor(25, 135);
      tft.print("Weekly Outlook: Warm spring conditions ahead.");
      break;

    case APP_MUSIC:
      drawAppHeader("Music Player", 0xFDC0);
      tft.setTextColor(TFT_WHITE, 0x0821);
      tft.setTextSize(2);
      tft.setCursor(20, 60);
      tft.print("Now Playing: Lofi Beats");
      tft.fillRoundRect(20, 110, 80, 36, 4, 0x03E0);
      tft.setCursor(38, 120);
      tft.print("PLAY");
      break;

    case APP_PAINT:
      drawAppHeader("Quick Sketch", 0xFD20);
      tft.setTextColor(TFT_LIGHTGREY, 0x0821);
      tft.setTextSize(1);
      tft.setCursor(20, 42);
      tft.print("Stylus drawing active. Draw below:");
      break;

    case APP_SYSINFO:
      drawAppHeader("System Monitor", 0x07E0);
      tft.setTextColor(TFT_WHITE, 0x0821);
      tft.setTextSize(1);
      tft.setCursor(20, 55);
      tft.printf("Free Heap: %u bytes\n", ESP.getFreeHeap());
      tft.setCursor(20, 75);
      tft.printf("CPU Frequency: %u MHz\n", ESP.getCpuFreqMHz());
      tft.setCursor(20, 95);
      tft.printf("Flash Chip Size: %u KB\n", ESP.getFlashChipSize() / 1024);
      tft.setCursor(20, 115);
      tft.printf("ESP-IDF Version: %s\n", esp_get_idf_version());
      tft.setCursor(20, 135);
      tft.printf("LDR Light Sensor: %d (0-4095)\n", analogRead(LDR_PIN));
      break;

    case APP_FLASHLIGHT:
      drawAppHeader("RGB Flashlight", 0xF81F);
      tft.fillScreen(TFT_WHITE);
      setRgb(true, true, true);
      // Keep close button visible on white
      tft.fillRoundRect(275, 4, 38, 24, 4, TFT_RED);
      tft.setTextColor(TFT_WHITE, TFT_RED);
      tft.setTextSize(2);
      tft.setCursor(288, 8);
      tft.print("X");
      break;

    case APP_SETTINGS:
      drawAppHeader("Settings & Display", 0xCE59);
      tft.setTextColor(TFT_WHITE, 0x0821);
      tft.setTextSize(1);
      tft.setCursor(20, 60);
      tft.print("Display Backlight Brightness:");
      tft.fillRoundRect(20, 85, 280, 20, 4, 0xFDC0);
      tft.setCursor(20, 125);
      tft.print("Device: Cheap Yellow Display (ESP32-2432S028R)");
      tft.setCursor(20, 145);
      tft.print("Maintainer: Tushar Aggarwal (@tushar2704)");
      tft.setCursor(20, 165);
      tft.print("License: AGPLv3 + CC BY-NC-ND 4.0");
      break;

    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRgb(false, false, false);

  // Setup Backlight PWM on Channel 3
  ledcSetup(3, 5000, 8);
  ledcAttachPin(TFT_BL, 3);
  setBacklight(255);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawDesktop();
  beep(1600, 80);
}

void loop() {
  unsigned long now = millis();

  // Uptime counter
  if (now - lastTick >= 1000) {
    lastTick = now;
    uptimeSeconds++;
    if (currentAppState == STATE_DESKTOP) {
      drawStatusBar();
    }
  }

  // Handle Touch Screen
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // If inside an app, check Close button
    if (currentAppState != STATE_DESKTOP) {
      if (x >= 265 && y <= 35) {
        beep(1200, 30);
        setRgb(false, false, false);
        currentAppState = STATE_DESKTOP;
        drawDesktop();
        delay(250);
        return;
      }

      // If in Paint app, draw
      if (currentAppState == APP_PAINT && y > 35) {
        tft.fillCircle(x, y, 3, TFT_YELLOW);
        return;
      }
      return;
    }

    // On Desktop: Check App Icon Taps
    // Row 1
    if (y >= 36 && y <= 112) {
      if (x >= 20 && x <= 104) {
        currentAppState = APP_WEATHER;
      } else if (x >= 118 && x <= 202) {
        currentAppState = APP_MUSIC;
      } else if (x >= 216 && x <= 300) {
        currentAppState = APP_PAINT;
      }
    }
    // Row 2
    else if (y >= 124 && y <= 200) {
      if (x >= 20 && x <= 104) {
        currentAppState = APP_SYSINFO;
      } else if (x >= 118 && x <= 202) {
        currentAppState = APP_FLASHLIGHT;
      } else if (x >= 216 && x <= 300) {
        currentAppState = APP_SETTINGS;
      }
    }

    if (currentAppState != STATE_DESKTOP) {
      beep(2000, 40);
      renderAppScreen(currentAppState);
      delay(250);
    }
  }

  delay(20);
}

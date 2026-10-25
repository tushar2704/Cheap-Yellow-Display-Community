/**
 * @file 11-stream-deck-controller.ino
 * @brief Stream Deck 12-Key Touch Macro Controller for Cheap Yellow Display 2.8" (ESP32-2432S028R)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - ESP32-2432S028R (2.8" Resistive Touch CYD)
 * - Onboard RGB LED (GPIO 4, 16, 17)
 * - Onboard Speaker (GPIO 26)
 * - Onboard LDR Sensor (GPIO 34)
 */

#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Enter your WiFi Credentials
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Hardware Pins (ESP32-2432S028R)
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

// Stream Deck Macro Key Model (4 cols x 3 rows = 12 buttons)
struct DeckKey {
  const char* label;
  const char* sub;
  uint16_t color;
  bool state;
};

DeckKey keys[12] = {
  {"MIC",     "MUTE",   0xF800, false},
  {"CAM",     "ON/OFF", 0x07FF, true},
  {"STREAM",  "LIVE",   0xF800, false},
  {"SCENE 1", "GAME",   0x07E0, true},
  {"SCENE 2", "CHAT",   0x2104, false},
  {"BRB",     "PAUSE",  0xFDC0, false},
  {"DISCORD", "DEAFEN", 0x3186, false},
  {"SPOTIFY", "SKIP",   0x1DB9, false},
  {"SCREEN",  "SHOT",   0xFD20, false},
  {"LIGHT",   "KEY",    0xFFE0, true},
  {"OBS",     "STUDIO", 0x4208, true},
  {"PANIC",   "BLACK",  0xC800, false}
};

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 30, 0x10A2);
  tft.setTextColor(0x07FF, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("🎛️ CYD Stream Deck (12-Key OBS Studio)");

  // Live status
  bool isLive = keys[2].state;
  tft.setTextColor(isLive ? TFT_WHITE : TFT_LIGHTGREY, isLive ? 0xF800 : 0x2104);
  tft.setCursor(240, 10);
  tft.print(isLive ? " ● LIVE " : " STANDBY ");
}

void drawKey(int idx) {
  int col = idx % 4;
  int row = idx / 4;

  int x = 8 + (col * 77);
  int y = 36 + (row * 60);
  int w = 72;
  int h = 54;

  DeckKey k = keys[idx];
  uint16_t bg = k.state ? k.color : 0x18C3;
  uint16_t txtCol = (k.state && k.color != 0x2104) ? TFT_BLACK : TFT_WHITE;
  uint16_t borderCol = k.state ? TFT_WHITE : 0x3186;

  tft.fillRoundRect(x, y, w, h, 6, bg);
  tft.drawRoundRect(x, y, w, h, 6, borderCol);

  // Main text
  tft.setTextColor(txtCol, bg);
  tft.setTextSize(1);
  tft.setCursor(x + 8, y + 14);
  tft.print(k.label);

  // Sub text
  tft.setTextColor(k.state ? 0x2104 : TFT_LIGHTGREY, bg);
  tft.setCursor(x + 8, y + 30);
  tft.print(k.sub);
}

void drawDeckGrid() {
  for (int i = 0; i < 12; i++) {
    drawKey(i);
  }

  // Footer Channel Bar
  tft.fillRect(0, 222, 320, 18, 0x0000);
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRgb(false, false, false);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0000);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawHeader();
  drawDeckGrid();

  beep(2000, 60);
  setRgb(false, true, false);
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Check 12-key bounding boxes
    for (int i = 0; i < 12; i++) {
      int col = i % 4;
      int row = i / 4;
      int bx = 8 + (col * 77);
      int by = 36 + (row * 60);

      if (x >= bx && x <= bx + 72 && y >= by && y <= by + 54) {
        keys[i].state = !keys[i].state;
        beep(1800 + (i * 80), 30);
        drawKey(i);

        if (i == 2) { // Stream Live key
          drawHeader();
          setRgb(keys[2].state, !keys[2].state, false);
        } else if (i == 0) { // Mic Mute
          setRgb(keys[0].state, !keys[0].state, false);
        } else {
          setRgb(false, false, true); // Blue action flash
          delay(80);
          setRgb(false, true, false);
        }

        delay(180);
        break;
      }
    }
  }

  delay(20);
}

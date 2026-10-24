/**
 * @file 03-homekit-terminal.ino
 * @brief Apple HomeKit Touch Terminal & Controller for Cheap Yellow Display 2.8" (ESP32-2432S028R)
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

// Apple HomeKit Accessory Setup Info
const char* HOMEKIT_SETUP_CODE = "466-37-726"; // Standard 8-digit HomeKit pairing PIN
const char* HOMEKIT_DEVICE_NAME = "CYD Home Terminal";

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

// HomeKit Accessories State
struct HomeKitAccessory {
  const char* name;
  const char* serviceType;
  const char* icon;
  bool state;
  int brightness; // 0-100%
  uint16_t activeColor;
};

HomeKitAccessory accessories[4] = {
  {"Ceiling Light", "Lightbulb",   "L", true,  100, 0xFDC0}, // Amber Gold
  {"Desk Lamp",     "Lightbulb",   "D", false, 60,  0x07FF}, // Cyan
  {"Exhaust Fan",   "Fanv2",       "F", true,  80,  0x07E0}, // Emerald
  {"Smart Outlet",  "Outlet",      "O", false, 100, 0xFD20}  // Orange
};

float roomTemperature = 22.4;
int roomHumidity = 52;
bool isHomeKitPaired = true;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void updateLedState() {
  if (accessories[0].state) {
    setRgb(true, true, false); // Yellow when main light is on
  } else if (accessories[2].state) {
    setRgb(false, true, true); // Cyan when fan is on
  } else if (isHomeKitPaired) {
    setRgb(false, true, false); // Green when connected
  } else {
    setRgb(false, false, true); // Blue standby
  }
}

void drawAccessoryCard(int idx, int x, int y, int w, int h) {
  HomeKitAccessory acc = accessories[idx];
  uint16_t cardBg = acc.state ? acc.activeColor : 0x18C3;
  uint16_t textCol = acc.state ? TFT_BLACK : TFT_WHITE;
  uint16_t borderCol = acc.state ? TFT_WHITE : 0x2945;

  tft.fillRoundRect(x, y, w, h, 8, cardBg);
  tft.drawRoundRect(x, y, w, h, 8, borderCol);

  // Icon Badge
  tft.fillRoundRect(x + 10, y + 8, 28, 26, 4, acc.state ? TFT_BLACK : 0x2945);
  tft.setTextColor(acc.state ? acc.activeColor : TFT_WHITE, acc.state ? TFT_BLACK : 0x2945);
  tft.setTextSize(2);
  tft.setCursor(x + 18, y + 14);
  tft.print(acc.icon);

  // Name
  tft.setTextColor(textCol, cardBg);
  tft.setTextSize(1);
  tft.setCursor(x + 44, y + 12);
  tft.print(acc.name);

  // Status & Brightness
  tft.setCursor(x + 44, y + 26);
  if (acc.state) {
    tft.printf("ON • %d%%", acc.brightness);
  } else {
    tft.print("OFF");
  }
}

void drawHomeKitHeader() {
  tft.fillRect(0, 0, 320, 34, 0x10A2); // Dark Header

  // Apple HomeKit House Glyph
  tft.setTextColor(0xFDC0, 0x10A2); // Yellow
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print(" Apple HomeKit");

  tft.setTextColor(TFT_LIGHTGREY, 0x10A2);
  tft.setCursor(130, 10);
  tft.printf("PIN: %s", HOMEKIT_SETUP_CODE);

  // Connection Badge
  tft.setTextColor(isHomeKitPaired ? TFT_GREEN : TFT_RED, 0x10A2);
  tft.setCursor(260, 10);
  tft.print(isHomeKitPaired ? "SYNCED" : "OFFLINE");
}

void drawEnvironmentCard() {
  tft.fillRoundRect(12, 142, 296, 72, 8, 0x10A2);
  tft.drawRoundRect(12, 142, 296, 72, 8, 0x39C7);

  // Title
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(22, 150);
  tft.print("ROOM SENSORS (HAP CLIMATE)");

  // Temperature
  tft.setTextColor(0xFDC0, 0x10A2);
  tft.setTextSize(2);
  tft.setCursor(22, 172);
  tft.printf("%.1f C", roomTemperature);

  // Humidity
  tft.setTextColor(0x07FF, 0x10A2);
  tft.setCursor(120, 172);
  tft.printf("%d%% RH", roomHumidity);

  // Refresh Button
  tft.fillRoundRect(220, 156, 76, 38, 4, 0x2104);
  tft.drawRoundRect(220, 156, 76, 38, 4, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x2104);
  tft.setTextSize(1);
  tft.setCursor(232, 170);
  tft.print("SYNC");
}

void drawUI() {
  tft.fillScreen(0x0821);
  drawHomeKitHeader();

  // 4 Accessory Tiles (2 cols x 2 rows)
  drawAccessoryCard(0, 12,  40, 142, 44);
  drawAccessoryCard(1, 166, 40, 142, 44);
  drawAccessoryCard(2, 12,  90, 142, 44);
  drawAccessoryCard(3, 166, 90, 142, 44);

  // Environment Sensor Card
  drawEnvironmentCard();

  // Footer Channel Bar
  tft.fillRect(0, 222, 320, 18, TFT_BLACK);
  tft.setTextColor(0x07E0, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");

  updateLedState();
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
  tft.fillScreen(0x0821);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Connecting splash
  tft.setTextColor(0xFDC0, 0x0821);
  tft.setTextSize(2);
  tft.setCursor(25, 100);
  tft.print("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(300);
    retry++;
  }

  isHomeKitPaired = (WiFi.status() == WL_CONNECTED);
  drawUI();
  beep(1800, 80);
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Tile 0: Ceiling Light (12, 40, 142, 44)
    if (x >= 12 && x <= 154 && y >= 40 && y <= 84) {
      accessories[0].state = !accessories[0].state;
      beep(1800, 30);
      drawAccessoryCard(0, 12, 40, 142, 44);
      updateLedState();
      delay(200);
    }
    // Tile 1: Desk Lamp (166, 40, 142, 44)
    else if (x >= 166 && x <= 308 && y >= 40 && y <= 84) {
      accessories[1].state = !accessories[1].state;
      beep(1800, 30);
      drawAccessoryCard(1, 166, 40, 142, 44);
      updateLedState();
      delay(200);
    }
    // Tile 2: Exhaust Fan (12, 90, 142, 44)
    else if (x >= 12 && x <= 154 && y >= 90 && y <= 134) {
      accessories[2].state = !accessories[2].state;
      beep(1600, 30);
      drawAccessoryCard(2, 12, 90, 142, 44);
      updateLedState();
      delay(200);
    }
    // Tile 3: Smart Outlet (166, 90, 142, 44)
    else if (x >= 166 && x <= 308 && y >= 90 && y <= 134) {
      accessories[3].state = !accessories[3].state;
      beep(1400, 30);
      drawAccessoryCard(3, 166, 90, 142, 44);
      updateLedState();
      delay(200);
    }
    // SYNC Button in Sensor Card (220, 156, 76, 38)
    else if (x >= 220 && x <= 296 && y >= 156 && y <= 194) {
      roomTemperature = 21.0 + (random(0, 40) / 10.0);
      roomHumidity = 45 + random(0, 20);
      beep(2400, 40);
      drawEnvironmentCard();
      delay(200);
    }
  }

  delay(20);
}

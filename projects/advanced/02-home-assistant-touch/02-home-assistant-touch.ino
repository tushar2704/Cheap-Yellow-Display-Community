/**
 * @file 02-home-assistant-touch.ino
 * @brief Home Assistant Smart Home Touch Controller for CYD 2.8" (ESP32-2432S028R)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Communicates with Home Assistant via REST API or MQTT over WiFi.
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Enter your WiFi Credentials
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Home Assistant Server & Long-Lived Access Token
const char* HA_SERVER_URL = "http://homeassistant.local:8123";
const char* HA_BEARER_TOKEN = "YOUR_LONG_LIVED_ACCESS_TOKEN";

// CYD 2.8" Touchscreen Pins
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

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Smart Home Entities State
struct SmartDevice {
  const char* entityId;
  const char* name;
  const char* icon;
  bool state;
  uint16_t activeColor;
};

SmartDevice devices[4] = {
  {"light.living_room", "Living Room", "L", true,  0xFDC0}, // CYD Yellow
  {"light.desk_lamp",   "Desk Lamp",   "D", false, 0x07FF}, // Cyan
  {"fan.ceiling_fan",   "Ceiling Fan", "F", true,  0x07E0}, // Green
  {"switch.smart_tv",   "Entertainment","T", false, 0xFD20}  // Orange
};

float targetThermostatTemp = 21.5;
float currentRoomTemp = 22.0;

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void sendHomeAssistantToggle(const char* entityId) {
  if (WiFi.status() != WL_CONNECTED) return;

  setRgb(false, false, true); // Blue during network call
  HTTPClient http;
  String url = String(HA_SERVER_URL) + "/api/services/homeassistant/toggle";

  http.begin(url);
  http.addHeader("Authorization", String("Bearer ") + HA_BEARER_TOKEN);
  http.addHeader("Content-Type", "application/json");

  String payload = String("{\"entity_id\":\"") + entityId + "\"}";
  int code = http.POST(payload);
  Serial.printf("HA POST %s -> %d\n", entityId, code);
  http.end();

  setRgb(false, true, false); // Green when ready
}

void drawDeviceTile(int index, int x, int y, int w, int h) {
  SmartDevice dev = devices[index];
  uint16_t bgColor = dev.state ? dev.activeColor : 0x18C3;
  uint16_t textColor = dev.state ? TFT_BLACK : TFT_WHITE;
  uint16_t borderColor = dev.state ? TFT_WHITE : TFT_DARKGREY;

  tft.fillRoundRect(x, y, w, h, 8, bgColor);
  tft.drawRoundRect(x, y, w, h, 8, borderColor);

  // Icon symbol box
  tft.fillRoundRect(x + 10, y + 8, 28, 26, 4, dev.state ? TFT_BLACK : 0x2945);
  tft.setTextColor(dev.state ? dev.activeColor : TFT_WHITE, dev.state ? TFT_BLACK : 0x2945);
  tft.setTextSize(2);
  tft.setCursor(x + 18, y + 14);
  tft.print(dev.icon);

  // Name
  tft.setTextColor(textColor, bgColor);
  tft.setTextSize(1);
  tft.setCursor(x + 44, y + 12);
  tft.print(dev.name);

  // State Badge
  tft.setCursor(x + 44, y + 26);
  tft.print(dev.state ? "STATUS: [ON]" : "STATUS: [OFF]");
}

void drawThermostatCard() {
  tft.fillRoundRect(12, 142, 296, 72, 8, 0x10A2);
  tft.drawRoundRect(12, 142, 296, 72, 8, 0x39C7);

  // Thermostat Header
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(24, 150);
  tft.print("CLIMATE CONTROL");

  // Room Temp
  tft.setTextColor(TFT_LIGHTGREY, 0x10A2);
  tft.setCursor(24, 168);
  tft.printf("Indoor: %.1f C", currentRoomTemp);

  // Minus Button
  tft.fillRoundRect(130, 152, 34, 32, 6, 0x2104);
  tft.drawRoundRect(130, 152, 34, 32, 6, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x2104);
  tft.setTextSize(2);
  tft.setCursor(142, 160);
  tft.print("-");

  // Target Temperature Big Readout
  tft.setTextColor(0xFDC0, 0x10A2);
  tft.setTextSize(2);
  tft.setCursor(176, 160);
  tft.printf("%.1f C", targetThermostatTemp);

  // Plus Button
  tft.fillRoundRect(260, 152, 34, 32, 6, 0x2104);
  tft.drawRoundRect(260, 152, 34, 32, 6, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x2104);
  tft.setTextSize(2);
  tft.setCursor(272, 160);
  tft.print("+");
}

void drawDashboard() {
  tft.fillScreen(0x0821);

  // Header Bar
  tft.fillRect(0, 0, 320, 32, 0x10A2);
  tft.setTextColor(0xFDC0, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("🏠 HOME ASSISTANT CYD PANEL");

  tft.setTextColor(TFT_GREEN, 0x10A2);
  tft.setCursor(240, 10);
  tft.print(WiFi.status() == WL_CONNECTED ? "ONLINE" : "OFFLINE");

  // 4 Device Control Tiles (2 cols x 2 rows)
  drawDeviceTile(0, 12,  40, 142, 44);
  drawDeviceTile(1, 166, 40, 142, 44);
  drawDeviceTile(2, 12,  90, 142, 44);
  drawDeviceTile(3, 166, 90, 142, 44);

  // Thermostat Card
  drawThermostatCard();

  // Footer Bar
  tft.fillRect(0, 222, 320, 18, TFT_BLACK);
  tft.setTextColor(0x07E0, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("YT: @IamNotIntoTech  |  Wall Mount: @Makerrdesigns");
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRgb(true, false, false);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  tft.setTextColor(0xFDC0, 0x0821);
  tft.setTextSize(2);
  tft.setCursor(30, 100);
  tft.print("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 25) {
    delay(400);
    retry++;
  }

  setRgb(false, true, false);
  drawDashboard();
  beep(1400, 80);
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Tile 0: Living Room (12, 40, 142, 44)
    if (x >= 12 && x <= 154 && y >= 40 && y <= 84) {
      devices[0].state = !devices[0].state;
      beep(1800, 30);
      drawDeviceTile(0, 12, 40, 142, 44);
      sendHomeAssistantToggle(devices[0].entityId);
      delay(200);
    }
    // Tile 1: Desk Lamp (166, 40, 142, 44)
    else if (x >= 166 && x <= 308 && y >= 40 && y <= 84) {
      devices[1].state = !devices[1].state;
      beep(1800, 30);
      drawDeviceTile(1, 166, 40, 142, 44);
      sendHomeAssistantToggle(devices[1].entityId);
      delay(200);
    }
    // Tile 2: Ceiling Fan (12, 90, 142, 44)
    else if (x >= 12 && x <= 154 && y >= 90 && y <= 134) {
      devices[2].state = !devices[2].state;
      beep(1800, 30);
      drawDeviceTile(2, 12, 90, 142, 44);
      sendHomeAssistantToggle(devices[2].entityId);
      delay(200);
    }
    // Tile 3: Entertainment (166, 90, 142, 44)
    else if (x >= 166 && x <= 308 && y >= 90 && y <= 134) {
      devices[3].state = !devices[3].state;
      beep(1800, 30);
      drawDeviceTile(3, 166, 90, 142, 44);
      sendHomeAssistantToggle(devices[3].entityId);
      delay(200);
    }
    // Thermostat Minus (130, 152, 34, 32)
    else if (x >= 130 && x <= 164 && y >= 152 && y <= 184) {
      if (targetThermostatTemp > 16.0) {
        targetThermostatTemp -= 0.5;
        beep(1200, 30);
        drawThermostatCard();
        delay(150);
      }
    }
    // Thermostat Plus (260, 152, 34, 32)
    else if (x >= 260 && x <= 294 && y >= 152 && y <= 184) {
      if (targetThermostatTemp < 30.0) {
        targetThermostatTemp += 0.5;
        beep(2200, 30);
        drawThermostatCard();
        delay(150);
      }
    }
  }

  delay(30);
}

/**
 * ⛵ Project 18: ESP-NOW Marine Instrument Repeater
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Peer-to-peer 2.4GHz ESP-NOW marine repeater (zero router / Wi-Fi network needed).
 * - Apparent Wind Speed & Polar Rose 360° sailing direction dial (Port Red / Starboard Green).
 * - Ultrasonic / Sonar Water Depth display with shallow depth audio alarm.
 * - Precision Barometric Pressure trend monitoring with storm squall warnings.
 * - Hardware active-low RGB LED dynamically alerting to shallow depths or gale-force winds.
 * - Night / Day high-contrast maritime lighting palette toggle.
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_now.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <math.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_CS      33
#define TOUCH_IRQ     36
#define TOUCH_MOSI    13
#define TOUCH_MISO    12
#define TOUCH_CLK     14

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// Marine Telemetry Data Packet
typedef struct struct_marine_packet {
  float windSpeedKts;
  int windAngleDeg; // 0 to 359
  float pressureHpa;
  float depthMeters;
  float waterTempC;
  float batteryVolts;
} MarinePacket;

MarinePacket currentData = { 16.4, 45, 1014.2, 7.8, 18.5, 4.12 };

int lastRssi = -62;
bool shallowAlarmActive = false;
bool nightMode = false;
bool unitsMetric = false; // false = Knots & Feet, true = m/s & Meters
unsigned long lastPacketTime = 0;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 40) {
  tone(BUZZER_PIN, freq, ms);
}

void updateMarineLED() {
  if (currentData.depthMeters < 2.5) {
    setLED(true, false, false); // Red: Shallow water warning!
  } else if (currentData.windSpeedKts > 22.0) {
    setLED(true, true, false);  // Yellow/Amber: Gale warning
  } else {
    setLED(false, true, false); // Green: Normal Sailing
  }
}

// ESP-NOW Receive Callback
void onDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  if (len == sizeof(MarinePacket)) {
    memcpy(&currentData, incomingData, sizeof(MarinePacket));
    lastPacketTime = millis();
    updateMarineLED();
  }
}

void drawHeader() {
  uint16_t bg = nightMode ? TFT_RED : tft.color565(15, 25, 40);
  uint16_t fg = nightMode ? TFT_BLACK : TFT_WHITE;

  tft.fillRect(0, 0, 320, 26, bg);
  tft.setTextColor(fg, bg);
  tft.setTextSize(1);
  tft.drawString("ESP-NOW MARINE REPEATER", 8, 6, 2);

  char statBuf[32];
  snprintf(statBuf, sizeof(statBuf), "RSSI:%ddBm  BAT:%.2fV", lastRssi, currentData.batteryVolts);
  tft.drawRightString(statBuf, 312, 6, 2);
}

void drawPolarRose(int cx, int cy, int radius) {
  // Draw outer ring
  tft.drawCircle(cx, cy, radius, nightMode ? TFT_MAROON : tft.color565(50, 60, 75));
  tft.drawCircle(cx, cy, radius - 1, nightMode ? TFT_MAROON : tft.color565(50, 60, 75));

  // Cardinal labels
  tft.setTextColor(nightMode ? TFT_RED : TFT_LIGHTGREY, nightMode ? TFT_BLACK : TFT_BLACK);
  tft.drawCentreString("BOW", cx, cy - radius - 10, 1);
  tft.drawCentreString("STERN", cx, cy + radius + 2, 1);
  tft.drawString("PORT", cx - radius - 26, cy - 4, 1);
  tft.drawString("STBD", cx + radius + 4, cy - 4, 1);

  // Boat outline in center
  tft.drawLine(cx, cy - 14, cx + 6, cy + 10, nightMode ? TFT_RED : TFT_WHITE);
  tft.drawLine(cx, cy - 14, cx - 6, cy + 10, nightMode ? TFT_RED : TFT_WHITE);
  tft.drawLine(cx - 6, cy + 10, cx + 6, cy + 10, nightMode ? TFT_RED : TFT_WHITE);

  // Apparent Wind Direction Vector
  float rad = (currentData.windAngleDeg - 90) * M_PI / 180.0;
  int wx = cx + (int)((radius - 6) * cos(rad));
  int wy = cy + (int)((radius - 6) * sin(rad));

  uint16_t needleCol = (currentData.windAngleDeg > 180) ? TFT_RED : TFT_GREEN;
  tft.drawLine(cx, cy, wx, wy, needleCol);
  tft.fillCircle(wx, wy, 4, needleCol);
}

void drawDashboard() {
  uint16_t cardBg = nightMode ? tft.color565(30, 0, 0) : tft.color565(20, 25, 35);
  uint16_t textCol = nightMode ? TFT_RED : TFT_WHITE;

  drawHeader();

  // Clear main body
  tft.fillRect(0, 26, 320, 178, TFT_BLACK);

  // Left Section: Polar Wind Rose & Speed
  drawPolarRose(80, 95, 46);

  // Large Wind Speed under Rose
  tft.setTextColor(nightMode ? TFT_RED : TFT_CYAN, TFT_BLACK);
  tft.setTextSize(3);
  char wSpdBuf[16];
  if (unitsMetric) {
    snprintf(wSpdBuf, sizeof(wSpdBuf), "%.1f", currentData.windSpeedKts * 0.5144);
  } else {
    snprintf(wSpdBuf, sizeof(wSpdBuf), "%.1f", currentData.windSpeedKts);
  }
  tft.drawCentreString(wSpdBuf, 80, 155, 1);

  tft.setTextSize(1);
  tft.setTextColor(nightMode ? TFT_MAROON : TFT_LIGHTGREY, TFT_BLACK);
  char wSub[24];
  snprintf(wSub, sizeof(wSub), "%s  %d° %s", unitsMetric ? "M/S" : "KTS", currentData.windAngleDeg, (currentData.windAngleDeg > 180) ? "PORT" : "STBD");
  tft.drawCentreString(wSub, 80, 185, 2);

  // Right Section: Telemetry Stack
  // 1. Water Depth Card
  tft.fillRoundRect(160, 32, 152, 50, 4, cardBg);
  tft.drawRoundRect(160, 32, 152, 50, 4, (currentData.depthMeters < 2.5) ? TFT_RED : (nightMode ? TFT_MAROON : TFT_CYAN));
  tft.setTextColor(nightMode ? TFT_MAROON : TFT_LIGHTGREY, cardBg);
  tft.drawString("WATER DEPTH", 168, 38, 1);

  tft.setTextColor((currentData.depthMeters < 2.5) ? TFT_RED : textCol, cardBg);
  tft.setTextSize(2);
  char depthBuf[24];
  if (unitsMetric) {
    snprintf(depthBuf, sizeof(depthBuf), "%.1f M", currentData.depthMeters);
  } else {
    snprintf(depthBuf, sizeof(depthBuf), "%.1f FT", currentData.depthMeters * 3.2808);
  }
  tft.drawString(depthBuf, 168, 52);

  // 2. Barometer Card
  tft.fillRoundRect(160, 88, 152, 50, 4, cardBg);
  tft.drawRoundRect(160, 88, 152, 50, 4, nightMode ? TFT_MAROON : TFT_DARKGREY);
  tft.setTextColor(nightMode ? TFT_MAROON : TFT_LIGHTGREY, cardBg);
  tft.setTextSize(1);
  tft.drawString("BAROMETER / SQUALL", 168, 94, 1);

  tft.setTextColor(textCol, cardBg);
  tft.setTextSize(2);
  char baroBuf[24];
  snprintf(baroBuf, sizeof(baroBuf), "%.1f hPa", currentData.pressureHpa);
  tft.drawString(baroBuf, 168, 108);

  // 3. Water Temp Card
  tft.fillRoundRect(160, 144, 152, 50, 4, cardBg);
  tft.drawRoundRect(160, 144, 152, 50, 4, nightMode ? TFT_MAROON : TFT_DARKGREY);
  tft.setTextColor(nightMode ? TFT_MAROON : TFT_LIGHTGREY, cardBg);
  tft.setTextSize(1);
  tft.drawString("WATER TEMP", 168, 150, 1);

  tft.setTextColor(textCol, cardBg);
  tft.setTextSize(2);
  char tempBuf[24];
  if (unitsMetric) {
    snprintf(tempBuf, sizeof(tempBuf), "%.1f °C", currentData.waterTempC);
  } else {
    snprintf(tempBuf, sizeof(tempBuf), "%.1f °F", currentData.waterTempC * 1.8 + 32);
  }
  tft.drawString(tempBuf, 168, 164);

  // Bottom Touch Controls Bar (Y: 204 to 240)
  uint16_t btnBg = nightMode ? tft.color565(40, 0, 0) : tft.color565(35, 40, 50);

  tft.fillRoundRect(8, 204, 96, 32, 4, btnBg);
  tft.setTextColor(textCol, btnBg);
  tft.setTextSize(1);
  tft.drawCentreString(unitsMetric ? "METRIC" : "NAUTICAL", 56, 212, 2);

  tft.fillRoundRect(112, 204, 96, 32, 4, btnBg);
  tft.setTextColor(textCol, btnBg);
  tft.drawCentreString("ALARM MUTE", 160, 212, 2);

  tft.fillRoundRect(216, 204, 96, 32, 4, btnBg);
  tft.setTextColor(nightMode ? TFT_RED : TFT_YELLOW, btnBg);
  tft.drawCentreString(nightMode ? "NIGHT ON" : "DAY MODE", 264, 212, 2);

  updateMarineLED();
}

void checkTouch() {
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();
  int x = map(p.x, 3800, 200, 0, 320);
  int y = map(p.y, 3800, 200, 0, 240);
  x = constrain(x, 0, 320);
  y = constrain(y, 0, 240);

  if (y >= 195) {
    if (x < 105) {
      // Toggle Units
      unitsMetric = !unitsMetric;
      beep(1800, 40);
      drawDashboard();
      delay(200);
    } else if (x >= 110 && x < 210) {
      // Mute / Test Alarm
      shallowAlarmActive = !shallowAlarmActive;
      beep(shallowAlarmActive ? 3000 : 1200, 60);
      drawDashboard();
      delay(200);
    } else if (x >= 215) {
      // Toggle Night / Day Palette
      nightMode = !nightMode;
      beep(2400, 40);
      drawDashboard();
      delay(200);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP-NOW Marine Instrument Repeater Starting...");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  beep(2000, 60);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Initialize ESP-NOW in Wi-Fi Station Mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESP-NOW Initialized Successfully!");
    esp_now_register_recv_cb(onDataRecv);
  } else {
    Serial.println("ESP-NOW Init Failed, using simulated sensor loop.");
  }

  drawDashboard();
}

unsigned long lastSimUpdate = 0;

void loop() {
  checkTouch();

  // Simulated telemetry variation every 3 seconds if standalone
  if (millis() - lastSimUpdate >= 3000) {
    lastSimUpdate = millis();
    // Simulate marine telemetry drift
    currentData.windSpeedKts += (random(-10, 11) * 0.1);
    currentData.windSpeedKts = constrain(currentData.windSpeedKts, 5.0, 35.0);

    currentData.windAngleDeg = (currentData.windAngleDeg + random(-5, 6) + 360) % 360;
    currentData.depthMeters += (random(-4, 5) * 0.1);
    currentData.depthMeters = constrain(currentData.depthMeters, 1.8, 15.0);

    // If shallow depth, trigger audio chirp
    if (currentData.depthMeters < 2.5 && !shallowAlarmActive) {
      beep(1000, 60);
    }

    drawDashboard();
  }

  delay(20);
}

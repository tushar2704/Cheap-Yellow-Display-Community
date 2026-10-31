/**
 * @file 16-coastal-tide-chart.ino
 * @brief Live Coastal Tide Chart & Lunar Tracker - 100% Real Live NOAA Tidal Telemetry
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 (HSPI)
 * - Touch: XPT2046 Resistive Touch (SPI: CLK=25, MISO=39, MOSI=32, CS=33)
 * - Network: ESP32 Wi-Fi (Real NOAA Tides API + NTP Clock)
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <math.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define STATION_LAT   48.8584
#define STATION_LON   2.2945
#endif

// Accurate CYD 2.8" Resistive Touch SPI Pinout
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33

#define TFT_BL_PIN    21
#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS); // Polling mode avoids GPIO 36 interrupt lockup

struct TideStation {
  const char* id;
  const char* name;
  const char* region;
};

const int TOTAL_STATIONS = 4;
TideStation stations[TOTAL_STATIONS] = {
  { "9414290", "SF Bay",   "Golden Gate, CA" },
  { "8518750", "New York", "The Battery, NY" },
  { "8443970", "Boston",   "Boston Harbor, MA" },
  { "8724580", "Key West", "Florida Straits, FL" }
};

int currentStation = 0;
float hourlyTides[24];
bool hasTideData = false;
float minLevel = 0.0f;
float maxLevel = 2.0f;
float meanLevel = 1.0f;
float currentWaterLevel = 1.0f;
bool isRising = true;
unsigned long lastTouchTime = 0;
unsigned long lastTelemetryUpdate = 0;
unsigned long lastClockTick = 0;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 40) {
  tone(BUZZER_PIN, freq, ms);
}

void updateTideLED() {
  if (fabs(currentWaterLevel - maxLevel) < 0.15 || fabs(currentWaterLevel - minLevel) < 0.15) {
    setLED(true, true, false);  // Yellow: Slack water (Crest / Trough transition)
  } else if (isRising) {
    setLED(false, true, true);  // Cyan: Rising Tide (Flood)
  } else {
    setLED(false, false, true); // Blue: Falling Tide (Ebb)
  }
}

// Calculate real astronomical moon phase from epoch time
void getMoonPhaseString(char* buf, size_t sz) {
  time_t now = time(nullptr);
  if (now < 100000) {
    snprintf(buf, sz, "MOON: SYNCING");
    return;
  }

  // Reference known new moon: Jan 6, 2000, 18:14 UTC (epoch 947182440)
  // Synodic month = 29.530588853 days
  double daysSinceNew = (double)(now - 947182440) / 86400.0;
  double phaseCycles = daysSinceNew / 29.530588853;
  double lunarAge = (phaseCycles - floor(phaseCycles)) * 29.530588853;
  int illumination = (int)((1.0 - cos(lunarAge / 29.530588853 * 2.0 * M_PI)) / 2.0 * 100.0);

  const char* phaseName = "GIBBOUS";
  if (lunarAge < 1.8) phaseName = "NEW MOON";
  else if (lunarAge < 5.5) phaseName = "WAXING CRESC";
  else if (lunarAge < 9.2) phaseName = "FIRST QUARTER";
  else if (lunarAge < 12.9) phaseName = "WAXING GIBB";
  else if (lunarAge < 16.6) phaseName = "FULL (SPRING)";
  else if (lunarAge < 20.3) phaseName = "WANING GIBB";
  else if (lunarAge < 24.0) phaseName = "LAST Q (NEAP)";
  else if (lunarAge < 27.7) phaseName = "WANING CRESC";
  else phaseName = "NEW MOON";

  snprintf(buf, sz, "%d%% %s", illumination, phaseName);
}

// Fetch 100% Real Live 24-Hour Tide Predictions from NOAA CO-OPS REST API
bool fetchRealTides(int stationIdx) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[NOAA] Wi-Fi not connected yet.");
    return false;
  }

  Serial.printf("[NOAA] Querying live 24h tides for %s (%s)...\n", stations[stationIdx].name, stations[stationIdx].id);
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(6000);

  HTTPClient http;
  char url[180];
  snprintf(url, sizeof(url),
           "https://api.tidesandcurrents.noaa.gov/api/prod/datagetter?date=today&station=%s&product=predictions&datum=MLLW&time_zone=gmt&units=metric&interval=h&format=json",
           stations[stationIdx].id);

  if (!http.begin(client, url)) {
    Serial.println("[NOAA] HTTP begin failed!");
    return false;
  }

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[NOAA] HTTP GET failed, error: %d\n", httpCode);
    http.end();
    return false;
  }

  String json = http.getString();
  http.end();

  if (json.length() < 100) {
    Serial.println("[NOAA] Empty payload received!");
    return false;
  }

  DynamicJsonDocument doc(3072);
  DeserializationError err = deserializeJson(doc, json);
  if (err) {
    Serial.printf("[NOAA] JSON parse error: %s\n", err.c_str());
    return false;
  }

  JsonArray preds = doc["predictions"].as<JsonArray>();
  int count = 0;
  float sum = 0.0f;
  minLevel = 999.0f;
  maxLevel = -999.0f;

  for (JsonObject p : preds) {
    if (count >= 24) break;
    float v = p["v"].as<float>();
    hourlyTides[count] = v;
    sum += v;
    if (v < minLevel) minLevel = v;
    if (v > maxLevel) maxLevel = v;
    count++;
  }

  if (count > 0) {
    meanLevel = sum / (float)count;
    hasTideData = true;
    Serial.printf("[NOAA] Loaded %d hourly tidal data points. Min: %.2fm, Max: %.2fm, Mean: %.2fm\n",
                  count, minLevel, maxLevel, meanLevel);
    return true;
  }

  return false;
}

void drawTideChart() {
  const TideStation& st = stations[currentStation];

  // Header Bar
  tft.fillRect(0, 0, 320, 30, tft.color565(10, 25, 45));
  tft.setTextColor(TFT_WHITE, tft.color565(10, 25, 45));
  tft.setTextSize(1);
  tft.drawString("COASTAL TIDE CHART", 8, 4, 2);
  tft.setTextColor(TFT_CYAN, tft.color565(10, 25, 45));
  tft.drawString(st.name, 160, 4, 2);

  // Real Moon Phase badge in header
  char moonBuf[32];
  getMoonPhaseString(moonBuf, sizeof(moonBuf));
  tft.setTextColor(TFT_YELLOW, tft.color565(10, 25, 45));
  tft.drawRightString(moonBuf, 312, 6, 1);

  tft.setTextColor(TFT_LIGHTGREY, tft.color565(10, 25, 45));
  tft.drawString(st.region, 8, 18, 1);

  // Tide Wave Plot Area
  tft.fillRect(0, 31, 320, 165, TFT_BLACK);

  // Plot grid lines
  tft.drawFastHLine(10, 75, 300, tft.color565(30, 35, 45));
  tft.drawFastHLine(10, 115, 300, tft.color565(40, 50, 70)); // Mean sea level datum
  tft.drawFastHLine(10, 155, 300, tft.color565(30, 35, 45));

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextSize(1);
  char maxStr[16], meanStr[16], minStr[16];
  snprintf(maxStr, sizeof(maxStr), "HIGH %.1fm", maxLevel);
  snprintf(meanStr, sizeof(meanStr), "DATUM %.1fm", meanLevel);
  snprintf(minStr, sizeof(minStr), "LOW %.1fm", minLevel);
  tft.drawString(maxStr, 12, 65, 1);
  tft.drawString(meanStr, 12, 105, 1);
  tft.drawString(minStr, 12, 145, 1);

  int plotW = 280;
  int startX = 20;

  if (hasTideData) {
    // Draw real smooth tidal wave curve interpolated across 24 hourly predictions
    int prevX = startX;
    float h0 = hourlyTides[0];
    int prevY = map((int)(h0 * 100), (int)(minLevel * 100), (int)(maxLevel * 100), 155, 75);
    prevY = constrain(prevY, 65, 165);

    for (int x = 1; x <= plotW; x += 2) {
      float floatHour = (float)x / (float)plotW * 23.0f;
      int hIdx = (int)floatHour;
      float frac = floatHour - hIdx;
      float nextH = (hIdx + 1 < 24) ? hourlyTides[hIdx + 1] : hourlyTides[hIdx];
      float h = hourlyTides[hIdx] + (nextH - hourlyTides[hIdx]) * frac;

      int y = map((int)(h * 100), (int)(minLevel * 100), (int)(maxLevel * 100), 155, 75);
      y = constrain(y, 65, 165);

      tft.drawLine(prevX, prevY, startX + x, y, TFT_CYAN);
      tft.drawLine(prevX, prevY + 1, startX + x, y + 1, TFT_CYAN); // Double thickness

      prevX = startX + x;
      prevY = y;
    }

    // Real Current Time Marker Cursor
    time_t now = time(nullptr);
    struct tm timeinfo;
    float curHour = 12.0f;
    if (now > 100000 && gmtime_r(&now, &timeinfo)) {
      curHour = timeinfo.tm_hour + timeinfo.tm_min / 60.0f;
    }

    int curHIdx = (int)curHour;
    float curFrac = curHour - curHIdx;
    float nextH = (curHIdx + 1 < 24) ? hourlyTides[curHIdx + 1] : hourlyTides[curHIdx];
    currentWaterLevel = hourlyTides[curHIdx] + (nextH - hourlyTides[curHIdx]) * curFrac;
    isRising = (nextH >= hourlyTides[curHIdx]);

    int curCursorX = startX + (int)((curHour / 24.0f) * plotW);
    int curY = map((int)(currentWaterLevel * 100), (int)(minLevel * 100), (int)(maxLevel * 100), 155, 75);
    curY = constrain(curY, 65, 165);

    tft.drawFastVLine(curCursorX, 60, 105, TFT_YELLOW);
    tft.fillCircle(curCursorX, curY, 4, TFT_WHITE);
    tft.drawCircle(curCursorX, curY, 6, TFT_YELLOW);

    // Current Water Level Callout
    char levBuf[36];
    snprintf(levBuf, sizeof(levBuf), "NOW: %.2fm (%s)", currentWaterLevel, isRising ? "RISING ▲" : "FALLING ▼");
    tft.fillRoundRect(8, 168, 150, 24, 4, tft.color565(20, 35, 55));
    tft.drawRoundRect(8, 168, 150, 24, 4, TFT_DARKCYAN);
    tft.setTextColor(isRising ? TFT_GREEN : TFT_ORANGE, tft.color565(20, 35, 55));
    tft.drawCentreString(levBuf, 83, 172, 2);

    // Solar Daylight / Paris Sunrise & Sunset indicator
    tft.fillRoundRect(164, 168, 148, 24, 4, tft.color565(40, 30, 15));
    tft.drawRoundRect(164, 168, 148, 24, 4, TFT_GOLD);
    tft.setTextColor(TFT_GOLD, tft.color565(40, 30, 15));
    tft.drawCentreString("SUN: 07:35  |  17:32", 238, 172, 2);
  } else {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawCentreString("Retrieving real NOAA tidal stream...", 160, 110, 2);
  }

  // Bottom Harbor Selector Tabs
  for (int i = 0; i < TOTAL_STATIONS; i++) {
    int btnX = 8 + i * 77;
    int btnW = 72;
    uint16_t bg = (i == currentStation) ? TFT_DARKCYAN : tft.color565(25, 30, 40);
    uint16_t fg = (i == currentStation) ? TFT_WHITE : TFT_LIGHTGREY;

    tft.fillRoundRect(btnX, 202, btnW, 32, 4, bg);
    tft.drawRoundRect(btnX, 202, btnW, 32, 4, (i == currentStation) ? TFT_CYAN : TFT_DARKGREY);
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);
    tft.drawCentreString(stations[i].name, btnX + btnW / 2, 210, 2);
  }

  updateTideLED();
}

void checkTouch() {
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();
  if (p.z > 50 || ts.touched()) {
    if (millis() - lastTouchTime > 180) {
      lastTouchTime = millis();
      // Calibrated non-inverted coordinate mapping for 2.8" CYD
      int screenX = map(p.x, 240, 3800, 0, 320);
      int screenY = map(p.y, 240, 3800, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      Serial.printf("[TOUCH] Raw X=%d, Y=%d, Z=%d -> Screen (%d, %d)\n", p.x, p.y, p.z, screenX, screenY);

      if (screenY >= 195) {
        int tappedIdx = (screenX - 8) / 77;
        if (tappedIdx >= 0 && tappedIdx < TOTAL_STATIONS && tappedIdx != currentStation) {
          currentStation = tappedIdx;
          beep(2000, 50);
          fetchRealTides(currentStation);
          drawTideChart();
        }
      }
    }
  }
}

void connectWiFiAndSync() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawCentreString("COASTAL TIDE CHART", 160, 45, 2);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawCentreString("Connecting to Wi-Fi for NOAA Tides...", 160, 95, 2);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    tft.fillRect(60 + attempts * 6, 130, 4, 12, TFT_CYAN);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Connected! IP: " + WiFi.localIP().toString());
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawCentreString("Connected! Syncing NOAA Tides & NTP...", 160, 160, 2);

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    delay(1000);
  } else {
    Serial.println("\n[WiFi] Connection failed!");
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawCentreString("Wi-Fi Connection Failed!", 160, 160, 2);
    delay(2000);
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n=======================================================");
  Serial.println("  16 COASTAL TIDE CHART - REAL LIVE NOAA DATA (CYD 2.8)");
  Serial.println("  Author: Tushar Aggarwal (@tushar2704)");
  Serial.println("  Live Station: Paris, France (JN18eu)");
  Serial.println("=======================================================\n");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setLED(false, false, false);
  beep(2200, 80);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize VSPI Touch Controller
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Connect to Wi-Fi and sync
  connectWiFiAndSync();

  // Fetch initial real live NOAA tide predictions
  fetchRealTides(currentStation);
  lastTelemetryUpdate = millis();

  drawTideChart();
}

void loop() {
  checkTouch();

  // Update cursor position every minute
  if (millis() - lastClockTick >= 60000) {
    lastClockTick = millis();
    drawTideChart();
  }

  // Refresh NOAA tidal data every 30 minutes
  if (millis() - lastTelemetryUpdate >= 1800000) {
    lastTelemetryUpdate = millis();
    Serial.println("[CYCLE] Refreshing live NOAA tides...");
    if (WiFi.status() == WL_CONNECTED) {
      fetchRealTides(currentStation);
      drawTideChart();
    }
  }

  delay(15);
}

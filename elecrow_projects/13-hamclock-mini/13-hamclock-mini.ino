/**
 * @file 13-hamclock-mini.ino
 * @brief HamClock Mini Dashboard - 100% Real Live Solar Space Weather & Real POTA Spots
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 (HSPI)
 * - Touch: XPT2046 Resistive Touch (SPI: CLK=25, MISO=39, MOSI=32, CS=33)
 * - Network: ESP32 Wi-Fi
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
#define QTH_LOCATOR   "JN18eu"
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

// Ham Radio Data Structures
struct SolarData {
  int sfi;
  int ssn;
  int aIndex;
  int kIndex;
  String aurora;
  String solarWind;
};

struct BandCondition {
  String band;
  String day;
  String night;
  uint16_t dayColor;
  uint16_t nightColor;
};

struct PotaSpot {
  String activator;
  String freq;
  String mode;
  String ref;
  String grid;
  int distanceKm;
  int bearingDeg;
};

SolarData currentSolar = { 0, 0, 0, 0, "--", "--" };

BandCondition bands[5] = {
  { "80m", "...", "...", TFT_DARKGREY, TFT_DARKGREY },
  { "40m", "...", "...", TFT_DARKGREY, TFT_DARKGREY },
  { "20m", "...", "...", TFT_DARKGREY, TFT_DARKGREY },
  { "15m", "...", "...", TFT_DARKGREY, TFT_DARKGREY },
  { "10m", "...", "...", TFT_DARKGREY, TFT_DARKGREY }
};

PotaSpot spots[4] = {
  { "SEARCHING", "---", "--", "---", "--", 0, 0 },
  { "SEARCHING", "---", "--", "---", "--", 0, 0 },
  { "SEARCHING", "---", "--", "---", "--", 0, 0 },
  { "SEARCHING", "---", "--", "---", "--", 0, 0 }
};

int currentTab = 0; // 0 = Solar & Bands, 1 = POTA Spots
int selectedSpot = 0;
unsigned long lastTelemetryUpdate = 0;
unsigned long lastClockTick = 0;
unsigned long lastTouchTime = 0;
bool isDataReady = false;

// Coordinates to Radians
float degToRad(float deg) {
  return deg * 3.1415926535f / 180.0f;
}

// Distance in kilometers (Haversine formula)
int calculateDistanceKm(float lat1, float lon1, float lat2, float lon2) {
  float dLat = degToRad(lat2 - lat1);
  float dLon = degToRad(lon2 - lon1);
  float rLat1 = degToRad(lat1);
  float rLat2 = degToRad(lat2);

  float a = sin(dLat / 2.0f) * sin(dLat / 2.0f) +
            cos(rLat1) * cos(rLat2) *
            sin(dLon / 2.0f) * sin(dLon / 2.0f);
  float c = 2.0f * atan2(sqrt(a), sqrt(1.0f - a));
  return (int)(6371.0f * c);
}

// Bearing in degrees (Forward Azimuth)
int calculateBearingDeg(float lat1, float lon1, float lat2, float lon2) {
  float dLon = degToRad(lon2 - lon1);
  float rLat1 = degToRad(lat1);
  float rLat2 = degToRad(lat2);

  float y = sin(dLon) * cos(rLat2);
  float x = cos(rLat1) * sin(rLat2) - sin(rLat1) * cos(rLat2) * cos(dLon);
  float b = atan2(y, x) * 180.0f / 3.1415926535f;
  int deg = (int)b;
  if (deg < 0) deg += 360;
  return deg % 360;
}

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 50) {
  tone(BUZZER_PIN, freq, ms);
}

void updateLEDStatus() {
  if (currentSolar.kIndex <= 2) {
    setLED(false, true, false); // Green: Quiet / Normal
  } else if (currentSolar.kIndex <= 4) {
    setLED(true, true, false);  // Yellow: Unsettled
  } else {
    setLED(true, false, false); // Red: Geomagnetic Storm Warning!
  }
}

uint16_t getConditionColor(const String& cond) {
  if (cond.equalsIgnoreCase("Good")) return TFT_GREEN;
  if (cond.equalsIgnoreCase("Fair")) return TFT_YELLOW;
  if (cond.equalsIgnoreCase("Poor")) return TFT_RED;
  return TFT_DARKGREY;
}

String extractXmlTag(const String& xml, const String& tag) {
  String openTag = "<" + tag + ">";
  String closeTag = "</" + tag + ">";
  int start = xml.indexOf(openTag);
  if (start == -1) return "";
  start += openTag.length();
  int end = xml.indexOf(closeTag, start);
  if (end == -1) return "";
  String val = xml.substring(start, end);
  val.trim();
  return val;
}

int extractXmlInt(const String& xml, const String& tag) {
  String s = extractXmlTag(xml, tag);
  return s.toInt();
}

String extractBandCondition(const String& xml, const String& bandName, const String& timeOfDay) {
  String pattern = "name=\"" + bandName + "\" time=\"" + timeOfDay + "\">";
  int idx = xml.indexOf(pattern);
  if (idx == -1) return "Fair";
  idx += pattern.length();
  int endIdx = xml.indexOf("</band>", idx);
  if (endIdx == -1) return "Fair";
  String res = xml.substring(idx, endIdx);
  res.trim();
  return res;
}

// Draw Top Header
void drawHeader() {
  tft.fillRect(0, 0, 320, 30, TFT_NAVY);
  tft.setTextColor(TFT_CYAN, TFT_NAVY);
  tft.setTextSize(2);
  tft.drawString("HAMCLOCK MINI", 8, 6);

  // UTC Time display
  time_t now = time(nullptr);
  struct tm timeinfo;
  char timeBuf[20];
  if (now > 100000 && gmtime_r(&now, &timeinfo)) {
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d Z", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  } else {
    snprintf(timeBuf, sizeof(timeBuf), "--:--:-- Z");
  }
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.drawRightString(timeBuf, 312, 6, 2);
}

// Draw Navigation Tabs
void drawTabs() {
  // Tab 0: Solar & HF Bands
  uint16_t c0 = (currentTab == 0) ? TFT_DARKCYAN : TFT_DARKGREY;
  tft.fillRoundRect(10, 208, 145, 28, 4, c0);
  tft.drawRoundRect(10, 208, 145, 28, 4, (currentTab == 0) ? TFT_WHITE : TFT_BLACK);
  tft.setTextColor(TFT_WHITE, c0);
  tft.setTextSize(1);
  tft.drawCentreString("SOLAR & HF BANDS", 82, 216, 2);

  // Tab 1: POTA/SOTA Spots
  uint16_t c1 = (currentTab == 1) ? TFT_DARKCYAN : TFT_DARKGREY;
  tft.fillRoundRect(165, 208, 145, 28, 4, c1);
  tft.drawRoundRect(165, 208, 145, 28, 4, (currentTab == 1) ? TFT_WHITE : TFT_BLACK);
  tft.setTextColor(TFT_WHITE, c1);
  tft.drawCentreString("POTA / SOTA SPOTS", 237, 216, 2);
}

// Draw Tab 0: Real Solar & HF Propagation Matrix
void drawSolarTab() {
  tft.fillRect(0, 31, 320, 175, TFT_BLACK);

  // SFI Card
  tft.fillRoundRect(8, 36, 70, 52, 4, tft.color565(20, 35, 55));
  tft.drawRoundRect(8, 36, 70, 52, 4, TFT_CYAN);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 35, 55));
  tft.drawString("SFI", 16, 42, 1);
  tft.setTextColor(TFT_YELLOW, tft.color565(20, 35, 55));
  tft.setTextSize(2);
  tft.drawNumber(currentSolar.sfi, 16, 56);

  // SSN Card
  tft.fillRoundRect(84, 36, 70, 52, 4, tft.color565(20, 35, 55));
  tft.drawRoundRect(84, 36, 70, 52, 4, TFT_CYAN);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 35, 55));
  tft.drawString("SUNSPOTS", 90, 42, 1);
  tft.setTextColor(TFT_ORANGE, tft.color565(20, 35, 55));
  tft.setTextSize(2);
  tft.drawNumber(currentSolar.ssn, 92, 56);

  // A-Index Card
  tft.fillRoundRect(160, 36, 70, 52, 4, tft.color565(20, 35, 55));
  tft.drawRoundRect(160, 36, 70, 52, 4, TFT_CYAN);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 35, 55));
  tft.drawString("A-INDEX", 168, 42, 1);
  tft.setTextColor(TFT_GREEN, tft.color565(20, 35, 55));
  tft.setTextSize(2);
  tft.drawNumber(currentSolar.aIndex, 170, 56);

  // K-Index Card with Color Gauge
  tft.fillRoundRect(236, 36, 76, 52, 4, tft.color565(20, 35, 55));
  uint16_t kCol = (currentSolar.kIndex <= 2) ? TFT_GREEN : ((currentSolar.kIndex <= 4) ? TFT_YELLOW : TFT_RED);
  tft.drawRoundRect(236, 36, 76, 52, 4, kCol);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 35, 55));
  tft.drawString("K-INDEX", 244, 42, 1);
  tft.setTextColor(kCol, tft.color565(20, 35, 55));
  tft.setTextSize(2);
  tft.drawNumber(currentSolar.kIndex, 246, 56);

  // HF Propagation Matrix Header
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("BAND PROPAGATION FORECAST", 12, 94, 2);
  tft.drawString("DAY", 175, 94, 2);
  tft.drawString("NIGHT", 245, 94, 2);

  tft.drawFastHLine(10, 112, 300, TFT_DARKGREY);

  for (int i = 0; i < 5; i++) {
    int y = 116 + i * 15;
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(bands[i].band, 20, y, 1);

    tft.setTextColor(bands[i].dayColor, TFT_BLACK);
    tft.drawString(bands[i].day, 175, y, 1);

    tft.setTextColor(bands[i].nightColor, TFT_BLACK);
    tft.drawString(bands[i].night, 245, y, 1);
  }

  // Footer: Aurora & Solar Wind
  char envBuf[48];
  snprintf(envBuf, sizeof(envBuf), "AURORA: %s | WIND: %s", currentSolar.aurora.c_str(), currentSolar.solarWind.c_str());
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawCentreString(envBuf, 160, 194, 1);
}

// Draw Tab 1: Real Live POTA Spots
void drawPotaTab() {
  tft.fillRect(0, 31, 320, 175, TFT_BLACK);

  // Top subheader
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("LIVE PARKS ON THE AIR (POTA)", 10, 35, 2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawRightString(QTH_LOCATOR " (Paris)", 310, 35, 2);

  for (int i = 0; i < 4; i++) {
    int y = 55 + i * 36;
    uint16_t bg = (i == selectedSpot) ? tft.color565(25, 55, 75) : tft.color565(15, 20, 30);
    uint16_t border = (i == selectedSpot) ? TFT_CYAN : TFT_DARKGREY;

    tft.fillRoundRect(8, y, 304, 33, 4, bg);
    tft.drawRoundRect(8, y, 304, 33, 4, border);

    // Activator
    tft.setTextColor(TFT_YELLOW, bg);
    tft.drawString(spots[i].activator, 14, y + 4, 2);

    // Frequency
    tft.setTextColor(TFT_WHITE, bg);
    tft.drawString(spots[i].freq + " MHz", 108, y + 4, 2);

    // Mode
    tft.setTextColor(TFT_GREEN, bg);
    tft.drawString(spots[i].mode, 195, y + 4, 2);

    // Park Ref
    tft.setTextColor(TFT_MAGENTA, bg);
    tft.drawRightString(spots[i].ref, 304, y + 4, 2);

    // Distance & Bearing from Paris
    char distBuf[48];
    if (spots[i].distanceKm > 0) {
      snprintf(distBuf, sizeof(distBuf), "%d km @ %d deg", spots[i].distanceKm, spots[i].bearingDeg);
    } else {
      snprintf(distBuf, sizeof(distBuf), "Awaiting location fix");
    }
    tft.setTextColor(TFT_LIGHTGREY, bg);
    tft.drawString(distBuf, 14, y + 20, 1);

    if (spots[i].grid.length() > 0) {
      tft.setTextColor(TFT_CYAN, bg);
      tft.drawRightString("Grid: " + spots[i].grid, 304, y + 20, 1);
    }
  }
}

void drawUI() {
  drawHeader();
  if (currentTab == 0) {
    drawSolarTab();
  } else {
    drawPotaTab();
  }
  drawTabs();
}

// Fetch Real Live Solar Weather from NOAA/HamQSL
bool fetchRealSolarData() {
  Serial.println("[SOLAR] Fetching real space weather from HamQSL...");
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(8000);

  HTTPClient http;
  if (!http.begin(client, "https://www.hamqsl.com/solarxml.php")) {
    Serial.println("[SOLAR] HTTP begin failed!");
    return false;
  }

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[SOLAR] HTTP GET failed, error: %d\n", httpCode);
    http.end();
    return false;
  }

  String xml = http.getString();
  http.end();

  if (xml.length() < 100) {
    Serial.println("[SOLAR] Response too short!");
    return false;
  }

  currentSolar.sfi = extractXmlInt(xml, "solarflux");
  currentSolar.aIndex = extractXmlInt(xml, "aindex");
  currentSolar.kIndex = extractXmlInt(xml, "kindex");
  currentSolar.ssn = extractXmlInt(xml, "sunspots");
  currentSolar.aurora = extractXmlTag(xml, "aurora");
  currentSolar.solarWind = extractXmlTag(xml, "solarwind") + " km/s";

  // Parse Band Conditions
  String d80 = extractBandCondition(xml, "80m-40m", "day");
  String n80 = extractBandCondition(xml, "80m-40m", "night");
  String d40 = extractBandCondition(xml, "80m-40m", "day");
  String n40 = extractBandCondition(xml, "80m-40m", "night");
  String d20 = extractBandCondition(xml, "30m-20m", "day");
  String n20 = extractBandCondition(xml, "30m-20m", "night");
  String d15 = extractBandCondition(xml, "17m-15m", "day");
  String n15 = extractBandCondition(xml, "17m-15m", "night");
  String d10 = extractBandCondition(xml, "12m-10m", "day");
  String n10 = extractBandCondition(xml, "12m-10m", "night");

  bands[0] = { "80m", d80, n80, getConditionColor(d80), getConditionColor(n80) };
  bands[1] = { "40m", d40, n40, getConditionColor(d40), getConditionColor(n40) };
  bands[2] = { "20m", d20, n20, getConditionColor(d20), getConditionColor(n20) };
  bands[3] = { "15m", d15, n15, getConditionColor(d15), getConditionColor(n15) };
  bands[4] = { "10m", d10, n10, getConditionColor(d10), getConditionColor(n10) };

  Serial.printf("[SOLAR] Live: SFI=%d, SSN=%d, A=%d, K=%d, Wind=%s\n",
                currentSolar.sfi, currentSolar.ssn, currentSolar.aIndex, currentSolar.kIndex, currentSolar.solarWind.c_str());

  updateLEDStatus();
  return true;
}

// Fetch Real Live POTA Spots from Parks on the Air API
bool fetchRealPotaSpots() {
  Serial.println("[POTA] Fetching live spots from api.pota.app...");
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(8000);

  HTTPClient http;
  if (!http.begin(client, "https://api.pota.app/spot/activator")) {
    Serial.println("[POTA] HTTP begin failed!");
    return false;
  }

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[POTA] HTTP GET failed, error: %d\n", httpCode);
    http.end();
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();
  if (!stream) {
    http.end();
    return false;
  }

  int spotsFound = 0;
  unsigned long timeout = millis() + 5000;

  while (stream->connected() && spotsFound < 4 && millis() < timeout) {
    if (!stream->available()) {
      delay(5);
      continue;
    }
    int c = stream->read();
    if (c == '{') {
      // Accumulate JSON object
      String objStr = "{";
      int depth = 1;
      unsigned long objLimit = millis() + 1500;
      while (depth > 0 && millis() < objLimit) {
        if (stream->available()) {
          char ch = (char)stream->read();
          objStr += ch;
          if (ch == '{') depth++;
          else if (ch == '}') depth--;
        } else {
          delay(2);
        }
      }

      StaticJsonDocument<768> doc;
      DeserializationError err = deserializeJson(doc, objStr);
      if (!err) {
        const char* act = doc["activator"];
        if (act && strlen(act) > 0) {
          spots[spotsFound].activator = String(act);

          float freqKhz = 0;
          if (doc["frequency"].is<float>()) {
            freqKhz = doc["frequency"].as<float>();
          } else if (doc["frequency"].is<const char*>()) {
            freqKhz = atof(doc["frequency"].as<const char*>());
          }

          if (freqKhz >= 1000.0f) {
            char fBuf[16];
            snprintf(fBuf, sizeof(fBuf), "%.3f", freqKhz / 1000.0f);
            spots[spotsFound].freq = String(fBuf);
          } else {
            spots[spotsFound].freq = String(freqKhz, 3);
          }

          const char* mode = doc["mode"];
          spots[spotsFound].mode = (mode && strlen(mode) > 0) ? String(mode) : "SSB";

          const char* ref = doc["reference"];
          spots[spotsFound].ref = ref ? String(ref) : "";

          const char* grid = doc["grid4"];
          spots[spotsFound].grid = grid ? String(grid) : "";

          float spotLat = doc["latitude"] | 0.0f;
          float spotLon = doc["longitude"] | 0.0f;

          if (spotLat != 0.0f || spotLon != 0.0f) {
            spots[spotsFound].distanceKm = calculateDistanceKm(STATION_LAT, STATION_LON, spotLat, spotLon);
            spots[spotsFound].bearingDeg = calculateBearingDeg(STATION_LAT, STATION_LON, spotLat, spotLon);
          } else {
            spots[spotsFound].distanceKm = 0;
            spots[spotsFound].bearingDeg = 0;
          }

          Serial.printf("[POTA] Spot %d: %s on %s MHz (%s) @ %s [%d km @ %d deg]\n",
                        spotsFound + 1, spots[spotsFound].activator.c_str(), spots[spotsFound].freq.c_str(),
                        spots[spotsFound].mode.c_str(), spots[spotsFound].ref.c_str(),
                        spots[spotsFound].distanceKm, spots[spotsFound].bearingDeg);

          spotsFound++;
        }
      }
    }
  }

  http.end();
  Serial.printf("[POTA] Successfully parsed %d real live spots.\n", spotsFound);
  return spotsFound > 0;
}

void checkTouch() {
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();
  if (p.z > 50 || ts.touched()) {
    if (millis() - lastTouchTime > 180) {
      lastTouchTime = millis();

      // Landscape calibration: non-inverted Y
      int x = map(p.x, 240, 3800, 0, 320);
      int y = map(p.y, 240, 3800, 0, 240);
      x = constrain(x, 0, 319);
      y = constrain(y, 0, 239);

      Serial.printf("[TOUCH] Raw X=%d, Y=%d, Z=%d -> Screen (%d, %d)\n", p.x, p.y, p.z, x, y);

      // Bottom Tabs switching (Y >= 200)
      if (y >= 200) {
        if (x < 160 && currentTab != 0) {
          currentTab = 0;
          beep(1800, 40);
          drawUI();
        } else if (x >= 160 && currentTab != 1) {
          currentTab = 1;
          beep(2200, 40);
          drawUI();
        }
      } else if (currentTab == 1 && y >= 55 && y < 200) {
        // Spot selection in POTA tab
        int spotIdx = (y - 55) / 36;
        if (spotIdx >= 0 && spotIdx < 4) {
          selectedSpot = spotIdx;
          beep(2500, 60);
          drawPotaTab();
        }
      }
    }
  }
}

void connectWiFi() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawCentreString("HAMCLOCK MINI", 160, 45, 2);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawCentreString("Connecting to Wi-Fi network...", 160, 95, 2);

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
    Serial.println("\n[WiFi] Connected successfully! IP: " + WiFi.localIP().toString());
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawCentreString("Connected! Syncing Real Telemetry...", 160, 160, 2);

    // Sync NTP Time
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("[NTP] Time synchronization started.");
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
  Serial.println("  HAMCLOCK MINI - REAL SOLAR & POTA TRACKER (CYD 2.8)");
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
  beep(1500, 80);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize touch on dedicated CYD SPI pins
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Connect to Wi-Fi and sync
  connectWiFi();

  // Fetch initial real live telemetry
  fetchRealSolarData();
  fetchRealPotaSpots();

  isDataReady = true;
  lastTelemetryUpdate = millis();

  updateLEDStatus();
  drawUI();
}

void loop() {
  checkTouch();

  // 1-second clock tick (Redraw only time text to prevent any flicker)
  if (millis() - lastClockTick >= 1000) {
    lastClockTick = millis();
    time_t now = time(nullptr);
    struct tm timeinfo;
    char timeBuf[20];
    if (now > 100000 && gmtime_r(&now, &timeinfo)) {
      snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d Z", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else {
      snprintf(timeBuf, sizeof(timeBuf), "--:--:-- Z");
    }
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.drawRightString(timeBuf, 312, 6, 2);
  }

  // Periodic real telemetry refresh every 90 seconds
  if (millis() - lastTelemetryUpdate >= 90000) {
    lastTelemetryUpdate = millis();
    Serial.println("[CYCLE] Refreshing live NOAA solar and POTA data...");
    if (WiFi.status() == WL_CONNECTED) {
      fetchRealSolarData();
      fetchRealPotaSpots();
      updateLEDStatus();
      if (currentTab == 0) {
        drawSolarTab();
      } else {
        drawPotaTab();
      }
    }
  }

  delay(15);
}

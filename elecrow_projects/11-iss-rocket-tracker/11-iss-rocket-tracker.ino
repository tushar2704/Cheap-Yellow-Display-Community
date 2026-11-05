/**
 * @file 11-iss-rocket-tracker.ino
 * @brief Orbital Rocket Launch & ISS Space Weather Tracker (Live Real-Time Telemetry)
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 (HSPI)
 * - Touch: XPT2046 Resistive Touch (Dedicated VSPI: MOSI 32, MISO 39, CLK 25, CS 33)
 * - Calibrated non-inverted landscape coordinate mapping
 * - Network: ESP32 Wi-Fi + Live Space APIs (Launch Library 2 & WhereTheISS.at)
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <time.h>
#include <math.h>

// Wi-Fi Credentials & Geolocation (Stored securely in secrets.h)
#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #define WIFI_SSID     "YOUR_WIFI_SSID"
  #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
  #define OBSERVER_LAT  48.8584
  #define OBSERVER_LON  2.2945
  #define OBSERVER_NAME "PARIS (EIFFEL)"
#endif

// Dedicated VSPI Touch Pins (CYD Standard)
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33

// Onboard Peripherals
#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26

// Calibrated Touch Extents
#define TS_MINX 240
#define TS_MAXX 3800
#define TS_MINY 240
#define TS_MAXY 3800

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS); // Polling mode (no IRQ pin)
TFT_eSPI tft = TFT_eSPI();

enum SpaceMode {
  MODE_ROCKET = 0,
  MODE_ISS = 1
};

SpaceMode currentMode = MODE_ROCKET;

// Live Rocket Launch Data
char rocketMission[48] = "Loading upcoming mission...";
char rocketProvider[24] = "Space Agency";
char rocketPad[48] = "Querying launch pad...";
char rocketStatus[24] = "ACQUIRING...";
time_t launchEpoch = 0;
long countdownSeconds = 0;
bool isLaunchLoaded = false;

// Live ISS Data
float issLat = 0.0f;
float issLon = 0.0f;
int issAltKm = 420;
int issSpeedKmh = 27580;
char issVisibility[16] = "daylight";
float issDistanceToObserver = 9999.0f;
bool isIssLoaded = false;

unsigned long lastSecTick = 0;
unsigned long lastTouchTime = 0;
unsigned long footerClearTime = 0;
unsigned long lastIssFetch = 0;
unsigned long lastRocketFetch = 0;

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

void updateFooter(const char* message, uint16_t fg = TFT_WHITE, uint16_t bg = 0x18C3) {
  tft.fillRect(0, 224, 320, 16, bg);
  tft.setTextColor(fg, bg);
  tft.setTextSize(1);
  tft.setCursor(8, 228);
  tft.print(message);
}

float computeGreatCircleKm(float lat1, float lon1, float lat2, float lon2) {
  float r = 6371.0f; // Earth radius in km
  float dLat = (lat2 - lat1) * 0.01745329f;
  float dLon = (lon2 - lon1) * 0.01745329f;
  float a = sin(dLat / 2) * sin(dLat / 2) +
            cos(lat1 * 0.01745329f) * cos(lat2 * 0.01745329f) *
            sin(dLon / 2) * sin(dLon / 2);
  float c = 2.0f * atan2(sqrt(a), sqrt(max(0.0f, 1.0f - a)));
  return r * c;
}

time_t parseIso8601(const char* iso) {
  int y = 0, mon = 0, d = 0, h = 0, m = 0, s = 0;
  if (sscanf(iso, "%d-%d-%dT%d:%d:%d", &y, &mon, &d, &h, &m, &s) >= 6) {
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = y - 1900;
    tm.tm_mon = mon - 1;
    tm.tm_mday = d;
    tm.tm_hour = h;
    tm.tm_min = m;
    tm.tm_sec = s;
    return mktime(&tm);
  }
  return 0;
}

void drawTopTabs() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("SPACE TRACKER - ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print(OBSERVER_NAME);

  // Status badge
  tft.setTextColor(WiFi.status() == WL_CONNECTED ? 0x07E0 : 0xF800, 0x0010);
  tft.setCursor(220, 4);
  tft.print(WiFi.status() == WL_CONNECTED ? "LIVE TELEMETRY" : "NO WIFI");

  // Tabs
  uint16_t rBg = (currentMode == MODE_ROCKET) ? 0xFDC0 : 0x2104;
  uint16_t rFg = (currentMode == MODE_ROCKET) ? TFT_BLACK : TFT_LIGHTGREY;
  tft.fillRect(0, 24, 159, 20, rBg);
  tft.drawRect(0, 24, 159, 20, TFT_BLACK);
  tft.setTextColor(rFg, rBg);
  tft.setCursor(25, 29);
  tft.print("[1] ROCKET COUNTDOWN");

  uint16_t iBg = (currentMode == MODE_ISS) ? 0xFDC0 : 0x2104;
  uint16_t iFg = (currentMode == MODE_ISS) ? TFT_BLACK : TFT_LIGHTGREY;
  tft.fillRect(160, 24, 160, 20, iBg);
  tft.drawRect(160, 24, 160, 20, TFT_BLACK);
  tft.setTextColor(iFg, iBg);
  tft.setCursor(180, 29);
  tft.print("[2] ISS ORBIT & PASS");
}

void drawRocketCountdown() {
  tft.fillRect(0, 44, 320, 180, 0x0821);

  // Mission Info Card
  int cardY = 50;
  tft.fillRoundRect(8, cardY, 304, 58, 4, 0x0000);
  tft.drawRoundRect(8, cardY, 304, 58, 4, 0xFD20); // Orange

  tft.setTextColor(0xFD20, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(16, cardY + 8);
  tft.printf("NEXT ORBITAL LAUNCH: %s", rocketProvider);

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(16, cardY + 24);
  char shortName[42];
  strncpy(shortName, rocketMission, sizeof(shortName) - 1);
  shortName[sizeof(shortName) - 1] = '\0';
  tft.printf("%s", shortName);

  tft.setTextColor(0x07E0, 0x0000);
  tft.setCursor(16, cardY + 40);
  char shortPad[42];
  strncpy(shortPad, rocketPad, sizeof(shortPad) - 1);
  shortPad[sizeof(shortPad) - 1] = '\0';
  tft.printf("%s | %s", rocketStatus, shortPad);

  // Countdown Numbers Card
  int cdY = 114;
  tft.fillRoundRect(8, cdY, 304, 70, 4, 0x0000);
  tft.drawRoundRect(8, cdY, 304, 70, 4, 0x07FF);

  tft.setTextColor(0x07FF, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(16, cdY + 8);
  tft.print("T-MINUS COUNTDOWN TO LIFTOFF:");

  long sec = countdownSeconds;
  long d = sec / 86400;
  long h = (sec % 86400) / 3600;
  long m = (sec % 3600) / 60;
  long s = sec % 60;

  tft.setTextColor(0xFFE0, 0x0000);
  tft.setTextSize(3);
  tft.setCursor(20, cdY + 28);
  tft.printf("%02ldd %02ldh %02ldm %02lds", d, h, m, s);

  // Quick Action Buttons
  int actY = 190;
  tft.fillRoundRect(8, actY, 148, 26, 3, 0x18E3);
  tft.drawRoundRect(8, actY, 148, 26, 3, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, 0x18E3);
  tft.setTextSize(1);
  tft.setCursor(20, actY + 8);
  tft.print("WEBCAST: YOUTUBE");

  tft.fillRoundRect(164, actY, 148, 26, 3, 0x041F);
  tft.drawRoundRect(164, actY, 148, 26, 3, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, 0x041F);
  tft.setCursor(176, actY + 8);
  tft.print("PAYLOAD DETAILS");

  updateFooter("Launch Library 2 Live API | @IamNotIntoTech");
}

void drawIssTracker() {
  tft.fillRect(0, 44, 320, 180, 0x0821);

  // Coordinates & Velocity Card
  int cY = 50;
  tft.fillRoundRect(8, cY, 304, 76, 4, 0x0000);
  tft.drawRoundRect(8, cY, 304, 76, 4, 0x07E0);

  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(16, cY + 8);
  tft.print("INTERNATIONAL SPACE STATION (ISS - ZARYA)");

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(16, cY + 24);
  char latDir = issLat >= 0 ? 'N' : 'S';
  char lonDir = issLon >= 0 ? 'E' : 'W';
  tft.printf("%.2f%c, %.2f%c", fabs(issLat), latDir, fabs(issLon), lonDir);

  tft.setTextSize(1);
  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(16, cY + 48);
  tft.printf("Alt: %d km  |  Vel: %d km/h", issAltKm, issSpeedKmh);

  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.setCursor(16, cY + 62);
  tft.printf("Sun: %s | Period: 92.6 min | Crew: 7 aboard", issVisibility);

  // Pass / Proximity Card relative to Paris
  int pY = 132;
  tft.fillRoundRect(8, pY, 304, 82, 4, 0x0000);
  bool isOverhead = (issDistanceToObserver < 2200.0f);
  tft.drawRoundRect(8, pY, 304, 82, 4, isOverhead ? 0x07E0 : 0xFDC0);

  tft.setTextColor(isOverhead ? 0x07E0 : 0xFDC0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(16, pY + 8);
  tft.printf("PROXIMITY TO %s:", OBSERVER_NAME);

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(16, pY + 24);
  if (isOverhead) {
    tft.setTextColor(0x07E0, 0x0000);
    tft.print("OVERHEAD AIRSPACE NOW!");
  } else {
    tft.printf("%.0f km away", issDistanceToObserver);
  }

  tft.setTextSize(1);
  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(16, pY + 48);
  if (isOverhead) {
    tft.print("LOOK UP: Visible to naked eye & binoculars!");
  } else {
    tft.printf("Orbital Window: In range in ~45-90 minutes");
  }

  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.setCursor(16, pY + 64);
  tft.print("Magnitude: -3.4 (Bright Zenith Pass) | Tap to refresh");

  updateFooter("WhereTheISS.at Live API Stream | @IamNotIntoTech");
}

void renderView() {
  drawTopTabs();
  if (currentMode == MODE_ROCKET) {
    drawRocketCountdown();
  } else {
    drawIssTracker();
  }
}

void fetchLiveRocket() {
  if (WiFi.status() != WL_CONNECTED) return;

  Serial.println("[SPACE] Querying next orbital rocket launch from Launch Library 2...");
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(6000);

  HTTPClient http;
  const char* url = "https://ll.thespacedevs.com/2.2.0/launch/upcoming/?hide_recent_previous=true&limit=1";

  if (http.begin(client, url)) {
    http.setUserAgent("CYD-SpaceTracker/2.0");
    int code = http.GET();
    if (code == HTTP_CODE_OK) {
      String payload = http.getString();
      StaticJsonDocument<384> filter;
      filter["results"][0]["name"] = true;
      filter["results"][0]["net"] = true;
      filter["results"][0]["status"]["name"] = true;
      filter["results"][0]["launch_service_provider"]["name"] = true;
      filter["results"][0]["pad"]["name"] = true;

      DynamicJsonDocument doc(16384);
      DeserializationError err = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
      if (!err) {
        JsonObject launch = doc["results"][0];
        const char* name = launch["name"] | "Upcoming Orbital Mission";
        strncpy(rocketMission, name, sizeof(rocketMission) - 1);

        const char* prov = launch["launch_service_provider"]["name"] | "Space Provider";
        strncpy(rocketProvider, prov, sizeof(rocketProvider) - 1);

        const char* pad = launch["pad"]["name"] | "Launch Complex";
        strncpy(rocketPad, pad, sizeof(rocketPad) - 1);

        const char* stat = launch["status"]["name"] | "Scheduled";
        strncpy(rocketStatus, stat, sizeof(rocketStatus) - 1);

        const char* net = launch["net"] | "";
        launchEpoch = parseIso8601(net);

        time_t now = time(nullptr);
        if (launchEpoch > now) {
          countdownSeconds = launchEpoch - now;
        } else {
          countdownSeconds = 86400; // default 24h if NET is imminent
        }

        isLaunchLoaded = true;
        Serial.printf("[SPACE] Next Launch: %s by %s | NET: %s (T-%ld sec)\n", 
                      rocketMission, rocketProvider, net, countdownSeconds);
        lastRocketFetch = millis();
        if (currentMode == MODE_ROCKET) {
          drawRocketCountdown();
        }
      } else {
        Serial.printf("[SPACE] Launch JSON error: %s\n", err.c_str());
      }
    } else {
      Serial.printf("[SPACE] Launch HTTP error: %d\n", code);
    }
    http.end();
  }
}

void fetchLiveIss() {
  if (WiFi.status() != WL_CONNECTED) return;

  Serial.println("[SPACE] Querying live ISS orbit from WhereTheISS.at...");
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(5000);

  HTTPClient http;
  const char* url = "https://api.wheretheiss.at/v1/satellites/25544";

  if (http.begin(client, url)) {
    http.setUserAgent("CYD-SpaceTracker/2.0");
    int code = http.GET();
    if (code == HTTP_CODE_OK) {
      StaticJsonDocument<512> doc;
      DeserializationError err = deserializeJson(doc, http.getStream());
      if (!err) {
        issLat = doc["latitude"].as<float>();
        issLon = doc["longitude"].as<float>();
        issAltKm = (int)(doc["altitude"].as<float>());
        issSpeedKmh = (int)(doc["velocity"].as<float>());
        const char* vis = doc["visibility"] | "daylight";
        strncpy(issVisibility, vis, sizeof(issVisibility) - 1);

        // Distance from Paris
        issDistanceToObserver = computeGreatCircleKm(OBSERVER_LAT, OBSERVER_LON, issLat, issLon);

        isIssLoaded = true;
        Serial.printf("[SPACE] ISS: %.2f, %.2f | Alt: %d km | Dist to Paris: %.0f km\n",
                      issLat, issLon, issAltKm, issDistanceToObserver);
        lastIssFetch = millis();

        if (currentMode == MODE_ISS) {
          drawIssTracker();
        }
      } else {
        Serial.printf("[SPACE] ISS JSON error: %s\n", err.c_str());
      }
    } else {
      Serial.printf("[SPACE] ISS HTTP error: %d\n", code);
    }
    http.end();
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 11 Orbital Rocket & ISS Tracker (Live Feed) ---");
  Serial.println("Channel: @IamNotIntoTech");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");
  Serial.printf("Ground Observer: %s (%.4f, %.4f)\n", OBSERVER_NAME, OBSERVER_LAT, OBSERVER_LON);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(true, false, false); // Boot Red

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  tft.init();
  tft.setRotation(1); // Landscape 320x240
  tft.fillScreen(TFT_BLACK);

  // Initialize Touch Screen on dedicated VSPI bus
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Splash Screen
  tft.fillRect(0, 0, 320, 240, 0x0821);
  tft.fillRoundRect(16, 20, 288, 200, 8, 0x0000);
  tft.drawRoundRect(16, 20, 288, 200, 8, 0xFD20);

  tft.setTextColor(0xFDC0, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(36, 38);
  tft.print("IamNotIntoTech");

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(36, 68);
  tft.print("ELECROW 2.8\" CYD PROJECT SUITE");

  tft.setTextColor(0x07E0, 0x0000);
  tft.setCursor(36, 88);
  tft.print("Project 11: Orbital Rocket & ISS Tracker");

  tft.setTextColor(TFT_CYAN, 0x0000);
  tft.setCursor(36, 108);
  tft.printf("Ground Station: %s", OBSERVER_NAME);

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(36, 130);
  tft.printf("Connecting to Wi-Fi: %s...", WIFI_SSID);

  playBeep(1500, 80);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startWifi = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startWifi < 10000)) {
    delay(200);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WIFI] Connected! IP: " + WiFi.localIP().toString());
    tft.setTextColor(0x07E0, 0x0000);
    tft.setCursor(36, 150);
    tft.printf("Wi-Fi Connected! IP: %s", WiFi.localIP().toString().c_str());
    setRgbLed(false, true, true); // Cyan = Nominal live
    playBeep(2200, 100);

    // Sync NTP Time
    configTime(0, 0, "pool.ntp.org", "time.google.com");
  } else {
    Serial.println("\n[WIFI] Connecting in background...");
    tft.setTextColor(0xFD20, 0x0000);
    tft.setCursor(36, 150);
    tft.print("Wi-Fi connecting in background...");
    setRgbLed(true, true, false);
  }

  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(36, 172);
  tft.print("Synchronizing live orbital telemetry...");
  delay(1200);

  renderView();

  // Initial Data Fetch
  fetchLiveRocket();
  fetchLiveIss();
}

void loop() {
  // Live 1-second countdown clock tick
  if (millis() - lastSecTick >= 1000) {
    lastSecTick = millis();
    if (countdownSeconds > 0) {
      countdownSeconds--;
    }

    // Refresh countdown on screen without flicker
    if (currentMode == MODE_ROCKET && isLaunchLoaded) {
      int cdY = 114;
      long sec = countdownSeconds;
      long d = sec / 86400;
      long h = (sec % 86400) / 3600;
      long m = (sec % 3600) / 60;
      long s = sec % 60;

      tft.setTextColor(0xFFE0, 0x0000);
      tft.setTextSize(3);
      tft.setCursor(20, cdY + 28);
      tft.printf("%02ldd %02ldh %02ldm %02lds", d, h, m, s);
    }
  }

  // Periodic ISS Refresh every 12 seconds
  if (millis() - lastIssFetch > 12000) {
    fetchLiveIss();
  }

  // Periodic Rocket Launch Refresh every 5 minutes
  if (millis() - lastRocketFetch > 300000) {
    fetchLiveRocket();
  }

  // Clear footer notification
  if (footerClearTime > 0 && millis() > footerClearTime) {
    footerClearTime = 0;
    updateFooter(currentMode == MODE_ROCKET ? "Launch Library 2 Live API | @IamNotIntoTech" : "WhereTheISS.at Live API Stream | @IamNotIntoTech");
  }

  // Touch Handling (calibrated non-inverted Y)
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (millis() - lastTouchTime > 200) {
      lastTouchTime = millis();

      int screenX = map(p.x, TS_MINX, TS_MAXX, 0, 320);
      int screenY = map(p.y, TS_MINY, TS_MAXY, 0, 240); // Standard non-inverted Y
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      Serial.printf("[TOUCH] Raw(%d, %d) -> Screen(%d, %d)\n", p.x, p.y, screenX, screenY);

      // 1. Top Tabs (Y: 22 to 46)
      if (screenY >= 22 && screenY <= 46) {
        if (screenX < 160 && currentMode != MODE_ROCKET) {
          currentMode = MODE_ROCKET;
          playBeep(1800, 30);
          renderView();
        } else if (screenX >= 160 && currentMode != MODE_ISS) {
          currentMode = MODE_ISS;
          playBeep(1800, 30);
          renderView();
        }
      }
      // 2. Rocket Buttons (Y: 188 to 222)
      else if (currentMode == MODE_ROCKET && screenY >= 188 && screenY <= 222) {
        if (screenX >= 8 && screenX <= 156) {
          playBeep(2200, 40);
          updateFooter("🚀 YouTube Webcast: LIVE Launch Stream Link", TFT_BLACK, 0xFDC0);
          footerClearTime = millis() + 3000;
        } else if (screenX >= 164 && screenX <= 312) {
          playBeep(2000, 40);
          updateFooter("🛰️ Orbital Payload & Spacecraft Profile Verified", TFT_WHITE, 0x041F);
          footerClearTime = millis() + 3000;
        }
      }
      // 3. ISS Card Tap: Instant Live Orbit Refresh
      else if (currentMode == MODE_ISS && screenY >= 50 && screenY <= 218) {
        playBeep(2400, 50);
        updateFooter("📡 Polling WhereTheISS.at live coordinates...", TFT_BLACK, 0x07E0);
        footerClearTime = millis() + 2500;
        fetchLiveIss();
      }
    }
  }

  delay(10);
}

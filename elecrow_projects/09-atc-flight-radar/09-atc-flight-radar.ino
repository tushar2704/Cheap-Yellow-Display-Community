/**
 * @file 09-atc-flight-radar.ino
 * @brief ATC Rotating Sweep Flight Radar - Real-Time Live ADS-B Stream
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 (HSPI)
 * - Touch: XPT2046 Resistive Touch (Dedicated VSPI: MOSI 32, MISO 39, CLK 25, CS 33)
 * - Calibrated non-inverted landscape coordinate mapping
 * - Network: ESP32 Wi-Fi + Live ADS-B Airspace Feed (adsb.lol REST API)
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

// Wi-Fi Credentials & Geolocation (Stored securely in secrets.h)
#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #define WIFI_SSID     "YOUR_WIFI_SSID"
  #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
  #define RADAR_LAT     48.8584
  #define RADAR_LON     2.2945
  #define RADAR_LOCNAME "PARIS (EIFFEL)"
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

// Radar Scope Geometry
const int RADAR_CX = 90;
const int RADAR_CY = 124;
const int RADAR_R  = 78;

float sweepAngle = 0.0f;
float lastSweepAngle = 0.0f;
int selectedRangeNM = 25; // 10, 25, 50 NM

struct Aircraft {
  char callsign[12];
  char model[8];
  int altitudeFt;
  int speedKts;
  int headingDeg;
  float distanceNM;
  float directionDeg; // Azimuth bearing from radar origin
  char squawk[6];
  int x;
  int y;
  bool isAlert;
};

#define MAX_AIRCRAFT 24
Aircraft traffic[MAX_AIRCRAFT];
int aircraftCount = 0;
int highlightedCraft = 0;

unsigned long lastRadarTick = 0;
unsigned long lastTouchTime = 0;
unsigned long lastLiveFetch = 0;
const unsigned long FETCH_INTERVAL = 20000; // Poll live ADS-B every 20 seconds

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

void drawStaticRadarScope() {
  // Header Bar
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("ATC - ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print(RADAR_LOCNAME);

  // Range and Status Badge (top right)
  tft.setTextColor(TFT_WHITE, 0x0010);
  tft.setCursor(205, 4);
  if (aircraftCount > 0) {
    tft.printf("LIVE: %d ACFT", aircraftCount);
  } else {
    tft.print(WiFi.status() == WL_CONNECTED ? "ACQUIRING..." : "NO WIFI");
  }

  // Clear Radar Scope Area
  tft.fillRect(0, 24, 180, 200, 0x0000);

  // Concentric Rings
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R, 0x03E0);          // Outer ring
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 2 / 3, 0x0260); // Mid ring
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R / 3, 0x0200);     // Inner ring

  // Crosshairs
  tft.drawFastHLine(RADAR_CX - RADAR_R, RADAR_CY, RADAR_R * 2, 0x0200);
  tft.drawFastVLine(RADAR_CX, RADAR_CY - RADAR_R, RADAR_R * 2, 0x0200);

  // Cardinal markers
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(RADAR_CX - 2, RADAR_CY - RADAR_R + 3); tft.print("N");
  tft.setCursor(RADAR_CX + RADAR_R - 8, RADAR_CY - 3); tft.print("E");
  tft.setCursor(RADAR_CX - 2, RADAR_CY + RADAR_R - 9); tft.print("S");
  tft.setCursor(RADAR_CX - RADAR_R + 2, RADAR_CY - 3); tft.print("W");

  // Vertical Separator
  tft.drawFastVLine(180, 24, 200, 0x2965);

  // Footer
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(8, 228);
  tft.printf("Live ADS-B feed (adsb.lol) | R: %d NM | @IamNotIntoTech", selectedRangeNM);
}

void drawFlightInfoCard() {
  int cardX = 184;
  int cardY = 28;
  int cardW = 132;

  if (aircraftCount == 0) {
    tft.fillRoundRect(cardX, cardY, cardW, 134, 4, 0x0000);
    tft.drawRoundRect(cardX, cardY, cardW, 134, 4, 0x041F);
    tft.setTextColor(0x07FF, 0x0000);
    tft.setTextSize(1);
    tft.setCursor(cardX + 8, cardY + 12);
    tft.print("AIRSPACE SCAN");
    tft.setTextColor(TFT_WHITE, 0x0000);
    tft.setCursor(cardX + 8, cardY + 36);
    tft.print("Querying ADS-B...");
    tft.setCursor(cardX + 8, cardY + 54);
    tft.printf("Lat: %.4f", RADAR_LAT);
    tft.setCursor(cardX + 8, cardY + 70);
    tft.printf("Lon: %.4f", RADAR_LON);
    tft.setTextColor(0x07E0, 0x0000);
    tft.setCursor(cardX + 8, cardY + 95);
    tft.print(RADAR_LOCNAME);
  } else {
    const Aircraft& a = traffic[highlightedCraft];

    // LED and Frame styling based on Squawk 7700 / Emergency
    if (a.isAlert) {
      setRgbLed(true, false, false); // Alert RED
      tft.drawRoundRect(cardX, cardY, cardW, 134, 4, 0xF800);
    } else {
      setRgbLed(false, true, false); // Nominal GREEN
      tft.drawRoundRect(cardX, cardY, cardW, 134, 4, 0x041F);
    }

    tft.fillRoundRect(cardX + 1, cardY + 1, cardW - 2, 132, 3, 0x0000);

    tft.setTextSize(1);
    tft.setTextColor(0x07FF, 0x0000);
    tft.setCursor(cardX + 8, cardY + 6);
    tft.printf("TARGET %d OF %d", highlightedCraft + 1, aircraftCount);

    // Callsign & Type
    tft.setTextColor(TFT_WHITE, 0x0000);
    tft.setTextSize(2);
    tft.setCursor(cardX + 8, cardY + 20);
    tft.print(a.callsign);

    tft.setTextSize(1);
    tft.setTextColor(0xFDC0, 0x0000);
    tft.setCursor(cardX + 8, cardY + 40);
    tft.printf("Type: %s", a.model);

    tft.setTextColor(TFT_WHITE, 0x0000);
    tft.setCursor(cardX + 8, cardY + 56);
    tft.printf("Alt:   %d ft", a.altitudeFt);

    tft.setCursor(cardX + 8, cardY + 70);
    tft.printf("Speed: %d kts", a.speedKts);

    tft.setCursor(cardX + 8, cardY + 84);
    tft.printf("Dist:  %.1f NM", a.distanceNM);

    tft.setCursor(cardX + 8, cardY + 98);
    tft.printf("Hdg:   %03d deg", a.headingDeg);

    tft.setCursor(cardX + 8, cardY + 114);
    if (a.isAlert) {
      tft.setTextColor(0xF800, 0x0000);
      tft.printf("SQUAWK: %s ALERT!", a.squawk);
    } else {
      tft.setTextColor(0x07E0, 0x0000);
      tft.printf("Squawk: %s OK", a.squawk);
    }
  }

  // Range Selector Buttons (3 buttons)
  int btnY = 166;
  int btnW = 41;
  const int ranges[3] = {10, 25, 50};

  for (int i = 0; i < 3; i++) {
    int bx = cardX + i * (btnW + 4);
    bool isSel = (ranges[i] == selectedRangeNM);
    tft.fillRoundRect(bx, btnY, btnW, 24, 3, isSel ? 0x07E0 : 0x18E3);
    tft.drawRoundRect(bx, btnY, btnW, 24, 3, TFT_WHITE);

    tft.setTextColor(isSel ? TFT_BLACK : TFT_WHITE, isSel ? 0x07E0 : 0x18E3);
    tft.setTextSize(1);
    tft.setCursor(bx + 4, btnY + 8);
    tft.printf("%dNM", ranges[i]);
  }

  // Next Target Button
  int nxtY = 194;
  tft.fillRoundRect(cardX, nxtY, cardW, 24, 3, 0xFD20);
  tft.drawRoundRect(cardX, nxtY, cardW, 24, 3, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, 0xFD20);
  tft.setTextSize(1);
  tft.setCursor(cardX + 18, nxtY + 8);
  tft.print(">> NEXT TARGET <<");
}

void fetchLiveAirspace() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ADS-B] Wi-Fi not connected yet. Waiting...");
    return;
  }

  Serial.println("[ADS-B] Querying live aircraft from adsb.lol...");
  WiFiClientSecure client;
  client.setInsecure(); // Public REST API, skip root CA check
  client.setTimeout(6000);

  HTTPClient http;
  char url[128];
  snprintf(url, sizeof(url), "https://api.adsb.lol/v2/lat/%.4f/lon/%.4f/dist/%d", RADAR_LAT, RADAR_LON, selectedRangeNM);
  Serial.printf("[ADS-B] GET %s\n", url);

  if (http.begin(client, url)) {
    http.setUserAgent("CYD-ATC-Radar/2.0");
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      StaticJsonDocument<384> filter;
      JsonObject acFilter = filter["ac"].createNestedObject();
      acFilter["flight"] = true;
      acFilter["t"] = true;
      acFilter["alt_baro"] = true;
      acFilter["gs"] = true;
      acFilter["track"] = true;
      acFilter["squawk"] = true;
      acFilter["dst"] = true;
      acFilter["dir"] = true;
      acFilter["emergency"] = true;

      DynamicJsonDocument doc(24576);
      DeserializationError err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
      if (!err) {
        JsonArray acList = doc["ac"].as<JsonArray>();
        int count = 0;
        for (JsonObject ac : acList) {
          if (count >= MAX_AIRCRAFT) break;
          if (!ac.containsKey("dst") || !ac.containsKey("dir")) continue;

          const char* flt = ac["flight"] | "";
          String cs = String(flt);
          cs.trim();
          if (cs.length() == 0) cs = "TGT" + String(count + 1);

          strncpy(traffic[count].callsign, cs.c_str(), sizeof(traffic[count].callsign) - 1);
          traffic[count].callsign[sizeof(traffic[count].callsign) - 1] = '\0';

          const char* mdl = ac["t"] | "UNKN";
          strncpy(traffic[count].model, mdl, sizeof(traffic[count].model) - 1);
          traffic[count].model[sizeof(traffic[count].model) - 1] = '\0';

          traffic[count].altitudeFt = ac["alt_baro"].as<int>();
          traffic[count].speedKts = ac["gs"].as<int>();
          traffic[count].headingDeg = ac["track"].as<int>();
          traffic[count].distanceNM = ac["dst"].as<float>();
          traffic[count].directionDeg = ac["dir"].as<float>();

          const char* sq = ac["squawk"] | "1200";
          strncpy(traffic[count].squawk, sq, sizeof(traffic[count].squawk) - 1);
          traffic[count].squawk[sizeof(traffic[count].squawk) - 1] = '\0';

          const char* emg = ac["emergency"] | "none";
          traffic[count].isAlert = (strcmp(emg, "none") != 0) || (strcmp(traffic[count].squawk, "7700") == 0);

          count++;
        }

        if (count > 0) {
          aircraftCount = count;
          if (highlightedCraft >= aircraftCount) highlightedCraft = 0;
          Serial.printf("[ADS-B] Successfully loaded %d live aircraft overhead!\n", aircraftCount);
          lastLiveFetch = millis();
          drawStaticRadarScope();
          drawFlightInfoCard();
        } else {
          Serial.println("[ADS-B] No aircraft currently in selected range");
        }
      } else {
        Serial.printf("[ADS-B] JSON parse error: %s\n", err.c_str());
      }
    } else {
      Serial.printf("[ADS-B] HTTP error: %d\n", httpCode);
    }
    http.end();
  }
}

void updateRadarSweep() {
  // Erase previous sweep line by redrawing black
  float rad = sweepAngle * 0.0174533f;
  float oldRad = lastSweepAngle * 0.0174533f;

  int oldX = RADAR_CX + (int)(cos(oldRad) * (RADAR_R - 2));
  int oldY = RADAR_CY + (int)(sin(oldRad) * (RADAR_R - 2));
  tft.drawLine(RADAR_CX, RADAR_CY, oldX, oldY, 0x0000);

  // Restore radar grid elements
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R, 0x03E0);
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 2 / 3, 0x0260);
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R / 3, 0x0200);
  tft.drawFastHLine(RADAR_CX - RADAR_R, RADAR_CY, RADAR_R * 2, 0x0200);
  tft.drawFastVLine(RADAR_CX, RADAR_CY - RADAR_R, RADAR_R * 2, 0x0200);

  // Draw new sweep line in phosphor green
  int newX = RADAR_CX + (int)(cos(rad) * (RADAR_R - 2));
  int newY = RADAR_CY + (int)(sin(rad) * (RADAR_R - 2));
  tft.drawLine(RADAR_CX, RADAR_CY, newX, newY, 0x07E0);

  // Plot Live Aircraft Blips
  for (int i = 0; i < aircraftCount; i++) {
    Aircraft& a = traffic[i];
    float distScale = a.distanceNM / (float)selectedRangeNM;
    distScale = constrain(distScale, 0.05f, 1.0f);

    // Azimuth bearing: 0 = North/Up, 90 = East/Right, 180 = South/Down, 270 = West/Left
    float craftRad = (a.directionDeg - 90.0f) * 0.01745329f;
    int px = RADAR_CX + (int)(cos(craftRad) * (RADAR_R * distScale));
    int py = RADAR_CY + (int)(sin(craftRad) * (RADAR_R * distScale));
    a.x = px;
    a.y = py;

    uint16_t blipCol = a.isAlert ? 0xF800 : (i == highlightedCraft ? TFT_WHITE : 0x07E0);
    tft.fillCircle(px, py, (i == highlightedCraft ? 3 : 2), blipCol);

    if (i == highlightedCraft) {
      tft.drawCircle(px, py, 6, 0x07FF);
    }
  }

  lastSweepAngle = sweepAngle;
  sweepAngle += 4.0f;
  if (sweepAngle >= 360.0f) {
    sweepAngle = 0.0f;
    // Rotation completed: proximity alert chirp if highlighted craft is close
    if (aircraftCount > 0 && traffic[highlightedCraft].distanceNM < 5.0f) {
      playBeep(2400, 30);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 09 ATC Flight Radar (Live Feed) ---");
  Serial.println("Channel: @IamNotIntoTech");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");
  Serial.printf("Radar Center: %s (%.4f, %.4f)\n", RADAR_LOCNAME, RADAR_LAT, RADAR_LON);

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
  tft.drawRoundRect(16, 20, 288, 200, 8, 0x07E0);

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
  tft.print("Project 09: Live ADS-B Flight Radar");

  tft.setTextColor(TFT_CYAN, 0x0000);
  tft.setCursor(36, 108);
  tft.printf("Origin: %s (%.2f, %.2f)", RADAR_LOCNAME, RADAR_LAT, RADAR_LON);

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(36, 130);
  tft.printf("Connecting to Wi-Fi: %s...", WIFI_SSID);

  playBeep(1800, 80);

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
    setRgbLed(false, true, false);
    playBeep(2400, 100);
  } else {
    Serial.println("\n[WIFI] Still connecting in background...");
    tft.setTextColor(0xFD20, 0x0000);
    tft.setCursor(36, 150);
    tft.print("Wi-Fi connecting in background...");
    setRgbLed(true, true, false); // Amber
  }

  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(36, 172);
  tft.print("Streaming live Paris airspace...");
  delay(1200);

  drawStaticRadarScope();
  drawFlightInfoCard();
  fetchLiveAirspace();
}

void loop() {
  // Update Radar Sweep Line (approx 28 FPS)
  if (millis() - lastRadarTick > 35) {
    lastRadarTick = millis();
    updateRadarSweep();
  }

  // Periodic Live ADS-B Refresh
  if (millis() - lastLiveFetch > FETCH_INTERVAL) {
    fetchLiveAirspace();
  }

  // Responsive Touch Handling with Calibrated Non-Inverted Coordinates
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (millis() - lastTouchTime > 220) {
      lastTouchTime = millis();

      int screenX = map(p.x, TS_MINX, TS_MAXX, 0, 320);
      int screenY = map(p.y, TS_MINY, TS_MAXY, 0, 240); // Standard non-inverted Y
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      Serial.printf("[TOUCH] Raw(%d, %d) -> Screen(%d, %d)\n", p.x, p.y, screenX, screenY);

      // 1. Direct Touch on Radar Scope -> Select Closest Live Plane
      if (screenX >= 0 && screenX <= 180 && screenY >= 24 && screenY <= 224) {
        if (aircraftCount > 0) {
          int closestIdx = -1;
          float minDistSq = 999999.0f;
          for (int i = 0; i < aircraftCount; i++) {
            float dx = screenX - traffic[i].x;
            float dy = screenY - traffic[i].y;
            float d2 = dx * dx + dy * dy;
            if (d2 < minDistSq) {
              minDistSq = d2;
              closestIdx = i;
            }
          }
          if (closestIdx >= 0 && minDistSq < 1600) { // Within ~40px radius
            highlightedCraft = closestIdx;
            playBeep(2400, 40);
            drawFlightInfoCard();
          }
        }
      }
      // 2. Range Selector Buttons (X: 184 to 316, Y: 166 to 190)
      else if (screenX >= 184 && screenX <= 316 && screenY >= 166 && screenY <= 190) {
        int idx = (screenX - 184) / 45;
        int newRange = selectedRangeNM;
        if (idx == 0) newRange = 10;
        else if (idx == 1) newRange = 25;
        else if (idx >= 2) newRange = 50;

        if (newRange != selectedRangeNM) {
          selectedRangeNM = newRange;
          playBeep(2000, 35);
          drawStaticRadarScope();
          drawFlightInfoCard();
          fetchLiveAirspace(); // Query new radius immediately
        }
      }
      // 3. Next Target Button (X: 184 to 316, Y: 194 to 222)
      else if (screenX >= 184 && screenX <= 316 && screenY >= 194 && screenY <= 222) {
        if (aircraftCount > 0) {
          highlightedCraft = (highlightedCraft + 1) % aircraftCount;
          playBeep(2200, 35);
          drawFlightInfoCard();
        }
      }
    }
  }

  delay(5);
}

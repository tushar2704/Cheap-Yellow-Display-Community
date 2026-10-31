/**
 * @file 10-sky-spotter-station.ino
 * @brief Sky-Spotter Flight Deck - Live 3D Spotter Line & Azimuth Calculator
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 (HSPI)
 * - Touch: XPT2046 Resistive Touch (Dedicated VSPI: MOSI 32, MISO 39, CLK 25, CS 33)
 * - Calibrated non-inverted landscape coordinate mapping
 * - Network: ESP32 Wi-Fi + Real Live ADS-B Airspace Feed (adsb.lol REST API)
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
#include <math.h>

// Wi-Fi Credentials & Geolocation (Stored securely in secrets.h)
#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #define WIFI_SSID     "YOUR_WIFI_SSID"
  #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
  #define SPOTTER_LAT   48.8584
  #define SPOTTER_LON   2.2945
  #define SPOTTER_NAME  "PARIS (EIFFEL)"
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

enum SpotterMode {
  MODE_BOARD = 0,
  MODE_SPOTTER_LINE = 1
};

SpotterMode currentMode = MODE_BOARD;
int selectedFlight = 0;
int boardPage = 0;

struct FlightInfo {
  char callsign[12];
  char aircraft[8];
  int altitudeFt;
  int speedKts;
  int bearingDeg;
  int elevationDeg; // 0 (horizon) to 90 (zenith overhead)
  float distanceNM;
  char squawk[6];
  uint16_t airlineColor;
  bool isAlert;
};

#define MAX_FLIGHTS 16
FlightInfo flights[MAX_FLIGHTS];
int flightCount = 0;

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

uint16_t getAirlineColor(const char* cs) {
  if (strncmp(cs, "AFR", 3) == 0) return 0x001F; // Air France Blue
  if (strncmp(cs, "BAW", 3) == 0) return 0xF800; // British Airways Red
  if (strncmp(cs, "DLH", 3) == 0) return 0xFDC0; // Lufthansa Yellow
  if (strncmp(cs, "EJU", 3) == 0 || strncmp(cs, "EZY", 3) == 0) return 0xFD20; // easyJet Orange
  if (strncmp(cs, "RYR", 3) == 0) return 0x03FF; // Ryanair Cyan
  if (strncmp(cs, "TVF", 3) == 0) return 0x07E0; // Transavia Green
  if (strncmp(cs, "VLG", 3) == 0) return 0xFFE0; // Vueling Yellow
  if (strncmp(cs, "UAE", 3) == 0) return 0xF800; // Emirates Red
  return 0x07FF; // Default Cyan
}

void updateFooter(const char* message, uint16_t fg = TFT_WHITE, uint16_t bg = 0x18C3) {
  tft.fillRect(0, 224, 320, 16, bg);
  tft.setTextColor(fg, bg);
  tft.setTextSize(1);
  tft.setCursor(8, 228);
  tft.print(message);
}

void drawTopTabs() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("SKY-SPOTTER - ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print(SPOTTER_NAME);

  // Status badge
  tft.setTextColor(flightCount > 0 ? 0x07E0 : 0xF800, 0x0010);
  tft.setCursor(205, 4);
  if (flightCount > 0) {
    tft.printf("LIVE: %d ACFT", flightCount);
  } else {
    tft.print(WiFi.status() == WL_CONNECTED ? "ACQUIRING..." : "NO WIFI");
  }

  // Tabs
  uint16_t bBg = (currentMode == MODE_BOARD) ? 0xFDC0 : 0x2104;
  uint16_t bFg = (currentMode == MODE_BOARD) ? TFT_BLACK : TFT_LIGHTGREY;
  tft.fillRect(0, 24, 159, 20, bBg);
  tft.drawRect(0, 24, 159, 20, TFT_BLACK);
  tft.setTextColor(bFg, bBg);
  tft.setCursor(30, 29);
  tft.print("[1] FLIGHT BOARD");

  uint16_t sBg = (currentMode == MODE_SPOTTER_LINE) ? 0xFDC0 : 0x2104;
  uint16_t sFg = (currentMode == MODE_SPOTTER_LINE) ? TFT_BLACK : TFT_LIGHTGREY;
  tft.fillRect(160, 24, 160, 20, sBg);
  tft.drawRect(160, 24, 160, 20, TFT_BLACK);
  tft.setTextColor(sFg, sBg);
  tft.setCursor(180, 29);
  tft.print("[2] SPOTTER LINE (3D)");
}

void drawFlightBoard() {
  tft.fillRect(0, 44, 320, 180, 0x0821);

  if (flightCount == 0) {
    tft.setTextColor(0x07FF, 0x0821);
    tft.setTextSize(1);
    tft.setCursor(30, 80);
    tft.print("Connecting to live ADS-B airspace feed...");
    tft.setTextColor(TFT_WHITE, 0x0821);
    tft.setCursor(30, 105);
    tft.printf("Origin: %s (%.4f, %.4f)", SPOTTER_NAME, SPOTTER_LAT, SPOTTER_LON);
    tft.setCursor(30, 125);
    tft.print("Radius: 25 Nautical Miles");
    tft.setTextColor(0x07E0, 0x0821);
    tft.setCursor(30, 150);
    tft.print("Scanning CDG & Orly corridors...");
    updateFooter("Searching live Paris flights... | @IamNotIntoTech");
    return;
  }

  int startIdx = boardPage * 4;
  for (int i = 0; i < 4; i++) {
    int idx = startIdx + i;
    int y = 48 + i * 42;

    if (idx < flightCount) {
      const FlightInfo& f = flights[idx];
      bool isSel = (idx == selectedFlight);

      tft.fillRoundRect(6, y, 308, 38, 4, isSel ? 0x0000 : 0x10A2);
      tft.drawRoundRect(6, y, 308, 38, 4, f.isAlert ? 0xF800 : (isSel ? 0x07E0 : TFT_DARKGREY));

      // Airline Color Tag
      tft.fillRect(8, y + 4, 4, 30, f.airlineColor);

      // Callsign & Type
      tft.setTextColor(TFT_WHITE, isSel ? 0x0000 : 0x10A2);
      tft.setTextSize(1);
      tft.setCursor(18, y + 6);
      tft.printf("%s (%s)", f.callsign, f.aircraft);

      tft.setTextColor(0x07FF, isSel ? 0x0000 : 0x10A2);
      tft.setCursor(18, y + 20);
      tft.printf("Alt: %d ft | Spd: %d kt", f.altitudeFt, f.speedKts);

      // Spotting Angle & Distance
      tft.setTextColor(isSel ? 0xFDC0 : TFT_LIGHTGREY, isSel ? 0x0000 : 0x10A2);
      tft.setCursor(195, y + 6);
      tft.printf("Brg: %03d deg", f.bearingDeg);

      tft.setTextColor(isSel ? 0x07E0 : TFT_WHITE, isSel ? 0x0000 : 0x10A2);
      tft.setCursor(195, y + 20);
      tft.printf("Elev: %d deg (%.1fNM)", f.elevationDeg, f.distanceNM);
    } else {
      // Empty slot
      tft.drawRoundRect(6, y, 308, 38, 4, 0x10A2);
    }
  }

  char footMsg[80];
  snprintf(footMsg, sizeof(footMsg), "Tap flight to view 3D Vector | Showing %d-%d of %d", 
           startIdx + 1, min(startIdx + 4, flightCount), flightCount);
  updateFooter(footMsg);
}

void drawSpotterLine() {
  tft.fillRect(0, 44, 320, 180, 0x0821);

  if (flightCount == 0) {
    tft.setTextColor(0x07FF, 0x0821);
    tft.setTextSize(1);
    tft.setCursor(40, 110);
    tft.print("No flight selected. Return to board.");
    return;
  }

  const FlightInfo& f = flights[selectedFlight];

  // Left Section: Compass Azimuth (X: 0 to 150)
  int cx = 75;
  int cy = 132;
  int r = 55;

  tft.drawCircle(cx, cy, r, 0x07E0);
  tft.drawCircle(cx, cy, r * 2 / 3, 0x0260);
  tft.drawCircle(cx, cy, 2, TFT_WHITE);

  // Compass cardinal marks
  tft.setTextColor(0x07E0, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(cx - 3, cy - r - 10); tft.print("N");
  tft.setCursor(cx + r + 4, cy - 3);   tft.print("E");
  tft.setCursor(cx - 3, cy + r + 3);   tft.print("S");
  tft.setCursor(cx - r - 10, cy - 3);  tft.print("W");

  // Draw Directional Bearing Needle
  float rad = (f.bearingDeg - 90.0f) * 0.0174533f;
  int nx = cx + (int)(cos(rad) * (r - 4));
  int ny = cy + (int)(sin(rad) * (r - 4));
  tft.drawLine(cx, cy, nx, ny, 0xF800);
  tft.drawLine(cx + 1, cy, nx + 1, ny, 0xF800);
  tft.fillCircle(nx, ny, 3, 0xF800);

  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setCursor(20, 52);
  tft.printf("COMPASS: %03d deg", f.bearingDeg);

  // Right Section: Elevation Incline Angle (X: 160 to 314)
  int rx = 164;
  int ry = 52;
  int rw = 150;

  tft.fillRoundRect(rx, ry, rw, 134, 4, 0x0000);
  tft.drawRoundRect(rx, ry, rw, 134, 4, f.isAlert ? 0xF800 : 0x07FF);

  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(rx + 8, ry + 8);
  tft.print("3D SPOTTER VECTOR");

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(rx + 8, ry + 22);
  tft.print(f.callsign);

  tft.setTextSize(1);
  tft.setTextColor(0xFDC0, 0x0000);
  tft.setCursor(rx + 8, ry + 42);
  tft.printf("LOOK UP: %d deg ELEVATION", f.elevationDeg);

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(rx + 8, ry + 56);
  tft.printf("Type: %s | Dist: %.1f NM", f.aircraft, f.distanceNM);

  tft.setCursor(rx + 8, ry + 68);
  tft.printf("Alt:  %d ft | Spd: %d kt", f.altitudeFt, f.speedKts);

  // Arc Gauge for Elevation Angle
  int arcBaseX = rx + 24;
  int arcBaseY = ry + 114;
  int arcLen = 50;
  tft.drawFastHLine(arcBaseX, arcBaseY, arcLen, TFT_DARKGREY); // Horizon
  tft.drawFastVLine(arcBaseX, arcBaseY - arcLen, arcLen, TFT_DARKGREY); // Zenith

  // Vector Line
  float elevRad = f.elevationDeg * 0.0174533f;
  int ex = arcBaseX + (int)(cos(elevRad) * arcLen);
  int ey = arcBaseY - (int)(sin(elevRad) * arcLen);
  tft.drawLine(arcBaseX, arcBaseY, ex, ey, 0x07E0);
  tft.drawLine(arcBaseX, arcBaseY + 1, ex, ey + 1, 0x07E0);
  tft.fillCircle(ex, ey, 3, 0xFDC0);

  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.setCursor(rx + 8, ry + 120);
  tft.print("Horizon (0 deg) -> Zenith (90 deg)");

  updateFooter("Binoculars Line-of-Sight Guide | Tap to Return");
}

void renderView() {
  drawTopTabs();
  if (currentMode == MODE_BOARD) {
    drawFlightBoard();
  } else {
    drawSpotterLine();
  }
}

void fetchLiveAirspace() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[SPOTTER] Wi-Fi not connected yet. Waiting...");
    return;
  }

  Serial.println("[SPOTTER] Querying live aircraft for 3D Spotter calculations...");
  WiFiClientSecure client;
  client.setInsecure(); // Public REST API, skip CA validation
  client.setTimeout(6000);

  HTTPClient http;
  char url[128];
  snprintf(url, sizeof(url), "https://api.adsb.lol/v2/lat/%.4f/lon/%.4f/dist/25", SPOTTER_LAT, SPOTTER_LON);
  Serial.printf("[SPOTTER] GET %s\n", url);

  if (http.begin(client, url)) {
    http.setUserAgent("CYD-SkySpotter/2.0");
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
          if (count >= MAX_FLIGHTS) break;
          if (!ac.containsKey("dst") || !ac.containsKey("dir")) continue;

          int alt = ac["alt_baro"].as<int>();
          if (alt <= 0) continue; // Airborne aircraft only

          const char* flt = ac["flight"] | "";
          String cs = String(flt);
          cs.trim();
          if (cs.length() == 0) cs = "AIR" + String(count + 1);

          strncpy(flights[count].callsign, cs.c_str(), sizeof(flights[count].callsign) - 1);
          flights[count].callsign[sizeof(flights[count].callsign) - 1] = '\0';

          const char* mdl = ac["t"] | "UNKN";
          strncpy(flights[count].aircraft, mdl, sizeof(flights[count].aircraft) - 1);
          flights[count].aircraft[sizeof(flights[count].aircraft) - 1] = '\0';

          flights[count].altitudeFt = alt;
          flights[count].speedKts = ac["gs"].as<int>();
          flights[count].bearingDeg = (int)(ac["dir"].as<float>());
          flights[count].distanceNM = ac["dst"].as<float>();

          // Calculate 3D Incline Elevation Angle: arctan(Altitude_ft / (Distance_NM * 6076.12))
          float distFt = max(0.1f, flights[count].distanceNM) * 6076.12f;
          float elevRad = atan2((float)alt, distFt);
          int elevDeg = (int)(elevRad * 57.29578f);
          flights[count].elevationDeg = constrain(elevDeg, 0, 90);

          const char* sq = ac["squawk"] | "1200";
          strncpy(flights[count].squawk, sq, sizeof(flights[count].squawk) - 1);
          flights[count].squawk[sizeof(flights[count].squawk) - 1] = '\0';

          const char* emg = ac["emergency"] | "none";
          flights[count].isAlert = (strcmp(emg, "none") != 0) || (strcmp(flights[count].squawk, "7700") == 0);
          flights[count].airlineColor = getAirlineColor(flights[count].callsign);

          count++;
        }

        if (count > 0) {
          flightCount = count;
          if (selectedFlight >= flightCount) selectedFlight = 0;
          Serial.printf("[SPOTTER] Successfully loaded %d live aircraft with 3D vectors!\n", flightCount);
          lastLiveFetch = millis();
          renderView();
        } else {
          Serial.println("[SPOTTER] No airborne aircraft found in range");
        }
      } else {
        Serial.printf("[SPOTTER] JSON error: %s\n", err.c_str());
      }
    } else {
      Serial.printf("[SPOTTER] HTTP error: %d\n", httpCode);
    }
    http.end();
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 10 Sky-Spotter Flight Deck (Live Feed) ---");
  Serial.println("Channel: @IamNotIntoTech");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");
  Serial.printf("Spotter Location: %s (%.4f, %.4f)\n", SPOTTER_NAME, SPOTTER_LAT, SPOTTER_LON);

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
  tft.drawRoundRect(16, 20, 288, 200, 8, 0x07FF);

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
  tft.print("Project 10: Sky-Spotter Flight Deck");

  tft.setTextColor(TFT_CYAN, 0x0000);
  tft.setCursor(36, 108);
  tft.printf("Origin: %s (%.2f, %.2f)", SPOTTER_NAME, SPOTTER_LAT, SPOTTER_LON);

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(36, 130);
  tft.printf("Connecting to Wi-Fi: %s...", WIFI_SSID);

  playBeep(1600, 80);

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
    playBeep(2100, 100);
  } else {
    Serial.println("\n[WIFI] Connecting in background...");
    tft.setTextColor(0xFD20, 0x0000);
    tft.setCursor(36, 150);
    tft.print("Wi-Fi connecting in background...");
    setRgbLed(true, true, false);
  }

  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(36, 172);
  tft.print("Calculating live 3D spotter vectors...");
  delay(1200);

  renderView();
  fetchLiveAirspace();
}

void loop() {
  // Periodic Live ADS-B Refresh
  if (millis() - lastLiveFetch > FETCH_INTERVAL) {
    fetchLiveAirspace();
  }

  // Touch Handling with Calibrated Non-Inverted Coordinates
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (millis() - lastTouchTime > 200) {
      lastTouchTime = millis();

      int screenX = map(p.x, TS_MINX, TS_MAXX, 0, 320);
      int screenY = map(p.y, TS_MINY, TS_MAXY, 0, 240); // Standard non-inverted Y
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      Serial.printf("[TOUCH] Raw(%d, %d) -> Screen(%d, %d)\n", p.x, p.y, screenX, screenY);

      // 1. Top Tab Switcher (Y: 24 to 46)
      if (screenY >= 24 && screenY <= 46) {
        if (screenX < 160 && currentMode != MODE_BOARD) {
          currentMode = MODE_BOARD;
          playBeep(1800, 30);
          renderView();
        } else if (screenX >= 160 && currentMode != MODE_SPOTTER_LINE) {
          currentMode = MODE_SPOTTER_LINE;
          playBeep(1800, 30);
          renderView();
        }
      }
      // 2. Flight Board Mode: Tap a flight card (Y: 48 to 220)
      else if (currentMode == MODE_BOARD && screenY >= 48 && screenY <= 220) {
        int rowIdx = (screenY - 48) / 42;
        int targetIdx = boardPage * 4 + rowIdx;
        if (targetIdx < flightCount) {
          selectedFlight = targetIdx;
          currentMode = MODE_SPOTTER_LINE;
          playBeep(2200, 40);
          renderView();
        }
      }
      // 3. Spotter Line Mode: Tap anywhere in main area to return to board
      else if (currentMode == MODE_SPOTTER_LINE && screenY > 46 && screenY < 224) {
        currentMode = MODE_BOARD;
        playBeep(1600, 30);
        renderView();
      }
      // 4. Footer Tap on Flight Board: Next Page
      else if (currentMode == MODE_BOARD && screenY >= 224) {
        if ((boardPage + 1) * 4 < flightCount) {
          boardPage++;
        } else {
          boardPage = 0;
        }
        playBeep(2000, 30);
        drawFlightBoard();
      }
    }
  }

  delay(10);
}

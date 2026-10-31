/**
 * @file 14-vector-flight-tracker.ino
 * @brief High-Contrast Vector Flight Tracker - 100% Real Live ADS-B Flight Radar Telemetry
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

#if __has_include("secrets.h")
#include "secrets.h"
#else
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define RADAR_LAT     48.8584
#define RADAR_LON     2.2945
#define RADAR_RANGE   35
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

struct FlightInfo {
  String callsign;
  String airline;
  String originIATA;
  String originCity;
  String destIATA;
  String destCity;
  String aircraft;
  int altitudeFt;
  int groundSpeedKts;
  int verticalSpeedFpm;
  int headingDeg;
  int progressPercent;
  float distanceNM;
  float directionDeg;
  uint16_t airlineThemeColor;
};

const int MAX_FLIGHTS = 12;
FlightInfo flights[MAX_FLIGHTS];
int totalFlights = 0;
int currentFlightIdx = 0;
bool autoCycle = true;
unsigned long lastCycleTime = 0;
unsigned long lastTelemetryUpdate = 0;
unsigned long lastTouchTime = 0;
bool isInitialLoadDone = false;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 50) {
  tone(BUZZER_PIN, freq, ms);
}

void updateFlightLED(const FlightInfo& f) {
  if (f.verticalSpeedFpm > 500) {
    setLED(false, true, false);  // Green: Climbing
  } else if (f.verticalSpeedFpm < -500) {
    setLED(true, true, false);   // Yellow: Descending
  } else {
    setLED(false, false, true);  // Blue/Cyan: Cruising at altitude
  }
}

String resolveAircraftModel(const String& code) {
  if (code == "B38M") return "B737 MAX 8";
  if (code == "B39M") return "B737 MAX 9";
  if (code == "B738") return "Boeing 737-800";
  if (code == "B737") return "Boeing 737-700";
  if (code == "A20N") return "Airbus A320neo";
  if (code == "A21N") return "Airbus A321neo";
  if (code == "A320") return "Airbus A320";
  if (code == "A321") return "Airbus A321";
  if (code == "A319") return "Airbus A319";
  if (code == "BCS3") return "Airbus A220-300";
  if (code == "BCS1") return "Airbus A220-100";
  if (code == "E190") return "Embraer E190";
  if (code == "E195") return "Embraer E195";
  if (code == "E295") return "Embraer E195-E2";
  if (code == "B789") return "B787-9 Dreamliner";
  if (code == "B788") return "B787-8 Dreamliner";
  if (code == "B78X") return "B787-10 Dreamliner";
  if (code == "A359") return "Airbus A350-900";
  if (code == "A35K") return "Airbus A350-1000";
  if (code == "B77W") return "Boeing 777-300ER";
  if (code == "B772") return "Boeing 777-200ER";
  if (code == "A333") return "Airbus A330-300";
  if (code == "A339") return "Airbus A330-900neo";
  if (code == "A388") return "A380-800 Superjumbo";
  if (code.length() > 0) return code;
  return "Civil Aviation";
}

void resolveAirlineInfo(const String& cs, String& airline, uint16_t& color) {
  if (cs.startsWith("AFR")) { airline = "Air France"; color = TFT_CYAN; }
  else if (cs.startsWith("BAW")) { airline = "British Airways"; color = TFT_BLUE; }
  else if (cs.startsWith("DLH")) { airline = "Lufthansa"; color = TFT_YELLOW; }
  else if (cs.startsWith("UAE")) { airline = "Emirates"; color = TFT_RED; }
  else if (cs.startsWith("KLM")) { airline = "KLM Royal Dutch"; color = tft.color565(0, 161, 222); }
  else if (cs.startsWith("IBE")) { airline = "Iberia"; color = TFT_RED; }
  else if (cs.startsWith("RYR")) { airline = "Ryanair"; color = TFT_YELLOW; }
  else if (cs.startsWith("EJU") || cs.startsWith("EZY")) { airline = "easyJet"; color = TFT_ORANGE; }
  else if (cs.startsWith("TVF") || cs.startsWith("TRA")) { airline = "Transavia"; color = TFT_GREEN; }
  else if (cs.startsWith("VLG")) { airline = "Vueling"; color = TFT_YELLOW; }
  else if (cs.startsWith("WZZ")) { airline = "Wizz Air"; color = TFT_MAGENTA; }
  else if (cs.startsWith("DAH")) { airline = "Air Algerie"; color = TFT_GREEN; }
  else if (cs.startsWith("FIN")) { airline = "Finnair"; color = TFT_BLUE; }
  else if (cs.startsWith("THY")) { airline = "Turkish Airlines"; color = TFT_RED; }
  else if (cs.startsWith("SWR")) { airline = "Swiss Int Air"; color = TFT_RED; }
  else if (cs.startsWith("SAS")) { airline = "Scandinavian Air"; color = TFT_BLUE; }
  else if (cs.startsWith("TAP")) { airline = "TAP Air Portugal"; color = TFT_GREEN; }
  else if (cs.startsWith("TVS")) { airline = "Smartwings"; color = TFT_ORANGE; }
  else if (cs.startsWith("NSZ")) { airline = "Norwegian Air"; color = TFT_RED; }
  else if (cs.startsWith("JAF")) { airline = "TUI fly Belgium"; color = TFT_CYAN; }
  else { airline = "Commercial Jet"; color = TFT_CYAN; }
}

String getCompassHeading(int deg) {
  deg = (deg % 360 + 360) % 360;
  if (deg >= 338 || deg < 23) return "N VECTOR";
  if (deg < 68) return "NE VECTOR";
  if (deg < 113) return "E VECTOR";
  if (deg < 158) return "SE VECTOR";
  if (deg < 203) return "S VECTOR";
  if (deg < 248) return "SW VECTOR";
  if (deg < 293) return "W VECTOR";
  return "NW VECTOR";
}

void drawFlightCard() {
  if (totalFlights == 0) {
    tft.fillRect(0, 0, 320, 240, TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawCentreString("PARIS RADAR SECTOR", 160, 60, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawCentreString("Scanning 35 NM airspace...", 160, 110, 2);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawCentreString("Connecting to live ADS-B feed", 160, 140, 2);
    return;
  }

  FlightInfo& f = flights[currentFlightIdx];

  // Header Bar
  tft.fillRect(0, 0, 320, 28, tft.color565(15, 20, 30));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(15, 20, 30));
  tft.setTextSize(1);
  tft.drawString("PARIS RADAR SECTOR (JN18eu)", 8, 8, 2);

  char countBuf[20];
  snprintf(countBuf, sizeof(countBuf), "%d OF %d LIVE", currentFlightIdx + 1, totalFlights);
  tft.drawRightString(countBuf, 312, 8, 2);

  // Background body
  tft.fillRect(0, 28, 320, 172, TFT_BLACK);

  // Call Sign & Airline Banner
  tft.setTextColor(f.airlineThemeColor, TFT_BLACK);
  tft.setTextSize(4);
  tft.drawString(f.callsign, 12, 34);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString(f.airline, 14, 70, 2);
  tft.drawRightString(f.aircraft, 308, 70, 2);

  // Route: Origin -> Destination
  tft.fillRoundRect(8, 90, 304, 38, 4, tft.color565(20, 25, 35));
  tft.drawRoundRect(8, 90, 304, 38, 4, TFT_DARKGREY);

  tft.setTextColor(TFT_WHITE, tft.color565(20, 25, 35));
  tft.drawString(f.originIATA, 20, 96, 2);
  tft.setTextColor(TFT_CYAN, tft.color565(20, 25, 35));
  tft.drawString("------>", 115, 96, 2);
  tft.setTextColor(TFT_WHITE, tft.color565(20, 25, 35));
  tft.drawRightString(f.destIATA, 300, 96, 2);

  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 25, 35));
  tft.drawString(f.originCity, 20, 114, 1);
  tft.drawRightString(f.destCity, 300, 114, 1);

  // Flight Progress Bar
  int barX = 14;
  int barY = 134;
  int barW = 292;
  int barH = 6;
  tft.fillRect(barX, barY, barW, barH, TFT_DARKGREY);
  int progressW = (barW * f.progressPercent) / 100;
  tft.fillRect(barX, barY, progressW, barH, TFT_GREEN);

  // Airplane Marker Icon
  int planeX = barX + progressW;
  tft.fillTriangle(planeX, barY - 3, planeX + 5, barY + 3, planeX - 5, barY + 3, TFT_YELLOW);

  // Vector Metrics Grid (Altitude, Speed, Heading)
  // Box 1: Altitude
  tft.fillRoundRect(8, 145, 96, 50, 4, tft.color565(25, 30, 45));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 45));
  tft.drawString("ALTITUDE", 14, 150, 1);
  tft.setTextColor(TFT_WHITE, tft.color565(25, 30, 45));
  tft.setTextSize(2);
  char altBuf[16];
  snprintf(altBuf, sizeof(altBuf), "%dK", f.altitudeFt / 1000);
  tft.drawString(altBuf, 14, 164);
  tft.setTextSize(1);
  if (f.verticalSpeedFpm > 400) {
    tft.setTextColor(TFT_GREEN, tft.color565(25, 30, 45));
    tft.drawString("+CLIMB", 14, 182);
  } else if (f.verticalSpeedFpm < -400) {
    tft.setTextColor(TFT_ORANGE, tft.color565(25, 30, 45));
    tft.drawString("-DESC", 14, 182);
  } else {
    tft.setTextColor(TFT_CYAN, tft.color565(25, 30, 45));
    tft.drawString("CRUISE", 14, 182);
  }

  // Box 2: Ground Speed
  tft.fillRoundRect(112, 145, 96, 50, 4, tft.color565(25, 30, 45));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 45));
  tft.drawString("SPEED", 118, 150, 1);
  tft.setTextColor(TFT_WHITE, tft.color565(25, 30, 45));
  tft.setTextSize(2);
  char spdBuf[16];
  snprintf(spdBuf, sizeof(spdBuf), "%d", f.groundSpeedKts);
  tft.drawString(spdBuf, 118, 164);
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 45));
  char machBuf[16];
  snprintf(machBuf, sizeof(machBuf), "KTS / M%.2f", f.groundSpeedKts / 661.47f);
  tft.drawString(machBuf, 118, 182);

  // Box 3: Heading & Vector
  tft.fillRoundRect(216, 145, 96, 50, 4, tft.color565(25, 30, 45));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 45));
  tft.drawString("HEADING", 222, 150, 1);
  tft.setTextColor(TFT_WHITE, tft.color565(25, 30, 45));
  tft.setTextSize(2);
  char hdgBuf[16];
  snprintf(hdgBuf, sizeof(hdgBuf), "%d deg", f.headingDeg);
  tft.drawString(hdgBuf, 222, 164);
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW, tft.color565(25, 30, 45));
  tft.drawString(getCompassHeading(f.headingDeg), 222, 182);

  // Bottom Touch Controls
  tft.fillRoundRect(8, 202, 95, 32, 4, tft.color565(35, 40, 50));
  tft.drawRoundRect(8, 202, 95, 32, 4, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, tft.color565(35, 40, 50));
  tft.drawCentreString("< PREV", 55, 210, 2);

  uint16_t autoCol = autoCycle ? TFT_DARKCYAN : tft.color565(35, 40, 50);
  tft.fillRoundRect(112, 202, 96, 32, 4, autoCol);
  tft.drawRoundRect(112, 202, 96, 32, 4, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, autoCol);
  tft.drawCentreString(autoCycle ? "AUTO ON" : "PAUSED", 160, 210, 2);

  tft.fillRoundRect(216, 202, 96, 32, 4, tft.color565(35, 40, 50));
  tft.drawRoundRect(216, 202, 96, 32, 4, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, tft.color565(35, 40, 50));
  tft.drawCentreString("NEXT >", 264, 210, 2);

  updateFlightLED(f);
}

// Fetch 100% Real Live ADS-B Flights around Paris Eiffel Tower
bool fetchRealFlights() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ADS-B] Waiting for Wi-Fi connection...");
    return false;
  }

  Serial.println("[ADS-B] Fetching real live flight radar data from api.adsb.lol...");
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(6000);

  HTTPClient http;
  char url[128];
  snprintf(url, sizeof(url), "https://api.adsb.lol/v2/lat/%.4f/lon/%.4f/dist/%d", RADAR_LAT, RADAR_LON, RADAR_RANGE);

  if (!http.begin(client, url)) {
    Serial.println("[ADS-B] HTTP begin failed!");
    return false;
  }

  http.setUserAgent("CYD-VectorFlight/2.0");
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[ADS-B] HTTP GET failed, error: %d\n", httpCode);
    http.end();
    return false;
  }

  StaticJsonDocument<384> filter;
  JsonObject acFilter = filter["ac"].createNestedObject();
  acFilter["flight"] = true;
  acFilter["t"] = true;
  acFilter["alt_baro"] = true;
  acFilter["gs"] = true;
  acFilter["baro_rate"] = true;
  acFilter["track"] = true;
  acFilter["dst"] = true;
  acFilter["dir"] = true;

  DynamicJsonDocument doc(24576);
  DeserializationError err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
  http.end();

  if (err) {
    Serial.printf("[ADS-B] JSON parse error: %s\n", err.c_str());
    return false;
  }

  JsonArray acList = doc["ac"].as<JsonArray>();
  int parsedCount = 0;

  for (JsonObject ac : acList) {
    if (parsedCount >= MAX_FLIGHTS) break;
    if (!ac.containsKey("dst") || !ac.containsKey("dir")) continue;

    const char* flt = ac["flight"] | "";
    String cs = String(flt);
    cs.trim();
    if (cs.length() == 0) continue; // Skip unassigned callsigns

    flights[parsedCount].callsign = cs;
    resolveAirlineInfo(cs, flights[parsedCount].airline, flights[parsedCount].airlineThemeColor);

    const char* t = ac["t"] | "";
    flights[parsedCount].aircraft = resolveAircraftModel(String(t));

    flights[parsedCount].altitudeFt = ac["alt_baro"].as<int>();
    flights[parsedCount].groundSpeedKts = ac["gs"].as<int>();
    flights[parsedCount].verticalSpeedFpm = ac["baro_rate"].as<int>();
    flights[parsedCount].headingDeg = ac["track"].as<int>();
    flights[parsedCount].distanceNM = ac["dst"].as<float>();
    flights[parsedCount].directionDeg = ac["dir"].as<float>();

    // Route Sector & Direction
    if (flights[parsedCount].altitudeFt < 12000 && flights[parsedCount].verticalSpeedFpm < -400) {
      flights[parsedCount].originIATA = "SECTOR";
      flights[parsedCount].originCity = "Approaching";
      flights[parsedCount].destIATA = "PARIS";
      flights[parsedCount].destCity = "LFPG / LFPO";
    } else if (flights[parsedCount].altitudeFt < 12000 && flights[parsedCount].verticalSpeedFpm > 400) {
      flights[parsedCount].originIATA = "PARIS";
      flights[parsedCount].originCity = "LFPG / LFPO";
      flights[parsedCount].destIATA = "EN ROUTE";
      flights[parsedCount].destCity = "Climbing Sector";
    } else {
      flights[parsedCount].originIATA = "PARIS";
      flights[parsedCount].originCity = "Overhead 35NM";
      flights[parsedCount].destIATA = "CRUISE";
      char dirBuf[24];
      snprintf(dirBuf, sizeof(dirBuf), "%.0f NM @ %.0f deg", flights[parsedCount].distanceNM, flights[parsedCount].directionDeg);
      flights[parsedCount].destCity = String(dirBuf);
    }

    // Dynamic progress bar: distance from radar boundary (35 NM) to center
    int prog = (int)((35.0f - flights[parsedCount].distanceNM) / 35.0f * 100.0f);
    flights[parsedCount].progressPercent = constrain(prog, 5, 95);

    Serial.printf("[ADS-B] Flight %d: %s (%s, %s) Alt: %d ft, Speed: %d kts, Hdg: %d deg, Dist: %.1f NM\n",
                  parsedCount + 1, cs.c_str(), flights[parsedCount].airline.c_str(),
                  flights[parsedCount].aircraft.c_str(), flights[parsedCount].altitudeFt,
                  flights[parsedCount].groundSpeedKts, flights[parsedCount].headingDeg,
                  flights[parsedCount].distanceNM);

    parsedCount++;
  }

  if (parsedCount > 0) {
    totalFlights = parsedCount;
    if (currentFlightIdx >= totalFlights) currentFlightIdx = 0;
    Serial.printf("[ADS-B] Successfully loaded %d real live aircraft.\n", totalFlights);
    return true;
  }

  Serial.println("[ADS-B] No commercial callsigns currently in sector.");
  return false;
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

      if (totalFlights == 0) return;

      // Bottom control buttons (Y >= 195)
      if (y >= 195) {
        if (x < 105) {
          // PREV
          currentFlightIdx = (currentFlightIdx - 1 + totalFlights) % totalFlights;
          beep(1800, 40);
          drawFlightCard();
        } else if (x >= 110 && x < 210) {
          // AUTO TOGGLE
          autoCycle = !autoCycle;
          beep(2200, 40);
          drawFlightCard();
        } else if (x >= 215) {
          // NEXT
          currentFlightIdx = (currentFlightIdx + 1) % totalFlights;
          beep(2500, 40);
          drawFlightCard();
        }
      }
      // Tap on flight card body to cycle flight
      else if (y > 28 && y < 195) {
        currentFlightIdx = (currentFlightIdx + 1) % totalFlights;
        beep(2000, 30);
        drawFlightCard();
      }
    }
  }
}

void connectWiFi() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawCentreString("VECTOR FLIGHT TRACKER", 160, 45, 2);

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
    tft.drawCentreString("Connected! Acquiring Live Flights...", 160, 160, 2);
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
  Serial.println("  14 VECTOR FLIGHT TRACKER - REAL LIVE ADS-B (CYD 2.8)");
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
  beep(2000, 60);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize touch on dedicated CYD SPI pins
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Connect to Wi-Fi
  connectWiFi();

  // Fetch real live flight telemetry
  fetchRealFlights();
  isInitialLoadDone = true;
  lastTelemetryUpdate = millis();
  lastCycleTime = millis();

  drawFlightCard();
}

void loop() {
  checkTouch();

  // Auto-cycle through real flights every 6 seconds if autoCycle is enabled
  if (autoCycle && totalFlights > 1 && (millis() - lastCycleTime >= 6000)) {
    lastCycleTime = millis();
    currentFlightIdx = (currentFlightIdx + 1) % totalFlights;
    beep(1600, 30);
    drawFlightCard();
  }

  // Refresh real ADS-B feed every 30 seconds
  if (millis() - lastTelemetryUpdate >= 30000) {
    lastTelemetryUpdate = millis();
    Serial.println("[CYCLE] Refreshing live ADS-B flight telemetry...");
    if (WiFi.status() == WL_CONNECTED) {
      if (fetchRealFlights()) {
        drawFlightCard();
      }
    }
  }

  delay(15);
}

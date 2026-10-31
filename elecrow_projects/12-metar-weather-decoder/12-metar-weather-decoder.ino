/**
 * @file 12-metar-weather-decoder.ino
 * @brief Pilot's Desk METAR & TAF Aviation Weather Decoder (Live NOAA Stream)
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 (HSPI)
 * - Touch: XPT2046 Resistive Touch (Dedicated VSPI: MOSI 32, MISO 39, CLK 25, CS 33)
 * - Calibrated non-inverted landscape coordinate mapping
 * - Network: ESP32 Wi-Fi + NOAA Aviation Weather REST API
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

// Wi-Fi Credentials & Configuration (Stored securely in secrets.h)
#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #define WIFI_SSID     "YOUR_WIFI_SSID"
  #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
  #define METAR_AIRPORTS "LFPG,LFPO,LFPB,EGLL"
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

enum FlightCategory {
  CAT_VFR = 0,
  CAT_MVFR,
  CAT_IFR,
  CAT_LIFR
};

struct MetarStation {
  char icao[8];
  char name[32];
  char rawMetar[96];
  FlightCategory category;
  int windDir;
  int windSpeedKts;
  int rwyHeading;
  int tempC;
  int dewC;
  char vis[16];
  char clouds[32];
  int altimeterHpa;
};

#define STATION_COUNT 4
MetarStation stations[STATION_COUNT] = {
  {"LFPG", "Paris Charles de Gaulle", "Connecting to live NOAA feed...", CAT_VFR, 20, 5, 270, 10, 7, "10+ km", "CAVOK", 1029},
  {"LFPO", "Paris Orly Airport",     "Connecting to live NOAA feed...", CAT_VFR, 20, 5, 240, 10, 6, "10+ km", "CAVOK", 1029},
  {"LFPB", "Paris Le Bourget",        "Connecting to live NOAA feed...", CAT_VFR, 360, 5, 210, 10, 7, "10+ km", "CAVOK", 1030},
  {"EGLL", "London Heathrow",         "Connecting to live NOAA feed...", CAT_VFR, 20, 2, 270, 10, 9, "9 km", "NCD", 1032}
};

int currentStation = 0;
unsigned long lastTouchTime = 0;
unsigned long lastMetarFetch = 0;
const unsigned long FETCH_INTERVAL = 60000; // Poll NOAA every 60 seconds

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

uint16_t getCategoryColor(FlightCategory cat) {
  switch (cat) {
    case CAT_VFR:  return 0x07E0; // Green
    case CAT_MVFR: return 0x001F; // Blue
    case CAT_IFR:  return 0xF800; // Red
    case CAT_LIFR: return 0xF81F; // Magenta
    default: return TFT_WHITE;
  }
}

const char* getCategoryLabel(FlightCategory cat) {
  switch (cat) {
    case CAT_VFR:  return "VFR (VISUAL)";
    case CAT_MVFR: return "MVFR (MARGINAL)";
    case CAT_IFR:  return "IFR (INSTRUMENT)";
    case CAT_LIFR: return "LIFR (LOW IFR)";
    default: return "UNKNOWN";
  }
}

FlightCategory parseFlightCategory(const char* cat) {
  if (strcmp(cat, "VFR") == 0) return CAT_VFR;
  if (strcmp(cat, "MVFR") == 0) return CAT_MVFR;
  if (strcmp(cat, "IFR") == 0) return CAT_IFR;
  if (strcmp(cat, "LIFR") == 0) return CAT_LIFR;
  return CAT_VFR;
}

void drawStationTabs() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("AVIATION METAR - ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print("PARIS REGION");

  tft.setTextColor(WiFi.status() == WL_CONNECTED ? 0x07E0 : 0xF800, 0x0010);
  tft.setCursor(220, 4);
  tft.print(WiFi.status() == WL_CONNECTED ? "LIVE NOAA DATA" : "OFFLINE");

  // 4 Tabs Across
  int tabW = 80;
  for (int i = 0; i < STATION_COUNT; i++) {
    bool isSel = (i == currentStation);
    uint16_t bg = isSel ? 0xFDC0 : 0x2104;
    uint16_t fg = isSel ? TFT_BLACK : TFT_LIGHTGREY;
    tft.fillRect(i * tabW, 24, tabW - 1, 20, bg);
    tft.drawRect(i * tabW, 24, tabW - 1, 20, TFT_BLACK);
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);
    tft.setCursor(i * tabW + 18, 29);
    tft.print(stations[i].icao);
  }
}

void drawDecodedWeather() {
  tft.fillRect(0, 44, 320, 180, 0x0821);

  const MetarStation& s = stations[currentStation];
  uint16_t catCol = getCategoryColor(s.category);

  // Station Header & Flight Rule Badge
  tft.fillRoundRect(6, 48, 308, 34, 4, 0x0000);
  tft.drawRoundRect(6, 48, 308, 34, 4, catCol);

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(14, 54);
  tft.printf("%s", s.icao);

  tft.setTextSize(1);
  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(72, 58);
  tft.print(s.name);

  // Category Badge (right side)
  tft.fillRoundRect(198, 52, 110, 22, 3, catCol);
  tft.setTextColor(TFT_BLACK, catCol);
  tft.setTextSize(1);
  tft.setCursor(206, 58);
  tft.print(getCategoryLabel(s.category));

  // Decoded Elements (Left Card)
  int lx = 6;
  int ly = 86;
  int lw = 152;
  int lh = 88;

  tft.fillRoundRect(lx, ly, lw, lh, 4, 0x0000);
  tft.drawRoundRect(lx, ly, lw, lh, 4, 0x2965);

  tft.setTextColor(0xFDC0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(lx + 8, ly + 8);
  tft.printf("WIND: %03d @ %d kts", s.windDir, s.windSpeedKts);

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(lx + 8, ly + 24);
  tft.printf("VIS:  %s", s.vis);

  tft.setCursor(lx + 8, ly + 40);
  tft.printf("CLD:  %s", s.clouds);

  tft.setCursor(lx + 8, ly + 56);
  tft.printf("TEMP: %d C  |  DEW: %d C", s.tempC, s.dewC);

  tft.setTextColor(0x07E0, 0x0000);
  tft.setCursor(lx + 8, ly + 72);
  tft.printf("QNH:  %d hPa", s.altimeterHpa);

  // Runway Crosswind Vector Calculator (Right Card)
  int rx = 162;
  int ry = 86;
  int rw = 152;
  int rh = 88;

  tft.fillRoundRect(rx, ry, rw, rh, 4, 0x0000);
  tft.drawRoundRect(rx, ry, rw, rh, 4, 0x07FF);

  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(rx + 8, ry + 8);
  tft.printf("RUNWAY %02d (Heading %03d)", s.rwyHeading / 10, s.rwyHeading);

  // Compute Crosswind & Headwind
  float angleDiffRad = (s.windDir - s.rwyHeading) * 0.0174533f;
  float headwind = cos(angleDiffRad) * s.windSpeedKts;
  float crosswind = sin(angleDiffRad) * s.windSpeedKts;

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(rx + 8, ry + 24);
  tft.printf("Headwind:  %+.1f kts", headwind);

  tft.setTextColor(fabs(crosswind) > 12.0f ? 0xF800 : 0x07E0, 0x0000);
  tft.setCursor(rx + 8, ry + 40);
  tft.printf("Crosswind: %.1f kt (%s)", fabs(crosswind), (crosswind >= 0 ? "R" : "L"));

  // Visual Crosswind Limit Bar
  tft.drawRect(rx + 8, ry + 58, 134, 10, TFT_WHITE);
  int cwFill = map((int)fabs(crosswind), 0, 25, 0, 130);
  cwFill = constrain(cwFill, 0, 130);
  uint16_t cwBarCol = (fabs(crosswind) > 15) ? 0xF800 : (fabs(crosswind) > 8 ? 0xFD20 : 0x07E0);
  tft.fillRect(rx + 10, ry + 60, cwFill, 6, cwBarCol);

  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.setCursor(rx + 8, ry + 74);
  tft.print("Max Component: 25 kts limit");

  // Raw METAR String Box
  int rawY = 178;
  tft.fillRoundRect(6, rawY, 308, 38, 3, 0x10A2);
  tft.drawRoundRect(6, rawY, 308, 38, 3, 0x2965);

  tft.setTextColor(0x07E0, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(12, rawY + 6);
  tft.print("RAW METAR:");
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setCursor(12, rawY + 20);
  char shortRaw[48];
  strncpy(shortRaw, s.rawMetar, sizeof(shortRaw) - 1);
  shortRaw[sizeof(shortRaw) - 1] = '\0';
  tft.print(shortRaw);

  // Footer
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(8, 228);
  tft.print("Live NOAA Aviation Weather Stream | @IamNotIntoTech");

  // Update RGB LED to match flight rule category
  if (s.category == CAT_VFR) setRgbLed(false, true, false);      // Green
  else if (s.category == CAT_MVFR) setRgbLed(false, false, true); // Blue
  else setRgbLed(true, false, false);                           // Red for IFR / LIFR
}

void renderView() {
  drawStationTabs();
  drawDecodedWeather();
}

void fetchLiveMetar() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[METAR] Wi-Fi not connected yet. Waiting...");
    return;
  }

  Serial.println("[METAR] Querying live NOAA Aviation Weather observations...");
  WiFiClientSecure client;
  client.setInsecure(); // Public NOAA API
  client.setTimeout(6000);

  HTTPClient http;
  const char* url = "https://aviationweather.gov/api/data/metar?ids=LFPG,LFPO,LFPB,EGLL&format=json";

  if (http.begin(client, url)) {
    http.setUserAgent("CYD-MetarDecoder/2.0");
    int code = http.GET();
    if (code == HTTP_CODE_OK) {
      String payload = http.getString();
      DynamicJsonDocument doc(4096);
      DeserializationError err = deserializeJson(doc, payload);
      if (!err) {
        JsonArray arr = doc.as<JsonArray>();
        for (JsonObject obs : arr) {
          const char* icao = obs["icaoId"] | "";
          for (int i = 0; i < STATION_COUNT; i++) {
            if (strcmp(stations[i].icao, icao) == 0) {
              const char* raw = obs["rawOb"] | "";
              strncpy(stations[i].rawMetar, raw, sizeof(stations[i].rawMetar) - 1);
              stations[i].rawMetar[sizeof(stations[i].rawMetar) - 1] = '\0';

              const char* fltCat = obs["fltCat"] | "VFR";
              stations[i].category = parseFlightCategory(fltCat);

              stations[i].windDir = obs["wdir"].as<int>();
              stations[i].windSpeedKts = obs["wspd"].as<int>();
              stations[i].tempC = obs["temp"].as<int>();
              stations[i].dewC = obs["dewp"].as<int>();
              stations[i].altimeterHpa = (int)(obs["altim"].as<float>());

              const char* vis = obs["visib"] | "10+ km";
              strncpy(stations[i].vis, vis, sizeof(stations[i].vis) - 1);
              stations[i].vis[sizeof(stations[i].vis) - 1] = '\0';

              if (obs.containsKey("clouds") && obs["clouds"].size() > 0) {
                const char* cov = obs["clouds"][0]["cover"] | "CAVOK";
                int base = obs["clouds"][0]["base"] | 0;
                if (base > 0) {
                  snprintf(stations[i].clouds, sizeof(stations[i].clouds), "%s %d ft", cov, base);
                } else {
                  snprintf(stations[i].clouds, sizeof(stations[i].clouds), "%s", cov);
                }
              } else {
                strncpy(stations[i].clouds, "CAVOK", sizeof(stations[i].clouds) - 1);
              }

              Serial.printf("[METAR] %s: %s | Wind: %d@%d | %dC/%dC | %s\n",
                            stations[i].icao, fltCat, stations[i].windDir, 
                            stations[i].windSpeedKts, stations[i].tempC, stations[i].dewC, raw);
              break;
            }
          }
        }
        lastMetarFetch = millis();
        renderView();
      } else {
        Serial.printf("[METAR] JSON parse error: %s\n", err.c_str());
      }
    } else {
      Serial.printf("[METAR] HTTP error: %d\n", code);
    }
    http.end();
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 12 Live METAR Aviation Weather Decoder ---");
  Serial.println("Channel: @IamNotIntoTech");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");
  Serial.println("Monitored Airports: Paris CDG (LFPG), Paris Orly (LFPO), Le Bourget (LFPB), London (EGLL)");

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
  tft.print("Project 12: Live Aviation METAR Decoder");

  tft.setTextColor(TFT_CYAN, 0x0000);
  tft.setCursor(36, 108);
  tft.print("Stations: LFPG, LFPO, LFPB, EGLL");

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
    Serial.println("\n[WIFI] Connecting in background...");
    tft.setTextColor(0xFD20, 0x0000);
    tft.setCursor(36, 150);
    tft.print("Wi-Fi connecting in background...");
    setRgbLed(true, true, false);
  }

  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(36, 172);
  tft.print("Streaming live NOAA weather observations...");
  delay(1200);

  renderView();
  fetchLiveMetar();
}

void loop() {
  // Periodic Live METAR Refresh
  if (millis() - lastMetarFetch > FETCH_INTERVAL) {
    fetchLiveMetar();
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

      // 1. Station Tabs (Y: 22 to 46)
      if (screenY >= 22 && screenY <= 46) {
        int tabIdx = screenX / 80;
        if (tabIdx >= 0 && tabIdx < STATION_COUNT) {
          if (tabIdx != currentStation) {
            currentStation = tabIdx;
            playBeep(2000, 35);
            renderView();
          }
        }
      }
      // 2. Tap on Weather Card or Raw METAR: Trigger Manual Refresh
      else if (screenY > 46) {
        playBeep(2400, 40);
        tft.fillRect(0, 224, 320, 16, 0x07E0);
        tft.setTextColor(TFT_BLACK, 0x07E0);
        tft.setTextSize(1);
        tft.setCursor(8, 228);
        tft.print("Refreshing live NOAA observation...");
        fetchLiveMetar();
      }
    }
  }

  delay(10);
}

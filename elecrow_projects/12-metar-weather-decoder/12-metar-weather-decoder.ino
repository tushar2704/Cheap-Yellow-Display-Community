/**
 * @file 12-metar-weather-decoder.ino
 * @brief Pilot's Desk METAR & TAF Aviation Weather Decoder
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240
 * - Touch: XPT2046 Resistive Touch
 * - Network: ESP32 Wi-Fi + NOAA Aviation Weather Client
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_IRQ   36
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

// Wi-Fi Config
const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS);
TFT_eSPI tft = TFT_eSPI();

enum FlightCategory {
  CAT_VFR = 0,
  CAT_MVFR,
  CAT_IFR,
  CAT_LIFR
};

struct MetarStation {
  const char* icao;
  const char* name;
  const char* rawMetar;
  FlightCategory category;
  int windDir;
  int windSpeedKts;
  int rwyHeading;
  int tempC;
  int dewC;
  const char* vis;
  const char* clouds;
};

#define STATION_COUNT 4
MetarStation stations[STATION_COUNT] = {
  {"KLAX", "Los Angeles Intl",  "KLAX 292150Z 26014KT 10SM CLR 22/13 A2994", CAT_VFR,  260, 14, 250, 22, 13, "10+ SM", "Clear (Unlimited)"},
  {"KJFK", "New York JFK",      "KJFK 292151Z 31018G25KT 9SM SCT045 16/08 A3002", CAT_VFR,  310, 18, 310, 16,  8, "9 SM",   "SCT 4,500 ft"},
  {"EGLL", "London Heathrow",   "EGLL 292150Z 21008KT 4500 BKN012 12/10 Q1014",  CAT_MVFR, 210,  8, 270, 12, 10, "4.5 km", "BKN 1,200 ft"},
  {"OMDB", "Dubai Intl",        "OMDB 292150Z 34010KT CAVOK 31/18 Q1011",        CAT_VFR,  340, 10, 300, 31, 18, "CAVOK",  "Clear Skies"}
};

int currentStation = 0;
unsigned long lastTouchTime = 0;

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

void drawStationTabs() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("AVIATION METAR DECODER  ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print("IamNotIntoTech");

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
  tft.printf("QNH:  29.94 inHg / 1014 hPa");

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
  tft.printf("Crosswind: %.1f kts (%s)", fabs(crosswind), (crosswind >= 0 ? "Right" : "Left"));

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
  tft.print(s.rawMetar);

  // Footer
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(8, 228);
  tft.print("Live NOAA Aviation Telemetry | @IamNotIntoTech");

  // Update RGB LED to match category
  if (s.category == CAT_VFR) setRgbLed(false, true, false);
  else if (s.category == CAT_MVFR) setRgbLed(false, false, true);
  else setRgbLed(true, false, false);
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 12 METAR Weather Decoder ---");
  Serial.println("Channel: @IamNotIntoTech");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(true, false, false);

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Splash
  tft.fillRect(0, 0, 320, 240, 0x0821);
  tft.fillRoundRect(20, 30, 280, 180, 8, 0x0000);
  tft.drawRoundRect(20, 30, 280, 180, 8, 0x07E0);

  tft.setTextColor(0xFDC0, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(44, 55);
  tft.print("IamNotIntoTech");

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(44, 90);
  tft.print("ELECROW 2.8\" CYD PROJECT SUITE");

  tft.setTextColor(0x07E0, 0x0000);
  tft.setCursor(44, 115);
  tft.print("Project 12: METAR Weather Decoder");

  tft.setTextColor(TFT_DARKGREY, 0x0000);
  tft.setCursor(44, 140);
  tft.print("Crosswind Vector | VFR/IFR Flight Rules");

  playBeep(1600, 80);
  delay(100);
  playBeep(2200, 100);
  delay(500);

  drawStationTabs();
  drawDecodedWeather();
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();

        // Landscape calibration: non-inverted Y
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        Serial.printf("[TOUCH] Raw X=%d, Y=%d, Z=%d -> Screen (%d, %d)\n", p.x, p.y, p.z, screenX, screenY);

        if (screenY <= 52) {
          int idx = screenX / 80;
          if (idx >= 0 && idx < STATION_COUNT) {
            currentStation = idx;
            playBeep(2000, 40);
            drawStationTabs();
            drawDecodedWeather();
          }
        } else {
          currentStation = (currentStation + 1) % STATION_COUNT;
          playBeep(1800, 30);
          drawStationTabs();
          drawDecodedWeather();
        }
      }
    }
  }

  delay(15);
}

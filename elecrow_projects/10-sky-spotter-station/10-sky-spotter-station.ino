/**
 * @file 10-sky-spotter-station.ino
 * @brief Sky-Spotter Flight Deck with Spotter Line Vector Calculation
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240
 * - Touch: XPT2046 Resistive Touch (SPI: CLK=25, MISO=39, MOSI=32, CS=33)
 * - Network: ESP32 Wi-Fi + Flight Tracker Engine
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Accurate CYD 2.8" Resistive Touch SPI Pinout
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33
#define XPT2046_IRQ   36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26

// Wi-Fi Config
const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS); // Polling mode avoids GPIO 36 interrupt lockup
TFT_eSPI tft = TFT_eSPI();

enum SpotterMode {
  MODE_BOARD = 0,
  MODE_SPOTTER_LINE = 1
};

SpotterMode currentMode = MODE_BOARD;
int selectedFlight = 0;

struct FlightInfo {
  const char* flightNo;
  const char* airline;
  const char* route;
  const char* aircraft;
  int altitudeFt;
  int speedKts;
  int bearingDeg;
  int elevationDeg; // 0 (horizon) to 90 (zenith overhead)
  float distanceNM;
  uint16_t airlineColor;
};

#define FLIGHT_COUNT 4
FlightInfo flights[FLIGHT_COUNT] = {
  {"DL 482", "Delta Air Lines", "JFK -> LAX", "A350-900", 34000, 490, 48, 64, 4.2f, 0x001F},
  {"UA 915", "United Airlines", "SFO -> LHR", "B787-9",   38000, 520, 185, 28, 9.8f, 0x03FF},
  {"AA 104", "American Air",    "ORD -> MIA", "B737-MAX", 28000, 440, 310, 45, 6.1f, 0xF800},
  {"LH 430", "Lufthansa Cargo", "FRA -> ORD", "B777-200", 31000, 480, 115, 78, 2.7f, 0xFDC0}
};

unsigned long lastTouchTime = 0;
unsigned long footerClearTime = 0;

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

void drawTopTabs() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("SKY-SPOTTER DECK  ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print("IamNotIntoTech");

  // Tabs
  uint16_t bBg = (currentMode == MODE_BOARD) ? 0xFDC0 : 0x2104;
  uint16_t bFg = (currentMode == MODE_BOARD) ? TFT_BLACK : TFT_LIGHTGREY;
  tft.fillRect(0, 24, 159, 20, bBg);
  tft.drawRect(0, 24, 159, 20, TFT_BLACK);
  tft.setTextColor(bFg, bBg);
  tft.setCursor(35, 29);
  tft.print("[1] FLIGHT BOARD");

  uint16_t sBg = (currentMode == MODE_SPOTTER_LINE) ? 0xFDC0 : 0x2104;
  uint16_t sFg = (currentMode == MODE_SPOTTER_LINE) ? TFT_BLACK : TFT_LIGHTGREY;
  tft.fillRect(160, 24, 160, 20, sBg);
  tft.drawRect(160, 24, 160, 20, TFT_BLACK);
  tft.setTextColor(sFg, sBg);
  tft.setCursor(185, 29);
  tft.print("[2] SPOTTER LINE (3D)");
}

void drawFlightBoard() {
  tft.fillRect(0, 44, 320, 180, 0x0821);

  for (int i = 0; i < FLIGHT_COUNT; i++) {
    int y = 48 + i * 42;
    const FlightInfo& f = flights[i];
    bool isSel = (i == selectedFlight);

    tft.fillRoundRect(6, y, 308, 38, 4, isSel ? 0x0000 : 0x10A2);
    tft.drawRoundRect(6, y, 308, 38, 4, isSel ? 0x07E0 : TFT_DARKGREY);

    // Airline Color Bar
    tft.fillRect(8, y + 4, 4, 30, f.airlineColor);

    // Flight & Route
    tft.setTextColor(TFT_WHITE, isSel ? 0x0000 : 0x10A2);
    tft.setTextSize(1);
    tft.setCursor(18, y + 6);
    tft.printf("%s (%s)", f.flightNo, f.route);

    tft.setTextColor(0x07FF, isSel ? 0x0000 : 0x10A2);
    tft.setCursor(18, y + 20);
    tft.printf("%s | Alt: %d ft", f.aircraft, f.altitudeFt);

    // Spotting Angle & Distance
    tft.setTextColor(isSel ? 0xFDC0 : TFT_LIGHTGREY, isSel ? 0x0000 : 0x10A2);
    tft.setCursor(200, y + 6);
    tft.printf("Brg: %03d deg", f.bearingDeg);

    tft.setCursor(200, y + 20);
    tft.printf("Elev: %d deg (%.1fNM)", f.elevationDeg, f.distanceNM);
  }

  updateFooter("Tap flight to view 3D Spotter Vector | @IamNotIntoTech");
}

void drawSpotterLine() {
  tft.fillRect(0, 44, 320, 180, 0x0821);

  const FlightInfo& f = flights[selectedFlight];

  // Left Section: Compass Azimuth (X: 0 to 150)
  int cx = 75;
  int cy = 132;
  int r = 55;

  tft.drawCircle(cx, cy, r, 0x07E0);
  tft.drawCircle(cx, cy, 2, TFT_WHITE);

  // Compass ticks
  tft.setTextColor(0x07E0, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(cx - 3, cy - r - 10); tft.print("N");
  tft.setCursor(cx + r + 3, cy - 3);   tft.print("E");
  tft.setCursor(cx - 3, cy + r + 3);   tft.print("S");
  tft.setCursor(cx - r - 10, cy - 3);  tft.print("W");

  // Draw Heading Needle
  float rad = (f.bearingDeg - 90) * 0.0174533f;
  int nx = cx + (int)(cos(rad) * (r - 4));
  int ny = cy + (int)(sin(rad) * (r - 4));
  tft.drawLine(cx, cy, nx, ny, 0xF800);
  tft.fillCircle(nx, ny, 3, 0xF800);

  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setCursor(30, 52);
  tft.printf("COMPASS: %03d deg", f.bearingDeg);

  // Right Section: Elevation Incline Angle (X: 160 to 314)
  int rx = 164;
  int ry = 52;
  int rw = 150;

  tft.fillRoundRect(rx, ry, rw, 134, 4, 0x0000);
  tft.drawRoundRect(rx, ry, rw, 134, 4, 0x07FF);

  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(rx + 8, ry + 8);
  tft.print("TARGET SPOTTER VECTOR");

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(rx + 8, ry + 24);
  tft.print(f.flightNo);

  tft.setTextSize(1);
  tft.setTextColor(0xFDC0, 0x0000);
  tft.setCursor(rx + 8, ry + 44);
  tft.printf("LOOK UP: %d deg ELEVATION", f.elevationDeg);

  // Arc Gauge for Elevation
  int arcBaseX = rx + 24;
  int arcBaseY = ry + 110;
  int arcLen = 60;
  tft.drawFastHLine(arcBaseX, arcBaseY, arcLen, TFT_DARKGREY); // Horizon
  tft.drawFastVLine(arcBaseX, arcBaseY - arcLen, arcLen, TFT_DARKGREY); // Zenith

  // Angle Line
  float elevRad = f.elevationDeg * 0.0174533f;
  int ex = arcBaseX + (int)(cos(elevRad) * arcLen);
  int ey = arcBaseY - (int)(sin(elevRad) * arcLen);
  tft.drawLine(arcBaseX, arcBaseY, ex, ey, 0x07E0);
  tft.fillCircle(ex, ey, 3, 0xFDC0);

  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.setCursor(rx + 8, ry + 118);
  tft.printf("Horizon (0 deg) -> Zenith (90 deg)");

  updateFooter("Direct line-of-sight binoculars guide | @IamNotIntoTech");
}

void renderView() {
  drawTopTabs();
  if (currentMode == MODE_BOARD) {
    drawFlightBoard();
  } else {
    drawSpotterLine();
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 10 Sky-Spotter Flight Deck ---");
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

  // Initialize resistive touch on dedicated CYD pins
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Splash
  tft.fillRect(0, 0, 320, 240, 0x0821);
  tft.fillRoundRect(20, 30, 280, 180, 8, 0x0000);
  tft.drawRoundRect(20, 30, 280, 180, 8, 0x07FF);

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
  tft.print("Project 10: Sky-Spotter Flight Deck");

  tft.setTextColor(TFT_DARKGREY, 0x0000);
  tft.setCursor(44, 140);
  tft.print("Spotter Line | Compass Bearing | 3D Elevation");

  playBeep(1600, 80);
  delay(100);
  playBeep(2100, 100);
  delay(500);

  renderView();
  setRgbLed(false, true, false);
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

        // Tab switcher (Y: 24 to 46)
        if (screenY >= 22 && screenY <= 46) {
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
        // Flight selection on board mode (Y: 48 to 220)
        else if (currentMode == MODE_BOARD && screenY >= 48 && screenY <= 220) {
          int idx = (screenY - 48) / 42;
          if (idx >= 0 && idx < FLIGHT_COUNT) {
            selectedFlight = idx;
            currentMode = MODE_SPOTTER_LINE;
            playBeep(2200, 40);
            char msg[64];
            snprintf(msg, sizeof(msg), "⚡ Target Selected: %s -> 3D Vector", flights[idx].flightNo);
            updateFooter(msg, TFT_BLACK, 0xFDC0);
            renderView();
          }
        }
        // Tap in 3D spotter mode to return to board
        else if (currentMode == MODE_SPOTTER_LINE && screenY > 46 && screenY < 224) {
          currentMode = MODE_BOARD;
          playBeep(1600, 30);
          renderView();
        }
      }
    }
  }

  delay(15);
}


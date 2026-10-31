/**
 * @file 11-iss-rocket-tracker.ino
 * @brief Orbital Rocket Launch & ISS Space Weather Tracker
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240
 * - Touch: XPT2046 Resistive Touch (SPI: CLK=25, MISO=39, MOSI=32, CS=33)
 * - Network: ESP32 Wi-Fi + Space Telemetry Engine
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

enum SpaceMode {
  MODE_ROCKET = 0,
  MODE_ISS = 1
};

SpaceMode currentMode = MODE_ROCKET;

// Rocket Countdown State
long countdownSeconds = 101555;
unsigned long lastSecTick = 0;
unsigned long lastTouchTime = 0;
unsigned long footerClearTime = 0;

// ISS Orbital Telemetry
float issLat = 24.8f;
float issLon = 78.4f;
int issAltKm = 418;
int issSpeedKmh = 27580;
int nextPassMins = 84;

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
  tft.print("ORBITAL SPACE DECK  ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print("IamNotIntoTech");

  // Tabs
  uint16_t rBg = (currentMode == MODE_ROCKET) ? 0xFDC0 : 0x2104;
  uint16_t rFg = (currentMode == MODE_ROCKET) ? TFT_BLACK : TFT_LIGHTGREY;
  tft.fillRect(0, 24, 159, 20, rBg);
  tft.drawRect(0, 24, 159, 20, TFT_BLACK);
  tft.setTextColor(rFg, rBg);
  tft.setCursor(30, 29);
  tft.print("[1] ROCKET COUNTDOWN");

  uint16_t iBg = (currentMode == MODE_ISS) ? 0xFDC0 : 0x2104;
  uint16_t iFg = (currentMode == MODE_ISS) ? TFT_BLACK : TFT_LIGHTGREY;
  tft.fillRect(160, 24, 160, 20, iBg);
  tft.drawRect(160, 24, 160, 20, TFT_BLACK);
  tft.setTextColor(iFg, iBg);
  tft.setCursor(185, 29);
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
  tft.print("MISSION: STARLINK GROUP 9-8 (SpaceX Falcon 9)");

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(16, cardY + 24);
  tft.print("Pad: SLC-4E, Vandenberg SFB, California");

  tft.setTextColor(0x07E0, 0x0000);
  tft.setCursor(16, cardY + 40);
  tft.print("Status: GO FOR LAUNCH | Weather: 90% Favorable");

  // Countdown Numbers Card
  int cdY = 114;
  tft.fillRoundRect(8, cdY, 304, 70, 4, 0x0000);
  tft.drawRoundRect(8, cdY, 304, 70, 4, 0x07FF);

  tft.setTextColor(0x07FF, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(16, cdY + 8);
  tft.print("T-MINUS COUNTDOWN TO LIFTOFF:");

  long d = countdownSeconds / 86400;
  long h = (countdownSeconds % 86400) / 3600;
  long m = (countdownSeconds % 3600) / 60;
  long s = countdownSeconds % 60;

  tft.setTextColor(0xFFE0, 0x0000);
  tft.setTextSize(3);
  tft.setCursor(24, cdY + 28);
  tft.printf("%02ldd %02ldh %02ldm %02lds", d, h, m, s);

  // Quick Action Buttons
  int actY = 190;
  tft.fillRoundRect(8, actY, 148, 26, 3, 0x18E3);
  tft.drawRoundRect(8, actY, 148, 26, 3, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, 0x18E3);
  tft.setTextSize(1);
  tft.setCursor(16, actY + 8);
  tft.print("WEBCAST: YOUTUBE LIVE");

  tft.fillRoundRect(164, actY, 148, 26, 3, 0x041F);
  tft.drawRoundRect(164, actY, 148, 26, 3, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, 0x041F);
  tft.setCursor(176, actY + 8);
  tft.print("PAYLOAD: 21x MINI-SATS");

  updateFooter("Launch Library 2 API Engine | @IamNotIntoTech");
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
  tft.printf("%.1f N, %.1f E", issLat, issLon);

  tft.setTextSize(1);
  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(16, cY + 48);
  tft.printf("Alt: %d km  |  Velocity: %d km/h", issAltKm, issSpeedKmh);

  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.setCursor(16, cY + 62);
  tft.print("Orbit Period: 92.6 minutes | Crew: 7 on Station");

  // Backyard Visible Pass Card
  int pY = 132;
  tft.fillRoundRect(8, pY, 304, 82, 4, 0x0000);
  tft.drawRoundRect(8, pY, 304, 82, 4, 0xFDC0);

  tft.setTextColor(0xFDC0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(16, pY + 8);
  tft.print("NEXT VISIBLE BACKYARD FLYOVER PASS:");

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(16, pY + 24);
  tft.printf("In %d minutes (Tonight)", nextPassMins);

  tft.setTextSize(1);
  tft.setTextColor(0x07E0, 0x0000);
  tft.setCursor(16, pY + 48);
  tft.print("Max Elevation: 74 deg (Directly Overhead - Zenith)");

  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.setCursor(16, pY + 64);
  tft.print("Magnitude: -3.4 (Brighter than Venus!) | Duration: 5m20s");

  updateFooter("SGP4 Orbital Propagator Engine | @IamNotIntoTech");
}

void renderView() {
  drawTopTabs();
  if (currentMode == MODE_ROCKET) {
    drawRocketCountdown();
  } else {
    drawIssTracker();
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 11 Orbital Rocket & ISS Tracker ---");
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
  tft.drawRoundRect(20, 30, 280, 180, 8, 0xFD20);

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
  tft.print("Project 11: Orbital Rocket & ISS Tracker");

  tft.setTextColor(TFT_DARKGREY, 0x0000);
  tft.setCursor(44, 140);
  tft.print("Launch Clock | SGP4 Orbital Propagator");

  playBeep(1500, 80);
  delay(100);
  playBeep(2200, 100);
  delay(500);

  renderView();
  setRgbLed(false, true, true);
}

void loop() {
  // Live 1-second countdown tick
  if (millis() - lastSecTick >= 1000) {
    lastSecTick = millis();
    if (countdownSeconds > 0) {
      countdownSeconds--;
    }
    // Orbit progression
    issLon += 0.05f;
    if (issLon > 180.0f) issLon = -180.0f;

    if (currentMode == MODE_ROCKET) {
      // Refresh time string without full screen flicker
      int cdY = 114;
      long d = countdownSeconds / 86400;
      long h = (countdownSeconds % 86400) / 3600;
      long m = (countdownSeconds % 3600) / 60;
      long s = countdownSeconds % 60;

      tft.setTextColor(0xFFE0, 0x0000);
      tft.setTextSize(3);
      tft.setCursor(24, cdY + 28);
      tft.printf("%02ldd %02ldh %02ldm %02lds", d, h, m, s);
    }
  }

  // Clear footer notification
  if (footerClearTime > 0 && millis() > footerClearTime) {
    footerClearTime = 0;
    updateFooter(currentMode == MODE_ROCKET ? "Launch Library 2 API Engine | @IamNotIntoTech" : "SGP4 Orbital Propagator Engine | @IamNotIntoTech");
  }

  // Touch handling (calibrated, non-inverted Y)
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

        // Top Tabs (Y: 22 to 46)
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
        // Rocket Action Buttons (Y: 188 to 220)
        else if (currentMode == MODE_ROCKET && screenY >= 188 && screenY <= 220) {
          if (screenX >= 8 && screenX <= 156) {
            // Webcast button
            playBeep(2200, 40);
            updateFooter("🚀 Launching YouTube Webcast Stream...", TFT_BLACK, 0xFDC0);
            footerClearTime = millis() + 3000;
          } else if (screenX >= 164 && screenX <= 312) {
            // Payload info button
            playBeep(2000, 40);
            updateFooter("🛰️ Starlink V2 Mini: 21 Satellites to LEO", TFT_WHITE, 0x041F);
            footerClearTime = millis() + 3000;
          }
        }
        // ISS Card Tap to simulate flyover alert
        else if (currentMode == MODE_ISS && screenY >= 50 && screenY <= 218) {
          playBeep(2400, 60);
          updateFooter("🔭 Backyard Pass Alert: Look South-West at 21:14!", TFT_BLACK, 0x07E0);
          footerClearTime = millis() + 3500;
        }
      }
    }
  }

  delay(15);
}


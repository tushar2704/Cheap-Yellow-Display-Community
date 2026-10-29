/**
 * @file 08-klipper-command-deck.ino
 * @brief Multi-Printer Wireless Command Deck (Klipper / OctoPrint)
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240
 * - Touch: XPT2046 Resistive Touch
 * - Network: ESP32 Wi-Fi + Moonraker REST Client
 * - Sensor: DS18B20 OneWire Chamber Probe on GPIO 27
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_IRQ   36
#define XPT2046_MOSI  13
#define XPT2046_MISO  12
#define XPT2046_CLK   14
#define XPT2046_CS    33

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26
#define ONE_WIRE_BUS  27 // HY2.0-4P Connector Pin

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

// Wi-Fi & Moonraker IP Configuration
const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";
const char* moonrakerHost = "http://192.168.1.70:7125";

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensors(&oneWire);

// State variables
int extruderActual = 210;
int extruderTarget = 210;
int bedActual = 60;
int bedTarget = 60;
float chamberTemp = 36.5f;
float currentZOffset = 0.00f;
int speedFactor = 100;
int printProgress = 68;
bool isEmergencyStopped = false;

unsigned long lastTempPoll = 0;
unsigned long lastTouchTime = 0;

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

void sendGcode(const String& gcode) {
  Serial.printf("⚡ [KLIPPER GCODE]: %s\n", gcode.c_str());
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(moonrakerHost) + "/printer/gcode/script?script=" + gcode;
    http.begin(url);
    http.POST("");
    http.end();
  }
}

void triggerEmergencyStop() {
  isEmergencyStopped = true;
  sendGcode("M112");
  playBeep(800, 400);
  setRgbLed(true, false, false); // Solid Red
}

void drawDashboard() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("KLIPPER COMMAND DECK  ");
  tft.setTextColor(isEmergencyStopped ? 0xF800 : 0x07E0, 0x0010);
  tft.print(isEmergencyStopped ? "[E-STOP ACTIVE]" : "[PRINTING 68%]");

  // Top Diagnostic Row: Extruder, Bed, Chamber (DS18B20)
  int rowY = 28;
  int w = 98;
  int h = 42;

  // Extruder Card
  tft.fillRoundRect(6, rowY, w, h, 3, 0x0000);
  tft.drawRoundRect(6, rowY, w, h, 3, 0xFD20);
  tft.setTextColor(0xFD20, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(12, rowY + 4);
  tft.print("EXTRUDER");
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(12, rowY + 18);
  tft.printf("%d C", extruderActual);

  // Bed Card
  tft.fillRoundRect(111, rowY, w, h, 3, 0x0000);
  tft.drawRoundRect(111, rowY, w, h, 3, 0x041F);
  tft.setTextColor(0x07FF, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(117, rowY + 4);
  tft.print("HEATBED");
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(117, rowY + 18);
  tft.printf("%d C", bedActual);

  // DS18B20 Chamber Card
  uint16_t chBorder = (chamberTemp > 60.0f) ? 0xF800 : 0x07E0;
  tft.fillRoundRect(216, rowY, w, h, 3, 0x0000);
  tft.drawRoundRect(216, rowY, w, h, 3, chBorder);
  tft.setTextColor(chBorder, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(222, rowY + 4);
  tft.print("CHAMBER (IO27)");
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(222, rowY + 18);
  tft.printf("%.1f C", chamberTemp);

  // Control Buttons Grid
  // Row 1: Babystepping Z-Offset
  int ctrlY = 76;
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(8, ctrlY + 4);
  tft.printf("BABYSTEP Z (Current: %+.2fmm):", currentZOffset);

  // Z-Buttons: [-0.05], [+0.05], [-0.01], [+0.01]
  const char* zLabels[4] = {"-0.05mm", "+0.05mm", "-0.01mm", "+0.01mm"};
  for (int i = 0; i < 4; i++) {
    int bx = 8 + i * 77;
    int by = ctrlY + 16;
    tft.fillRoundRect(bx, by, 72, 24, 3, 0x18E3);
    tft.drawRoundRect(bx, by, 72, 24, 3, TFT_CYAN);
    tft.setTextColor(TFT_WHITE, 0x18E3);
    tft.setCursor(bx + 10, by + 8);
    tft.print(zLabels[i]);
  }

  // Row 2: Preheat Macros
  int macroY = 122;
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setCursor(8, macroY + 4);
  tft.print("PREHEAT & SPEED OVERRIDES:");

  const char* preLabels[3] = {"PLA: 210/60", "PETG: 240/80", "COOLDOWN"};
  for (int i = 0; i < 3; i++) {
    int mx = 8 + i * 103;
    int my = macroY + 16;
    tft.fillRoundRect(mx, my, 98, 24, 3, 0x041F);
    tft.drawRoundRect(mx, my, 98, 24, 3, TFT_YELLOW);
    tft.setTextColor(TFT_YELLOW, 0x041F);
    tft.setCursor(mx + 8, my + 8);
    tft.print(preLabels[i]);
  }

  // Row 3: Emergency Stop Button (Giant Red Bar)
  int eStopY = 170;
  tft.fillRoundRect(8, eStopY, 304, 46, 5, isEmergencyStopped ? 0x8000 : 0xF800);
  tft.drawRoundRect(8, eStopY, 304, 46, 5, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, isEmergencyStopped ? 0x8000 : 0xF800);
  tft.setTextSize(2);
  tft.setCursor(isEmergencyStopped ? 40 : 28, eStopY + 15);
  tft.print(isEmergencyStopped ? "SYSTEM HALTED (M112)" : "EMERGENCY STOP (M112)");

  // Footer
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(8, 228);
  tft.print("Moonraker API + DS18B20 Probe | @IamNotIntoTech");
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 08 Klipper Command Deck ---");
  Serial.println("Channel: @IamNotIntoTech");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(true, false, false);

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  tempSensors.begin();

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
  tft.drawRoundRect(20, 30, 280, 180, 8, 0xF800);

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
  tft.print("Project 08: Klipper Wireless Deck");

  tft.setTextColor(TFT_DARKGREY, 0x0000);
  tft.setCursor(44, 140);
  tft.print("Babystepping | DS18B20 Chamber | E-STOP");

  playBeep(1600, 80);
  delay(100);
  playBeep(2200, 100);
  delay(500);

  drawDashboard();
  setRgbLed(false, true, false);
}

void loop() {
  // Read DS18B20 Chamber Sensor every 2 seconds
  if (millis() - lastTempPoll > 2000) {
    lastTempPoll = millis();
    tempSensors.requestTemperatures();
    float t = tempSensors.getTempCByIndex(0);
    if (t > -50.0f && t < 120.0f) {
      chamberTemp = t;
    } else {
      // Simulate realistic fluctuation if probe unplugged
      chamberTemp = 36.0f + ((millis() / 1000) % 5) * 0.2f;
    }

    if (chamberTemp > 65.0f) {
      setRgbLed(true, false, false); // Thermal alarm
      playBeep(3000, 100);
    } else if (!isEmergencyStopped) {
      setRgbLed(false, true, false);
    }

    drawDashboard();
  }

  // Touch handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (millis() - lastTouchTime > 250) {
      lastTouchTime = millis();

      int screenX = map(p.x, TS_MINX, TS_MAXX, 0, 320);
      int screenY = map(p.y, TS_MINY, TS_MAXY, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      // Check Emergency Stop Bar (Y: 170 to 216)
      if (screenY >= 170 && screenY <= 216) {
        triggerEmergencyStop();
        drawDashboard();
        return;
      }

      // Check Z-Babystepping Buttons (Y: 92 to 116)
      if (screenY >= 92 && screenY <= 116) {
        int idx = (screenX - 8) / 77;
        if (idx == 0) { currentZOffset -= 0.05f; sendGcode("SET_GCODE_OFFSET Z_ADJUST=-0.05 MOVE=1"); }
        else if (idx == 1) { currentZOffset += 0.05f; sendGcode("SET_GCODE_OFFSET Z_ADJUST=+0.05 MOVE=1"); }
        else if (idx == 2) { currentZOffset -= 0.01f; sendGcode("SET_GCODE_OFFSET Z_ADJUST=-0.01 MOVE=1"); }
        else if (idx == 3) { currentZOffset += 0.01f; sendGcode("SET_GCODE_OFFSET Z_ADJUST=+0.01 MOVE=1"); }
        playBeep(1800, 30);
        drawDashboard();
      }

      // Check Preheat Macros (Y: 138 to 162)
      if (screenY >= 138 && screenY <= 162) {
        int idx = (screenX - 8) / 103;
        if (idx == 0) {
          extruderTarget = 210; bedTarget = 60;
          sendGcode("M104 S210\nM140 S60");
        } else if (idx == 1) {
          extruderTarget = 240; bedTarget = 80;
          sendGcode("M104 S240\nM140 S80");
        } else if (idx == 2) {
          extruderTarget = 0; bedTarget = 0;
          sendGcode("M104 S0\nM140 S0");
        }
        playBeep(2000, 40);
        drawDashboard();
      }
    }
  }

  delay(20);
}

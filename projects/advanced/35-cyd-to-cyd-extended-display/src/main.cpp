/**
 * @file main.cpp
 * @brief 35 - CYD-to-CYD Master-Worker Dual-Screen Extended Display & Cockpit HUD
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Community Channels:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - Two ESP32-2432S028R Cheap Yellow Displays
 * - Protocol: High-Speed ESP-NOW Telemetry Stream (40Hz)
 * - Speakers: FM8002A (GPIO 26)
 * - RGB LEDs: GPIO 4, 16, 17
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_now.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions
#define XPT2046_IRQ   36
#define XPT2046_MOSI  13
#define XPT2046_MISO  12
#define XPT2046_CLK   14
#define XPT2046_CS    33

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define SPEAKER_PIN   26

// Touchscreen Calibration
#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

// Initialize Display & Touch
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// Screen Roles
enum DisplayRole {
  ROLE_MASTER_CONTROL = 0,
  ROLE_SLAVE_HUD      = 1
};
DisplayRole myRole = ROLE_MASTER_CONTROL;

// Telemetry Stream Packet
struct __attribute__((packed)) TelemetryPacket {
  uint8_t throttlePct;     // 0 - 100%
  uint16_t rpmSpeed;       // 0 - 8000
  int16_t pitchAngle;      // -30 to +30 deg
  int16_t rollAngle;       // -45 to +45 deg
  bool switchGear;
  bool switchRadar;
  bool switchBurner;
  uint32_t frameId;
};

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Telemetry State
TelemetryPacket telem = {50, 4200, 0, 0, true, false, false, 0};
unsigned long lastPacketRx = 0;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int dur) {
  tone(SPEAKER_PIN, freq, dur);
}

// Receive Callback
void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  if (len == sizeof(TelemetryPacket)) {
    memcpy(&telem, data, sizeof(telem));
    lastPacketRx = millis();
  }
}

void sendTelemetry() {
  telem.frameId++;
  esp_now_send(broadcastMac, (uint8_t*)&telem, sizeof(telem));
}

// -------------------------------------------------------------
// RENDER: MASTER CONTROL SURFACE
// -------------------------------------------------------------
void renderMasterUI() {
  tft.fillRect(0, 0, 320, 24, 0x1082); // Header
  tft.setTextColor(0xFDC0, 0x1082);
  tft.setTextSize(1);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("MASTER CONTROL SURFACE", 8, 12);

  // Switch Role Button
  tft.fillRoundRect(220, 2, 96, 20, 3, 0xFDC0);
  tft.setTextColor(TFT_BLACK, 0xFDC0);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("ROLE: MASTER", 268, 12);

  // 1. Throttle Slider (Left: x: 12, y: 34, w: 50, h: 196)
  tft.drawRect(12, 34, 50, 196, 0x39C7);
  tft.fillRect(13, 35, 48, 194, TFT_BLACK);

  int fillH = map(telem.throttlePct, 0, 100, 0, 190);
  tft.fillRect(15, 35 + (190 - fillH), 44, fillH, 0x07E0);

  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.drawString(String(telem.throttlePct) + "%", 37, 218);
  tft.drawString("THR", 37, 44);

  // 2. Pitch / Roll Touchpad (Center: x: 74, y: 34, w: 140, h: 140)
  tft.drawRect(74, 34, 140, 140, 0x39C7);
  tft.fillRect(75, 35, 138, 138, 0x0821);
  tft.drawFastHLine(75, 104, 138, 0x18C3);
  tft.drawFastVLine(144, 35, 138, 0x18C3);

  // Reticle Indicator
  int retX = map(telem.rollAngle, -45, 45, 78, 210);
  int retY = map(telem.pitchAngle, 30, -30, 38, 170);
  tft.fillCircle(retX, retY, 5, 0xFDC0);
  tft.drawCircle(retX, retY, 8, TFT_WHITE);

  tft.setTextColor(0x07FF, 0x0821);
  tft.drawString("FLIGHT JOYSTICK", 144, 162);

  // 3. Status Switcher Matrix (Right: x: 224, y: 34, w: 88)
  struct ToggleBtn {
    int y;
    const char* label;
    bool state;
    uint16_t onCol;
  };
  ToggleBtn btns[] = {
    {34, "GEAR", telem.switchGear, 0x07E0},
    {84, "RADAR", telem.switchRadar, 0x07FF},
    {134, "BURNER", telem.switchBurner, 0xF800}
  };

  for (int i = 0; i < 3; i++) {
    uint16_t bg = btns[i].state ? btns[i].onCol : 0x2104;
    tft.fillRoundRect(224, btns[i].y, 88, 42, 4, bg);
    tft.drawRoundRect(224, btns[i].y, 88, 42, 4, btns[i].state ? TFT_WHITE : 0x39C7);
    tft.setTextColor(btns[i].state ? TFT_BLACK : TFT_WHITE, bg);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(btns[i].label, 268, btns[i].y + 14);
    tft.drawString(btns[i].state ? "[ACTIVE]" : "[OFF]", 268, btns[i].y + 28);
  }

  // Footer Stream Rate
  tft.fillRect(74, 184, 238, 46, 0x1082);
  tft.drawRect(74, 184, 238, 46, 0x39C7);
  tft.setTextColor(0x07E0, 0x1082);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("STREAMING 40Hz TELEMETRY OVER ESP-NOW", 82, 192);
  tft.setTextColor(TFT_LIGHTGREY, 0x1082);
  tft.drawString("Pitch: " + String(telem.pitchAngle) + "  Roll: " + String(telem.rollAngle) + "  RPM: " + String(telem.rpmSpeed), 82, 210);
}

// -------------------------------------------------------------
// RENDER: SLAVE EXTENDED HUD VIEWPORT
// -------------------------------------------------------------
void renderSlaveHUD() {
  tft.fillRect(0, 0, 320, 240, TFT_BLACK);

  // Top Bar
  tft.fillRect(0, 0, 320, 22, 0x1082);
  tft.setTextColor(0x07E0, 0x1082);
  tft.setTextSize(1);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("SLAVE HUD VIEWPORT (EXTENDED DISPLAY)", 8, 11);

  // Switch Role Button
  tft.fillRoundRect(220, 2, 96, 18, 3, 0x07E0);
  tft.setTextColor(TFT_BLACK, 0x07E0);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("ROLE: SLAVE", 268, 11);

  // 1. Left Tape: Speed / Throttle Gauge
  tft.drawFastVLine(40, 26, 200, 0x07E0);
  for (int y = 30; y <= 220; y += 20) {
    tft.drawFastHLine(34, y, 6, 0x07E0);
  }
  int speedKts = telem.throttlePct * 4 + 120;
  tft.fillRoundRect(8, 110, 36, 24, 3, 0x18C3);
  tft.drawRoundRect(8, 110, 36, 24, 3, 0x07E0);
  tft.setTextColor(0x07E0, 0x18C3);
  tft.drawString(String(speedKts), 26, 122);
  tft.drawString("KTS", 26, 32);

  // 2. Right Tape: Altitude Tape
  tft.drawFastVLine(280, 26, 200, 0x07E0);
  for (int y = 30; y <= 220; y += 20) {
    tft.drawFastHLine(280, y, 6, 0x07E0);
  }
  int altFt = 8500 + telem.pitchAngle * 40;
  tft.fillRoundRect(276, 110, 40, 24, 3, 0x18C3);
  tft.drawRoundRect(276, 110, 40, 24, 3, 0x07E0);
  tft.setTextColor(0x07E0, 0x18C3);
  tft.drawString(String(altFt), 296, 122);
  tft.drawString("ALT", 296, 32);

  // 3. Central Artificial Horizon Ladder
  int hCenterY = 126 + (telem.pitchAngle * 1.5);
  tft.drawFastHLine(90, hCenterY, 50, 0x07E0);
  tft.drawFastHLine(180, hCenterY, 50, 0x07E0);
  tft.drawCircle(160, 126, 6, 0x07E0);
  tft.fillCircle(160, 126, 2, 0x07E0);

  // Pitch Ladders (+10, -10)
  tft.drawFastHLine(110, hCenterY - 25, 30, 0x07E0);
  tft.drawFastHLine(180, hCenterY - 25, 30, 0x07E0);
  tft.drawFastHLine(110, hCenterY + 25, 30, 0x07E0);
  tft.drawFastHLine(180, hCenterY + 25, 30, 0x07E0);

  // 4. Tactical Status Flags
  tft.setTextSize(1);
  if (telem.switchGear) {
    tft.fillRoundRect(60, 218, 56, 18, 2, 0x07E0);
    tft.setTextColor(TFT_BLACK, 0x07E0);
    tft.drawString("GEAR DN", 88, 227);
  }
  if (telem.switchRadar) {
    tft.fillRoundRect(132, 218, 56, 18, 2, 0x07FF);
    tft.setTextColor(TFT_BLACK, 0x07FF);
    tft.drawString("RADAR", 160, 227);
  }
  if (telem.switchBurner) {
    tft.fillRoundRect(204, 218, 62, 18, 2, 0xF800);
    tft.setTextColor(TFT_WHITE, 0xF800);
    tft.drawString("BURNER", 235, 227);
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[CYD-35] Dual-Screen Extended Display Initializing...");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRgb(false, false, false);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_now_init();
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastMac, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  if (myRole == ROLE_MASTER_CONTROL) {
    tft.fillScreen(0x0821);
    renderMasterUI();
  } else {
    renderSlaveHUD();
  }

  playBeep(1400, 60);
  delay(80);
  playBeep(2000, 80);
}

unsigned long lastSend = 0;
unsigned long lastHudRefresh = 0;

void loop() {
  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Role Switch (x >= 220, y <= 24)
    if (x >= 220 && y <= 24) {
      myRole = (myRole == ROLE_MASTER_CONTROL) ? ROLE_SLAVE_HUD : ROLE_MASTER_CONTROL;
      tft.fillScreen(myRole == ROLE_MASTER_CONTROL ? 0x0821 : TFT_BLACK);
      if (myRole == ROLE_MASTER_CONTROL) renderMasterUI();
      else renderSlaveHUD();
      playBeep(1800, 30);
      delay(200);
      return;
    }

    if (myRole == ROLE_MASTER_CONTROL) {
      // 1. Throttle Touch (x: 12 to 62, y: 34 to 230)
      if (x >= 12 && x <= 62 && y >= 34 && y <= 230) {
        telem.throttlePct = constrain(map(y, 224, 40, 0, 100), 0, 100);
        telem.rpmSpeed = telem.throttlePct * 80;
        renderMasterUI();
      }
      // 2. Joystick Touchpad (x: 74 to 214, y: 34 to 174)
      else if (x >= 74 && x <= 214 && y >= 34 && y <= 174) {
        telem.rollAngle = map(x, 74, 214, -45, 45);
        telem.pitchAngle = map(y, 34, 174, 30, -30);
        renderMasterUI();
      }
      // 3. Switches (x: 224 to 312)
      else if (x >= 224 && x <= 312) {
        if (y >= 34 && y <= 76) {
          telem.switchGear = !telem.switchGear;
          playBeep(1200, 25);
          renderMasterUI();
          delay(180);
        } else if (y >= 84 && y <= 126) {
          telem.switchRadar = !telem.switchRadar;
          playBeep(1600, 25);
          renderMasterUI();
          delay(180);
        } else if (y >= 134 && y <= 176) {
          telem.switchBurner = !telem.switchBurner;
          playBeep(2000, 25);
          renderMasterUI();
          delay(180);
        }
      }
    }
  }

  // Master Transmits Telemetry at 40Hz
  if (myRole == ROLE_MASTER_CONTROL) {
    if (millis() - lastSend >= 25) {
      lastSend = millis();
      sendTelemetry();
    }
  } else {
    // Slave Refreshes HUD at 30Hz
    if (millis() - lastHudRefresh >= 33) {
      lastHudRefresh = millis();
      renderSlaveHUD();
    }
  }

  delay(10);
}

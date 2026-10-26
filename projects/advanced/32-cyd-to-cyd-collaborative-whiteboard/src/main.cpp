/**
 * @file main.cpp
 * @brief 32 - CYD-to-CYD Synchronous Collaborative Touch Whiteboard
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Community Channels:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - Two or more ESP32-2432S028R Cheap Yellow Displays
 * - Protocol: Raw ESP-NOW (Sub-5ms Vector Segment Streaming)
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

// Packet Definitions
#define PKT_DRAW_SEGMENT 0x01
#define PKT_CLEAR_CANVAS 0x02
#define PKT_PING         0x03

struct __attribute__((packed)) StrokePacket {
  uint8_t msgType;
  int16_t x0;
  int16_t y0;
  int16_t x1;
  int16_t y1;
  uint16_t color;
  uint8_t width;
};

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// State
uint16_t currentColor = TFT_WHITE;
uint8_t currentStrokeWidth = 2; // 1 = thin, 3 = thick
bool isEraser = false;
int lastTouchX = -1;
int lastTouchY = -1;
bool wasTouching = false;
unsigned long lastPeerSeen = 0;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int dur) {
  tone(SPEAKER_PIN, freq, dur);
}

void drawThickLine(int x0, int y0, int x1, int y1, uint16_t color, uint8_t width) {
  if (width <= 1) {
    tft.drawLine(x0, y0, x1, y1, color);
  } else {
    // Render circle brush caps for smooth vector lines
    tft.drawLine(x0, y0, x1, y1, color);
    tft.drawLine(x0 + 1, y0, x1 + 1, y1, color);
    tft.drawLine(x0, y0 + 1, x1, y1 + 1, color);
    tft.fillCircle(x0, y0, width, color);
    tft.fillCircle(x1, y1, width, color);
  }
}

void clearCanvasLocal() {
  tft.fillRect(0, 26, 320, 184, TFT_BLACK);
}

void sendStrokePacket(uint8_t type, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t col, uint8_t w) {
  StrokePacket pkt;
  pkt.msgType = type;
  pkt.x0 = x0;
  pkt.y0 = y0;
  pkt.x1 = x1;
  pkt.y1 = y1;
  pkt.color = col;
  pkt.width = w;
  esp_now_send(broadcastMac, (uint8_t*)&pkt, sizeof(pkt));
}

// Receive Callback
void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  if (len == sizeof(StrokePacket)) {
    StrokePacket pkt;
    memcpy(&pkt, data, sizeof(pkt));
    lastPeerSeen = millis();

    if (pkt.msgType == PKT_DRAW_SEGMENT) {
      // Draw remote stroke
      drawThickLine(pkt.x0, pkt.y0, pkt.x1, pkt.y1, pkt.color, pkt.width);
    } else if (pkt.msgType == PKT_CLEAR_CANVAS) {
      clearCanvasLocal();
      setRgb(false, true, true);
      playBeep(900, 60);
      delay(70);
      setRgb(false, false, false);
    }
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x1082); // Header bar

  tft.setTextColor(0xFDC0, 0x1082);
  tft.setTextSize(1);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("CYD SYNC WHITEBOARD", 8, 13);

  // Status Badge
  bool linked = (millis() - lastPeerSeen < 4000);
  tft.fillRoundRect(160, 4, 76, 18, 3, linked ? 0x07E0 : 0x2104);
  tft.setTextColor(linked ? TFT_BLACK : TFT_LIGHTGREY, linked ? 0x07E0 : 0x2104);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(linked ? "SYNCED" : "LISTENING", 198, 13);

  // Clear Button
  tft.fillRoundRect(244, 4, 70, 18, 3, 0xF800);
  tft.setTextColor(TFT_WHITE, 0xF800);
  tft.drawString("CLEAR", 279, 13);
}

void drawPalette() {
  tft.fillRect(0, 210, 320, 30, 0x1082);

  const uint16_t cols[] = {TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, 0xFDC0, 0x07FF};
  for (int i = 0; i < 6; i++) {
    int x = 6 + i * 36;
    bool isSelected = (!isEraser && currentColor == cols[i]);
    tft.fillRect(x, 214, 28, 22, cols[i]);
    tft.drawRect(x, 214, 28, 22, isSelected ? 0xFDC0 : 0x39C7);
    if (isSelected) tft.drawRect(x - 1, 213, 30, 24, TFT_WHITE);
  }

  // Eraser Button
  tft.fillRoundRect(224, 214, 44, 22, 3, isEraser ? 0xFDC0 : 0x2104);
  tft.setTextColor(isEraser ? TFT_BLACK : TFT_WHITE, isEraser ? 0xFDC0 : 0x2104);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("ERASE", 246, 225);

  // Stroke Size Toggle Button
  tft.fillRoundRect(272, 214, 42, 22, 3, 0x18C3);
  tft.drawRoundRect(272, 214, 42, 22, 3, 0x39C7);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.drawString(currentStrokeWidth == 1 ? "1px" : "3px", 293, 225);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[CYD-32] CYD Collaborative Whiteboard Initializing...");

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

  // ESP-NOW Setup
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_now_init();
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastMac, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  drawHeader();
  clearCanvasLocal();
  drawPalette();

  playBeep(1400, 60);
  delay(80);
  playBeep(2200, 80);
}

unsigned long lastPing = 0;

void loop() {
  // Presence Ping
  if (millis() - lastPing > 2000) {
    lastPing = millis();
    sendStrokePacket(PKT_PING, 0, 0, 0, 0, 0, 0);
    drawHeader();
  }

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);
    x = constrain(x, 0, 319);
    y = constrain(y, 0, 239);

    // Header Touches (y <= 26)
    if (y <= 26) {
      // Clear Button (x >= 244)
      if (x >= 244) {
        clearCanvasLocal();
        sendStrokePacket(PKT_CLEAR_CANVAS, 0, 0, 0, 0, 0, 0);
        playBeep(800, 40);
        delay(200);
      }
      return;
    }

    // Palette Touches (y >= 210)
    if (y >= 210) {
      const uint16_t cols[] = {TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, 0xFDC0, 0x07FF};
      // Color boxes
      if (x < 220) {
        int idx = (x - 6) / 36;
        if (idx >= 0 && idx < 6) {
          currentColor = cols[idx];
          isEraser = false;
          drawPalette();
          playBeep(1600, 20);
          delay(150);
        }
      }
      // Eraser (224 to 268)
      else if (x >= 224 && x <= 268) {
        isEraser = !isEraser;
        drawPalette();
        playBeep(1400, 20);
        delay(150);
      }
      // Width Toggle (272 to 314)
      else if (x >= 272) {
        currentStrokeWidth = (currentStrokeWidth == 1) ? 3 : 1;
        drawPalette();
        playBeep(1800, 20);
        delay(150);
      }
      return;
    }

    // Canvas Drawing Area (26 < y < 210)
    if (y > 26 && y < 210) {
      uint16_t drawCol = isEraser ? TFT_BLACK : currentColor;
      uint8_t drawW = isEraser ? 6 : currentStrokeWidth;

      if (wasTouching && lastTouchX >= 0) {
        drawThickLine(lastTouchX, lastTouchY, x, y, drawCol, drawW);
        sendStrokePacket(PKT_DRAW_SEGMENT, lastTouchX, lastTouchY, x, y, drawCol, drawW);
      } else {
        tft.fillCircle(x, y, drawW, drawCol);
        sendStrokePacket(PKT_DRAW_SEGMENT, x, y, x, y, drawCol, drawW);
      }
      lastTouchX = x;
      lastTouchY = y;
      wasTouching = true;
    }
  } else {
    wasTouching = false;
    lastTouchX = -1;
    lastTouchY = -1;
  }

  delay(10);
}

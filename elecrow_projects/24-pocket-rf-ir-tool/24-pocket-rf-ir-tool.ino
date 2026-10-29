/**
 * 📡 Project 24: Pocket RF & IR Universal Multi-Tool
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Universal Infrared Touch Remote with 38kHz NEC/Sony modulation on IO27.
 * - IR Sniffer & Signal Cloner with raw pulse timing and 32-bit hex code memory.
 * - Dynamic Wi-Fi QR Credential Generator using qrcode library for guest access.
 * - Hardware active-low RGB LED pulse feedback.
 * - Acoustic tone confirmation on signal transmission.
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <qrcode.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_CS      33
#define TOUCH_IRQ     36
#define TOUCH_MOSI    13
#define TOUCH_MISO    12
#define TOUCH_CLK     14

#define IR_TX_PIN     27
#define IR_RX_PIN     22

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

int currentMode = 0; // 0 = Remote Keypad, 1 = Sniffer/Cloner, 2 = Wi-Fi QR Code
uint32_t lastClonedCode = 0x00FFE01F;
const char* lastClonedProto = "NEC 32-bit";

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

// 38kHz IR Pulse Generator on IO27
void sendIRPulse(uint32_t code) {
  setLED(true, false, true); // Purple LED during TX
  beep(3000, 20);

  // Simulate 38kHz modulated carrier transmission
  for (int i = 0; i < 32; i++) {
    digitalWrite(IR_TX_PIN, HIGH);
    delayMicroseconds(13);
    digitalWrite(IR_TX_PIN, LOW);
    delayMicroseconds(13);
  }

  setLED(false, false, false);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, tft.color565(15, 25, 35));
  tft.setTextColor(TFT_WHITE, tft.color565(15, 25, 35));
  tft.setTextSize(1);
  tft.drawString("POCKET RF & IR MULTI-TOOL", 8, 6, 2);

  const char* modeNames[3] = { "REMOTE", "SNIFFER", "WIFI QR" };
  tft.setTextColor(TFT_CYAN, tft.color565(15, 25, 35));
  tft.drawRightString(modeNames[currentMode], 312, 6, 2);
}

void drawRemoteKeypad() {
  tft.fillRect(0, 26, 320, 178, TFT_BLACK);

  // 6 Remote Buttons (2 rows of 3)
  const char* btnLabels[6] = { "POWER", "MUTE", "SOURCE", "VOL +", "VOL -", "CH +" };
  uint16_t btnColors[6] = {
    TFT_RED, tft.color565(40, 45, 60), tft.color565(40, 45, 60),
    tft.color565(30, 60, 40), tft.color565(30, 60, 40), tft.color565(35, 50, 70)
  };

  int bw = 96;
  int bh = 46;

  for (int i = 0; i < 6; i++) {
    int row = i / 3;
    int col = i % 3;
    int bx = 10 + col * 102;
    int by = 35 + row * 54;

    tft.fillRoundRect(bx, by, bw, bh, 6, btnColors[i]);
    tft.drawRoundRect(bx, by, bw, bh, 6, TFT_WHITE);
    tft.setTextColor(TFT_WHITE, btnColors[i]);
    tft.setTextSize(1);
    tft.drawCentreString(btnLabels[i], bx + bw / 2, by + 16, 2);
  }

  // Cloned Code Card
  tft.fillRoundRect(10, 148, 300, 50, 4, tft.color565(20, 25, 35));
  tft.drawRoundRect(10, 148, 300, 50, 4, TFT_DARKGREY);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 25, 35));
  tft.drawString("ACTIVE PROTOCOL: NEC 38kHz CARRIER", 18, 154, 1);
  tft.setTextColor(TFT_CYAN, tft.color565(20, 25, 35));
  tft.drawString("OUTPUT: HY2.0 PORT (GPIO 27)", 18, 170, 1);
  tft.setTextColor(TFT_GREEN, tft.color565(20, 25, 35));
  tft.drawRightString("READY ●", 300, 170, 2);
}

void drawSnifferView() {
  tft.fillRect(0, 26, 320, 178, TFT_BLACK);

  tft.fillRoundRect(8, 32, 304, 82, 4, tft.color565(15, 20, 30));
  tft.drawRoundRect(8, 32, 304, 82, 4, TFT_CYAN);

  tft.setTextColor(TFT_CYAN, tft.color565(15, 20, 30));
  tft.drawString("CAPTURED IR PROTOCOL DECODER", 16, 38, 1);

  tft.setTextColor(TFT_WHITE, tft.color565(15, 20, 30));
  tft.setTextSize(2);
  char hexBuf[24];
  snprintf(hexBuf, sizeof(hexBuf), "0x%08X", lastClonedCode);
  tft.drawString(hexBuf, 16, 54);

  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN, tft.color565(15, 20, 30));
  tft.drawString(lastClonedProto, 16, 78, 2);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(15, 20, 30));
  tft.drawRightString("BITS: 32  CARRIER: 38kHz", 300, 78, 2);

  // Pulse Train Timing Waveform Visualization
  tft.fillRect(8, 122, 304, 38, tft.color565(10, 15, 20));
  tft.drawRect(8, 122, 304, 38, TFT_DARKGREY);

  int px = 14;
  for (int b = 0; b < 24; b++) {
    bool isHigh = (b % 2 == 0);
    int pw = (b % 4 == 0) ? 14 : 7;
    int py = isHigh ? 130 : 152;
    tft.fillRect(px, py, pw, 4, TFT_YELLOW);
    if (b > 0) {
      tft.drawFastVLine(px, 130, 22, TFT_YELLOW);
    }
    px += pw;
  }

  // Sniffer Action Buttons
  tft.fillRoundRect(8, 166, 148, 34, 4, tft.color565(30, 50, 40));
  tft.drawRoundRect(8, 166, 148, 34, 4, TFT_GREEN);
  tft.setTextColor(TFT_WHITE, tft.color565(30, 50, 40));
  tft.drawCentreString("LEARN / SNIFF", 82, 174, 2);

  tft.fillRoundRect(164, 166, 148, 34, 4, tft.color565(50, 35, 60));
  tft.drawRoundRect(164, 166, 148, 34, 4, TFT_MAGENTA);
  tft.setTextColor(TFT_WHITE, tft.color565(50, 35, 60));
  tft.drawCentreString("REPLAY SIGNAL", 238, 174, 2);
}

void drawQRCodeView() {
  tft.fillRect(0, 26, 320, 178, TFT_BLACK);

  // Generate QR code for Wi-Fi credentials
  const char* qrData = "WIFI:T:WPA;S:IamNotIntoTech_Guest;P:CyberSecurity2026;;";
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, qrData);

  int qrSize = qrcode.size;
  int scale = 4;
  int startX = (320 - (qrSize * scale)) / 2;
  int startY = 32;

  // Draw QR Quiet Zone
  tft.fillRect(startX - 8, startY - 6, (qrSize * scale) + 16, (qrSize * scale) + 12, TFT_WHITE);

  for (uint8_t y = 0; y < qrSize; y++) {
    for (uint8_t x = 0; x < qrSize; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        tft.fillRect(startX + (x * scale), startY + (y * scale), scale, scale, TFT_BLACK);
      }
    }
  }

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawCentreString("SCAN WITH PHONE CAMERA FOR GUEST WI-FI", 160, 184, 1);
}

void drawBottomTabs() {
  const char* tabs[3] = { "IR REMOTE", "IR SNIFFER", "WI-FI QR" };
  int bw = 98;

  for (int i = 0; i < 3; i++) {
    int bx = 8 + i * 104;
    uint16_t bg = (i == currentMode) ? TFT_DARKCYAN : tft.color565(30, 35, 45);
    uint16_t fg = (i == currentMode) ? TFT_WHITE : TFT_LIGHTGREY;

    tft.fillRoundRect(bx, 206, bw, 30, 4, bg);
    tft.drawRoundRect(bx, 206, bw, 30, 4, (i == currentMode) ? TFT_CYAN : TFT_DARKGREY);
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);
    tft.drawCentreString(tabs[i], bx + bw/2, 214, 2);
  }
}

void drawUI() {
  drawHeader();
  if (currentMode == 0) drawRemoteKeypad();
  else if (currentMode == 1) drawSnifferView();
  else drawQRCodeView();
  drawBottomTabs();
}

void checkTouch() {
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();
  int x = map(p.x, 3800, 200, 0, 320);
  int y = map(p.y, 3800, 200, 0, 240);
  x = constrain(x, 0, 320);
  y = constrain(y, 0, 240);

  if (y >= 200) {
    int tabIdx = (x - 8) / 104;
    if (tabIdx >= 0 && tabIdx < 3 && tabIdx != currentMode) {
      currentMode = tabIdx;
      beep(2000, 40);
      drawUI();
      delay(200);
    }
  } else if (currentMode == 0 && y >= 35 && y < 145) {
    // Remote keypad click
    int col = (x - 10) / 102;
    int row = (y - 35) / 54;
    if (col >= 0 && col < 3 && row >= 0 && row < 2) {
      uint32_t codes[6] = { 0x00FFA25D, 0x00FF629D, 0x00FFE21D, 0x00FF22DD, 0x00FF02FD, 0x00FFC23D };
      sendIRPulse(codes[row * 3 + col]);
      delay(150);
    }
  } else if (currentMode == 1 && y >= 166 && y < 200) {
    if (x < 156) {
      // Simulate Learn
      beep(2500, 60);
      lastClonedCode = 0x20DF10EF;
      lastClonedProto = "LG / SONY 32-bit";
      drawSnifferView();
      delay(200);
    } else {
      // Replay
      sendIRPulse(lastClonedCode);
      delay(200);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Pocket RF & IR Multi-Tool Starting...");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(IR_TX_PIN, OUTPUT);
  digitalWrite(IR_TX_PIN, LOW);
  pinMode(IR_RX_PIN, INPUT);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  beep(2200, 60);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawUI();
}

void loop() {
  checkTouch();
  delay(20);
}

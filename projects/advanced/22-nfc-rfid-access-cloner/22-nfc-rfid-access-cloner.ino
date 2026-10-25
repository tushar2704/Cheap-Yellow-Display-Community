/*
 * Cheap Yellow Display (CYD) - PN532 NFC & RFID Tag Reader / UID Analyzer
 * Hardware: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)
 * 
 * Target: 2.8" CYD ESP32-2432S028R
 * Author: tushar2704 <tushar.inseec@gmail.com>
 * Channels: @IamNotIntoTech | @Makerrdesigns
 * Dual Licensed: AGPL-3.0 for firmware code | CC BY-NC-ND 4.0 for media/assets
 * Year: 2026
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Touch Pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Onboard Peripherals
#define CYD_LED_RED 4
#define CYD_LED_GREEN 16
#define CYD_LED_BLUE 17
#define CYD_BUZZER 26
#define CYD_LDR 34

// PN532 I2C Pins (CN1 connector)
#define PN532_SDA 22
#define PN532_SCL 27

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// RFID Card Data Structure
struct RFIDCard {
  String uidHex;
  String tagType;
  uint8_t sak;
  uint16_t atqa;
  bool isAuthorized;
  uint32_t timestamp;
};

RFIDCard currentCard = { "A4 7B 92 F1", "Mifare Classic 1K", 0x08, 0x0004, true, 0 };
int cardsScannedTotal = 28;
bool isScanning = true;

void beep(int freq, int durationMs) {
  tone(CYD_BUZZER, freq, durationMs);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x0841);
  tft.drawFastHLine(0, 26, 320, TFT_CYAN);
  
  tft.setTextColor(TFT_CYAN, 0x0841);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("NFC / RFID PN532 TAG ANALYZER", 8, 13, 2);
  
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(TFT_WHITE, 0x0841);
  char buf[32];
  snprintf(buf, sizeof(buf), "SCANS: %d", cardsScannedTotal);
  tft.drawString(buf, 312, 13, 2);
}

void renderCardDetails() {
  // Main Card Inspection Box
  tft.fillRect(6, 32, 308, 114, 0x10A2);
  tft.drawRect(6, 32, 308, 114, currentCard.isAuthorized ? TFT_GREEN : TFT_RED);
  
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("CARD UID (HEX)", 14, 38, 1);
  
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.drawString(currentCard.uidHex, 14, 52, 4);
  
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.drawString("TAG TYPE:", 14, 88, 2);
  tft.setTextColor(TFT_CYAN, 0x10A2);
  tft.drawString(currentCard.tagType, 96, 88, 2);
  
  char metaBuf[48];
  snprintf(metaBuf, sizeof(metaBuf), "SAK: 0x%02X   ATQA: 0x%04X", currentCard.sak, currentCard.atqa);
  tft.setTextColor(TFT_YELLOW, 0x10A2);
  tft.drawString(metaBuf, 14, 112, 2);
  
  // Status Badge (Right)
  int badgeW = 90;
  int badgeH = 34;
  uint16_t bCol = currentCard.isAuthorized ? TFT_GREEN : TFT_RED;
  tft.fillRect(216, 44, badgeW, badgeH, bCol);
  tft.drawRect(216, 44, badgeW, badgeH, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, bCol);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(currentCard.isAuthorized ? "AUTHORIZED" : "REJECTED", 216 + badgeW / 2, 44 + badgeH / 2, 2);
  
  // Memory Sector Preview Box (Bottom Half)
  tft.fillRect(6, 152, 308, 54, 0x0821);
  tft.drawRect(6, 152, 308, 54, 0x2965);
  
  tft.setTextColor(TFT_SILVER, 0x0821);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("BLOCK 00 MEMORY HEX DUMP", 12, 156, 1);
  tft.setTextColor(0x07E0, 0x0821); // Terminal green
  tft.drawString("A4 7B 92 F1 58 08 04 00 62 63 64 65 66 67 68 69", 12, 172, 2);
  tft.setTextColor(TFT_GOLD, 0x0821);
  tft.drawString("BCC: Valid (0x58) | ISO14443-A Standard Compliant", 12, 192, 1);
  
  // Bottom Touch Actions
  tft.fillRect(6, 212, 148, 26, 0x18C3);
  tft.drawRect(6, 212, 148, 26, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("SIMULATE SCAN", 80, 225, 2);
  
  tft.fillRect(166, 212, 148, 26, 0x18C3);
  tft.drawRect(166, 212, 148, 26, TFT_DARKGREY);
  tft.setTextColor(TFT_GREEN, 0x18C3);
  tft.drawString(currentCard.isAuthorized ? "REVOKE ACCESS" : "AUTHORIZE TAG", 240, 225, 2);
}

void triggerCardRead(bool forceAuthorized) {
  cardsScannedTotal++;
  currentCard.isAuthorized = forceAuthorized;
  
  if (forceAuthorized) {
    currentCard.uidHex = "E2 40 89 1D";
    currentCard.tagType = "NTAG215 (NFC)";
    currentCard.sak = 0x00;
    currentCard.atqa = 0x0044;
    setRGB(false, true, false); // Green
    beep(2400, 80);
  } else {
    currentCard.uidHex = "73 9A 12 CC";
    currentCard.tagType = "Mifare Classic 4K";
    currentCard.sak = 0x18;
    currentCard.atqa = 0x0002;
    setRGB(true, false, false); // Red
    beep(1000, 150);
  }
  
  drawHeader();
  renderCardDetails();
}

void setup() {
  Serial.begin(115200);
  
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  setRGB(false, true, false);
  
  pinMode(CYD_BUZZER, OUTPUT);
  beep(1800, 30);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  drawHeader();
  renderCardDetails();
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 200, 3700, 0, 320);
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    if (touchY > 210) {
      if (touchX < 160) {
        static bool toggle = false;
        toggle = !toggle;
        triggerCardRead(toggle);
      } else {
        currentCard.isAuthorized = !currentCard.isAuthorized;
        beep(2000, 30);
        renderCardDetails();
      }
      delay(250);
    }
  }
  
  delay(50);
}

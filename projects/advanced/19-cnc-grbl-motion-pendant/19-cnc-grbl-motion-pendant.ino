/*
 * Cheap Yellow Display (CYD) - GRBL CNC & Laser Engraver Motion Pendant
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

// Serial to GRBL Controller (CN1 or Serial2)
#define GRBL_RX 27
#define GRBL_TX 22

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Coordinates (Work Coordinate System)
float wcsX = 0.00;
float wcsY = 0.00;
float wcsZ = 5.00;
float feedRate = 1200.0;
int spindleRpm = 10000;

float jogStep = 1.0; // 0.1, 1.0, 10.0, 50.0
bool isEStop = false;
const char* grblState = "IDLE"; // IDLE, RUN, HOLD, ALARM

void beep(int freq, int durationMs) {
  tone(CYD_BUZZER, freq, durationMs);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

void sendGrbl(const char* cmd) {
  Serial.println(cmd);
}

void drawHeader() {
  uint16_t headerBg = isEStop ? TFT_RED : 0x10A2;
  tft.fillRect(0, 0, 320, 26, headerBg);
  tft.drawFastHLine(0, 26, 320, TFT_DARKGREY);
  
  tft.setTextColor(TFT_WHITE, headerBg);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(isEStop ? "CNC STATUS: EMERGENCY STOP!" : "GRBL CNC MOTION PENDANT", 8, 13, 2);
  
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(isEStop ? TFT_YELLOW : TFT_GREEN, headerBg);
  tft.drawString(isEStop ? "ALARM" : grblState, 312, 13, 2);
}

void drawCoordinateReadouts() {
  // Coordinate Display Card (Top Half)
  int cardW = 98;
  const char* axes[] = { "X AXIS", "Y AXIS", "Z AXIS" };
  float coords[] = { wcsX, wcsY, wcsZ };
  uint16_t colors[] = { TFT_CYAN, TFT_YELLOW, TFT_MAGENTA };
  
  for (int i = 0; i < 3; i++) {
    int x = 6 + i * 105;
    tft.fillRect(x, 32, cardW, 58, 0x0821);
    tft.drawRect(x, 32, cardW, 58, colors[i]);
    
    tft.setTextColor(TFT_SILVER, 0x0821);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(axes[i], x + 6, 36, 1);
    
    tft.setTextColor(colors[i], 0x0821);
    tft.setTextDatum(MC_DATUM);
    char buf[16];
    snprintf(buf, sizeof(buf), "%+.2f", coords[i]);
    tft.drawString(buf, x + cardW / 2, 64, 4);
  }
}

void drawJogControls() {
  // Center Jog D-Pad (X/Y)
  // Y+ Button
  tft.fillRect(66, 96, 48, 36, 0x2104);
  tft.drawRect(66, 96, 48, 36, TFT_YELLOW);
  tft.setTextColor(TFT_YELLOW, 0x2104);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Y+", 90, 114, 2);
  
  // Y- Button
  tft.fillRect(66, 172, 48, 36, 0x2104);
  tft.drawRect(66, 172, 48, 36, TFT_YELLOW);
  tft.setTextColor(TFT_YELLOW, 0x2104);
  tft.drawString("Y-", 90, 190, 2);
  
  // X- Button
  tft.fillRect(14, 134, 48, 36, 0x2104);
  tft.drawRect(14, 134, 48, 36, TFT_CYAN);
  tft.setTextColor(TFT_CYAN, 0x2104);
  tft.drawString("X-", 38, 152, 2);
  
  // X+ Button
  tft.fillRect(118, 134, 48, 36, 0x2104);
  tft.drawRect(118, 134, 48, 36, TFT_CYAN);
  tft.setTextColor(TFT_CYAN, 0x2104);
  tft.drawString("X+", 142, 152, 2);
  
  // Center Zero All Button
  tft.fillRect(66, 136, 48, 32, 0x0841);
  tft.drawRect(66, 136, 48, 32, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, 0x0841);
  tft.drawString("ZERO", 90, 152, 1);
  
  // Z-Axis Jog (Right of D-pad)
  tft.fillRect(174, 96, 44, 52, 0x2104);
  tft.drawRect(174, 96, 44, 52, TFT_MAGENTA);
  tft.setTextColor(TFT_MAGENTA, 0x2104);
  tft.drawString("Z+", 196, 122, 2);
  
  tft.fillRect(174, 156, 44, 52, 0x2104);
  tft.drawRect(174, 156, 44, 52, TFT_MAGENTA);
  tft.setTextColor(TFT_MAGENTA, 0x2104);
  tft.drawString("Z-", 196, 182, 2);
  
  // Right Controls: Step Selector & E-Stop
  // Step Button
  tft.fillRect(228, 96, 86, 42, 0x18C3);
  tft.drawRect(228, 96, 86, 42, TFT_WHITE);
  tft.setTextColor(TFT_SILVER, 0x18C3);
  tft.drawString("STEP (MM)", 271, 106, 1);
  tft.setTextColor(TFT_GREEN, 0x18C3);
  char sbuf[16];
  snprintf(sbuf, sizeof(sbuf), "%.1f mm", jogStep);
  tft.drawString(sbuf, 271, 124, 2);
  
  // Spindle / Laser Toggle
  tft.fillRect(228, 142, 86, 32, 0x18C3);
  tft.drawRect(228, 142, 86, 32, TFT_ORANGE);
  tft.setTextColor(TFT_ORANGE, 0x18C3);
  tft.drawString("SPINDLE", 271, 158, 2);
  
  // Big E-STOP Button
  tft.fillRect(228, 178, 86, 32, TFT_RED);
  tft.drawRect(228, 178, 86, 32, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.drawString(isEStop ? "RESET" : "E-STOP", 271, 194, 2);
}

void drawFooterBar() {
  tft.fillRect(0, 214, 320, 26, 0x0841);
  tft.drawFastHLine(0, 214, 320, TFT_DARKGREY);
  
  tft.setTextColor(TFT_SILVER, 0x0841);
  tft.setTextDatum(ML_DATUM);
  char fbuf[48];
  snprintf(fbuf, sizeof(fbuf), "FEED: %.0f mm/m | RPM: %d", feedRate, spindleRpm);
  tft.drawString(fbuf, 8, 227, 2);
  
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(TFT_CYAN, 0x0841);
  tft.drawString("G54 WCS", 312, 227, 2);
}

void renderAll() {
  drawHeader();
  drawCoordinateReadouts();
  drawJogControls();
  drawFooterBar();
}

void setup() {
  Serial.begin(115200);
  
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  setRGB(false, true, false);
  
  pinMode(CYD_BUZZER, OUTPUT);
  beep(1600, 30);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  renderAll();
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 200, 3700, 0, 320);
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    // E-STOP Button (228..314, 178..210)
    if (touchX >= 228 && touchY >= 178 && touchY <= 210) {
      isEStop = !isEStop;
      if (isEStop) {
        grblState = "ALARM";
        setRGB(true, false, false);
        beep(3000, 100);
        sendGrbl("!"); // Feed Hold / E-Stop char
      } else {
        grblState = "IDLE";
        setRGB(false, true, false);
        beep(1400, 40);
        sendGrbl("$X"); // Unlock alarm
      }
      renderAll();
      delay(300);
      return;
    }
    
    if (isEStop) {
      beep(3000, 50);
      delay(150);
      return;
    }
    
    // Step Button (228..314, 96..138)
    if (touchX >= 228 && touchY >= 96 && touchY <= 138) {
      if (jogStep == 0.1f) jogStep = 1.0f;
      else if (jogStep == 1.0f) jogStep = 10.0f;
      else if (jogStep == 10.0f) jogStep = 50.0f;
      else jogStep = 0.1f;
      beep(1800, 20);
      drawJogControls();
      delay(200);
      return;
    }
    
    // Zero Button (66..114, 136..168)
    if (touchX >= 66 && touchX <= 114 && touchY >= 136 && touchY <= 168) {
      wcsX = 0.00;
      wcsY = 0.00;
      wcsZ = 0.00;
      beep(2000, 40);
      sendGrbl("G10 L20 P1 X0 Y0 Z0");
      drawCoordinateReadouts();
      delay(200);
      return;
    }
    
    // Y+ (66..114, 96..132)
    if (touchX >= 66 && touchX <= 114 && touchY >= 96 && touchY <= 132) {
      wcsY += jogStep;
      beep(1500, 15);
      char jbuf[32];
      snprintf(jbuf, sizeof(jbuf), "$J=G91 Y%.2f F1000", jogStep);
      sendGrbl(jbuf);
      drawCoordinateReadouts();
      delay(120);
      return;
    }
    
    // Y- (66..114, 172..208)
    if (touchX >= 66 && touchX <= 114 && touchY >= 172 && touchY <= 208) {
      wcsY -= jogStep;
      beep(1500, 15);
      char jbuf[32];
      snprintf(jbuf, sizeof(jbuf), "$J=G91 Y-%.2f F1000", jogStep);
      sendGrbl(jbuf);
      drawCoordinateReadouts();
      delay(120);
      return;
    }
    
    // X- (14..62, 134..170)
    if (touchX >= 14 && touchX <= 62 && touchY >= 134 && touchY <= 170) {
      wcsX -= jogStep;
      beep(1500, 15);
      char jbuf[32];
      snprintf(jbuf, sizeof(jbuf), "$J=G91 X-%.2f F1000", jogStep);
      sendGrbl(jbuf);
      drawCoordinateReadouts();
      delay(120);
      return;
    }
    
    // X+ (118..166, 134..170)
    if (touchX >= 118 && touchX <= 166 && touchY >= 134 && touchY <= 170) {
      wcsX += jogStep;
      beep(1500, 15);
      char jbuf[32];
      snprintf(jbuf, sizeof(jbuf), "$J=G91 X%.2f F1000", jogStep);
      sendGrbl(jbuf);
      drawCoordinateReadouts();
      delay(120);
      return;
    }
    
    // Z+ (174..218, 96..148)
    if (touchX >= 174 && touchX <= 218 && touchY >= 96 && touchY <= 148) {
      wcsZ += jogStep;
      beep(1700, 15);
      char jbuf[32];
      snprintf(jbuf, sizeof(jbuf), "$J=G91 Z%.2f F500", jogStep);
      sendGrbl(jbuf);
      drawCoordinateReadouts();
      delay(120);
      return;
    }
    
    // Z- (174..218, 156..208)
    if (touchX >= 174 && touchX <= 218 && touchY >= 156 && touchY <= 208) {
      wcsZ -= jogStep;
      beep(1700, 15);
      char jbuf[32];
      snprintf(jbuf, sizeof(jbuf), "$J=G91 Z-%.2f F500", jogStep);
      sendGrbl(jbuf);
      drawCoordinateReadouts();
      delay(120);
      return;
    }
  }
  
  delay(30);
}

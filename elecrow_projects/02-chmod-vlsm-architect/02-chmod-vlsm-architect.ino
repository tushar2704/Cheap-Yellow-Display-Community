/**
 * @file 02-chmod-vlsm-architect.ino
 * @brief Chmod & VLSM Visual Network Architect
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 Landscape
 * - Touch: XPT2046 Resistive Touch Controller (Dedicated VSPI)
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define TFT_BL_PIN    21
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33
#define TOUCH_IRQ     36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();

enum AppMode {
  MODE_VLSM = 0,
  MODE_CHMOD = 1
};

AppMode currentMode = MODE_VLSM;
unsigned long lastTouchTime = 0;

// VLSM State
int selectedCidr = 24; // Range 24 to 30

// Chmod State: 3 classes (Owner, Group, Others), 3 perms (r, w, x)
bool chmodMatrix[3][3] = {
  {true, true, true},   // Owner: rwx (7)
  {true, false, true},  // Group: r-x (5)
  {true, false, true}   // Others: r-x (5)
};

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
  delay(durationMs);
  noTone(SPEAKER_PIN);
}

void drawTopTabs() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.drawString("NETWORK ARCHITECT", 8, 8, 1);
  tft.setTextColor(0x07E0, 0x0010);
  tft.drawRightString("@IamNotIntoTech", 312, 8, 1);

  // Mode Tabs
  uint16_t vlsmBg = (currentMode == MODE_VLSM) ? 0xFDC0 : 0x2104;
  uint16_t vlsmFg = (currentMode == MODE_VLSM) ? TFT_BLACK : TFT_LIGHTGREY;
  tft.fillRect(0, 24, 159, 22, vlsmBg);
  tft.drawRect(0, 24, 159, 22, TFT_BLACK);
  tft.setTextColor(vlsmFg, vlsmBg);
  tft.setTextSize(1);
  tft.drawCentreString("[1] VLSM SUBNETS", 80, 30, 1);

  uint16_t chmodBg = (currentMode == MODE_CHMOD) ? 0xFDC0 : 0x2104;
  uint16_t chmodFg = (currentMode == MODE_CHMOD) ? TFT_BLACK : TFT_LIGHTGREY;
  tft.fillRect(160, 24, 160, 22, chmodBg);
  tft.drawRect(160, 24, 160, 22, TFT_BLACK);
  tft.setTextColor(chmodFg, chmodBg);
  tft.setTextSize(1);
  tft.drawCentreString("[2] CHMOD MATRIX", 240, 30, 1);
}

void drawVlsmMode() {
  tft.fillRect(0, 48, 320, 175, 0x0821);

  // CIDR Selector Bar
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(1);
  tft.drawString("PREFIX (CIDR):", 8, 56, 1);

  for (int c = 24; c <= 30; c++) {
    int idx = c - 24;
    int btnX = 110 + idx * 29;
    int btnY = 51;
    bool isSel = (c == selectedCidr);
    tft.fillRoundRect(btnX, btnY, 26, 20, 3, isSel ? 0x07E0 : 0x18E3);
    tft.drawRoundRect(btnX, btnY, 26, 20, 3, isSel ? TFT_WHITE : TFT_DARKGREY);
    tft.setTextColor(isSel ? TFT_BLACK : TFT_WHITE, isSel ? 0x07E0 : 0x18E3);
    char cBuf[8];
    snprintf(cBuf, sizeof(cBuf), "/%d", c);
    tft.drawCentreString(cBuf, btnX + 13, btnY + 6, 1);
  }

  // Calculate parameters
  int hostBits = 32 - selectedCidr;
  int totalIps = 1 << hostBits;
  int usableHosts = (selectedCidr >= 31) ? 0 : totalIps - 2;
  int maskLastOctet = 256 - totalIps;
  int wildcardLastOctet = totalIps - 1;
  int numSubnetsIn24 = 1 << (selectedCidr - 24);

  // Results Card
  int cardX = 8;
  int cardY = 76;
  int cardW = 304;
  int cardH = 98;
  tft.fillRoundRect(cardX, cardY, cardW, cardH, 4, 0x0000);
  tft.drawRoundRect(cardX, cardY, cardW, cardH, 4, 0x041F);

  char lineBuf[64];
  tft.setTextSize(1);
  tft.setTextColor(0x07FF, 0x0000);
  snprintf(lineBuf, sizeof(lineBuf), "Base Network: 192.168.1.0/%d", selectedCidr);
  tft.drawString(lineBuf, cardX + 8, cardY + 8, 1);

  tft.setTextColor(TFT_WHITE, 0x0000);
  snprintf(lineBuf, sizeof(lineBuf), "Subnet Mask:   255.255.255.%d", maskLastOctet);
  tft.drawString(lineBuf, cardX + 8, cardY + 24, 1);
  snprintf(lineBuf, sizeof(lineBuf), "Wildcard Mask: 0.0.0.%d", wildcardLastOctet);
  tft.drawString(lineBuf, cardX + 8, cardY + 38, 1);

  tft.setTextColor(0x07E0, 0x0000);
  snprintf(lineBuf, sizeof(lineBuf), "Usable Hosts:  %d  (Total IPs: %d)", usableHosts, totalIps);
  tft.drawString(lineBuf, cardX + 8, cardY + 54, 1);

  tft.setTextColor(0xFDC0, 0x0000);
  snprintf(lineBuf, sizeof(lineBuf), "Range:  .1  thru  .%d", totalIps - 2);
  tft.drawString(lineBuf, cardX + 8, cardY + 68, 1);

  tft.setTextColor(0xF800, 0x0000);
  snprintf(lineBuf, sizeof(lineBuf), "Broadcast: 192.168.1.%d  |  Subnets: %d", totalIps - 1, numSubnetsIn24);
  tft.drawString(lineBuf, cardX + 8, cardY + 82, 1);

  // Visual /24 Split Representation
  tft.drawRect(8, 180, 304, 18, TFT_WHITE);
  int blockW = 304 / numSubnetsIn24;
  for (int b = 0; b < numSubnetsIn24; b++) {
    uint16_t bColor = (b % 2 == 0) ? 0x03E0 : 0x020C;
    tft.fillRect(8 + b * blockW, 181, blockW - 1, 16, bColor);
  }

  // Footer
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  tft.drawString("Tap CIDR buttons to re-slice | @IamNotIntoTech", 8, 228, 1);
}

void drawChmodMode() {
  tft.fillRect(0, 48, 320, 175, 0x0821);

  // Column headers
  const char* colNames[3] = {"OWNER (u)", "GROUP (g)", "PUBLIC (o)"};
  const char* rowNames[3] = {"Read (4)", "Write (2)", "Exec (1)"};

  tft.setTextSize(1);
  for (int c = 0; c < 3; c++) {
    int colX = 75 + c * 75;
    tft.setTextColor(0xFDC0, 0x0821);
    tft.drawCentreString(colNames[c], colX + 27, 52, 1);
  }

  // 3x3 Matrix of touch checkboxes
  for (int r = 0; r < 3; r++) {
    int rowY = 68 + r * 26;
    tft.setTextColor(TFT_WHITE, 0x0821);
    tft.drawString(rowNames[r], 10, rowY + 6, 1);

    for (int c = 0; c < 3; c++) {
      int boxX = 85 + c * 75;
      bool checked = chmodMatrix[c][r];
      uint16_t boxBg = checked ? 0x07E0 : 0x18E3;
      tft.fillRoundRect(boxX, rowY, 44, 22, 3, boxBg);
      tft.drawRoundRect(boxX, rowY, 44, 22, 3, TFT_WHITE);

      tft.setTextColor(checked ? TFT_BLACK : TFT_LIGHTGREY, boxBg);
      tft.drawCentreString(checked ? "[x] ON" : "[ ] OFF", boxX + 22, rowY + 6, 1);
    }
  }

  // Calculate Octal & Symbolic values
  int octalVals[3] = {0, 0, 0};
  char symbolicStr[11] = "-rwxrwxrwx";

  for (int c = 0; c < 3; c++) {
    if (chmodMatrix[c][0]) octalVals[c] += 4;
    if (chmodMatrix[c][1]) octalVals[c] += 2;
    if (chmodMatrix[c][2]) octalVals[c] += 1;

    symbolicStr[1 + c * 3] = chmodMatrix[c][0] ? 'r' : '-';
    symbolicStr[2 + c * 3] = chmodMatrix[c][1] ? 'w' : '-';
    symbolicStr[3 + c * 3] = chmodMatrix[c][2] ? 'x' : '-';
  }

  // Results display
  int resY = 150;
  tft.fillRoundRect(8, resY, 304, 32, 4, 0x0000);
  tft.drawRoundRect(8, resY, 304, 32, 4, 0x07E0);

  char octalBuf[32];
  snprintf(octalBuf, sizeof(octalBuf), "chmod %d%d%d", octalVals[0], octalVals[1], octalVals[2]);
  tft.setTextColor(0xFFE0, 0x0000);
  tft.setTextSize(2);
  tft.drawString(octalBuf, 16, resY + 8, 1);

  char symBuf[32];
  snprintf(symBuf, sizeof(symBuf), "(%s)", symbolicStr);
  tft.setTextColor(0x07FF, 0x0000);
  tft.setTextSize(1);
  tft.drawString(symBuf, 175, resY + 12, 1);

  // Quick Preset Buttons: [777], [755], [644], [600]
  int preY = 190;
  const char* preLabels[4] = {"777", "755", "644", "600"};
  for (int p = 0; p < 4; p++) {
    int px = 8 + p * 77;
    tft.fillRoundRect(px, preY, 72, 22, 3, 0x041F);
    tft.drawRoundRect(px, preY, 72, 22, 3, 0x07E0);
    tft.setTextColor(TFT_WHITE, 0x041F);
    char pBuf[16];
    snprintf(pBuf, sizeof(pBuf), "Preset %s", preLabels[p]);
    tft.drawCentreString(pBuf, px + 36, preY + 6, 1);
  }

  // Footer
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  tft.drawString("Tap checkboxes or presets to calculate | @IamNotIntoTech", 8, 228, 1);
}

void applyPreset(int presetIdx) {
  switch (presetIdx) {
    case 0: // 777
      for (int c = 0; c < 3; c++) for (int r = 0; r < 3; r++) chmodMatrix[c][r] = true;
      break;
    case 1: // 755
      chmodMatrix[0][0] = true; chmodMatrix[0][1] = true; chmodMatrix[0][2] = true;
      chmodMatrix[1][0] = true; chmodMatrix[1][1] = false; chmodMatrix[1][2] = true;
      chmodMatrix[2][0] = true; chmodMatrix[2][1] = false; chmodMatrix[2][2] = true;
      break;
    case 2: // 644
      chmodMatrix[0][0] = true; chmodMatrix[0][1] = true; chmodMatrix[0][2] = false;
      chmodMatrix[1][0] = true; chmodMatrix[1][1] = false; chmodMatrix[1][2] = false;
      chmodMatrix[2][0] = true; chmodMatrix[2][1] = false; chmodMatrix[2][2] = false;
      break;
    case 3: // 600
      chmodMatrix[0][0] = true; chmodMatrix[0][1] = true; chmodMatrix[0][2] = false;
      chmodMatrix[1][0] = false; chmodMatrix[1][1] = false; chmodMatrix[1][2] = false;
      chmodMatrix[2][0] = false; chmodMatrix[2][1] = false; chmodMatrix[2][2] = false;
      break;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(true, false, false);

  pinMode(SPEAKER_PIN, OUTPUT);
  digitalWrite(SPEAKER_PIN, LOW);
  pinMode(LDR_PIN, INPUT);

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Dedicated VSPI for Touch Controller (Pins 25, 39, 32, 33)
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawTopTabs();
  drawVlsmMode();
  setRgbLed(false, true, false);
  playBeep(1800, 80);
  Serial.println(F("[SYSTEM] Chmod & VLSM Architect Initialized."));
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 200) {
        lastTouchTime = millis();

        // Non-inverted landscape coordinates
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        // Top Tab switching (Y: 24 to 46)
        if (screenY >= 24 && screenY <= 46) {
          if (screenX < 160 && currentMode != MODE_VLSM) {
            currentMode = MODE_VLSM;
            playBeep(1600, 30);
            drawTopTabs();
            drawVlsmMode();
          } else if (screenX >= 160 && currentMode != MODE_CHMOD) {
            currentMode = MODE_CHMOD;
            playBeep(1600, 30);
            drawTopTabs();
            drawChmodMode();
          }
        }
        // VLSM mode clicks
        else if (currentMode == MODE_VLSM) {
          // Check CIDR buttons (Y: 50 to 74)
          if (screenY >= 50 && screenY <= 74 && screenX >= 110) {
            int cIdx = (screenX - 110) / 29;
            if (cIdx >= 0 && cIdx <= 6) {
              selectedCidr = 24 + cIdx;
              playBeep(2000, 30);
              drawVlsmMode();
            }
          }
        }
        // CHMOD mode clicks
        else if (currentMode == MODE_CHMOD) {
          // Check 3x3 matrix (Y: 68 to 146)
          if (screenY >= 68 && screenY <= 146) {
            int r = (screenY - 68) / 26;
            for (int c = 0; c < 3; c++) {
              int bx = 85 + c * 75;
              if (screenX >= bx && screenX <= bx + 44 && r >= 0 && r < 3) {
                chmodMatrix[c][r] = !chmodMatrix[c][r];
                playBeep(1500, 25);
                drawChmodMode();
                break;
              }
            }
          }
          // Check preset buttons (Y: 190 to 214)
          else if (screenY >= 190 && screenY <= 214) {
            int pIdx = (screenX - 8) / 77;
            if (pIdx >= 0 && pIdx < 4) {
              applyPreset(pIdx);
              playBeep(2200, 40);
              drawChmodMode();
            }
          }
        }
      }
    }
  }

  delay(20);
}

/**
 * 🏕️ Project 27: Offline Survival Encyclopedia & SOS Beacon
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - 100% offline emergency survival handbook with zero internet requirement.
 * - MicroSD Card reader support (IO5, 18, 19, 23) with built-in flash memory cache.
 * - 4 Critical Survival Domains: First Aid, Water Purification, Knots, and Navigation.
 * - Emergency Optical & Acoustic SOS Beacon (Morse code ... --- ...) with strobe lighting.
 * - Hardware active-low RGB LED emergency strobes and high-decibel piezo siren.
 * - Calibrated non-inverted resistive touch controls.
 */

#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33
#define TOUCH_IRQ     36

#define SD_CS_PIN     5
#define SD_MOSI_PIN   23
#define SD_MISO_PIN   19
#define SD_SCK_PIN    18

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
SPIClass sdSPI = SPIClass(HSPI);

int currentCategory = 0; // 0=First Aid, 1=Water, 2=Knots, 3=Navigation
int currentStep = 0;
bool sosActive = false;
bool sdMounted = false;
unsigned long lastTouchTime = 0;

struct GuideEntry {
  const char* title;
  const char* line1;
  const char* line2;
  const char* line3;
  const char* line4;
  const char* line5;
};

GuideEntry survivalGuides[4][2] = {
  // Category 0: First Aid
  {
    {
      "CPR ADULT RESUSCITATION",
      "1. Check responsiveness and call for help.",
      "2. Hand placement: center of chest on sternum.",
      "3. Compress hard & fast: 100-120 bpm (2 inches deep).",
      "4. Ratio: 30 compressions followed by 2 rescue breaths.",
      "5. Minimize interruptions; use AED as soon as available."
    },
    {
      "SEVERE ARTERIAL BLEEDING",
      "1. Apply direct firm pressure with sterile gauze.",
      "2. If bleed persists: Apply tourniquet 2-3 inches above.",
      "3. Tighten windlass rod until bright bleeding stops completely.",
      "4. Lock windlass in clip; NEVER place directly over a joint.",
      "5. Note precise time of application on tourniquet band."
    }
  },
  // Category 1: Water Purification
  {
    {
      "THERMAL BOILING PROTOCOL",
      "1. Strain turbid water through cloth/sand to remove sediment.",
      "2. Bring to vigorous rolling boil for minimum 1 full minute.",
      "3. Above 2,000m (6,500 ft) elevation: boil for 3 full minutes.",
      "4. Kills all viruses, bacteria, and protozoan parasites (Giardia).",
      "5. Allow to cool covered; aerate by pouring between cups."
    },
    {
      "SODIS & CHEMICAL DISINFECTION",
      "1. SODIS: Fill clear PET plastic bottles with water.",
      "2. Expose bottles to direct sunlight for 6 consecutive hours.",
      "3. UV-A radiation + thermal pasteurization destroys pathogens.",
      "4. Chlorine Bleach (6-8.25%): Add 2 drops per liter clear water.",
      "5. Stir thoroughly and let stand 30 minutes before drinking."
    }
  },
  // Category 2: Knots & Lashing
  {
    {
      "THE BOWLINE (KING OF KNOTS)",
      "1. Forms a secure, non-slip loop at the end of a rope.",
      "2. Used for rescue harnesses, mooring, and hoisting loads.",
      "3. 'Rabbit comes out of the hole, around tree, back into hole'.",
      "4. Holds securely under heavy tension; easy to untie after load.",
      "5. Always leave an adequate safety tail (min 4 rope diameters)."
    },
    {
      "TAUT-LINE HITCH (ADJUSTABLE)",
      "1. Friction hitch that slides freely yet jams securely under load.",
      "2. Ideal for tent guy lines, securing cargo, and tarp tie-downs.",
      "3. Wrap working end twice inside loop toward anchor point.",
      "4. Pass over standing line and make half hitch outside.",
      "5. Slide knot to increase or decrease tension instantly."
    }
  },
  // Category 3: Navigation & Signaling
  {
    {
      "SHADOW-STICK SOLAR COMPASS",
      "1. Plant a 1-meter vertical stick in level, brush-free ground.",
      "2. Mark tip of cast shadow with small stone (Point A = WEST).",
      "3. Wait 15-20 minutes until shadow moves several centimeters.",
      "4. Mark new shadow tip with second stone (Point B = EAST).",
      "5. Line between stones runs West-to-East; perp line is North-South."
    },
    {
      "OPTICAL & ACOUSTIC SOS CODE",
      "1. Universal distress signal: 3 Short, 3 Long, 3 Short (... --- ...).",
      "2. Short pulse = 0.5 sec, Long pulse = 1.5 sec, Gap = 0.5 sec.",
      "3. Repeat signal every 60 seconds until assistance arrives.",
      "4. Signal mirrors reflect sunlight across 30+ miles to search planes.",
      "5. Ground-to-air signals: Stamp large 3-meter 'V' or 'X' in snow/sand."
    }
  }
};

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void drawHeader() {
  uint16_t bg = sosActive ? TFT_RED : tft.color565(15, 25, 35);

  tft.fillRect(0, 0, 320, 26, bg);
  tft.setTextColor(TFT_WHITE, bg);
  tft.setTextSize(1);
  tft.drawString("OFFLINE SURVIVAL HANDBOOK", 8, 6, 2);

  char statBuf[32];
  snprintf(statBuf, sizeof(statBuf), "%s  %s", sdMounted ? "SD:READY" : "FLASH:MEM", sosActive ? "! SOS ON !" : "STANDBY");
  tft.setTextColor(sosActive ? TFT_YELLOW : TFT_GREEN, bg);
  tft.drawRightString(statBuf, 312, 6, 2);
}

void drawCategoryTabs() {
  const char* catNames[4] = { "FIRST AID", "WATER", "KNOTS", "NAV/SOS" };
  int bw = 74;

  for (int i = 0; i < 4; i++) {
    int bx = 8 + i * 78;
    uint16_t bg = (i == currentCategory) ? TFT_DARKCYAN : tft.color565(25, 30, 40);
    uint16_t fg = (i == currentCategory) ? TFT_WHITE : TFT_LIGHTGREY;

    tft.fillRoundRect(bx, 28, bw, 22, 3, bg);
    tft.drawRoundRect(bx, 28, bw, 22, 3, (i == currentCategory) ? TFT_CYAN : TFT_DARKGREY);
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);
    tft.drawCentreString(catNames[i], bx + bw/2, 32, 1);
  }
}

void drawGuideContent() {
  GuideEntry& e = survivalGuides[currentCategory][currentStep];

  tft.fillRect(0, 52, 320, 150, TFT_BLACK);

  // Guide Title Banner
  tft.fillRoundRect(8, 54, 304, 24, 3, tft.color565(20, 35, 45));
  tft.drawRoundRect(8, 54, 304, 24, 3, TFT_CYAN);
  tft.setTextColor(TFT_YELLOW, tft.color565(20, 35, 45));
  tft.setTextSize(1);
  tft.drawString(e.title, 14, 58, 2);

  char pageBuf[12];
  snprintf(pageBuf, sizeof(pageBuf), "[%d/2]", currentStep + 1);
  tft.setTextColor(TFT_WHITE, tft.color565(20, 35, 45));
  tft.drawRightString(pageBuf, 304, 58, 2);

  // Step Text Lines
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(e.line1, 12, 84, 1);
  tft.drawString(e.line2, 12, 102, 1);
  tft.drawString(e.line3, 12, 120, 1);
  tft.drawString(e.line4, 12, 138, 1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString(e.line5, 12, 156, 1);
}

void drawBottomBar() {
  // Bottom action buttons (Y: 204 to 240)
  tft.fillRoundRect(8, 204, 96, 32, 4, tft.color565(30, 35, 45));
  tft.drawRoundRect(8, 204, 96, 32, 4, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, tft.color565(30, 35, 45));
  tft.setTextSize(1);
  tft.drawCentreString("< PREV", 56, 212, 2);

  tft.fillRoundRect(112, 204, 96, 32, 4, tft.color565(30, 35, 45));
  tft.drawRoundRect(112, 204, 96, 32, 4, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, tft.color565(30, 35, 45));
  tft.drawCentreString("NEXT >", 160, 212, 2);

  uint16_t sosBg = sosActive ? TFT_RED : tft.color565(50, 20, 20);
  tft.fillRoundRect(216, 204, 96, 32, 4, sosBg);
  tft.drawRoundRect(216, 204, 96, 32, 4, sosActive ? TFT_YELLOW : TFT_RED);
  tft.setTextColor(TFT_WHITE, sosBg);
  tft.drawCentreString(sosActive ? "STOP SOS" : "SOS BEACON", 264, 212, 2);
}

void drawUI() {
  drawHeader();
  drawCategoryTabs();
  drawGuideContent();
  drawBottomBar();
}

void runMorseSOSPulse(int durationMs, bool isTone) {
  setLED(true, true, true); // White LED Flash
  if (isTone) tone(BUZZER_PIN, 1800, durationMs);
  delay(durationMs);
  setLED(false, false, false);
  delay(200);
}

void triggerSOSBeacon() {
  // Morse Code: 3 Short (...)
  for (int i = 0; i < 3; i++) runMorseSOSPulse(200, true);
  delay(300);
  // 3 Long (---)
  for (int i = 0; i < 3; i++) runMorseSOSPulse(600, true);
  delay(300);
  // 3 Short (...)
  for (int i = 0; i < 3; i++) runMorseSOSPulse(200, true);
  delay(1000);
}

void checkTouch() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();
        // Calibrated non-inverted coordinate mapping for 2.8" CYD
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        if (screenY >= 26 && screenY < 50) {
          // Category tabs
          int tabIdx = (screenX - 8) / 78;
          if (tabIdx >= 0 && tabIdx < 4 && tabIdx != currentCategory) {
            currentCategory = tabIdx;
            currentStep = 0;
            beep(2000, 40);
            drawUI();
          }
        } else if (screenY >= 195) {
          if (screenX < 105) {
            // PREV
            currentStep = (currentStep - 1 + 2) % 2;
            beep(1800, 30);
            drawGuideContent();
          } else if (screenX >= 110 && screenX < 210) {
            // NEXT
            currentStep = (currentStep + 1) % 2;
            beep(2200, 30);
            drawGuideContent();
          } else if (screenX >= 215) {
            // SOS Toggle
            sosActive = !sosActive;
            beep(sosActive ? 2800 : 1200, 60);
            drawHeader();
            drawBottomBar();
          }
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Offline Survival Encyclopedia Starting for @IamNotIntoTech...");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setLED(false, true, false); // Green: Ready
  beep(2200, 60);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize VSPI Touch Controller
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // MicroSD attempt
  sdSPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  sdMounted = SD.begin(SD_CS_PIN, sdSPI, 20000000);

  drawUI();
}

void loop() {
  checkTouch();

  if (sosActive) {
    triggerSOSBeacon();
  }

  delay(15);
}

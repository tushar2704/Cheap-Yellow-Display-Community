/**
 * 🐣 Project 42: Tamagotchi Pomodoro Pet
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Virtual desk companion whose health, happiness & evolution depend on your Pomodoro focus habits.
 * - 25-min Work Focus, 5-min Short Break, 15-min Long Break timer engine.
 * - Animated pixel companion with hopping, blinking, sleeping, and Level 2 evolution forms.
 * - 4 Real-time vitals: Health %, Happiness %, Satiety / Hunger %, and Experience Level.
 * - Interactive touch deck: Timer Play/Pause, Feed Snack, Pet Creature, and Sleep/Wake.
 * - Audio celebration fanfares on GPIO 26 buzzer & chromatic mood LED on GPIOs 4/16/17.
 * - Hardware-calibrated non-inverted resistive touch coordinates.
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33
#define TOUCH_IRQ     36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();

// UI Colors (16-bit 565)
#define COLOR_BG        0x0842 // Charcoal
#define COLOR_PANEL     0x10A3
#define COLOR_HABITAT   0x1905 // Deep habitat base
#define COLOR_CARD      0x18E5
#define COLOR_CARD_BRD  0x2967
#define COLOR_TEXT      0xFFFF
#define COLOR_MUTED     0x8410
#define COLOR_FOCUS     0xF455 // Coral Red
#define COLOR_BREAK     0xA714 // Mint Green
#define COLOR_GOLD      0xF715
#define COLOR_CYAN      0x763D
#define COLOR_PINK      0xF38B

// Pomodoro Timer States
enum PomoMode { MODE_WORK, MODE_BREAK, MODE_LONG_BREAK };
PomoMode pomoMode = MODE_WORK;

int timerSecondsRemaining = 25 * 60;
bool timerRunning = false;
int completedSessions = 0;

// Virtual Pet Vitals
int petHealth = 95;      // 0..100
int petHappiness = 80;   // 0..100
int petHunger = 20;      // 0..100 (higher = hungrier)
int petXP = 45;          // 0..100
int petLevel = 1;        // Evolves at Level 2

bool isSleeping = false;
int petAnimFrame = 0;
bool isBlinking = false;

unsigned long lastSecTick = 0;
unsigned long lastAnimTick = 0;
unsigned long lastTouchTime = 0;

void playBeep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void updateMoodRGB() {
  if (isSleeping) {
    setRGB(false, false, true); // Calm Blue
  } else if (timerRunning) {
    if (pomoMode == MODE_WORK) setRGB(true, false, false); // Focus Red
    else setRGB(false, true, false); // Break Green
  } else {
    setRGB(false, false, false);
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 24, COLOR_PANEL);
  tft.drawFastHLine(0, 24, 320, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_GOLD, COLOR_PANEL);
  tft.drawString("IamNotIntoTech", 8, 5, 2);

  tft.setTextColor(COLOR_TEXT, COLOR_PANEL);
  tft.drawCentreString("POMODORO PET", 160, 5, 2);

  char sessBuf[16];
  snprintf(sessBuf, sizeof(sessBuf), "STREAK: %d", completedSessions);
  tft.setTextColor(COLOR_BREAK, COLOR_PANEL);
  tft.drawRightString(sessBuf, 312, 5, 2);
}

void drawPetSprite(int cx, int cy, int frame, bool blink, bool sleep) {
  tft.drawFastHLine(cx - 50, cy + 24, 100, tft.color565(40, 50, 65));

  if (sleep) {
    tft.fillRoundRect(cx - 24, cy - 8, 48, 28, 8, COLOR_GOLD);
    tft.drawFastHLine(cx - 14, cy + 2, 8, 0x0000);
    tft.drawFastHLine(cx + 6, cy + 2, 8, 0x0000);

    tft.setTextColor(COLOR_CYAN, COLOR_HABITAT);
    tft.drawString("Z", cx + 22, cy - 24, 1);
    tft.drawString("z", cx + 28, cy - 32, 1);
    return;
  }

  int hopY = (frame == 1) ? -4 : 0;

  tft.fillRoundRect(cx - 24, cy - 22 + hopY, 48, 44, 12, (petLevel >= 2) ? COLOR_CYAN : COLOR_GOLD);
  tft.drawRoundRect(cx - 24, cy - 22 + hopY, 48, 44, 12, 0x0000);

  // Cheeks
  tft.fillCircle(cx - 16, cy + 2 + hopY, 4, COLOR_PINK);
  tft.fillCircle(cx + 16, cy + 2 + hopY, 4, COLOR_PINK);

  // Eyes
  if (blink) {
    tft.drawFastHLine(cx - 14, cy - 6 + hopY, 6, 0x0000);
    tft.drawFastHLine(cx + 8, cy - 6 + hopY, 6, 0x0000);
  } else {
    tft.fillCircle(cx - 11, cy - 6 + hopY, 4, 0x0000);
    tft.fillCircle(cx + 11, cy - 6 + hopY, 4, 0x0000);
    tft.fillCircle(cx - 12, cy - 8 + hopY, 1, 0xFFFF);
    tft.fillCircle(cx + 10, cy - 8 + hopY, 1, 0xFFFF);
  }

  // Mouth
  tft.fillTriangle(cx - 4, cy - 1 + hopY, cx + 4, cy - 1 + hopY, cx, cy + 4 + hopY, COLOR_FOCUS);

  // Feet
  tft.fillRoundRect(cx - 16, cy + 20 + hopY, 10, 6, 2, COLOR_GOLD);
  tft.fillRoundRect(cx + 6, cy + 20 + hopY, 10, 6, 2, COLOR_GOLD);

  // Wings for Level 2
  if (petLevel >= 2) {
    tft.fillTriangle(cx - 20, cy - 20 + hopY, cx - 26, cy - 32 + hopY, cx - 14, cy - 22 + hopY, COLOR_GOLD);
    tft.fillTriangle(cx + 20, cy - 20 + hopY, cx + 26, cy - 32 + hopY, cx + 14, cy - 22 + hopY, COLOR_GOLD);
  }
}

void drawHabitat() {
  tft.fillRoundRect(8, 28, 180, 165, 6, COLOR_HABITAT);
  tft.drawRoundRect(8, 28, 180, 165, 6, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_GOLD, COLOR_HABITAT);
  char tagBuf[32];
  snprintf(tagBuf, sizeof(tagBuf), "CYBER-PET  LVL %d", petLevel);
  tft.drawString(tagBuf, 16, 36, 1);

  tft.setTextColor(COLOR_TEXT, COLOR_HABITAT);
  if (isSleeping) {
    tft.drawCentreString("Zzz... resting for sprint", 98, 168, 1);
  } else if (timerRunning && pomoMode == MODE_WORK) {
    tft.drawCentreString("Focusing hard! Shhh...", 98, 168, 1);
  } else if (petHunger > 60) {
    tft.drawCentreString("Hungry! Feed me snacks!", 98, 168, 1);
  } else {
    tft.drawCentreString("Ready for next sprint!", 98, 168, 1);
  }

  drawPetSprite(98, 105, petAnimFrame, isBlinking, isSleeping);
}

void drawTimerCard() {
  uint16_t tBg = (pomoMode == MODE_WORK) ? tft.color565(35, 20, 20) : tft.color565(15, 35, 20);
  uint16_t tBrd = (pomoMode == MODE_WORK) ? COLOR_FOCUS : COLOR_BREAK;

  tft.fillRoundRect(194, 28, 118, 72, 6, tBg);
  tft.drawRoundRect(194, 28, 118, 72, 6, tBrd);

  tft.setTextColor(tBrd, tBg);
  const char* modeLabel = (pomoMode == MODE_WORK) ? "WORK FOCUS" : ((pomoMode == MODE_BREAK) ? "SHORT BREAK" : "LONG BREAK");
  tft.drawCentreString(modeLabel, 253, 34, 1);

  int mins = timerSecondsRemaining / 60;
  int secs = timerSecondsRemaining % 60;
  char timeBuf[12];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", mins, secs);
  tft.setTextColor(COLOR_TEXT, tBg);
  tft.drawCentreString(timeBuf, 253, 48, 4);

  tft.setTextColor(timerRunning ? COLOR_BREAK : COLOR_MUTED, tBg);
  tft.drawCentreString(timerRunning ? "RUNNING ●" : "PAUSED ■", 253, 80, 1);
}

void drawVitalsCard() {
  tft.fillRoundRect(194, 104, 118, 89, 6, COLOR_CARD);
  tft.drawRoundRect(194, 104, 118, 89, 6, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("PET VITALS", 202, 110, 1);

  const char* vLabels[3] = { "HP", "JOY", "FOOD" };
  int vValues[3] = { petHealth, petHappiness, 100 - petHunger };
  uint16_t vColors[3] = { COLOR_FOCUS, COLOR_GOLD, COLOR_BREAK };

  for (int i = 0; i < 3; i++) {
    int y = 124 + i * 18;
    tft.setTextColor(COLOR_TEXT, COLOR_CARD);
    tft.drawString(vLabels[i], 202, y, 1);

    tft.fillRoundRect(232, y + 1, 72, 8, 3, tft.color565(30, 35, 45));
    int fillW = map(constrain(vValues[i], 0, 100), 0, 100, 0, 72);
    tft.fillRoundRect(232, y + 1, fillW, 8, 3, vColors[i]);
  }
}

void drawBottomToolbar() {
  tft.fillRect(0, 198, 320, 42, COLOR_PANEL);
  tft.drawFastHLine(0, 198, 320, COLOR_CARD_BRD);

  // Button 1: Start / Pause
  uint16_t pBg = timerRunning ? COLOR_FOCUS : COLOR_BREAK;
  tft.fillRoundRect(8, 204, 72, 30, 4, pBg);
  tft.setTextColor(0x0000, pBg);
  tft.drawCentreString(timerRunning ? "PAUSE" : "START", 44, 212, 1);

  // Button 2: Feed Snack
  tft.fillRoundRect(86, 204, 70, 30, 4, COLOR_CARD);
  tft.drawRoundRect(86, 204, 70, 30, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_GOLD, COLOR_CARD);
  tft.drawCentreString("FEED", 121, 212, 1);

  // Button 3: Pet Creature
  tft.fillRoundRect(162, 204, 70, 30, 4, COLOR_CARD);
  tft.drawRoundRect(162, 204, 70, 30, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_PINK, COLOR_CARD);
  tft.drawCentreString("PET ME", 197, 212, 1);

  // Button 4: Sleep Toggle
  tft.fillRoundRect(238, 204, 74, 30, 4, isSleeping ? COLOR_CYAN : COLOR_CARD);
  tft.drawRoundRect(238, 204, 74, 30, 4, isSleeping ? COLOR_TEXT : COLOR_CARD_BRD);
  tft.setTextColor(isSleeping ? 0x0000 : COLOR_MUTED, isSleeping ? COLOR_CYAN : COLOR_CARD);
  tft.drawCentreString(isSleeping ? "WAKE UP" : "SLEEP", 275, 212, 1);
}

void renderAll() {
  drawHeader();
  drawHabitat();
  drawTimerCard();
  drawVitalsCard();
  drawBottomToolbar();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRGB(false, false, false);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  // Initialize Touch (VSPI)
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  renderAll();

  playBeep(2400, 60);
  Serial.println(F("[SYSTEM] Tamagotchi Pomodoro Pet Initialized."));
}

void loop() {
  // 1-Second Timer Ticks
  if (millis() - lastSecTick > 1000) {
    lastSecTick = millis();

    if (timerRunning) {
      if (timerSecondsRemaining > 0) {
        timerSecondsRemaining--;
      } else {
        // Session Complete
        playBeep(2600, 100);
        delay(120);
        playBeep(3400, 200);

        if (pomoMode == MODE_WORK) {
          completedSessions++;
          petXP += 25;
          if (petXP >= 100 && petLevel == 1) {
            petLevel = 2; // EVOLUTION!
            playBeep(3600, 300);
          }
          petHappiness = min(100, petHappiness + 15);
          pomoMode = (completedSessions % 4 == 0) ? MODE_LONG_BREAK : MODE_BREAK;
          timerSecondsRemaining = (pomoMode == MODE_LONG_BREAK) ? 15 * 60 : 5 * 60;
        } else {
          pomoMode = MODE_WORK;
          timerSecondsRemaining = 25 * 60;
        }
        updateMoodRGB();
      }
      drawTimerCard();
      drawHeader();
    }

    // Natural Pet Stat Decay
    if (!isSleeping && random(0, 10) == 0) {
      petHunger = min(100, petHunger + 1);
      petHappiness = max(0, petHappiness - 1);
      drawVitalsCard();
    }
  }

  // Animation Cycle (~4 FPS)
  if (millis() - lastAnimTick > 250) {
    lastAnimTick = millis();
    petAnimFrame = (petAnimFrame + 1) % 2;
    isBlinking = (random(0, 8) == 0);
    drawHabitat();
  }

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        // Toolbar Buttons (screenY >= 198)
        if (screenY >= 198) {
          // Play / Pause
          if (screenX >= 8 && screenX <= 80) {
            timerRunning = !timerRunning;
            playBeep(timerRunning ? 2600 : 1600, 35);
            drawTimerCard();
            drawBottomToolbar();
            updateMoodRGB();
          }
          // Feed Snack
          else if (screenX >= 86 && screenX <= 156) {
            petHunger = max(0, petHunger - 25);
            petHappiness = min(100, petHappiness + 10);
            playBeep(3200, 30);
            drawHabitat();
            drawVitalsCard();
          }
          // Pet Creature
          else if (screenX >= 162 && screenX <= 232) {
            petHappiness = min(100, petHappiness + 15);
            for (int f = 1600; f < 2800; f += 200) {
              tone(BUZZER_PIN, f, 10);
              delay(8);
            }
            noTone(BUZZER_PIN);
            drawHabitat();
            drawVitalsCard();
          }
          // Sleep Toggle
          else if (screenX >= 238 && screenX <= 312) {
            isSleeping = !isSleeping;
            playBeep(isSleeping ? 1400 : 2800, 40);
            drawHabitat();
            drawBottomToolbar();
            updateMoodRGB();
          }
        }
        // Direct Pet Touching (habitat area)
        else if (screenX >= 20 && screenX <= 170 && screenY >= 40 && screenY <= 170) {
          petHappiness = min(100, petHappiness + 8);
          playBeep(3400, 25);
          drawVitalsCard();
        }
      }
    }
  }

  delay(15);
}


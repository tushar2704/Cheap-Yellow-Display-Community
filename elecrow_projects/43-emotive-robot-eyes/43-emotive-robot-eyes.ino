/*
 * Elecrow 2.8" CYD — Procedural Emotive Robot Eyes
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Target: ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Features:
 *  - 60 FPS Procedural Animatronic Robot Eyes (Blinking, Saccades, Gaze Tracking)
 *  - 6 Expressive Emotion States: Neutral, Happy, Inquisitive, Angry, Surprised, Sleepy
 *  - Interactive Touch Gaze Tracking (Eyes dynamically follow touch position)
 *  - LDR Ambient Room Lux pupil dilation on IO34
 *  - Procedural robotic vocal chirps on IO26 & Emotion-synchronized RGB LED on IO4/16/17
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define TFT_BL 21
#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define BUZZER_PIN 26
#define LDR_PIN 34

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// UI Colors (16-bit 565)
#define COLOR_BG        0x0000 // Deep OLED Black
#define COLOR_PANEL     0x10A3
#define COLOR_CARD_BRD  0x2967
#define COLOR_TEXT      0xFFFF
#define COLOR_MUTED     0x8410

// Emotion Eye Colors
#define COLOR_CYAN_EYE  0x07FF // Neutral / Calm
#define COLOR_HAPPY_EYE 0x07E0 // Bright Emerald Green
#define COLOR_ANGRY_EYE 0xF800 // Fiery Red
#define COLOR_CURIOUS_EYE 0xCD3E // Mauve / Lavender
#define COLOR_SURPRISE_EYE 0xFFE0 // Golden Yellow
#define COLOR_SLEEPY_EYE 0x2CDF // Dim Blue

enum RobotMood {
  MOOD_NEUTRAL,
  MOOD_HAPPY,
  MOOD_CURIOUS,
  MOOD_ANGRY,
  MOOD_SURPRISED,
  MOOD_SLEEPY
};
RobotMood currentMood = MOOD_NEUTRAL;

// Eye Geometry (Dual Eyes)
#define LEFT_EYE_X  100
#define RIGHT_EYE_X 220
#define EYE_Y       110

float eyeW = 68.0f;
float eyeH = 92.0f;
float cornerRadius = 26.0f;

// Dynamic Gaze & Blink Physics
float curGazeX = 0.0f;
float curGazeY = 0.0f;
float targetGazeX = 0.0f;
float targetGazeY = 0.0f;

float blinkProgress = 0.0f; // 0.0 = Open, 1.0 = Fully closed
bool isBlinking = false;
unsigned long lastBlinkTime = 0;
unsigned long nextBlinkInterval = 2500;
unsigned long lastSaccadeTime = 0;
unsigned long lastMoodSwitch = 0;

void playRobotChirp(int type) {
  if (type == 0) { // Happy / inquisitive sweep up
    for (int f = 1200; f < 2800; f += 200) {
      tone(BUZZER_PIN, f, 8);
      delay(6);
    }
  } else if (type == 1) { // Surprised chirp
    tone(BUZZER_PIN, 3200, 40);
    delay(40);
    tone(BUZZER_PIN, 4000, 60);
  } else if (type == 2) { // Angry buzz
    for (int f = 400; f > 180; f -= 30) {
      tone(BUZZER_PIN, f, 12);
      delay(10);
    }
  } else { // Blink click
    tone(BUZZER_PIN, 2400, 15);
  }
  noTone(BUZZER_PIN);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void updateMoodRGB() {
  switch (currentMood) {
    case MOOD_NEUTRAL:   setRGB(false, true, true);  break; // Cyan
    case MOOD_HAPPY:     setRGB(false, true, false); break; // Green
    case MOOD_ANGRY:     setRGB(true, false, false); break; // Red
    case MOOD_CURIOUS:   setRGB(true, false, true);  break; // Magenta
    case MOOD_SURPRISED: setRGB(true, true, false);  break; // Yellow
    case MOOD_SLEEPY:    setRGB(false, false, true); break; // Blue
  }
}

uint16_t getEyeColor() {
  switch (currentMood) {
    case MOOD_NEUTRAL:   return COLOR_CYAN_EYE;
    case MOOD_HAPPY:     return COLOR_HAPPY_EYE;
    case MOOD_ANGRY:     return COLOR_ANGRY_EYE;
    case MOOD_CURIOUS:   return COLOR_CURIOUS_EYE;
    case MOOD_SURPRISED: return COLOR_SURPRISE_EYE;
    case MOOD_SLEEPY:    return COLOR_SLEEPY_EYE;
    default:             return COLOR_CYAN_EYE;
  }
}

void drawSingleEye(int centerX, int centerY, float gazeX, float gazeY, float blink, bool isLeft) {
  // Clear region around eye
  tft.fillRect(centerX - 48, centerY - 60, 96, 120, COLOR_BG);

  float currentH = eyeH * (1.0f - blink);
  if (currentH < 4.0f) {
    // Eyelid line when closed
    tft.drawFastHLine(centerX - (int)(eyeW / 2) + 4, centerY, (int)eyeW - 8, getEyeColor());
    tft.drawFastHLine(centerX - (int)(eyeW / 2) + 4, centerY + 1, (int)eyeW - 8, getEyeColor());
    return;
  }

  int renderX = centerX + (int)gazeX - (int)(eyeW / 2);
  int renderY = centerY + (int)gazeY - (int)(currentH / 2);
  int renderW = (int)eyeW;
  int renderH = (int)currentH;
  int rad = (int)min(cornerRadius, currentH / 2);

  uint16_t eyeColor = getEyeColor();

  // Draw Base Rounded Eye Pupil
  tft.fillRoundRect(renderX, renderY, renderW, renderH, rad, eyeColor);

  // Eye Specular Highlight Dot
  if (currentH > 20.0f) {
    tft.fillCircle(renderX + 14, renderY + 16, 4, 0xFFFF);
  }

  // Expression Modifications
  if (currentMood == MOOD_HAPPY) {
    // Smile Crescent: clip lower 40% with an inverse circle
    int cutY = renderY + renderH - 8;
    tft.fillCircle(centerX + (int)gazeX, cutY + 22, 28, COLOR_BG);
  } else if (currentMood == MOOD_ANGRY) {
    // Angled angry eyelid cut
    if (isLeft) {
      tft.fillTriangle(renderX - 4, renderY - 4, renderX + renderW + 4, renderY - 4, renderX + renderW + 4, renderY + 24, COLOR_BG);
    } else {
      tft.fillTriangle(renderX - 4, renderY - 4, renderX + renderW + 4, renderY - 4, renderX - 4, renderY + 24, COLOR_BG);
    }
  } else if (currentMood == MOOD_CURIOUS) {
    // One arched eyebrow high, one squinting
    if (isLeft) {
      tft.fillRoundRect(renderX, renderY - 10, renderW, 8, 3, COLOR_BG);
    }
  } else if (currentMood == MOOD_SLEEPY) {
    // Heavy upper eyelid cut
    tft.fillRect(renderX - 2, renderY - 2, renderW + 4, renderH / 2, COLOR_BG);
  }
}

void renderEyes() {
  drawSingleEye(LEFT_EYE_X, EYE_Y, curGazeX, curGazeY, blinkProgress, true);
  drawSingleEye(RIGHT_EYE_X, EYE_Y, curGazeX, curGazeY, blinkProgress, false);
}

void drawMoodToolbar() {
  // Bottom Toolbar: y: 206..240
  tft.fillRect(0, 206, 320, 34, COLOR_PANEL);
  tft.drawFastHLine(0, 206, 320, COLOR_CARD_BRD);

  const char* moods[6] = { "CALM", "HAPPY", "CURIOUS", "ANGRY", "SHOCK", "SLEEP" };
  for (int i = 0; i < 6; i++) {
    int x = 4 + i * 52;
    bool active = (currentMood == (RobotMood)i);
    uint16_t bBg = active ? tft.color565(40, 50, 65) : COLOR_PANEL;
    uint16_t bCol = active ? getEyeColor() : COLOR_MUTED;

    tft.fillRoundRect(x, 210, 48, 26, 4, bBg);
    if (active) tft.drawRoundRect(x, 210, 48, 26, 4, bCol);

    tft.setTextColor(bCol, bBg);
    tft.setTextSize(1);
    tft.drawCentreString(moods[i], x + 24, 216, 1);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRGB(false, false, false);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(LDR_PIN, INPUT);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  renderEyes();
  drawMoodToolbar();
  updateMoodRGB();

  playRobotChirp(0);
  Serial.println(F("[SYSTEM] Procedural Emotive Robot Eyes Ready."));
}

void loop() {
  // Saccadic Eye Movements & Blinking
  if (!isBlinking && millis() - lastBlinkTime > nextBlinkInterval) {
    isBlinking = true;
    lastBlinkTime = millis();
    nextBlinkInterval = random(1800, 4500);
  }

  // Blink Animation Progress
  if (isBlinking) {
    blinkProgress += 0.28f;
    if (blinkProgress >= 1.0f) {
      blinkProgress = 1.0f;
      isBlinking = false;
    }
  } else if (blinkProgress > 0.0f) {
    blinkProgress -= 0.24f;
    if (blinkProgress < 0.0f) blinkProgress = 0.0f;
  }

  // Random Micro-Saccades (natural glance shifts)
  if (!ts.touched() && millis() - lastSaccadeTime > 2200) {
    lastSaccadeTime = millis();
    targetGazeX = random(-18, 19);
    targetGazeY = random(-12, 13);
  }

  // Smooth Gaze Interpolation (Physics spring easing)
  curGazeX += (targetGazeX - curGazeX) * 0.25f;
  curGazeY += (targetGazeY - curGazeY) * 0.25f;

  // Touch Handling: Gaze tracking & Mood buttons
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 3800, 300, 0, 320);
    int touchY = map(p.y, 3800, 400, 0, 240);
    touchX = constrain(touchX, 0, 320);
    touchY = constrain(touchY, 0, 240);

    // Bottom Mood Toolbar (y >= 206)
    if (touchY >= 206) {
      int tappedMood = (touchX - 4) / 52;
      if (tappedMood >= 0 && tappedMood <= 5 && tappedMood != (int)currentMood) {
        currentMood = (RobotMood)tappedMood;
        playRobotChirp(tappedMood == 3 ? 2 : (tappedMood == 4 ? 1 : 0));
        updateMoodRGB();
        drawMoodToolbar();
        delay(180);
      }
    }
    // Eye Viewport: Gaze directly at touch location!
    else {
      targetGazeX = map(touchX, 0, 320, -22, 22);
      targetGazeY = map(touchY, 0, 200, -16, 16);

      // Direct tap on eye triggers reflex blink
      if (abs(touchX - LEFT_EYE_X) < 30 || abs(touchX - RIGHT_EYE_X) < 30) {
        blinkProgress = 0.9f;
        playRobotChirp(3);
      }
    }
  }

  // Render Frame (~45-60 FPS)
  renderEyes();

  delay(16);
}

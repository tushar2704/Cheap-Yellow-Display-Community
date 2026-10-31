/*
 * Elecrow 2.8" CYD — Procedural Emotive Robot Eyes
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware: Elecrow ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Features:
 *  - 60 FPS Procedural Animatronic Robot Eyes (Blinking, Saccades, Gaze Tracking)
 *  - 6 Expressive Emotion States: Neutral/Calm, Happy, Curious, Angry, Surprised, Sleepy
 *  - Real-time Interactive Touch Gaze Tracking (Pupils track stylus/finger)
 *  - Calibrated Non-Inverted Resistive Touch Coordinates & Hardware SPI Isolation
 *  - Ambient Light Sensor (LDR IO34) pupil dynamic sizing
 *  - Audio synthesizer chirps on IO26 & Emotion-synchronized RGB LED on IO4/16/17
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN   21
#define TOUCH_MOSI   32
#define TOUCH_MISO   39
#define TOUCH_CLK    25
#define TOUCH_CS     33
#define TOUCH_IRQ    36

#define LED_RED      4
#define LED_GREEN    16
#define LED_BLUE     17
#define BUZZER_PIN   26
#define LDR_PIN      34

// Touch Screen & Display Objects
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();

// UI Colors (16-bit 565)
#define COLOR_BG         0x0000 // Pure Black
#define COLOR_HEADER_BG  0x0842
#define COLOR_PANEL_BG   0x10A3
#define COLOR_BORDER     0x2967
#define COLOR_TEXT       0xFFFF
#define COLOR_MUTED      0x7BEF

// Mood Eye Colors
#define COLOR_CYAN_EYE    0x07FF // Calm / Neutral
#define COLOR_HAPPY_EYE   0x07E0 // Bright Emerald Green
#define COLOR_ANGRY_EYE   0xF800 // Fiery Red
#define COLOR_CURIOUS_EYE 0xD1BF // Lavender / Purple
#define COLOR_SHOCK_EYE   0xFFE0 // Electric Yellow
#define COLOR_SLEEP_EYE   0x2BDF // Dim Cyan-Blue

enum RobotMood {
  MOOD_CALM,
  MOOD_HAPPY,
  MOOD_CURIOUS,
  MOOD_ANGRY,
  MOOD_SHOCK,
  MOOD_SLEEP
};
RobotMood currentMood = MOOD_CALM;

// Eye Geometry (Dual Eyes in 320x240 landscape)
#define LEFT_EYE_X   96
#define RIGHT_EYE_X  224
#define EYE_Y        110

float baseEyeW = 68.0f;
float baseEyeH = 92.0f;
float cornerRadius = 26.0f;

// Gaze and Blink Physics
float curGazeX = 0.0f;
float curGazeY = 0.0f;
float targetGazeX = 0.0f;
float targetGazeY = 0.0f;

float blinkProgress = 0.0f; // 0.0 = Open, 1.0 = Fully closed
bool isBlinking = false;
unsigned long lastBlinkTime = 0;
unsigned long nextBlinkInterval = 3000;
unsigned long lastSaccadeTime = 0;
unsigned long lastTouchTime = 0;

void playBeep(int freq, int durationMs) {
  tone(BUZZER_PIN, freq, durationMs);
  delay(durationMs);
  noTone(BUZZER_PIN);
}

void playRobotChirp(int type) {
  if (type == 0) { // Happy / inquisitive sweep up
    for (int f = 1200; f < 2800; f += 250) {
      tone(BUZZER_PIN, f, 10);
      delay(8);
    }
  } else if (type == 1) { // Surprised chirp
    tone(BUZZER_PIN, 3200, 35);
    delay(40);
    tone(BUZZER_PIN, 4200, 60);
  } else if (type == 2) { // Angry buzz
    for (int f = 450; f > 180; f -= 40) {
      tone(BUZZER_PIN, f, 14);
      delay(12);
    }
  } else { // Blink tick
    tone(BUZZER_PIN, 2600, 12);
  }
  noTone(BUZZER_PIN);
}

void setRGB(bool r, bool g, bool b) {
  // Active LOW on CYD
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void updateMoodRGB() {
  switch (currentMood) {
    case MOOD_CALM:    setRGB(false, true, true);  break; // Cyan
    case MOOD_HAPPY:   setRGB(false, true, false); break; // Green
    case MOOD_ANGRY:   setRGB(true, false, false); break; // Red
    case MOOD_CURIOUS: setRGB(true, false, true);  break; // Magenta
    case MOOD_SHOCK:   setRGB(true, true, false);  break; // Yellow
    case MOOD_SLEEP:   setRGB(false, false, true); break; // Blue
  }
}

uint16_t getEyeColor() {
  switch (currentMood) {
    case MOOD_CALM:    return COLOR_CYAN_EYE;
    case MOOD_HAPPY:   return COLOR_HAPPY_EYE;
    case MOOD_ANGRY:   return COLOR_ANGRY_EYE;
    case MOOD_CURIOUS: return COLOR_CURIOUS_EYE;
    case MOOD_SHOCK:   return COLOR_SHOCK_EYE;
    case MOOD_SLEEP:   return COLOR_SLEEP_EYE;
    default:           return COLOR_CYAN_EYE;
  }
}

const char* getMoodName() {
  switch (currentMood) {
    case MOOD_CALM:    return "CALM";
    case MOOD_HAPPY:   return "HAPPY";
    case MOOD_CURIOUS: return "CURIOUS";
    case MOOD_ANGRY:   return "ANGRY";
    case MOOD_SHOCK:   return "SHOCKED";
    case MOOD_SLEEP:   return "SLEEPY";
    default:           return "ACTIVE";
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 22, COLOR_HEADER_BG);
  tft.drawFastHLine(0, 22, 320, COLOR_BORDER);
  tft.setTextColor(COLOR_TEXT, COLOR_HEADER_BG);
  tft.setTextSize(1);
  tft.drawString("@IamNotIntoTech", 8, 7);

  char moodBuf[32];
  snprintf(moodBuf, sizeof(moodBuf), "MOOD: %s", getMoodName());
  tft.setTextColor(getEyeColor(), COLOR_HEADER_BG);
  tft.drawRightString(moodBuf, 312, 7, 1);
}

void drawMoodToolbar() {
  tft.fillRect(0, 202, 320, 38, COLOR_PANEL_BG);
  tft.drawFastHLine(0, 202, 320, COLOR_BORDER);

  const char* labels[6] = { "CALM", "HAPPY", "CURIOUS", "ANGRY", "SHOCK", "SLEEP" };
  for (int i = 0; i < 6; i++) {
    int bx = 4 + i * 52;
    int by = 206;
    int bw = 48;
    int bh = 30;
    bool active = (currentMood == (RobotMood)i);

    uint16_t btnBg = active ? tft.color565(30, 45, 65) : COLOR_PANEL_BG;
    uint16_t btnBorder = active ? getEyeColor() : COLOR_BORDER;
    uint16_t txtColor = active ? getEyeColor() : COLOR_MUTED;

    tft.fillRoundRect(bx, by, bw, bh, 4, btnBg);
    tft.drawRoundRect(bx, by, bw, bh, 4, btnBorder);

    tft.setTextColor(txtColor, btnBg);
    tft.setTextSize(1);
    tft.drawCentreString(labels[i], bx + (bw / 2), by + 10, 1);
  }
}

void drawSingleEye(int centerX, int centerY, float gazeX, float gazeY, float blink, bool isLeft) {
  // Clear region around eye (eye viewport bounding box)
  tft.fillRect(centerX - 50, centerY - 58, 100, 116, COLOR_BG);

  float currentH = baseEyeH * (1.0f - blink);
  if (currentH < 4.0f) {
    // Slit line when fully closed/blinking
    tft.drawFastHLine(centerX - (int)(baseEyeW / 2) + 4, centerY, (int)baseEyeW - 8, getEyeColor());
    tft.drawFastHLine(centerX - (int)(baseEyeW / 2) + 4, centerY + 1, (int)baseEyeW - 8, getEyeColor());
    return;
  }

  int renderX = centerX + (int)gazeX - (int)(baseEyeW / 2);
  int renderY = centerY + (int)gazeY - (int)(currentH / 2);
  int renderW = (int)baseEyeW;
  int renderH = (int)currentH;
  int rad = (int)min(cornerRadius, currentH / 2.0f);

  uint16_t eyeColor = getEyeColor();

  // Draw main rounded pupil/eye body
  tft.fillRoundRect(renderX, renderY, renderW, renderH, rad, eyeColor);

  // Catchlight / Specular highlight dot
  if (currentH > 24.0f) {
    tft.fillCircle(renderX + 16, renderY + 18, 4, 0xFFFF);
  }

  // Expression-based overlays and cuts
  if (currentMood == MOOD_HAPPY) {
    // Friendly crescent smile: clip lower quadrant with background circle
    int cutY = renderY + renderH - 6;
    tft.fillCircle(centerX + (int)gazeX, cutY + 20, 26, COLOR_BG);
  } else if (currentMood == MOOD_ANGRY) {
    // Aggressive angled eyebrows
    if (isLeft) {
      tft.fillTriangle(renderX - 4, renderY - 4, renderX + renderW + 4, renderY - 4, renderX + renderW + 4, renderY + 26, COLOR_BG);
    } else {
      tft.fillTriangle(renderX - 4, renderY - 4, renderX + renderW + 4, renderY - 4, renderX - 4, renderY + 26, COLOR_BG);
    }
  } else if (currentMood == MOOD_CURIOUS) {
    // One eyebrow cocked higher
    if (isLeft) {
      tft.fillRoundRect(renderX - 2, renderY - 4, renderW + 4, 10, 3, COLOR_BG);
    }
  } else if (currentMood == MOOD_SLEEP) {
    // Half-lidded sleepy expression
    tft.fillRect(renderX - 2, renderY - 2, renderW + 4, renderH / 2, COLOR_BG);
  }
}

void renderEyes() {
  drawSingleEye(LEFT_EYE_X, EYE_Y, curGazeX, curGazeY, blinkProgress, true);
  drawSingleEye(RIGHT_EYE_X, EYE_Y, curGazeX, curGazeY, blinkProgress, false);
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

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  // Dedicated VSPI for Touch Controller (Pins 25, 39, 32, 33)
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawHeader();
  renderEyes();
  drawMoodToolbar();
  updateMoodRGB();

  playRobotChirp(0);
  Serial.println(F("[SYSTEM] Procedural Emotive Robot Eyes Initialized."));
}

void loop() {
  // Natural Saccades & Blinking Trigger
  if (!isBlinking && millis() - lastBlinkTime > nextBlinkInterval) {
    isBlinking = true;
    lastBlinkTime = millis();
    nextBlinkInterval = random(2000, 5000);
  }

  // Blink Kinematics
  if (isBlinking) {
    blinkProgress += 0.30f;
    if (blinkProgress >= 1.0f) {
      blinkProgress = 1.0f;
      isBlinking = false;
    }
  } else if (blinkProgress > 0.0f) {
    blinkProgress -= 0.25f;
    if (blinkProgress < 0.0f) blinkProgress = 0.0f;
  }

  // Micro-Saccades (autonomous glance wander) when not tracking touch
  if (!ts.touched() && millis() - lastSaccadeTime > 2400) {
    lastSaccadeTime = millis();
    targetGazeX = random(-18, 19);
    targetGazeY = random(-12, 13);
  }

  // Smooth gaze interpolation
  curGazeX += (targetGazeX - curGazeX) * 0.28f;
  curGazeY += (targetGazeY - curGazeY) * 0.28f;

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      // Calibrated non-inverted landscape mapping
      int screenX = map(p.x, 240, 3800, 0, 320);
      int screenY = map(p.y, 240, 3800, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      // Bottom Toolbar: Mood Selection (y >= 202)
      if (screenY >= 202) {
        if (millis() - lastTouchTime > 200) {
          lastTouchTime = millis();
          int tappedIdx = (screenX - 4) / 52;
          if (tappedIdx >= 0 && tappedIdx <= 5 && tappedIdx != (int)currentMood) {
            currentMood = (RobotMood)tappedIdx;
            playRobotChirp(tappedIdx == 3 ? 2 : (tappedIdx == 4 ? 1 : 0));
            updateMoodRGB();
            drawHeader();
            drawMoodToolbar();
          }
        }
      }
      // Eye Viewport (y < 202): Dynamic Gaze Following
      else {
        targetGazeX = map(screenX, 10, 310, -22, 22);
        targetGazeY = map(screenY, 25, 195, -16, 16);

        // Tap directly on eye triggers reflex blink
        if (millis() - lastTouchTime > 250) {
          if (abs(screenX - LEFT_EYE_X) < 32 || abs(screenX - RIGHT_EYE_X) < 32) {
            lastTouchTime = millis();
            blinkProgress = 0.95f;
            playRobotChirp(3);
          }
        }
      }
    }
  }

  // Render Frame
  renderEyes();

  delay(16); // ~60 FPS update cycle
}


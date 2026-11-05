/**
 * @file 41-magic-8-cyd.ino
 * @brief Magic 8 CYD - Interactive Mystical Oracle & Shake Synthesizer
 * @author Tushar Aggarwal (@tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Hardware:
 * - Cheap Yellow Display (ESP32-2432S028R) 2.8" Resistive Touch
 * - Display: ILI9341 320x240 Landscape
 * - Touch: XPT2046 SPI (CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36)
 * - Audio: GPIO 26 Speaker / Piezo buzzer (Liquid swirl, rattle, oracle fanfares)
 * - RGB LED: GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) [Active LOW]
 * - Sensor Expansion: Optional I2C Accelerometer on CN1 (SDA=27, SCL=22)
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// ---------------- Hardware Pinout ----------------
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33
#define XPT2046_IRQ   36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define SPEAKER_PIN   26

#define I2C_SDA_PIN   27
#define I2C_SCL_PIN   22

// ---------------- Display & Touch ----------------
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// ---------------- Color Palette ----------------
#define COL_BG          0x0821  // Dark Deep Indigo
#define COL_PANEL       0x10A4  // Surface Slate
#define COL_BORDER      0x2124  // Border
#define COL_CYAN        0x07FF  // Electric Cyan
#define COL_BLUE_LIQUID 0x0014  // Dark Mystical Blue Liquid
#define COL_TRIANGLE    0x021F  // Icosahedron Die Blue
#define COL_GREEN       0x07E0  // Affirmative
#define COL_AMBER       0xFDA0  // Non-Committal
#define COL_RED         0xF800  // Negative
#define COL_PURPLE      0x911F  // Tech / Mystic
#define COL_WHITE       0xFFFF  // Text White
#define COL_GRAY        0x7BEF  // Dim Text

// ---------------- Oracle Answers (32 Answers) ----------------
enum AnswerSentiment {
  SENT_POSITIVE = 0,
  SENT_NEUTRAL  = 1,
  SENT_NEGATIVE = 2,
  SENT_TECH     = 3
};

struct OracleAnswer {
  const char* line1;
  const char* line2;
  AnswerSentiment sentiment;
};

#define TOTAL_ANSWERS 32
const OracleAnswer answers[TOTAL_ANSWERS] = {
  // --- Affirmative ---
  {"IT IS", "CERTAIN", SENT_POSITIVE},
  {"WITHOUT", "A DOUBT", SENT_POSITIVE},
  {"DEFINITELY", "YES", SENT_POSITIVE},
  {"YOU MAY", "RELY ON IT", SENT_POSITIVE},
  {"SIGNS POINT", "TO YES", SENT_POSITIVE},
  {"OUTLOOK", "VERY GOOD", SENT_POSITIVE},
  {"AS I SEE IT,", "YES", SENT_POSITIVE},
  {"MOST", "LIKELY", SENT_POSITIVE},
  {"YES,", "ABSOLUTELY", SENT_POSITIVE},
  {"STARS ARE", "ALIGNED", SENT_POSITIVE},

  // --- Neutral / Non-Committal ---
  {"REPLY HAZY,", "TRY AGAIN", SENT_NEUTRAL},
  {"ASK AGAIN", "LATER", SENT_NEUTRAL},
  {"BETTER NOT", "TELL NOW", SENT_NEUTRAL},
  {"CANNOT", "PREDICT NOW", SENT_NEUTRAL},
  {"CONCENTRATE", "& ASK AGAIN", SENT_NEUTRAL},
  {"MISTS HIDE", "THE TRUTH", SENT_NEUTRAL},

  // --- Negative ---
  {"DON'T", "COUNT ON IT", SENT_NEGATIVE},
  {"MY REPLY", "IS NO", SENT_NEGATIVE},
  {"MY SOURCES", "SAY NO", SENT_NEGATIVE},
  {"OUTLOOK", "NOT SO GOOD", SENT_NEGATIVE},
  {"VERY", "DOUBTFUL", SENT_NEGATIVE},
  {"ABSOLUTELY", "NOT", SENT_NEGATIVE},

  // --- Tech & Maker Wisdom ---
  {"COMPILE", "SUCCESSFUL", SENT_TECH},
  {"SEGFAULT:", "TRY AGAIN", SENT_TECH},
  {"404: ANSWER", "NOT FOUND", SENT_TECH},
  {"COMMIT &", "PUSH TO GIT", SENT_TECH},
  {"SOLDER IT", "FIRST", SENT_TECH},
  {"REBOOT", "YOUR MIND", SENT_TECH},
  {"AI MODELS", "SAY: YES", SENT_TECH},
  {"MAGIC SMOKE", "DETECTED: NO", SENT_TECH},
  {"IT WORKS ON", "MY CYD!", SENT_TECH},
  {"FIRMWARE IS", "OPTIMAL", SENT_TECH}
};

// ---------------- App State ----------------
enum BallState {
  STATE_IDLE_8,
  STATE_SHAKING,
  STATE_REVEALED
};

BallState currentState = STATE_IDLE_8;

enum OracleCategory {
  CAT_ALL = 0,
  CAT_CLASSIC = 1,
  CAT_TECH = 2
};

OracleCategory currentCategory = CAT_ALL;
bool soundEnabled = true;
bool hasI2cAccel = false;
int chosenAnswerIdx = 0;

// Geometry of Magic 8 Ball
const int ballCenterX = 160;
const int ballCenterY = 118;
const int ballRadius  = 86;

// Shake animation state
unsigned long shakeStartTime = 0;
unsigned long lastTouchTime = 0;
int16_t lastAccelX = 0, lastAccelY = 0, lastAccelZ = 0;

// ---------------- Hardware Helpers ----------------
void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playToneBeep(int freq, int durationMs) {
  if (!soundEnabled) return;
  tone(SPEAKER_PIN, freq, durationMs);
}

void playShakeRattle() {
  if (!soundEnabled) return;
  for (int i = 0; i < 3; i++) {
    tone(SPEAKER_PIN, random(300, 900), 18);
    delay(24);
  }
}

void playRevealChime(AnswerSentiment s) {
  if (!soundEnabled) return;
  if (s == SENT_POSITIVE) {
    // Triumphant ascending arpeggio
    tone(SPEAKER_PIN, 523, 70); delay(75);
    tone(SPEAKER_PIN, 659, 70); delay(75);
    tone(SPEAKER_PIN, 784, 80); delay(85);
    tone(SPEAKER_PIN, 1046, 140);
  } else if (s == SENT_NEUTRAL) {
    // Eerie ambient wobble
    tone(SPEAKER_PIN, 440, 90); delay(95);
    tone(SPEAKER_PIN, 415, 120);
  } else if (s == SENT_NEGATIVE) {
    // Low double buzz
    tone(SPEAKER_PIN, 220, 100); delay(110);
    tone(SPEAKER_PIN, 180, 150);
  } else {
    // 8-bit cyber trill
    for (int f = 600; f < 1800; f += 200) {
      tone(SPEAKER_PIN, f, 20); delay(24);
    }
  }
}

void playClickSound() {
  if (!soundEnabled) return;
  tone(SPEAKER_PIN, 1600, 14);
}

// ---------------- I2C Accelerometer Detection ----------------
void checkI2cSensor() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setTimeOut(30);

  // Probe MPU6050 (0x68)
  Wire.beginTransmission(0x68);
  if (Wire.endTransmission() == 0) {
    hasI2cAccel = true;
    // Wake up MPU6050 (PWR_MGMT_1 = 0)
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission();
    Serial.println("[I2C] MPU-6050 Accelerometer Detected on GPIO 27/22!");
    return;
  }

  // Probe ADXL345 (0x53)
  Wire.beginTransmission(0x53);
  if (Wire.endTransmission() == 0) {
    hasI2cAccel = true;
    Serial.println("[I2C] ADXL345 Accelerometer Detected on GPIO 27/22!");
    return;
  }

  hasI2cAccel = false;
  Serial.println("[I2C] No digital axis sensor detected (Touch Shake active).");
}

bool checkPhysicalShake() {
  if (!hasI2cAccel) return false;

  Wire.beginTransmission(0x68);
  Wire.write(0x3B); // Accel Xout_H
  if (Wire.endTransmission(false) != 0) return false;

  if (Wire.requestFrom((uint8_t)0x68, (size_t)6) == 6) {
    int16_t ax = (Wire.read() << 8) | Wire.read();
    int16_t ay = (Wire.read() << 8) | Wire.read();
    int16_t az = (Wire.read() << 8) | Wire.read();

    int delta = abs(ax - lastAccelX) + abs(ay - lastAccelY) + abs(az - lastAccelZ);
    lastAccelX = ax;
    lastAccelY = ay;
    lastAccelZ = az;

    // Threshold for physical shaking
    if (delta > 14000) return true;
  }
  return false;
}

// ---------------- Drawing Functions ----------------
void drawHeader() {
  tft.fillRect(0, 0, 320, 24, COL_PANEL);
  tft.drawFastHLine(0, 24, 320, COL_BORDER);

  // Oracle Mode Button
  tft.fillRoundRect(3, 3, 94, 18, 3, COL_PURPLE);
  tft.setTextColor(COL_WHITE, COL_PURPLE);
  tft.setTextSize(1);
  tft.setCursor(8, 8);
  if (currentCategory == CAT_ALL) tft.print("MODE: ALL (32)");
  else if (currentCategory == CAT_CLASSIC) tft.print("MODE: CLASSIC");
  else tft.print("MODE: TECH");

  // Audio Button
  tft.fillRoundRect(102, 3, 62, 18, 3, soundEnabled ? 0x03E0 : 0x4208);
  tft.setTextColor(COL_WHITE, soundEnabled ? 0x03E0 : 0x4208);
  tft.setCursor(108, 8);
  tft.print(soundEnabled ? "AUDIO ON" : "MUTED");

  // Accelerometer Status Pill
  tft.fillRoundRect(168, 3, 76, 18, 3, hasI2cAccel ? 0x04F3 : 0x2965);
  tft.setTextColor(COL_WHITE, hasI2cAccel ? 0x04F3 : 0x2965);
  tft.setCursor(174, 8);
  tft.print(hasI2cAccel ? "AXIS: DETECT" : "AXIS: TOUCH");

  // Shake Action Pill
  tft.fillRoundRect(248, 3, 68, 18, 3, 0x05DF);
  tft.setTextColor(TFT_BLACK, 0x05DF);
  tft.setCursor(256, 8);
  tft.print("SHAKE >");
}

void drawMagic8BallIdle() {
  tft.fillRect(0, 25, 320, 192, COL_BG);

  // 1. Shaded 3D Sphere Body
  // Concentric shading for 3D sphere illusion
  for (int r = ballRadius; r > 30; r -= 3) {
    uint8_t c = map(r, ballRadius, 30, 0x00, 0x30);
    uint16_t shade = tft.color565(c, c, c + 8);
    tft.fillCircle(ballCenterX, ballCenterY, r, shade);
  }

  // Specular Reflection Highlight (Upper Left)
  tft.fillCircle(ballCenterX - 36, ballCenterY - 36, 14, 0x5AEB);
  tft.fillCircle(ballCenterX - 38, ballCenterY - 38, 7, COL_WHITE);

  // 2. White Center Circle (Classic 8 Circle)
  tft.fillCircle(ballCenterX, ballCenterY, 34, COL_WHITE);
  tft.drawCircle(ballCenterX, ballCenterY, 34, COL_BORDER);
  tft.drawCircle(ballCenterX, ballCenterY, 35, 0x0000);

  // 3. Bold Black Numeral '8'
  tft.setTextColor(TFT_BLACK, COL_WHITE);
  tft.setTextSize(4);
  tft.setCursor(ballCenterX - 11, ballCenterY - 14);
  tft.print("8");

  // Footer Instructions
  tft.fillRect(0, 218, 320, 22, COL_PANEL);
  tft.drawFastHLine(0, 217, 320, COL_BORDER);
  tft.setTextColor(COL_CYAN, COL_PANEL);
  tft.setTextSize(1);
  tft.setCursor(18, 224);
  tft.print("ASK A QUESTION IN YOUR MIND, THEN TAP OR SHAKE");

  setRgbLed(false, false, true); // Idle Blue glow
}

void triggerShake() {
  currentState = STATE_SHAKING;
  shakeStartTime = millis();

  // Pick Answer based on Category Filter
  if (currentCategory == CAT_CLASSIC) {
    chosenAnswerIdx = random(0, 22); // 0 to 21
  } else if (currentCategory == CAT_TECH) {
    chosenAnswerIdx = random(22, TOTAL_ANSWERS); // 22 to 31
  } else {
    chosenAnswerIdx = random(0, TOTAL_ANSWERS); // All 32
  }

  // Draw Dark Blue Liquid Window
  tft.fillCircle(ballCenterX, ballCenterY, 38, COL_BLUE_LIQUID);
  tft.drawCircle(ballCenterX, ballCenterY, 38, COL_BORDER);

  // Dynamic vortex / bubbles animation
  for (int step = 0; step < 16; step++) {
    // Redraw liquid
    tft.fillCircle(ballCenterX, ballCenterY, 37, COL_BLUE_LIQUID);

    // Swirling bubbles
    for (int b = 0; b < 6; b++) {
      float a = random(0, 360) * DEG_TO_RAD;
      int dist = random(6, 32);
      int bx = ballCenterX + (int)(dist * cos(a));
      int by = ballCenterY + (int)(dist * sin(a));
      tft.fillCircle(bx, by, random(1, 4), 0x041F);
    }

    // Swirl RGB LED
    if (step % 2 == 0) setRgbLed(true, false, true); // Magenta
    else setRgbLed(false, false, true); // Blue

    playShakeRattle();
    delay(40);
  }

  // Enter Revealed State
  currentState = STATE_REVEALED;
  renderAnswerReveal();
}

void renderAnswerReveal() {
  const OracleAnswer& ans = answers[chosenAnswerIdx];

  // 1. Draw Liquid Well
  tft.fillCircle(ballCenterX, ballCenterY, 52, COL_BLUE_LIQUID);
  tft.drawCircle(ballCenterX, ballCenterY, 52, COL_CYAN);

  // 2. Draw Floating Blue Die Triangle (Icosahedron Face)
  // Inverted equilateral triangle floating in fluid
  int tX1 = ballCenterX;
  int tY1 = ballCenterY + 42;
  int tX2 = ballCenterX - 44;
  int tY2 = ballCenterY - 34;
  int tX3 = ballCenterX + 44;
  int tY3 = ballCenterY - 34;

  tft.fillTriangle(tX1, tY1, tX2, tY2, tX3, tY3, COL_TRIANGLE);
  tft.drawTriangle(tX1, tY1, tX2, tY2, tX3, tY3, COL_CYAN);

  // 3. Highlighted Inner Bevel
  tft.drawTriangle(tX1, tY1 - 2, tX2 + 2, tY2 + 1, tX3 - 2, tY3 + 1, 0x05DF);

  // 4. Render Glowing Oracle Text
  uint16_t textCol = COL_WHITE;
  if (ans.sentiment == SENT_POSITIVE) {
    setRgbLed(false, true, false); // Emerald Green
  } else if (ans.sentiment == SENT_NEUTRAL) {
    setRgbLed(true, true, false);  // Amber
  } else if (ans.sentiment == SENT_NEGATIVE) {
    setRgbLed(true, false, false); // Red
  } else {
    setRgbLed(false, true, true);  // Cyan Tech
  }

  tft.setTextColor(textCol, COL_TRIANGLE);
  tft.setTextSize(1);

  // Center Line 1
  int len1 = strlen(ans.line1);
  int x1 = ballCenterX - (len1 * 6) / 2;
  tft.setCursor(x1, ballCenterY - 14);
  tft.print(ans.line1);

  // Center Line 2
  int len2 = strlen(ans.line2);
  int x2 = ballCenterX - (len2 * 6) / 2;
  tft.setCursor(x2, ballCenterY);
  tft.print(ans.line2);

  // Play Oracle Music
  playRevealChime(ans.sentiment);

  // Update Footer
  tft.fillRect(0, 218, 320, 22, COL_PANEL);
  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setCursor(18, 224);
  tft.print("ORACLE SPOKEN! TAP ANYWHERE TO ASK AGAIN");
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n=======================================================");
  Serial.println("  Magic 8 CYD - Project 41");
  Serial.println("  Interactive Mystical Oracle & Shake Synthesizer");
  Serial.println("  Author: Tushar Aggarwal (@tushar2704)");
  Serial.println("  Channels: @IamNotIntoTech | @Makerrdesigns");
  Serial.println("=======================================================\n");

  // Seed random from ESP32 hardware entropy
  randomSeed(esp_random());

  // Pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(false, false, false);

  pinMode(SPEAKER_PIN, OUTPUT);

  // Display Init
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COL_BG);

  // Touch Init (VSPI dedicated pins)
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Detect I2C Digital Axis Sensor (MPU6050 / ADXL345)
  checkI2cSensor();

  // Render Initial UI
  drawHeader();
  drawMagic8BallIdle();

  // Mystical Greeting Chime
  playRevealChime(SENT_POSITIVE);
}

// ---------------- Main Loop ----------------
void loop() {
  unsigned long now = millis();

  // 1. Physical Shake Detection via Accelerometer (if attached)
  if (currentState != STATE_SHAKING && checkPhysicalShake()) {
    triggerShake();
    return;
  }

  // 2. Touch Screen Interaction
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (now - lastTouchTime > 220) {
        lastTouchTime = now;

        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        // Header Actions (Y <= 24)
        if (screenY <= 24) {
          playClickSound();
          // Oracle Category Toggle (X: 3..98)
          if (screenX <= 98) {
            currentCategory = (OracleCategory)((currentCategory + 1) % 3);
            drawHeader();
          }
          // Audio Mute/Unmute (X: 102..164)
          else if (screenX >= 102 && screenX <= 164) {
            soundEnabled = !soundEnabled;
            drawHeader();
          }
          // Re-check I2C Sensor (X: 168..244)
          else if (screenX >= 168 && screenX <= 244) {
            checkI2cSensor();
            drawHeader();
          }
          // SHAKE Button (X: 248..316)
          else if (screenX >= 248) {
            triggerShake();
            return;
          }
        }
        // If an answer is currently revealed: tap anywhere resets back to idle '8'
        else if (currentState == STATE_REVEALED) {
          playClickSound();
          currentState = STATE_IDLE_8;
          drawMagic8BallIdle();
        }
        // If in idle state: tap the ball area triggers shake
        else if (currentState == STATE_IDLE_8) {
          // Distance from ball center
          int dx = screenX - ballCenterX;
          int dy = screenY - ballCenterY;
          if (dx * dx + dy * dy <= ballRadius * ballRadius + 200) {
            triggerShake();
            return;
          }
        }
      }
    }
  }

  delay(20);
}

// =========================================================================
//  Project 53 — CYD CyberPet Emotive Desk Face
//  Cheap Yellow Display (ESP32-2432S028R)
//
//  📺 Tutorial:  https://youtube.com/@IamNotIntoTech
//  🖨️ Enclosure: https://youtube.com/@Makerrdesigns
//
//  Branding: IamNotIntoTech
// =========================================================================

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

// =========================================================================
//  HARDWARE PINS & CALIBRATION (CYD 2.8" ESP32-2432S028R)
// =========================================================================
#define TOUCH_CS   33
#define TOUCH_CLK  25
#define TOUCH_MISO 39
#define TOUCH_MOSI 32
#define TOUCH_IRQ  36

#define LED_R      4
#define LED_G      16
#define LED_B      17

#define BUZZER_PIN 26
#define LDR_PIN    34

#define TS_MIN_X 240
#define TS_MAX_X 3800
#define TS_MIN_Y 240
#define TS_MAX_Y 3800

// High-contrast cyber colors
#define C_BLACK    0x0000  // Pure black
#define C_BG       0x0842  // Deep midnight slate
#define C_EYE      0x07FF  // Electric Cyan eyes
#define C_CYAN     0x07FF  // Electric Cyan
#define C_PUPIL    0x0000  // Pure black pupil
#define C_WHITE    0xFFFF  // Specular reflection
#define C_BLUSH    0xFBAE  // Sakura Pink blush
#define C_MOUTH    0xFD20  // Warm orange/amber
#define C_GREEN    0x07E0  // Matrix green
#define C_YELLOW   0xFFE0  // Bright yellow
#define C_RED      0xF800  // Angry red
#define C_PURPLE   0x915C  // Sleepy violet
#define C_DIM      0x4208  // Muted gray
#define C_CARD     0x10A2  // Dark card

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// =========================================================================
//  PET STATE & MOOD ENGINE
// =========================================================================
enum PetMood {
    MOOD_HAPPY,
    MOOD_EXCITED,
    MOOD_LOVE,
    MOOD_COOL,
    MOOD_BORED,
    MOOD_SLEEPY,
    MOOD_SHOCKED,
    MOOD_GRUMPY
};

PetMood currentMood = MOOD_HAPPY;

// Tamagotchi stats
int happiness = 85; // 0..100
int energy = 90;    // 0..100
int petCount = 14;
bool showHUD = true;

// Procedural Eye State
const int EYE_L_X = 100;
const int EYE_R_X = 220;
const int EYE_Y   = 112;
const int EYE_W   = 44;
const int EYE_H   = 52;

int pupilOffsetX = 0;
int pupilOffsetY = 0;
int targetPupilX = 0;
int targetPupilY = 0;

bool isBlinking = false;
unsigned long lastBlinkTime = 0;
unsigned long blinkInterval = 3500;
unsigned long moodChangeTime = 0;
unsigned long lastTickTime = 0;
unsigned long lastTouchTime = 0;

// Zzz animation for sleep mode
int zzzY[3] = {100, 80, 60};
int zzzX[3] = {230, 245, 260};

// Forward declarations
void setRGB(uint8_t r, uint8_t g, uint8_t b);
void playTone(int freq, int durationMs);
void chirpHappy();
void chirpExcited();
void chirpLove();
void chirpPurr();
void chirpShock();
void chirpGrumpy();
void drawFace();
void drawHUD();
void handleTouch();
void updateMood();

// =========================================================================
//  AUDIO ENGINE
// =========================================================================
void playTone(int freq, int durationMs) {
    tone(BUZZER_PIN, freq, durationMs);
}

void chirpHappy() {
    playTone(1318, 50); // E6
    delay(50);
    playTone(1760, 90); // A6
}

void chirpExcited() {
    for (int f = 1200; f < 2400; f += 200) {
        playTone(f, 30);
        delay(30);
    }
}

void chirpLove() {
    playTone(1046, 70); // C6
    delay(70);
    playTone(1318, 70); // E6
    delay(70);
    playTone(1568, 70); // G6
    delay(70);
    playTone(2093, 140); // C7
}

void chirpPurr() {
    for (int i = 0; i < 4; i++) {
        playTone(320, 25);
        delay(35);
    }
}

void chirpShock() {
    playTone(2200, 120);
    delay(130);
    playTone(880, 180);
}

void chirpGrumpy() {
    playTone(280, 80);
    delay(90);
    playTone(220, 140);
}

// =========================================================================
//  RGB LED FEEDBACK (ACTIVE LOW)
// =========================================================================
void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
}

void updateMoodLED() {
    switch (currentMood) {
        case MOOD_HAPPY:   setRGB(0, 255, 120); break; // Cyan/Green
        case MOOD_EXCITED: setRGB(255, 255, 0); break; // Yellow
        case MOOD_LOVE:    setRGB(255, 40, 180); break; // Pink/Magenta
        case MOOD_COOL:    setRGB(0, 240, 255); break; // Electric Cyan
        case MOOD_BORED:   setRGB(40, 60, 180); break; // Dim Blue
        case MOOD_SLEEPY:  setRGB(120, 0, 180); break; // Purple
        case MOOD_SHOCKED: setRGB(255, 150, 0); break; // Orange
        case MOOD_GRUMPY:  setRGB(255, 0, 0);   break; // Red
    }
}

// =========================================================================
//  PROCEDURAL VECTOR EYE & MOUTH DRAWING
// =========================================================================
void drawEye(int cx, int cy, bool isLeft) {
    if (isBlinking || currentMood == MOOD_SLEEPY) {
        // Closed / Blinking curved eye line
        tft.fillRoundRect(cx - EYE_W / 2, cy - 4, EYE_W, 8, 4, C_EYE);
        tft.drawFastHLine(cx - EYE_W / 2, cy, EYE_W, C_WHITE);
        return;
    }

    if (currentMood == MOOD_LOVE) {
        // Heart-shaped eyes!
        int hx = cx - 18;
        int hy = cy - 16;
        tft.fillCircle(hx + 10, hy + 10, 10, C_BLUSH);
        tft.fillCircle(hx + 26, hy + 10, 10, C_BLUSH);
        tft.fillTriangle(hx, hy + 14, hx + 36, hy + 14, hx + 18, hy + 34, C_BLUSH);
        return;
    }

    if (currentMood == MOOD_COOL) {
        // Pixel Sunglasses!
        int sx = cx - EYE_W / 2 - 2;
        int sy = cy - 18;
        tft.fillRect(sx, sy, EYE_W + 4, 30, C_BLACK);
        tft.drawRect(sx, sy, EYE_W + 4, 30, C_WHITE);
        // Reflection shine line
        tft.drawLine(sx + 6, sy + 6, sx + 22, sy + 22, C_WHITE);
        tft.drawLine(sx + 7, sy + 6, sx + 23, sy + 22, C_WHITE);
        if (isLeft) {
            // Bridge connecting the glasses
            tft.fillRect(EYE_L_X + EYE_W / 2, cy - 8, EYE_R_X - EYE_L_X - EYE_W, 6, C_BLACK);
            tft.drawFastHLine(EYE_L_X + EYE_W / 2, cy - 8, EYE_R_X - EYE_L_X - EYE_W, C_WHITE);
        }
        return;
    }

    // Outer Eye Sclera (Pill rounded rectangle)
    uint16_t eyeColor = (currentMood == MOOD_GRUMPY) ? C_RED : ((currentMood == MOOD_EXCITED) ? C_YELLOW : C_EYE);
    tft.fillRoundRect(cx - EYE_W / 2, cy - EYE_H / 2, EYE_W, EYE_H, 20, eyeColor);
    tft.drawRoundRect(cx - EYE_W / 2, cy - EYE_H / 2, EYE_W, EYE_H, 20, C_WHITE);

    // Eyelid droop for BORED
    if (currentMood == MOOD_BORED) {
        tft.fillRoundRect(cx - EYE_W / 2, cy - EYE_H / 2, EYE_W, 22, 10, C_BG);
        tft.drawFastHLine(cx - EYE_W / 2, cy - EYE_H / 2 + 22, EYE_W, C_WHITE);
    }

    // Slanted angry eyebrows for GRUMPY
    if (currentMood == MOOD_GRUMPY) {
        int browY = cy - EYE_H / 2 - 6;
        if (isLeft) {
            tft.fillTriangle(cx - 24, browY - 6, cx + 24, browY + 8, cx - 24, browY + 2, C_WHITE);
        } else {
            tft.fillTriangle(cx - 24, browY + 8, cx + 24, browY - 6, cx + 24, browY + 2, C_WHITE);
        }
    }

    // Pupil
    int pupilRadius = (currentMood == MOOD_SHOCKED) ? 7 : ((currentMood == MOOD_EXCITED) ? 18 : 14);
    int px = cx + pupilOffsetX;
    int py = cy + pupilOffsetY;
    px = constrain(px, cx - 12, cx + 12);
    py = constrain(py, cy - 14, cy + 14);

    tft.fillCircle(px, py, pupilRadius, C_PUPIL);

    // Specular Reflection (Anime light shine)
    tft.fillCircle(px - pupilRadius / 3, py - pupilRadius / 3, pupilRadius / 3, C_WHITE);
    tft.fillCircle(px + pupilRadius / 2, py + pupilRadius / 2, pupilRadius / 5, C_WHITE);
}

void drawMouth() {
    int mx = 160;
    int my = 176;

    // Clear mouth zone
    tft.fillRect(mx - 40, my - 14, 80, 32, C_BG);

    switch (currentMood) {
        case MOOD_HAPPY:
        case MOOD_LOVE:
        case MOOD_COOL:
            // Curved smile
            tft.drawCircle(mx, my - 6, 20, C_WHITE);
            tft.fillRect(mx - 22, my - 28, 44, 22, C_BG); // Cut upper half
            break;

        case MOOD_EXCITED:
            // Big open laughing mouth
            tft.fillCircle(mx, my, 18, C_MOUTH);
            tft.fillRect(mx - 20, my - 20, 40, 20, C_BG); // Cut top
            tft.fillCircle(mx, my + 6, 8, C_BLUSH);       // Tongue
            tft.drawCircle(mx, my, 18, C_WHITE);
            break;

        case MOOD_BORED:
            // Flat straight mouth
            tft.fillRect(mx - 18, my, 36, 4, C_WHITE);
            break;

        case MOOD_SHOCKED:
            // Big O mouth
            tft.fillCircle(mx, my + 2, 12, C_PUPIL);
            tft.drawCircle(mx, my + 2, 12, C_WHITE);
            break;

        case MOOD_GRUMPY:
            // Downward frown
            tft.drawCircle(mx, my + 14, 20, C_WHITE);
            tft.fillRect(mx - 22, my + 14, 44, 22, C_BG); // Cut lower half
            break;

        case MOOD_SLEEPY:
            // Tiny sleeping "o" mouth
            tft.drawCircle(mx, my, 5, C_WHITE);
            break;
    }
}

void drawBlush() {
    if (currentMood == MOOD_HAPPY || currentMood == MOOD_LOVE || currentMood == MOOD_EXCITED) {
        tft.fillCircle(55, 148, 12, C_BLUSH);
        tft.fillCircle(265, 148, 12, C_BLUSH);
        // Little blush lines
        tft.drawLine(50, 144, 46, 152, C_WHITE);
        tft.drawLine(56, 144, 52, 152, C_WHITE);
        tft.drawLine(260, 144, 256, 152, C_WHITE);
        tft.drawLine(266, 144, 262, 152, C_WHITE);
    } else {
        // Clear blush zones
        tft.fillCircle(55, 148, 14, C_BG);
        tft.fillCircle(265, 148, 14, C_BG);
    }
}

void drawSleepZzz() {
    if (currentMood != MOOD_SLEEPY) return;

    for (int i = 0; i < 3; i++) {
        // Erase old position
        tft.setTextColor(C_BG, C_BG);
        tft.drawString("Z", zzzX[i], zzzY[i], (i == 0) ? 1 : 2);

        // Move up
        zzzY[i] -= 2;
        if (zzzY[i] < 36) {
            zzzY[i] = 110;
        }

        // Draw new
        tft.setTextColor(C_PURPLE, C_BG);
        tft.drawString("Z", zzzX[i], zzzY[i], (i == 0) ? 1 : 2);
    }
}

void drawHUD() {
    if (!showHUD) return;

    // Top status ribbon with IamNotIntoTech branding
    tft.fillRect(0, 0, 320, 24, C_CARD);
    tft.drawFastHLine(0, 24, 320, C_DIM);

    // Pet Name / Branding
    tft.setTextColor(C_CYAN, C_CARD);
    tft.setTextSize(1);
    tft.drawString("CYBERPET", 8, 6, 2);

    tft.setTextColor(C_YELLOW, C_CARD);
    tft.drawString("IamNotIntoTech", 195, 6, 2);

    // Mood Badge Pill
    const char* moodNames[] = {"HAPPY", "EXCITED", "LOVE", "COOL", "BORED", "SLEEP", "SHOCK", "GRUMP"};
    tft.fillRoundRect(102, 3, 76, 18, 4, C_BG);
    tft.drawRoundRect(102, 3, 76, 18, 4, C_DIM);
    tft.setTextColor(C_GREEN, C_BG);
    tft.drawString(moodNames[currentMood], 112, 6, 1);

    // Bottom Stats bar (Happiness & Energy & Pets)
    tft.fillRect(0, 218, 320, 22, C_CARD);
    tft.drawFastHLine(0, 218, 320, C_DIM);

    char statBuf[64];
    snprintf(statBuf, sizeof(statBuf), "Happy: %d%%  Energy: %d%%  Pets: %d", happiness, energy, petCount);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString(statBuf, 12, 222, 1);

    tft.setTextColor(C_DIM, C_CARD);
    tft.drawString("(Double-tap to hide HUD)", 182, 222, 1);
}

void drawFace() {
    drawEye(EYE_L_X, EYE_Y, true);
    drawEye(EYE_R_X, EYE_Y, false);
    drawMouth();
    drawBlush();
}

// =========================================================================
//  MOOD & AI LOGIC
// =========================================================================
void setMood(PetMood newMood) {
    if (currentMood == newMood) return;
    currentMood = newMood;
    moodChangeTime = millis();
    updateMoodLED();

    tft.fillScreen(C_BG);
    drawHUD();
    drawFace();

    switch (newMood) {
        case MOOD_HAPPY:   chirpHappy();   break;
        case MOOD_EXCITED: chirpExcited(); break;
        case MOOD_LOVE:    chirpLove();    break;
        case MOOD_COOL:    playTone(880, 80); delay(90); playTone(1175, 140); break;
        case MOOD_SHOCKED: chirpShock();   break;
        case MOOD_GRUMPY:  chirpGrumpy();  break;
        case MOOD_SLEEPY:  chirpPurr();    break;
        default: break;
    }
}

void updateMood() {
    unsigned long now = millis();

    // Ambient Light Sensor (LDR) Check
    int lightLvl = analogRead(LDR_PIN);
    if (lightLvl < 600 && currentMood != MOOD_SLEEPY) {
        // Room lights turned off: fall asleep!
        setMood(MOOD_SLEEPY);
        return;
    } else if (lightLvl >= 1200 && currentMood == MOOD_SLEEPY) {
        // Room lights turned on: wake up!
        setMood(MOOD_HAPPY);
        return;
    }

    // Auto mood decay if untouched for a while
    if (now - lastTouchTime > 25000 && currentMood != MOOD_SLEEPY) {
        if (currentMood != MOOD_BORED) {
            setMood(MOOD_BORED);
        }
    }

    // Random idle pupil wandering
    if (now - lastTickTime > 1800 && currentMood != MOOD_SLEEPY) {
        lastTickTime = now;
        targetPupilX = random(-12, 13);
        targetPupilY = random(-10, 11);
    }

    // Smooth pupil interpolation
    if (pupilOffsetX != targetPupilX || pupilOffsetY != targetPupilY) {
        pupilOffsetX += (targetPupilX - pupilOffsetX) / 2;
        pupilOffsetY += (targetPupilY - pupilOffsetY) / 2;
        drawFace();
    }

    // Blinking trigger
    if (now - lastBlinkTime > blinkInterval && currentMood != MOOD_SLEEPY) {
        lastBlinkTime = now;
        blinkInterval = random(2500, 5000);
        isBlinking = true;
        drawFace();
        delay(120);
        isBlinking = false;
        drawFace();
    }

    // Sleep Zzz drift
    if (currentMood == MOOD_SLEEPY) {
        drawSleepZzz();
        delay(60);
    }
}

// =========================================================================
//  TOUCH INTERACTION (PETTING, TICKLING, POKING)
// =========================================================================
void handleTouch() {
    if (!touch.touched()) return;

    unsigned long now = millis();
    if (now - lastTouchTime < 200) return; // Debounce

    TS_Point p = touch.getPoint();
    int tx = map(p.x, TS_MIN_X, TS_MAX_X, 0, 320);
    int ty = map(p.y, TS_MIN_Y, TS_MAX_Y, 0, 240);
    tx = constrain(tx, 0, 320);
    ty = constrain(ty, 0, 240);

    lastTouchTime = now;

    // Double-tap at the very top toggles HUD
    if (ty < 26) {
        showHUD = !showHUD;
        tft.fillScreen(C_BG);
        drawHUD();
        drawFace();
        return;
    }

    // If sleeping: tap gently wakes up!
    if (currentMood == MOOD_SLEEPY) {
        setMood(MOOD_HAPPY);
        return;
    }

    // Interactive Touch Zones:
    // 1. Forehead / Head (Y < 75): Petting!
    if (ty < 75) {
        petCount++;
        happiness = min(100, happiness + 5);
        targetPupilX = 0;
        targetPupilY = -8;
        if (petCount % 5 == 0) {
            setMood(MOOD_LOVE); // Heart eyes after consecutive pets!
        } else {
            setMood(MOOD_HAPPY);
        }
        return;
    }

    // 2. Cheeks / Sides (X < 70 or X > 250): Tickles!
    if (tx < 70 || tx > 250) {
        happiness = min(100, happiness + 8);
        setMood(MOOD_EXCITED);
        return;
    }

    // 3. Nose / Center Poke (130 <= X <= 190, 90 <= Y <= 150): Poking!
    if (tx >= 130 && tx <= 190 && ty >= 90 && ty <= 150) {
        setMood(MOOD_SHOCKED);
        return;
    }

    // 4. Chin / Mouth (Y > 165): Cool sunglasses toggle!
    if (ty > 165) {
        if (currentMood == MOOD_COOL) setMood(MOOD_HAPPY);
        else setMood(MOOD_COOL);
        return;
    }

    // Default: Pupils track touch location!
    targetPupilX = map(tx, 0, 320, -12, 12);
    targetPupilY = map(ty, 0, 240, -10, 10);
    playTone(1800, 20);
}

// =========================================================================
//  SETUP & MAIN LOOP
// =========================================================================
void setup() {
    Serial.begin(115200);

    // RGB LED init
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    updateMoodLED();

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    pinMode(LDR_PIN, INPUT);

    tft.init();
    tft.setRotation(1); // Landscape 320x240

    // Splash screen with IamNotIntoTech branding
    tft.fillScreen(C_BG);
    tft.setTextColor(C_EYE, C_BG);
    tft.drawString("CYBERPET EMOTIVE FACE", 30, 60, 4);
    tft.setTextColor(C_YELLOW, C_BG);
    tft.drawString("Interactive AI Desktop Companion", 45, 100, 2);
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("Designed with IamNotIntoTech", 65, 140, 2);

    chirpHappy();

    // Touch Controller Init on HSPI
    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    touch.begin(touchSPI);
    touch.setRotation(1);

    delay(1000);

    tft.fillScreen(C_BG);
    drawHUD();
    drawFace();
}

void loop() {
    handleTouch();
    updateMood();
    delay(20);
}

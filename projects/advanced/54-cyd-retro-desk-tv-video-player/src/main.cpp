// =========================================================================
//  Project 54 — CYD Retro Desk TV / Video & Animation Player
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

// 16-bit 565 Colors
#define C_BLACK       0x0000
#define C_WHITE       0xFFFF
#define C_GRAY        0x8410
#define C_DARKGRAY    0x2104
#define C_CHASSIS     0x18C3  // Vintage dark slate television body
#define C_BEZEL       0x3186  // Plastic screen bezel
#define C_PANEL       0x2124  // Side control panel
#define C_BUTTON      0x4208
#define C_BUTTON_ACT  0x632C
#define C_OSD_GREEN   0x07E0  // VCR OSD Green
#define C_CYAN        0x07FF
#define C_YELLOW      0xFFE0
#define C_MAGENTA     0xF81F
#define C_RED         0xF800
#define C_BLUE        0x001F
#define C_AMBER       0xFDC0
#define C_NEON_PINK   0xFD14
#define C_NEON_PURPLE 0x901F

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// =========================================================================
//  TV GEOMETRY & STATE
// =========================================================================
// Active CRT Screen Area
const int SCR_X = 6;
const int SCR_Y = 6;
const int SCR_W = 236;
const int SCR_H = 228;

// Right Control Stack Area
const int CTL_X = 246;
const int CTL_Y = 0;
const int CTL_W = 74;
const int CTL_H = 240;

const int NUM_CHANNELS = 6;
int currentChannel = 0; // 0..5
bool tvPoweredOn = true;
bool scanlinesEnabled = true;
int volumeLevel = 2;    // 0: Mute, 1: Low, 2: Med, 3: High
bool osdVisible = true;
unsigned long osdTimer = 0;

// DVD Bouncer State
float dvdX = 50.0;
float dvdY = 50.0;
float dvdVx = 2.4;
float dvdVy = 1.8;
const int DVD_W = 44;
const int DVD_H = 22;
uint16_t dvdColor = C_CYAN;
int dvdBounces = 0;
int dvdCornerHits = 0;
unsigned long dvdCornerHitTime = 0;

// Matrix Rain State
const int MATRIX_COLS = 19;
int matrixY[MATRIX_COLS];
int matrixSpeed[MATRIX_COLS];
char matrixChars[MATRIX_COLS];

// Synthwave State
float horizonPhase = 0;

// Oscilloscope State
float oscPhase = 0;

// Frame timing
unsigned long lastFrameTime = 0;
unsigned long lastAudioToneTime = 0;

// =========================================================================
//  SOUND & HAPTICS
// =========================================================================
void playBeep(int freq, int ms) {
    if (volumeLevel == 0) return;
    int scaledMs = ms;
    if (volumeLevel == 1) scaledMs = ms / 2;
    tone(BUZZER_PIN, freq, scaledMs);
}

void playChirp() {
    if (volumeLevel == 0) return;
    tone(BUZZER_PIN, 1800, 20);
    delay(25);
    tone(BUZZER_PIN, 2400, 30);
}

void setTVLed(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
}

// =========================================================================
//  CRT POWER TRANSITIONS
// =========================================================================
void animatePowerOff() {
    playBeep(440, 100);
    // 1. Collapse vertically to a horizontal bright white line in center
    int centerY = SCR_Y + (SCR_H / 2);
    for (int h = SCR_H / 2; h >= 2; h -= 6) {
        tft.fillRect(SCR_X, SCR_Y + (SCR_H / 2) - h, SCR_W, 6, C_BLACK);
        tft.fillRect(SCR_X, SCR_Y + (SCR_H / 2) + h - 6, SCR_W, 6, C_BLACK);
        delay(12);
    }
    tft.fillRect(SCR_X, centerY - 2, SCR_W, 4, C_WHITE);
    delay(100);

    // 2. Collapse horizontally to a single center dot
    int centerX = SCR_X + (SCR_W / 2);
    for (int w = SCR_W / 2; w >= 4; w -= 10) {
        tft.fillRect(SCR_X + (SCR_W / 2) - w, centerY - 2, 10, 4, C_BLACK);
        tft.fillRect(SCR_X + (SCR_W / 2) + w - 10, centerY - 2, 10, 4, C_BLACK);
        delay(10);
    }
    tft.fillCircle(centerX, centerY, 4, C_WHITE);
    delay(180);
    tft.fillCircle(centerX, centerY, 4, C_GRAY);
    delay(180);
    tft.fillCircle(centerX, centerY, 4, C_BLACK);
    
    // Turn screen black and LED red (standby)
    tft.fillRect(SCR_X, SCR_Y, SCR_W, SCR_H, C_BLACK);
    setTVLed(80, 0, 0); // Standby Red
}

void animatePowerOn() {
    setTVLed(0, 100, 120); // Powering on cyan/green
    playChirp();
    
    int centerX = SCR_X + (SCR_W / 2);
    int centerY = SCR_Y + (SCR_H / 2);
    
    // Phosphor warm-up circle expands
    for (int r = 2; r < 50; r += 6) {
        tft.fillCircle(centerX, centerY, r, C_DARKGRAY);
        delay(15);
    }
    tft.fillRect(SCR_X, SCR_Y, SCR_W, SCR_H, C_BLACK);
}

// =========================================================================
//  SIDE CONTROL PANEL DRAWING
// =========================================================================
void drawControlPanel() {
    // Chassis background
    tft.fillRect(CTL_X, CTL_Y, CTL_W, CTL_H, C_PANEL);
    tft.drawFastVLine(CTL_X, 0, 240, C_DARKGRAY);

    // Brand Badge Header
    tft.fillRect(CTL_X + 2, 6, CTL_W - 4, 24, C_BLACK);
    tft.drawRect(CTL_X + 2, 6, CTL_W - 4, 24, C_BUTTON_ACT);
    tft.setTextColor(C_YELLOW, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNot", CTL_X + (CTL_W / 2), 13, 2);
    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("IntoTech", CTL_X + (CTL_W / 2), 23, 1);

    // Channel Indicator LED Display
    tft.fillRect(CTL_X + 8, 36, CTL_W - 16, 26, C_BLACK);
    tft.drawRect(CTL_X + 8, 36, CTL_W - 16, 26, C_GRAY);
    tft.setTextColor(C_OSD_GREEN, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    char chBuf[8];
    snprintf(chBuf, sizeof(chBuf), "CH %02d", currentChannel + 1);
    tft.drawString(chBuf, CTL_X + (CTL_W / 2), 49, 2);

    // CH + Button
    tft.fillRoundRect(CTL_X + 6, 68, CTL_W - 12, 22, 4, C_BUTTON);
    tft.drawRoundRect(CTL_X + 6, 68, CTL_W - 12, 22, 4, C_WHITE);
    tft.setTextColor(C_WHITE, C_BUTTON);
    tft.drawString("CH  +", CTL_X + (CTL_W / 2), 79, 2);

    // CH - Button
    tft.fillRoundRect(CTL_X + 6, 95, CTL_W - 12, 22, 4, C_BUTTON);
    tft.drawRoundRect(CTL_X + 6, 95, CTL_W - 12, 22, 4, C_WHITE);
    tft.drawString("CH  -", CTL_X + (CTL_W / 2), 106, 2);

    // Volume Button
    tft.fillRoundRect(CTL_X + 6, 122, CTL_W - 12, 22, 4, C_BUTTON);
    tft.drawRoundRect(CTL_X + 6, 122, CTL_W - 12, 22, 4, C_CYAN);
    tft.setTextColor(C_CYAN, C_BUTTON);
    char volBuf[10];
    if (volumeLevel == 0) snprintf(volBuf, sizeof(volBuf), "MUTE");
    else snprintf(volBuf, sizeof(volBuf), "VOL %d", volumeLevel);
    tft.drawString(volBuf, CTL_X + (CTL_W / 2), 133, 2);

    // CRT Scanlines Toggle Button
    uint16_t scanCol = scanlinesEnabled ? C_OSD_GREEN : C_DARKGRAY;
    tft.fillRoundRect(CTL_X + 6, 149, CTL_W - 12, 22, 4, C_BUTTON);
    tft.drawRoundRect(CTL_X + 6, 149, CTL_W - 12, 22, 4, scanCol);
    tft.setTextColor(scanCol, C_BUTTON);
    tft.drawString("SCAN", CTL_X + (CTL_W / 2), 160, 2);

    // Vintage Speaker Grill Slits
    for (int y = 178; y <= 194; y += 4) {
        tft.drawFastHLine(CTL_X + 12, y, CTL_W - 24, C_BLACK);
        tft.drawFastHLine(CTL_X + 12, y + 1, CTL_W - 24, C_GRAY);
    }

    // Power Rocker Button
    uint16_t pwrBg = tvPoweredOn ? C_RED : C_DARKGRAY;
    tft.fillRoundRect(CTL_X + 6, 204, CTL_W - 12, 28, 4, pwrBg);
    tft.drawRoundRect(CTL_X + 6, 204, CTL_W - 12, 28, 4, C_WHITE);
    tft.setTextColor(C_WHITE, pwrBg);
    tft.drawString("POWER", CTL_X + (CTL_W / 2), 218, 2);
}

// =========================================================================
//  CRT BEZEL & SCANLINES
// =========================================================================
void drawCRTBezel() {
    // Outer TV case bevel
    tft.drawRect(SCR_X - 2, SCR_Y - 2, SCR_W + 4, SCR_H + 4, C_DARKGRAY);
    tft.drawRect(SCR_X - 1, SCR_Y - 1, SCR_W + 2, SCR_H + 2, C_BEZEL);
}

void applyScanlines() {
    if (!scanlinesEnabled || !tvPoweredOn) return;
    for (int y = SCR_Y; y < SCR_Y + SCR_H; y += 3) {
        tft.drawFastHLine(SCR_X, y, SCR_W, C_BLACK);
    }
}

void showOSD() {
    osdVisible = true;
    osdTimer = millis();
}

void drawOSD() {
    if (!osdVisible || !tvPoweredOn) return;
    
    if (millis() - osdTimer > 3500) {
        osdVisible = false;
        return;
    }

    // Vintage Green OSD banner in top corner
    tft.setTextColor(C_OSD_GREEN, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    char osdBuf[32];
    const char* chNames[] = {
        "SMPTE BARS",
        "STATIC SNOW",
        "SYNTH HORIZON",
        "BOUNCING DVD",
        "MATRIX RAIN",
        "OSCILLOSCOPE"
    };
    snprintf(osdBuf, sizeof(osdBuf), "CH 0%d  %s", currentChannel + 1, chNames[currentChannel]);
    tft.drawString(osdBuf, SCR_X + 8, SCR_Y + 8, 2);

    tft.setTextDatum(TR_DATUM);
    tft.drawString("NTSC-M  STEREO", SCR_X + SCR_W - 8, SCR_Y + 8, 2);
}

// =========================================================================
//  CHANNEL RENDERERS
// =========================================================================

// --- CH 01: SMPTE COLOR BARS & TEST TONE ---
void renderChannelSMPTE() {
    // 7 standard primary vertical bars
    const uint16_t primaryBars[7] = {
        C_WHITE, C_YELLOW, C_CYAN, C_OSD_GREEN, C_MAGENTA, C_RED, C_BLUE
    };
    int barW = SCR_W / 7;
    int topH = (SCR_H * 2) / 3;

    for (int i = 0; i < 7; i++) {
        int x = SCR_X + (i * barW);
        int w = (i == 6) ? (SCR_X + SCR_W - x) : barW;
        tft.fillRect(x, SCR_Y, w, topH, primaryBars[i]);
    }

    // Castellation / Middle band
    int midY = SCR_Y + topH;
    int midH = SCR_H / 12;
    const uint16_t midBars[7] = {
        C_BLUE, C_BLACK, C_MAGENTA, C_BLACK, C_CYAN, C_BLACK, C_WHITE
    };
    for (int i = 0; i < 7; i++) {
        int x = SCR_X + (i * barW);
        int w = (i == 6) ? (SCR_X + SCR_W - x) : barW;
        tft.fillRect(x, midY, w, midH, midBars[i]);
    }

    // Bottom Sub-bars (I-signal, 100% White, Q, Black-4, Black-0, Black+4 PLUGE)
    int botY = midY + midH;
    int botH = SCR_Y + SCR_H - botY;
    tft.fillRect(SCR_X, botY, barW * 2, botH, 0x0810);
    tft.fillRect(SCR_X + (barW * 2), botY, barW * 2, botH, C_WHITE);
    tft.fillRect(SCR_X + (barW * 4), botY, barW, botH, 0x3000);
    tft.fillRect(SCR_X + (barW * 5), botY, barW * 2, botH, C_BLACK);

    // Station Call Sign Card
    tft.fillRect(SCR_X + 24, SCR_Y + 70, SCR_W - 48, 40, C_BLACK);
    tft.drawRect(SCR_X + 24, SCR_Y + 70, SCR_W - 48, 40, C_WHITE);
    tft.setTextColor(C_YELLOW, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech TV", SCR_X + (SCR_W / 2), SCR_Y + 83, 2);
    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("STANDARD TEST NTSC 480i", SCR_X + (SCR_W / 2), SCR_Y + 98, 1);

    // Optional 1 kHz calibration pulse every 2 seconds
    if (millis() - lastAudioToneTime > 2500 && volumeLevel > 0) {
        lastAudioToneTime = millis();
        tone(BUZZER_PIN, 1000, 80);
    }
}

// --- CH 02: ANALOG TV STATIC / WHITE NOISE ---
void renderChannelStatic() {
    // Generate fast noise blocks across the screen
    for (int y = SCR_Y; y < SCR_Y + SCR_H; y += 4) {
        for (int x = SCR_X; x < SCR_X + SCR_W; x += 8) {
            uint8_t r = random(0, 256);
            uint16_t c;
            if (r < 85) c = C_BLACK;
            else if (r < 170) c = C_DARKGRAY;
            else if (r < 235) c = C_GRAY;
            else c = C_WHITE;
            tft.fillRect(x, y, 8, 4, c);
        }
    }

    // Flickering "NO SIGNAL" badge
    if ((millis() / 500) % 2 == 0) {
        tft.fillRect(SCR_X + 40, SCR_Y + 90, SCR_W - 80, 36, C_BLACK);
        tft.drawRect(SCR_X + 40, SCR_Y + 90, SCR_W - 80, 36, C_WHITE);
        tft.setTextColor(C_WHITE, C_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("UHF ANTENNA 02", SCR_X + (SCR_W / 2), SCR_Y + 102, 2);
        tft.drawString("NO SIGNAL / AUTO-MUTE", SCR_X + (SCR_W / 2), SCR_Y + 115, 1);
    }

    // Static hiss audio pulse
    if (volumeLevel > 0 && random(0, 10) == 0) {
        tone(BUZZER_PIN, random(120, 600), 10);
    }
}

// --- CH 03: SYNTHWAVE / OUTRUN SUNSET ---
void renderChannelSynthwave() {
    int horizonY = SCR_Y + 120;

    // 1. Sky Gradient & Mountains
    tft.fillRect(SCR_X, SCR_Y, SCR_W, 60, 0x1808); // Dark purple sky
    tft.fillRect(SCR_X, SCR_Y + 60, SCR_W, 60, 0x600C); // Warm twilight

    // Distant jagged mountains
    for (int x = SCR_X; x < SCR_X + SCR_W; x += 16) {
        int my = horizonY - 12 - (int)(sin((x + 10) * 0.04) * 10);
        tft.drawLine(x, horizonY, x + 8, my, C_BLACK);
        tft.drawLine(x + 8, my, x + 16, horizonY, C_BLACK);
    }

    // 2. Neon Outrun Sun with horizontal scanline cuts
    int sunCenterX = SCR_X + (SCR_W / 2);
    int sunCenterY = horizonY - 16;
    int sunRadius = 34;

    for (int r = sunRadius; r > 0; r -= 2) {
        uint16_t sColor = (r > 18) ? C_NEON_PINK : C_YELLOW;
        tft.drawCircle(sunCenterX, sunCenterY, r, sColor);
    }
    // Sun horizontal scan cuts
    for (int sy = sunCenterY - sunRadius; sy < sunCenterY + sunRadius; sy += 6) {
        if (sy > sunCenterY) {
            int cutH = (sy - sunCenterY) / 4 + 1;
            tft.drawFastHLine(sunCenterX - sunRadius, sy, sunRadius * 2, 0x600C);
        }
    }

    // 3. 3D Perspective Ground Grid
    tft.fillRect(SCR_X, horizonY, SCR_W, SCR_Y + SCR_H - horizonY, C_BLACK);
    tft.drawFastHLine(SCR_X, horizonY, SCR_W, C_NEON_PINK);

    // Vanishing perspective vertical lines
    for (int x = -100; x <= SCR_W + 100; x += 36) {
        tft.drawLine(sunCenterX, horizonY, SCR_X + x, SCR_Y + SCR_H, C_NEON_PURPLE);
    }

    // Moving horizontal grid lines (scrolling towards viewer)
    horizonPhase += 0.8;
    if (horizonPhase > 24) horizonPhase = 0;

    for (float d = horizonPhase; d < 108; d += 16) {
        int py = horizonY + (int)((d * d) / 100);
        if (py < SCR_Y + SCR_H) {
            tft.drawFastHLine(SCR_X, py, SCR_W, C_CYAN);
        }
    }

    // Synth banner
    tft.setTextColor(C_CYAN, C_BLACK);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("IamNotIntoTech SYNTH-84", SCR_X + (SCR_W / 2), SCR_Y + SCR_H - 4, 1);
}

// --- CH 04: BOUNCING DVD SCREENSAVER ---
void renderChannelDVD() {
    // Clear screen area
    tft.fillRect(SCR_X, SCR_Y, SCR_W, SCR_H, C_BLACK);

    // Update DVD coordinates
    dvdX += dvdVx;
    dvdY += dvdVy;

    bool hitX = false;
    bool hitY = false;

    // Left / Right boundaries
    if (dvdX <= SCR_X) {
        dvdX = SCR_X;
        dvdVx = -dvdVx;
        hitX = true;
    } else if (dvdX + DVD_W >= SCR_X + SCR_W) {
        dvdX = SCR_X + SCR_W - DVD_W;
        dvdVx = -dvdVx;
        hitX = true;
    }

    // Top / Bottom boundaries
    if (dvdY <= SCR_Y) {
        dvdY = SCR_Y;
        dvdVy = -dvdVy;
        hitY = true;
    } else if (dvdY + DVD_H >= SCR_Y + SCR_H) {
        dvdY = SCR_Y + SCR_H - DVD_H;
        dvdVy = -dvdVy;
        hitY = true;
    }

    // Color cycle list
    const uint16_t dvdColors[6] = {
        C_CYAN, C_MAGENTA, C_YELLOW, C_OSD_GREEN, C_AMBER, C_WHITE
    };

    if (hitX || hitY) {
        dvdBounces++;
        dvdColor = dvdColors[random(0, 6)];
        playBeep(hitX && hitY ? 2000 : 880, 20);

        // Corner Hit celebration!
        if (hitX && hitY) {
            dvdCornerHits++;
            dvdCornerHitTime = millis();
            setTVLed(255, 255, 0); // Flash yellow
            playChirp();
        }
    }

    // Draw DVD Badge
    int ix = (int)dvdX;
    int iy = (int)dvdY;
    tft.fillRoundRect(ix, iy, DVD_W, DVD_H, 6, dvdColor);
    tft.fillRoundRect(ix + 2, iy + 2, DVD_W - 4, DVD_H - 4, 4, C_BLACK);
    tft.setTextColor(dvdColor, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("DVD", ix + (DVD_W / 2), iy + 8, 2);
    tft.drawString("VIDEO", ix + (DVD_W / 2), iy + 16, 1);

    // Top Stats HUD
    tft.setTextColor(C_DARKGRAY, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    char statBuf[40];
    snprintf(statBuf, sizeof(statBuf), "BOUNCES: %d  CORNERS: %d", dvdBounces, dvdCornerHits);
    tft.drawString(statBuf, SCR_X + 6, SCR_Y + 6, 1);

    // Corner Hit Splash Banner
    if (millis() - dvdCornerHitTime < 2000) {
        tft.fillRoundRect(SCR_X + 30, SCR_Y + 80, SCR_W - 60, 40, 8, C_YELLOW);
        tft.setTextColor(C_BLACK, C_YELLOW);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("PERFECT CORNER HIT!", SCR_X + (SCR_W / 2), SCR_Y + 95, 2);
        tft.drawString("IamNotIntoTech CELEBRATION", SCR_X + (SCR_W / 2), SCR_Y + 110, 1);
    }
}

// --- CH 05: MATRIX DIGITAL RAIN ---
void initMatrix() {
    for (int i = 0; i < MATRIX_COLS; i++) {
        matrixY[i] = random(-40, 0);
        matrixSpeed[i] = random(3, 7);
        matrixChars[i] = '0' + random(0, 10);
    }
}

void renderChannelMatrix() {
    // Semi-dim trailing background effect
    tft.fillRect(SCR_X, SCR_Y, SCR_W, SCR_H, C_BLACK);

    for (int i = 0; i < MATRIX_COLS; i++) {
        int colX = SCR_X + 6 + (i * 12);
        
        // Draw trailing faint green characters
        for (int t = 1; t <= 5; t++) {
            int ty = matrixY[i] - (t * 14);
            if (ty >= SCR_Y && ty < SCR_Y + SCR_H - 12) {
                uint16_t trailCol = (t == 1) ? C_OSD_GREEN : 0x03E0;
                if (t >= 4) trailCol = 0x01E0;
                tft.setTextColor(trailCol, C_BLACK);
                char tc = (char)(33 + ((matrixChars[i] + t * 7) % 90));
                tft.drawChar(tc, colX, ty, 2);
            }
        }

        // Draw leading bright white head glyph
        if (matrixY[i] >= SCR_Y && matrixY[i] < SCR_Y + SCR_H - 12) {
            tft.setTextColor(C_WHITE, C_BLACK);
            tft.drawChar(matrixChars[i], colX, matrixY[i], 2);
        }

        // Advance column
        matrixY[i] += matrixSpeed[i];
        if (matrixY[i] > SCR_Y + SCR_H) {
            matrixY[i] = random(-30, 0);
            matrixSpeed[i] = random(3, 7);
            matrixChars[i] = 'A' + random(0, 26);
        }
    }

    tft.setTextColor(0x07E0, C_BLACK);
    tft.setTextDatum(BR_DATUM);
    tft.drawString("IamNotIntoTech // CYBERDECK", SCR_X + SCR_W - 6, SCR_Y + SCR_H - 4, 1);
}

// --- CH 06: OSCILLOSCOPE & STEREO VU METERS ---
void renderChannelOscilloscope() {
    tft.fillRect(SCR_X, SCR_Y, SCR_W, SCR_H, C_BLACK);

    // Oscilloscope Grid Lines (dark teal/green)
    uint16_t gridCol = 0x0208;
    for (int x = SCR_X; x < SCR_X + SCR_W; x += 24) {
        tft.drawFastVLine(x, SCR_Y, SCR_H, gridCol);
    }
    for (int y = SCR_Y; y < SCR_Y + SCR_H; y += 24) {
        tft.drawFastHLine(SCR_X, y, SCR_W, gridCol);
    }
    // Crosshairs
    int midY = SCR_Y + (SCR_H / 2) - 15;
    tft.drawFastHLine(SCR_X, midY, SCR_W, 0x03E0);
    tft.drawFastVLine(SCR_X + (SCR_W / 2), SCR_Y, SCR_H - 30, 0x03E0);

    // Real-time Waveform trace
    oscPhase += 0.15;
    int lastY = midY;
    for (int x = 0; x < SCR_W; x += 2) {
        float angle = (x * 0.06) + oscPhase;
        int waveY = midY + (int)(sin(angle) * 32.0 + sin(angle * 2.3) * 12.0);
        tft.drawLine(SCR_X + x - 2, lastY, SCR_X + x, waveY, C_OSD_GREEN);
        tft.drawLine(SCR_X + x - 2, lastY + 1, SCR_X + x, waveY + 1, C_WHITE); // Phosphor glow core
        lastY = waveY;
    }

    // Bottom Stereo VU Meter Bars
    int vuY = SCR_Y + SCR_H - 26;
    int vuW = (SCR_W - 24) / 2;

    int leftLevel = (int)((sin(oscPhase * 1.5) + 1.0) * (vuW / 2));
    int rightLevel = (int)((cos(oscPhase * 1.8) + 1.0) * (vuW / 2));

    tft.setTextColor(C_GRAY, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("L", SCR_X + 4, vuY, 1);
    tft.drawString("R", SCR_X + 4, vuY + 12, 1);

    tft.fillRect(SCR_X + 16, vuY, leftLevel, 8, leftLevel > vuW - 15 ? C_RED : C_OSD_GREEN);
    tft.fillRect(SCR_X + 16 + leftLevel, vuY, vuW - leftLevel, 8, C_DARKGRAY);

    tft.fillRect(SCR_X + 16, vuY + 12, rightLevel, 8, rightLevel > vuW - 15 ? C_RED : C_OSD_GREEN);
    tft.fillRect(SCR_X + 16 + rightLevel, vuY + 12, vuW - rightLevel, 8, C_DARKGRAY);

    // Telemetry display
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString("1000 Hz  1.42 Vpp", SCR_X + SCR_W - 6, SCR_Y + 8, 1);
    tft.drawString("IamNotIntoTech LABS", SCR_X + SCR_W - 6, SCR_Y + 18, 1);
}

// =========================================================================
//  CHANNEL SWITCHING & TOUCH
// =========================================================================
void switchChannel(int newCh) {
    if (newCh < 0) newCh = NUM_CHANNELS - 1;
    if (newCh >= NUM_CHANNELS) newCh = 0;
    currentChannel = newCh;

    playChirp();

    // Quick burst of static fuzz for authentic channel tuning effect
    for (int i = 0; i < 3; i++) {
        for (int y = SCR_Y; y < SCR_Y + SCR_H; y += 8) {
            uint16_t sc = (random(0, 2) == 0) ? C_BLACK : C_GRAY;
            tft.drawFastHLine(SCR_X, y, SCR_W, sc);
        }
        delay(10);
    }
    tft.fillRect(SCR_X, SCR_Y, SCR_W, SCR_H, C_BLACK);

    if (currentChannel == 4) initMatrix();

    showOSD();
    drawControlPanel();
}

void handleTouch() {
    if (!touch.touched()) return;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, TS_MIN_X, TS_MAX_X, 0, 320);
    int ty = map(p.y, TS_MIN_Y, TS_MAX_Y, 0, 240);
    tx = constrain(tx, 0, 319);
    ty = constrain(ty, 0, 239);

    // Check Control Stack interactions (X > 240)
    if (tx >= CTL_X) {
        // CH + Button (Y: 68..90)
        if (ty >= 68 && ty <= 90) {
            switchChannel(currentChannel + 1);
            delay(250);
            return;
        }

        // CH - Button (Y: 95..117)
        if (ty >= 95 && ty <= 117) {
            switchChannel(currentChannel - 1);
            delay(250);
            return;
        }

        // Volume Toggle (Y: 122..144)
        if (ty >= 122 && ty <= 144) {
            volumeLevel = (volumeLevel + 1) % 4; // 0, 1, 2, 3
            playBeep(1200, 40);
            drawControlPanel();
            showOSD();
            delay(250);
            return;
        }

        // Scanlines Toggle (Y: 149..171)
        if (ty >= 149 && ty <= 171) {
            scanlinesEnabled = !scanlinesEnabled;
            playBeep(1600, 40);
            drawControlPanel();
            delay(250);
            return;
        }

        // Power Button (Y: 204..232)
        if (ty >= 204 && ty <= 232) {
            tvPoweredOn = !tvPoweredOn;
            if (tvPoweredOn) {
                animatePowerOn();
                showOSD();
            } else {
                animatePowerOff();
            }
            drawControlPanel();
            delay(400);
            return;
        }
    } else {
        // Screen tapped: toggle OSD or awaken TV
        if (!tvPoweredOn) {
            tvPoweredOn = true;
            animatePowerOn();
            showOSD();
            drawControlPanel();
        } else {
            showOSD();
            // If in DVD channel, give logo a random velocity boost
            if (currentChannel == 3) {
                dvdVx = (random(0, 2) == 0 ? 1 : -1) * (2.0 + random(0, 20) / 10.0);
                dvdVy = (random(0, 2) == 0 ? 1 : -1) * (1.5 + random(0, 20) / 10.0);
                playChirp();
            }
        }
        delay(200);
    }
}

// =========================================================================
//  SETUP & MAIN LOOP
// =========================================================================
void setup() {
    Serial.begin(115200);
    delay(200);

    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LDR_PIN, INPUT);

    setTVLed(0, 0, 0);

    // Init SPI and Display
    tft.init();
    tft.setRotation(1); // Landscape 320x240
    tft.fillScreen(C_BLACK);

    // Init Touch
    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    touch.begin(touchSPI);
    touch.setRotation(1);

    // Boot Sequence
    tft.setTextColor(C_YELLOW, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech TRINITRON", 160, 90, 4);
    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("Retro Desktop Television Player", 160, 130, 2);
    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString("Initializing NTSC Demodulator...", 160, 160, 1);

    playChirp();
    delay(1000);

    tft.fillScreen(C_BLACK);
    drawCRTBezel();
    drawControlPanel();
    initMatrix();
    showOSD();

    setTVLed(0, 180, 200);
}

void loop() {
    handleTouch();

    if (tvPoweredOn) {
        // Render Active Channel at ~30 FPS
        unsigned long now = millis();
        if (now - lastFrameTime >= 33) {
            lastFrameTime = now;

            switch (currentChannel) {
                case 0: renderChannelSMPTE(); break;
                case 1: renderChannelStatic(); break;
                case 2: renderChannelSynthwave(); break;
                case 3: renderChannelDVD(); break;
                case 4: renderChannelMatrix(); break;
                case 5: renderChannelOscilloscope(); break;
            }

            applyScanlines();
            drawOSD();
        }
    } else {
        // Low power standby mode: check LDR or gentle red LED pulse
        int pulse = (millis() / 20) % 256;
        int bright = (pulse < 128) ? pulse : 255 - pulse;
        setTVLed(bright / 2, 0, 0);
        delay(30);
    }
}


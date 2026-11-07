// =========================================================================
//  Project 55 — CYD PeanutGB Classic Game Boy Handheld
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
#define C_CHASSIS     0xC618  // Classic Game Boy warm off-white / light grey
#define C_BEZEL_DARK  0x4208  // Dark grey screen lens bezel
#define C_DPAD        0x2104  // Dark matte black D-Pad
#define C_BUTTON_A    0x9806  // Burgundy / Magenta button A & B
#define C_BUTTON_B    0x9806
#define C_BUTTON_LBL  0x0010  // Dark navy button label text
#define C_SELECT_BTN  0x7BEF  // Slanted rubber grey buttons
#define C_ACCENT_BLUE 0x001F
#define C_ACCENT_MAG  0xF81F

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// =========================================================================
//  LCD GEOMETRY & COLOR PALETTES
// =========================================================================
// Native Game Boy Screen: 160 x 144 pixels centered
const int GB_X = 80;
const int GB_Y = 24;
const int GB_W = 160;
const int GB_H = 144;

enum PaletteMode {
    PAL_DMG_GREEN,
    PAL_POCKET_GRAY,
    PAL_CYBER_NEON,
    PAL_SOLAR_AMBER
};

int currentPalette = PAL_DMG_GREEN;

// 4 shades per palette (0: Lightest/BG, 1: Light, 2: Dark, 3: Darkest)
const uint16_t PALETTES[4][4] = {
    // DMG Original Pea Soup
    { 0x9FE6, 0x8682, 0x3341, 0x0980 },
    // Pocket Silver Monochrome
    { 0xFFFF, 0xBDD7, 0x6B4D, 0x0000 },
    // Cyberpunk Neon
    { 0x07FF, 0x0210, 0xFD14, 0x0802 },
    // Solar Amber
    { 0xFFE0, 0xFCC0, 0x9340, 0x2100 }
};

#define COL_0 (PALETTES[currentPalette][0])
#define COL_1 (PALETTES[currentPalette][1])
#define COL_2 (PALETTES[currentPalette][2])
#define COL_3 (PALETTES[currentPalette][3])

// Handheld State
enum GameSelection {
    GAME_BRICK_BREAKER,
    GAME_PIXEL_RUNNER,
    GAME_CHIPTUNE_SYNTH
};

int currentGame = GAME_BRICK_BREAKER;
bool soundEnabled = true;
int fpsCounter = 60;
unsigned long lastFpsUpdate = 0;
int frameCount = 0;

// Button States
bool btnUp = false;
bool btnDown = false;
bool btnLeft = false;
bool btnRight = false;
bool btnA = false;
bool btnB = false;
bool btnSelect = false;
bool btnStart = false;

// =========================================================================
//  GAME 1: BRICK BREAKER STATE
// =========================================================================
float paddleX = 60.0;
const int PADDLE_W = 28;
const int PADDLE_H = 5;

float ballX = 80.0;
float ballY = 90.0;
float ballVx = 1.6;
float ballVy = -1.8;
const int BALL_SIZE = 3;

const int BRICK_ROWS = 4;
const int BRICK_COLS = 8;
const int BRICK_W = 18;
const int BRICK_H = 6;
bool bricks[BRICK_ROWS][BRICK_COLS];
int brickScore = 0;
int brickLives = 3;
bool brickGameOver = false;

// =========================================================================
//  GAME 2: PIXEL RUNNER STATE
// =========================================================================
float runnerY = 70.0;
float runnerVy = 0.0;
const float GRAVITY = 0.45;
const float JUMP_IMPULSE = -4.5;
float pipeX = 160.0;
int pipeGapY = 60;
const int PIPE_GAP_H = 46;
const int PIPE_W = 14;
int runnerScore = 0;
int runnerHighScore = 0;
bool runnerDead = false;

// =========================================================================
//  SOUND & LED
// =========================================================================
void playChirp(int freq, int ms) {
    if (!soundEnabled) return;
    tone(BUZZER_PIN, freq, ms);
}

void setGBLed(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
}

// =========================================================================
//  AUTHENTIC GAME BOY STARTUP SEQUENCE
// =========================================================================
void runBootSequence() {
    tft.fillScreen(C_CHASSIS);
    
    // Draw LCD Bezel
    tft.fillRoundRect(GB_X - 6, GB_Y - 6, GB_W + 12, GB_H + 12, 6, C_BEZEL_DARK);
    tft.fillRect(GB_X, GB_Y, GB_W, GB_H, COL_0);

    // Power LED (red on bezel)
    tft.fillCircle(GB_X - 18, GB_Y + 16, 3, C_BLACK);
    tft.fillCircle(GB_X - 18, GB_Y + 16, 2, 0xF800); // Red
    tft.setTextColor(C_WHITE, C_BEZEL_DARK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("BATTERY", GB_X - 18, GB_Y + 28, 1);

    // Scrolling IamNotIntoTech logo
    for (int y = -20; y <= 60; y += 4) {
        tft.fillRect(GB_X, GB_Y, GB_W, GB_H, COL_0);
        tft.setTextColor(COL_3, COL_0);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("IamNotIntoTech", GB_X + (GB_W / 2), GB_Y + y, 2);
        delay(35);
    }

    // Trademark (R) badge appears
    tft.drawString("(R)", GB_X + (GB_W / 2) + 68, GB_Y + 56, 1);

    // Authentic two-tone Game Boy boot chime!
    if (soundEnabled) {
        tone(BUZZER_PIN, 1046, 120); // C6
        delay(130);
        tone(BUZZER_PIN, 2093, 280); // C7
        delay(300);
    }

    setGBLed(0, 200, 50); // Power Good Green
    delay(500);
}

// =========================================================================
//  HANDHELD BEZEL & ON-SCREEN CONTROLS
// =========================================================================
void drawHandheldBezel() {
    tft.fillScreen(C_CHASSIS);

    // Top Brand Stripes & Logo
    tft.drawFastHLine(0, 6, 320, C_ACCENT_BLUE);
    tft.drawFastHLine(0, 8, 320, C_ACCENT_MAG);
    tft.setTextColor(0x0010, C_CHASSIS);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("IamNotIntoTech", 80, 10, 2);
    tft.setTextColor(0x4208, C_CHASSIS);
    tft.drawString("GAME BOY", 184, 11, 2);

    // Outer LCD Bezel
    tft.fillRoundRect(GB_X - 6, GB_Y - 6, GB_W + 12, GB_H + 12, 6, C_BEZEL_DARK);
    tft.drawRoundRect(GB_X - 6, GB_Y - 6, GB_W + 12, GB_H + 12, 6, C_BLACK);
    
    // Battery LED
    tft.fillCircle(GB_X - 22, GB_Y + 20, 3, 0xF800);
    tft.setTextColor(C_WHITE, C_CHASSIS);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("BATT", GB_X - 22, GB_Y + 30, 1);

    // Left D-Pad Controls (Center: X=38, Y=96)
    int dcx = 38;
    int dcy = 96;
    // Cross Background
    tft.fillRect(dcx - 10, dcy - 30, 20, 60, C_DPAD);
    tft.fillRect(dcx - 30, dcy - 10, 60, 20, C_DPAD);
    tft.fillCircle(dcx, dcy, 6, 0x10A2); // Center pivot dimple
    // Arrow markers
    tft.setTextColor(0x632C, C_DPAD);
    tft.drawString("^", dcx, dcy - 20, 2);
    tft.drawString("v", dcx, dcy + 20, 2);
    tft.drawString("<", dcx - 20, dcy, 2);
    tft.drawString(">", dcx + 20, dcy, 2);

    // Right Action Buttons (A and B)
    // Button B (X=260, Y=110)
    tft.fillCircle(260, 110, 14, C_BUTTON_B);
    tft.drawCircle(260, 110, 14, C_BLACK);
    tft.setTextColor(C_WHITE, C_BUTTON_B);
    tft.drawString("B", 260, 110, 2);

    // Button A (X=296, Y=86)
    tft.fillCircle(296, 86, 14, C_BUTTON_A);
    tft.drawCircle(296, 86, 14, C_BLACK);
    tft.setTextColor(C_WHITE, C_BUTTON_A);
    tft.drawString("A", 296, 86, 2);

    // Bottom Select & Start Buttons
    // SELECT (X=115, Y=214)
    tft.fillRoundRect(95, 212, 32, 12, 4, C_SELECT_BTN);
    tft.drawRoundRect(95, 212, 32, 12, 4, C_BLACK);
    tft.setTextColor(C_BUTTON_LBL, C_CHASSIS);
    tft.drawString("SELECT", 111, 230, 1);

    // START (X=165, Y=214)
    tft.fillRoundRect(150, 212, 32, 12, 4, C_SELECT_BTN);
    tft.drawRoundRect(150, 212, 32, 12, 4, C_BLACK);
    tft.drawString("START", 166, 230, 1);

    // Bottom System Bar (Palette, Game Switcher, Sound)
    tft.fillRoundRect(6, 185, 66, 20, 4, 0x8410);
    tft.drawRoundRect(6, 185, 66, 20, 4, C_BLACK);
    tft.setTextColor(C_WHITE, 0x8410);
    tft.drawString("PALETTE", 39, 195, 1);

    tft.fillRoundRect(248, 185, 66, 20, 4, 0x8410);
    tft.drawRoundRect(248, 185, 66, 20, 4, C_BLACK);
    tft.drawString("GAME", 281, 195, 1);

    tft.fillRoundRect(248, 212, 66, 20, 4, soundEnabled ? 0x03E0 : 0x7800);
    tft.drawString(soundEnabled ? "SFX: ON" : "SFX: OFF", 281, 222, 1);
}

// =========================================================================
//  GAME 1: SPACE BRICK BREAKER
// =========================================================================
void resetBrickBreaker() {
    brickScore = 0;
    brickLives = 3;
    brickGameOver = false;
    paddleX = (GB_W - PADDLE_W) / 2.0;
    ballX = GB_W / 2.0;
    ballY = GB_H - 30.0;
    ballVx = 1.6;
    ballVy = -1.8;

    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            bricks[r][c] = true;
        }
    }
}

void updateAndRenderBrickBreaker() {
    // Clear GB screen
    tft.fillRect(GB_X, GB_Y, GB_W, GB_H, COL_0);

    // Input Handling
    if (btnLeft && paddleX > 2) paddleX -= 3.5;
    if (btnRight && paddleX < GB_W - PADDLE_W - 2) paddleX += 3.5;

    if (!brickGameOver) {
        ballX += ballVx;
        ballY += ballVy;

        // Wall collisions
        if (ballX <= 2) { ballX = 2; ballVx = -ballVx; playChirp(800, 15); }
        if (ballX >= GB_W - BALL_SIZE - 2) { ballX = GB_W - BALL_SIZE - 2; ballVx = -ballVx; playChirp(800, 15); }
        if (ballY <= 16) { ballY = 16; ballVy = -ballVy; playChirp(800, 15); }

        // Paddle Collision
        if (ballY + BALL_SIZE >= GB_H - 12 && ballY + BALL_SIZE <= GB_H - 8) {
            if (ballX + BALL_SIZE >= paddleX && ballX <= paddleX + PADDLE_W) {
                ballVy = -abs(ballVy);
                float hitOffset = (ballX + (BALL_SIZE / 2.0)) - (paddleX + (PADDLE_W / 2.0));
                ballVx = hitOffset * 0.18;
                playChirp(1200, 20);
            }
        }

        // Bottom Drain Collision
        if (ballY > GB_H) {
            brickLives--;
            playChirp(300, 100);
            if (brickLives <= 0) {
                brickGameOver = true;
            } else {
                ballX = paddleX + (PADDLE_W / 2);
                ballY = GB_H - 25;
                ballVy = -1.8;
                ballVx = 1.4;
            }
        }

        // Brick Collisions
        int activeBricks = 0;
        for (int r = 0; r < BRICK_ROWS; r++) {
            for (int c = 0; c < BRICK_COLS; c++) {
                if (bricks[r][c]) {
                    activeBricks++;
                    int bx = 8 + (c * (BRICK_W + 1));
                    int by = 22 + (r * (BRICK_H + 2));

                    if (ballX + BALL_SIZE >= bx && ballX <= bx + BRICK_W &&
                        ballY + BALL_SIZE >= by && ballY <= by + BRICK_H) {
                        bricks[r][c] = false;
                        ballVy = -ballVy;
                        brickScore += 10;
                        playChirp(1400 + (r * 150), 20);
                    }
                }
            }
        }

        // Win check
        if (activeBricks == 0) {
            resetBrickBreaker();
            playChirp(2000, 150);
        }
    }

    // Draw Bricks
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            if (bricks[r][c]) {
                int bx = GB_X + 8 + (c * (BRICK_W + 1));
                int by = GB_Y + 22 + (r * (BRICK_H + 2));
                uint16_t bCol = (r % 2 == 0) ? COL_3 : COL_2;
                tft.fillRect(bx, by, BRICK_W, BRICK_H, bCol);
                tft.drawRect(bx, by, BRICK_W, BRICK_H, COL_1);
            }
        }
    }

    // Draw Paddle
    tft.fillRect(GB_X + (int)paddleX, GB_Y + GB_H - 12, PADDLE_W, PADDLE_H, COL_3);
    tft.fillRect(GB_X + (int)paddleX + 1, GB_Y + GB_H - 11, PADDLE_W - 2, PADDLE_H - 2, COL_2);

    // Draw Ball
    tft.fillRect(GB_X + (int)ballX, GB_Y + (int)ballY, BALL_SIZE, BALL_SIZE, COL_3);

    // Draw Top HUD inside GB Screen
    tft.drawFastHLine(GB_X, GB_Y + 14, GB_W, COL_2);
    tft.setTextColor(COL_3, COL_0);
    tft.setTextDatum(TL_DATUM);
    char scoreBuf[24];
    snprintf(scoreBuf, sizeof(scoreBuf), "SCORE:%04d", brickScore);
    tft.drawString(scoreBuf, GB_X + 4, GB_Y + 3, 1);

    tft.setTextDatum(TR_DATUM);
    char livesBuf[16];
    snprintf(livesBuf, sizeof(livesBuf), "LIVES:%d", brickLives);
    tft.drawString(livesBuf, GB_X + GB_W - 4, GB_Y + 3, 1);

    if (brickGameOver) {
        tft.fillRect(GB_X + 25, GB_Y + 65, GB_W - 50, 28, COL_3);
        tft.setTextColor(COL_0, COL_3);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("GAME OVER", GB_X + (GB_W / 2), GB_Y + 74, 2);
        tft.drawString("TAP 'A' TO RETRY", GB_X + (GB_W / 2), GB_Y + 86, 1);

        if (btnA || btnStart) {
            resetBrickBreaker();
            delay(200);
        }
    }
}

// =========================================================================
//  GAME 2: PIXEL RUNNER (FLAPPY PIXEL)
// =========================================================================
void resetPixelRunner() {
    runnerY = 60.0;
    runnerVy = 0.0;
    pipeX = GB_W + 10;
    pipeGapY = random(30, 85);
    runnerScore = 0;
    runnerDead = false;
}

void updateAndRenderPixelRunner() {
    tft.fillRect(GB_X, GB_Y, GB_W, GB_H, COL_0);

    // Ground plane
    int groundY = GB_H - 16;
    tft.fillRect(GB_X, GB_Y + groundY, GB_W, 16, COL_2);
    tft.drawFastHLine(GB_X, GB_Y + groundY, GB_W, COL_3);

    // Background mountains
    for (int x = 0; x < GB_W; x += 32) {
        tft.drawLine(GB_X + x, GB_Y + groundY, GB_X + x + 16, GB_Y + groundY - 14, COL_1);
        tft.drawLine(GB_X + x + 16, GB_Y + groundY - 14, GB_X + x + 32, GB_Y + groundY, COL_1);
    }

    if (!runnerDead) {
        // Handle Flap / Jump
        if (btnA || btnUp) {
            runnerVy = JUMP_IMPULSE;
            playChirp(1400, 20);
        }

        // Apply physics
        runnerVy += GRAVITY;
        runnerY += runnerVy;

        // Pipe Motion
        pipeX -= 2.2;
        if (pipeX + PIPE_W < 0) {
            pipeX = GB_W;
            pipeGapY = random(30, 80);
            runnerScore++;
            if (runnerScore > runnerHighScore) runnerHighScore = runnerScore;
            playChirp(1800, 25);
        }

        // Collision Checks
        int rx = 36;
        int ry = (int)runnerY;
        const int R_SIZE = 8;

        // Ground collision
        if (ry + R_SIZE >= groundY) {
            runnerDead = true;
            playChirp(250, 150);
        }

        // Pipe collisions
        if (rx + R_SIZE >= (int)pipeX && rx <= (int)pipeX + PIPE_W) {
            if (ry < pipeGapY || ry + R_SIZE > pipeGapY + PIPE_GAP_H) {
                runnerDead = true;
                playChirp(250, 150);
            }
        }
    }

    // Draw Obstacle Pipes
    int px = GB_X + (int)pipeX;
    if (px < GB_X + GB_W && px + PIPE_W > GB_X) {
        // Upper pipe
        tft.fillRect(px, GB_Y, PIPE_W, pipeGapY, COL_3);
        tft.fillRect(px - 2, GB_Y + pipeGapY - 6, PIPE_W + 4, 6, COL_3);
        // Lower pipe
        int botPipeY = pipeGapY + PIPE_GAP_H;
        int botPipeH = groundY - botPipeY;
        if (botPipeH > 0) {
            tft.fillRect(px, GB_Y + botPipeY, PIPE_W, botPipeH, COL_3);
            tft.fillRect(px - 2, GB_Y + botPipeY, PIPE_W + 4, 6, COL_3);
        }
    }

    // Draw Pixel Character (Flying Bird / Sprite)
    int srx = GB_X + 36;
    int sry = GB_Y + (int)runnerY;
    tft.fillRect(srx, sry, 8, 8, COL_3);
    tft.fillRect(srx + 2, sry + 2, 4, 4, COL_2);
    tft.fillRect(srx + 5, sry + 1, 2, 2, COL_0); // Eye
    tft.fillRect(srx + 7, sry + 3, 3, 2, COL_3); // Beak

    // Score HUD
    tft.setTextColor(COL_3, COL_0);
    tft.setTextDatum(TC_DATUM);
    char scBuf[16];
    snprintf(scBuf, sizeof(scBuf), "%02d", runnerScore);
    tft.drawString(scBuf, GB_X + (GB_W / 2), GB_Y + 6, 2);

    if (runnerDead) {
        tft.fillRect(GB_X + 20, GB_Y + 45, GB_W - 40, 42, COL_3);
        tft.setTextColor(COL_0, COL_3);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("CRASHED!", GB_X + (GB_W / 2), GB_Y + 56, 2);
        char hsBuf[24];
        snprintf(hsBuf, sizeof(hsBuf), "BEST: %02d", runnerHighScore);
        tft.drawString(hsBuf, GB_X + (GB_W / 2), GB_Y + 70, 1);
        tft.drawString("PRESS 'A'", GB_X + (GB_W / 2), GB_Y + 80, 1);

        if (btnA || btnStart) {
            resetPixelRunner();
            delay(200);
        }
    }
}

// =========================================================================
//  GAME 3: CHIPTUNE SOUND SYNTHESIZER
// =========================================================================
int currentNote = -1;
const int NOTES[8] = { 261, 293, 329, 349, 392, 440, 493, 523 }; // C4..C5
const char* NOTE_NAMES[8] = { "C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5" };

void updateAndRenderChiptuneSynth() {
    tft.fillRect(GB_X, GB_Y, GB_W, GB_H, COL_0);

    tft.setTextColor(COL_3, COL_0);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("CHIPTUNE SYNTH", GB_X + (GB_W / 2), GB_Y + 8, 2);
    tft.drawString("GB 4-VOICE PSG", GB_X + (GB_W / 2), GB_Y + 24, 1);

    // Draw Piano Keys
    int keyW = GB_W / 8;
    int keyH = 65;
    int keyY = GB_Y + 42;

    for (int k = 0; k < 8; k++) {
        int kx = GB_X + (k * keyW);
        uint16_t kBg = (currentNote == k) ? COL_3 : COL_1;
        uint16_t kFg = (currentNote == k) ? COL_0 : COL_3;
        tft.fillRect(kx, keyY, keyW - 1, keyH, kBg);
        tft.drawRect(kx, keyY, keyW - 1, keyH, COL_3);
        tft.setTextColor(kFg, kBg);
        tft.setTextDatum(BC_DATUM);
        tft.drawString(NOTE_NAMES[k], kx + (keyW / 2), keyY + keyH - 4, 1);
    }

    // Active Note Visualizer Waveform
    int waveY = GB_Y + GB_H - 18;
    tft.drawFastHLine(GB_X, waveY, GB_W, COL_2);
    if (currentNote >= 0) {
        for (int x = 0; x < GB_W; x += 6) {
            int h = ((x / 6) % 2 == 0) ? -8 : 8;
            tft.drawFastHLine(GB_X + x, waveY + h, 6, COL_3);
            tft.drawFastVLine(GB_X + x, waveY - 8, 16, COL_3);
        }
    }

    // Touch key detection
    if (btnLeft && currentNote > 0) { currentNote--; playChirp(NOTES[currentNote], 100); }
    if (btnRight && currentNote < 7) { currentNote++; playChirp(NOTES[currentNote], 100); }
}

// =========================================================================
//  TOUCH INPUT PROCESSING
// =========================================================================
void processTouch() {
    btnUp = btnDown = btnLeft = btnRight = btnA = btnB = btnSelect = btnStart = false;

    if (!touch.touched()) return;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, TS_MIN_X, TS_MAX_X, 0, 320);
    int ty = map(p.y, TS_MIN_Y, TS_MAX_Y, 0, 240);
    tx = constrain(tx, 0, 319);
    ty = constrain(ty, 0, 239);

    // 1. Left D-Pad (Center: X=38, Y=96)
    if (tx >= 6 && tx <= 72 && ty >= 64 && ty <= 130) {
        if (ty < 84) btnUp = true;
        else if (ty > 108) btnDown = true;
        else if (tx < 30) btnLeft = true;
        else if (tx > 46) btnRight = true;
        return;
    }

    // 2. Right Action Buttons
    // Button A (Center: 296, 86, Radius 16)
    if (abs(tx - 296) < 22 && abs(ty - 86) < 22) {
        btnA = true;
        return;
    }
    // Button B (Center: 260, 110, Radius 16)
    if (abs(tx - 260) < 22 && abs(ty - 110) < 22) {
        btnB = true;
        return;
    }

    // 3. Bottom Controls
    // SELECT (X: 95..127, Y: 210..235)
    if (tx >= 90 && tx <= 130 && ty >= 205 && ty <= 238) {
        btnSelect = true;
        return;
    }
    // START (X: 145..185, Y: 210..235)
    if (tx >= 145 && tx <= 185 && ty >= 205 && ty <= 238) {
        btnStart = true;
        return;
    }

    // 4. System Quick Toggles
    // Palette Switcher (X: 6..72, Y: 180..210)
    if (tx >= 6 && tx <= 72 && ty >= 180 && ty <= 210) {
        currentPalette = (currentPalette + 1) % 4;
        playChirp(1500, 30);
        delay(250);
        return;
    }

    // Game Switcher (X: 248..314, Y: 180..210)
    if (tx >= 248 && tx <= 314 && ty >= 180 && ty <= 210) {
        currentGame = (currentGame + 1) % 3;
        if (currentGame == GAME_BRICK_BREAKER) resetBrickBreaker();
        if (currentGame == GAME_PIXEL_RUNNER) resetPixelRunner();
        playChirp(1200, 40);
        delay(250);
        return;
    }

    // Sound Switcher (X: 248..314, Y: 212..238)
    if (tx >= 248 && tx <= 314 && ty >= 212 && ty <= 238) {
        soundEnabled = !soundEnabled;
        drawHandheldBezel();
        delay(250);
        return;
    }

    // Direct Screen Tap: Trigger A or Game interaction
    if (tx >= GB_X && tx <= GB_X + GB_W && ty >= GB_Y && ty <= GB_Y + GB_H) {
        if (currentGame == GAME_CHIPTUNE_SYNTH) {
            int keyW = GB_W / 8;
            int k = (tx - GB_X) / keyW;
            if (k >= 0 && k < 8) {
                currentNote = k;
                playChirp(NOTES[k], 80);
            }
        } else {
            btnA = true;
        }
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

    setGBLed(0, 0, 0);

    // Init Display & Touch
    tft.init();
    tft.setRotation(1); // Landscape 320x240
    tft.fillScreen(C_BLACK);

    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    touch.begin(touchSPI);
    touch.setRotation(1);

    // Run Nostalgic Boot Sequence
    runBootSequence();

    // Draw Bezel
    drawHandheldBezel();
    resetBrickBreaker();
    resetPixelRunner();
}

void loop() {
    processTouch();

    switch (currentGame) {
        case GAME_BRICK_BREAKER:
            updateAndRenderBrickBreaker();
            break;
        case GAME_PIXEL_RUNNER:
            updateAndRenderPixelRunner();
            break;
        case GAME_CHIPTUNE_SYNTH:
            updateAndRenderChiptuneSynth();
            break;
    }

    // FPS Meter
    frameCount++;
    if (millis() - lastFpsUpdate >= 1000) {
        fpsCounter = frameCount;
        frameCount = 0;
        lastFpsUpdate = millis();
    }

    delay(20);
}


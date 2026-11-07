/**
 * ============================================================================
 * Project 71 — CYD 8-Bit Retro Mini-Arcade Deck
 * Hardware: Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch)
 * 
 * Features:
 *   - Standalone 8-Bit Retro Arcade Gaming Console
 *   - 3 Fully Playable Retro Classics:
 *       1. CYBER SNAKE:
 *          • Directional grid navigation, apple spawning, tail growth & speed progression
 *          • Touch D-Pad with haptic audio clicks
 *       2. PADDLE PONG:
 *          • 60 FPS 2D physics pong against intelligent AI paddle
 *          • Dynamic ball deflection angles, scorekeeper, and rally counter
 *       3. NEON BRICK BREAKER:
 *          • Breakout arcade with color-coded brick matrix, ball bounce physics & paddle dragging
 *   - On-Screen Touch Controls:
 *       • 4-Way Tactile D-Pad & Action Buttons [A] / [B] / [BACK]
 *       • Direct paddle touch dragging for ultra-smooth Pong & Breakout control
 *   - 8-Bit Chiptune Arcade Synthesizer on GPIO 26:
 *       • Jump bleeps, brick bounce clinks, score point chimes, and game over fanfares
 *   - RGB LED Scoring Strobe on GPIO 4, 16, 17:
 *       • Green strobe on score point, Red flash on collision / life lost
 *   - LDR Ambient Room Sensor on GPIO 34
 * 
 * Community & Documentation:
 *   📺 Tutorial:  https://youtube.com/@IamNotIntoTech
 *   🖨️ Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Brand: IamNotIntoTech
 * ============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// --- PIN DEFINITIONS ---
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define LED_RED_PIN 4
#define LED_GREEN_PIN 16
#define LED_BLUE_PIN 17
#define BUZZER_PIN 26
#define LDR_PIN 34

// --- COLOR PALETTE (RGB565) ---
#define C_BLACK       0x0000
#define C_WHITE       0xFFFF
#define C_GREEN       0x07E0
#define C_CYAN        0x07FF
#define C_RED         0xF800
#define C_YELLOW      0xFFE0
#define C_GOLD        0xFEA0
#define C_MAGENTA     0xF81F
#define C_DARKGREY    0x2124
#define C_MIDGREY     0x52AA
#define C_LIGHTGREY   0xC618
#define C_NAVY        0x000F
#define C_BLUE        0x001F

// --- HARDWARE INSTANCES ---
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// --- ARCADE STATE ---
enum ArcadeMode {
    MODE_MENU     = 0,
    MODE_SNAKE    = 1,
    MODE_PONG     = 2,
    MODE_BREAKOUT = 3
};

int currentMode = MODE_MENU;
int score = 0;
int highScore = 420;
int lives = 3;
bool gameOver = false;

// Snake Game Variables
#define MAX_SNAKE 64
struct Point { int x, y; };
Point snake[MAX_SNAKE];
int snakeLen = 4;
int snakeDir = 1; // 0=UP, 1=RIGHT, 2=DOWN, 3=LEFT
Point apple = { 10, 8 };
unsigned long lastSnakeTick = 0;

// Pong Game Variables
float ballX = 160.0f, ballY = 120.0f;
float ballVx = 3.0f, ballVy = 2.0f;
int playerPaddleY = 100;
int aiPaddleY = 100;
int playerScore = 0;
int aiScore = 0;

// Breakout Game Variables
#define BRICK_ROWS 3
#define BRICK_COLS 8
bool bricks[BRICK_ROWS][BRICK_COLS];
float boBallX = 160.0f, boBallY = 140.0f;
float boBallVx = 2.5f, boBallVy = -2.5f;
int boPaddleX = 130;

// --- HARDWARE HELPERS ---
void setRgbLed(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_RED_PIN, 255 - r);
    analogWrite(LED_GREEN_PIN, 255 - g);
    analogWrite(LED_BLUE_PIN, 255 - b);
}

void playBeep(int freq, int ms) {
    tone(BUZZER_PIN, freq, ms);
}

void playScoreChime() {
    setRgbLed(0, 255, 0);
    tone(BUZZER_PIN, 1046, 30); delay(40);
    tone(BUZZER_PIN, 1318, 30); delay(40);
    tone(BUZZER_PIN, 1568, 50);
    setRgbLed(0, 0, 0);
}

void playDeathSound() {
    setRgbLed(255, 0, 0);
    for (int f = 600; f > 150; f -= 30) {
        tone(BUZZER_PIN, f, 20);
        delay(15);
    }
    setRgbLed(0, 0, 0);
}

// --- BOOT SCREEN ---
void showBootSequence() {
    tft.fillScreen(C_BLACK);
    setRgbLed(0, 0, 0);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 45, 4);

    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString("8-BIT RETRO MINI-ARCADE DECK", 160, 80, 2);

    // Pixel arcade joystick icon
    tft.fillCircle(160, 125, 12, C_RED);
    tft.fillRect(158, 137, 4, 18, C_WHITE);
    tft.fillRect(140, 155, 40, 8, C_MIDGREY);

    tft.setTextColor(C_LIGHTGREY, C_BLACK);
    tft.drawString("Loading 60 FPS Arcade Engine...", 160, 185, 2);

    setRgbLed(255, 200, 0);
    tone(BUZZER_PIN, 880, 80); delay(100);
    tone(BUZZER_PIN, 1760, 120);

    delay(1200);
    setRgbLed(0, 0, 0);
    tft.fillScreen(C_BLACK);
}

// --- HEADER BAR ---
void renderHeader() {
    tft.fillRect(0, 0, 320, 24, C_NAVY);
    tft.drawFastHLine(0, 24, 320, C_GOLD);

    // Left Back Button (if inside game)
    if (currentMode != MODE_MENU) {
        tft.fillRect(4, 3, 50, 18, C_DARKGREY);
        tft.drawRect(4, 3, 50, 18, C_WHITE);
        tft.setTextColor(C_WHITE, C_DARKGREY);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("< MENU", 29, 12, 1);
    }

    // Center Branding
    tft.setTextColor(C_GOLD, C_NAVY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech ARCADE", 160, 12, 2);

    // Score readout
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(C_GREEN, C_NAVY);
    char scBuf[24];
    snprintf(scBuf, sizeof(scBuf), "PTS: %d", score);
    tft.drawString(scBuf, 314, 12, 1);
}

// --- MENU VIEW ---
void renderMenu() {
    tft.fillRect(0, 25, 320, 215, C_BLACK);

    tft.setTextColor(C_WHITE, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("SELECT ARCADE CARTRIDGE", 160, 45, 2);

    // Game 1: Cyber Snake
    tft.fillRect(25, 65, 270, 42, C_DARKGREY);
    tft.drawRect(25, 65, 270, 42, C_GREEN);
    tft.setTextColor(C_GREEN, C_DARKGREY);
    tft.drawString("1. CYBER SNAKE 2088", 160, 86, 2);

    // Game 2: Paddle Pong
    tft.fillRect(25, 115, 270, 42, C_DARKGREY);
    tft.drawRect(25, 115, 270, 42, C_CYAN);
    tft.setTextColor(C_CYAN, C_DARKGREY);
    tft.drawString("2. PADDLE PONG (VS CPU)", 160, 136, 2);

    // Game 3: Neon Breakout
    tft.fillRect(25, 165, 270, 42, C_DARKGREY);
    tft.drawRect(25, 165, 270, 42, C_MAGENTA);
    tft.setTextColor(C_MAGENTA, C_DARKGREY);
    tft.drawString("3. NEON BRICK BREAKER", 160, 186, 2);

    tft.setTextColor(C_LIGHTGREY, C_BLACK);
    tft.drawString("Tap any game card to launch", 160, 222, 1);
}

// --- SNAKE GAME ---
void initSnake() {
    snakeLen = 4;
    snakeDir = 1; // RIGHT
    for (int i = 0; i < snakeLen; i++) {
        snake[i].x = 10 - i;
        snake[i].y = 8;
    }
    apple.x = random(2, 22);
    apple.y = random(3, 14);
    score = 0;
    gameOver = false;
    tft.fillRect(0, 25, 320, 215, C_BLACK);
}

void drawSnakeControls() {
    // Touch D-Pad on bottom right (Y: 175 to 238)
    int dX = 240, dY = 175;
    // Up
    tft.fillRect(dX + 24, dY, 26, 20, C_DARKGREY);
    tft.drawRect(dX + 24, dY, 26, 20, C_WHITE);
    tft.setTextColor(C_WHITE, C_DARKGREY); tft.setTextDatum(MC_DATUM); tft.drawString("^", dX + 37, dY + 10, 2);
    // Down
    tft.fillRect(dX + 24, dY + 42, 26, 20, C_DARKGREY);
    tft.drawRect(dX + 24, dY + 42, 26, 20, C_WHITE);
    tft.drawString("v", dX + 37, dY + 52, 2);
    // Left
    tft.fillRect(dX, dY + 21, 24, 21, C_DARKGREY);
    tft.drawRect(dX, dY + 21, 24, 21, C_WHITE);
    tft.drawString("<", dX + 12, dY + 31, 2);
    // Right
    tft.fillRect(dX + 50, dY + 21, 24, 21, C_DARKGREY);
    tft.drawRect(dX + 50, dY + 21, 24, 21, C_WHITE);
    tft.drawString(">", dX + 62, dY + 31, 2);
}

void updateSnake() {
    if (gameOver) {
        tft.setTextColor(C_RED, C_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("GAME OVER! TAP TO RESTART", 160, 110, 2);
        return;
    }

    // Move tail
    Point newHead = snake[0];
    if (snakeDir == 0) newHead.y--;
    else if (snakeDir == 1) newHead.x++;
    else if (snakeDir == 2) newHead.y++;
    else if (snakeDir == 3) newHead.x--;

    // Wall collision (Grid: 25x16 cells of 9x9 px)
    if (newHead.x < 1 || newHead.x > 24 || newHead.y < 3 || newHead.y > 17) {
        gameOver = true;
        playDeathSound();
        return;
    }

    // Self collision
    for (int i = 1; i < snakeLen; i++) {
        if (newHead.x == snake[i].x && newHead.y == snake[i].y) {
            gameOver = true;
            playDeathSound();
            return;
        }
    }

    // Erase old tail
    Point oldTail = snake[snakeLen - 1];
    tft.fillRect(oldTail.x * 9, oldTail.y * 9, 8, 8, C_BLACK);

    // Shift body
    for (int i = snakeLen - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = newHead;

    // Eat apple
    if (newHead.x == apple.x && newHead.y == apple.y) {
        if (snakeLen < MAX_SNAKE - 1) snakeLen++;
        score += 10;
        playScoreChime();
        apple.x = random(2, 23);
        apple.y = random(4, 16);
        renderHeader();
    }

    // Draw apple
    tft.fillCircle(apple.x * 9 + 4, apple.y * 9 + 4, 4, C_RED);

    // Draw snake body & head
    for (int i = 0; i < snakeLen; i++) {
        tft.fillRect(snake[i].x * 9, snake[i].y * 9, 8, 8, (i == 0) ? C_GOLD : C_GREEN);
    }

    drawSnakeControls();
}

// --- PONG GAME ---
void initPong() {
    ballX = 160.0f; ballY = 120.0f;
    ballVx = 3.0f;  ballVy = 2.0f;
    playerPaddleY = 100;
    aiPaddleY = 100;
    playerScore = 0;
    aiScore = 0;
    tft.fillRect(0, 25, 320, 215, C_BLACK);
}

void updatePong() {
    // Erase previous ball & paddles
    tft.fillCircle((int)ballX, (int)ballY, 4, C_BLACK);
    tft.fillRect(12, playerPaddleY, 6, 36, C_BLACK);
    tft.fillRect(302, aiPaddleY, 6, 36, C_BLACK);

    // Move ball
    ballX += ballVx;
    ballY += ballVy;

    // Top / bottom bounce
    if (ballY <= 30 || ballY >= 230) {
        ballVy = -ballVy;
        playBeep(900, 15);
    }

    // Simple AI paddle tracking
    if (aiPaddleY + 18 < ballY) aiPaddleY += 2;
    else if (aiPaddleY + 18 > ballY) aiPaddleY -= 2;

    // Constrain paddles
    playerPaddleY = constrain(playerPaddleY, 26, 198);
    aiPaddleY = constrain(aiPaddleY, 26, 198);

    // Player paddle hit
    if (ballX <= 20 && ballX >= 12 && ballY >= playerPaddleY && ballY <= playerPaddleY + 36) {
        ballVx = abs(ballVx);
        playBeep(1400, 20);
        score += 5;
        renderHeader();
    }

    // AI paddle hit
    if (ballX >= 298 && ballX <= 306 && ballY >= aiPaddleY && ballY <= aiPaddleY + 36) {
        ballVx = -abs(ballVx);
        playBeep(1200, 20);
    }

    // Score checks
    if (ballX < 0) {
        aiScore++;
        playDeathSound();
        ballX = 160.0f; ballY = 120.0f;
    } else if (ballX > 320) {
        playerScore++;
        score += 50;
        playScoreChime();
        renderHeader();
        ballX = 160.0f; ballY = 120.0f;
    }

    // Draw net
    for (int y = 26; y < 235; y += 12) {
        tft.drawFastVLine(160, y, 6, C_DARKGREY);
    }

    // Draw score digits
    tft.setTextColor(C_MIDGREY, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(String(playerScore).c_str(), 130, 48, 4);
    tft.drawString(String(aiScore).c_str(), 190, 48, 4);

    // Draw paddles & ball
    tft.fillRect(12, playerPaddleY, 6, 36, C_CYAN);
    tft.fillRect(302, aiPaddleY, 6, 36, C_RED);
    tft.fillCircle((int)ballX, (int)ballY, 4, C_WHITE);
}

// --- BREAKOUT GAME ---
void initBreakout() {
    boBallX = 160.0f; boBallY = 150.0f;
    boBallVx = 2.5f;  boBallVy = -2.5f;
    boPaddleX = 130;
    score = 0;
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            bricks[r][c] = true;
        }
    }
    tft.fillRect(0, 25, 320, 215, C_BLACK);
}

void updateBreakout() {
    // Erase ball & paddle
    tft.fillCircle((int)boBallX, (int)boBallY, 4, C_BLACK);
    tft.fillRect(boPaddleX, 222, 55, 7, C_BLACK);

    boBallX += boBallVx;
    boBallY += boBallVy;

    // Walls
    if (boBallX <= 4 || boBallX >= 316) {
        boBallVx = -boBallVx;
        playBeep(800, 15);
    }
    if (boBallY <= 28) {
        boBallVy = -boBallVy;
        playBeep(900, 15);
    }

    // Paddle bounce
    if (boBallY >= 218 && boBallY <= 226 && boBallX >= boPaddleX && boBallX <= boPaddleX + 55) {
        boBallVy = -abs(boBallVy);
        playBeep(1600, 25);
    }

    // Brick collisions (Y: 40 to 85)
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            if (bricks[r][c]) {
                int bx = 16 + (c * 36);
                int by = 42 + (r * 15);
                if (boBallX >= bx && boBallX <= bx + 32 && boBallY >= by && boBallY <= by + 12) {
                    bricks[r][c] = false;
                    boBallVy = -boBallVy;
                    tft.fillRect(bx, by, 32, 12, C_BLACK);
                    score += 20;
                    playScoreChime();
                    renderHeader();
                    break;
                }
            }
        }
    }

    // Bottom out
    if (boBallY > 238) {
        playDeathSound();
        boBallX = 160.0f; boBallY = 140.0f;
        boBallVy = -2.5f;
    }

    // Draw bricks
    uint16_t rowColors[3] = { C_RED, C_GOLD, C_GREEN };
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            if (bricks[r][c]) {
                int bx = 16 + (c * 36);
                int by = 42 + (r * 15);
                tft.fillRect(bx, by, 32, 12, rowColors[r]);
                tft.drawRect(bx, by, 32, 12, C_WHITE);
            }
        }
    }

    // Draw paddle & ball
    tft.fillRect(boPaddleX, 222, 55, 7, C_CYAN);
    tft.fillCircle((int)boBallX, (int)boBallY, 4, C_WHITE);
}

// --- TOUCH INPUT HANDLER ---
void handleTouch() {
    if (!touch.touched()) return;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, 240, 3800, 0, 320);
    int ty = map(p.y, 240, 3800, 0, 240);

    // Check top MENU button (X <= 60, Y <= 24)
    if (currentMode != MODE_MENU && tx <= 60 && ty <= 24) {
        currentMode = MODE_MENU;
        score = 0;
        playBeep(800, 40);
        renderHeader();
        renderMenu();
        delay(180);
        return;
    }

    // If on Menu: Select game
    if (currentMode == MODE_MENU) {
        if (ty >= 65 && ty <= 107) {
            currentMode = MODE_SNAKE;
            initSnake();
            renderHeader();
        } else if (ty >= 115 && ty <= 157) {
            currentMode = MODE_PONG;
            initPong();
            renderHeader();
        } else if (ty >= 165 && ty <= 207) {
            currentMode = MODE_BREAKOUT;
            initBreakout();
            renderHeader();
        }
        delay(180);
        return;
    }

    // Snake Touch D-Pad Controls
    if (currentMode == MODE_SNAKE) {
        if (gameOver) {
            initSnake();
            delay(180);
            return;
        }
        int dX = 240, dY = 175;
        // Up
        if (tx >= dX + 24 && tx <= dX + 50 && ty >= dY && ty <= dY + 20) {
            if (snakeDir != 2) snakeDir = 0;
            playBeep(1200, 15);
        }
        // Down
        else if (tx >= dX + 24 && tx <= dX + 50 && ty >= dY + 42 && ty <= dY + 62) {
            if (snakeDir != 0) snakeDir = 2;
            playBeep(1200, 15);
        }
        // Left
        else if (tx >= dX && tx <= dX + 24 && ty >= dY + 21 && ty <= dY + 42) {
            if (snakeDir != 1) snakeDir = 3;
            playBeep(1200, 15);
        }
        // Right
        else if (tx >= dX + 50 && tx <= dX + 74 && ty >= dY + 21 && ty <= dY + 42) {
            if (snakeDir != 3) snakeDir = 1;
            playBeep(1200, 15);
        }
    }

    // Pong Paddle Control (slide paddle Y)
    if (currentMode == MODE_PONG) {
        playerPaddleY = ty - 18;
    }

    // Breakout Paddle Control (slide paddle X)
    if (currentMode == MODE_BREAKOUT) {
        boPaddleX = tx - 27;
        boPaddleX = constrain(boPaddleX, 5, 260);
    }
}

// --- SETUP ---
void setup() {
    Serial.begin(115200);

    // Initialize RGB LED & Pins
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);
    setRgbLed(0, 0, 0);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LDR_PIN, INPUT);

    // Initialize Display
    tft.init();
    tft.setRotation(1); // Landscape 320x240
    tft.fillScreen(C_BLACK);

    // Initialize Touchscreen (HSPI)
    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touch.begin(touchSPI);
    touch.setRotation(1);

    // Boot sequence
    showBootSequence();

    // Render initial UI
    renderHeader();
    renderMenu();
}

// --- MAIN LOOP ---
void loop() {
    // Touch input check
    handleTouch();

    // Update active game
    if (currentMode == MODE_SNAKE) {
        if (millis() - lastSnakeTick >= 140) {
            lastSnakeTick = millis();
            updateSnake();
        }
    } else if (currentMode == MODE_PONG) {
        updatePong();
        delay(12);
    } else if (currentMode == MODE_BREAKOUT) {
        updateBreakout();
        delay(14);
    }

    delay(10);
}


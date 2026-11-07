// =========================================================================
//  Project 61 — CYD Creator Live Studio Ticker
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
#include <ArduinoJson.h>

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
#define C_BG          0x0821  // Dark graphite slate
#define C_CARD        0x10A2  // Slightly elevated card
#define C_CARD_BORDER 0x2124
#define C_YT_RED      0xF800  // YouTube Red
#define C_GOLD        0xFDC0  // Gold Play Button yellow
#define C_GREEN       0x07E0  // Growth Green
#define C_CYAN        0x07FF
#define C_GRAY        0x73AE
#define C_DARKGRAY    0x2965

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// =========================================================================
//  CREATOR METRICS & SIMULATION
// =========================================================================
unsigned long subscriberCount = 128495;
unsigned long prevSubCount = 128495;
int subsGainedToday = 184;
float totalViewsMillions = 15.82;
int totalVideos = 142;
int viewVelocityPerHour = 940;

// Milestone Goal
unsigned long nextMilestone = 150000;
unsigned long prevMilestone = 100000;

// Latest Video Stats
const char* latestVideoTitle = "ESP32 CYD Ultimate Desk Setup";
int latestVideoRank = 1; // 1 of 10
int latestVideoViewsK = 42;
int latestVideoLikes = 3120;

// Recent Activity
const char* recentSubName = "Alex_Maker";
int recentSubMinutesAgo = 4;

bool chimeEnabled = true;
int refreshCountdownSec = 30;
unsigned long lastSecTick = 0;
unsigned long lastSubTick = 0;

// =========================================================================
//  SOUND & LED OUTPUT
// =========================================================================
void playCelebrationChime() {
    if (!chimeEnabled) return;
    tone(BUZZER_PIN, 1046, 70); // C6
    delay(80);
    tone(BUZZER_PIN, 1318, 70); // E6
    delay(80);
    tone(BUZZER_PIN, 1567, 70); // G6
    delay(80);
    tone(BUZZER_PIN, 2093, 140); // C7
}

void playTouchClick() {
    tone(BUZZER_PIN, 1800, 15);
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
}

// =========================================================================
//  UI DRAWING ROUTINES
// =========================================================================
void drawHeader() {
    tft.fillRect(0, 0, 320, 24, C_BG);

    // YouTube Icon (Red rounded rect with white play triangle)
    tft.fillRoundRect(8, 4, 20, 16, 4, C_YT_RED);
    tft.fillTriangle(15, 8, 15, 16, 22, 12, C_WHITE);

    // Channel Brand
    tft.setTextColor(C_WHITE, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("IamNotIntoTech", 34, 5, 2);

    tft.setTextColor(C_GOLD, C_BG);
    tft.drawString("STUDIO", 144, 5, 2);

    // Live Badge
    tft.fillRoundRect(248, 3, 64, 18, 9, C_CARD);
    tft.drawRoundRect(248, 3, 64, 18, 9, C_CARD_BORDER);
    tft.setTextColor(C_GREEN, C_CARD);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("LIVE SYNC", 280, 12, 1);

    tft.drawFastHLine(0, 24, 320, C_CARD_BORDER);
}

void drawSubscriberCard() {
    int cx = 8;
    int cy = 28;
    int cw = 304;
    int ch = 68;

    tft.fillRoundRect(cx, cy, cw, ch, 6, C_CARD);
    tft.drawRoundRect(cx, cy, cw, ch, 6, C_CARD_BORDER);

    // Header Label & Today Gain
    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("SUBSCRIBERS", cx + 12, cy + 6, 1);

    char gainBuf[24];
    snprintf(gainBuf, sizeof(gainBuf), "+%d today", subsGainedToday);
    tft.setTextColor(C_GREEN, C_CARD);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(gainBuf, cx + cw - 12, cy + 6, 1);

    // Big Odometer Subscriber Count
    char subStr[24];
    snprintf(subStr, sizeof(subStr), "%lu", subscriberCount);

    tft.setTextColor(C_WHITE, C_CARD);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(subStr, cx + (cw / 2), cy + 30, 6);

    // Goal Progress Bar (Y: cy + 50)
    int barW = cw - 24;
    tft.fillRoundRect(cx + 12, cy + 52, barW, 8, 4, C_DARKGRAY);

    long progress = subscriberCount - prevMilestone;
    long totalRange = nextMilestone - prevMilestone;
    int fillW = map(progress, 0, totalRange, 0, barW);
    fillW = constrain(fillW, 0, barW);
    tft.fillRoundRect(cx + 12, cy + 52, fillW, 8, 4, C_GOLD);

    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(BL_DATUM);
    tft.drawString("Goal: 150K Silver Play Button", cx + 12, cy + ch - 2, 1);

    tft.setTextDatum(BR_DATUM);
    float pct = ((float)progress / totalRange) * 100.0;
    char pctBuf[16];
    snprintf(pctBuf, sizeof(pctBuf), "%.1f%%", pct);
    tft.drawString(pctBuf, cx + cw - 12, cy + ch - 2, 1);
}

void drawChannelAnalytics() {
    int ax = 8;
    int ay = 100;
    int aw = 148;
    int ah = 64;

    tft.fillRoundRect(ax, ay, aw, ah, 6, C_CARD);
    tft.drawRoundRect(ax, ay, aw, ah, 6, C_CARD_BORDER);

    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("CHANNEL TOTALS", ax + 8, ay + 6, 1);

    // Views
    tft.setTextColor(C_WHITE, C_CARD);
    char vBuf[24];
    snprintf(vBuf, sizeof(vBuf), "%.2fM Views", totalViewsMillions);
    tft.drawString(vBuf, ax + 8, ay + 18, 2);

    // Uploads
    tft.setTextColor(C_GRAY, C_CARD);
    char uBuf[24];
    snprintf(uBuf, sizeof(uBuf), "%d Videos Uploaded", totalVideos);
    tft.drawString(uBuf, ax + 8, ay + 36, 1);

    // Velocity
    tft.setTextColor(C_CYAN, C_CARD);
    char velBuf[24];
    snprintf(velBuf, sizeof(velBuf), "+%d views/hr", viewVelocityPerHour);
    tft.drawString(velBuf, ax + 8, ay + 48, 1);
}

void drawLatestVideoCard() {
    int vx = 164;
    int vy = 100;
    int vw = 148;
    int vh = 64;

    tft.fillRoundRect(vx, vy, vw, vh, 6, C_CARD);
    tft.drawRoundRect(vx, vy, vw, vh, 6, C_CARD_BORDER);

    // Latest Upload Header & Rank
    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("LATEST UPLOAD", vx + 8, vy + 6, 1);

    tft.setTextColor(C_GOLD, C_CARD);
    tft.setTextDatum(TR_DATUM);
    char rkBuf[16];
    snprintf(rkBuf, sizeof(rkBuf), "%d of 10 *", latestVideoRank);
    tft.drawString(rkBuf, vx + vw - 8, vy + 6, 1);

    // Video Title
    tft.setTextColor(C_WHITE, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("CYD Desk Setup", vx + 8, vy + 20, 2);

    // Views & Likes
    tft.setTextColor(C_GREEN, C_CARD);
    char vlBuf[32];
    snprintf(vlBuf, sizeof(vlBuf), "%d.4K Views", latestVideoViewsK);
    tft.drawString(vlBuf, vx + 8, vy + 38, 1);

    tft.setTextColor(C_GRAY, C_CARD);
    char lkBuf[16];
    snprintf(lkBuf, sizeof(lkBuf), "%d Likes", latestVideoLikes);
    tft.drawString(lkBuf, vx + 8, vy + 50, 1);
}

void drawRecentActivity() {
    int rx = 8;
    int ry = 168;
    int rw = 304;
    int rh = 42;

    tft.fillRoundRect(rx, ry, rw, rh, 6, C_CARD);
    tft.drawRoundRect(rx, ry, rw, rh, 6, C_CARD_BORDER);

    // Recent Subscriber Banner
    tft.fillCircle(rx + 16, ry + 14, 5, C_GOLD);
    tft.setTextColor(C_GOLD, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("NEW SUBSCRIBER", rx + 26, ry + 8, 1);

    tft.setTextColor(C_WHITE, C_CARD);
    char subMsg[48];
    snprintf(subMsg, sizeof(subMsg), "%s joined the channel %dm ago!", recentSubName, recentSubMinutesAgo);
    tft.drawString(subMsg, rx + 26, ry + 22, 1);
}

void drawFooter() {
    int by = 214;
    tft.fillRect(0, by, 320, 26, C_BG);
    tft.drawFastHLine(0, by, 320, C_CARD_BORDER);

    // Refresh countdown
    tft.setTextColor(C_GRAY, C_BG);
    tft.setTextDatum(TL_DATUM);
    char refBuf[24];
    snprintf(refBuf, sizeof(refBuf), "Sync in %02ds", refreshCountdownSec);
    tft.drawString(refBuf, 8, by + 6, 2);

    // Audio chime toggle
    tft.setTextColor(chimeEnabled ? C_GREEN : C_GRAY, C_BG);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(chimeEnabled ? "CHIME: ON" : "CHIME: OFF", 312, by + 6, 2);
}

// =========================================================================
//  TOUCH INPUT HANDLING
// =========================================================================
void handleTouch() {
    if (!touch.touched()) return;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, TS_MIN_X, TS_MAX_X, 0, 320);
    int ty = map(p.y, TS_MIN_Y, TS_MAX_Y, 0, 240);
    tx = constrain(tx, 0, 319);
    ty = constrain(ty, 0, 239);

    // 1. Chime Toggle (Right of footer, X: 220..315, Y: 210..240)
    if (ty >= 210 && tx >= 220) {
        chimeEnabled = !chimeEnabled;
        playTouchClick();
        drawFooter();
        delay(250);
        return;
    }

    // 2. Direct Screen Tap: Force Sync / Increment demo subscriber
    if (ty < 210) {
        playCelebrationChime();
        subscriberCount++;
        subsGainedToday++;
        setRGB(255, 200, 0); // Gold pulse
        drawSubscriberCard();
        delay(200);
        setRGB(0, 0, 0);
        return;
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

    setRGB(0, 0, 0);

    // Init Display & Touch
    tft.init();
    tft.setRotation(1); // Landscape 320x240
    tft.fillScreen(C_BG);

    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    touch.begin(touchSPI);
    touch.setRotation(1);

    // Studio Boot Screen
    tft.setTextColor(C_YT_RED, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 80, 4);
    tft.setTextColor(C_GOLD, C_BG);
    tft.drawString("CREATOR LIVE STUDIO TICKER", 160, 120, 2);
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("Connecting to YouTube Data API v3...", 160, 150, 1);

    playTouchClick();
    delay(100);
    playCelebrationChime();
    delay(600);

    tft.fillScreen(C_BG);

    drawHeader();
    drawSubscriberCard();
    drawChannelAnalytics();
    drawLatestVideoCard();
    drawRecentActivity();
    drawFooter();
}

void loop() {
    handleTouch();

    unsigned long now = millis();

    // 1-second clock countdown
    if (now - lastSecTick >= 1000) {
        lastSecTick = now;
        refreshCountdownSec--;

        if (refreshCountdownSec <= 0) {
            refreshCountdownSec = 30;
            // Simulated live drift
            viewVelocityPerHour += random(-10, 15);
            totalViewsMillions += 0.0001;
            drawChannelAnalytics();
        }
        drawFooter();
    }

    // Periodic organic subscriber gain in demo mode (every 22 seconds)
    if (now - lastSubTick >= 22000) {
        lastSubTick = now;
        subscriberCount++;
        subsGainedToday++;
        playCelebrationChime();
        setRGB(255, 200, 0);
        drawSubscriberCard();
        delay(150);
        setRGB(0, 0, 0);
    }

    delay(20);
}

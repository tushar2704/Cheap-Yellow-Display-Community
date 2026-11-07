// =========================================================================
//  Project 56 — CYD Spotify / Apple Music Desk Controller (Car Thing)
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
#define C_BG          0x1082  // Deep dark Spotify navy/black
#define C_CARD        0x18C3  // Slightly lighter card container
#define C_GREEN       0x1DB2  // Spotify signature vibrant green (0x1DB954)
#define C_LIGHTGREEN  0x3FF4
#define C_GRAY        0x73AE  // Subtitle / muted text
#define C_DARKGRAY    0x2965  // Sliders and unselected buttons
#define C_HEART_RED   0xF800  // Liked heart red
#define C_CYAN        0x07FF
#define C_ORANGE      0xFD20
#define C_PURPLE      0x915C

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// =========================================================================
//  TRACK METADATA & PLAYBACK STATE
// =========================================================================
struct TrackInfo {
    const char* title;
    const char* artist;
    const char* album;
    int durationSec;
    uint16_t dominantColor;
};

const int PLAYLIST_SIZE = 5;
const TrackInfo DEMO_PLAYLIST[PLAYLIST_SIZE] = {
    { "Midnight City", "M83", "Hurry Up, We're Dreaming", 243, 0xF940 },
    { "Resonance", "HOME", "Odyssey", 212, 0x05BF },
    { "Starboy", "The Weeknd ft. Daft Punk", "Starboy", 230, 0xF800 },
    { "After Dark", "Mr.Kitty", "Time", 259, 0x8115 },
    { "Synthwave Dreams", "IamNotIntoTech", "Cyber Lounge Vol. 1", 195, 0x1DB2 }
};

int currentTrackIndex = 0;
bool isPlaying = true;
bool isShuffle = false;
int repeatMode = 0; // 0: Off, 1: All, 2: One
bool isLiked = true;
int volumePct = 78; // 0..100
int trackElapsedSec = 45;
unsigned long lastSecondTick = 0;
unsigned long lastBeatTick = 0;
float vinylAngle = 0;

// Device list
const char* DEVICES[3] = { "Studio Desk", "Living Room", "MacBook Pro" };
int currentDeviceIndex = 0;

// =========================================================================
//  SOUND & LED BEAT
// =========================================================================
void playClick(int freq = 1600, int ms = 15) {
    tone(BUZZER_PIN, freq, ms);
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
}

// =========================================================================
//  UI DRAWING
// =========================================================================
void drawHeader() {
    tft.fillRect(0, 0, 320, 26, C_BG);

    // Spotify/Brand Icon & Name
    tft.fillCircle(12, 13, 7, C_GREEN);
    // 3 curved lines inside circle
    tft.drawFastHLine(9, 10, 6, C_BLACK);
    tft.drawFastHLine(8, 13, 8, C_BLACK);
    tft.drawFastHLine(9, 16, 6, C_BLACK);

    tft.setTextColor(C_WHITE, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("IamNotIntoTech", 25, 7, 2);

    // Connected Device Chip
    tft.fillRoundRect(160, 4, 110, 18, 9, C_CARD);
    tft.drawRoundRect(160, 4, 110, 18, 9, C_DARKGRAY);
    tft.setTextColor(C_GREEN, C_CARD);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(DEVICES[currentDeviceIndex], 215, 13, 1);

    // Signal / WiFi icon
    tft.fillCircle(302, 13, 4, C_GREEN);
    tft.drawFastHLine(0, 26, 320, C_CARD);
}

void drawAlbumArt() {
    const int ax = 8;
    const int ay = 34;
    const int as = 104;

    // Album background card
    tft.fillRoundRect(ax, ay, as, as, 8, C_CARD);
    tft.drawRoundRect(ax, ay, as, as, 8, DEMO_PLAYLIST[currentTrackIndex].dominantColor);

    // Vinyl Record disc animation
    int cx = ax + (as / 2);
    int cy = ay + (as / 2);
    int r = 44;

    tft.fillCircle(cx, cy, r, C_BLACK);
    tft.drawCircle(cx, cy, r, 0x31A6);
    tft.drawCircle(cx, cy, r - 8, 0x2124);
    tft.drawCircle(cx, cy, r - 16, 0x31A6);
    tft.drawCircle(cx, cy, r - 24, 0x2124);

    // Center record label
    tft.fillCircle(cx, cy, 14, DEMO_PLAYLIST[currentTrackIndex].dominantColor);
    tft.fillCircle(cx, cy, 3, C_WHITE); // Spindle hole

    // Stylus / Tone Arm indicator in top-right corner
    tft.drawLine(ax + as - 12, ay + 6, cx + 18, cy - 14, C_GRAY);
    tft.fillCircle(ax + as - 12, ay + 6, 3, C_WHITE);
}

void drawTrackDetails() {
    int tx = 120;
    int ty = 34;
    int tw = 320 - tx - 8;

    tft.fillRect(tx, ty, tw, 72, C_BG);

    // Song Title
    tft.setTextColor(C_WHITE, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(DEMO_PLAYLIST[currentTrackIndex].title, tx, ty + 2, 4);

    // Artist
    tft.setTextColor(C_GRAY, C_BG);
    tft.drawString(DEMO_PLAYLIST[currentTrackIndex].artist, tx, ty + 26, 2);

    // Album
    tft.setTextColor(0x52AA, C_BG);
    tft.drawString(DEMO_PLAYLIST[currentTrackIndex].album, tx, ty + 46, 1);

    // Heart / Favorite Icon (X: 300, Y: ty + 4)
    uint16_t hCol = isLiked ? C_HEART_RED : C_DARKGRAY;
    tft.fillCircle(298, ty + 10, 4, hCol);
    tft.fillCircle(306, ty + 10, 4, hCol);
    tft.fillTriangle(294, ty + 10, 310, ty + 10, 302, ty + 20, hCol);
}

void drawProgressBar() {
    int px = 120;
    int py = 110;
    int pw = 192;
    int totalSec = DEMO_PLAYLIST[currentTrackIndex].durationSec;

    // Elapsed & Remaining time text
    tft.setTextColor(C_GRAY, C_BG);
    tft.setTextDatum(TL_DATUM);
    char elBuf[10];
    snprintf(elBuf, sizeof(elBuf), "%02d:%02d", trackElapsedSec / 60, trackElapsedSec % 60);
    tft.drawString(elBuf, px, py + 12, 1);

    tft.setTextDatum(TR_DATUM);
    char remBuf[10];
    snprintf(remBuf, sizeof(remBuf), "%02d:%02d", totalSec / 60, totalSec % 60);
    tft.drawString(remBuf, px + pw, py + 12, 1);

    // Track Bar Background
    tft.fillRoundRect(px, py, pw, 6, 3, C_DARKGRAY);

    // Progress Bar Fill
    int fillW = map(trackElapsedSec, 0, totalSec, 0, pw);
    fillW = constrain(fillW, 0, pw);
    tft.fillRoundRect(px, py, fillW, 6, 3, C_GREEN);

    // Thumb Knob
    if (fillW > 3 && fillW < pw - 3) {
        tft.fillCircle(px + fillW, py + 3, 5, C_WHITE);
    }
}

void drawMediaControls() {
    int cy = 168;

    // 1. Shuffle (Center X = 32)
    uint16_t shCol = isShuffle ? C_GREEN : C_GRAY;
    tft.setTextColor(shCol, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("SHUF", 32, cy, 2);

    // 2. Previous Track (Center X = 92)
    tft.fillCircle(92, cy, 18, C_CARD);
    tft.drawCircle(92, cy, 18, C_DARKGRAY);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString("|<<", 92, cy, 2);

    // 3. Play / Pause Big Button (Center X = 160)
    uint16_t ppCol = isPlaying ? C_GREEN : C_WHITE;
    uint16_t ppFg = isPlaying ? C_BLACK : C_BLACK;
    tft.fillCircle(160, cy, 24, ppCol);
    tft.setTextColor(ppFg, ppCol);
    if (isPlaying) {
        tft.fillRect(153, cy - 8, 4, 16, C_BLACK);
        tft.fillRect(163, cy - 8, 4, 16, C_BLACK);
    } else {
        tft.fillTriangle(154, cy - 9, 154, cy + 9, 170, cy, C_BLACK);
    }

    // 4. Next Track (Center X = 228)
    tft.fillCircle(228, cy, 18, C_CARD);
    tft.drawCircle(228, cy, 18, C_DARKGRAY);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString(">>|", 228, cy, 2);

    // 5. Repeat (Center X = 288)
    uint16_t repCol = (repeatMode > 0) ? C_GREEN : C_GRAY;
    tft.setTextColor(repCol, C_BG);
    tft.drawString(repeatMode == 2 ? "REP 1" : "REP", 288, cy, 2);
}

void drawVolumeBar() {
    int vy = 212;
    int vx = 40;
    int vw = 200;

    // Volume speaker icon (X: 18)
    tft.setTextColor(C_GRAY, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("VOL", 18, vy + 4, 1);

    // Slider track
    tft.fillRoundRect(vx, vy, vw, 8, 4, C_DARKGRAY);
    int fillW = map(volumePct, 0, 100, 0, vw);
    fillW = constrain(fillW, 0, vw);
    tft.fillRoundRect(vx, vy, fillW, 8, 4, C_GREEN);
    tft.fillCircle(vx + fillW, vy + 4, 6, C_WHITE);

    // Percentage Label
    tft.setTextColor(C_WHITE, C_BG);
    tft.setTextDatum(TL_DATUM);
    char volBuf[8];
    snprintf(volBuf, sizeof(volBuf), "%d%%", volumePct);
    tft.drawString(volBuf, vx + vw + 10, vy, 2);
}

// =========================================================================
//  TRACK NAVIGATION & CONTROLS
// =========================================================================
void switchTrack(int dir) {
    playClick(1800, 20);
    currentTrackIndex = (currentTrackIndex + dir + PLAYLIST_SIZE) % PLAYLIST_SIZE;
    trackElapsedSec = 0;
    
    // Flash RGB LED dominant color
    uint16_t dc = DEMO_PLAYLIST[currentTrackIndex].dominantColor;
    uint8_t r = (dc >> 11) << 3;
    uint8_t g = ((dc >> 5) & 0x3F) << 2;
    uint8_t b = (dc & 0x1F) << 3;
    setRGB(r, g, b);

    drawAlbumArt();
    drawTrackDetails();
    drawProgressBar();
}

void togglePlayPause() {
    isPlaying = !isPlaying;
    playClick(isPlaying ? 2000 : 1000, 25);
    drawMediaControls();
}

// =========================================================================
//  TOUCH INPUT PROCESSING
// =========================================================================
void handleTouch() {
    if (!touch.touched()) return;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, TS_MIN_X, TS_MAX_X, 0, 320);
    int ty = map(p.y, TS_MIN_Y, TS_MAX_Y, 0, 240);
    tx = constrain(tx, 0, 319);
    ty = constrain(ty, 0, 239);

    // 1. Device Chip Tap (Header)
    if (ty < 26 && tx >= 160 && tx <= 270) {
        currentDeviceIndex = (currentDeviceIndex + 1) % 3;
        playClick(1400, 20);
        drawHeader();
        delay(250);
        return;
    }

    // 2. Favorite Heart Tap
    if (ty >= 30 && ty <= 60 && tx >= 288 && tx <= 316) {
        isLiked = !isLiked;
        playClick(isLiked ? 2200 : 800, 25);
        drawTrackDetails();
        delay(250);
        return;
    }

    // 3. Scrub Progress Bar Tap (Y: 104..128, X: 115..315)
    if (ty >= 104 && ty <= 128 && tx >= 115 && tx <= 315) {
        int pw = 192;
        int tapOffset = tx - 120;
        tapOffset = constrain(tapOffset, 0, pw);
        trackElapsedSec = map(tapOffset, 0, pw, 0, DEMO_PLAYLIST[currentTrackIndex].durationSec);
        playClick(1600, 15);
        drawProgressBar();
        delay(200);
        return;
    }

    // 4. Media Controls (Y: 144..192)
    if (ty >= 144 && ty <= 192) {
        // Shuffle (X: 10..55)
        if (tx >= 10 && tx <= 55) {
            isShuffle = !isShuffle;
            playClick(1500, 20);
            drawMediaControls();
            delay(250);
            return;
        }

        // Previous Track (X: 72..112)
        if (tx >= 72 && tx <= 112) {
            switchTrack(-1);
            delay(250);
            return;
        }

        // Play / Pause (X: 136..184)
        if (tx >= 136 && tx <= 184) {
            togglePlayPause();
            delay(250);
            return;
        }

        // Next Track (X: 208..248)
        if (tx >= 208 && tx <= 248) {
            switchTrack(1);
            delay(250);
            return;
        }

        // Repeat (X: 268..310)
        if (tx >= 268 && tx <= 310) {
            repeatMode = (repeatMode + 1) % 3;
            playClick(1500, 20);
            drawMediaControls();
            delay(250);
            return;
        }
    }

    // 5. Volume Slider Tap (Y: 200..235, X: 35..250)
    if (ty >= 200 && ty <= 235 && tx >= 35 && tx <= 250) {
        int vw = 200;
        int tapOffset = tx - 40;
        tapOffset = constrain(tapOffset, 0, vw);
        volumePct = map(tapOffset, 0, vw, 0, 100);
        playClick(1200, 15);
        drawVolumeBar();
        delay(150);
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

    // Boot Logo
    tft.setTextColor(C_GREEN, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 90, 4);
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("CAR THING & DESK AUDIO", 160, 130, 2);
    tft.setTextColor(C_GRAY, C_BG);
    tft.drawString("Connecting to Spotify Web Engine...", 160, 160, 1);

    playClick(1000, 60);
    delay(80);
    playClick(1600, 100);
    delay(700);

    tft.fillScreen(C_BG);

    drawHeader();
    drawAlbumArt();
    drawTrackDetails();
    drawProgressBar();
    drawMediaControls();
    drawVolumeBar();

    setRGB(0, 180, 50); // Spotify Green
}

void loop() {
    handleTouch();

    unsigned long now = millis();

    // 1-second track playback progress
    if (isPlaying && now - lastSecondTick >= 1000) {
        lastSecondTick = now;
        trackElapsedSec++;

        if (trackElapsedSec >= DEMO_PLAYLIST[currentTrackIndex].durationSec) {
            if (repeatMode == 2) {
                trackElapsedSec = 0;
            } else {
                switchTrack(1);
            }
        }
        drawProgressBar();
    }

    // Gentle rhythm beat pulse on RGB LED when playing
    if (isPlaying && now - lastBeatTick >= 500) {
        lastBeatTick = now;
        uint16_t dc = DEMO_PLAYLIST[currentTrackIndex].dominantColor;
        uint8_t r = (dc >> 11) << 3;
        uint8_t g = ((dc >> 5) & 0x3F) << 2;
        uint8_t b = (dc & 0x1F) << 3;
        setRGB(r / 2, g / 2, b / 2);
    }

    delay(20);
}


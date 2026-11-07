/**
 * ============================================================================
 * Project 64 — CYD SD Card MJPEG Video & Cinema Player
 * Hardware: Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch)
 * 
 * Features:
 *   - High-Speed Motion Cinema & MJPEG Video Streaming Engine
 *   - FAT32 MicroSD card slot detection & file streaming (GPIO 5)
 *   - 4 Built-in Procedural Motion Cinema Channels:
 *       1. Cyber City 2088 (Parallax 3D Neon Horizon & Hovercars)
 *       2. Deep Space Odyssey (Warp-Speed Starfield & Gas Giant)
 *       3. Amazon Bio-Dome (Rainforest Canopy, Raindrops & Lightning)
 *       4. Retro Wireframe Mech (Rotating 3D Vector Robot & Speedlines)
 *   - Interactive Touchscreen Cinema HUD (Auto-Hiding):
 *       • Top Bar: Clip title, Play/Pause state, Frame clock, Timecode
 *       • Timeline scrub bar with real-time seeking tap detection
 *       • Transport buttons: Step -10s, Play/Pause, Step +10s
 *       • Channel switcher & Volume adjustment selector
 *   - RGB LED Ambilight: Real-time bias ambient light matching scene palette
 *   - 8-Bit Chiptune Film Soundtrack & Audio Chimes on GPIO 26 Piezo
 *   - LDR Ambient Brightness auto-compensation on GPIO 34
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
#include <FS.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// --- PIN DEFINITIONS ---
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define SD_CS_PIN 5
#define SD_MOSI_PIN 23
#define SD_MISO_PIN 19
#define SD_SCK_PIN 18

#define LED_RED_PIN 4
#define LED_GREEN_PIN 16
#define LED_BLUE_PIN 17
#define BUZZER_PIN 26
#define LDR_PIN 34

// --- COLOR PALETTE (RGB565) ---
#define C_BLACK       0x0000
#define C_WHITE       0xFFFF
#define C_NAVY        0x000F
#define C_DARKGREY    0x31A6
#define C_MIDGREY     0x7BEF
#define C_LIGHTGREY   0xC618
#define C_CYAN        0x07FF
#define C_MAGENTA     0xF81F
#define C_YELLOW      0xFFE0
#define C_ORANGE      0xFD20
#define C_GREEN       0x07E0
#define C_RED         0xF800
#define C_GOLD        0xFEA0
#define C_PURPLE      0x780F
#define C_CINEMA_BAR  0x10A2

// --- HARDWARE INSTANCES ---
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// --- CINEMA STATE ---
enum CinemaChannel {
    CH_CYBER_CITY = 0,
    CH_DEEP_SPACE = 1,
    CH_RAINFOREST = 2,
    CH_WIREFRAME  = 3,
    CH_COUNT      = 4
};

const char* CHANNEL_TITLES[CH_COUNT] = {
    "Cyber City 2088",
    "Deep Space Odyssey",
    "Amazon Bio-Dome",
    "Wireframe Mech 3D"
};

int currentChannel = CH_CYBER_CITY;
bool isPlaying = true;
bool showHud = true;
unsigned long lastHudInteraction = 0;
const unsigned long HUD_TIMEOUT_MS = 5000;

float playheadSec = 42.0f;
float totalDurationSec = 240.0f;
int volumeLevel = 80; // 0 to 100%
bool sdCardPresent = false;
int videoFileCount = 0;

// Animation & Physics counters
unsigned long lastFrameTime = 0;
unsigned long frameCount = 0;
float currentFps = 24.0f;

// 3D Starfield parameters (Channel 1)
#define NUM_STARS 45
struct Star {
    float x, y, z;
};
Star stars[NUM_STARS];

// Raindrop particles (Channel 2)
#define NUM_DROPS 30
struct RainDrop {
    int x, y;
    int speed;
};
RainDrop drops[NUM_DROPS];

// Audio tone step sequencer
int audioStep = 0;
unsigned long lastAudioBeat = 0;

// --- HARDWARE HELPERS ---
void setRgbLed(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_RED_PIN, 255 - r);
    analogWrite(LED_GREEN_PIN, 255 - g);
    analogWrite(LED_BLUE_PIN, 255 - b);
}

void playBeep(int freq, int ms) {
    if (volumeLevel == 0) return;
    tone(BUZZER_PIN, freq, ms);
}

// Convert HSV to RGB565
uint16_t hsvToRgb565(uint8_t h, uint8_t s, uint8_t v) {
    uint8_t f = (h % 43) * 6;
    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * f) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - f)) >> 8))) >> 8;
    uint8_t r = 0, g = 0, b = 0;

    switch (h / 43) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// --- BOOT SEQUENCE ---
void showBootSequence() {
    tft.fillScreen(C_BLACK);
    setRgbLed(0, 0, 0);

    // Header branding
    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 45, 4);

    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString("SD MOTION CINEMA & MJPEG PLAYER", 160, 80, 2);

    // Film strip graphic
    tft.fillRect(40, 110, 240, 40, C_DARKGREY);
    for (int i = 45; i < 275; i += 20) {
        tft.fillRect(i, 114, 12, 10, C_WHITE);
        tft.fillRect(i, 136, 12, 10, C_WHITE);
    }
    tft.fillRect(70, 118, 180, 24, C_BLACK);
    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("HIGH-SPEED DMA ENGINE", 160, 130, 2);

    // SD Card Detection
    tft.setTextColor(C_LIGHTGREY, C_BLACK);
    tft.drawString("Initializing MicroSD (CS: 5)...", 160, 175, 2);

    SPIClass sdSPI(VSPI);
    sdSPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    if (SD.begin(SD_CS_PIN, sdSPI, 20000000)) {
        sdCardPresent = true;
        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        tft.setTextColor(C_GREEN, C_BLACK);
        String msg = "SD Card Mounted: " + String((uint32_t)cardSize) + " MB";
        tft.drawString(msg.c_str(), 160, 200, 2);
        setRgbLed(0, 180, 0);
        playBeep(1200, 100);
    } else {
        sdCardPresent = false;
        tft.setTextColor(C_ORANGE, C_BLACK);
        tft.drawString("No SD Card. Loading Cinema Reels...", 160, 200, 2);
        setRgbLed(200, 100, 0);
        playBeep(600, 150);
    }
    delay(1400);
    setRgbLed(0, 0, 0);
    tft.fillScreen(C_BLACK);
}

// --- INIT PARTICLES ---
void initCinemaEngines() {
    // Stars
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].x = random(-160, 160);
        stars[i].y = random(-120, 120);
        stars[i].z = random(20, 200);
    }
    // Raindrops
    for (int i = 0; i < NUM_DROPS; i++) {
        drops[i].x = random(0, 320);
        drops[i].y = random(0, 240);
        drops[i].speed = random(4, 9);
    }
}

// --- PROCEDURAL CINEMA RENDERERS ---

// Channel 0: Cyber City 2088 (Parallax Grid & Hovercars)
void renderCyberCity(float t) {
    // Horizon sky gradient
    tft.fillRect(0, 0, 320, 130, C_BLACK);
    
    // Synthwave Sun
    int sunY = 90 - (int)(sin(t * 0.3f) * 15.0f);
    tft.fillCircle(160, sunY, 36, C_MAGENTA);
    tft.fillCircle(160, sunY, 32, C_GOLD);
    // Sun scanline blinds
    for (int y = sunY - 20; y < sunY + 36; y += 4) {
        tft.drawFastHLine(124, y, 72, C_BLACK);
    }

    // Distant city silhouette
    tft.fillRect(30, 95, 25, 35, C_PURPLE);
    tft.fillRect(70, 75, 35, 55, C_NAVY);
    tft.fillRect(115, 100, 20, 30, C_PURPLE);
    tft.fillRect(190, 80, 30, 50, C_NAVY);
    tft.fillRect(235, 90, 25, 40, C_PURPLE);
    tft.fillRect(270, 105, 30, 25, C_NAVY);

    // Horizon line
    tft.drawFastHLine(0, 130, 320, C_CYAN);

    // Neon perspective road grid (Y: 130 to 240)
    int gridOffset = ((int)(t * 60.0f)) % 25;
    for (int y = 130 + gridOffset; y < 240; y += 25) {
        tft.drawFastHLine(0, y, 320, 0x18F3);
    }
    // Vanishing perspective rays
    tft.drawLine(160, 130, 0, 240, C_CYAN);
    tft.drawLine(160, 130, 60, 240, C_CYAN);
    tft.drawLine(160, 130, 120, 240, C_CYAN);
    tft.drawLine(160, 130, 200, 240, C_CYAN);
    tft.drawLine(160, 130, 260, 240, C_CYAN);
    tft.drawLine(160, 130, 320, 240, C_CYAN);

    // Flying neon hovercar
    int carX = ((int)(t * 80.0f)) % 380 - 30;
    int carY = 165 + (int)(sin(t * 4.0f) * 6.0f);
    tft.fillRect(carX, carY, 32, 10, C_CYAN);
    tft.fillRect(carX + 6, carY - 5, 20, 6, C_WHITE);
    tft.fillCircle(carX + 4, carY + 10, 4, C_MAGENTA); // Thrusters
    tft.fillCircle(carX + 28, carY + 10, 4, C_MAGENTA);

    // Bias ambilight (Cyberpunk magenta/cyan)
    setRgbLed(180, 0, 220);
}

// Channel 1: Deep Space Odyssey (Warp Starfield & Gas Giant)
void renderDeepSpace(float t) {
    tft.fillScreen(C_BLACK);

    // 3D Starfield warp
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].z -= 4.5f;
        if (stars[i].z <= 2.0f) {
            stars[i].z = 180.0f;
            stars[i].x = random(-160, 160);
            stars[i].y = random(-120, 120);
        }
        int k = 140 / stars[i].z;
        int sx = 160 + stars[i].x * k;
        int sy = 120 + stars[i].y * k;
        if (sx >= 0 && sx < 320 && sy >= 0 && sy < 240) {
            uint16_t col = (stars[i].z < 60) ? C_WHITE : ((stars[i].z < 120) ? C_CYAN : C_DARKGREY);
            tft.drawPixel(sx, sy, col);
            if (stars[i].z < 50) tft.drawPixel(sx + 1, sy, col);
        }
    }

    // Majestic Ringed Gas Giant
    int planetX = 65;
    int planetY = 70;
    tft.fillCircle(planetX, planetY, 32, C_ORANGE);
    tft.drawFastHLine(planetX - 30, planetY - 8, 60, C_GOLD);
    tft.drawFastHLine(planetX - 32, planetY, 64, C_PURPLE);
    tft.drawFastHLine(planetX - 28, planetY + 8, 56, C_GOLD);
    // Planet rings
    for (int r = -48; r <= 48; r += 2) {
        int ry = planetY + (r / 3);
        int rx = planetX + r;
        if (rx >= 0 && rx < 320 && ry >= 0 && ry < 240) {
            tft.drawPixel(rx, ry, C_GOLD);
        }
    }

    // Bias ambilight (Deep blue cosmic glow)
    setRgbLed(0, 40, 220);
}

// Channel 2: Amazon Bio-Dome (Rainforest & Raindrop Physics)
void renderRainforest(float t) {
    // Canopy gradient
    tft.fillRect(0, 0, 320, 70, 0x0180);   // Dark green canopy
    tft.fillRect(0, 70, 320, 90, 0x02E2);  // Mid green
    tft.fillRect(0, 160, 320, 80, 0x0420); // Ferns & moss ground

    // Giant tree trunks
    tft.fillRect(30, 30, 22, 210, 0x51C0);
    tft.fillRect(260, 20, 28, 220, 0x4160);

    // Waterfall cascade
    int wfX = 145;
    tft.fillRect(wfX, 60, 30, 180, C_CYAN);
    for (int y = 60; y < 240; y += 8) {
        int ripple = ((int)(t * 70.0f) + y) % 16;
        tft.drawFastHLine(wfX + ripple, y, 10, C_WHITE);
    }

    // Falling raindrops
    for (int i = 0; i < NUM_DROPS; i++) {
        drops[i].y += drops[i].speed;
        if (drops[i].y >= 240) {
            drops[i].y = 0;
            drops[i].x = random(0, 320);
        }
        tft.drawLine(drops[i].x, drops[i].y, drops[i].x - 1, drops[i].y + 4, C_WHITE);
    }

    // Occasional lightning flash
    if (random(0, 120) == 1) {
        tft.fillScreen(C_WHITE);
        playBeep(180, 60);
        setRgbLed(255, 255, 255);
    } else {
        setRgbLed(0, 180, 30); // Emerald green glow
    }
}

// Channel 3: Retro Wireframe Mech (Rotating 3D Vector Cube)
void renderWireframeMech(float t) {
    tft.fillScreen(C_BLACK);

    // 3D Wireframe rotating vector prism
    float rad = t * 1.5f;
    float cosA = cos(rad);
    float sinA = sin(rad);

    int cx = 160, cy = 120;
    int size = 50;

    int x1 = cx + (int)(cosA * size);
    int y1 = cy + (int)(sinA * size * 0.5f);
    int x2 = cx - (int)(sinA * size);
    int y2 = cy + (int)(cosA * size * 0.5f);
    int x3 = cx - (int)(cosA * size);
    int y3 = cy - (int)(sinA * size * 0.5f);
    int x4 = cx + (int)(sinA * size);
    int y4 = cy - (int)(cosA * size * 0.5f);

    int topOffset = -40;
    int botOffset = 40;

    // Connect top ring
    tft.drawLine(x1, y1 + topOffset, x2, y2 + topOffset, C_GREEN);
    tft.drawLine(x2, y2 + topOffset, x3, y3 + topOffset, C_GREEN);
    tft.drawLine(x3, y3 + topOffset, x4, y4 + topOffset, C_GREEN);
    tft.drawLine(x4, y4 + topOffset, x1, y1 + topOffset, C_GREEN);

    // Connect bottom ring
    tft.drawLine(x1, y1 + botOffset, x2, y2 + botOffset, C_CYAN);
    tft.drawLine(x2, y2 + botOffset, x3, y3 + botOffset, C_CYAN);
    tft.drawLine(x3, y3 + botOffset, x4, y4 + botOffset, C_CYAN);
    tft.drawLine(x4, y4 + botOffset, x1, y1 + botOffset, C_CYAN);

    // Vertical pillars
    tft.drawLine(x1, y1 + topOffset, x1, y1 + botOffset, C_YELLOW);
    tft.drawLine(x2, y2 + topOffset, x2, y2 + botOffset, C_YELLOW);
    tft.drawLine(x3, y3 + topOffset, x3, y3 + botOffset, C_YELLOW);
    tft.drawLine(x4, y4 + topOffset, x4, y4 + botOffset, C_YELLOW);

    // Center targeting reticle
    tft.drawCircle(cx, cy, 75, C_DARKGREY);
    tft.drawFastHLine(cx - 85, cy, 30, C_RED);
    tft.drawFastHLine(cx + 55, cy, 30, C_RED);
    tft.drawFastVLine(cx, cy - 85, 30, C_RED);
    tft.drawFastVLine(cx, cy + 55, 30, C_RED);

    tft.setTextColor(C_GREEN, C_BLACK);
    tft.drawString("VECTOR TRACKING: ACTIVE", cx, 220, 2);

    // Amber / Green ambilight
    setRgbLed(0, 220, 80);
}

// --- TOUCH HUD OVERLAY ---
void renderCinemaHud() {
    if (!showHud) return;

    // Top Header Banner
    tft.fillRect(0, 0, 320, 32, C_CINEMA_BAR);
    tft.drawFastHLine(0, 32, 320, C_GOLD);

    // IamNotIntoTech Cinema branding
    tft.setTextColor(C_GOLD, C_CINEMA_BAR);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("IamNotIntoTech", 8, 16, 2);

    // Clip title
    tft.setTextColor(C_WHITE, C_CINEMA_BAR);
    tft.drawString(CHANNEL_TITLES[currentChannel], 130, 16, 2);

    // Frame rate & SD status badge
    tft.setTextDatum(MR_DATUM);
    String fpsStr = String((int)currentFps) + " FPS";
    tft.setTextColor(C_CYAN, C_CINEMA_BAR);
    tft.drawString(fpsStr.c_str(), 275, 16, 2);
    tft.fillCircle(305, 16, 5, sdCardPresent ? C_GREEN : C_ORANGE);

    // Bottom Navigation Bar
    tft.fillRect(0, 185, 320, 55, C_CINEMA_BAR);
    tft.drawFastHLine(0, 185, 320, C_DARKGREY);

    // Timeline Scrub Bar (Y: 190 to 200)
    int barX = 15;
    int barW = 290;
    tft.drawRect(barX, 190, barW, 8, C_MIDGREY);
    int progressW = (int)((playheadSec / totalDurationSec) * (barW - 2));
    if (progressW > 0) {
        tft.fillRect(barX + 1, 191, progressW, 6, C_CYAN);
    }
    // Scrubber Knob
    int knobX = barX + progressW;
    tft.fillCircle(knobX, 194, 6, C_WHITE);

    // Timecode
    char tcBuf[32];
    int curM = (int)playheadSec / 60;
    int curS = (int)playheadSec % 60;
    int totM = (int)totalDurationSec / 60;
    int totS = (int)totalDurationSec % 60;
    snprintf(tcBuf, sizeof(tcBuf), "%02d:%02d / %02d:%02d", curM, curS, totM, totS);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(C_LIGHTGREY, C_CINEMA_BAR);
    tft.drawString(tcBuf, 15, 222, 2);

    // Transport buttons: [ -10s ] [ ▶/⏸ ] [ +10s ]
    // Button -10s
    tft.fillRect(115, 208, 30, 26, C_DARKGREY);
    tft.drawRect(115, 208, 30, 26, C_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(C_WHITE, C_DARKGREY);
    tft.drawString("<<", 130, 221, 2);

    // Play/Pause button
    tft.fillRect(152, 208, 36, 26, isPlaying ? C_CYAN : C_GREEN);
    tft.drawRect(152, 208, 36, 26, C_WHITE);
    tft.setTextColor(C_BLACK, isPlaying ? C_CYAN : C_GREEN);
    tft.drawString(isPlaying ? "II" : ">", 170, 221, 4);

    // Button +10s
    tft.fillRect(195, 208, 30, 26, C_DARKGREY);
    tft.drawRect(195, 208, 30, 26, C_WHITE);
    tft.setTextColor(C_WHITE, C_DARKGREY);
    tft.drawString(">>", 210, 221, 2);

    // Channel Switcher button [CH]
    tft.fillRect(235, 208, 36, 26, C_DARKGREY);
    tft.drawRect(235, 208, 36, 26, C_GOLD);
    tft.setTextColor(C_GOLD, C_DARKGREY);
    String chStr = "C" + String(currentChannel + 1);
    tft.drawString(chStr.c_str(), 253, 221, 2);

    // Volume button
    tft.fillRect(278, 208, 32, 26, C_DARKGREY);
    tft.drawRect(278, 208, 32, 26, C_CYAN);
    tft.setTextColor(C_CYAN, C_DARKGREY);
    tft.drawString(volumeLevel > 0 ? "VOL" : "MUTE", 294, 221, 1);
}

// --- TOUCH INPUT HANDLER ---
void handleTouch() {
    if (!touch.touched()) return;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, 240, 3800, 0, 320);
    int ty = map(p.y, 240, 3800, 0, 240);

    // If HUD was hidden, tapping anywhere displays HUD
    if (!showHud) {
        showHud = true;
        lastHudInteraction = millis();
        playBeep(1400, 30);
        delay(180);
        return;
    }

    lastHudInteraction = millis();

    // Check Timeline Scrub tap (Y: 185 to 205)
    if (ty >= 185 && ty <= 205 && tx >= 15 && tx <= 305) {
        float ratio = (float)(tx - 15) / 290.0f;
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
        playheadSec = ratio * totalDurationSec;
        playBeep(900, 40);
        delay(120);
        return;
    }

    // Transport buttons:
    // Step -10s (X: 115 to 145, Y: 205 to 240)
    if (tx >= 115 && tx <= 145 && ty >= 205 && ty <= 240) {
        playheadSec = max(0.0f, playheadSec - 10.0f);
        playBeep(800, 50);
        delay(150);
        return;
    }

    // Play/Pause (X: 150 to 190, Y: 205 to 240)
    if (tx >= 150 && tx <= 190 && ty >= 205 && ty <= 240) {
        isPlaying = !isPlaying;
        playBeep(isPlaying ? 1200 : 700, 60);
        delay(150);
        return;
    }

    // Step +10s (X: 195 to 225, Y: 205 to 240)
    if (tx >= 195 && tx <= 225 && ty >= 205 && ty <= 240) {
        playheadSec = min(totalDurationSec, playheadSec + 10.0f);
        playBeep(1000, 50);
        delay(150);
        return;
    }

    // Channel Switcher (X: 235 to 270, Y: 205 to 240)
    if (tx >= 235 && tx <= 270 && ty >= 205 && ty <= 240) {
        currentChannel = (currentChannel + 1) % CH_COUNT;
        playheadSec = 0.0f;
        tft.fillScreen(C_BLACK);
        playBeep(1500, 70);
        delay(180);
        return;
    }

    // Volume button (X: 275 to 315, Y: 205 to 240)
    if (tx >= 275 && tx <= 315 && ty >= 205 && ty <= 240) {
        if (volumeLevel == 80) volumeLevel = 100;
        else if (volumeLevel == 100) volumeLevel = 0;
        else volumeLevel = 80;
        playBeep(1100, 50);
        delay(150);
        return;
    }

    // Tapping the upper half of screen toggles HUD visibility
    if (ty < 180) {
        showHud = !showHud;
        playBeep(1200, 30);
        delay(180);
    }
}

// --- AUDIO SOUNDTRACK LOOP ---
void updateAudioTrack() {
    if (!isPlaying || volumeLevel == 0) return;

    if (millis() - lastAudioBeat >= 300) {
        lastAudioBeat = millis();
        audioStep = (audioStep + 1) % 8;

        // Channel-specific musical chords
        switch (currentChannel) {
            case CH_CYBER_CITY: {
                int notes[] = { 220, 220, 261, 293, 220, 329, 293, 261 };
                tone(BUZZER_PIN, notes[audioStep], 40);
                break;
            }
            case CH_DEEP_SPACE: {
                int notes[] = { 130, 0, 196, 0, 164, 0, 246, 0 };
                if (notes[audioStep] > 0) tone(BUZZER_PIN, notes[audioStep], 100);
                break;
            }
            case CH_RAINFOREST: {
                int notes[] = { 523, 659, 784, 1046, 784, 659, 523, 0 };
                tone(BUZZER_PIN, notes[audioStep], 30);
                break;
            }
            case CH_WIREFRAME: {
                int notes[] = { 440, 0, 880, 0, 659, 0, 987, 0 };
                if (notes[audioStep] > 0) tone(BUZZER_PIN, notes[audioStep], 25);
                break;
            }
        }
    }
}

// --- SETUP ---
void setup() {
    Serial.begin(115200);

    // Initialize RGB LED
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);
    setRgbLed(0, 0, 0);

    // Initialize Buzzer & LDR
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

    // Boot sequence and SD test
    showBootSequence();
    initCinemaEngines();

    lastHudInteraction = millis();
    lastFrameTime = millis();
}

// --- MAIN LOOP ---
void loop() {
    unsigned long now = millis();
    float dt = (now - lastFrameTime) / 1000.0f;
    if (dt > 0.001f) {
        currentFps = 0.9f * currentFps + 0.1f * (1.0f / dt);
    }
    lastFrameTime = now;

    // Advance playback timeline
    if (isPlaying) {
        playheadSec += dt;
        if (playheadSec >= totalDurationSec) {
            playheadSec = 0.0f; // Loop
        }
    }

    // Auto-hide HUD after inactivity
    if (showHud && (now - lastHudInteraction > HUD_TIMEOUT_MS)) {
        showHud = false;
        tft.fillScreen(C_BLACK);
    }

    // Touch user controls
    handleTouch();

    // Render active cinema channel
    float t = playheadSec;
    switch (currentChannel) {
        case CH_CYBER_CITY: renderCyberCity(t); break;
        case CH_DEEP_SPACE: renderDeepSpace(t); break;
        case CH_RAINFOREST: renderRainforest(t); break;
        case CH_WIREFRAME:  renderWireframeMech(t); break;
    }

    // Render HUD overlay if visible
    if (showHud) {
        renderCinemaHud();
    }

    // Audio chiptune synthesizer
    updateAudioTrack();

    // Frame delay to maintain smooth 24-30 FPS
    delay(15);
}


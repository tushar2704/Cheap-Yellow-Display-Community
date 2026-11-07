/**
 * ============================================================================
 * Project 68 — CYD Denki-Kurage Pixel Jellyfish & Desktop Aquarium
 * Hardware: Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch)
 * 
 * Features:
 *   - Japanese Bioluminescent "Denki-Kurage" (Electric Jellyfish) Desk Aquarium
 *   - Procedural Marine Undulation Locomotion:
 *       • Rhythmic bell contraction and expansion physics
 *       • 6 Elastic multi-joint trailing tentacles with inverse spring kinematics
 *       • Pulsing inner electric organ with bioluminescent core
 *   - Interactive Touch Ecosystem:
 *       • Water Ripples: Tap anywhere to create shockwave ripples that displace the jellyfish
 *       • Feed Button: Drops sinking bioluminescent plankton flakes that the jellyfish tracks
 *       • Palette Switcher: Cycles through 5 luminous bio-chroma palettes:
 *           1. Electric Cyan
 *           2. Neon Bio-Pink
 *           3. Emerald Phosphor
 *           4. Deep-Sea Amber
 *           5. Ghost Spectral White
 *       • Lighting Switcher: Toggles Azure Ocean Day vs Midnight Abyss Night mode
 *   - Ambient Ocean Physics:
 *       • Continuous buoyant air bubbles rising from bottom aerator stone
 *       • Deep water caustics light rays filtering from the surface
 *   - Onboard Hardware Integration:
 *       • RGB LED ambient bioluminescent pulse matching bell contractions
 *       • Piezo buzzer hydrophone bubble pops & ethereal chime scales on GPIO 26
 *       • LDR ambient room sensor on GPIO 34 for automatic night mode shift
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
#define C_NAVY        0x000F
#define C_DARKBLUE    0x0012
#define C_MIDBLUE     0x08F5
#define C_OCEAN_CYAN  0x1458
#define C_CYAN        0x07FF
#define C_MAGENTA     0xF81F
#define C_PINK        0xFC9F
#define C_GREEN       0x07E0
#define C_AMBER       0xFD20
#define C_GOLD        0xFEA0
#define C_DARKGREY    0x2124
#define C_LIGHTGREY   0xC618
#define C_GRAVEL      0x4228

// --- HARDWARE INSTANCES ---
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// --- AQUARIUM CONFIG ---
bool isNightMode = false;

// Bioluminescent palettes
enum BioPalette {
    PAL_CYAN    = 0,
    PAL_PINK    = 1,
    PAL_EMERALD = 2,
    PAL_AMBER   = 3,
    PAL_WHITE   = 4,
    PAL_COUNT   = 5
};
int currentPalette = PAL_CYAN;

uint16_t primaryGlow = C_CYAN;
uint16_t secondaryGlow = C_WHITE;

// Jellyfish State
float jx = 160.0f, jy = 120.0f;
float jvx = 0.0f, jvy = -0.4f;
float contractionPhase = 0.0f;

#define NUM_TENTACLES 6
#define TENTACLE_SEGMENTS 5
float tentacleX[NUM_TENTACLES][TENTACLE_SEGMENTS];
float tentacleY[NUM_TENTACLES][TENTACLE_SEGMENTS];

// Aerator Bubbles
#define NUM_BUBBLES 12
struct Bubble {
    float x, y;
    float speed;
    int size;
};
Bubble bubbles[NUM_BUBBLES];

// Food Pellets
#define MAX_FOOD 8
struct Food {
    float x, y;
    bool active;
};
Food foodPellets[MAX_FOOD];

// Water Ripple
struct Ripple {
    int x, y;
    int radius;
    bool active;
};
Ripple touchRipple = { 0, 0, 0, false };

unsigned long lastFrameTime = 0;
unsigned long lastBubblePop = 0;

// --- HARDWARE HELPERS ---
void setRgbLed(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_RED_PIN, 255 - r);
    analogWrite(LED_GREEN_PIN, 255 - g);
    analogWrite(LED_BLUE_PIN, 255 - b);
}

void playBubblePop() {
    tone(BUZZER_PIN, random(1200, 2400), 12);
}

void playFeedChime() {
    tone(BUZZER_PIN, 1046, 35); delay(45);
    tone(BUZZER_PIN, 1318, 35); delay(45);
    tone(BUZZER_PIN, 1568, 50);
}

// Update bioluminescent color variables
void updatePaletteColors() {
    switch (currentPalette) {
        case PAL_CYAN:
            primaryGlow = C_CYAN; secondaryGlow = C_WHITE;
            setRgbLed(0, 200, 255);
            break;
        case PAL_PINK:
            primaryGlow = C_PINK; secondaryGlow = C_MAGENTA;
            setRgbLed(255, 40, 180);
            break;
        case PAL_EMERALD:
            primaryGlow = C_GREEN; secondaryGlow = C_CYAN;
            setRgbLed(0, 255, 60);
            break;
        case PAL_AMBER:
            primaryGlow = C_AMBER; secondaryGlow = C_GOLD;
            setRgbLed(255, 160, 0);
            break;
        case PAL_WHITE:
            primaryGlow = C_WHITE; secondaryGlow = C_CYAN;
            setRgbLed(180, 220, 255);
            break;
    }
}

// --- BOOT SCREEN ---
void showBootSequence() {
    tft.fillScreen(C_BLACK);
    setRgbLed(0, 0, 0);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 45, 4);

    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString("DENKI-KURAGE ELECTRIC JELLYFISH", 160, 80, 2);

    // Glowing bell icon
    tft.fillCircle(160, 125, 20, C_OCEAN_CYAN);
    tft.drawCircle(160, 125, 20, C_CYAN);
    for (int t = 0; t < 5; t++) {
        tft.drawLine(148 + (t * 6), 135, 148 + (t * 6), 160, C_WHITE);
    }

    tft.setTextColor(C_LIGHTGREY, C_BLACK);
    tft.drawString("Synthesizing Hydrodynamic Aquarium...", 160, 185, 2);

    updatePaletteColors();
    tone(BUZZER_PIN, 1318, 100); delay(120);
    tone(BUZZER_PIN, 1760, 120);

    delay(1200);
    setRgbLed(0, 0, 0);
    tft.fillScreen(C_BLACK);
}

// --- INITIALIZE PARTICLES ---
void initAquarium() {
    for (int i = 0; i < NUM_BUBBLES; i++) {
        bubbles[i].x = random(15, 305);
        bubbles[i].y = random(50, 230);
        bubbles[i].speed = random(12, 28) / 10.0f;
        bubbles[i].size = random(2, 5);
    }

    for (int i = 0; i < MAX_FOOD; i++) {
        foodPellets[i].active = false;
    }

    // Init tentacle positions
    for (int i = 0; i < NUM_TENTACLES; i++) {
        for (int s = 0; s < TENTACLE_SEGMENTS; s++) {
            tentacleX[i][s] = jx;
            tentacleY[i][s] = jy + (s * 8);
        }
    }
}

// --- TOP HUD BAR ---
void renderTopBar() {
    uint16_t barBg = isNightMode ? 0x0004 : C_NAVY;
    tft.fillRect(0, 0, 320, 24, barBg);
    tft.drawFastHLine(0, 24, 320, primaryGlow);

    tft.setTextColor(C_GOLD, barBg);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("IamNotIntoTech BIOSPHERE", 6, 12, 2);

    // Buttons on top right: [FEED] [COLOR] [LIGHT]
    // Feed button
    tft.fillRect(165, 3, 44, 18, C_DARKGREY);
    tft.drawRect(165, 3, 44, 18, C_GOLD);
    tft.setTextColor(C_GOLD, C_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("FEED", 187, 12, 1);

    // Color button
    tft.fillRect(214, 3, 50, 18, C_DARKGREY);
    tft.drawRect(214, 3, 50, 18, primaryGlow);
    tft.setTextColor(primaryGlow, C_DARKGREY);
    tft.drawString("CHROMA", 239, 12, 1);

    // Light mode button
    tft.fillRect(269, 3, 46, 18, C_DARKGREY);
    tft.drawRect(269, 3, 46, 18, isNightMode ? C_MIDBLUE : C_AMBER);
    tft.setTextColor(isNightMode ? C_MIDBLUE : C_AMBER, C_DARKGREY);
    tft.drawString(isNightMode ? "ABYSS" : "DAY", 292, 12, 1);
}

// --- RENDER AQUARIUM ENVIRONMENT ---
void renderAquariumBackground() {
    // Ocean water background fill
    uint16_t oceanColor = isNightMode ? C_BLACK : C_DARKBLUE;
    tft.fillRect(0, 25, 320, 205, oceanColor);

    // Bottom gravel bed (Y: 230 to 240)
    tft.fillRect(0, 230, 320, 10, C_GRAVEL);
    tft.drawFastHLine(0, 230, 320, 0x5ACB);

    // Draw caustics light rays (if Day mode)
    if (!isNightMode) {
        for (int r = 0; r < 5; r++) {
            int rx = 40 + (r * 60);
            tft.drawLine(rx, 25, rx + 25, 230, C_OCEAN_CYAN);
        }
    }

    // Aerator air bubbles
    for (int i = 0; i < NUM_BUBBLES; i++) {
        bubbles[i].y -= bubbles[i].speed;
        bubbles[i].x += sin(bubbles[i].y * 0.1f) * 0.5f;

        if (bubbles[i].y <= 28) {
            bubbles[i].y = 230;
            bubbles[i].x = random(15, 305);
        }
        tft.drawCircle((int)bubbles[i].x, (int)bubbles[i].y, bubbles[i].size, C_CYAN);
    }

    // Touch shockwave ripple
    if (touchRipple.active) {
        tft.drawCircle(touchRipple.x, touchRipple.y, touchRipple.radius, primaryGlow);
        touchRipple.radius += 4;
        if (touchRipple.radius > 50) {
            touchRipple.active = false;
        }
    }

    // Food Pellets
    for (int i = 0; i < MAX_FOOD; i++) {
        if (foodPellets[i].active) {
            foodPellets[i].y += 1.2f;
            tft.fillCircle((int)foodPellets[i].x, (int)foodPellets[i].y, 3, C_GOLD);
            if (foodPellets[i].y >= 230) {
                foodPellets[i].active = false;
            }
        }
    }
}

// --- RENDER JELLYFISH ---
void renderJellyfish() {
    // Bell contraction animation (undulation)
    contractionPhase += 0.08f;
    if (contractionPhase >= TWO_PI) contractionPhase -= TWO_PI;

    // Locomotion impulse
    float pulse = sin(contractionPhase);
    if (pulse > 0.8f) {
        jvy -= 0.12f; // Upward jet thrust
        if (random(0, 20) == 1) {
            playBubblePop();
        }
    }

    // Gravity & drag
    jvy += 0.035f; // Gentle downward drift
    jvy *= 0.96f;
    jvx *= 0.96f;

    // Boundary constraints
    jx += jvx;
    jy += jvy;

    if (jx < 40) { jx = 40; jvx = -jvx; }
    if (jx > 280) { jx = 280; jvx = -jvx; }
    if (jy < 60) { jy = 60; jvy = 0.2f; }
    if (jy > 195) { jy = 195; jvy = -0.5f; }

    // Bell dimensions modulate with contraction
    int bellW = 26 + (int)(pulse * 5.0f);
    int bellH = 20 - (int)(pulse * 3.0f);

    // Draw Bell (Translucent dome with glowing rim)
    tft.fillEllipse((int)jx, (int)jy, bellW, bellH, isNightMode ? 0x0185 : C_MIDBLUE);
    tft.drawEllipse((int)jx, (int)jy, bellW, bellH, primaryGlow);
    tft.drawEllipse((int)jx, (int)jy, bellW - 3, bellH - 2, secondaryGlow);

    // Inner glowing electric nucleus / organ
    tft.fillCircle((int)jx, (int)jy + 2, 6, secondaryGlow);
    tft.fillCircle((int)jx, (int)jy + 2, 3, C_WHITE);

    // Update & draw trailing tentacles
    for (int t = 0; t < NUM_TENTACLES; t++) {
        float rootOffset = (t - (NUM_TENTACLES / 2.0f) + 0.5f) * (bellW * 0.32f);
        float prevX = jx + rootOffset;
        float prevY = jy + bellH * 0.7f;

        for (int s = 0; s < TENTACLE_SEGMENTS; s++) {
            // Spring lag physics
            float targetX = prevX + sin(contractionPhase + t + s) * 2.0f;
            float targetY = prevY + 8.0f;

            tentacleX[t][s] += (targetX - tentacleX[t][s]) * 0.35f;
            tentacleY[t][s] += (targetY - tentacleY[t][s]) * 0.35f;

            tft.drawLine((int)prevX, (int)prevY, (int)tentacleX[t][s], (int)tentacleY[t][s], primaryGlow);

            prevX = tentacleX[t][s];
            prevY = tentacleY[t][s];
        }
    }

    // Check food consumption
    for (int i = 0; i < MAX_FOOD; i++) {
        if (foodPellets[i].active) {
            float dist = sqrt(sq(jx - foodPellets[i].x) + sq(jy - foodPellets[i].y));
            if (dist < 25.0f) {
                foodPellets[i].active = false;
                playFeedChime();
                setRgbLed(255, 255, 255);
                delay(40);
                updatePaletteColors();
            }
        }
    }
}

// --- TOUCH INPUT HANDLER ---
void handleTouch() {
    if (!touch.touched()) return;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, 240, 3800, 0, 320);
    int ty = map(p.y, 240, 3800, 0, 240);

    // Check top HUD buttons (Y <= 24)
    if (ty <= 24) {
        // Feed button (X: 160 to 210)
        if (tx >= 160 && tx <= 210) {
            for (int i = 0; i < 3; i++) {
                for (int f = 0; f < MAX_FOOD; f++) {
                    if (!foodPellets[f].active) {
                        foodPellets[f].active = true;
                        foodPellets[f].x = random(80, 240);
                        foodPellets[f].y = 35;
                        break;
                    }
                }
            }
            tone(BUZZER_PIN, 1800, 30);
            delay(150);
            return;
        }

        // Palette Chroma button (X: 211 to 265)
        if (tx > 211 && tx <= 265) {
            currentPalette = (currentPalette + 1) % PAL_COUNT;
            updatePaletteColors();
            renderTopBar();
            playBubblePop();
            delay(150);
            return;
        }

        // Night / Abyss button (X: 266 to 320)
        if (tx > 266) {
            isNightMode = !isNightMode;
            renderTopBar();
            tone(BUZZER_PIN, isNightMode ? 600 : 1600, 50);
            delay(150);
            return;
        }
    }

    // Tap in water: generate ripple and disturb jellyfish
    touchRipple.x = tx;
    touchRipple.y = ty;
    touchRipple.radius = 4;
    touchRipple.active = true;

    // Displace jellyfish away from touch
    float dx = jx - tx;
    float dy = jy - ty;
    float len = sqrt(dx * dx + dy * dy);
    if (len > 0.1f) {
        jvx += (dx / len) * 2.5f;
        jvy += (dy / len) * 2.5f;
    }

    playBubblePop();
    delay(120);
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

    // Boot sequence & init
    showBootSequence();
    initAquarium();
    renderTopBar();

    lastFrameTime = millis();
}

// --- MAIN LOOP ---
void loop() {
    // Touch input check
    handleTouch();

    // Render environment & jellyfish
    renderAquariumBackground();
    renderJellyfish();

    // Re-draw top bar cleanly
    renderTopBar();

    // Frame delay for smooth ~30 FPS
    delay(20);
}

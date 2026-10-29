#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

/*
 * Project 50: Wet-Finger Multi-Zone Kitchen Hub & Meat Probe
 * Board: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware Pinouts:
 *  - ILI9341 TFT SPI: MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21
 *  - XPT2046 Touch VSPI: MOSI=13, MISO=12, SCK=14, CS=33, IRQ=36
 *  - HY2.0 Expansion Port: IO27 (OneWire Probe / Sim), IO22 (Aux GPIO)
 *  - RGB LED: IO4 (Red), IO16 (Green), IO17 (Blue) - Active LOW
 *  - Piezo Speaker: IO26 (DAC/PWM tone)
 *  - LDR Light Sensor: IO34 (ADC1_CH6)
 */

#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define HY20_PROBE_PIN 27
#define RGB_RED_PIN    4
#define RGB_GREEN_PIN  16
#define RGB_BLUE_PIN   17
#define SPEAKER_PIN    26
#define LDR_PIN        34

#define TS_MINX 250
#define TS_MAXX 3800
#define TS_MINY 250
#define TS_MAXY 3850

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

enum KitchenTab {
    TAB_TIMERS = 0,
    TAB_PROBE = 1,
    TAB_RECIPE = 2
};
KitchenTab currentTab = TAB_TIMERS;

// --- 4 Independent Cooking Timer Zones ---
struct CookingTimer {
    const char* label;
    int32_t remainingSec;
    bool isRunning;
    bool isAlarming;
    uint16_t headerColor;
};

CookingTimer timers[4] = {
    { "ZONE 1: OVEN",    25 * 60, false, false, 0xFDE0 }, // Orange
    { "ZONE 2: PAN SEAR", 8 * 60, false, false, 0xFA20 }, // Red-Orange
    { "ZONE 3: SIMMER",  45 * 60, false, false, 0x05BF }, // Cyan
    { "ZONE 4: PASTA",   10 * 60, false, false, 0x07E0 }  // Green
};
uint8_t selectedTimerIdx = 0;

// --- Meat Doneness Database ---
struct MeatDoneness {
    const char* label;
    float targetC;
};

const MeatDoneness donenessProfiles[] = {
    { "Beef Rare (52C / 125F)",        52.0f },
    { "Beef Med-Rare (57C / 135F)",    57.0f },
    { "Beef Medium (63C / 145F)",      63.0f },
    { "Pork / Well-Done (71C / 160F)", 71.0f },
    { "Poultry Safe (74C / 165F)",     74.0f }
};
const uint8_t totalProfiles = sizeof(donenessProfiles) / sizeof(donenessProfiles[0]);
uint8_t selectedProfileIdx = 1; // Default to Medium-Rare

float currentProbeTempC = 24.5f;
bool probeAlarming = false;

// --- Recipe Step Checklist ---
struct RecipeStep {
    const char* text;
    bool completed;
};

RecipeStep recipeSteps[5] = {
    { "1. Sear ribeye steak on cast iron (2m/side)", false },
    { "2. Insert probe into center of meat", false },
    { "3. Roast in oven until target internal 57C", false },
    { "4. Baste with rosemary butter & garlic", false },
    { "5. Rest 8 mins on cutting board before slicing", false }
};

// State timers
uint32_t secondTicker = 0;
uint32_t alarmToneTimer = 0;
bool alarmBeepToggle = false;

void setRGB(bool r, bool g, bool b) {
    digitalWrite(RGB_RED_PIN, r ? LOW : HIGH);
    digitalWrite(RGB_GREEN_PIN, g ? LOW : HIGH);
    digitalWrite(RGB_BLUE_PIN, b ? LOW : HIGH);
}

void playBeep(uint16_t freq, uint16_t durationMs) {
    ledcAttachPin(SPEAKER_PIN, 0);
    ledcWriteTone(0, freq);
    delay(durationMs);
    ledcWriteTone(0, 0);
    ledcDetachPin(SPEAKER_PIN);
}

void drawTopTabs() {
    tft.fillRect(0, 0, 320, 26, 0x18C3);
    tft.drawFastHLine(0, 26, 320, 0x4208);

    // Tab 1: 4-ZONE TIMERS (6..106)
    uint16_t t1Bg = (currentTab == TAB_TIMERS) ? 0x032E : 0x2124;
    tft.fillRect(6, 3, 100, 20, t1Bg);
    tft.drawRect(6, 3, 100, 20, (currentTab == TAB_TIMERS) ? TFT_CYAN : 0x52AA);
    tft.setTextColor((currentTab == TAB_TIMERS) ? TFT_CYAN : TFT_LIGHTGREY, t1Bg);
    tft.drawCentreString("4-ZONES", 56, 5, 2);

    // Tab 2: MEAT PROBE (110..210)
    uint16_t t2Bg = (currentTab == TAB_PROBE) ? 0x032E : 0x2124;
    tft.fillRect(110, 3, 100, 20, t2Bg);
    tft.drawRect(110, 3, 100, 20, (currentTab == TAB_PROBE) ? TFT_CYAN : 0x52AA);
    tft.setTextColor((currentTab == TAB_PROBE) ? TFT_CYAN : TFT_LIGHTGREY, t2Bg);
    tft.drawCentreString("MEAT PROBE", 160, 5, 2);

    // Tab 3: RECIPE STEPS (214..314)
    uint16_t t3Bg = (currentTab == TAB_RECIPE) ? 0x032E : 0x2124;
    tft.fillRect(214, 3, 100, 20, t3Bg);
    tft.drawRect(214, 3, 100, 20, (currentTab == TAB_RECIPE) ? TFT_CYAN : 0x52AA);
    tft.setTextColor((currentTab == TAB_RECIPE) ? TFT_CYAN : TFT_LIGHTGREY, t3Bg);
    tft.drawCentreString("RECIPE", 264, 5, 2);
}

void renderTimersTab() {
    // 2x2 Grid of Big Touch Zones
    // Zone 0: Top Left (6, 30, 150, 86)
    // Zone 1: Top Right (164, 30, 150, 86)
    // Zone 2: Bottom Left (6, 122, 150, 86)
    // Zone 3: Bottom Right (164, 122, 150, 86)

    const int zonePos[4][2] = {
        { 6, 30 },
        { 164, 30 },
        { 6, 122 },
        { 164, 122 }
    };

    for (int i = 0; i < 4; i++) {
        int x = zonePos[i][0];
        int y = zonePos[i][1];

        bool isSel = (selectedTimerIdx == i);
        uint16_t cardBg = timers[i].isAlarming ? 0x8000 : (timers[i].isRunning ? 0x0208 : 0x10A2);
        uint16_t borderCol = isSel ? TFT_WHITE : (timers[i].isAlarming ? TFT_RED : (timers[i].isRunning ? 0x07E0 : 0x3186));

        tft.fillRect(x, y, 150, 86, cardBg);
        tft.drawRect(x, y, 150, 86, borderCol);
        if (isSel) tft.drawRect(x + 1, y + 1, 148, 84, borderCol);

        // Header label
        tft.setTextColor(timers[i].headerColor, cardBg);
        tft.drawString(timers[i].label, x + 8, y + 6, 2);

        // Big Countdown Format MM:SS
        int mins = timers[i].remainingSec / 60;
        int secs = timers[i].remainingSec % 60;
        char timeStr[16];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", mins, secs);

        tft.setTextColor(timers[i].isAlarming ? TFT_WHITE : (timers[i].isRunning ? 0x07E0 : TFT_LIGHTGREY), cardBg);
        tft.drawCentreString(timeStr, x + 75, y + 26, 4);

        // Mini status bar
        tft.setTextColor(0x7BEF, cardBg);
        const char* st = timers[i].isAlarming ? "ALARM! TAP RESET" : (timers[i].isRunning ? "RUNNING..." : "PAUSED");
        tft.drawCentreString(st, x + 75, y + 62, 1);
    }

    // Bottom Action Bar (Large Wet-Finger Target Buttons)
    tft.fillRect(0, 212, 320, 28, 0x18C3);
    tft.drawFastHLine(0, 211, 320, 0x4208);

    // START / STOP Button (6..86)
    CookingTimer* sel = &timers[selectedTimerIdx];
    uint16_t runBtnColor = sel->isRunning ? 0x8000 : 0x0400;
    tft.fillRect(6, 214, 80, 24, runBtnColor);
    tft.drawRect(6, 214, 80, 24, TFT_WHITE);
    tft.setTextColor(TFT_WHITE, runBtnColor);
    tft.drawCentreString(sel->isRunning ? "PAUSE" : "START", 46, 218, 2);

    // +1 MIN Button (92..152)
    tft.fillRect(92, 214, 60, 24, 0x2124);
    tft.drawRect(92, 214, 60, 24, 0x52AA);
    tft.setTextColor(TFT_WHITE, 0x2124);
    tft.drawCentreString("+1m", 122, 218, 2);

    // +5 MIN Button (158..218)
    tft.fillRect(158, 214, 60, 24, 0x2124);
    tft.drawRect(158, 214, 60, 24, 0x52AA);
    tft.setTextColor(TFT_WHITE, 0x2124);
    tft.drawCentreString("+5m", 188, 218, 2);

    // RESET Button (224..314)
    tft.fillRect(224, 214, 90, 24, 0x2124);
    tft.drawRect(224, 214, 90, 24, 0x52AA);
    tft.setTextColor(TFT_WHITE, 0x2124);
    tft.drawCentreString("RESET", 269, 218, 2);
}

void renderProbeTab() {
    tft.fillRect(0, 27, 320, 185, TFT_BLACK);

    float targetC = donenessProfiles[selectedProfileIdx].targetC;
    float currentF = currentProbeTempC * 1.8f + 32.0f;
    float targetF = targetC * 1.8f + 32.0f;

    // Card 1: Target Profile Card (6..314, Y: 30..64)
    tft.fillRect(6, 30, 308, 34, 0x10A2);
    tft.drawRect(6, 30, 308, 34, 0x3186);
    tft.setTextColor(TFT_WHITE, 0x10A2);
    tft.drawString("Target Profile (Tap to Change):", 12, 34, 1);
    tft.setTextColor(TFT_YELLOW, 0x10A2);
    tft.drawString(donenessProfiles[selectedProfileIdx].label, 12, 46, 2);

    // Card 2: Giant Current Temperature Readout (6..314, Y: 68..150)
    uint16_t tempCardBg = probeAlarming ? 0x8000 : 0x10A2;
    tft.fillRect(6, 68, 308, 82, tempCardBg);
    tft.drawRect(6, 68, 308, 82, probeAlarming ? TFT_RED : 0x3186);

    tft.setTextColor(TFT_LIGHTGREY, tempCardBg);
    tft.drawString("Internal Meat Core Temp", 14, 72, 2);

    char tempBuf[32];
    snprintf(tempBuf, sizeof(tempBuf), "%.1f C", currentProbeTempC);
    tft.setTextColor(probeAlarming ? TFT_WHITE : 0x07E0, tempCardBg);
    tft.drawRightString(tempBuf, 302, 74, 6);

    char fBuf[32];
    snprintf(fBuf, sizeof(fBuf), "(%.1f F) / Target: %.0fC", currentF, targetC);
    tft.setTextColor(0x7BEF, tempCardBg);
    tft.drawString(fBuf, 14, 126, 2);

    // Card 3: Doneness Progress Bar (Y: 154..172)
    float progress = constrain((currentProbeTempC - 20.0f) / (targetC - 20.0f), 0.0f, 1.0f);
    int barW = (int)(progress * 304.0f);
    tft.fillRect(6, 154, 308, 18, 0x18E3);
    tft.drawRect(6, 154, 308, 18, 0x39E7);

    uint16_t pBarColor = (progress >= 1.0f) ? TFT_RED : (progress > 0.8f ? 0xFDE0 : 0x05BF);
    tft.fillRect(8, 156, barW, 14, pBarColor);

    // Bottom Action Bar
    tft.fillRect(0, 212, 320, 28, 0x18C3);
    tft.drawFastHLine(0, 211, 320, 0x4208);

    // Cycle Doneness Button
    tft.fillRect(6, 214, 140, 24, 0x032E);
    tft.drawRect(6, 214, 140, 24, 0x05BF);
    tft.setTextColor(TFT_WHITE, 0x032E);
    tft.drawCentreString("CHANGE DONENESS", 76, 218, 2);

    // Sim Temp +/-
    tft.fillRect(160, 214, 60, 24, 0x2124);
    tft.drawRect(160, 214, 60, 24, 0x52AA);
    tft.setTextColor(TFT_WHITE, 0x2124);
    tft.drawCentreString("-2C", 190, 218, 2);

    tft.fillRect(230, 214, 60, 24, 0x2124);
    tft.drawRect(230, 214, 60, 24, 0x52AA);
    tft.setTextColor(TFT_WHITE, 0x2124);
    tft.drawCentreString("+2C", 260, 218, 2);
}

void renderRecipeTab() {
    tft.fillRect(0, 27, 320, 213, TFT_BLACK);

    tft.fillRect(6, 30, 308, 22, 0x18C3);
    tft.setTextColor(TFT_WHITE, 0x18C3);
    tft.drawString("Reverse Sear Ribeye Protocol", 12, 34, 2);

    for (int i = 0; i < 5; i++) {
        int y = 56 + i * 35;
        uint16_t rowBg = recipeSteps[i].completed ? 0x0280 : 0x10A2;
        tft.fillRect(6, y, 308, 30, rowBg);
        tft.drawRect(6, y, 308, 30, recipeSteps[i].completed ? 0x07E0 : 0x3186);

        // Checkbox square
        int cbX = 14, cbY = y + 7;
        tft.fillRect(cbX, cbY, 16, 16, recipeSteps[i].completed ? 0x07E0 : TFT_BLACK);
        tft.drawRect(cbX, cbY, 16, 16, TFT_WHITE);
        if (recipeSteps[i].completed) {
            tft.setTextColor(TFT_BLACK, 0x07E0);
            tft.drawCentreString("X", cbX + 8, cbY + 1, 2);
        }

        tft.setTextColor(recipeSteps[i].completed ? 0x9E7F : TFT_WHITE, rowBg);
        tft.drawString(recipeSteps[i].text, 36, y + 8, 1);
    }
}

void sampleMeatProbe() {
    // Check if probe temperature reaches target
    float targetC = donenessProfiles[selectedProfileIdx].targetC;
    probeAlarming = (currentProbeTempC >= targetC);
}

void handleTouch() {
    if (!ts.touched()) return;

    TS_Point p = ts.getPoint();
    int touchX = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int touchY = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    touchX = constrain(touchX, 0, 319);
    touchY = constrain(touchY, 0, 239);

    // Top Tabs (Y: 0..26)
    if (touchY <= 26) {
        if (touchX >= 6 && touchX <= 106) {
            currentTab = TAB_TIMERS;
            playBeep(1800, 25);
            drawTopTabs();
            renderTimersTab();
            delay(200);
            return;
        }
        if (touchX >= 110 && touchX <= 210) {
            currentTab = TAB_PROBE;
            playBeep(1800, 25);
            drawTopTabs();
            renderProbeTab();
            delay(200);
            return;
        }
        if (touchX >= 214 && touchX <= 314) {
            currentTab = TAB_RECIPE;
            playBeep(1800, 25);
            drawTopTabs();
            renderRecipeTab();
            delay(200);
            return;
        }
    }

    // Timers Tab Touch Handling
    if (currentTab == TAB_TIMERS) {
        // Tap on 2x2 Zone Cards to select or silence
        if (touchY >= 30 && touchY <= 116) {
            if (touchX >= 6 && touchX <= 156) {
                selectedTimerIdx = 0;
                timers[0].isAlarming = false;
                playBeep(1500, 20);
                renderTimersTab();
                delay(150);
                return;
            }
            if (touchX >= 164 && touchX <= 314) {
                selectedTimerIdx = 1;
                timers[1].isAlarming = false;
                playBeep(1500, 20);
                renderTimersTab();
                delay(150);
                return;
            }
        } else if (touchY >= 122 && touchY <= 208) {
            if (touchX >= 6 && touchX <= 156) {
                selectedTimerIdx = 2;
                timers[2].isAlarming = false;
                playBeep(1500, 20);
                renderTimersTab();
                delay(150);
                return;
            }
            if (touchX >= 164 && touchX <= 314) {
                selectedTimerIdx = 3;
                timers[3].isAlarming = false;
                playBeep(1500, 20);
                renderTimersTab();
                delay(150);
                return;
            }
        }

        // Bottom Controls
        if (touchY >= 212) {
            CookingTimer* sel = &timers[selectedTimerIdx];
            // Start / Pause (6..86)
            if (touchX >= 6 && touchX <= 86) {
                sel->isRunning = !sel->isRunning;
                sel->isAlarming = false;
                playBeep(sel->isRunning ? 2000 : 1000, 30);
                renderTimersTab();
                delay(200);
                return;
            }
            // +1m (92..152)
            if (touchX >= 92 && touchX <= 152) {
                sel->remainingSec += 60;
                sel->isAlarming = false;
                playBeep(1600, 20);
                renderTimersTab();
                delay(150);
                return;
            }
            // +5m (158..218)
            if (touchX >= 158 && touchX <= 218) {
                sel->remainingSec += 300;
                sel->isAlarming = false;
                playBeep(1800, 20);
                renderTimersTab();
                delay(150);
                return;
            }
            // Reset (224..314)
            if (touchX >= 224 && touchX <= 314) {
                sel->isRunning = false;
                sel->isAlarming = false;
                sel->remainingSec = (selectedTimerIdx == 0 ? 25 : (selectedTimerIdx == 1 ? 8 : (selectedTimerIdx == 2 ? 45 : 10))) * 60;
                playBeep(1200, 30);
                renderTimersTab();
                delay(200);
                return;
            }
        }
    }

    // Probe Tab Touch Handling
    if (currentTab == TAB_PROBE) {
        // Change Doneness Button or top card (6..146 at bottom, or Y: 30..64)
        if ((touchY >= 212 && touchX >= 6 && touchX <= 146) || (touchY >= 30 && touchY <= 64)) {
            selectedProfileIdx = (selectedProfileIdx + 1) % totalProfiles;
            playBeep(1700, 30);
            sampleMeatProbe();
            renderProbeTab();
            delay(200);
            return;
        }
        // Sim Temp -2C (160..220)
        if (touchY >= 212 && touchX >= 160 && touchX <= 220) {
            currentProbeTempC = max(10.0f, currentProbeTempC - 2.0f);
            playBeep(900, 20);
            sampleMeatProbe();
            renderProbeTab();
            delay(150);
            return;
        }
        // Sim Temp +2C (230..290)
        if (touchY >= 212 && touchX >= 230 && touchX <= 290) {
            currentProbeTempC = min(100.0f, currentProbeTempC + 2.0f);
            playBeep(2100, 20);
            sampleMeatProbe();
            renderProbeTab();
            delay(150);
            return;
        }
    }

    // Recipe Tab Touch Handling
    if (currentTab == TAB_RECIPE && touchY >= 56 && touchY <= 230) {
        int idx = (touchY - 56) / 35;
        if (idx >= 0 && idx < 5) {
            recipeSteps[idx].completed = !recipeSteps[idx].completed;
            playBeep(recipeSteps[idx].completed ? 2200 : 1200, 25);
            renderRecipeTab();
            delay(200);
            return;
        }
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(RGB_RED_PIN, OUTPUT);
    pinMode(RGB_GREEN_PIN, OUTPUT);
    pinMode(RGB_BLUE_PIN, OUTPUT);
    pinMode(LDR_PIN, INPUT);
    pinMode(HY20_PROBE_PIN, INPUT_PULLUP);
    setRGB(false, false, false);

    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(touchSPI);
    ts.setRotation(1);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    drawTopTabs();
    renderTimersTab();
    playBeep(1700, 80);
    secondTicker = millis();
}

void loop() {
    handleTouch();

    // 1-second countdown cycle
    if (millis() - secondTicker >= 1000) {
        secondTicker = millis();
        bool anyRunning = false;
        bool anyAlarm = probeAlarming;

        for (int i = 0; i < 4; i++) {
            if (timers[i].isRunning) {
                anyRunning = true;
                if (timers[i].remainingSec > 0) {
                    timers[i].remainingSec--;
                    if (timers[i].remainingSec == 0) {
                        timers[i].isRunning = false;
                        timers[i].isAlarming = true;
                    }
                }
            }
            if (timers[i].isAlarming) anyAlarm = true;
        }

        // Live probe reading slow climb simulation if in roasting
        if (timers[0].isRunning) {
            currentProbeTempC += 0.2f;
            sampleMeatProbe();
        }

        // Update active view
        if (currentTab == TAB_TIMERS) renderTimersTab();
        else if (currentTab == TAB_PROBE) renderProbeTab();

        // Alarm management
        if (anyAlarm) {
            alarmBeepToggle = !alarmBeepToggle;
            setRGB(alarmBeepToggle, false, false); // Flashing Red
            if (alarmBeepToggle) playBeep(2700, 80);
        } else if (anyRunning) {
            setRGB(false, true, false); // Solid Green when actively cooking
        } else {
            setRGB(false, false, true); // Idle Blue
        }
    }

    delay(10);
}

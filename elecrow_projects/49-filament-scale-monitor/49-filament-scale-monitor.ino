/*
 * Project 49: Smart 3D Printer Filament Scale & Spool Monitor
 * Board: Elecrow ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware Pinouts:
 *  - ILI9341 TFT SPI: MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21
 *  - XPT2046 Touch VSPI: MOSI=32, MISO=39, CLK=25, CS=33
 *  - HY2.0 Load Cell Port: IO27 (HX711 DT), IO22 (HX711 SCK)
 *  - RGB LED: IO4 (Red), IO16 (Green), IO17 (Blue) - Active LOW
 *  - Piezo Speaker: IO26 (Buzzer tone)
 *  - LDR Light Sensor: IO34 (ADC1)
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define TFT_BL_PIN    21
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33
#define TOUCH_IRQ     36

#define HX711_DT_PIN  27
#define HX711_SCK_PIN 22

#define RGB_RED_PIN   4
#define RGB_GREEN_PIN 16
#define RGB_BLUE_PIN  17
#define SPEAKER_PIN   26
#define LDR_PIN       34

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();

// Spool Brand Tare Database
struct SpoolBrand {
    const char* name;
    float tareGrams;
};

const SpoolBrand brands[] = {
    { "Bambu Lab", 250.0f },
    { "Prusament", 208.0f },
    { "Polymaker", 145.0f },
    { "eSUN Spool", 220.0f },
    { "Sunlu Plastic", 140.0f },
    { "Cardboard Core", 135.0f }
};
const uint8_t totalBrands = sizeof(brands) / sizeof(brands[0]);
uint8_t selectedBrandIdx = 0;

// Material Database
struct FilamentMaterial {
    const char* name;
    float gramsPerMeter;
    uint16_t themeColor;
};

const FilamentMaterial materials[] = {
    { "PLA",  2.98f, 0x05BF }, // Cyan
    { "PETG", 3.05f, 0xFDE0 }, // Orange
    { "ABS",  2.50f, 0xF81F }, // Magenta
    { "TPU",  2.91f, 0x07E0 }  // Green
};
const uint8_t totalMaterials = sizeof(materials) / sizeof(materials[0]);
uint8_t selectedMaterialIdx = 0;

// Measurement state
float currentGrossGrams = 785.0f;
float userZeroOffset = 0.0f;
bool hardwareHX711Found = false;
uint32_t lastScaleReadTime = 0;
uint32_t lastAlarmBlinkTime = 0;
bool alarmLedState = false;
unsigned long lastTouchTime = 0;

void setRGB(bool r, bool g, bool b) {
    digitalWrite(RGB_RED_PIN, r ? LOW : HIGH);
    digitalWrite(RGB_GREEN_PIN, g ? LOW : HIGH);
    digitalWrite(RGB_BLUE_PIN, b ? LOW : HIGH);
}

void playBeep(uint16_t freq, uint16_t durationMs) {
    tone(SPEAKER_PIN, freq, durationMs);
    delay(durationMs);
    noTone(SPEAKER_PIN);
}

// Bitbang HX711 24-bit ADC reading
bool readHX711Raw(int32_t &value) {
    if (digitalRead(HX711_DT_PIN) == HIGH) {
        return false;
    }
    int32_t count = 0;
    for (int i = 0; i < 24; i++) {
        digitalWrite(HX711_SCK_PIN, HIGH);
        delayMicroseconds(1);
        count = count << 1;
        digitalWrite(HX711_SCK_PIN, LOW);
        delayMicroseconds(1);
        if (digitalRead(HX711_DT_PIN)) count++;
    }
    digitalWrite(HX711_SCK_PIN, HIGH);
    delayMicroseconds(1);
    digitalWrite(HX711_SCK_PIN, LOW);
    delayMicroseconds(1);

    if (count & 0x800000) count |= 0xFF000000;
    value = count;
    return true;
}

void drawStaticDashboard() {
    tft.fillRect(0, 0, 320, 26, 0x18C3);
    tft.drawFastHLine(0, 26, 320, 0x4208);

    tft.setTextColor(TFT_WHITE, 0x18C3);
    tft.setTextSize(1);
    tft.drawString("@IamNotIntoTech", 8, 8, 1);

    // Hardware status indicator (Top Right)
    tft.setTextColor(hardwareHX711Found ? 0x07E0 : 0xFDE0, 0x18C3);
    tft.drawRightString(hardwareHX711Found ? "HX711: ONLINE" : "MODE: SIM/TEST", 314, 8, 1);

    // Spool Gauge Background (Y: 104..120, width 308)
    tft.fillRect(6, 104, 308, 16, 0x18E3);
    tft.drawRect(6, 104, 308, 16, 0x39E7);

    // Bottom Action Bar
    tft.fillRect(0, 214, 320, 26, 0x18C3);
    tft.drawFastHLine(0, 213, 320, 0x4208);

    // Tare Button (6..66)
    tft.fillRect(6, 216, 60, 22, 0x8000);
    tft.drawRect(6, 216, 60, 22, TFT_RED);
    tft.setTextColor(TFT_WHITE, 0x8000);
    tft.drawCentreString("TARE", 36, 219, 2);

    // Spool Brand Button (72..152)
    tft.fillRect(72, 216, 80, 22, 0x2124);
    tft.drawRect(72, 216, 80, 22, 0x52AA);
    tft.setTextColor(TFT_WHITE, 0x2124);
    tft.drawCentreString("SPOOL", 112, 219, 2);

    // Material Button (158..238)
    tft.fillRect(158, 216, 80, 22, 0x2124);
    tft.drawRect(158, 216, 80, 22, 0x52AA);
    tft.setTextColor(TFT_WHITE, 0x2124);
    tft.drawCentreString("MATERIAL", 198, 219, 2);

    // Sim Adjust (+/- 50g) (244..314)
    tft.fillRect(244, 216, 32, 22, 0x032E);
    tft.drawRect(244, 216, 32, 22, 0x05BF);
    tft.setTextColor(TFT_WHITE, 0x032E);
    tft.drawCentreString("-", 260, 219, 2);

    tft.fillRect(282, 216, 32, 22, 0x032E);
    tft.drawRect(282, 216, 32, 22, 0x05BF);
    tft.setTextColor(TFT_WHITE, 0x032E);
    tft.drawCentreString("+", 298, 219, 2);
}

void renderDashboard() {
    float spoolTare = brands[selectedBrandIdx].tareGrams;
    float netGrams = max(0.0f, (currentGrossGrams - spoolTare - userZeroOffset));
    float remainingPct = constrain((netGrams / 1000.0f) * 100.0f, 0.0f, 100.0f);
    float remainingMeters = netGrams / materials[selectedMaterialIdx].gramsPerMeter;
    
    // Estimate print hours remaining at ~12g/hr average 3D print rate
    float estHours = netGrams / 12.0f;
    int hPart = (int)estHours;
    int mPart = (int)((estHours - hPart) * 60.0f);

    // 1. Hero Card: Net Filament Weight (Y: 30..100)
    tft.fillRect(6, 30, 308, 70, 0x10A2);
    tft.drawRect(6, 30, 308, 70, 0x3186);

    // Brand and Material Tags
    tft.setTextColor(TFT_LIGHTGREY, 0x10A2);
    tft.drawString(brands[selectedBrandIdx].name, 12, 36, 2);

    uint16_t matColor = materials[selectedMaterialIdx].themeColor;
    tft.fillRect(115, 36, 52, 18, matColor);
    tft.setTextColor(TFT_BLACK, matColor);
    tft.drawCentreString(materials[selectedMaterialIdx].name, 141, 37, 2);

    // Weight Readout
    char wBuf[32];
    snprintf(wBuf, sizeof(wBuf), "%.0f g NET", netGrams);
    tft.setTextColor(netGrams < 60.0f ? TFT_RED : TFT_WHITE, 0x10A2);
    tft.drawRightString(wBuf, 304, 40, 4);

    // Subtitle Tare notice
    char tSub[32];
    snprintf(tSub, sizeof(tSub), "Spool Tare: %.0fg", spoolTare);
    tft.setTextColor(0x7BEF, 0x10A2);
    tft.drawString(tSub, 12, 78, 2);

    // 2. Progress Gauge Bar (Y: 104..120)
    int fillW = (int)((remainingPct / 100.0f) * 304.0f);
    uint16_t gaugeColor = (netGrams < 60.0f) ? TFT_RED : (netGrams < 200.0f ? 0xFDE0 : 0x07E0);
    tft.fillRect(8, 106, fillW, 12, gaugeColor);
    tft.fillRect(8 + fillW, 106, 304 - fillW, 12, 0x18E3);

    // 3. Telemetry Matrix Cards (Y: 126..208)
    // Card A: Remaining Length in Meters (6..106)
    tft.fillRect(6, 126, 98, 82, 0x10A2);
    tft.drawRect(6, 126, 98, 82, 0x3186);
    tft.setTextColor(TFT_LIGHTGREY, 0x10A2);
    tft.drawString("Length", 12, 130, 2);
    char lenBuf[16];
    snprintf(lenBuf, sizeof(lenBuf), "%.1f", remainingMeters);
    tft.setTextColor(TFT_CYAN, 0x10A2);
    tft.drawCentreString(lenBuf, 55, 150, 4);
    tft.setTextColor(0x7BEF, 0x10A2);
    tft.drawCentreString("meters", 55, 184, 2);

    // Card B: Estimated Print Time (110..210)
    tft.fillRect(110, 126, 100, 82, 0x10A2);
    tft.drawRect(110, 126, 100, 82, 0x3186);
    tft.setTextColor(TFT_LIGHTGREY, 0x10A2);
    tft.drawString("Print Time", 116, 130, 2);
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%dh%02dm", hPart, mPart);
    tft.setTextColor(0xFDE0, 0x10A2);
    tft.drawCentreString(timeBuf, 160, 152, 4);
    tft.setTextColor(0x7BEF, 0x10A2);
    tft.drawCentreString("@ 12g/hr avg", 160, 184, 1);

    // Card C: Gross Total Weight (216..314)
    tft.fillRect(216, 126, 98, 82, 0x10A2);
    tft.drawRect(216, 126, 98, 82, 0x3186);
    tft.setTextColor(TFT_LIGHTGREY, 0x10A2);
    tft.drawString("Gross Scale", 222, 130, 2);
    char grossBuf[16];
    snprintf(grossBuf, sizeof(grossBuf), "%.0f g", currentGrossGrams);
    tft.setTextColor(TFT_WHITE, 0x10A2);
    tft.drawCentreString(grossBuf, 265, 150, 4);
    char pctBuf[16];
    snprintf(pctBuf, sizeof(pctBuf), "%.0f%% full", remainingPct);
    tft.setTextColor(0x07E0, 0x10A2);
    tft.drawCentreString(pctBuf, 265, 184, 2);

    // LED & Beeper Alerts
    if (netGrams < 50.0f) {
        if (millis() - lastAlarmBlinkTime >= 500) {
            lastAlarmBlinkTime = millis();
            alarmLedState = !alarmLedState;
            setRGB(alarmLedState, false, false);
            if (alarmLedState) playBeep(2500, 40);
        }
    } else if (netGrams < 150.0f) {
        setRGB(true, true, false); // Amber Warning
    } else {
        setRGB(false, true, false); // Green Safe
    }
}

void sampleScale() {
    int32_t rawVal = 0;
    if (readHX711Raw(rawVal)) {
        hardwareHX711Found = true;
        currentGrossGrams = (float)rawVal / 420.0f;
    } else {
        hardwareHX711Found = false;
        currentGrossGrams += (random(-5, 6) * 0.1f);
    }
}

void handleTouch() {
    if (!ts.touched()) return;

    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
        int touchX = map(p.x, 240, 3800, 0, 320);
        int touchY = map(p.y, 240, 3800, 0, 240);
        touchX = constrain(touchX, 0, 319);
        touchY = constrain(touchY, 0, 239);

        // Bottom Action Bar (y >= 214)
        if (touchY >= 214) {
            if (millis() - lastTouchTime > 180) {
                lastTouchTime = millis();
                // Tare Button (6..66)
                if (touchX >= 6 && touchX <= 66) {
                    userZeroOffset = currentGrossGrams - brands[selectedBrandIdx].tareGrams;
                    playBeep(1800, 40);
                    renderDashboard();
                    return;
                }
                // Spool Brand Cycle (72..152)
                if (touchX >= 72 && touchX <= 152) {
                    selectedBrandIdx = (selectedBrandIdx + 1) % totalBrands;
                    playBeep(1400, 30);
                    drawStaticDashboard();
                    renderDashboard();
                    return;
                }
                // Material Cycle (158..238)
                if (touchX >= 158 && touchX <= 238) {
                    selectedMaterialIdx = (selectedMaterialIdx + 1) % totalMaterials;
                    playBeep(1600, 30);
                    drawStaticDashboard();
                    renderDashboard();
                    return;
                }
                // Sim Weight -50g (244..276)
                if (touchX >= 244 && touchX <= 276) {
                    currentGrossGrams = max(50.0f, currentGrossGrams - 50.0f);
                    playBeep(900, 25);
                    renderDashboard();
                    return;
                }
                // Sim Weight +50g (282..314)
                if (touchX >= 282 && touchX <= 314) {
                    currentGrossGrams = min(2500.0f, currentGrossGrams + 50.0f);
                    playBeep(2100, 25);
                    renderDashboard();
                    return;
                }
            }
        }
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(RGB_RED_PIN, OUTPUT);
    pinMode(RGB_GREEN_PIN, OUTPUT);
    pinMode(RGB_BLUE_PIN, OUTPUT);
    pinMode(LDR_PIN, INPUT);
    setRGB(false, false, false);

    pinMode(SPEAKER_PIN, OUTPUT);
    digitalWrite(SPEAKER_PIN, LOW);

    pinMode(TFT_BL_PIN, OUTPUT);
    digitalWrite(TFT_BL_PIN, HIGH);

    pinMode(HX711_DT_PIN, INPUT);
    pinMode(HX711_SCK_PIN, OUTPUT);
    digitalWrite(HX711_SCK_PIN, LOW);

    // Dedicated VSPI for Touch Controller (Pins 25, 39, 32, 33)
    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    ts.begin(touchSPI);
    ts.setRotation(1);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    sampleScale();
    drawStaticDashboard();
    renderDashboard();
    playBeep(1650, 70);
    lastScaleReadTime = millis();
    Serial.println(F("[SYSTEM] Smart 3D Printer Filament Scale & Spool Monitor Initialized."));
}

void loop() {
    handleTouch();

    if (millis() - lastScaleReadTime >= 150) {
        lastScaleReadTime = millis();
        sampleScale();
        renderDashboard();
    }

    delay(10);
}


#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <math.h>

/*
 * Project 46: Boids Artificial Life & 4D Tesseract Physics Sandbox
 * Board: Elecrow CrowPanel 2.8" CYD (ESP32-2432S028R)
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware Pinouts:
 *  - ILI9341 TFT SPI: MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21
 *  - XPT2046 Touch VSPI: MOSI=13, MISO=12, SCK=14, CS=33, IRQ=36
 *  - RGB LED: IO4 (Red), IO16 (Green), IO17 (Blue) - Active LOW
 *  - Piezo Speaker: IO26 (DAC/PWM tone)
 *  - LDR Light Sensor: IO34 (ADC1_CH6)
 */

#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define RGB_RED_PIN   4
#define RGB_GREEN_PIN 16
#define RGB_BLUE_PIN  17
#define SPEAKER_PIN   26
#define LDR_PIN       34

#define TS_MINX 250
#define TS_MAXX 3800
#define TS_MINY 250
#define TS_MAXY 3850

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Operating Modes
enum SimulationMode {
    MODE_BOIDS = 0,
    MODE_TESSERACT = 1
};
SimulationMode currentMode = MODE_BOIDS;

// --- Boids Flocking Parameters ---
#define NUM_BOIDS 42
struct Boid {
    float x, y;
    float vx, vy;
    float prev_x, prev_y;
};
Boid boids[NUM_BOIDS];

bool attractMode = false; // false = Repulsor (Predator), true = Attractor (Food)
bool hasTouchTarget = false;
float touchTargetX = 0, touchTargetY = 0;
uint32_t targetExpiryTime = 0;

// --- 4D Tesseract Parameters ---
struct Point4D {
    float x, y, z, w;
};

struct Point3D {
    float x, y, z;
};

struct Point2D {
    int16_t x, y;
};

// 16 vertices of a 4D unit tesseract
Point4D tesseractVertices[16];
Point2D projectedVertices[16];
Point2D prevProjectedVertices[16];

// 32 edges of a 4D tesseract
const uint8_t tesseractEdges[32][2] = {
    // Inner cube edges
    {0,1}, {1,3}, {3,2}, {2,0},
    {4,5}, {5,7}, {7,6}, {6,4},
    {0,4}, {1,5}, {2,6}, {3,7},
    // Outer cube edges
    {8,9}, {9,11}, {11,10}, {10,8},
    {12,13}, {13,15}, {15,14}, {14,12},
    {8,12}, {9,13}, {10,14}, {11,15},
    // Connecting edges between inner and outer cubes
    {0,8}, {1,9}, {2,10}, {3,11},
    {4,12}, {5,13}, {6,14}, {7,15}
};

float rotAngleXW = 0.0f;
float rotAngleYZ = 0.0f;
float rotAngleXY = 0.0f;
float rotSpeed = 0.035f;

// Performance metrics
uint32_t frameCount = 0;
uint32_t fpsTimer = 0;
uint16_t currentFPS = 0;

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

void initBoids() {
    for (int i = 0; i < NUM_BOIDS; i++) {
        boids[i].x = random(30, 290);
        boids[i].y = random(40, 200);
        boids[i].prev_x = boids[i].x;
        boids[i].prev_y = boids[i].y;
        float angle = random(0, 360) * 0.0174533f;
        float speed = random(15, 30) * 0.1f;
        boids[i].vx = cos(angle) * speed;
        boids[i].vy = sin(angle) * speed;
    }
}

void initTesseract() {
    int idx = 0;
    for (int x = -1; x <= 1; x += 2) {
        for (int y = -1; y <= 1; y += 2) {
            for (int z = -1; z <= 1; z += 2) {
                for (int w = -1; w <= 1; w += 2) {
                    tesseractVertices[idx].x = (float)x;
                    tesseractVertices[idx].y = (float)y;
                    tesseractVertices[idx].z = (float)z;
                    tesseractVertices[idx].w = (float)w;
                    prevProjectedVertices[idx].x = 160;
                    prevProjectedVertices[idx].y = 120;
                    idx++;
                }
            }
        }
    }
}

void drawUIStatic() {
    tft.fillRect(0, 0, 320, 24, 0x18C3);
    tft.drawFastHLine(0, 24, 320, 0x4208);
    
    tft.setTextColor(TFT_WHITE, 0x18C3);
    tft.drawString("IamNotIntoTech", 6, 5, 2);
    
    // Mode Switch Button (Top right)
    tft.fillRect(205, 2, 110, 20, 0x032E);
    tft.drawRect(205, 2, 110, 20, 0x05BF);
    tft.setTextColor(TFT_CYAN, 0x032E);
    tft.drawCentreString(currentMode == MODE_BOIDS ? "VIEW: TESSERACT" : "VIEW: BOIDS", 260, 4, 2);

    // Bottom action bar
    tft.fillRect(0, 216, 320, 24, 0x18C3);
    tft.drawFastHLine(0, 215, 320, 0x4208);

    if (currentMode == MODE_BOIDS) {
        // Attract / Repel Toggle button
        uint16_t btnColor = attractMode ? 0x0400 : 0x8000;
        tft.fillRect(6, 218, 90, 20, btnColor);
        tft.drawRect(6, 218, 90, 20, TFT_WHITE);
        tft.setTextColor(TFT_WHITE, btnColor);
        tft.drawCentreString(attractMode ? "ATTRACTOR" : "PREDATOR", 51, 221, 2);

        // Reset Boids button
        tft.fillRect(104, 218, 70, 20, 0x2124);
        tft.drawRect(104, 218, 70, 20, 0x52AA);
        tft.setTextColor(TFT_LIGHTGREY, 0x2124);
        tft.drawCentreString("SCATTER", 139, 221, 2);

        // Status text
        tft.setTextColor(0x9E7F, 0x18C3);
        tft.drawString("Touch screen to place beacon", 182, 221, 1);
    } else {
        // Tesseract Speed buttons
        tft.fillRect(6, 218, 70, 20, 0x2124);
        tft.drawRect(6, 218, 70, 20, 0x52AA);
        tft.setTextColor(TFT_WHITE, 0x2124);
        tft.drawCentreString("SPEED -", 41, 221, 2);

        tft.fillRect(82, 218, 70, 20, 0x2124);
        tft.drawRect(82, 218, 70, 20, 0x52AA);
        tft.setTextColor(TFT_WHITE, 0x2124);
        tft.drawCentreString("SPEED +", 117, 221, 2);

        tft.fillRect(160, 218, 74, 20, 0x028A);
        tft.drawRect(160, 218, 74, 20, 0x0493);
        tft.setTextColor(TFT_CYAN, 0x028A);
        tft.drawCentreString("ROT AXIS", 197, 221, 2);

        // Real-time FPS string
        tft.setTextColor(0x07E0, 0x18C3);
        char fBuf[16];
        snprintf(fBuf, sizeof(fBuf), "FPS: %d", currentFPS);
        tft.drawRightString(fBuf, 314, 221, 2);
    }
}

void updateBoids() {
    const float visualRange = 36.0f;
    const float minDistance = 12.0f;
    const float maxSpeed = 3.8f;
    const float minSpeed = 1.4f;

    for (int i = 0; i < NUM_BOIDS; i++) {
        float closeDx = 0, closeDy = 0;
        float xVelAvg = 0, yVelAvg = 0;
        float xPosAvg = 0, yPosAvg = 0;
        int neighboringBoids = 0;

        for (int j = 0; j < NUM_BOIDS; j++) {
            if (i == j) continue;
            float dx = boids[i].x - boids[j].x;
            float dy = boids[i].y - boids[j].y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist < minDistance && dist > 0.001f) {
                closeDx += dx / dist;
                closeDy += dy / dist;
            } else if (dist < visualRange) {
                xVelAvg += boids[j].vx;
                yVelAvg += boids[j].vy;
                xPosAvg += boids[j].x;
                yPosAvg += boids[j].y;
                neighboringBoids++;
            }
        }

        // Rule 1: Separation
        boids[i].vx += closeDx * 0.16f;
        boids[i].vy += closeDy * 0.16f;

        // Rule 2 & 3: Alignment & Cohesion
        if (neighboringBoids > 0) {
            xVelAvg /= neighboringBoids;
            yVelAvg /= neighboringBoids;
            boids[i].vx += (xVelAvg - boids[i].vx) * 0.05f;
            boids[i].vy += (yVelAvg - boids[i].vy) * 0.05f;

            xPosAvg /= neighboringBoids;
            yPosAvg /= neighboringBoids;
            boids[i].vx += (xPosAvg - boids[i].x) * 0.01f;
            boids[i].vy += (yPosAvg - boids[i].y) * 0.01f;
        }

        // Touch Interaction: Attractor vs Repulsor (Predator)
        if (hasTouchTarget && millis() < targetExpiryTime) {
            float tdx = touchTargetX - boids[i].x;
            float tdy = touchTargetY - boids[i].y;
            float tdist = sqrtf(tdx * tdx + tdy * tdy);
            if (tdist > 1.0f && tdist < 120.0f) {
                if (attractMode) {
                    // Attract towards food
                    boids[i].vx += (tdx / tdist) * 0.35f;
                    boids[i].vy += (tdy / tdist) * 0.35f;
                } else {
                    // Flee in terror from predator hawk
                    boids[i].vx -= (tdx / tdist) * 0.75f;
                    boids[i].vy -= (tdy / tdist) * 0.75f;
                }
            }
        }

        // Bounds avoidance (Soft bounce margin)
        const float marginX = 14.0f;
        const float marginYTop = 32.0f;
        const float marginYBottom = 208.0f;
        const float turnFactor = 0.5f;

        if (boids[i].x < marginX) boids[i].vx += turnFactor;
        if (boids[i].x > (320 - marginX)) boids[i].vx -= turnFactor;
        if (boids[i].y < marginYTop) boids[i].vy += turnFactor;
        if (boids[i].y > marginYBottom) boids[i].vy -= turnFactor;

        // Speed clamping
        float speed = sqrtf(boids[i].vx * boids[i].vx + boids[i].vy * boids[i].vy);
        if (speed > maxSpeed) {
            boids[i].vx = (boids[i].vx / speed) * maxSpeed;
            boids[i].vy = (boids[i].vy / speed) * maxSpeed;
        } else if (speed < minSpeed) {
            boids[i].vx = (boids[i].vx / speed) * minSpeed;
            boids[i].vy = (boids[i].vy / speed) * minSpeed;
        }

        boids[i].prev_x = boids[i].x;
        boids[i].prev_y = boids[i].y;
        boids[i].x += boids[i].vx;
        boids[i].y += boids[i].vy;
    }
}

void renderBoids() {
    // Erase old boids
    for (int i = 0; i < NUM_BOIDS; i++) {
        int px = (int)boids[i].prev_x;
        int py = (int)boids[i].prev_y;
        tft.fillCircle(px, py, 2, TFT_BLACK);
    }

    // Erase old target beacon if expired
    static int prevTargetX = -1, prevTargetY = -1;
    if (prevTargetX >= 0) {
        tft.drawCircle(prevTargetX, prevTargetY, 8, TFT_BLACK);
        tft.drawCircle(prevTargetX, prevTargetY, 14, TFT_BLACK);
        prevTargetX = -1;
        prevTargetY = -1;
    }

    // Draw active beacon
    if (hasTouchTarget && millis() < targetExpiryTime) {
        uint16_t beaconColor = attractMode ? 0x07E0 : 0xF800;
        int tx = (int)touchTargetX;
        int ty = (int)touchTargetY;
        tft.drawCircle(tx, ty, 8, beaconColor);
        tft.drawCircle(tx, ty, 14, beaconColor);
        prevTargetX = tx;
        prevTargetY = ty;
    }

    // Draw new boids with direction vectors
    for (int i = 0; i < NUM_BOIDS; i++) {
        int nx = (int)boids[i].x;
        int ny = (int)boids[i].y;
        
        // Color based on velocity angle
        uint16_t bColor = (i == 0) ? TFT_YELLOW : (i % 3 == 0 ? TFT_CYAN : 0x7E0);
        tft.fillCircle(nx, ny, 2, bColor);
        
        // Forward nose pointer
        int noseX = nx + (int)(boids[i].vx * 2.2f);
        int noseY = ny + (int)(boids[i].vy * 2.2f);
        tft.drawLine(nx, ny, noseX, noseY, bColor);
    }
}

void updateTesseract() {
    rotAngleXW += rotSpeed;
    rotAngleYZ += rotSpeed * 0.72f;
    rotAngleXY += rotSpeed * 0.45f;

    float cosXW = cosf(rotAngleXW), sinXW = sinf(rotAngleXW);
    float cosYZ = cosf(rotAngleYZ), sinYZ = sinf(rotAngleYZ);
    float cosXY = cosf(rotAngleXY), sinXY = sinf(rotAngleXY);

    const float dist4D = 2.4f;
    const float dist3D = 3.2f;
    const float scale2D = 175.0f;

    for (int i = 0; i < 16; i++) {
        prevProjectedVertices[i] = projectedVertices[i];

        Point4D p = tesseractVertices[i];

        // 4D Rotation in XW plane
        float x1 = p.x * cosXW - p.w * sinXW;
        float w1 = p.x * sinXW + p.w * cosXW;
        float y1 = p.y;
        float z1 = p.z;

        // 4D Rotation in YZ plane
        float y2 = y1 * cosYZ - z1 * sinYZ;
        float z2 = y1 * sinYZ + z1 * cosYZ;
        float x2 = x1;
        float w2 = w1;

        // 4D Rotation in XY plane
        float x3 = x2 * cosXY - y2 * sinXY;
        float y3 = x2 * sinXY + y2 * cosXY;
        float z3 = z2;
        float w3 = w2;

        // Perspective 4D -> 3D
        float factor4D = 1.0f / (dist4D - w3);
        Point3D p3 = { x3 * factor4D, y3 * factor4D, z3 * factor4D };

        // Perspective 3D -> 2D
        float factor3D = 1.0f / (dist3D - p3.z);
        projectedVertices[i].x = 160 + (int16_t)(p3.x * factor3D * scale2D);
        projectedVertices[i].y = 120 + (int16_t)(p3.y * factor3D * scale2D);
    }
}

void renderTesseract() {
    // Erase old 32 edges
    for (int i = 0; i < 32; i++) {
        uint8_t u = tesseractEdges[i][0];
        uint8_t v = tesseractEdges[i][1];
        tft.drawLine(prevProjectedVertices[u].x, prevProjectedVertices[u].y,
                     prevProjectedVertices[v].x, prevProjectedVertices[v].y, TFT_BLACK);
    }

    // Erase old vertices
    for (int i = 0; i < 16; i++) {
        tft.fillCircle(prevProjectedVertices[i].x, prevProjectedVertices[i].y, 3, TFT_BLACK);
    }

    // Draw new 32 edges with depth shading
    for (int i = 0; i < 32; i++) {
        uint8_t u = tesseractEdges[i][0];
        uint8_t v = tesseractEdges[i][1];
        
        // Depth-based color tint
        uint16_t edgeColor = (i < 12) ? 0x05BF : (i < 24 ? 0xFDE0 : 0xFA20);
        tft.drawLine(projectedVertices[u].x, projectedVertices[u].y,
                     projectedVertices[v].x, projectedVertices[v].y, edgeColor);
    }

    // Draw new 16 vertices
    for (int i = 0; i < 16; i++) {
        uint16_t vColor = (i < 8) ? TFT_CYAN : TFT_MAGENTA;
        tft.fillCircle(projectedVertices[i].x, projectedVertices[i].y, 3, vColor);
    }
}

void handleTouch() {
    if (!ts.touched()) return;

    TS_Point p = ts.getPoint();
    int touchX = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int touchY = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    touchX = constrain(touchX, 0, 319);
    touchY = constrain(touchY, 0, 239);

    // Top Bar Mode Switch (205, 2 to 315, 22)
    if (touchY < 26 && touchX >= 200) {
        currentMode = (currentMode == MODE_BOIDS) ? MODE_TESSERACT : MODE_BOIDS;
        playBeep(2400, 40);
        tft.fillScreen(TFT_BLACK);
        drawUIStatic();
        delay(250);
        return;
    }

    // Bottom Bar Buttons
    if (touchY >= 214) {
        if (currentMode == MODE_BOIDS) {
            // Attractor/Predator Toggle (6, 218 to 96, 238)
            if (touchX >= 6 && touchX <= 96) {
                attractMode = !attractMode;
                playBeep(attractMode ? 1800 : 800, 35);
                drawUIStatic();
                delay(200);
                return;
            }
            // Scatter Button (104, 218 to 174, 238)
            if (touchX >= 104 && touchX <= 174) {
                initBoids();
                playBeep(1200, 50);
                delay(200);
                return;
            }
        } else {
            // Speed - (6, 218 to 76, 238)
            if (touchX >= 6 && touchX <= 76) {
                rotSpeed = max(0.01f, rotSpeed - 0.01f);
                playBeep(1000, 30);
                delay(150);
                return;
            }
            // Speed + (82, 218 to 152, 238)
            if (touchX >= 82 && touchX <= 152) {
                rotSpeed = min(0.12f, rotSpeed + 0.01f);
                playBeep(2000, 30);
                delay(150);
                return;
            }
            // Rot Axis Toggle (160, 218 to 234, 238)
            if (touchX >= 160 && touchX <= 234) {
                rotAngleXW += 1.57079f; // 90 deg jump in 4D space
                playBeep(1500, 30);
                delay(200);
                return;
            }
        }
    }

    // Middle Play Area Touch
    if (touchY > 26 && touchY < 214) {
        if (currentMode == MODE_BOIDS) {
            hasTouchTarget = true;
            touchTargetX = touchX;
            touchTargetY = touchY;
            targetExpiryTime = millis() + 4500; // 4.5 seconds beacon lifetime
            playBeep(attractMode ? 1400 : 600, 20);
        } else {
            // Dragging adjusts rotation in 4D
            rotAngleXW += (touchX - 160) * 0.0008f;
            rotAngleYZ += (touchY - 120) * 0.0008f;
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

    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(touchSPI);
    ts.setRotation(1);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    initBoids();
    initTesseract();

    drawUIStatic();
    playBeep(1600, 80);
    fpsTimer = millis();
}

void loop() {
    handleTouch();

    if (currentMode == MODE_BOIDS) {
        updateBoids();
        renderBoids();
        setRGB(false, attractMode, !attractMode);
    } else {
        updateTesseract();
        renderTesseract();
        bool ledPulse = ((int)(rotAngleXW * 4.0f) % 2) == 0;
        setRGB(ledPulse, false, !ledPulse);
    }

    // FPS Meter update
    frameCount++;
    if (millis() - fpsTimer >= 1000) {
        currentFPS = frameCount;
        frameCount = 0;
        fpsTimer = millis();

        if (currentMode == MODE_TESSERACT) {
            tft.setTextColor(0x07E0, 0x18C3);
            char fBuf[16];
            snprintf(fBuf, sizeof(fBuf), "FPS: %d ", currentFPS);
            tft.drawRightString(fBuf, 314, 221, 2);
        }
    }

    delay(12); // ~60-70 Hz target frame pacing
}

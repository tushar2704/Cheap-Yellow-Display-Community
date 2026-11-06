/**
 * @file main.cpp
 * @brief CYD Ghost Radar & EMF Spirit Tracker - Paranormal Hunter Deck
 * @author Tushar Aggarwal (@tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 *
 * Hardware:
 * - Cheap Yellow Display (ESP32-2432S028R) 2.8" Resistive Touch
 * - Display: ILI9341 320x240 Landscape
 * - Touch: XPT2046 SPI (CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36)
 * - Audio: GPIO 26 Speaker / Piezo buzzer (Sonar pings, EVP static, EMF siren)
 * - RGB LED: GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) [Active LOW]
 * - Optical Sensor: GPIO 34 LDR (Shadow / Cold spot detection)
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// ---------------- Pinout ----------------
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
#define XPT2046_IRQ 36

#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define SPEAKER_PIN 26
#define LDR_PIN 34

// ---------------- Display & Touch ----------------
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// ---------------- Paranormal Color Palette ----------------
#define COL_BG 0x0000       // Void Black
#define COL_PANEL 0x0841    // Deep Sonar Green
#define COL_GRID 0x0280     // Radar Grid
#define COL_PHOSPHOR 0x07E0 // Radar Phosphor Green
#define COL_ANOMALY 0xF800  // Entity Crimson Red
#define COL_COLD_ICE 0x05BF // Freezing Cyan
#define COL_AMBER 0xFDA0    // EMF Warning
#define COL_WHITE 0xFFFF    // Clean White
#define COL_GRAY 0x4208     // Inactive

// ---------------- Radar Geometry ----------------
#define RADAR_CX 110
#define RADAR_CY 128
#define RADAR_R 85

float sweepAngle = 0.0f;
float prevSweepAngle = 0.0f;

// ---------------- Anomaly / Ghost Entities ----------------
struct Entity {
  bool active;
  float angle;    // Radians
  float distance; // Pixels from center (10..RADAR_R)
  int emfLevel;   // 1 to 5
  uint8_t alpha;  // Persistence 0..255
};

#define MAX_ENTITIES 4
Entity entities[MAX_ENTITIES];

// ---------------- Spirit Box EVP Word Database ----------------
const char *EVP_WORDS[] = {"BEHIND YOU",    "COLD...",   "RUN!",
                           "FOUND",         "LEAVE NOW", "SHADOW",
                           "WATCHING",      "DARKNESS",  "TRAPPED",
                           "HERE WITH YOU", "HELP ME",   "CLOSE..."};
#define TOTAL_EVP_WORDS 12

const char *currentEvpWord = "SCANNING...";
unsigned long lastEvpTrigger = 0;
unsigned long lastAnomalySpawn = 0;
unsigned long lastTempUpdate = 0;
int currentEmfLevel = 1;
float ambientTemp = 21.4f;
bool soundEnabled = true;

// ---------------- RGB Control ----------------
void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

// ---------------- Audio Synthesizer ----------------
void playSonarPing() {
  if (!soundEnabled)
    return;
  tone(SPEAKER_PIN, 1850, 18);
}

void playEVPStatic() {
  if (!soundEnabled)
    return;
  for (int i = 0; i < 8; i++) {
    tone(SPEAKER_PIN, random(250, 2200), 8);
    delay(6);
  }
  noTone(SPEAKER_PIN);
}

void playEMFAlarm() {
  if (!soundEnabled)
    return;
  tone(SPEAKER_PIN, 2600, 60);
}

// ---------------- Draw Radar Frames & Grid ----------------
void drawRadarBase() {
  tft.fillScreen(COL_BG);

  // Header Banner
  tft.fillRect(0, 0, 320, 30, 0x00A0);
  tft.drawFastHLine(0, 30, 320, COL_PHOSPHOR);

  tft.setTextFont(2);
  tft.setTextColor(COL_PHOSPHOR, 0x00A0);
  tft.setCursor(8, 6);
  tft.print("P.K.E. PARANORMAL TRACKER // EMF DECK");

  // Circular Radar Scope Rings
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R, COL_PHOSPHOR);
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 0.75, COL_GRID);
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 0.50, COL_GRID);
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 0.25, COL_GRID);
  tft.drawCircle(RADAR_CX, RADAR_CY, 3, COL_PHOSPHOR);

  // Crosshairs
  tft.drawFastHLine(RADAR_CX - RADAR_R, RADAR_CY, RADAR_R * 2, COL_GRID);
  tft.drawFastVLine(RADAR_CX, RADAR_CY - RADAR_R, RADAR_R * 2, COL_GRID);

  // Right Side Telemetry HUD (X: 206..314)
  tft.drawRoundRect(204, 36, 110, 196, 6, COL_PHOSPHOR);
  tft.fillRect(206, 38, 106, 192, 0x0041);

  // EMF Meter Header
  tft.setTextFont(2);
  tft.setTextColor(COL_PHOSPHOR, 0x0041);
  tft.drawCentreString("EMF LEVEL", 259, 44, 2);

  // Draw 5 EMF LED Boxes
  for (int e = 0; e < 5; e++) {
    int ey = 66 + (e * 14);
    tft.drawRect(216, ey, 86, 11, COL_GRID);
  }

  // Cold Spot / Temp Header
  tft.setTextFont(1);
  tft.setTextColor(COL_WHITE, 0x0041);
  tft.setCursor(212, 142);
  tft.print("TEMP SENSOR:");

  // Spirit Box EVP Header
  tft.setCursor(212, 178);
  tft.print("EVP DECODER:");
}

// ---------------- Update EMF 5-LED Gauge ----------------
void renderEMFMeter(int level) {
  uint16_t colors[] = {COL_PHOSPHOR, COL_PHOSPHOR, COL_AMBER, 0xFD20,
                       COL_ANOMALY};
  for (int e = 0; e < 5; e++) {
    int ey = 66 + (e * 14);
    if (e < level) {
      tft.fillRect(218, ey + 2, 82, 7, colors[e]);
    } else {
      tft.fillRect(218, ey + 2, 82, 7, 0x0000);
    }
  }

  // Level number text
  tft.setTextFont(2);
  tft.setTextColor((level == 5) ? COL_ANOMALY : COL_WHITE, 0x0041);
  char buf[16];
  snprintf(buf, sizeof(buf), "LEVEL %d", level);
  tft.drawCentreString(buf, 259, 118, 2);
}

// ---------------- Update Temp & EVP HUD ----------------
void renderTelemetry(float temp, const char *evp) {
  // Temp Readout
  tft.fillRect(212, 154, 94, 20, 0x0041);
  tft.setTextFont(2);
  tft.setTextColor((temp < 10.0f) ? COL_COLD_ICE : COL_WHITE, 0x0041);
  tft.setCursor(212, 154);
  tft.printf("%4.1f C %s", temp, (temp < 10.0f) ? "COLD" : "");

  // EVP Word Readout
  tft.fillRect(210, 192, 98, 34, 0x0041);
  tft.setTextFont(2);
  tft.setTextColor(COL_ANOMALY, 0x0041);
  tft.drawCentreString(evp, 259, 198, 2);
}

// ---------------- Spawn Random Entity Blip ----------------
void spawnEntity() {
  for (int i = 0; i < MAX_ENTITIES; i++) {
    if (!entities[i].active) {
      entities[i].active = true;
      entities[i].angle = random(0, 628) / 100.0f;
      entities[i].distance = random(20, RADAR_R - 5);
      entities[i].emfLevel = random(2, 6);
      entities[i].alpha = 255;
      break;
    }
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(F("[GHOST RADAR] Initializing Paranormal Sensors..."));

  // Pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  setRGB(false, false, false);
  noTone(SPEAKER_PIN);

  // Initialize Display
  tft.init();
  tft.setRotation(1);

  // Initialize Touch
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Clear Entities
  for (int i = 0; i < MAX_ENTITIES; i++) {
    entities[i].active = false;
  }

  drawRadarBase();
  renderEMFMeter(currentEmfLevel);
  renderTelemetry(ambientTemp, currentEvpWord);

  // Startup Sonar Sweep sound
  playSonarPing();
}

// ---------------- Main Loop ----------------
void loop() {
  unsigned long now = millis();

  // 1. Read Ambient Optical Sensor (Shadow / Cold Spot detector)
  int ldrVal = analogRead(LDR_PIN);

  // If ambient light drops suddenly (casting shadow over CYD), simulate a
  // chilling cold spot!
  if (ldrVal < 800) {
    ambientTemp = 4.2f + (ldrVal * 0.005f); // Freezing temps
  } else {
    ambientTemp = 20.8f + (ldrVal * 0.001f);
  }

  // 2. Erase previous radar sweep line
  int prevX = RADAR_CX + cos(prevSweepAngle) * (RADAR_R - 2);
  int prevY = RADAR_CY + sin(prevSweepAngle) * (RADAR_R - 2);
  tft.drawLine(RADAR_CX, RADAR_CY, prevX, prevY, COL_BG);

  // Re-draw inner scope rings that might have been erased
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 0.75, COL_GRID);
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 0.50, COL_GRID);
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 0.25, COL_GRID);
  tft.drawCircle(RADAR_CX, RADAR_CY, 3, COL_PHOSPHOR);

  // 3. Advance Radar Sweep Angle
  prevSweepAngle = sweepAngle;
  sweepAngle += 0.08f;
  if (sweepAngle > 6.28318f) {
    sweepAngle -= 6.28318f;
    playSonarPing(); // Ping on complete revolution
  }

  // 4. Draw Current Sweep Line
  int curX = RADAR_CX + cos(sweepAngle) * (RADAR_R - 2);
  int curY = RADAR_CY + sin(sweepAngle) * (RADAR_R - 2);
  tft.drawLine(RADAR_CX, RADAR_CY, curX, curY, COL_PHOSPHOR);

  // 5. Spawn Anomaly randomly or when shadow passes
  if (now - lastAnomalySpawn >= 5000) {
    lastAnomalySpawn = now;
    if (random(0, 10) < 6 || ldrVal < 800) {
      spawnEntity();
    }
  }

  // 6. Draw Entity Blips & Check Radar Sweep Intersection
  int maxDetectedEmf = 1;
  for (int i = 0; i < MAX_ENTITIES; i++) {
    if (entities[i].active) {
      int ex = RADAR_CX + cos(entities[i].angle) * entities[i].distance;
      int ey = RADAR_CY + sin(entities[i].angle) * entities[i].distance;

      // Draw blip
      tft.fillCircle(ex, ey, 4, COL_ANOMALY);
      tft.drawCircle(ex, ey, 7, COL_WHITE);

      // Check if sweep beam is currently hitting this entity
      float diff = fabs(sweepAngle - entities[i].angle);
      if (diff < 0.12f || diff > 6.16f) {
        maxDetectedEmf = max(maxDetectedEmf, entities[i].emfLevel);
        playEMFAlarm();
      }

      // Fade entity decay
      if (entities[i].alpha > 12) {
        entities[i].alpha -= 4;
      } else {
        // Clear blip & restore rings
        tft.fillCircle(ex, ey, 8, COL_BG);
        tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 0.75, COL_GRID);
        tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 0.50, COL_GRID);
        tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 0.25, COL_GRID);
        tft.drawCircle(RADAR_CX, RADAR_CY, 3, COL_PHOSPHOR);
        entities[i].active = false;
      }
    }
  }

  // 7. Update EMF Meter & RGB Alert
  if (maxDetectedEmf != currentEmfLevel) {
    currentEmfLevel = maxDetectedEmf;
    renderEMFMeter(currentEmfLevel);

    if (currentEmfLevel == 5) {
      setRGB(true, false, false); // Critical Red
    } else if (currentEmfLevel >= 3) {
      setRGB(true, true, false); // Amber
    } else {
      setRGB(false, true, false); // Calm Green
    }
  }

  // 8. Dynamic Temperature HUD update (every 400ms)
  if (now - lastTempUpdate >= 400) {
    lastTempUpdate = now;
    renderTelemetry(ambientTemp, currentEvpWord);
  }

  // 9. Spirit Box EVP Radio static & word trigger
  if (now - lastEvpTrigger >= 6000) {
    lastEvpTrigger = now;
    if (random(0, 10) < 5 || currentEmfLevel >= 4) {
      currentEvpWord = EVP_WORDS[random(0, TOTAL_EVP_WORDS)];
      playEVPStatic();
      renderTelemetry(ambientTemp, currentEvpWord);
    }
  }

  // 9. Touch Controls
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int tx = map(p.x, 240, 3800, 0, 320);
    int ty = map(p.y, 240, 3800, 0, 240);
    tx = constrain(tx, 0, 319);
    ty = constrain(ty, 0, 239);

    // Tap radar scope to manually place an entity (creator video trick!)
    if (tx < 195 && ty > 36) {
      float dx = tx - RADAR_CX;
      float dy = ty - RADAR_CY;
      float dist = sqrt(dx * dx + dy * dy);
      if (dist < RADAR_R) {
        spawnEntity();
        playEMFAlarm();
        delay(200);
      }
    }
    // Tap Right HUD to trigger EVP spirit box burst
    else if (tx >= 204 && ty >= 170) {
      currentEvpWord = EVP_WORDS[random(0, TOTAL_EVP_WORDS)];
      playEVPStatic();
      renderTelemetry(ambientTemp, currentEvpWord);
      delay(250);
    }
  }

  delay(20);
}

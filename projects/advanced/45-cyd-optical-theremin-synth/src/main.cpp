/**
 * @file main.cpp
 * @brief CYD Optical Theremin & Neon Synth Jam - Hands-Free Music Machine
 * @author Tushar Aggarwal (@tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 *
 * Hardware:
 * - Cheap Yellow Display (ESP32-2432S028R) 2.8" Resistive Touch
 * - Display: ILI9341 320x240 Landscape
 * - Touch: XPT2046 SPI (CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36)
 * - Audio: GPIO 26 Speaker / Piezo buzzer (Continuous Theremin & Synth tones)
 * - RGB LED: GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) [Active LOW]
 * - Optical Sensor: GPIO 34 LDR (Light Dependent Resistor)
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

// ---------------- Neon Cyberpunk Palette ----------------
#define COL_BG 0x0821          // Deep Synth Void
#define COL_PANEL 0x10A4       // Surface Slate
#define COL_BORDER 0x2147      // Grid lines
#define COL_NEON_CYAN 0x07FF   // Lead Synth
#define COL_NEON_PINK 0xF81F   // Treble / Glow
#define COL_NEON_YELLOW 0xFFE0 // Arp Glow
#define COL_NEON_GREEN 0x07E0  // Bass
#define COL_WHITE 0xFFFF       // Clean White
#define COL_GRAY 0x5AEB        // Inactive

// ---------------- Musical Scales & Notes ----------------
enum SynthMode {
  MODE_THEREMIN,   // Continuous glissando frequency sweep
  MODE_PENTATONIC, // Snapped quantizer scale (never sounds off-key)
  MODE_ARPEGGIATOR // 8-Bit rapid chord chiptune rolls
};

SynthMode currentMode = MODE_THEREMIN;

// Pentatonic Scale (C Major Pentatonic across 2+ Octaves)
const int PENTATONIC_FREQS[] = {
    262,  294,  330, 392, 440, // C4, D4, E4, G4, A4
    523,  587,  659, 784, 880, // C5, D5, E5, G5, A5
    1046, 1175, 1318           // C6, D6, E6
};
const char *PENTATONIC_NAMES[] = {"C4", "D4", "E4", "G4", "A4", "C5", "D5",
                                  "E5", "G5", "A5", "C6", "D6", "E6"};
#define SCALE_STEPS 13

// Arpeggio chords (Chiptune chords)
const int ARP_CHORDS[3][4] = {
    {262, 330, 392, 523}, // C Major
    {349, 440, 523, 698}, // F Major
    {392, 494, 587, 784}  // G Major
};

// ---------------- State Variables ----------------
int ldrMin = 4095;
int ldrMax = 0;
int currentFreq = 440;
int targetFreq = 440;
int prevAudioFreq = -1;
int lastRenderedFreq = -1;
const char *lastRenderedNote = "";
bool lastRenderedMuted = false;
const char *currentNoteName = "A4";
bool isMuted = false;
float oscPhase = 0.0f;
int arpIndex = 0;
unsigned long lastArpTick = 0;
unsigned long lastUiRefresh = 0;

// Oscilloscope Geometry
#define OSC_X 10
#define OSC_Y 48
#define OSC_W 300
#define OSC_H 76

// ---------------- RGB Control ----------------
void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playTone(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
  delay(durationMs);
  noTone(SPEAKER_PIN);
}

// ---------------- Drum Sound Synthesizer ----------------
void playDrumKick() {
  for (int f = 160; f > 40; f -= 12) {
    tone(SPEAKER_PIN, f, 8);
    delay(5);
  }
}

void playDrumSnare() {
  for (int i = 0; i < 14; i++) {
    tone(SPEAKER_PIN, random(800, 2600), 6);
    delay(5);
  }
}

// ---------------- Draw Frame & HUD ----------------
void drawSynthHUD() {
  tft.fillScreen(COL_BG);

  // Top Neon Banner
  tft.fillRect(0, 0, 320, 36, COL_PANEL);
  tft.drawFastHLine(0, 36, 320, COL_NEON_PINK);

  tft.setTextFont(2);
  tft.setTextColor(COL_NEON_CYAN, COL_PANEL);
  tft.setCursor(8, 4);
  tft.print("CYD OPTICAL THEREMIN // SYNTH LAB");

  tft.setTextFont(1);
  tft.setTextColor(COL_NEON_PINK, COL_PANEL);
  tft.setCursor(8, 22);
  tft.print("HOVER HAND OVER LDR SENSOR TO MODULATE PITCH");

  // Oscilloscope Container
  tft.drawRect(OSC_X - 1, OSC_Y - 1, OSC_W + 2, OSC_H + 2, COL_BORDER);
  tft.fillRect(OSC_X, OSC_Y, OSC_W, OSC_H, 0x0000);

  // Center Zero Line
  tft.drawFastHLine(OSC_X, OSC_Y + (OSC_H / 2), OSC_W, 0x0185);

  // Middle Telemetry Strip (Y: 130..165)
  tft.drawRoundRect(8, 128, 304, 38, 6, COL_PANEL);
  tft.fillRect(10, 130, 300, 34, 0x0041);

  // Mode Selection Tabs (Y: 172..202)
  const char *modeNames[] = {"THEREMIN", "PENTATONIC", "ARPEGGIATOR"};
  for (int m = 0; m < 3; m++) {
    int mx = 8 + (m * 102);
    uint16_t mCol = (m == (int)currentMode) ? COL_NEON_CYAN : COL_PANEL;
    tft.fillRoundRect(mx, 172, 98, 30, 4, mCol);
    tft.drawRoundRect(mx, 172, 98, 30, 4, COL_BORDER);
    tft.setTextFont(2);
    tft.setTextColor((m == (int)currentMode) ? 0x0000 : COL_WHITE, mCol);
    tft.drawCentreString(modeNames[m], mx + 49, 178, 2);
  }

  // Bottom Touch Beat Pads (Y: 206..236)
  // [KICK] [SNARE] [MUTE/PLAY]
  tft.fillRoundRect(8, 206, 96, 30, 4, COL_NEON_PINK);
  tft.setTextFont(2);
  tft.setTextColor(COL_WHITE, COL_NEON_PINK);
  tft.drawCentreString("TAP KICK", 56, 212, 2);

  tft.fillRoundRect(112, 206, 96, 30, 4, COL_NEON_YELLOW);
  tft.setTextColor(0x0000, COL_NEON_YELLOW);
  tft.drawCentreString("TAP SNARE", 160, 212, 2);

  tft.fillRoundRect(216, 206, 96, 30, 4, isMuted ? COL_GRAY : COL_NEON_GREEN);
  tft.setTextColor(COL_BG, isMuted ? COL_GRAY : COL_NEON_GREEN);
  tft.drawCentreString(isMuted ? "MUTED" : "LIVE", 264, 212, 2);
}

// ---------------- Update Oscilloscope Visualizer ----------------
void renderOscilloscope(int freq) {
  // Clear oscilloscope canvas
  tft.fillRect(OSC_X, OSC_Y, OSC_W, OSC_H, 0x0000);
  tft.drawFastHLine(OSC_X, OSC_Y + (OSC_H / 2), OSC_W, 0x0185);

  uint16_t waveColor =
      (currentMode == MODE_THEREMIN)
          ? COL_NEON_CYAN
          : ((currentMode == MODE_PENTATONIC) ? COL_NEON_GREEN
                                              : COL_NEON_YELLOW);

  float cyclesAcross = map(constrain(freq, 100, 2000), 100, 2000, 2, 14);
  float step = (cyclesAcross * 6.28318f) / OSC_W;

  oscPhase += 0.25f;
  if (oscPhase > 6.28318f)
    oscPhase -= 6.28318f;

  int centerY = OSC_Y + (OSC_H / 2);
  int amplitude = isMuted ? 0 : 28;

  int prevY = centerY + sin(oscPhase) * amplitude;
  for (int x = 1; x < OSC_W; x++) {
    int curY = centerY + sin(oscPhase + (x * step)) * amplitude;
    tft.drawLine(OSC_X + x - 1, prevY, OSC_X + x, curY, waveColor);
    prevY = curY;
  }
}

// ---------------- Update Frequency Telemetry ----------------
void renderTelemetry(int freq, const char *note) {
  if (freq == lastRenderedFreq && strcmp(note, lastRenderedNote) == 0 &&
      isMuted == lastRenderedMuted) {
    return; // Skip redundant draw
  }
  lastRenderedFreq = freq;
  lastRenderedNote = note;
  lastRenderedMuted = isMuted;

  tft.fillRect(14, 132, 292, 30, 0x0041);

  tft.setTextFont(4);
  tft.setTextColor(COL_WHITE, 0x0041);
  tft.setCursor(20, 134);
  tft.printf("%4d Hz", freq);

  tft.setTextColor(COL_NEON_PINK, 0x0041);
  tft.setCursor(150, 134);
  tft.printf("NOTE: %s", note);

  tft.setTextFont(2);
  tft.setTextColor(COL_NEON_YELLOW, 0x0041);
  tft.setCursor(252, 138);
  tft.print(isMuted ? "[OFF]" : "[ON]");
}

void updateAudioTone() {
  if (isMuted) {
    if (prevAudioFreq != 0) {
      prevAudioFreq = 0;
      noTone(SPEAKER_PIN);
    }
  } else {
    if (abs(currentFreq - prevAudioFreq) >= 3) {
      prevAudioFreq = currentFreq;
      tone(SPEAKER_PIN, currentFreq);
    }
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(F("[OPTICAL THEREMIN] Initializing Synth Core..."));

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

  // Initial Calibration of LDR
  int sample = analogRead(LDR_PIN);
  ldrMin = max(100, sample - 400);
  ldrMax = min(4000, sample + 400);

  drawSynthHUD();
  renderTelemetry(currentFreq, currentNoteName);

  // Startup Synth Arpeggio
  playTone(262, 50);
  playTone(330, 50);
  playTone(392, 50);
  playTone(523, 90);
}

// ---------------- Main Loop ----------------
void loop() {
  unsigned long now = millis();

  // 1. Read Optical Light Sensor (LDR)
  int rawLdr = analogRead(LDR_PIN);

  // Dynamic Auto-Calibration tracking
  if (rawLdr < ldrMin)
    ldrMin = rawLdr;
  if (rawLdr > ldrMax)
    ldrMax = rawLdr;
  if (ldrMax <= ldrMin)
    ldrMax = ldrMin + 1;

  // 2. Compute Pitch from Light Shadow
  // Invert mapping: Hand closer -> casts shadow (lower light) -> pitch bend!
  int lightNormalized = map(rawLdr, ldrMin, ldrMax, 0, 1000);
  lightNormalized = constrain(lightNormalized, 0, 1000);

  if (currentMode == MODE_THEREMIN) {
    // Glissando Continuous Theremin (180 Hz to 1800 Hz)
    targetFreq = map(1000 - lightNormalized, 0, 1000, 180, 1800);
    currentFreq = (currentFreq * 0.7f) + (targetFreq * 0.3f);
    currentNoteName = "~GLISS";
    updateAudioTone();
  } else if (currentMode == MODE_PENTATONIC) {
    // Quantized Pentatonic Scale
    int step = map(1000 - lightNormalized, 0, 1000, 0, SCALE_STEPS - 1);
    step = constrain(step, 0, SCALE_STEPS - 1);
    currentFreq = PENTATONIC_FREQS[step];
    currentNoteName = PENTATONIC_NAMES[step];
    updateAudioTone();
  } else if (currentMode == MODE_ARPEGGIATOR) {
    // 8-Bit Chiptune Chord Rolling
    int chordIdx = map(1000 - lightNormalized, 0, 1000, 0, 2);
    chordIdx = constrain(chordIdx, 0, 2);

    if (now - lastArpTick >= 90) {
      lastArpTick = now;
      arpIndex = (arpIndex + 1) % 4;
      currentFreq = ARP_CHORDS[chordIdx][arpIndex];
      currentNoteName =
          (chordIdx == 0) ? "C-MAJ" : ((chordIdx == 1) ? "F-MAJ" : "G-MAJ");
      updateAudioTone();
    }
  }

  // 3. LED Color reacts to Pitch
  if (!isMuted) {
    if (currentFreq < 350)
      setRGB(false, false, true); // Deep Blue Bass
    else if (currentFreq < 650)
      setRGB(false, true, true); // Cyan Mid
    else if (currentFreq < 1000)
      setRGB(false, true, false); // Green High
    else
      setRGB(true, false, true); // Magenta Treble
  } else {
    setRGB(false, false, false);
    updateAudioTone();
  }

  // 4. Touch Controls
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int tx = map(p.x, 240, 3800, 0, 320);
    int ty = map(p.y, 240, 3800, 0, 240);
    tx = constrain(tx, 0, 319);
    ty = constrain(ty, 0, 239);

    // Mode Selection Tap (Y: 172..202)
    if (ty >= 170 && ty <= 204) {
      SynthMode newMode = currentMode;
      if (tx < 106)
        newMode = MODE_THEREMIN;
      else if (tx < 210)
        newMode = MODE_PENTATONIC;
      else
        newMode = MODE_ARPEGGIATOR;

      if (newMode != currentMode) {
        currentMode = newMode;
        drawSynthHUD();
        delay(150);
      }
    }
    // Bottom Drum & Mute Pads (Y: 206..238)
    else if (ty >= 206 && ty <= 238) {
      // Tap Kick
      if (tx >= 8 && tx <= 104) {
        playDrumKick();
      }
      // Tap Snare
      else if (tx >= 112 && tx <= 208) {
        playDrumSnare();
      }
      // Mute / Live Toggle
      else if (tx >= 216 && tx <= 312) {
        isMuted = !isMuted;
        if (isMuted)
          noTone(SPEAKER_PIN);
        drawSynthHUD();
        delay(200);
      }
    }
  }

  // 5. Visualizer Refresh
  if (now - lastUiRefresh >= 50) {
    lastUiRefresh = now;
    renderOscilloscope(currentFreq);
    renderTelemetry(currentFreq, currentNoteName);
  }

  delay(10);
}

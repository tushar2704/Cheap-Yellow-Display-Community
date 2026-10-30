/**
 * @file 05-local-ai-desk-terminal.ino
 * @brief Local-First Private AI Desk Terminal (Ollama Client & Streaming Engine)
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 (HSPI: MOSI=13, MISO=12, CLK=14, CS=15)
 * - Touch: XPT2046 Resistive Touch (VSPI: MOSI=32, MISO=39, CLK=25, CS=33)
 * - Network: ESP32 Wi-Fi + HTTP Streaming Client
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Accurate CYD 2.8" Resistive Touch SPI Pinout
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33
#define XPT2046_IRQ   36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26

// Wi-Fi & Local Ollama Host Configuration
const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";
const char* ollamaHost = "http://192.168.1.50:11434/api/generate";

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS); // Polling mode avoids GPIO 36 interrupt lockup
TFT_eSPI tft = TFT_eSPI();

const char* models[] = {"llama3", "mistral", "phi3", "gemma2"};
#define MODEL_COUNT 4
int currentModelIdx = 0;

struct PromptCard {
  const char* title;
  const char* prompt;
  uint16_t color;
};

#define PROMPT_COUNT 4
PromptCard promptCards[PROMPT_COUNT] = {
  {"1. SUMMARIZE",    "Summarize this text in 3 concise bullet points:\n", 0x07E0}, // Green
  {"2. FIX CODE",     "Identify the bug in this code and write the fix:\n", 0x07FF}, // Cyan
  {"3. EXPLAIN REGEX", "Deconstruct and explain this regex pattern:\n",    0xFD20}, // Orange
  {"4. DRAFT REPLY",  "Draft a polite, concise professional email:\n",    0xF81F}  // Magenta
};

int cursorX = 12;
int cursorY = 82;
bool isGenerating = false;
unsigned long lastTouchTime = 0;
unsigned long footerClearTime = 0;

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

void updateFooter(const char* message, uint16_t fg = TFT_WHITE, uint16_t bg = 0x18C3) {
  tft.fillRect(0, 224, 320, 16, bg);
  tft.setTextColor(fg, bg);
  tft.setTextSize(1);
  tft.setCursor(8, 228);
  tft.print(message);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 6);
  tft.print("LOCAL AI TERMINAL  ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print("@IamNotIntoTech");

  // Model Selector Badge (top right)
  tft.fillRoundRect(214, 2, 102, 20, 3, 0x18E3);
  tft.drawRoundRect(214, 2, 102, 20, 3, TFT_CYAN);
  tft.setTextColor(TFT_WHITE, 0x18E3);
  tft.setCursor(220, 6);
  tft.printf("Model: %s", models[currentModelIdx]);
}

void drawCard(int i, bool pressed) {
  int startX = 6;
  int startY = 28;
  int cardW = 74;
  int cardH = 26;
  int gap = 5;

  int cx = startX + i * (cardW + gap);
  uint16_t bg = pressed ? promptCards[i].color : 0x0000;
  uint16_t fg = pressed ? TFT_BLACK : promptCards[i].color;
  uint16_t border = pressed ? TFT_WHITE : promptCards[i].color;

  tft.fillRoundRect(cx, startY, cardW, cardH, 3, bg);
  tft.drawRoundRect(cx, startY, cardW, cardH, 3, border);

  tft.setTextColor(fg, bg);
  tft.setTextSize(1);
  tft.setCursor(cx + 4, startY + 9);
  tft.print(promptCards[i].title);
}

void drawPromptCards() {
  for (int i = 0; i < PROMPT_COUNT; i++) {
    drawCard(i, false);
  }
}

void drawTerminalWindow() {
  // Clear Terminal Output Area
  tft.fillRoundRect(6, 58, 308, 162, 4, 0x0000);
  tft.drawRoundRect(6, 58, 308, 162, 4, 0x2965);

  tft.fillRect(7, 59, 306, 16, 0x10A2);
  tft.setTextColor(0x07FF, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(12, 63);
  tft.print("> Ollama Output Stream (/api/generate)");

  cursorX = 12;
  cursorY = 82;

  updateFooter("Tap prompt card to dispatch query | @IamNotIntoTech");
}

void printTerminalToken(const char* text, uint16_t color) {
  tft.setTextColor(color, 0x0000);
  tft.setTextSize(1);

  for (size_t i = 0; i < strlen(text); i++) {
    char c = text[i];
    if (c == '\n' || cursorX > 298) {
      cursorX = 12;
      cursorY += 12;
      if (cursorY > 206) {
        // Wrap and clear text window body
        cursorY = 82;
        tft.fillRect(7, 76, 306, 142, 0x0000);
      }
    }
    if (c != '\n') {
      tft.setCursor(cursorX, cursorY);
      tft.print(c);
      cursorX += 6;
    }
  }
}

void simulateAiResponse(int promptIdx) {
  const char* sampleReplies[4] = {
    "- Edge AI executes 100% locally on workstation hardware.\n- Zero cloud dependencies, zero data leakage, zero API bills.\n- Sub-second UI touch latency on 2.8\" QVGA display.\n",
    "Bug identified: Memory leak in packet deserialization loop.\nFix:\nDynamicJsonDocument doc(512);\n// Always ensure document is cleared\ndoc.clear();\n",
    "Regex: ^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\\.[a-zA-Z0-9-.]+$\nDeconstruction:\n- ^ / $ -> Anchor start and end of string\n- [a-zA-Z0-9_.+-]+ -> Username alphanumeric\n- @ -> Mandatory delimiter\n- Domain name and TLD extension check\n",
    "Hi Team,\n\nThanks for reaching out. The hardware firmware has been flashed, calibrated, and verified on the CYD 2.8\" unit. Ready for sign-off.\n\nBest regards,\nTushar\n"
  };

  const char* reply = sampleReplies[promptIdx];
  char token[2] = {0, 0};
  for (size_t i = 0; i < strlen(reply); i++) {
    token[0] = reply[i];
    printTerminalToken(token, 0x07E0);
    delay(18); // Natural LLM token streaming speed
  }
}

void streamOllamaQuery(int promptIdx) {
  isGenerating = true;

  // 1. Highlight card
  drawCard(promptIdx, true);
  playBeep(2200, 50);
  setRgbLed(false, false, true); // Blue = Processing

  // 2. Clear terminal window and show query banner
  drawTerminalWindow();

  char banner[64];
  snprintf(banner, sizeof(banner), "Prompt: %s", promptCards[promptIdx].title);
  updateFooter(banner, TFT_BLACK, promptCards[promptIdx].color);

  printTerminalToken("[Query Dispatched]: ", 0xFDC0);
  printTerminalToken(promptCards[promptIdx].title, 0x07FF);
  printTerminalToken("\n[Target Model]: ", 0xFDC0);
  printTerminalToken(models[currentModelIdx], 0x07E0);
  printTerminalToken("\n-----------------------------------------------\n", 0x2965);

  delay(100);
  drawCard(promptIdx, false);

  // 3. Dispatch Live Ollama Query or Fallback to Stream Simulation
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(ollamaHost);
    http.setTimeout(8000);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> reqDoc;
    reqDoc["model"] = models[currentModelIdx];
    reqDoc["prompt"] = promptCards[promptIdx].prompt;
    reqDoc["stream"] = false;

    String reqBody;
    serializeJson(reqDoc, reqBody);

    int httpCode = http.POST(reqBody);
    if (httpCode == HTTP_CODE_OK) {
      String response = http.getString();
      StaticJsonDocument<1024> respDoc;
      deserializeJson(respDoc, response);
      const char* reply = respDoc["response"];
      printTerminalToken(reply ? reply : "[Empty Response]", 0x07E0);
    } else {
      printTerminalToken("[Local Ollama Host Offline] -> Simulating Response:\n", 0xFD20);
      simulateAiResponse(promptIdx);
    }
    http.end();
  } else {
    printTerminalToken("[Standalone Demo Mode] -> Simulating Response:\n", 0x07FF);
    simulateAiResponse(promptIdx);
  }

  printTerminalToken("\n[Generation Complete]", 0xFDC0);
  playBeep(2600, 80);
  setRgbLed(false, true, false); // Green = Done
  isGenerating = false;
  footerClearTime = millis() + 4000;
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 05 Local AI Desk Terminal ---");
  Serial.println("Channel: @IamNotIntoTech");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(true, false, false); // Red = Init

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Dedicated CYD 2.8" Resistive Touch initialization
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Background Wi-Fi attempt (non-blocking)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Splash Screen
  tft.fillRect(0, 0, 320, 240, 0x0821);
  tft.fillRoundRect(20, 30, 280, 180, 8, 0x0000);
  tft.drawRoundRect(20, 30, 280, 180, 8, 0x07E0);

  tft.setTextColor(0xFDC0, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(44, 55);
  tft.print("IamNotIntoTech");

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(44, 90);
  tft.print("ELECROW 2.8\" CYD PROJECT SUITE");

  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(44, 115);
  tft.print("Project 05: Local AI Desk Terminal");

  tft.setTextColor(TFT_DARKGREY, 0x0000);
  tft.setCursor(44, 140);
  tft.print("Ollama Client | Touch Prompts | Streaming");

  playBeep(1400, 80);
  delay(100);
  playBeep(2100, 100);
  delay(500);

  drawHeader();
  drawPromptCards();
  drawTerminalWindow();
  printTerminalToken("Ready. Tap any prompt card above to query local AI model...\n", 0x07E0);
  setRgbLed(false, true, false); // Green = Ready
}

void loop() {
  // Clear action message in footer after timeout
  if (footerClearTime > 0 && millis() > footerClearTime) {
    footerClearTime = 0;
    updateFooter(WiFi.status() == WL_CONNECTED ? "Wi-Fi: Connected | Tap prompt card" : "Standalone Demo Mode | Tap prompt card");
  }

  // Touch Handling
  if (!isGenerating && ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 200) {
        lastTouchTime = millis();

        // Elecrow 2.8" CYD Landscape calibration (non-inverted Y)
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        Serial.printf("[TOUCH] Raw X=%d, Y=%d, Z=%d -> Screen (%d, %d)\n", p.x, p.y, p.z, screenX, screenY);

        // 1. Model Selector Badge click (X: 210 to 320, Y: 0 to 26)
        if (screenX >= 210 && screenY <= 26) {
          currentModelIdx = (currentModelIdx + 1) % MODEL_COUNT;
          playBeep(1800, 40);
          drawHeader();
          char buf[48];
          snprintf(buf, sizeof(buf), "Active Model Switched to: %s", models[currentModelIdx]);
          updateFooter(buf, TFT_BLACK, 0x07FF);
          footerClearTime = millis() + 2500;
          return;
        }

        // 2. Prompt Cards click (Y: 26 to 58)
        if (screenY >= 26 && screenY <= 58) {
          int cardIdx = -1;
          if (screenX >= 4 && screenX < 82) cardIdx = 0;
          else if (screenX >= 82 && screenX < 161) cardIdx = 1;
          else if (screenX >= 161 && screenX < 240) cardIdx = 2;
          else if (screenX >= 240 && screenX <= 318) cardIdx = 3;

          if (cardIdx >= 0 && cardIdx < PROMPT_COUNT) {
            streamOllamaQuery(cardIdx);
            return;
          }
        }

        // 3. Terminal Output Window tap (Y: 62 to 220) -> Clear Terminal
        if (screenY >= 62 && screenY <= 220) {
          playBeep(1600, 40);
          drawTerminalWindow();
          printTerminalToken("Terminal Cleared. Tap any prompt card above to test...\n", 0x07E0);
          return;
        }

        // 4. Footer tap -> Show system diagnostic info
        if (screenY > 220) {
          playBeep(1400, 30);
          char diag[64];
          snprintf(diag, sizeof(diag), "RAM: %d KB free | Chip: ESP32-D0WD", ESP.getFreeHeap() / 1024);
          updateFooter(diag, TFT_YELLOW, 0x0010);
          footerClearTime = millis() + 3000;
          return;
        }
      }
    }
  }

  delay(20);
}

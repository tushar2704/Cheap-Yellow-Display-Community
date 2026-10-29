/**
 * @file 05-local-ai-desk-terminal.ino
 * @brief Local-First Private AI Desk Terminal (Ollama Client)
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240
 * - Touch: XPT2046 Resistive Touch
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

#define XPT2046_IRQ   36
#define XPT2046_MOSI  13
#define XPT2046_MISO  12
#define XPT2046_CLK   14
#define XPT2046_CS    33

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

// Wi-Fi & Local Ollama Host Configuration
const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";
const char* ollamaHost = "http://192.168.1.50:11434/api/generate";

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

const char* models[] = {"llama3", "mistral", "phi3"};
int currentModelIdx = 0;

struct PromptCard {
  const char* title;
  const char* prompt;
  uint16_t color;
};

#define PROMPT_COUNT 4
PromptCard promptCards[PROMPT_COUNT] = {
  {"1. SUMMARIZE",    "Summarize this text in 3 concise bullet points:\n", 0x07E0},
  {"2. FIX CODE",     "Identify the bug in this code and write the fix:\n", 0x041F},
  {"3. EXPLAIN REGEX", "Deconstruct and explain this regex pattern step-by-step:\n", 0xFD20},
  {"4. DRAFT REPLY",  "Draft a polite, concise professional email response:\n", 0x780F}
};

int cursorX = 8;
int cursorY = 82;
bool isGenerating = false;
unsigned long lastTouchTime = 0;

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("LOCAL AI TERMINAL  ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print("IamNotIntoTech");

  // Model Selector Badge (top right)
  tft.fillRoundRect(220, 2, 94, 20, 3, 0x18E3);
  tft.drawRoundRect(220, 2, 94, 20, 3, TFT_CYAN);
  tft.setTextColor(TFT_WHITE, 0x18E3);
  tft.setCursor(226, 6);
  tft.printf("Model: %s", models[currentModelIdx]);
}

void drawPromptCards() {
  int startX = 6;
  int startY = 28;
  int cardW = 74;
  int cardH = 26;
  int gap = 5;

  for (int i = 0; i < PROMPT_COUNT; i++) {
    int cx = startX + i * (cardW + gap);
    tft.fillRoundRect(cx, startY, cardW, cardH, 3, 0x0000);
    tft.drawRoundRect(cx, startY, cardW, cardH, 3, promptCards[i].color);

    tft.setTextColor(promptCards[i].color, 0x0000);
    tft.setTextSize(1);
    tft.setCursor(cx + 4, startY + 8);
    tft.print(promptCards[i].title);
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

  // Footer
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(8, 228);
  tft.print("Tap prompt card to dispatch query | @IamNotIntoTech");
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
        // Simple scroll or wrap
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

void streamOllamaQuery(int promptIdx) {
  isGenerating = true;
  drawTerminalWindow();
  setRgbLed(false, false, true); // Blue / Cyan while busy

  printTerminalToken("[Query Dispatched]: ", 0xFDC0);
  printTerminalToken(promptCards[promptIdx].title, 0x07FF);
  printTerminalToken("\n", TFT_WHITE);

  // If Wi-Fi connected, attempt live query, otherwise run simulated tokens
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(ollamaHost);
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
      printTerminalToken("[Server Offline] Falling back to offline local buffer...\n", 0xF800);
      simulateAiResponse(promptIdx);
    }
    http.end();
  } else {
    printTerminalToken("[Demo Mode - Wi-Fi Offline]\n", 0xFD20);
    simulateAiResponse(promptIdx);
  }

  playBeep(2400, 80);
  setRgbLed(false, true, false); // Green: Done
  isGenerating = false;
}

void simulateAiResponse(int promptIdx) {
  const char* sampleReplies[4] = {
    "- Core architecture runs entirely on local edge hardware.\n- Zero cloud dependencies or recurring fees.\n- Sub-second touch latency on 2.8\" QVGA display.\n",
    "Bug found: null pointer dereference on socket buffer.\nFix:\nif (buf != NULL) {\n  process_packet(buf);\n}\n",
    "Pattern: ^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\\.[a-zA-Z0-9-.]+$\nExplanation:\n- Validates standard email addresses.\n- Enforces @ symbol and domain TLD extensions.\n",
    "Hi Team,\n\nThanks for following up. The hardware deployment has been tested and verified successfully. We are ready to proceed.\n\nBest regards,\nTushar\n"
  };

  const char* reply = sampleReplies[promptIdx];
  char token[8] = "";
  for (size_t i = 0; i < strlen(reply); i++) {
    token[0] = reply[i];
    token[1] = '\0';
    printTerminalToken(token, 0x07E0);
    delay(20);
  }
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
  setRgbLed(true, false, false);

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Splash
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
  tft.print("Ollama Client | Zero Cloud | Touch Prompts");

  playBeep(1400, 80);
  delay(100);
  playBeep(2100, 100);
  delay(500);

  drawHeader();
  drawPromptCards();
  drawTerminalWindow();
  printTerminalToken("Ready. Tap a prompt card above to test local AI models...\n", 0x07E0);
  setRgbLed(false, true, false);
}

void loop() {
  if (!isGenerating && ts.touched()) {
    TS_Point p = ts.getPoint();
    if (millis() - lastTouchTime > 250) {
      lastTouchTime = millis();

      int screenX = map(p.x, TS_MINX, TS_MAXX, 0, 320);
      int screenY = map(p.y, TS_MINY, TS_MAXY, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      // Model selector badge click (X: 220 to 314, Y: 2 to 24)
      if (screenX >= 220 && screenX <= 314 && screenY >= 2 && screenY <= 24) {
        currentModelIdx = (currentModelIdx + 1) % 3;
        playBeep(1800, 40);
        drawHeader();
        return;
      }

      // Prompt cards click (Y: 28 to 56)
      if (screenY >= 28 && screenY <= 56) {
        int startX = 6;
        int cardW = 74;
        int gap = 5;

        for (int i = 0; i < PROMPT_COUNT; i++) {
          int cx = startX + i * (cardW + gap);
          if (screenX >= cx && screenX <= cx + cardW) {
            playBeep(1900, 50);
            streamOllamaQuery(i);
            break;
          }
        }
      }
    }
  }

  delay(20);
}

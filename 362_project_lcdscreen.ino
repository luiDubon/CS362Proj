#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>

// ============ I2C ADDRESS ============
#define LCD_SLAVE_ADDR 1

// ============ LCD SETUP ============
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int LCD_COLS = 16;

String welcomeMsg = "   Welcome to the game!   ";
String bottomLine = "Press start --->";

int scrollIndex = 0;
unsigned long lastScrollTime = 0;
const unsigned long scrollInterval = 400;

// ============ GAME STATE ============
const int STATE_IDLE   = 0;
const int STATE_PLAYING = 1;
const int STATE_WIN    = 2;
const int STATE_LOSE   = 3;

int gameState = STATE_IDLE;

// I2C flags
volatile bool startFlag = false;
volatile bool winFlag   = false;
volatile bool loseFlag  = false;

// score from master (ex: l20 or l.20 → 20)
volatile int pendingScore = 0;

// ============ RGB STRIP ============
#define LED_PIN 7
#define NUM_LEDS 30

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
unsigned long lastColorTime = 0;
uint16_t colorWheelPos = 0;

// ======================================================
//                   SETUP
// ======================================================
void setup() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Loading...");
  lcd.setCursor(0,1); lcd.print(bottomLine);

  strip.begin();
  strip.setBrightness(40);
  strip.show();

  Wire.begin(1);
  Wire.onReceive(onReceiveCommand);

  gameState = STATE_IDLE;
}

// ======================================================
//                   MAIN LOOP
// ======================================================
void loop() {

  if (startFlag) {
    startFlag = false;
    startGame();
  }

  if (winFlag) {
    winFlag = false;
    showWin();
  }

  if (loseFlag) {
    loseFlag = false;
    showLose(pendingScore);   // score built from chars after 'l'
  }

  if (gameState == STATE_IDLE) {
    scrollWelcomeMessage();
  }

  fadeRGBStrip();
}

// ======================================================
//                 I2C RECEIVE
// ======================================================
void onReceiveCommand(int count) {
  if (count <= 0) return;

  // First char is the command
  char cmd = Wire.read();
  count--;

  // ---- Start Game: command 's' ----
  if (cmd == 's') {
    startFlag = true;
    // ignore any extra bytes
    while (Wire.available()) Wire.read();
    return;
  }

  // ---- Win: command 'w' ----
  if (cmd == 'w') {
    winFlag = true;
    while (Wire.available()) Wire.read();
    return;
  }

  // ---- Lose + score: command 'l' ----
  // Remaining bytes are characters of score, e.g. "20" or ".20"
  if (cmd == 'l') {
    int score = 0;

    while (Wire.available()) {
      char c = Wire.read();

      // keep only digits
      if (c >= '0' && c <= '9') {
        score = score * 10 + (c - '0');
      }
      // if it's '.', just ignore it
    }

    pendingScore = score;
    loseFlag = true;
    return;
  }

  // Unknown command: flush remaining bytes
  while (Wire.available()) Wire.read();
}

// ======================================================
//                 GAME STATE HANDLERS
// ======================================================
void startGame() {
  gameState = STATE_PLAYING;
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Game started!");
  lcd.setCursor(0,1); lcd.print("Good luck :) ");
}

void showWin() {
  gameState = STATE_WIN;
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("   YOU WIN!   ");
  lcd.setCursor(0,1); lcd.print("Nice memory :)");
}

void showLose(int score) {
  gameState = STATE_LOSE;
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("   YOU LOST!  ");

  lcd.setCursor(0,1);
  lcd.print("Reached lvl ");
  lcd.print(score);
}

// ======================================================
//                 SCROLLING TEXT
// ======================================================
void scrollWelcomeMessage() {
  unsigned long now = millis();
  if (now - lastScrollTime < scrollInterval) return;
  lastScrollTime = now;

  int len = welcomeMsg.length();
  String window = "";

  for (int i = 0; i < LCD_COLS; i++) {
    int idx = (scrollIndex + i) % len;
    window += welcomeMsg[idx];
  }

  lcd.setCursor(0,0); lcd.print(window);
  lcd.setCursor(0,1); lcd.print(bottomLine);

  scrollIndex++;
  if (scrollIndex >= len) scrollIndex = 0;
}

// ======================================================
//                 RGB FADE EFFECT
// ======================================================
void fadeRGBStrip() {
  if (millis() - lastColorTime < 20) return;
  lastColorTime = millis();

  uint32_t color = strip.ColorHSV(colorWheelPos * 256);
  strip.fill(color);
  strip.show();

  colorWheelPos++;
  if (colorWheelPos >= 255) colorWheelPos = 0;
}

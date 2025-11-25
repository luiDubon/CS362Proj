#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>

// ---------------- LCD SETUP ----------------

// LCD pin mapping: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int LCD_COLS = 16;
const int LCD_ROWS = 2;

// Scrolling welcome message
String welcomeMsg = "   Welcome to the game!   ";

int scrollIndex = 0;
unsigned long lastScrollTime = 0;
const unsigned long scrollInterval = 400;  // ms

// Bottom line when idle
String bottomLine = "Press start --->";

// ---------------- GAME STATE ----------------
const int STATE_IDLE    = 0;
const int STATE_PLAYING = 1;
const int STATE_WIN     = 2;
const int STATE_LOSE    = 3;

int gameState   = STATE_IDLE;
int currentScore = 0;
int highScore    = 0;
bool isGameOver  = false;
bool isWinState  = false;

// ---------------- RGB STRIP SETUP ----------------

#define LED_PIN 7
#define NUM_LEDS 30   // Change to your number of LEDs

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
unsigned long lastColorTime = 0;
uint16_t colorWheelPos = 0;

// ---------------- INPUT PINS FROM MASTER ----------------
// (Change these if you wired differently)
#define PIN_START 6
#define PIN_WIN   8
#define PIN_LOSE  9
#define PIN_RESET 10

// 4-bit score input
#define PIN_B0 A0
#define PIN_B1 A1
#define PIN_B2 A2
#define PIN_B3 A3

// ---------------- FUNCTION DECLARATIONS ----------------
void setupInputPins();
void checkMasterInputs();
int  readScoreFromPins();

void startGame();
void resetGame();
void updateScore(int score);
void setWin(int score);
void setLose(int score);
void showScoreScreen();

void displayIncomingMessage(const String &msg); // not used by master, but kept if you want
String padOrTrim(String s, int n);
void scrollWelcomeMessage();
void fadeRGBStrip();

// ---------------- SETUP ----------------

void setup() {
  // LCD
  lcd.begin(LCD_COLS, LCD_ROWS);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Loading...");
  lcd.setCursor(0, 1);
  lcd.print(bottomLine);

  // RGB strip
  strip.begin();
  strip.setBrightness(40);   // dimmer LEDs (0–255)
  strip.show();

  // Input pins from master
  setupInputPins();
}

// ---------------- MAIN LOOP ----------------

void loop() {
  // 1) Check wired signals from master
  checkMasterInputs();

  // 2) When idle, show scrolling welcome
  if (gameState == STATE_IDLE) {
    scrollWelcomeMessage();
  }

  // 3) RGB strip animation always runs
  fadeRGBStrip();
}

// ---------------- INPUT PIN SETUP ----------------

void setupInputPins() {
  pinMode(PIN_START, INPUT);
  pinMode(PIN_WIN,   INPUT);
  pinMode(PIN_LOSE,  INPUT);
  pinMode(PIN_RESET, INPUT);

  pinMode(PIN_B0, INPUT);
  pinMode(PIN_B1, INPUT);
  pinMode(PIN_B2, INPUT);
  pinMode(PIN_B3, INPUT);
}

// Read score from the 4-bit inputs (0–15)
int readScoreFromPins() {
  int b0 = digitalRead(PIN_B0);
  int b1 = digitalRead(PIN_B1);
  int b2 = digitalRead(PIN_B2);
  int b3 = digitalRead(PIN_B3);

  // Convert binary bits to integer
  int value = (b0) + (b1 << 1) + (b2 << 2) + (b3 << 3);
  return value;
}

// Check control lines from master and update game state
void checkMasterInputs() {
  // RESET has highest priority
  if (digitalRead(PIN_RESET) == HIGH) {
    resetGame();
    return;
  }

  // START
  if (digitalRead(PIN_START) == HIGH) {
    startGame();
    return;
  }

  // WIN
  if (digitalRead(PIN_WIN) == HIGH) {
    int score = readScoreFromPins();
    setWin(score);
    return;
  }

  // LOSE
  if (digitalRead(PIN_LOSE) == HIGH) {
    int score = readScoreFromPins();
    setLose(score);
    return;
  }

  // During game, update score continuously from pins
  if (gameState == STATE_PLAYING) {
    int score = readScoreFromPins();
    updateScore(score);
  }
}

// ---------------- GAME STATE FUNCTIONS ----------------

void startGame() {
  gameState   = STATE_PLAYING;
  isGameOver  = false;
  isWinState  = false;
  currentScore = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Game started!  ");
  lcd.setCursor(0, 1);
  lcd.print("Good luck :)   ");
}

void resetGame() {
  gameState   = STATE_IDLE;
  isGameOver  = false;
  isWinState  = false;
  currentScore = 0;

  lcd.clear();
  // scrollWelcomeMessage() will take over in loop
}

void updateScore(int score) {
  currentScore = score;
  if (currentScore > highScore) {
    highScore = currentScore;
  }

  if (gameState == STATE_PLAYING) {
    showScoreScreen();
  }
}

void setWin(int score) {
  currentScore = score;
  if (currentScore > highScore) {
    highScore = currentScore;
  }

  gameState   = STATE_WIN;
  isGameOver  = true;
  isWinState  = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   YOU WIN!    ");

  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(currentScore);
  lcd.print("  Hi:");
  lcd.print(highScore);
}

void setLose(int score) {
  currentScore = score;
  if (currentScore > highScore) {
    highScore = currentScore;   // keep best score even on loss
  }

  gameState   = STATE_LOSE;
  isGameOver  = true;
  isWinState  = false;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   YOU LOSE!   ");

  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(currentScore);
  lcd.print("  Hi:");
  lcd.print(highScore);
}

void showScoreScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Playing...     ");

  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(currentScore);
  lcd.print("  Hi:");
  lcd.print(highScore);
}

// ---------------- GENERIC TEXT DISPLAY (unused but handy) ----------------

void displayIncomingMessage(const String &msg) {
  lcd.clear();

  String line1 = msg.substring(0, min(LCD_COLS, msg.length()));
  String line2 = "";

  if (msg.length() > LCD_COLS) {
    line2 = msg.substring(LCD_COLS, min(2 * LCD_COLS, msg.length()));
  }

  lcd.setCursor(0, 0);
  lcd.print(padOrTrim(line1, LCD_COLS));

  lcd.setCursor(0, 1);
  lcd.print(padOrTrim(line2, LCD_COLS));
}

String padOrTrim(String s, int n) {
  if ((int)s.length() > n) {
    return s.substring(0, n);
  }
  while ((int)s.length() < n)
    s += " ";
  return s;
}

// ---------------- SCROLLING TEXT (IDLE) ----------------

void scrollWelcomeMessage() {
  unsigned long now = millis();
  if (now - lastScrollTime < scrollInterval) {
    return;
  }
  lastScrollTime = now;

  String window = "";
  int len = welcomeMsg.length();

  for (int i = 0; i < LCD_COLS; i++) {
    int idx = (scrollIndex + i) % len;
    window += welcomeMsg[idx];
  }

  lcd.setCursor(0, 0);
  lcd.print(window);

  lcd.setCursor(0, 1);
  lcd.print(bottomLine);

  scrollIndex++;
  if (scrollIndex >= len) {
    scrollIndex = 0;
  }
}

// ---------------- RGB FADE FUNCTION ----------------

void fadeRGBStrip() {
  unsigned long now = millis();

  // Adjust speed (bigger = slower)
  if (now - lastColorTime < 20)
    return;

  lastColorTime = now;

  uint32_t color = strip.ColorHSV(colorWheelPos * 256);
  strip.fill(color);
  strip.show();

  colorWheelPos++;
  if (colorWheelPos >= 255)
    colorWheelPos = 0;
}

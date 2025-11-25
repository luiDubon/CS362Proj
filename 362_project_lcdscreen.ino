#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>

// ------------- I2C SETUP -------------
#define LCD_SLAVE_ADDR 0x20   // change if needed

// ------------- LCD SETUP -------------

// LCD pin mapping: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int LCD_COLS = 16;
const int LCD_ROWS = 2;

// Scrolling welcome message (looped)
String welcomeMsg = "   Welcome to the game!   ";
int scrollIndex = 0;
unsigned long lastScrollTime = 0;
const unsigned long scrollInterval = 400;  // ms

// Bottom line when idle
String bottomLine = "Press start --->";

// ------------- GAME STATE -------------
const int STATE_IDLE    = 0;
const int STATE_PLAYING = 1;
const int STATE_WIN     = 2;
const int STATE_LOSE    = 3;

int gameState = STATE_IDLE;

// Flags set by I2C onReceive handler
volatile bool startFlag = false;
volatile bool winFlag   = false;
volatile bool loseFlag  = false;
volatile bool resetFlag = false;

// ------------- RGB STRIP SETUP -------------

#define LED_PIN 7
#define NUM_LEDS 30   // Change to your number of LEDs

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
unsigned long lastColorTime = 0;
uint16_t colorWheelPos = 0;

// ------------- FUNCTION DECLARATIONS -------------
void startGame();
void resetGame();
void setWin();
void setLose();

void scrollWelcomeMessage();
void fadeRGBStrip();

void printWelcomeStatic();
void printPlaying();
void printWin();
void printLose();

String padOrTrim(String s, int n);

// I2C callback
void onReceiveCommand(int numBytes);

// ------------- SETUP -------------

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
  strip.setBrightness(40);   // 0–255
  strip.show();

  // I2C slave
  Wire.begin(LCD_SLAVE_ADDR);
  Wire.onReceive(onReceiveCommand);

  // Start in idle (welcome scrolling)
  gameState = STATE_IDLE;
}

// ------------- MAIN LOOP -------------

void loop() {
  // Handle flags set from I2C interrupt
  if (resetFlag) {
    resetFlag = false;
    resetGame();
  }

  if (startFlag) {
    startFlag = false;
    startGame();
  }

  if (winFlag) {
    winFlag = false;
    setWin();
  }

  if (loseFlag) {
    loseFlag = false;
    setLose();
  }

  // When idle, run scrolling welcome text
  if (gameState == STATE_IDLE) {
    scrollWelcomeMessage();
  }

  // Always run RGB fade
  fadeRGBStrip();
}

// ------------- I2C RECEIVE HANDLER -------------

void onReceiveCommand(int numBytes) {
  while (Wire.available()) {
    char cmd = Wire.read();

    switch (cmd) {
      case 's':   // start game
        startFlag = true;
        break;
      case 'w':   // win
        winFlag = true;
        break;
      case 'l':   // lose
        loseFlag = true;
        break;
      case 'r':   // reset back to welcome
        resetFlag = true;
        break;
      default:
        // ignore unknown commands
        break;
    }
  }
}

// ------------- GAME STATE FUNCTIONS -------------

void startGame() {
  gameState = STATE_PLAYING;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Game started!  ");
  lcd.setCursor(0, 1);
  lcd.print("Good luck :)   ");
}

void resetGame() {
  gameState = STATE_IDLE;
  lcd.clear();
  // scrolling welcome will take over in loop()
}

void setWin() {
  gameState = STATE_WIN;

  lcd.clear();
  printWin();
}

void setLose() {
  gameState = STATE_LOSE;

  lcd.clear();
  printLose();
}

// ------------- LCD MESSAGE HELPERS -------------

void printWelcomeStatic() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Memory Matrix ");
  lcd.setCursor(0, 1);
  lcd.print(bottomLine);
}

void printPlaying() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Playing...     ");
  lcd.setCursor(0, 1);
  lcd.print("Focus!         ");
}

void printWin() {
  lcd.setCursor(0, 0);
  lcd.print("   YOU WIN!    ");
  lcd.setCursor(0, 1);
  lcd.print("Nice memory :) ");
}

void printLose() {
  lcd.setCursor(0, 0);
  lcd.print("   YOU LOSE    ");
  lcd.setCursor(0, 1);
  lcd.print("Try again...   ");
}

// ------------- SCROLLING TEXT (IDLE) -------------

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
  lcd.print(padOrTrim(bottomLine, LCD_COLS));

  scrollIndex++;
  if (scrollIndex >= len) {
    scrollIndex = 0;
  }
}

// ------------- RGB FADE FUNCTION -------------

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

// ------------- STRING UTIL -------------

String padOrTrim(String s, int n) {
  if ((int)s.length() > n) {
    return s.substring(0, n);
  }
  while ((int)s.length() < n) {
    s += " ";
  }
  return s;
}

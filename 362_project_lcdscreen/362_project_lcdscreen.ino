#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>
#define LCD_SLAVE_ADDR 1

// lcd setup
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int LCD_COLS = 16;

String welcomeMsg = "   Welcome to the game!   ";
String bottomLine = "Press start --->";

int scrollIndex = 0;
unsigned long lastScrollTime = 0;
const unsigned long scrollInterval = 400;

// game state
const int idle   = 0;
const int playing = 1;
const int win    = 2;
const int lose   = 3;

int gameState = idle;

// I2C flags
volatile bool startFlag = false;
volatile bool loseFlag  = false;

// score from master 
volatile int pendingScore = 0;

// ============ RGB STRIP ============
#define LED_PIN 7
#define NUM_LEDS 30

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
unsigned long lastColorTime = 0;
uint16_t colorWheelPos = 0;


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

  gameState = idle;
}


void loop() {

  if (startFlag) {
    startFlag = false;
    startGame();
  }


  if (loseFlag) {
    loseFlag = false;
    showLose(pendingScore);   // score built from chars after 'l'
  }

  if (gameState == idle) {
    scrollWelcomeMessage();
  }

  fadeRGBStrip();
}


// recieving commands
void onReceiveCommand(int count) {
  if (count <= 0) return;

  char cmd = Wire.read();
  count--;

  // start game
  if (cmd == 's') {
    startFlag = true;
    while (Wire.available()) Wire.read();
    return;
  }



  // lose game
  if (cmd == 'l') {
    int score = 0;

    while (Wire.available()) {
      char c = Wire.read();

      // keep only digits
      if (c >= '0' && c <= '9') {
        score = score * 10 + (c - '0');
      }
    }

    pendingScore = score;
    loseFlag = true;
    return;
  }

  while (Wire.available()) Wire.read();
}



void startGame() {
  gameState = playing;
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Game started!");
  lcd.setCursor(0,1); lcd.print("Good luck :) ");
}


void showLose(int score) {
  gameState = lose;
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("   YOU LOST!  ");

  lcd.setCursor(0,1);
  lcd.print("Reached lvl ");
  lcd.print(score);
}

// Scrolling welcome message
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


// rgb fading
void fadeRGBStrip() {
  if (millis() - lastColorTime < 20) return;
  lastColorTime = millis();

  uint32_t color = strip.ColorHSV(colorWheelPos * 256);
  strip.fill(color);
  strip.show();

  colorWheelPos++;
  if (colorWheelPos >= 255) colorWheelPos = 0;
}

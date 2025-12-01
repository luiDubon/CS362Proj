/*
Harasimowicz, Zachary
Lozano, Brian
Dubon, Luis
Zhang, Vincent
NetID:zhara2
      bloza4
      ldubo
      vzhan2
CS 362, FINAL GROUP PROJECT MASTER FILE
ABSTRACT:
  The memory matrix is a memory game using the keypad that challenges the player to remember and enter the number sequence as it continues to grow longer. 
  It’s built using multiple Arduinos connected with I2C, the system uses audio and visual feedback to create an arcade-style experience. 
  Players will progress through levels that also increase in difficulty. 


  Demonstrated on: 12/1/2025 @ 430PM
*/
#include <Wire.h>

const int buttonPin = 7;
int* nums = nullptr;
int* guessedNums = nullptr;
bool isStarted = false;
bool yetToGuess = false;
bool waitForGuess = false;
bool acceptingInput = false;
bool gameStarted = false;

int roundNum = 1;
int lives = 3;
int guessIndex = 0;
String numString;


int buttonState;
int lastButtonState = LOW;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 0;

int SLAVELCD = 1;
int SLAVEKEY = 3;
int SLAVESEG = 2;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  randomSeed(analogRead(3));
  pinMode(buttonPin, INPUT);

  Wire.begin();
}

void startNewRound() {
  buildNums();   
  guessIndex = 0;  
  yetToGuess = true;
  waitForGuess = true; 





  acceptingInput = true; 
}


void buildNums(){
  delete[] nums;
  nums = new int[roundNum];
  numString = "SEQ:";
  for(int i = 0; i < roundNum;++i){
    //Fill nums with random numbers 0-9 up to roundNum times
    nums[i] = random(10);
    Serial.println(nums[i]);
    numString += nums[i];
  }

  Wire.beginTransmission(SLAVESEG);
  Wire.write(numString.c_str());
  Wire.endTransmission();
}


bool checkGuess(int digit) {
  Serial.print("Received digit: ");
  Serial.println(digit);

  Serial.print("Expecting nums[");
  Serial.print(guessIndex);
  Serial.print("] = ");
  Serial.println(nums[guessIndex]);

  // Wrong digit?
  if (digit != nums[guessIndex]) {
    lives--;
    Serial.print("Wrong! Lives left: ");
    Serial.println(lives);
    Wire.beginTransmission(SLAVESEG);
    Wire.write('I');
    Wire.endTransmission();
    

    if (lives < 1) {
      Serial.println("Game over!");
      sendLoseToLCD(roundNum);
      clearKeypadBuffer();
      resetGame();
      return true;
    }

    // Let them restart this round from the beginning
    guessIndex = 0;
    return false;   
  }

  // If correct so far:
  guessIndex++;

  // Finished the whole sequence correctly?
  if (guessIndex >= roundNum) {
    Serial.println("Round complete!");

    roundNum++;        
    guessIndex = 0;
    waitForGuess = false; 
    yetToGuess = true;
    acceptingInput = false; 
    clearKeypadBuffer();

    return true;     
  }

  return false;        
}


void resetGame(){
  delete[] nums;
  delete[] guessedNums;
  nums = nullptr;
  guessedNums = nullptr;

  lives = 3;
  roundNum = 1;
  guessIndex = 0;

  isStarted = false;
  waitForGuess = false;
  yetToGuess = false;   
  acceptingInput = false;
  Wire.beginTransmission(SLAVESEG);
  Wire.write('R');
  Wire.endTransmission();
}


void readDigitsFromKeypad() {
  int bytes = Wire.requestFrom(SLAVEKEY, 32);
  if (bytes <= 0) return;
  if (!Wire.available()) return;

  // first byte is count
  int keyCount = Wire.read();

  for (int i = 0; i < keyCount && Wire.available(); ++i) {
    char c = (char)Wire.read();
    if (c >= '0' && c <= '9') {
      int d = c - '0';

      bool done = checkGuess(d);

      // If round finished OR game over, ignore remaining digits from this batch
      if (done) {
        // Flush leftover bytes in this I2C packet
        while (Wire.available()) Wire.read();
        break;
      }
    }
  }
}

void sendLoseToLCD(int levelReached) {
  Wire.beginTransmission(SLAVELCD);
  Wire.write('l');     
  Wire.print(levelReached);  
  Wire.endTransmission();
}

void clearKeypadBuffer() {
  Wire.beginTransmission(SLAVEKEY);
  Wire.write('R');  
  Wire.endTransmission();
}


void loop() {
  // put your main code here, to run repeatedly:
  if(isStarted && yetToGuess){
    if(!waitForGuess){
      startNewRound();
    }else if(acceptingInput){
      readDigitsFromKeypad();
    }



  }

  // put your main code here, to run repeatedly:
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if( (millis() - lastDebounceTime) > debounceDelay){

    if(reading != buttonState){
      buttonState = reading;


      if(buttonState == HIGH){
        resetGame();
        isStarted = true;
        yetToGuess = true;
        waitForGuess = false;

        Wire.beginTransmission(SLAVELCD);
        Wire.write('s');
        Wire.endTransmission();
      }

    }
  }



  lastButtonState = reading;
}

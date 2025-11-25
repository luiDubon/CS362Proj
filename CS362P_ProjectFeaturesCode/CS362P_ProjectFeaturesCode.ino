//library that will allow us to print and display digits very easily instead of manually lighting up every pin
#include "SevSeg.h"
//communication library using SCL, SDA pins
#include <Wire.h>

SevSeg sevseg; 
 //components for our SevSeg object
byte numDigits = 1;
byte digitPins[] = {};
byte segmentPins[] = {6, 5, 2, 3, 4, 7, 8, 9};
bool resistorsOnSegments = true;
byte hardwareConfig = COMMON_CATHODE; 

//easy, medium, hard. updated by first button. 
char difficulty[3] = {'e', 'm', 'h'};
//update on other button press.
bool gameStart = false;

char sequenceBuffer[32];
int sequenceLength = 0;
String cmd = "";
bool receivedSequence = false;
bool gotCmd = false;

const int buzzerPin = 13;
const int ledPins[3] = {10, 11, 12}; 
int lives = 3;

//needed for displaying sequence
bool showingSequence = false;
unsigned long lastDigitTime = 0;
int currentDigitIndex = 0;
unsigned long showDuration = 800;   // show digit for 800ms
unsigned long pauseDuration = 250;  // pause between digits
bool inPause = false;


void setup(){
    
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  sevseg.setBrightness(90);

  for(int i = 0; i < 3; i++){
    pinMode(ledPins[i], OUTPUT);
  }

  pinMode(buzzerPin, OUTPUT);
  //address for slave to send/receive info
  Wire.begin(0x30);
  Wire.onRequest(handleRequest);
  Wire.onReceive(handleReceive);
  
  //debugginng
  Serial.begin(9600);

}

//probably just loop on button presses, all it should do is update the difficulty 
void loop(){

    //handle button pressing and updating of lives based on whether command was received or sequence

    //if cmd was received, 
    if(gotCmd){
      
      if(cmd == "LIFE"){
        lives--;
        updateLives();
      }

      if(cmd == "CORRECT"){ buzz(true); }

      if(cmd == "INCORRECT"){ buzz(false); }

      gotCmd = false;
    }

    if(receivedSequence){
      startSequence(sequenceBuffer, sequenceLength);
      receivedSequence = false;
    }

    if (showingSequence) {
      displaySequence();
    }

    sevseg.refreshDisplay();
}



/*
Not sure exactly just how much we have to accommodate for, we could always assume 
from the master that we've received everything OK.
nevermind, just remembered we have to send difficulty and whether game should start with second pushButton. 
*/
void handleRequest(){
  //maybe send difficulty?
}

/*
Depending on the content of the message, we have to either:
Display the current string/code that the player will enter on the keypad
Update our lives
Give cue whether input was correct

*/
void handleReceive(int bytes){
  String incoming = "";

  while(Wire.available()){
    incoming += (char)Wire.read();
  }

  if(incoming.startsWith("SEQ:")){
    sequenceLength = 0;
    for(int i = 4; i < incoming.length(); i++){

      char c = incoming[i];
      
      if(c == '#') break;
      sequenceBuffer[sequenceLength++] = c;

    }

    //delimiter for safety checking
    sequenceBuffer[sequenceLength] = '\0';
    receivedSequence = true;
    return;
  }

  cmd = incoming;
  gotCmd = true;
}


//lower the lit LED which represent the lives an user has 
void updateLives(){
  for(int i = 0; i < 3; i++){
    digitalWrite(ledPins[i], LOW);
  }

  for(int i = 0; i < lives; i++){
    digitalWrite(ledPins[i], HIGH);
  }

}

//depending on boolean, emit difference pitch to our buzzer;
void buzz(bool correct){
  if(correct){
    tone(buzzerPin, 1200, 500);
  } else {
    tone(buzzerPin, 400, 500);
  }
}

void startSequence(char* sequence, int len){
  currentDigitIndex = 0;
  showingSequence = true;
  inPause = false;
  lastDigitTime = millis();
}

//display number on our 7-segment display according to our sequence
void displaySequence(){
  unsigned long now = millis();

    // Finished the whole sequence?
    if (currentDigitIndex >= sequenceLength) {
        showingSequence = false;
        sevseg.blank(); // optional
        return;
    }

    if (!inPause) {
        // Show the current digit
        int digit = sequenceBuffer[currentDigitIndex] - '0';
        sevseg.setNumber(digit);

        // If time to switch to pause
        if (now - lastDigitTime >= showDuration) {
            inPause = true;
            lastDigitTime = now;
        }
    }
    else {
        // Pause between digits
        if (now - lastDigitTime >= pauseDuration) {
            inPause = false;
            lastDigitTime = now;
            currentDigitIndex++;   // move to next digit
        }
    }
}

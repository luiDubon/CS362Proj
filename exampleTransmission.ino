#include <Wire.h>

const int buttonPin = 7;

int buttonState;
int lastButtonState = LOW;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 0;



void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin, INPUT);
  Wire.begin();

}

void loop() {
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
        Wire.beginTransmission(1);
        char c = 's';
        Wire.write(c);
        Wire.endTransmission();
      }

    }
  }



  lastButtonState = reading;
}

#include <Keypad.h>
#include <Wire.h>
#include <string.h>

#define MAX_KEYS   100      
#define SLAVE_ADDR 0x3     

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'D', '#', '0', '*'},
  {'C', '9', '8', '7'},
  {'B', '6', '5', '4'},
  {'A', '3', '2', '1'}
};

byte rowPins[ROWS] = {6, 7, 8, 9};
byte colPins[COLS] = {2, 3, 4, 5};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

char keyBuffer[MAX_KEYS];
int  keyCount = 0;


void sendKeys();
void receiveCommand(int bytes);

// setup
void setup() {
  Wire.begin(SLAVE_ADDR);
  Wire.onRequest(sendKeys);
  Wire.onReceive(receiveCommand);

  Serial.begin(9600);
  Serial.println("Keypad slave online (always-reading mode)");
}

//loop
void loop() {

  char key = customKeypad.getKey();

  if (key) {

    // If digit 0-9, store it
    if (isDigit(key)) {
      if (keyCount < MAX_KEYS) {
        keyBuffer[keyCount++] = key;
        Serial.print("Stored digit: ");
        Serial.println(key);
      }
    }

    // '#' marks the end of the sequence
    else if (key == '#') {
      Serial.println("Sequence ended with '#'");
    }

    // ignore all other keys
    else {
      Serial.print("Ignored key: ");
      Serial.println(key);
    }
  }
}

//i2c send
void sendKeys() {

  // send count first
  Wire.write((byte)keyCount);

  // send keys only if count > 0
  for (int i = 0; i < keyCount; i++) {
    Wire.write((byte)keyBuffer[i]);
  }

  Serial.print("Master requested data — sent ");
  Serial.print(keyCount);
  Serial.println(" keys.");

  // After sending, clear buffer
  keyCount = 0;
}

// i2c receive
void receiveCommand(int bytes) {

  while (Wire.available()) {
    char cmd = (char)Wire.read();

    switch (cmd) {
      // buffer reset
      case 'R':   
        keyCount = 0;
        Serial.println("Buffer reset (R)");
        break;

      default:
        Serial.print("Unknown I2C cmd: ");
        Serial.println(cmd);
        break;
    }
  }
}





#include <Keypad.h>
#include <Wire.h>
#include <string.h>

#define MAX_KEYS   100      // max digits we’ll store
#define SLAVE_ADDR 0x10     // I2C address for THIS keypad Arduino

const byte ROWS = 4;
const byte COLS = 4;

// Key layout
char hexaKeys[ROWS][COLS] = {
  {'D', '#', '0', '*'},
  {'C', '9', '8', '7'},
  {'B', '6', '5', '4'},
  {'A', '3', '2', '1'}
};

// You wired keypad pins 1–8 -> Arduino pins 2–9
byte rowPins[ROWS] = {2, 3, 4, 5};   // keypad rows
byte colPins[COLS] = {6, 7, 8, 9};   // keypad columns

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Buffer for digits 0-9
char keyBuffer[MAX_KEYS];
int  keyCount = 0;

// State flags
bool inputEnabled = false;   // true = we’re accepting keypad input
bool sendReady    = false;   // true = sequence finished & ready to send

// Forward declarations for I2C callbacks
void sendKeys();
void receiveCommand(int bytes);

// -------------------------------------------------------
// SETUP
// -------------------------------------------------------
void setup() {
  // I2C slave at address SLAVE_ADDR
  Wire.begin(SLAVE_ADDR);
  Wire.onRequest(sendKeys);        // when master calls Wire.requestFrom()
  Wire.onReceive(receiveCommand);  // when master calls Wire.beginTransmission()

  Serial.begin(9600);
  Serial.println("Keypad slave online");

  // TEMP: uncomment this line if you want to test keypad
  // without the master sending 'E' (enable) first:
  // inputEnabled = true;
}

// -------------------------------------------------------
// MAIN LOOP
// -------------------------------------------------------
void loop() {
  char key = customKeypad.getKey();

  if (!inputEnabled) {
    return;   // ignore keypad until master enables it
  }

  if (key) {
    // Accept only digits 0–9
    if (isDigit(key)) {
      if (keyCount < MAX_KEYS) {
        keyBuffer[keyCount++] = key;

        Serial.print("Stored key: ");
        Serial.println(key);
      } else {
        Serial.println("Buffer full, ignoring extra keys");
      }
    }
    // '#' ends the sequence and marks it ready to send
    else if (key == '#') {
      sendReady    = true;
      inputEnabled = false;   // stop reading until master is done
      Serial.println("Sequence complete, ready to send...");
    }
    // Everything else ignored (A,B,C,D,*, etc.)
    else {
      Serial.print("Unaccepted input: ");
      Serial.println(key);
    }
  }
}

// -------------------------------------------------------
// I2C onRequest: master is asking for data
// -------------------------------------------------------
void sendKeys() {
  if (!sendReady) {
    // Not ready; send count 0 so master knows
    Wire.write((byte)0);
    Serial.println("Master requested data, but not ready...");
    return;
  }

  // First send how many keys
  Wire.write((byte)keyCount);

  // Then send each digit
  for (int i = 0; i < keyCount; i++) {
    Wire.write((byte)keyBuffer[i]);
  }

  Serial.print("Sent ");
  Serial.print(keyCount);
  Serial.println(" keys to master");

  // Reset for next sequence
  keyCount   = 0;
  sendReady  = false;
}

// -------------------------------------------------------
// I2C onReceive: master sends us commands
// -------------------------------------------------------
void receiveCommand(int bytes) {
  while (Wire.available()) {
    char cmd = (char)Wire.read();

    switch (cmd) {
      case 'R':     // Reset / clear sequence
        keyCount   = 0;
        sendReady  = false;
        Serial.println("Sequence cleared (R)");
        break;

      case 'E':     // Enable keypad reading
        inputEnabled = true;
        sendReady    = false;
        Serial.println("Reading enabled (E)");
        break;

      case 'D':     // Disable keypad reading
        inputEnabled = false;
        Serial.println("Reading disabled (D)");
        break;

      default:
        Serial.print("Unknown command: ");
        Serial.println(cmd);
        break;
    }
  }
}



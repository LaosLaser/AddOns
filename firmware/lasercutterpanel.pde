// Laser cutter panel on Arduino
// by Jaap Vermaas <jaap@fablabtruck.nl>
//
// test programma voor de arduino duemilanove
//
// multi master I2C to MBED on 42
//
// include libraries:
#include <LiquidCrystal.h> // LCD display driver
#include <Wire.h> // Wire library for I2C
#include <Keypad.h>

// initialize libraries with numbers of the interface pins

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'#','0','*'}
};
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {A0, A1, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
// I2C is using analog pins 4 and 5
// Arduino analog input 4 = I2C SDA 
// Arduino analog input 5 = I2C SCL 

static int ourI2C = 2;

char newkey, lastkey, keystate, blinkcnt;
int counter = 0;

void setup(){
  pinMode(A0,OUTPUT);
  digitalWrite(A0,HIGH);
  pinMode(A1,OUTPUT);
  digitalWrite(A1,HIGH);
  pinMode(A2, OUTPUT);
  analogWrite(A2, 255);
  Serial.begin(9600); 
  Wire.begin(ourI2C); // start Wire library as I2C-Bus Client
  Wire.onReceive(receiveEvent);  //register I2C events
  Wire.onRequest(requestEvent);
  blinkcnt = 255;
  // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  lcd.noCursor();
  lcd.noAutoscroll();
  
  // Print a message to the LCD.
  //lcd.print("I2C laser-panel");
  //Serial.println("rebooted");
  lcd.clear();
  
}

void loop(){
  char key = keypad.getKey();
  keystate = keypad.getState();
  if (key != NO_KEY) {  // Not NULL
    newkey = key;
    Serial.print("Key pressed: ");
    Serial.println(key);
    lastkey = 1;
  } 
  if (blinkcnt <255) {
    delay(100);
    blinkcnt = 255;
    analogWrite(A2, 255);
  }
}

void requestEvent() {
    if ((lastkey==1) || (keystate == HOLD) || (keystate == PRESSED)) {
      lastkey = 0;
      Wire.send(newkey);
    } else {
      Wire.send(0);
    }
}
  
void receiveEvent(int howMany) {
  byte y;
  while (howMany--) {
    y = Wire.receive();
    switch (y) {
        case 255:  // LCD Clear cmd
                    lcd.clear();
                    Serial.println("LCDClear");
                    break;
        case 254:  // LCD Home cmd
                    lcd.home();
                    Serial.println("LCDHome");
                    break;
        case 9:    // Tab is blink screen
                    analogWrite(A2, 0);
                    blinkcnt = 0;
                    break;
        case 10:  // New line
                    lcd.setCursor(0,1);
                    Serial.println("Newline");
                    break;
        case 13:  // Cariage return
                    Serial.println("Carriage return");
                    lcd.setCursor(0,1);
                    break;
        default:    // all other keys
                    Serial.print(y);
                    if ((y > 31) && (y< 125)) {
                         lcd.write(y);
                    }
    }
  }
}

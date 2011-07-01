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

char newkey;
int counter = 0;

void setup(){
  pinMode(A0,OUTPUT);
  digitalWrite(A0,HIGH);
  pinMode(A1,OUTPUT);
  digitalWrite(A1,HIGH);
  Serial.begin(9600); 
  Wire.begin(ourI2C); // start Wire library as I2C-Bus Client
  Wire.onReceive(receiveEvent);  //register I2C events
  Wire.onRequest(requestEvent);
  
  // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  lcd.noCursor();
  
  // Print a message to the LCD.
  lcd.print("I2C laser-panel");
  Serial.println("rebooted");
}

void loop(){
  
  char key = keypad.getKey();
  if (key != NO_KEY) {  // Not NULL
    newkey = key;
    Serial.println(key);
  }
}

void requestEvent() {
  if (newkey != 0) {
    char x;
    Wire.send(newkey);
    newkey = 0;
    Serial.print("Sending character: ");
    Serial.println(newkey);
  } else {
    Serial.print("Sending empty string, no input\n");
    Wire.send(0);
  }
}
  
void receiveEvent(int howMany) {
  int cnt = 0;
  lcd.clear();
  byte y;
  while (Wire.available()) {
    cnt++;
    y = Wire.receive();
    lcd.write(y);
    Serial.write(y);
    if (cnt == 16) {
       lcd.setCursor(0,1);
    }
  }
}

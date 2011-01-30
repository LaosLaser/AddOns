// Laser cutter panel on Arduino
// by Jaap Vermaas <jaap@fablabtruck.nl>
//
// test programma voor de arduino duemilanove
//
// multi master I2C to MBED on 42
//
// include libraries:
#include <LiquidCrystal.h> // LCD display driver
#include <Gkos.h>  // 6-key keyboard library
#include <Wire.h> // Wire library for I2C

// initialize libraries with numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
Gkos gkos(0, 2, 3, 4, 5, 6);

// I2C is using analog pins 4 and 5
// Arduino analog input 4 = I2C SDA 
// Arduino analog input 5 = I2C SCL 

static char* gEntry = "";
static int ourI2C = 2;

char buffer[10] = "";
int counter = 0;

void setup(){
  Serial.begin(9600); 
  Wire.begin(ourI2C); // start Wire library as I2C-Bus Client
  Wire.onReceive(receiveEvent);  //register I2C events
  Wire.onRequest(requestEvent);
  
  // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  lcd.noCursor();
  
  // Print a message to the LCD.
  lcd.print("I2C laser-panel");
}

void loop(){
  gEntry = gkos.entry(); // Will return empty immediately if no entry
  if ((gEntry[0] != 0) && (counter<10)) {  // Not NULL
    buffer[counter++] = gEntry[0];
    Serial.write(gEntry[0]);
  }
}

void requestEvent() {
  char x;
  Wire.send(buffer[0]);
  for (int x=1; x<counter; x++) {
    buffer[x-1] = buffer[x];
  }
  counter--;
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

/* Laser cutter panel on Arduino
 * by Jaap Vermaas <jaap@fablabtruck.nl>
 *enee
 * This code is part of the LAOS - Laser Open Source project
 * see: http://www.laoslaser.org
 *
 *   LaOS is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   LaOS is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with LaOS.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Important notice: I2C on the Arduino (wire.h) uses 7-bit addressing with
 * the 8th bit being reserved for read/write.
 * So the Arduino code in this example uses I2C address 2, but on the
 * mbed, this corresponds to 0x04
 */
  
// include libraries:
#include <LiquidCrystal.h>   	// LCD display driver
#include <Wire.h>           	// Wire library for I2C
#include <Keypad.h>         	// Keypad driver

// define keypad (as used by Keypad.h)
const byte ROWS = 2; 
const byte COLS = 5; 
char keys[ROWS][COLS] = {
  {'2','6','8','5','4'},
  {'9','3','0','1','7'}
};

// define Arduino pins
#define LCDBACKLIGHT 9
#define LCDCONTRAST 10
#define DEBUG 0
#define BUZZER A1

LiquidCrystal lcd(A3, A2, A0, 11, 12, 13);
byte rowPins[ROWS] = {7,8}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2,3,4,5,6}; //connect to the column pinouts of the keypad
// I2C is using analog pins 4 and 5
// Arduino analog input A4 = I2C SDA
// Arduino analog input A5 = I2C SCL
// Serial RXD = pin 0, TXD = pin 1
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
static int ourI2C = 2; // this defines us as I2C port 4!
byte newkey, lastkey, keystate, blink;
int counter = 0;
int posx = 0;
int posy = 0;

void setup(){
  Wire.begin(ourI2C); // start Wire library as I2C-Bus Client
  Wire.onReceive(receiveEvent);  //register I2C events
  Wire.onRequest(requestEvent);
  // initialize LCD screen
  // setPwmFrequency(LCDCONTRAST, 1);
  TCCR1B = TCCR1B & 0b11111000 | 0x01;

  pinMode(LCDBACKLIGHT, OUTPUT);
  analogWrite(LCDBACKLIGHT, 100);
  pinMode(LCDCONTRAST, OUTPUT);
  analogWrite(LCDCONTRAST, 70);
  pinMode(BUZZER, OUTPUT);
  
  // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  lcd.noCursor();
  lcd.noAutoscroll();
  lcd.print("I2C laser-panel");
  lcd.setCursor(0,1);
  lcd.print("laoslaser.org");
 
  if (DEBUG == 1) {
    Serial.begin(9600);
    Serial.println("LAOS panel setup completed");
  }
}

void buzz(){
  for (int x=0; x<15; x++) {
      analogWrite(BUZZER, 255);
      delay(1);
      analogWrite(BUZZER, 0);
  }
}
  
void loop(){
  byte key = keypad.getKey();
  keystate = keypad.getState();
  if (key != NO_KEY) {  // Not NULL
    newkey = key;
    if (DEBUG == 1) {
      Serial.print("Key pressed: ");
      Serial.println(key);
      lcd.setCursor(0,1);
      lcd.write(key);
    }
    buzz();
    lastkey = 1;
  }
}

void requestEvent() {
    if ((lastkey==1) || (keystate == HOLD) || (keystate == PRESSED)) {
      
      Wire.write(newkey);
      if (DEBUG == 1) {
        if (lastkey==0) {
          Serial.print("I2C send key:");
        } else {
          Serial.print("I2C resend key: ");
        }
        Serial.println(newkey);
      }
      lastkey = 0;
    } else {
      byte tmpkey = 0;
      Wire.write(tmpkey);
      if (DEBUG == 1) {
        //Serial.println("I2C: no key to send");
      }  
    }
}
  
void receiveEvent(int howMany) {
  //lcd.clear();
  //posx = 0; posy = 0;
  while (howMany--) {
    byte y = Wire.read();
    switch (y) {
        case 255:  // LCD Clear cmd
                    lcd.clear();
                    posx = 0;
                    posy = 0;
                    if (DEBUG == 1) {
                       Serial.println("I2C received LCDClear");
                    }
                    break;
        case 254:  // LCD Home cmd
                    lcd.home();
                    posx = 0;
                    posy = 0;
                    if (DEBUG == 1) {
                       Serial.println("I2C received LCDHome");
                    }
                    break;
        case 10:  // New line
                    if (posy == 0) {
                      lcd.setCursor(0,1);
                      posy = 1;
                    } else {
                      lcd.setCursor(0,0);
                      posy = 0;
                    }
                    if (DEBUG == 1) {
                       Serial.println("I2C received Newline");
                    }
                    break;
        case 13:  // Cariage return
                    if (DEBUG == 1) {
                       Serial.println("I2C received Carriage return");
                    }
                    if (posy == 0) {
                      lcd.setCursor(0,1);
                      posy = 1;
                    } else {
                      lcd.setCursor(0,0);
                      posy = 0;
                    }
                    break;
        case 7:  // Bell
                    if (DEBUG == 1) {
                        Serial.println("Bell character");
                    }
                    buzz();
                    break;
        default:    // all other keys
                    if (DEBUG == 1) { 
                        Serial.print(y); 
                    }
                    if ((y > 31) && (y< 125)) {
                         if ((posx == 16) && (posy == 0)) {
                           posx=0; posy=1;
                           lcd.setCursor(0,1);
                         } 
                         if ((posx == 16) && (posy== 1)) {
                           posx=0; posy=0;
                           lcd.clear();
                         }
                         lcd.write(y);
                         posx++;
                    }
    }
  }
}

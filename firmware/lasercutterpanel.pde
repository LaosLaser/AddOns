/* Laser cutter panel on Arduino
 * by Jaap Vermaas <jaap@fablabtruck.nl>
 *
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
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

// define Arduino pins
#define LCDBACKLIGHT A1
#define LASERLED A2
#define OKLED A3
#define DEBUG 0
LiquidCrystal lcd(13, A0, 12, 11, 10, 9);
byte rowPins[ROWS] = {5,6,7,8}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2,3,4}; //connect to the column pinouts of the keypad
// I2C is using analog pins 4 and 5
// Arduino analog input A4 = I2C SDA
// Arduino analog input A5 = I2C SCL
// Serial RXD = pin 0, TXD = pin 1
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
static int ourI2C = 2; // this defines us as I2C port 4!
char newkey, lastkey, keystate, blink;
int counter = 0;
char lled, okled;

void setup(){
  // initialize LCD screen
  pinMode(LCDBACKLIGHT, OUTPUT);
  analogWrite(LCDBACKLIGHT, 255);
  // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  lcd.noCursor();
  lcd.noAutoscroll();
  lcd.clear();
  lcd.print("I2C laser-panel");
  lcd.setCursor(0,1);
  lcd.print("laoslaser.org");
  
  pinMode(LASERLED, OUTPUT);
  analogWrite(LASERLED, 0);
  pinMode(OKLED, OUTPUT);
  analogWrite(OKLED, 0);
  
  delay(5000);
  lcd.clear();
  analogWrite(OKLED, 0);
  analogWrite(LASERLED, 255);
  lled = 0;
  okled = 0;
  
  Serial.begin(9600);
  Wire.begin(ourI2C); // start Wire library as I2C-Bus Client
  Wire.onReceive(receiveEvent);  //register I2C events
  Wire.onRequest(requestEvent);
  blink = 0;
  if (DEBUG) {
    Serial.println("LAOS panel setup completed");
  }
}


void loop(){
  char key = keypad.getKey();
  keystate = keypad.getState();
  if (key != NO_KEY) {  // Not NULL
    newkey = key;
    if (DEBUG) {
      Serial.print("Key pressed: ");
      Serial.println(key);
      lcd.setCursor(0,1);
      lcd.write(key);
    }
    lastkey = 1;
  } 
  if (blink == 1) {
    delay(100);
    blink = 0;
    analogWrite(LCDBACKLIGHT, 255);
  }
}

void requestEvent() {
    if ((lastkey==1) || (keystate == HOLD) || (keystate == PRESSED)) {
      
      Wire.send(newkey);
      if (DEBUG) {
        if (lastkey==0) {
          Serial.print("I2C send key:");
        } else {
          Serial.print("I2C resend key: ");
        }
        Serial.println(newkey);
      }
      lastkey = 0;
    } else {
      Wire.send(0);
      if (DEBUG) {
        Serial.println("I2C: no key to send");
      }  
    }
}
  
void receiveEvent(int howMany) {
  byte y;
  while (howMany--) {
    y = Wire.receive();
    switch (y) {
        case 255:  // LCD Clear cmd
                    lcd.clear();
                    if (DEBUG) {
                       Serial.println("I2C received LCDClear");
                    }
                    break;
        case 254:  // LCD Home cmd
                    lcd.home();
                    if (DEBUG) {
                       Serial.println("I2C received LCDHome");
                    }
                    break;
        case 7:	  // Turn on/off ok led
		    if (okled == 255) okled = 0; else okled = 255;
		    analogWrite(OKLED, okled);
                    if (DEBUG) {
                       Serial.println("I2C received OK LED switch");
                    }
		    break; 
	case 8:	  // Turn on/off laser led
		    if (okled == 255) okled = 0; else okled=255;
		    analogWrite(LASERLED, lled);
                    if (DEBUG) {
                       Serial.println("I2C received LASER LED switch");
                    }
		    break; 
        case 9:    // Tab is blink screen
                    //analogWrite(LCDBACKLIGHT, 0);
                    if (DEBUG) {
                       Serial.println("I2C received Blink screen");
                    }
                    blink = 1;
                    break;
        case 10:  // New line
                    lcd.setCursor(0,1);
                    if (DEBUG) {
                       Serial.println("I2C received Newline");
                    }
                    break;
        case 13:  // Cariage return
                    if (DEBUG) {
                       Serial.println("I2C received Carriave return");
                    }
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

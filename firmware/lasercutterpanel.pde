// Laser cutter panel on Arduino
// by Jaap Vermaas <jaap@fablabtruck.nl>
//
// multi master I2C to MBED, channel 4 (not 2!)
//
// include libraries:
#include <LiquidCrystal.h>   // LCD display driver
#include <Wire.h>           // Wire library for I2C
#include <Keypad.h>         // Keypad driver

// define keypad
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

// define Arduino pins
#define LCDBACKLIGHT A2
#define LASERLED A3
#define OKLED 4
#define DEBUG 0
LiquidCrystal lcd(2, 3, 5, 6, 7, 8);
byte rowPins[ROWS] = {13,12,11,10}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {A1, A0, 9}; //connect to the column pinouts of the keypad
// I2C is using analog pins 4 and 5
// Arduino analog input A4 = I2C SDA = 
// Arduino analog input A5 = I2C SCL

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


static int ourI2C = 2; // this defines us as I2C port 4!

char newkey, lastkey, keystate, blink;
int counter = 0;
bool lled, okled;

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
  digitalWrite(LASERLED, HIGH);
  pinMode(OKLED, OUTPUT);
  digitalWrite(OKLED, LOW);
  
  delay(5000);
  lcd.clear();
  digitalWrite(OKLED, HIGH);
  digitalWrite(LASERLED, LOW);
  lled = LOW;
  okled = HIGH;
  
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
		    okled = not(okled);
		    digitalWrite(OKLED, okled);
                    if (DEBUG) {
                       Serial.println("I2C received OK LED switch");
                    }
		    break; 
	case 8:	  // Turn on/off laser led
		    lled = not(lled);
		    digitalWrite(LASERLED, lled);
                    if (DEBUG) {
                       Serial.println("I2C received LASER LED switch");
                    }
		    break; 
        case 9:    // Tab is blink screen
                    analogWrite(LCDBACKLIGHT, 0);
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

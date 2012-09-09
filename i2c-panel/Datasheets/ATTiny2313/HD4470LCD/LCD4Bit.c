/*
LCD4Bit v0.1 16/Oct/2006 neillzero http://abstractplain.net

What is this?
An arduino library for comms with HD44780-compatible LCD, in 4-bit mode (saves pins)

Sources:
- The original "LiquidCrystal" 8-bit library and tutorial
    http://www.arduino.cc/en/uploads/Tutorial/LiquidCrystal.zip
    http://www.arduino.cc/en/Tutorial/LCDLibrary
- DEM 16216 datasheet http://www.maplin.co.uk/Media/PDFs/N27AZ.pdf
- Massimo's suggested 4-bit code (I took initialization from here) http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1144924220/8
See also:
- glasspusher's code (probably more correct): http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1160586800/0#0

Tested only with a DEM 16216 (maplin "N27AZ" - http://www.maplin.co.uk/Search.aspx?criteria=N27AZ)
If you use this successfully, consider feeding back to the arduino wiki with a note of which LCD it worked on.

Usage:
see the examples folder of this library distribution.

Modified 5th February 2010 John Crouchley for the ATTiny2313 I2C LCD controller 

*/

#include "LCD4Bit.h"
#include <stdio.h>  //not needed yet
#include <string.h> //needed for strlen()
#include <inttypes.h>
#include "WConstants.h"  //all things wiring / arduino
#include <avr/eeprom.h>
#include "I2C_LCD.h"

//command bytes for LCD
#define CMD_CLR 0x01
#define CMD_RIGHT 0x1C
#define CMD_LEFT 0x18
#define CMD_HOME 0x02

// Useful macros
#define output_low(port,pin) port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)
#define output_nibble(port,nibble) port = (port & 0xf0) | (nibble & 0x0f)

// --------- PINS -------------------------------------

//RS and Enable can be set to whatever you like
#define RS_PORT PORTA
#define RS_PIN 1
#define RS_DDR DDRA

#define ENABLE_PORT PORTB
#define ENABLE_PIN 6
#define ENABLE_DDR DDRB

//DB should be on one port pins 0-3 
#define LCD_INTERFACE_PORT PORTB
#define LCD_INTERFACE_DDR DDRB

//--------------------------------------------------------

//pulse the Enable pin high (for a microsecond).
//This clocks whatever command or data is in DB4~7 into the LCD controller.
void pulseEnablePin(){
  // send a pulse to enable
  output_high(ENABLE_PORT, ENABLE_PIN);
  delayMicroseconds(1);
  output_low(ENABLE_PORT, ENABLE_PIN);
  delayMicroseconds(1);
}

//push a nibble of data through the the LCD's DB4~7 pins, clocking with the Enable pin.
//We don't care what RS and RW are, here.
void pushNibble(uint8_t value){
  output_nibble(LCD_INTERFACE_PORT, value);
  pulseEnablePin();
}

//push a byte of data through the LCD's DB4~7 pins, in two steps, clocking each with the enable pin.
void pushByte(uint8_t value){
  pushNibble(value >> 4);
  pushNibble(value);
}


//stuff the library user might call---------------------------------

void commandWriteNibble(uint8_t nibble) {
  output_low(RS_PORT, RS_PIN);
  pushNibble(nibble);
}


void commandWrite(uint8_t value) {
  output_low(RS_PORT, RS_PIN);
  pushByte(value);
  if (value < 0x04)	// home or clear command
	delay(2);			// 2mS delay
  else
	delayMicroseconds(50);
}

//print the given character at the current cursor position. overwrites, doesn't insert.
void print(uint8_t value) {
  output_high(RS_PORT, RS_PIN); //set the RS pin to show we're writing data
  pushByte(value);		//let pushByte worry about the intricacies of Enable, nibble order.
  delayMicroseconds(50);
}

void printStrEE(uint8_t* value)
{
  while (eeprom_read_byte(value)) print(eeprom_read_byte(value++));
}

//send the clear screen command to the LCD
void clear(){
  commandWrite(CMD_CLR);
}


// initiatize lcd after a short pause
//while there are hard-coded details here of lines, cursor and blink settings, you can override these original settings after calling .init()
void LCD_init (void) {
  RS_DDR |= (1<<RS_PIN);
  LCD_INTERFACE_DDR |= (1<<ENABLE_PIN) | 0x0f;	// enable output on pins 0-3 - enable pin is on same port
  output_low(ENABLE_PORT, ENABLE_PIN);
  delay(50);		// why??

  //The first 4 nibbles and timings are not in my DEM16217 SYH datasheet, but apparently are HD44780 standard...
  commandWriteNibble(0x03);
  delay(5);
  commandWriteNibble(0x03);
  delayMicroseconds(100);
  commandWriteNibble(0x03);
  delay(5);

  // needed by the LCDs controller
  //this being 2 sets up 4-bit mode.
  commandWriteNibble(0x02);
  commandWriteNibble(eeprom_read_byte(&ee_lcd_function_mode) >> 4);

  //NFXX where
  //N = num lines (0=1 line or 1=2 lines).
  //F= format (number of dots (0=5x7 or 1=5x10)).
  //X=don't care
  commandWriteNibble(eeprom_read_byte(&ee_lcd_function_mode));
  delayMicroseconds(60);

  //The rest of the init is not specific to 4-bit mode.
  //NOTE: we're writing full bytes now, not nibbles.

  // display control:
  // turn display on, cursor off, no blinking
  commandWrite(eeprom_read_byte(&ee_cursor_direction));

  // entry mode
  // increment automatically, display shift, entire shift off
  commandWrite(eeprom_read_byte(&ee_entry_mode));

  //clear display
  commandWrite(eeprom_read_byte(&ee_lcd_clear));
}

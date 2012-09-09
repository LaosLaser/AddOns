/********************************************************************************

ATTiny2313 I2C Keypad and LCD controller.
main code

Created 5th February 2010 by John Crouchley
johng at crouchley.me.uk

This program is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
********************************************************************************/

#include <avr/io.h>     //Include the headers that handles Input/Output function for AVR chips
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "I2C_LCD.h"
#include "usiTwiSlave.h"
#include "LCD4Bit.h"
#include "wiring.h"
#include "Keyboard.h"

uint8_t EEMEM b_my_ee_address = MY_ADDRESS;
uint8_t EEMEM b_my_ee_version_number = MY_VERSION_NUMBER;
uint8_t EEMEM b_ee_lcd_function_mode = LCD_FUNCTION_MODE;
uint8_t EEMEM b_ee_cursor_direction = CURSOR_DIRECTION;
uint8_t EEMEM b_ee_entry_mode = ENTRY_MODE;
uint8_t EEMEM b_ee_lcd_clear = LCD_CLEAR;
uint8_t EEMEM b_ee_keyb_debounce = KEYB_DEBOUNCE;
uint8_t EEMEM b_ee_spare=0;
uint8_t EEMEM b_ee_keyb_map[13] = {0,'1','2','3','4','5','6','7','8','9','*','0','#'};
uint8_t EEMEM b_ee_init_str[13] = {'J','G','C',' ','I','2','C',' ','V',VERSION_HIGH,'.',VERSION_LOW,0};
uint8_t EEMEM my_ee_address = MY_ADDRESS;
uint8_t EEMEM my_ee_version_number = MY_VERSION_NUMBER;
uint8_t EEMEM ee_lcd_function_mode = LCD_FUNCTION_MODE;
uint8_t EEMEM ee_cursor_direction = CURSOR_DIRECTION;
uint8_t EEMEM ee_entry_mode = ENTRY_MODE;
uint8_t EEMEM ee_lcd_clear = LCD_CLEAR;
uint8_t EEMEM ee_keyb_debounce = KEYB_DEBOUNCE;
uint8_t EEMEM ee_spare=0;
uint8_t EEMEM ee_keyb_map[13] = {0,'1','2','3','4','5','6','7','8','9','*','0','#'};
uint8_t EEMEM ee_init_str[13] = {'J','G','C',' ','I','2','C',' ','V',VERSION_HIGH,'.',VERSION_LOW,0};


//Functions

void init_EE(void)
{
	// not initialised
	// so initialise (write eeprom)
	for (uint8_t i=0; i < (uint16_t)&my_ee_address ; i++)
		eeprom_write_byte(&my_ee_address + i, eeprom_read_byte(&b_my_ee_address + i));
}

void init_mem(void)
{
	PORTD = 0x70; // enable pullup keypad columns
	cli();	// disable interrupts
	init();	// initialize timers etc.
	// pullups should have had time to take effect (had problems without delay)
	if ((PIND & 0x10) == 0) init_EE();	// force reset of EEPROM

	DDRB |= (1<<PB4);			// backlight is an output
	usiTwiSlaveInit(eeprom_read_byte(&my_ee_address));
	initKeyb();
	sei();		// LCD_init uses delay - which require interrupts.
	LCD_init();
	printStrEE((uint8_t*)&ee_init_str);
	PORTB |= (1<<PB4);		// backlight on
}


void processTWI( void )
{
	uint8_t b,c;

	b = usiTwiReceiveByte();
	switch (b)
	{
	case 0xFE:					// LCD command
		// commands expect a second character
		c = usiTwiReceiveByte();
		switch (c)
		{
		case 0xFE:				// escape for 0xFE
			print(c);
			break;
		case 0xFF:				// escape for 0xFF
			print(c);
			break;
		default:
			commandWrite(c);
			break;
		}
		break;
	case 0xFF:					// special commands
		c = usiTwiReceiveByte();
		switch (c)
		{
		case 0x01:				// back light 0=off 1=on
			if (usiTwiReceiveByte())	// on
				PORTB |= (1<<PB4);		// make sure it is on
			else
				PORTB &= ~(1<<PB4);		// make sure it is off
			break;
		case 0x02:				// read eeprom byte
			c = usiTwiReceiveByte();
			// - we need the space (costs 18 bytes) -  if (c > 128 -(uint16_t)&my_ee_address) break;
			usiTwiTransmitByte(eeprom_read_byte(&my_ee_address + c));
			break;
		case 0x03:				// write eeprom byte
			c = usiTwiReceiveByte();
			// - we need the space (costs 18 bytes) - if (c > 128 -(uint16_t)&my_ee_address) {usiTwiReceiveByte(); break;}
			eeprom_write_byte(&my_ee_address + c, usiTwiReceiveByte() );
			break;
		case 0x04:
			printStrEE((uint8_t*)&my_ee_address + usiTwiReceiveByte() );
			break;
		case 0x10:				// number of characters in keypad buffer
			usiTwiTransmitByte((KeyHead-KeyTail)&0x0f);
			break;
		case 0x11:				// read keyboard
			usiTwiTransmitByte(readKeyb());
			break;
		case 0x12:				// KeyDown
			if (KeyTimer) usiTwiTransmitByte(0);
			else usiTwiTransmitByte(LastKeyState);
			break;
		case 0x13:				// Clear keybuff
			KeyHead = KeyTail = 0;	//
			output_low(PORTA,1);	// clear interrupt
			break;
		case 0x14:				// read keyboard bytes
			c = usiTwiReceiveByte();
			while (c--)
				usiTwiTransmitByte(readKeyb());
			break;
		case 0x15:				// switch off interrupt pin
			PORTA &= ~0x01;
			break;
		case 0x16:				// switch on interrupt pin
			PORTA |= 0x01;
			break;
		case 0xF0:				// Reset EEPROM to defaults and init all
			init_EE();
		case 0xF1:				// software reset
			init_mem();
			break;
		}
		break;
	default:					// output whatever is received as data
		print(b);
		break;
	}
}

//Main Function
int main(void)
{
	init_mem();	// read memory and set up the IO

	for (;;)   //Loop forever
	{
		while (usiTwiDataInReceiveBuffer())	// process I2C command
		{
			processTWI();
		}
	}
}


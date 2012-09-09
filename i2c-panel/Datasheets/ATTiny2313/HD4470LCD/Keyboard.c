/********************************************************************************

ATTiny2313 I2C Keypad and LCD controller.
Keypad code

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

#include "Keyboard.h"
#include <stdio.h>  //not needed yet
#include <string.h> //needed for strlen()
#include <inttypes.h>
//#include "Wiring.h"
#include "WConstants.h"  //all things wiring / arduino
#include <avr/eeprom.h>
#include "I2C_LCD.h"

uint8_t KeyBuffer[MAX_KEYB_BUFFER];
uint8_t KeyHead;
uint8_t KeyTail;
uint8_t KeyTimer;
uint8_t LastKeyState;

void initKeyb(void)
{
	// Rows PD0-PD3,  Cols PD4,PD5,PD6 Interrupt PA0
	DDRD = 0;	// PD0-PD6 input
	DDRA |= 0x01;	// PA0 output
	PORTA &= ~0x01;	// ensure interrupt off

	KeyTimer = LastKeyState = KeyHead = KeyTail = 0;
}

uint8_t performScan(void)
{
	uint8_t i,j;
	for (i=0; i<4; i++)
	{
		DDRD = (1 << i);	// scanned row as output
		PORTD = 0x70;	// need this write after setting output bit - pullup on the columns
					// and force a low output on the row.
		j=~(PIND >> 4) & 0x07;
		DDRD = 0;		// set all rows back to input
		if (j > 3) j=3;	// bit 0x04 = column 3
		if (j) break;
	}
	if (j) return i*3 + j;
	return 0;
}

uint8_t readKeyb(void)
{
	uint8_t ret;
	if (KeyHead == KeyTail) return eeprom_read_byte(&ee_keyb_map[0]);
	ret = KeyBuffer[KeyTail++];
	KeyTail &= 0x0f;
	if (KeyHead == KeyTail) PORTA &= ~0x01;	// ensure interrupt off if buffer empty
	return ret;
}

void writeKeyb(uint8_t value)
{
	if (((KeyHead+1) & 0x0f) == KeyTail) return;	// overflow
	KeyBuffer[KeyHead++] = value;
	KeyHead &= 0x0f;
	PORTA |= 0x01;					// interrupt on
}

void scanKeyb( void )		// called once a mS by timer interrupt
{
	uint8_t KeyState = performScan();
	if (KeyState != LastKeyState)
	{
		KeyTimer = eeprom_read_byte(&ee_keyb_debounce);
		LastKeyState = KeyState;
	}
	else if (KeyTimer)	// we have had a key change - and are debouncing
	{
		if (!(--KeyTimer))
		{
			// KeyState has been steady for the debounce time
			if (KeyState)	// we have a key (otherwise we just debounced key up
				writeKeyb(eeprom_read_byte(&ee_keyb_map[KeyState]));
			// KeyTimer is now zero and will not trigger a read until a change is seen.
		}
	}
}
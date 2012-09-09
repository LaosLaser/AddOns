/********************************************************************************

ATTiny2313 I2C Keypad and LCD controller.
Keypad header code

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

#ifndef Keyboard_h
#define Keyboard_h

#include <inttypes.h>
#include <avr/eeprom.h>

#define MAX_KEYB_BUFFER 16	// used for some nify masking

extern uint8_t EEMEM ee_keyb_debounce;

extern uint8_t KeyHead;
extern uint8_t KeyTail;
extern uint8_t KeyTimer;
extern uint8_t LastKeyState;

void initKeyb(void);
uint8_t readKeyb(void);
void scanKeyb(void);	// called from timer interrupt

#endif
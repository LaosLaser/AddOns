/********************************************************************************

ATTiny2313 I2C Keypad and LCD controller.
header

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
#ifndef I2C_LCD_h
#define I2C_LCD_h

#include <avr/eeprom.h>


// defaults
#define MY_ADDRESS 0x12
#define MY_VERSION_NUMBER 0x0a	// 0.1
#define VERSION_HIGH '0'
#define VERSION_LOW 'A'
#define LCD_FUNCTION_MODE 0x28
#define CURSOR_DIRECTION 0x0e
#define ENTRY_MODE 0x06
#define LCD_CLEAR 0x01
#define KEYB_DEBOUNCE 30	// 30 mS debounce

extern uint8_t EEMEM my_ee_address;
extern uint8_t EEMEM my_ee_version_number;
extern uint8_t EEMEM ee_lcd_function_mode;
extern uint8_t EEMEM ee_cursor_direction;
extern uint8_t EEMEM ee_entry_mode;
extern uint8_t EEMEM ee_lcd_clear;
extern uint8_t EEMEM ee_keyb_debounce;
extern uint8_t EEMEM ee_spare;
extern uint8_t EEMEM ee_keyb_map[13];
extern uint8_t EEMEM ee_init_str[13];


// helper macros
#define output_low(port,pin) port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)

#endif

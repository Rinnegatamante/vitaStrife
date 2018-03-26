//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//       Key definitions
//

#ifndef __DOOMKEYS__
#define __DOOMKEYS__

//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#define KEYP_MULTIPLY   '*'
#define KEY_MINUS	0x2d
#define KEY_CAPSLOCK    (0x80+0x3a)
#define KEY_NUMLOCK     (0x80+0x45)
#define KEY_SCRLCK      (0x80+0x46)
#define KEY_PRTSCR      (0x80+0x59)
#define KEYP_5          '5'
#define KEYP_PLUS       0x7F
#define KEYP_MINUS      0x1F
/*
#define KEY_UPARROW             173
#define KEY_DOWNARROW		9
#define KEY_LEFTARROW           91
#define KEY_RIGHTARROW          93

#define KEY_TRIANGLE		175
#define KEY_CROSS		157
#define KEY_SQUARE		47
#define KEY_CIRCLE		32

//#define KEY_TRIANGLE		'A'
//#define KEY_CROSS		'B'
//#define KEY_SQUARE		'C'
//#define KEY_CIRCLE		'D'

#define KEY_SELECT		27
#define KEY_START		13
#define KEY_LEFTTRIGGER		44
#define KEY_RIGHTTRIGGER	46

//#define KEY_RIGHTARROW	0xae
//#define KEY_LEFTARROW	0xac
//#define KEY_UPARROW	0xad
//#define KEY_DOWNARROW	0xaf

//#define KEY_ESCAPE	27
//#define KEY_ENTER	13
//#define KEY_TAB		0xaf

#define KEY_F1		(0x80+0x3b)
#define KEY_F2		(0x80+0x3c)
#define KEY_F3		(0x80+0x3d)
#define KEY_F4		(0x80+0x3e)
#define KEY_F5		(0x80+0x3f)
#define KEY_F6		(0x80+0x40)
#define KEY_F7		(0x80+0x41)
#define KEY_F8		(0x80+0x42)
#define KEY_F9		(0x80+0x43)
#define KEY_F10		(0x80+0x44)
#define KEY_F11		(0x80+0x57)
#define KEY_F12		(0x80+0x58)

#define KEY_BACKSPACE	0x7f
#define KEY_PAUSE	0xff

//#define KEY_EQUALS	0x3d

#define KEY_RSHIFT	(0x80+0x36)
#define KEY_RCTRL	(0x80+0x1d)
#define KEY_RALT	(0x80+0x38)

//#define KEY_LALT	KEY_RALT

// new keys:

#define KEY_HOME        (0x80+0x47)
#define KEY_END         (0x80+0x4f)
#define KEY_PGUP        (0x80+0x49)
#define KEY_PGDN        (0x80+0x51)
#define KEY_INS         (0x80+0x52)
#define KEY_DEL         (0x80+0x53)

//#define KEY_1          0x31
//#define KEY_2          0x32
//#define KEY_3          0x33
//#define KEY_4          0x34

//#define KEYP_0          0
//#define KEYP_1          KEY_END
//#define KEYP_2          KEY_DOWNARROW
//#define KEYP_3          KEY_PGDN
//#define KEYP_4          KEY_LEFTARROW

//#define KEYP_6          KEY_RIGHTARROW
//#define KEYP_7          KEY_HOME
//#define KEYP_8          KEY_UPARROW
//#define KEYP_9          KEY_PGUP

//#define KEYP_DIVIDE     '/'

//#define KEYP_PERIOD     0
//#define KEYP_EQUALS     KEY_EQUALS
//#define KEYP_ENTER      KEY_ENTER
*/

//#define	KEY_RIGHTARROW		0xae
//#define	KEY_LEFTARROW		0xac
//#define	KEY_UPARROW			0xad
//#define	KEY_DOWNARROW		0xaf
//#define	KEY_ESCAPE			27
//#define	KEY_ENTER			13
//#define	KEY_F1				(0x80+0x3b)
//#define	KEY_F2				(0x80+0x3c)
//#define	KEY_F3				(0x80+0x3d)
//#define	KEY_F4				(0x80+0x3e)
//#define	KEY_F5				(0x80+0x3f)
//#define	KEY_F6				(0x80+0x40)
//#define	KEY_F7				(0x80+0x41)
//#define	KEY_F8				(0x80+0x42)
//#define	KEY_F9				(0x80+0x43)
//#define	KEY_F10				(0x80+0x44)
//#define	KEY_F11				(0x80+0x57)
//#define	KEY_F12				(0x80+0x58)

//#define	KEY_BACKSPACE		127
//#define	KEY_PAUSE			0xff

//#define KEY_EQUALS			0x3d
//#define KEY_MINUS			0x2d

//#define	KEY_RSHIFT			(0x80+0x36)
//#define	KEY_RCTRL			(0x80+0x1d)
//#define	KEY_RALT			(0x80+0x38)

//#define	KEY_LALT			KEY_RALT

#define KEY_FIVE			0x35
#define KEY_SIX				0x36
#define KEY_SEVEN			0x37
#define KEY_EIGHT			0x38
#define KEY_NINE			0x39
#define KEY_ZERO			0x30

//#define KEY_LEFTBRACKET		91
//#define KEY_SEMICOLON		59

#define KEY_UPARROW             173
#define KEY_DOWNARROW		9
#define KEY_LEFTARROW           91
#define KEY_RIGHTARROW          93
#define KEY_TRIANGLE		175
#define KEY_CROSS		157
#define KEY_SQUARE		47
#define KEY_CIRCLE		32
#define KEY_SELECT		27
#define KEY_START		13
#define KEY_LEFTTRIGGER		44
#define KEY_RIGHTTRIGGER	46

#define KEY_TAB			9

#define KEY_COMMA		44
#define KEY_SLASH		47
#define KEY_BACKSLASH		0x5C
#define KEY_EQUALS              0x3d

#define KEY_ENTER               13
#define	KEY_PGUP		150
#define	KEY_PGDN		149
#define	KEY_INS			147
#define KEY_HOME                (0x80+0x5b)
#define KEY_END                 (0x80+0x5c)
#define	KEY_DEL			148
#define KEY_RSHIFT              (0x80+0x36)
#define KEY_RALT                (0x80+0x38)
#define KEY_RCTRL               (0x80+0x1d)

#define KEY_ESCAPE              27
#define KEY_F1                  (0x80+0x3b)
#define KEY_F2                  (0x80+0x3c)
#define KEY_F3                  (0x80+0x3d)
#define KEY_F4                  (0x80+0x3e)
#define KEY_F5                  (0x80+0x3f)
#define KEY_F6                  (0x80+0x40)
#define KEY_F7                  (0x80+0x41)
#define KEY_F8                  (0x80+0x42)
#define KEY_F9                  (0x80+0x43)
#define KEY_F10                 (0x80+0x44)
#define KEY_F11                 (0x80+0x57)
#define KEY_F12                 (0x80+0x58)

//#define KEY_INSERT              (0x80+0x59)
//#define KEY_DELETE              (0x80+0x5a)
//#define KEY_PAGEUP              (0x80+0x5d)
//#define KEY_PAGEDOWN            (0x80+0x5f)

#define KEY_BACKSPACE           127
#define KEY_PAUSE               0xff
#define KEY_LALT                KEY_RALT

#endif          // __DOOMKEYS__


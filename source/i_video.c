// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for SDL library
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <vitasdk.h>
#include <vita2d.h>

#include "m_config.h"
#include "deh_str.h"
#include "w_wad.h"
#include "z_zone.h"
#include "i_swap.h"
#include "doomkeys.h"
#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
//#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

vita2d_texture *tex_buffer;

static byte *disk_image = NULL;
//static byte *saved_background;

//static SceCtrlData ctl;
static SceCtrlData pad;
static SceCtrlData oldpad;

static unsigned int __attribute__((aligned(16))) list[262144];

// display has been set up?

boolean initialized = false;

unsigned int __attribute__((aligned(16))) colors[256];

void *fbp0;

long SCREENMULT = SCREENWIDTH*SCREENHEIGHT;

// Fake mouse handling.
//boolean		grabMouse;

extern boolean wipe;
extern boolean display_ticker;

extern int followplayer;
extern int next_weapon;

extern int frame_limit;
/*

// Lookup table for mapping ASCII characters to their equivalent when
// shift is pressed on an American layout keyboard:

static const char shiftxform[] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, ' ', '!', '"', '#', '$', '%', '&',
    '"', // shift-'
    '(', ')', '*', '+',
    '<', // shift-,
    '_', // shift--
    '>', // shift-.
    '?', // shift-/
    ')', // shift-0
    '!', // shift-1
    '@', // shift-2
    '#', // shift-3
    '$', // shift-4
    '%', // shift-5
    '^', // shift-6
    '&', // shift-7
    '*', // shift-8
    '(', // shift-9
    ':',
    ':', // shift-;
    '<',
    '+', // shift-=
    '>', '?', '@',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '[', // shift-[
    '!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
    ']', // shift-]
    '"', '_',
    '\'', // shift-`
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '{', '|', '}', '~', 127
};


#define LOADING_DISK_W 16
#define LOADING_DISK_H 16

// Non aspect ratio-corrected modes (direct multiples of 320x200)

static screen_mode_t *screen_modes[] = {
    &mode_scale_1x,
    &mode_scale_2x,
    &mode_scale_3x,
    &mode_scale_4x,
    &mode_scale_5x,
};

// Aspect ratio corrected modes (4:3 ratio)

static screen_mode_t *screen_modes_corrected[] = {

    // Vertically stretched modes (320x200 -> 320x240 and multiples)

    &mode_stretch_1x,
    &mode_stretch_2x,
    &mode_stretch_3x,
    &mode_stretch_4x,
    &mode_stretch_5x,

    // Horizontally squashed modes (320x200 -> 256x200 and multiples)

    &mode_squash_1x,
    &mode_squash_2x,
    &mode_squash_3x,
    &mode_squash_4x,
    &mode_squash_5x,
};

// SDL video driver name

char *video_driver = "";

// Window position:

static char *window_position = "";

// SDL surface for the screen.

static SDL_Surface *screen;

// Window title

static char *window_title = "";

// Intermediate 8-bit buffer that we draw to instead of 'screen'.
// This is used when we are rendering in 32-bit screen mode.
// When in a real 8-bit screen mode, screenbuffer == screen.

static SDL_Surface *screenbuffer = NULL;

// palette

static SDL_Color palette[256];
static boolean palette_to_set;

// disable mouse?

static boolean nomouse = false;
int usemouse = 1;

// Bit mask of mouse button state.

static unsigned int mouse_button_state = 0;

// Disallow mouse and joystick movement to cause forward/backward
// motion.  Specified with the '-novert' command line parameter.
// This is an int to allow saving to config file

int novert = 0;

// Save screenshots in PNG format.

int png_screenshots = 0;

// if true, I_VideoBuffer is screen->pixels

static boolean native_surface;

// Screen width and height, from configuration file.

int screen_width = SCREENWIDTH;
int screen_height = SCREENHEIGHT;

// Color depth.

int screen_bpp = 0;

// Automatically adjust video settings if the selected mode is 
// not a valid video mode.

static int autoadjust_video_settings = 1;

// Run in full screen mode?  (int type for config code)

int fullscreen = true;

// Aspect ratio correction mode

int aspect_ratio_correct = true;

// Time to wait for the screen to settle on startup before starting the
// game (ms)

static int startup_delay = 1000;

// Grab the mouse? (int type for config code)

static int grabmouse = true;

// The screen buffer; this is modified to draw things to the screen

byte *I_VideoBuffer = NULL;

// If true, game is running as a screensaver

boolean screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible;

// If true, we display dots at the bottom of the screen to 
// indicate FPS.

static boolean display_fps_dots;

// If this is true, the screen is rendered but not blitted to the
// video buffer.

static boolean noblit;

// Callback function to invoke to determine whether to grab the 
// mouse pointer.

static grabmouse_callback_t grabmouse_callback = NULL;

// disk image data and background overwritten by the disk to be
// restored by EndRead

static boolean window_focused;

// Empty mouse cursor

static SDL_Cursor *cursors[2];

// The screen mode and scale functions being used

static screen_mode_t *screen_mode;

// Window resize state.

static boolean need_resize = false;
static unsigned int resize_w, resize_h;
static unsigned int last_resize_time;

// If true, keyboard mapping is ignored, like in Vanilla Doom.
// The sensible thing to do is to disable this if you have a non-US
// keyboard.

int vanilla_keyboard_mapping = true;

// Is the shift key currently down?

static int shiftdown = 0;

// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of mouse_threshold, they are multiplied
// by mouse_acceleration to increase the speed.

float mouse_acceleration = 2.0;
int mouse_threshold = 10;

// Gamma correction level to use
*/

// If true, we display dots at the bottom of the screen to 
// indicate FPS.
 
struct Vertex
{
	float u,v;
	float x,y,z;
};


//
// I_StartFrame
//
void I_StartFrame (void)
{
	// er...???
}

void CheckKey(int key, int gamekey)
{
    event_t event;
    if(pad.buttons & key && !(oldpad.buttons & key))
    {
	event.type = ev_keydown;
	event.data1 = gamekey;
	D_PostEvent(&event);
    }
    if(!(pad.buttons & key) && oldpad.buttons & key)
    {
	event.type = ev_keyup;
	event.data1 = gamekey;
	D_PostEvent(&event);
    }
}
/*
static void ApplyWindowResize(unsigned int w, unsigned int h);

static boolean MouseShouldBeGrabbed()
{
    // never grab the mouse when in screensaver mode
   
    if (screensaver_mode)
        return false;

    // if the window doesn't have focus, never grab it

    if (!window_focused)
        return false;

    // always grab the mouse when full screen (dont want to 
    // see the mouse pointer)

    if (fullscreen)
        return true;

    // Don't grab the mouse if mouse input is disabled

    if (!usemouse || nomouse)
        return false;

    // if we specify not to grab the mouse, never grab

    if (!grabmouse)
        return false;

    // Invoke the grabmouse callback function to determine whether
    // the mouse should be grabbed

    if (grabmouse_callback != NULL)
    {
        return grabmouse_callback();
    }
    else
    {
        return true;
    }
}

void I_SetGrabMouseCallback(grabmouse_callback_t func)
{
    grabmouse_callback = func;
}

// Update the value of window_focused when we get a focus event
//
// We try to make ourselves be well-behaved: the grab on the mouse
// is removed if we lose focus (such as a popup window appearing),
// and we dont move the mouse around if we aren't focused either.

static void UpdateFocus(void)
{
    Uint8 state;

    state = SDL_GetAppState();

    // We should have input (keyboard) focus and be visible 
    // (not minimized)

    window_focused = (state & SDL_APPINPUTFOCUS) && (state & SDL_APPACTIVE);

    // Should the screen be grabbed?

    screenvisible = (state & SDL_APPACTIVE) != 0;
}

// Show or hide the mouse cursor. We have to use different techniques
// depending on the OS.

static void SetShowCursor(boolean show)
{
    // On Windows, using SDL_ShowCursor() adds lag to the mouse input,
    // so work around this by setting an invisible cursor instead. On
    // other systems, it isn't possible to change the cursor, so this
    // hack has to be Windows-only. (Thanks to entryway for this)

#ifdef _WIN32
    if (show)
    {
        SDL_SetCursor(cursors[1]);
    }
    else
    {
        SDL_SetCursor(cursors[0]);
    }
#else
    SDL_ShowCursor(show);
#endif

    // When the cursor is hidden, grab the input.

    if (!screensaver_mode)
    {
        SDL_WM_GrabInput(!show);
    }
}

//
// Translates the SDL key
//

static int TranslateKey(SDL_keysym *sym)
{
    switch(sym->sym)
    {
      case SDLK_LEFT:	return KEY_LEFTARROW;
      case SDLK_RIGHT:	return KEY_RIGHTARROW;
      case SDLK_DOWN:	return KEY_DOWNARROW;
      case SDLK_UP:	return KEY_UPARROW;
      case SDLK_ESCAPE:	return KEY_ESCAPE;
      case SDLK_RETURN:	return KEY_ENTER;
      case SDLK_TAB:	return KEY_TAB;
      case SDLK_F1:	return KEY_F1;
      case SDLK_F2:	return KEY_F2;
      case SDLK_F3:	return KEY_F3;
      case SDLK_F4:	return KEY_F4;
      case SDLK_F5:	return KEY_F5;
      case SDLK_F6:	return KEY_F6;
      case SDLK_F7:	return KEY_F7;
      case SDLK_F8:	return KEY_F8;
      case SDLK_F9:	return KEY_F9;
      case SDLK_F10:	return KEY_F10;
      case SDLK_F11:	return KEY_F11;
      case SDLK_F12:	return KEY_F12;
      case SDLK_PRINT:  return KEY_PRTSCR;

      case SDLK_BACKSPACE: return KEY_BACKSPACE;
      case SDLK_DELETE:	return KEY_DEL;

      case SDLK_PAUSE:	return KEY_PAUSE;

#if !SDL_VERSION_ATLEAST(1, 3, 0)
      case SDLK_EQUALS: return KEY_EQUALS;
#endif

      case SDLK_MINUS:          return KEY_MINUS;

      case SDLK_LSHIFT:
      case SDLK_RSHIFT:
	return KEY_RSHIFT;
	
      case SDLK_LCTRL:
      case SDLK_RCTRL:
	return KEY_RCTRL;
	
      case SDLK_LALT:
      case SDLK_RALT:
#if !SDL_VERSION_ATLEAST(1, 3, 0)
      case SDLK_LMETA:
      case SDLK_RMETA:
#endif
        return KEY_RALT;

      case SDLK_CAPSLOCK: return KEY_CAPSLOCK;
      case SDLK_SCROLLOCK: return KEY_SCRLCK;
      case SDLK_NUMLOCK: return KEY_NUMLOCK;

      case SDLK_KP0: return KEYP_0;
      case SDLK_KP1: return KEYP_1;
      case SDLK_KP2: return KEYP_2;
      case SDLK_KP3: return KEYP_3;
      case SDLK_KP4: return KEYP_4;
      case SDLK_KP5: return KEYP_5;
      case SDLK_KP6: return KEYP_6;
      case SDLK_KP7: return KEYP_7;
      case SDLK_KP8: return KEYP_8;
      case SDLK_KP9: return KEYP_9;

      case SDLK_KP_PERIOD:   return KEYP_PERIOD;
      case SDLK_KP_MULTIPLY: return KEYP_MULTIPLY;
      case SDLK_KP_PLUS:     return KEYP_PLUS;
      case SDLK_KP_MINUS:    return KEYP_MINUS;
      case SDLK_KP_DIVIDE:   return KEYP_DIVIDE;
      case SDLK_KP_EQUALS:   return KEYP_EQUALS;
      case SDLK_KP_ENTER:    return KEYP_ENTER;

      case SDLK_HOME: return KEY_HOME;
      case SDLK_INSERT: return KEY_INS;
      case SDLK_END: return KEY_END;
      case SDLK_PAGEUP: return KEY_PGUP;
      case SDLK_PAGEDOWN: return KEY_PGDN;

#ifdef SDL_HAVE_APP_KEYS
        case SDLK_APP1:        return KEY_F1;
        case SDLK_APP2:        return KEY_F2;
        case SDLK_APP3:        return KEY_F3;
        case SDLK_APP4:        return KEY_F4;
        case SDLK_APP5:        return KEY_F5;
        case SDLK_APP6:        return KEY_F6;
#endif

      default:
        return tolower(sym->sym);
    }
}

static void UpdateMouseButtonState(unsigned int button, boolean on)
{
    event_t event;

    if (button < SDL_BUTTON_LEFT || button > MAX_MOUSE_BUTTONS)
    {
        return;
    }

    // Note: button "0" is left, button "1" is right,
    // button "2" is middle for Doom.  This is different
    // to how SDL sees things.

    switch (button)
    {
        case SDL_BUTTON_LEFT:
            button = 0;
            break;

        case SDL_BUTTON_RIGHT:
            button = 1;
            break;

        case SDL_BUTTON_MIDDLE:
            button = 2;
            break;

        default:
            // SDL buttons are indexed from 1.
            --button;
            break;
    }

    // Turn bit representing this button on or off.

    if (on)
    {
        mouse_button_state |= (1 << button);
    }
    else
    {
        mouse_button_state &= ~(1 << button);
    }

    // Post an event with the new button state.

    event.type = ev_mouse;
    event.data1 = mouse_button_state;
    event.data2 = event.data3 = 0;
    D_PostEvent(&event);
}

static int AccelerateMouse(int val)
{
    if (val < 0)
        return -AccelerateMouse(-val);

    if (val > mouse_threshold)
    {
        return (int)((val - mouse_threshold) * mouse_acceleration + mouse_threshold);
    }
    else
    {
        return val;
    }
}

// Get the equivalent ASCII (Unicode?) character for a keypress.

static int GetTypedChar(SDL_Event *event)
{
    int key;

    // If Vanilla keyboard mapping enabled, the keyboard
    // scan code is used to give the character typed.
    // This does not change depending on keyboard layout.
    // If you have a German keyboard, pressing 'z' will
    // give 'y', for example.  It is desirable to be able
    // to fix this so that people with non-standard 
    // keyboard mappings can type properly.  If vanilla
    // mode is disabled, use the properly translated 
    // version.

    if (vanilla_keyboard_mapping)
    {
        key = TranslateKey(&event->key.keysym);

        // Is shift held down?  If so, perform a translation.

        if (shiftdown > 0)
        {
            if (key >= 0 && key < arrlen(shiftxform))
            {
                key = shiftxform[key];
            }
            else
            {
                key = 0;
            }
        }

        return key;
    }
    else
    {
        // Unicode value, from key layout.

        return tolower(event->key.keysym.unicode);
    }
}

static void UpdateShiftStatus(SDL_Event *event)
{
    int change;

    if (event->type == SDL_KEYDOWN)
    {
        change = 1;
    }
    else if (event->type == SDL_KEYUP)
    {
        change = -1;
    }
    else
    {
        return;
    }

    if (event->key.keysym.sym == SDLK_LSHIFT 
     || event->key.keysym.sym == SDLK_RSHIFT)
    {
        shiftdown += change;
    }
}
*/

static unsigned long control_bef_ctl  = 0;

#define KEY_1               0x02

unsigned long key;

#define DO_KEY_EVENT(pspkey, gamekey) \
{ \
    if (key & pspkey) \
    { \
        kbevent.type = ev_keydown; \
        kbevent.data1 = gamekey; \
        D_PostEvent(&kbevent); \
    } \
    else \
    { \
        kbevent.type = ev_keyup; \
        kbevent.data1 = gamekey; \
        D_PostEvent(&kbevent); \
    } \
}

extern int button_layout;

unsigned long Read_Key(void)
{
    sceCtrlReadBufferPositive(0, &pad, 1); 

    if (pad.buttons == control_bef_ctl)
    {
	return 0;
    }
    control_bef_ctl  = pad.buttons;
    return control_bef_ctl;
}

boolean dont_move_forwards = false;

void I_GetEvent(void)
{
/*
    SDL_Event sdlevent;
    event_t event;

    // possibly not needed
    
    SDL_PumpEvents();

    // put event-grabbing stuff in here
    
    while (SDL_PollEvent(&sdlevent))
    {
        // ignore mouse events when the window is not focused

        if (!window_focused 
         && (sdlevent.type == SDL_MOUSEMOTION
          || sdlevent.type == SDL_MOUSEBUTTONDOWN
          || sdlevent.type == SDL_MOUSEBUTTONUP))
        {
            continue;
        }

        if (screensaver_mode && sdlevent.type == SDL_QUIT)
        {
            I_Quit();
        }

        UpdateShiftStatus(&sdlevent);

        // process event
        
        switch (sdlevent.type)
        {
            case SDL_KEYDOWN:
                // data1 has the key pressed, data2 has the character
                // (shift-translated, etc)
                event.type = ev_keydown;
                event.data1 = TranslateKey(&sdlevent.key.keysym);
                event.data2 = GetTypedChar(&sdlevent);

                if (event.data1 != 0)
                {
                    D_PostEvent(&event);
                }
                break;

            case SDL_KEYUP:
                event.type = ev_keyup;
                event.data1 = TranslateKey(&sdlevent.key.keysym);

                // data2 is just initialized to zero for ev_keyup.
                // For ev_keydown it's the shifted Unicode character
                // that was typed, but if something wants to detect
                // key releases it should do so based on data1
                // (key ID), not the printable char.

                event.data2 = 0;

                if (event.data1 != 0)
                {
                    D_PostEvent(&event);
                }
                break;

//            case SDL_MOUSEMOTION:
//                event.type = ev_mouse;
//                event.data1 = mouse_button_state;
//                event.data2 = AccelerateMouse(sdlevent.motion.xrel);
//                event.data3 = -AccelerateMouse(sdlevent.motion.yrel);
//                D_PostEvent(&event);
//                break;

            case SDL_MOUSEBUTTONDOWN:
		if (usemouse && !nomouse)
		{
                    UpdateMouseButtonState(sdlevent.button.button, true);
		}
                break;

            case SDL_MOUSEBUTTONUP:
		if (usemouse && !nomouse)
		{
                    UpdateMouseButtonState(sdlevent.button.button, false);
		}
                break;

            case SDL_QUIT:
                event.type = ev_quit;
                D_PostEvent(&event);
                break;

            case SDL_ACTIVEEVENT:
                // need to update our focus state
                UpdateFocus();
                break;

            case SDL_VIDEOEXPOSE:
                palette_to_set = true;
                break;

            case SDL_RESIZABLE:
                need_resize = true;
                resize_w = sdlevent.resize.w;
                resize_h = sdlevent.resize.h;
                last_resize_time = SDL_GetTicks();
                break;

            default:
                break;
        }
    }
*/
    static event_t	event;
    static event_t	kbevent;

    int			xmickeys, ymickeys;

//    unsigned long	key;

    oldpad = pad;

    sceCtrlPeekBufferPositive(0, &pad, 1);

    if(button_layout == 0)				// FOR PS VITA
    {
    	if(wipe == false)				// FOR PSP: check for SCREEN WIPE and if so, DON'T...
    	{						// ...let ANY key filter through (prevent game hangup)
	    CheckKey(SCE_CTRL_UP, KEY_UPARROW);
	    CheckKey(SCE_CTRL_LEFT, KEY_LEFTARROW);
	    CheckKey(SCE_CTRL_RIGHT, KEY_RIGHTARROW);
	    CheckKey(SCE_CTRL_DOWN, KEY_DOWNARROW);
	    CheckKey(SCE_CTRL_TRIANGLE, KEY_TRIANGLE);
	    CheckKey(SCE_CTRL_CROSS, KEY_CROSS);
	    CheckKey(SCE_CTRL_SQUARE, KEY_SQUARE);
	    CheckKey(SCE_CTRL_CIRCLE, KEY_CIRCLE);
	    CheckKey(SCE_CTRL_SELECT, KEY_SELECT);
	    CheckKey(SCE_CTRL_START, KEY_START);
	    CheckKey(SCE_CTRL_LTRIGGER, KEY_LEFTTRIGGER);
	    CheckKey(SCE_CTRL_RTRIGGER, KEY_RIGHTTRIGGER);

	    xmickeys = ymickeys = 0;

	    if(pad.lx < 40) xmickeys = -16;
	    if(pad.lx > 216) xmickeys = 16;
	    if(pad.lx < 2) xmickeys = -64;
	    if(pad.lx > 253) xmickeys = 64;
	
	    if(pad.ly < 40) ymickeys = -8;
	    if(pad.ly > 216) ymickeys = 8;
	    if(pad.ly < 2) ymickeys = -32;
	    if(pad.ly > 253) ymickeys = 32;

	    if((xmickeys!=0)||(ymickeys!=0))
	    {
	    	event.type=ev_mouse;
	    	event.data1=0;
	    	event.data2=xmickeys;
	    	event.data3=-ymickeys;
	    	D_PostEvent(&event);
	    }
        }
    }
    else if(button_layout == 1)				// FOR PSP
    {
    	if(wipe == false)				// FOR PSP: check for SCREEN WIPE and if so, DON'T...
    	{						// ...let ANY key filter through (prevent game hangup)
	    CheckKey(SCE_CTRL_UP, KEY_UPARROW);
	    CheckKey(SCE_CTRL_LEFT, '[');
	    CheckKey(SCE_CTRL_RIGHT, ']');
	    CheckKey(SCE_CTRL_DOWN, KEY_DOWNARROW);
	    CheckKey(SCE_CTRL_TRIANGLE, KEY_TRIANGLE);
	    CheckKey(SCE_CTRL_CROSS, KEY_CROSS);
	    CheckKey(SCE_CTRL_SQUARE, '/');
	    CheckKey(SCE_CTRL_CIRCLE, ' ');
	    CheckKey(SCE_CTRL_SELECT, KEY_SELECT);
	    CheckKey(SCE_CTRL_START, KEY_START);
	    CheckKey(SCE_CTRL_LTRIGGER, ',');
	    CheckKey(SCE_CTRL_RTRIGGER, '.');

	    xmickeys = ymickeys = 0;

	    if(pad.lx < 40) xmickeys = -16;
	    if(pad.lx > 216) xmickeys = 16;
	    if(pad.lx < 2) xmickeys = -64;
	    if(pad.lx > 253) xmickeys = 64;
	
	    if(pad.ly < 40) ymickeys = -8;
	    if(pad.ly > 216) ymickeys = 8;
	    if(pad.ly < 2) ymickeys = -32;
	    if(pad.ly > 253) ymickeys = 32;

	    if((xmickeys!=0)||(ymickeys!=0))
	    {
	    	event.type=ev_mouse;
	    	event.data1=0;
	    	event.data2=xmickeys;
	    	event.data3=-ymickeys;
	    	D_PostEvent(&event);
	    }
        }
    }

/*									// FOR PSP: WILL KEEP THIS...
    if (gamestate == GS_LEVEL && (!menuactive || !automapactive))	// ...HERE (CHANGES WEAPON),...
    {									// ...BUT ONLY CHANGES TO...
	if(pad.buttons != oldpad.buttons)				// ...NORMAL WEAPONS...
	{								// ...(SWITCHES TO ALTERNATE...
	    oldpad = pad;						// ...WEAPONS ONLY IF WE RUN...
	    if(pad.buttons & SCE_CTRL_UP)				// ...OUT OF AMMO FOR NORMAL...
	    {								// ...WEAPONS)
		next_weapon = 1;
	    }								// THEREFORE DEACTIVATED
	}
    }
*/
    static player_t*	plyrweap; 
					// BUGGY ??? IN A CERTAIN CASE, IT ACTIVATED AUTOMAP WHILE...
    weapontype_t	num;		// ...IT SHOULDN'T. I MODIFIED THE CODE AFTERWARDS AND IT...
					// ...DIDN'T HAPPEN AGAIN FOR ME, BUT IS IT REALLY GONE ???
    key = Read_Key();

    extern boolean	gamekeydown[NUMKEYS];
    extern int 		key_use;

    if (gamestate == GS_LEVEL && !(menuactive || automapactive))
    {
	plyrweap = &players[consoleplayer];

	num = plyrweap->readyweapon;

	if(pad.buttons != oldpad.buttons)
	{
	    oldpad = pad;

	    if (key & SCE_CTRL_UP && button_layout == 0)
	    {
	        while (1)
	        {
		    dont_move_forwards = true;
/*
								// PSP-FIXME: DOESN'T WORK AS EXPECTED
								// (SKIP CHANGING TO GUNS WITH NO AMMO)
		    if (plyrweap->ammo[weaponinfo[num].ammo] == 0)
		    {
		        num = plyrweap->pendingweapon;
		    }
		    else
*/
		        num++;

		    if (num > wp_torpedo)
		        num = wp_fist;

		    if (plyrweap->weaponowned[num])
		    {
		        plyrweap->pendingweapon = num;

		        break;
		    }
	        }
	        kbevent.type = ev_keydown;
	        kbevent.data1 = KEY_1 + num;

	        D_PostEvent(&kbevent);

		dont_move_forwards = false;
	    }
	    else if (key & SCE_CTRL_UP && button_layout == 1 && gamekeydown[key_use])
	    {
	        while (1)
	        {
		    dont_move_forwards = true;
/*
								// PSP-FIXME: DOESN'T WORK AS EXPECTED
								// (SKIP CHANGING TO GUNS WITH NO AMMO)
		    if (plyrweap->ammo[weaponinfo[num].ammo] == 0)
		    {
		        num = plyrweap->pendingweapon;
		    }
		    else
*/
		        num++;

		    if (num > wp_torpedo)
		        num = wp_fist;

		    if (plyrweap->weaponowned[num])
		    {
		        plyrweap->pendingweapon = num;

		        break;
		    }
	        }
	        kbevent.type = ev_keydown;
	        kbevent.data1 = KEY_1 + num;

	        D_PostEvent(&kbevent);

		dont_move_forwards = false;
	    }
	}
/*			// THE REST OF THE CODE IN THIS FUNCTION IS MOST LIKELY NOT NEEDED FOR PSP
	else  
	{
	    kbevent.type = ev_keyup;
	    kbevent.data1 = KEY_1 + num;

	    D_PostEvent(&kbevent);
	}
*/
//	if (key & SCE_CTRL_DOWN)
//	{
//	    while (1)
//	    {
//		num--;

//		if (num == -1)
//		    num = wp_supershotgun;

//		if (plyrweap->weaponowned[num])
//		{
//		    plyrweap->pendingweapon = num;

//		    break;
//		}
//	    }
//	    kbevent.type = ev_keydown;
//	    kbevent.data1 = KEY_1 + num;

//	    D_PostEvent(&kbevent);
//	}
    }
}

/*
// Warp the mouse back to the middle of the screen

static void CenterMouse(void)
{
    // Warp the the screen center

    SDL_WarpMouse(screen->w / 2, screen->h / 2);

    // Clear any relative movement caused by warping

    SDL_PumpEvents();
#if SDL_VERSION_ATLEAST(1, 3, 0)
    SDL_GetRelativeMouseState(0, NULL, NULL);
#else
    SDL_GetRelativeMouseState(NULL, NULL);
#endif
}

//
// Read the change in mouse state to generate mouse motion events
//
// This is to combine all mouse movement for a tic into one mouse
// motion event.

static void I_ReadMouse(void)
{
    int x, y;
    event_t ev;

#if SDL_VERSION_ATLEAST(1, 3, 0)
    SDL_GetRelativeMouseState(0, &x, &y);
#else
    SDL_GetRelativeMouseState(&x, &y);
#endif

    if (x != 0 || y != 0) 
    {
        ev.type = ev_mouse;
        ev.data1 = mouse_button_state;
        ev.data2 = AccelerateMouse(x);

        if (!novert)
        {
            ev.data3 = -AccelerateMouse(y);
        }
        else
        {
            ev.data3 = 0;
        }
        
        D_PostEvent(&ev);
    }

    if (MouseShouldBeGrabbed())
    {
        CenterMouse();
    }
}
*/
//
// I_StartTic
//
void I_StartTic (void)
{
    if (!initialized)
    {
        return;
    }

    I_GetEvent( ); 
/*
    if (usemouse && !nomouse)
    {
        I_ReadMouse();
    }

    I_UpdateJoystick();
*/
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}
/*
static void UpdateGrab(void)
{
    static boolean currently_grabbed = false;
    boolean grab;

    grab = MouseShouldBeGrabbed();

    if (screensaver_mode)
    {
        // Hide the cursor in screensaver mode

        SetShowCursor(false);
    }
    else if (grab && !currently_grabbed)
    {
        SetShowCursor(false);
        CenterMouse();
    }
    else if (!grab && currently_grabbed)
    {
        SetShowCursor(true);

        // When releasing the mouse from grab, warp the mouse cursor to
        // the bottom-right of the screen. This is a minimally distracting
        // place for it to appear - we may only have released the grab
        // because we're at an end of level intermission screen, for
        // example.

        SDL_WarpMouse(screen->w - 16, screen->h - 16);
        SDL_GetRelativeMouseState(NULL, NULL);
    }

    currently_grabbed = grab;

}

// Update a small portion of the screen
//
// Does stretching and buffer blitting if neccessary
//
// Return true if blit was successful.

static boolean BlitArea(int x1, int y1, int x2, int y2)
{
    int x_offset, y_offset;
    boolean result;

    // No blit needed on native surface

    if (native_surface)
    {
	return true;
    }

    x_offset = (screenbuffer->w - screen_mode->width) / 2;
    y_offset = (screenbuffer->h - screen_mode->height) / 2;

    if (SDL_LockSurface(screenbuffer) >= 0)
    {
        I_InitScale(I_VideoBuffer,
                    (byte *) screenbuffer->pixels
                                + (y_offset * screenbuffer->pitch)
                                + x_offset,
                    screenbuffer->pitch);
        result = screen_mode->DrawScreen(x1, y1, x2, y2);
      	SDL_UnlockSurface(screenbuffer);
    }
    else
    {
        result = false;
    }

    return result;
}

static void UpdateRect(int x1, int y1, int x2, int y2)
{
    int x1_scaled, x2_scaled, y1_scaled, y2_scaled;

    // Do stretching and blitting

    if (BlitArea(x1, y1, x2, y2))
    {
        // Update the area

        x1_scaled = (x1 * screen_mode->width) / SCREENWIDTH;
        y1_scaled = (y1 * screen_mode->height) / SCREENHEIGHT;
        x2_scaled = (x2 * screen_mode->width) / SCREENWIDTH;
        y2_scaled = (y2 * screen_mode->height) / SCREENHEIGHT;

        SDL_UpdateRect(screen,
                       x1_scaled, y1_scaled,
                       x2_scaled - x1_scaled,
                       y2_scaled - y1_scaled);
    }
}
*/
//
// I_FinishUpdate
//
void I_FinishUpdate (int refresh)//(void)
{
    static int	lasttic;
    int		tics;
    int		i;

    if (!initialized)
        return;
/*
    if (noblit)
        return;

    if (need_resize && SDL_GetTicks() > last_resize_time + 500)
    {
        ApplyWindowResize(resize_w, resize_h);
        need_resize = false;
        palette_to_set = true;
    }

    UpdateGrab();

    // Don't update the screen if the window isn't visible.
    // Not doing this breaks under Windows when we alt-tab away 
    // while fullscreen.

    if (!(SDL_GetAppState() & SDL_APPACTIVE))
        return;

    // draws little dots on the bottom of the screen
*/
    if (display_ticker)
    {
	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;

	if (tics > 20) tics = 20;

	for (i=0 ; i<tics*4 ; i+=4)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;

	for ( ; i<20*4 ; i+=4)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
    }
/*
    // draw to screen

    BlitArea(0, 0, SCREENWIDTH, SCREENHEIGHT);

    if (palette_to_set)
    {
        SDL_SetColors(screenbuffer, palette, 0, 256);
        palette_to_set = false;

        // In native 8-bit mode, if we have a palette to set, the act
        // of setting the palette updates the screen

        if (screenbuffer == screen)
        {
            return;
        }
    }

    // In 8in32 mode, we must blit from the fake 8-bit screen buffer
    // to the real screen before doing a screen flip.

    if (screenbuffer != screen)
    {
        SDL_Rect dst_rect;

        // Center the buffer within the full screen space.

        dst_rect.x = (screen->w - screenbuffer->w) / 2;
        dst_rect.y = (screen->h - screenbuffer->h) / 2;

        SDL_BlitSurface(screenbuffer, NULL, screen, &dst_rect);
    }

    SDL_Flip(screen);
*/

	vita2d_start_drawing();
	vita2d_clear_screen();
	vita2d_draw_texture_scale(tex_buffer, 0, 0, 3.0f, 2.72f);
	vita2d_end_drawing();

    // Sync
    vita2d_wait_rendering_done();
	vita2d_swap_buffers();
	
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, /*I_VideoBuffer, */screens[0], SCREENMULT/*SCREENWIDTH*SCREENHEIGHT*/);
}

//
// I_SetPalette
//
uint8_t palette_set = 0;
void I_SetPalette (byte *doompalette)
{
	if (palette_set) return;
    int i;

    unsigned int* colors256 = vita2d_texture_get_palette(tex_buffer);

    for (i=0; i<256; ++i)
    {
        // Zero out the bottom two bits of each channel - the PC VGA
        // controller only supports 6 bits of accuracy.
        /*palette[i].r*/unsigned int r = gammatable[usegamma][*doompalette++] & ~3;
        /*palette[i].g*/unsigned int g = gammatable[usegamma][*doompalette++] & ~3;
        /*palette[i].b*/unsigned int b = gammatable[usegamma][*doompalette++] & ~3;

	*(colors256++) = (0xFF << 24)|(b << 16)|(g << 8)|(r);
    }
	
	palette_set = 1;
//    palette_to_set = true;
}

// Screen clearing function from PSP-Hexen & PSP-Heretic to remove the scratches upon game startup

void I_ClearPart(int color, int x1, int y1, int x2, int y2)
{
    int yy;
    byte *scr0;

    if(!(y1 < y2))
	return;

    if(!(x1 < x2))
	return;

    for(yy = y1; yy < y2; yy++)
    {
	scr0 = screens[0] + yy * SCREENWIDTH + x1;
	memset(scr0, 0, x2-x1);
    }
}

void I_InitGraphics(void)
{
/*					// NOT NECCESSARY FOR PSP
    static int	firsttime=1;
    long video_w, video_h, w, h;
    long video_bpp;
    if (!firsttime)
	return;
    firsttime = 0;
    multiply = 1;
    if (M_CheckParm("-2"))
	multiply = 2;
    if (M_CheckParm("-3"))
	multiply = 3;
    // check if the user wants to grab the mouse (quite unnice)
    grabMouse = !!M_CheckParm("-grabmouse");
    video_w = w = 320;
    video_h = h = 200;
    video_bpp = 8;
    w = SCREENWIDTH * multiply;
    h = SCREENHEIGHT * multiply;
*/
    byte* doompal;

	vita2d_init();
	tex_buffer = vita2d_create_empty_texture_format(SCREENWIDTH, SCREENHEIGHT, SCE_GXM_TEXTURE_BASE_FORMAT_P8);
	screens[0] = vita2d_texture_get_datap(tex_buffer);
	vita2d_start_drawing();
	vita2d_clear_screen();
	vita2d_end_drawing();
	vita2d_wait_rendering_done();
	vita2d_swap_buffers();
    sceDisplayWaitVblankStart();

    // Flush the whole screen in order to make the game loading indicator appear on startup
    I_ClearPart(0, 0, 0, SCREENWIDTH, SCREENHEIGHT);

    doompal = W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE);

    I_SetPalette(doompal);

    initialized = true;
}

void I_ShutdownGraphics(void)
{
    if (initialized)
    {
		vita2d_wait_rendering_done();
		vita2d_free_texture(tex_buffer);
    	vita2d_fini();
/*
        free(screens);			// PSP-GCC v4.8.2 compiler warning
        free(list);			// PSP-GCC v4.8.2 compiler warning

        SetShowCursor(true);

        SDL_QuitSubSystem(SDL_INIT_VIDEO);
*/
        initialized = false;
    }
}
/*
void I_BindVideoVariables(void)
{

    M_BindVariable("autoadjust_video_settings", &autoadjust_video_settings);
    M_BindVariable("fullscreen",                &fullscreen);
    M_BindVariable("aspect_ratio_correct",      &aspect_ratio_correct);
    M_BindVariable("startup_delay",             &startup_delay);
    M_BindVariable("screen_width",              &screen_width);
    M_BindVariable("screen_height",             &screen_height);
    M_BindVariable("screen_bpp",                &screen_bpp);
    M_BindVariable("grabmouse",                 &grabmouse);
    M_BindVariable("mouse_acceleration",        &mouse_acceleration);
    M_BindVariable("mouse_threshold",           &mouse_threshold);
    M_BindVariable("video_driver",              &video_driver);
    M_BindVariable("window_position",           &window_position);
    M_BindVariable("usegamma",                  &usegamma);
    M_BindVariable("vanilla_keyboard_mapping",  &vanilla_keyboard_mapping);
    M_BindVariable("novert",                    &novert);
    M_BindVariable("gl_max_scale",              &gl_max_scale);
    M_BindVariable("png_screenshots",           &png_screenshots);
    // Windows Vista or later?  Set screen color depth to
    // 32 bits per pixel, as 8-bit palettized screen modes
    // don't work properly in recent versions.
*/
/*
#if defined(_WIN32) && !defined(_WIN32_WCE)
    {
        OSVERSIONINFOEX version_info;

        ZeroMemory(&version_info, sizeof(OSVERSIONINFOEX));
        version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        GetVersionEx((OSVERSIONINFO *) &version_info);

        if (version_info.dwPlatformId == VER_PLATFORM_WIN32_NT
         && version_info.dwMajorVersion >= 6)
        {
            screen_bpp = 32;
        }
    }
#endif
*/
    // Disable fullscreen by default on OS X, as there is an SDL bug
    // where some old versions of OS X (<= Snow Leopard) crash.
/*
#ifdef __MACOSX__
    fullscreen = 0;
    screen_width = 800;
    screen_height = 600;
#endif

}
*/

patch_t *disk;		// MOVED HERE FOR PSP

void I_EnableLoadingDisk(void)
{
//    patch_t *disk;	// CHANGED FOR PSP: MOVED TO OUTSIDE OF THIS FUNCTION FOR I_BEGINREAD & I_ENDREAD
    byte *tmpbuf;
    char *disk_name = NULL;
    int y;
    char buf[20];

//    SDL_VideoDriverName(buf, 15);

    if (!strcmp(buf, "Quartz"))
    {
        // MacOS Quartz gives us pageflipped graphics that screw up the 
        // display when we use the loading disk.  Disable it.
        // This is a gross hack.

        return;
    }
/*
    if (M_CheckParm("-cdrom") > 0)
        disk_name = DEH_String("STCDROM");
    else
*/
    disk_name = DEH_String("STDISK");

    disk = W_CacheLumpName(disk_name, PU_STATIC);

    // Draw the patch into a temporary buffer

    tmpbuf = Z_Malloc(SCREENWIDTH * (disk->height + 1), PU_STATIC, NULL);
    V_UseBuffer(tmpbuf);

    // Draw the disk to the screen:

//    V_DrawPatch(0, 0, 0, disk);

    disk_image = Z_Malloc(LOADING_DISK_W * LOADING_DISK_H, PU_STATIC, NULL);
//    saved_background = Z_Malloc(LOADING_DISK_W * LOADING_DISK_H, PU_STATIC, NULL);

    for (y=0; y<LOADING_DISK_H; ++y) 
    {
        memcpy(disk_image + LOADING_DISK_W * y,
               tmpbuf + SCREENWIDTH * y,
               LOADING_DISK_W);
    }

    // All done - free the screen buffer and restore the normal 
    // video buffer.

    W_ReleaseLumpName(disk_name);
    V_RestoreBuffer();
    Z_Free(tmpbuf);
}
/*						// WON'T WORK FOR PSP - MADE A WORKAROUND FOR TICKER
void I_DisplayFPSDots(boolean dots_on)
{
    display_fps_dots = dots_on;
}
*/
// Given an RGB value, find the closest matching palette index.
/*
int I_GetPaletteIndex(int r, int g, int b)
{
    int best, best_diff, diff;
    int i;

    best = 0; best_diff = INT_MAX;

    for (i = 0; i < 256; ++i)
    {
        diff = (r - palette[i].r) * (r - palette[i].r)
             + (g - palette[i].g) * (g - palette[i].g)
             + (b - palette[i].b) * (b - palette[i].b);

        if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }

        if (diff == 0)
        {
            break;
        }
    }

    return best;
}
*/

extern int screenSize;

void I_BeginRead(void)
{
    byte *screenloc = screens[0]
                    + (SCREENHEIGHT - LOADING_DISK_H) * SCREENWIDTH
                    + (SCREENWIDTH - LOADING_DISK_W);
    int y;

    if (!initialized || disk_image == NULL/* || using_opengl*/)
        return;

    // save background and copy the disk image in

    for (y=0; y<LOADING_DISK_H; ++y)
    {
        memcpy(screens[1] + y * LOADING_DISK_W,
               screenloc,
               LOADING_DISK_W);
        memcpy(screenloc,
               disk_image + y * LOADING_DISK_W,
               LOADING_DISK_W);

        screenloc += SCREENWIDTH;
    }
/*						// WON'T WORK FOR THE PSP (EVEN PSP-GCC & PSP-SDL)
    UpdateRect(SCREENWIDTH - LOADING_DISK_W,
	       SCREENHEIGHT - LOADING_DISK_H,
               SCREENWIDTH, SCREENHEIGHT);
*/
    if(!wipe && screenSize > 6) 				// THIS WORKS PRETTY GOOD FOR PSP, BUT...
        V_DrawPatch(SCREENWIDTH - LOADING_DISK_W, 0, 0, disk);	// ...ONLY AVAILABLE IN FULLSCREEN MODE
}

void I_EndRead(void)
{
    byte *screenloc = screens[0]
                    + (SCREENHEIGHT - LOADING_DISK_H) * SCREENWIDTH
                    + (SCREENWIDTH - LOADING_DISK_W);
    int y;

    if (!initialized || disk_image == NULL/* || using_opengl*/)
        return;

    // save background and copy the disk image in

    for (y=0; y<LOADING_DISK_H; ++y)
    {
        memcpy(screenloc,
               screens[1] + y * LOADING_DISK_W,
               LOADING_DISK_W);

        screenloc += SCREENWIDTH;
    }
/*						// WON'T WORK FOR THE PSP (EVEN PSP-GCC & PSP-SDL)
    UpdateRect(SCREENWIDTH - LOADING_DISK_W,
	       SCREENHEIGHT - LOADING_DISK_H,
               SCREENWIDTH, SCREENHEIGHT);
*/
    if(!wipe && screenSize > 6) 				// THIS WORKS PRETTY GOOD FOR PSP, BUT...
        V_DrawPatch(SCREENWIDTH - LOADING_DISK_W, 0, 0, disk);	// ...ONLY AVAILABLE IN FULLSCREEN MODE
}


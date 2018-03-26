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
//      Timer functions.
//

#include "sys/time.h"
#include <SDL.h>

#include <vitasdk.h>

#include "i_timer.h"
#include "doomtype.h"

extern int frame_limit;

//
// I_GetTime
// returns time in 1/35th second tics
//

static Uint32 basetime = 0;

/*						// PSP-GCC v4.8.2: THIS CAUSES STUTTERING EVERY SECOND
int  I_GetTime (void)
{
    Uint32 ticks;

    ticks = SDL_GetTicks();

    if (basetime == 0)
        basetime = ticks;

    ticks -= basetime;

    return (ticks * TICRATE) / 1000;
}
*/

unsigned long GetTicks()			// NEW CODE FOR PSP-GCC v4.8.2 (prevents stutter)
{
    static boolean first = true;
    static long start_sec;
    struct timeval tv;

    sceKernelLibcGettimeofday (&tv, NULL);
    if (first)
    {
	start_sec = tv.tv_sec;
	first = false;
    }
    return ((unsigned long) ((tv.tv_sec - start_sec) * 1000 + tv.tv_usec / 1000));
}

//
// I_GetTime
// returns time in 1/35 second tics
//

int  I_GetTime (void)				// NEW CODE FOR PSP-GCC v4.8.2 (prevents stutter)
{
    return ((GetTicks())*(TICRATE))/1000;
}

//
// Same as I_GetTime, but returns time in milliseconds
//

int I_GetTimeMS(void)
{
    Uint32 ticks;

    ticks = SDL_GetTicks();

    if (basetime == 0)
        basetime = ticks;

    return ticks - basetime;
}

// Sleep for a specified number of ms

void I_Sleep(int ms)
{
    SDL_Delay(ms);
}


void I_WaitVBL(int count)
{
    if(frame_limit != 0)
    	I_Sleep((count * 1000) / (70 * frame_limit));
}


void I_InitTimer(void)
{
    // initialize timer

    SDL_Init(SDL_INIT_TIMER);
}


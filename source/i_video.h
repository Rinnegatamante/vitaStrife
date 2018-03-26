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
// DESCRIPTION:
//	System specific interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef __I_VIDEO__
#define __I_VIDEO__


#include "doomtype.h"
/*
#ifdef __GNUG__
#pragma interface
#endif
*/

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void I_StartTic (void);

void I_InitGraphics (void);

void I_StartFrame (void);

void I_DisplayFPSDots(boolean dots_on);

void I_EnableLoadingDisk(void);

void I_ShutdownGraphics(void);

void I_ClearPart(int color, int x1, int y1, int x2, int y2);

// Takes full 8 bit values.
void I_SetPalette ();

int I_GetPaletteIndex(int r, int g, int b);

void I_UpdateNoBlit (void);
//void I_FinishUpdate (void);
void I_FinishUpdate (int refresh);

// Wait for vertical retrace or pause a bit.
void I_WaitVBL(int count);

void I_ReadScreen (byte* scr);

void I_BeginRead (void);
void I_EndRead (void);

extern boolean initialized;

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------

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
//	Gamma correction LUT.
//	Functions to draw patches (by post) directly to screen.
//	Functions to blit a block to the screen.
//
//-----------------------------------------------------------------------------


#ifndef __V_VIDEO__
#define __V_VIDEO__

#include "doomtype.h"

#include "doomdef.h"

// Needed because we are refering to patches.
#include "r_data.h"

//
// VIDEO
//

#define CENTERY			(SCREENHEIGHT/2)


// Screen 0 is the screen updated by I_Update screen.
// Screen 1 is an extra buffer.

extern byte*		screens[6];
//byte*		screens[1];

extern  int	dirtybox[0];

extern	byte	gammatable[5][256];
extern	int	usegamma;

// haleyjd 08/28/10: implemented for Strife support
// haleyjd 08/28/10: Patch clipping callback, implemented to support Choco
// Strife.
typedef boolean (*vpatchclipfunc_t)(patch_t *, int, int);
void V_SetPatchClipCallback(vpatchclipfunc_t func);

// Allocates buffer screens, call before R_Init.
void V_Init (void);

/*
void
V_CopyRect
( int		srcx,
  int		srcy,
  int		srcscrn,
  int		width,
  int		height,
  int		destx,
  int		desty,
  int		destscrn );
*/

void
V_DrawPatch
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch);

/*
void
V_DrawPatchDirect
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );
*/

// Draw a linear block of pixels into the view buffer.
void
V_DrawBlock
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		src );

/*
// Reads a linear block of pixels into the view buffer.
void
V_GetBlock
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		dest );
*/

void
V_MarkRect
( int		x,
  int		y,
  int		width,
  int		height );

// Return to using the normal screen buffer to draw graphics.

void V_RestoreBuffer(void);

//void V_DrawMouseSpeedBox(int speed);

void V_DrawFilledBox(int x, int y, int w, int h, int c);

// villsa [STRIFE]
// Load the lookup table for translucency calculations from the XLATAB
// lump.

void V_LoadXlaTable(void);

// Temporarily switch to using a different buffer to draw graphics, etc.

void V_UseBuffer(byte* buffer);

void V_DrawXlaPatch(int x, int y, patch_t * patch);     // villsa [STRIFE]

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------

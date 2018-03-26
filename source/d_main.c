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
//	DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
//	plus functions to determine game mode (shareware, registered),
//	parse command line parameters, configure game parameters (turbo),
//	and call the startup functions.
//

#include <vitasdk.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "config.h"
#include "deh_str.h"
#include "doomdef.h"
#include "doomstat.h"

#include "dstrings.h"
#include "doomfeatures.h"
#include "sounds.h"

#include "d_iwad.h"

#include "z_zone.h"
#include "w_main.h"
#include "w_wad.h"
#include "s_sound.h"
#include "v_video.h"

#include "f_finale.h"
#include "f_wipe.h"

//#include "m_argv.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_saves.h" // haleyjd [STRIFE]
#include "p_saveg.h"
#include "p_dialog.h" // haleyjd [STRIFE]

#include "i_endoom.h"
//#include "i_joystick.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "i_swap.h"

#include "g_game.h"

#include "hu_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
//#include "net_client.h"
//#include "net_dedicated.h"
//#include "net_query.h"

#include "p_setup.h"
#include "r_local.h"

#include "d_main.h"

//#include "w_merge.h"

#define printf psvDebugScreenPrintf

//
// D-DoomLoop()
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//
void D_DoomLoop (void);

static boolean D_AddFile(char *filename);

// Location where savegames are stored

char *          savegamedir;

// location of IWAD and WAD files

char *          iwadfile;


boolean         devparm = false;		// CHANGED FOR PSP: ENABLE / DISABLE DEVELOPMENT MODE
//boolean	devparm_print_dir = false;	// not available
boolean		devparm_opl_music = false;
boolean		devparm_opl_write = false;
boolean		devparm_opl_read = false;
boolean		devparm_wipe = false;
//boolean	devparm_sdl_sound = false;	// not available

boolean		autostart = false;		// CHANGED FOR PSP: MOVED UP HERE FOR DEVPARM MODE
/*
boolean         nomonsters;     		// checkparm of -nomonsters
boolean         respawnparm;    		// checkparm of -respawn
boolean         fastparm;       		// checkparm of -fast
*/
boolean         flipparm;       		// [STRIFE] haleyjd 20110629: checkparm of -flip
//boolean	randomparm;     		// [STRIFE] haleyjd 20130915: checkparm of -random

boolean         showintro = true;   		// [STRIFE] checkparm of -nograph, disables intro

/*
extern int	soundVolume;
extern int	sfxVolume;
extern int	musicVolume;
*/

#ifdef SHAREWARE
boolean		STRIFE_1_0_SHAREWARE;
boolean		STRIFE_1_1_SHAREWARE;
#endif

boolean		STRIFE_1_0_REGISTERED;
boolean		STRIFE_1_X_REGISTERED;

boolean		VOICES_1_X_REGISTERED;

extern boolean	inhelpscreens;

skill_t		startskill;
int             startepisode;
int		startmap;
int             startloadgame;

#ifdef SHAREWARE
int		filesize_strife0_wad = 0;
int		strife0_wad_exists = 0;
#endif

int		filesize_strife1_wad = 0;
int		strife1_wad_exists = 0;

int		filesize_voices_wad = 0;
int		voices_wad_exists = 0;

boolean		advancedemo;

// villsa [STRIFE] workparm variable (similar to devparm?)
boolean         workparm = false;

// villsa [STRIFE] stonecold cheat variable
boolean         stonecold = false;

// haleyjd 09/11/10: [STRIFE] Game type variables
boolean         isregistered;
boolean         isdemoversion;

int		dots_enabled = 0;

int		fps_enabled = 0;
int		display_fps = 0;

// Store demo, do not accept any inputs
// haleyjd [STRIFE] Unused.
//boolean       storedemo;


char		wadfile[1024];          // primary wad file
char		mapdir[1024];           // directory of development maps

int             show_endoom = 1;
int             graphical_startup = 1;

int		iwad_message_has_been_shown = 0;
int		warning_message_has_been_shown = 0;

int		wad_missing = 0;

// If true, startup has completed and the main game loop has started.

static boolean	main_loop_started = false;

static SceCtrlData pad;
static SceCtrlData lastpad;

// fraggle 06/03/11 [STRIFE]: Unused config variable, preserved
// for compatibility:

//static int	comport = 0;

// fraggle 06/03/11 [STRIFE]: Multiplayer nickname?
//char		*nickname = NULL;

//void D_ConnectNetGame(void);
void D_CheckNetGame(void);


//
// D_ProcessEvents
// Send all the events of the given timestamp down the responder chain
//
void D_ProcessEvents (void)
{
    event_t*    ev;

    // haleyjd 08/22/2010: [STRIFE] there is no such thing as a "store demo" 
    // version of Strife

    // IF STORE DEMO, DO NOT ACCEPT INPUT
    //if (storedemo)
    //    return;

    while ((ev = D_PopEvent()) != NULL)
    {
        if (M_Responder (ev) || AM_Responder (ev))		// FOR PSP: MOVED HERE BECAUSE OF...
            continue;		// menu or map ate the event	// ...AUTOMAP ZOOM LOOPING EVEN IF...
        G_Responder (ev);					// ...THE KEY HAS ALMOST BEEN RELEASED
    }
}




//
// D_Display
//  draw current display, possibly wiping it from the previous
//
// wipegamestate can be set to -1 to force a wipe on the next draw
//
// haleyjd 08/23/10: [STRIFE]:
// * Changes to eliminate intermission and change timing of screenwipe
// * 20100901: Added ST_DrawExternal and popupactivestate static variable
// * 20110206: Start wipegamestate at GS_UNKNOWN (STRIFE-TODO: rename?)
//
gamestate_t     wipegamestate = GS_UNKNOWN;
extern boolean	setsizeneeded;
extern boolean	wipe;

extern int	showMessages;	// [STRIFE] USUALLY THERE'S NO such variable

void R_ExecuteSetViewSize (void);

void D_Display (void)
{
    static  boolean             viewactivestate = false;
    static  boolean             menuactivestate = false;
    static  boolean             inhelpscreensstate = false;
    static  boolean             popupactivestate = false; // [STRIFE]
    static  boolean             fullscreen = false;
    static  gamestate_t         oldgamestate = -1;
    static  int                 borderdrawcount;
    int                         nowtime;
    int                         tics;
    int                         wipestart;
    int                         y;
    boolean                     done;
//    boolean                     wipe;
    boolean                     redrawsbar;

    if (nodrawers)
        return;                    // for comparative timing / profiling

    redrawsbar = false;
    
    // change the view size if needed
    if (setsizeneeded)
    {
        R_ExecuteSetViewSize ();
        oldgamestate = -1;                      // force background redraw
        borderdrawcount = 3;
    }

    // save the current screen if about to wipe
    if (gamestate != wipegamestate)
    {
	if(dots_enabled == 1)			// ADDED FOR PSP TO PREVENT CRASH...
    	    display_ticker = false;		// ...UPON WIPING SCREEN WITH ENABLED DISPLAY TICKER
	if(fps_enabled == 1)			// ADDED FOR PSP TO PREVENT CRASH...
    	    display_fps = 0;			// ...UPON WIPING SCREEN WITH ENABLED DISPLAY TICKER

        wipe = true;
        wipe_StartScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);
    }
    else
    {
	if(dots_enabled == 1)			// ADDED FOR PSP TO PREVENT CRASH...
    	    display_ticker = true;		// ...UPON WIPING SCREEN WITH ENABLED DISPLAY TICKER
	if(fps_enabled == 1)			// ADDED FOR PSP TO PREVENT CRASH...
    	    display_fps = 1;			// ...UPON WIPING SCREEN WITH ENABLED DISPLAY TICKER

        wipe = false;
    }

    if (gamestate == GS_LEVEL && gametic)
        HU_Erase();

    // do buffered drawing
    switch (gamestate)
    {
    case GS_LEVEL:
        if (!gametic)
            break;
        if (automapactive)
            AM_Drawer ();
        if (wipe || (viewheight != 200 && fullscreen) )
            redrawsbar = true;
        // haleyjd 08/29/10: [STRIFE] Always redraw sbar if menu is/was active
        if (menuactivestate || (inhelpscreensstate && !inhelpscreens))
            redrawsbar = true;              // just put away the help screen
        ST_Drawer (viewheight == 200, redrawsbar );
        fullscreen = viewheight == 200;
        break;
      
     // haleyjd 08/23/2010: [STRIFE] No intermission
     /*
     case GS_INTERMISSION:
         WI_Drawer ();
         break;
     */

    case GS_FINALE:
        F_Drawer ();
        break;

    case GS_DEMOSCREEN:
        D_PageDrawer ();
        break;
    
    default:
        break;
    }
    
    // draw buffered stuff to screen
    I_UpdateNoBlit ();

    // draw the view directly
    if (gamestate == GS_LEVEL && !automapactive && gametic)
        R_RenderPlayerView (&players[displayplayer]);

    // clean up border stuff
    if (gamestate != oldgamestate && gamestate != GS_LEVEL)
        I_SetPalette (W_CacheLumpName (("PLAYPAL"),PU_CACHE));

    // see if the border needs to be initially drawn
    if (gamestate == GS_LEVEL && oldgamestate != GS_LEVEL)
    {
        viewactivestate = false;        // view was not active
        R_FillBackScreen ();    // draw the pattern into the back screen
    }
    // see if the border needs to be updated to the screen
    if (gamestate == GS_LEVEL && !automapactive && scaledviewwidth != 320)
    {
        if (menuactive || menuactivestate || !viewactivestate)
        {
            borderdrawcount = 3;
            popupactivestate = false;
        }
        if (borderdrawcount)
        {
            R_DrawViewBorder ();    // erase old menu stuff
            borderdrawcount--;
        }

    }
/*
    if (testcontrols)
    {
        // Box showing current mouse speed

        V_DrawMouseSpeedBox(testcontrols_mousespeed);
    }
*/
    menuactivestate = menuactive;
    viewactivestate = viewactive;
    inhelpscreensstate = inhelpscreens;
    oldgamestate = wipegamestate = gamestate;

    // haleyjd 20120208: [STRIFE] Rogue moved this down to below border drawing
    if (gamestate == GS_LEVEL && gametic)
    {
        HU_Drawer ();
        if(ST_DrawExternal()) 
            popupactivestate = true;
        else if(popupactivestate)
        {
            popupactivestate = false;
            menuactivestate = 1;
        }
    }

    // draw pause pic
    if (paused)
    {
        if (automapactive)
            y = 4;
        else
            y = viewwindowy+4;
        V_DrawPatch(viewwindowx + (scaledviewwidth - 68) / 2, y, 0,
                          W_CacheLumpName (("M_PAUSE"), PU_CACHE));
    }


    // menus go directly to the screen
    M_Drawer ();          // menu is drawn even on top of everything
    NetUpdate ();         // send out any new accumulation


    // normal update
    if (!wipe)
    {
//	I_FinishUpdate ();              // page flip or blit buffer
	I_FinishUpdate(1);
        return;
    }
    
    // wipe update
    wipe_EndScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);

    wipestart = I_GetTime () - 1;

    do
    {
        do
        {
            nowtime = I_GetTime ();
            tics = nowtime - wipestart;
            I_Sleep(1);
        } while (tics < 3); // haleyjd 08/23/2010: [STRIFE] Changed from == 0 to < 3

        // haleyjd 08/26/10: [STRIFE] Changed to use ColorXForm wipe.
        wipestart = nowtime;
        done = wipe_ScreenWipe(wipe_ColorXForm
                               , 0, 0, SCREENWIDTH, SCREENHEIGHT, tics);
        I_UpdateNoBlit ();
        M_Drawer ();                            // menu is drawn even on top of wipes
//        I_FinishUpdate ();                      // page flip or blit buffer
	I_FinishUpdate(1);
    } while (!done);
}

//
// Add configuration file variable bindings.
//

void D_BindVariables(void)
{
//    int i;

//    M_ApplyPlatformDefaults();

//    I_BindVideoVariables();
//    I_BindJoystickVariables();
    I_BindSoundVariables();

    M_BindBaseControls();
//    M_BindWeaponControls();
//    M_BindMapControls();
//    M_BindMenuControls();
//    M_BindStrifeControls(); // haleyjd 09/01/10: [STRIFE]
//    M_BindChatControls(MAXPLAYERS);

    // haleyjd 20130915: Strife chat keys
/*
    key_multi_msgplayer[0] = '1';
    key_multi_msgplayer[1] = '2';
    key_multi_msgplayer[2] = '3';
    key_multi_msgplayer[3] = '4';
    key_multi_msgplayer[4] = '5';
    key_multi_msgplayer[5] = '6';
    key_multi_msgplayer[6] = '7';
    key_multi_msgplayer[7] = '8';
*/
/*
#ifdef FEATURE_MULTIPLAYER
    NET_BindVariables();
#endif
*/

    // haleyjd 08/29/10: [STRIFE]
    // * Added voice volume
    // * Added back flat
    // * Removed show_messages
    // * Added show_talk
    // fraggle 03/06/10: [STRIFE]
    // * Removed detailLevel
    // * screenblocks -> screensize
    // * Added nickname, comport
/*
    M_BindVariable("sfx_volume",             &sfxVolume);
    M_BindVariable("music_volume",           &musicVolume);
    M_BindVariable("show_talk",              &dialogshowtext);
    M_BindVariable("voice_volume",           &voiceVolume); 
    M_BindVariable("screensize",             &screenblocks);

    M_BindVariable("snd_channels",           &snd_channels);
    M_BindVariable("vanilla_savegame_limit", &vanilla_savegame_limit);
    M_BindVariable("vanilla_demo_limit",     &vanilla_demo_limit);
    M_BindVariable("show_endoom",            &show_endoom);
    M_BindVariable("back_flat",              &back_flat);
    M_BindVariable("graphical_startup",      &graphical_startup);

    M_BindVariable("nickname",               &nickname);
    M_BindVariable("comport",                &comport);

    // Multiplayer chat macros

    for (i=0; i<10; ++i)
    {
        char buf[12];

        M_snprintf(buf, sizeof(buf), "chatmacro%i", i);
        M_BindVariable(buf, &chat_macros[i]);
    }
*/
}

//
// D_GrabMouseCallback
//
// Called to determine whether to grab the mouse pointer
//
/*
boolean D_GrabMouseCallback(void)
{
    // Drone players don't need mouse focus

    if (drone)
        return false;

    // when menu is active or game is paused, release the mouse.

    if (menuactive || paused)
        return false;

    // only grab mouse when playing levels (but not demos)

    return (gamestate == GS_LEVEL) && !demoplayback;
}

// During startup, never grab the mouse.

static boolean D_StartupGrabCallback(void)
{
    return false;
}
*/

//
//  D_DoomLoop
//
//  haleyjd 08/23/10: [STRIFE] Verified unmodified.
//
void D_DoomLoop (void)
{
    if (demorecording)
        G_BeginRecording ();

    main_loop_started = true;

    TryRunTics();

    if (!showintro)
    {
//        I_SetWindowTitle(gamedescription);
        I_InitGraphics();
    }

    I_EnableLoadingDisk();
//    I_SetGrabMouseCallback(D_GrabMouseCallback);

    V_RestoreBuffer();
    R_ExecuteSetViewSize();

    D_StartGameLoop();
/*
    if (testcontrols)
    {
        wipegamestate = gamestate;
    }
*/

    while (1)
    {
/*
//	printf("Free RAM: %d Bytes\n",sceKernelTotalFreeMemSize());	// FOR PSP (RAM DEBUG INFO)
//	sceKernelDelayThreadCB(100000);

//	int mainram = 0;
//	printf("main  = %d\n", (int) mainram);
//	printf("Stack = %d\n", (int) &pad);

//	int maxFreeMemSize = sceKernelMaxFreeMemSize();
//	printf("Maximum Free RAM   = %d\n", maxFreeMemSize);
//	int totalFreeMemSize = sceKernelTotalFreeMemSize();
//	printf("sceKernelTotalFreeMemSize = %d\n", totalFreeMemSize);
*/
        // frame syncronous IO operations
        I_StartFrame ();

        // process one or more tics
        TryRunTics (); // will run at least one tic

        S_UpdateSounds (players[consoleplayer].mo);// move positional sounds

        // Update display, next frame, with current state.
//        if (screenvisible)
            D_Display ();
    }
}



//
//  DEMO LOOP
//
int             demosequence;
int             pagetic;
char                    *pagename;


//
// D_PageTicker
// Handles timing for warped projection
//
// haleyjd 08/22/2010: [STRIFE] verified unmodified
//
void D_PageTicker (void)
{
    if (--pagetic < 0)
        D_AdvanceDemo ();
}



//
// D_PageDrawer
//
// haleyjd 08/22/2010: [STRIFE] verified unmodified
//
void D_PageDrawer (void)
{
    V_DrawPatch (0,0, 0, W_CacheLumpName(pagename, PU_CACHE));
}


//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
// haleyjd 08/22/2010: [STRIFE] verified unmodified
//
void D_AdvanceDemo (void)
{
    advancedemo = true;
}


//
// This cycles through the demo sequences.
// FIXME - version dependend demo numbers?
//
// [STRIFE] Modified for the opening slideshow and the exit screen
//
void D_DoAdvanceDemo (void)
{
    players[consoleplayer].playerstate = PST_LIVE;  // not reborn
    advancedemo = false;
    usergame = false;               // no save / end game here
    paused = false;
    gameaction = ga_nothing;
    
    // villsa 09/12/10: [STRIFE] converted pagetics to ticrate
    switch (demosequence)
    {
    case -5: // exit the game
        I_Quit();
        return;
    case -4: // show exit screen
        menuactive = false;
        pagetic = 3*TICRATE;
        gamestate = GS_DEMOSCREEN;
        pagename = DEH_String("PANEL7");
//        S_StartMusic(mus_fast);	// moved to M_QuitResponse function cause for PSP
					// seems not to work right here in this place.
        if(isdemoversion)
            demosequence = -3; // show Velocity logo
        else
            demosequence = -5; // exit
        return;
    case -3: // show Velocity logo for demo version
        pagetic = 6*TICRATE;
        gamestate = GS_DEMOSCREEN;
        pagename = DEH_String("vellogo");
        demosequence = -5; // exit
        return;
    case -2: // title screen
        pagetic = 6*TICRATE;
        gamestate = GS_DEMOSCREEN;
        pagename = DEH_String("TITLEPIC");
        S_StartMusic(mus_logo);
        demosequence = -1; // start intro cinematic
        return;
    case -1: // start of intro cinematic
        pagetic = 10;
        gamestate = GS_DEMOSCREEN;

	if(STRIFE_1_0_REGISTERED || STRIFE_1_X_REGISTERED) // FOR PSP: (NOT AVAIL. IN SHAREWARE v1.0)
    	    pagename = DEH_String("PANEL0");
#ifdef SHAREWARE
	else if(STRIFE_1_1_SHAREWARE)
    	    pagename = DEH_String("HELP0");
#endif
        S_StartSound(NULL, sfx_rb2act);
        wipegamestate = -1;
        break;
    case 0: // Rogue logo
        pagetic = 4*TICRATE;
        gamestate = GS_DEMOSCREEN;
        pagename = DEH_String("RGELOGO");
        wipegamestate = -1;
        break;
    case 1:
        pagetic = 7*TICRATE;              // The comet struck our planet without
        gamestate = GS_DEMOSCREEN;        // warning.We lost our paradise in a 
        pagename = DEH_String("PANEL1");  // single, violent stroke.
        I_StartVoice(DEH_String("pro1")); 
        S_StartMusic(mus_intro);
        break;
    case 2:
        pagetic = 9*TICRATE;              // The impact released a virus which 
        gamestate = GS_DEMOSCREEN;        // swept through the land and killed 
        pagename = DEH_String("PANEL2");  // millions. They turned out to be 
        I_StartVoice(DEH_String("pro2")); // the lucky ones...
        break;
    case 3:
        pagetic = 12*TICRATE;             // For those that did not die became 
        gamestate = GS_DEMOSCREEN;        // mutations of humanity. Some became
        pagename = DEH_String("PANEL3");  // fanatics who heard the voice of a
        I_StartVoice(DEH_String("pro3")); // malignant God in their heads, and 
        break;                            // called themselves the Order.
    case 4:
        pagetic = 11*TICRATE;             // Those of us who were deaf to this
        pagename = DEH_String("PANEL4");  // voice suffer horribly and are 
        gamestate = GS_DEMOSCREEN;        // forced to serve these ruthless
        I_StartVoice(DEH_String("pro4")); // psychotics, who wield weapons more
        break;                            // powerful than anything we can muster.
    case 5:
        pagetic = 10*TICRATE;             // They destroy our women and children,
        gamestate = GS_DEMOSCREEN;        // so that we must hide them underground,
        pagename = DEH_String("PANEL5");  // and live like animals in constant
        I_StartVoice(DEH_String("pro5")); // fear for our lives.
        break;
    case 6:                               // But there are whispers of discontent.
        pagetic = 16*TICRATE;             // If we organize, can we defeat our
        gamestate = GS_DEMOSCREEN;        // masters? Weapons are being stolen,
        pagename = DEH_String("PANEL6");  // soldiers are being trained. A 
        I_StartVoice(DEH_String("pro6")); // Movement is born! Born of lifelong 
        break;                            // STRIFE!
    case 7: // titlepic again - unused...
        pagetic = 9*TICRATE;
        gamestate = GS_DEMOSCREEN;
        pagename = DEH_String("TITLEPIC");
        wipegamestate = -1;
        break;
    case 8: // demo
        ClearTmp();
        pagetic = 9*TICRATE;
        G_DeferedPlayDemo(("demo1"));
        break;
    case 9: // velocity logo? - unused...
        pagetic = 6*TICRATE;
        gamestate = GS_DEMOSCREEN;
        pagename = DEH_String("vellogo");
        wipegamestate = -1;
        break;
    case 10: // credits
        gamestate = GS_DEMOSCREEN;
        pagetic = 12*TICRATE;
        pagename = DEH_String("CREDIT");
        wipegamestate = -1;
        break;
    default:
        break;
    }

    ++demosequence;

    if(demosequence > 11)
        demosequence = -2;
    if(demosequence == 7 || demosequence == 9)
        ++demosequence;
}



//
// D_StartTitle
//
// [STRIFE]
// haleyjd 09/11/10: Small modifications for new demo sequence.
//
void D_StartTitle (void)
{
    gamestate = GS_DEMOSCREEN;
    gameaction = ga_nothing;
    demosequence = -2;
    D_AdvanceDemo ();
}

//
// D_QuitGame
//
// [STRIFE] New function
// haleyjd 09/11/10: Sets up the quit game snippet powered by the
// demo sequence.
//
void D_QuitGame(void)
{
    gameaction = ga_nothing;
    demosequence = -4;
    D_AdvanceDemo();
}

// Strings for dehacked replacements of the startup banner
//
// These are from the original source: some of them are perhaps
// not used in any dehacked patches

static char *banners[] =
{
    // strife1.wad:

    "                      "
    "STRIFE:  Quest for the Sigil v1.2"
    "                                 "
};

//
// Get game name: if the startup banner has been replaced, use that.
// Otherwise, use the name given
// 

static char *GetGameName(char *gamename)
{
    size_t i;
    char *deh_sub;
    
    for (i=0; i<arrlen(banners); ++i)
    {
        // Has the banner been replaced?

        deh_sub = (banners[i]);
        
        if (deh_sub != banners[i])
        {
            size_t gamename_size;

            // Has been replaced
            // We need to expand via printf to include the Doom version 
            // number
            // We also need to cut off spaces to get the basic name

            gamename_size = strlen(deh_sub) + 10;
            gamename = Z_Malloc(gamename_size, PU_STATIC, 0);
            M_snprintf(gamename, gamename_size, deh_sub,
                       STRIFE_VERSION / 100, STRIFE_VERSION % 100);

            while (gamename[0] != '\0' && isspace(gamename[0]))
            {
                memmove(gamename, gamename + 1, gamename_size - 1);
            }

            while (gamename[0] != '\0' && isspace(gamename[strlen(gamename)-1]))
                gamename[strlen(gamename) - 1] = '\0';

            return gamename;
        }
    }

    return gamename;
}

//
// Find out what version of Doom is playing.
//
/*
void D_IdentifyVersion(void)
{
    // gamemission is set up by the D_FindIWAD function.  But if 
    // we specify '-iwad', we have to identify using 
    // IdentifyIWADByName.  However, if the iwad does not match
    // any known IWAD name, we may have a dilemma.  Try to 
    // identify by its contents.
    
    // STRIFE-TODO: some elaborate checks? for now we assume...
    // The logic in strife1.exe is simple:
    // * if strife1.wad is found, set isregistered = true
    // * if strife0.wad is found, set isdemoversion = true

    // Make sure gamemode is set up correctly
    gamemode = commercial;
    gamemission = strife;
    isregistered = true;

    // Load voices.wad 
    if(isregistered)
    {
        char *name = D_FindWADByName("voices.wad");

        if(!name) // not found?
        {

            int p;

            // haleyjd STRIFE-FIXME: Temporary?
            // If -iwad was used, check and see if voices.wad exists on the
            // same filepath.
            if((p = M_CheckParm("-iwad")) && p < myargc - 1)
            {
                char   *iwad     = myargv[p + 1];
                size_t  len      = strlen(iwad) + 24;
                char   *filename = malloc(len);
                char    sepchar;

                // how the heck is Choco surviving without this routine?
                sepchar = M_GetFilePath(iwad, filename, len);
                filename[strlen(filename)] = sepchar;
                M_StringConcat(filename, "voices.wad", sizeof(filename));

                if(!M_FileExists(filename))
                    disable_voices = 1;
                else
                    name = filename; // STRIFE-FIXME: memory leak!!
            }
            else

                disable_voices = 1;
        }

        if(disable_voices) // voices disabled?
        {
            if(devparm)
                 printf("Voices disabled\n");
            return;
        }

	STRIFE_1_0_SHAREWARE = true;		// FOR PSP: NO VOICES.WAD FOR SHAREWARE
	STRIFE_1_1_SHAREWARE = true;		// FOR PSP: NO VOICES.WAD FOR SHAREWARE

	if(STRIFE_1_0_REGISTERED || STRIFE_1_X_REGISTERED)
	    if(!(STRIFE_1_0_SHAREWARE || STRIFE_1_1_SHAREWARE))
		D_AddFile(name);
    }
}
*/
#if 0
//
// DoTimeBomb
//
// haleyjd 08/23/2010: [STRIFE] New function
// Code with no xrefs; probably left over from a private alpha or beta.
// Translated here because it explains what the SERIAL lump was meant to do.
//
void DoTimeBomb(void)
{
    dosdate_t date;
    char *serial;
    int serialnum;
    int serial_year;
    int serial_month;

    serial = W_CacheLumpName("serial", PU_CACHE);
    serialnum = atoi(serial);

    // Rogue, much like Governor Mourel, were lousy liars. These deceptive
    // error messages are pretty low :P
    dos_getdate(&date);
    if(date.year > 1996 || date.day > 15 && date.month > 4)
        I_Error("Data error! Corrupted WAD File!");
    serial_year = serialnum / 10000;
    serial_month = serialnum / 100 - 100 * serial_year;
    if(date.year < serial_year || 
       date.day < serialnum - 100 * serial_month - 10000 * serial_year &&
       date.month < serial_month)
       I_Error("Bad wadfile");
}
#endif

// Set the gamedescription string

void D_SetGameDescription(void)
{
    gamedescription = GetGameName("Strife: Quest for the Sigil");
}

//      print title for every printed line
char            title[128];
#define MAXWADFILES 20
char*		wadfiles[MAXWADFILES];

static boolean D_AddFile(char *filename)
{

    wad_file_t *handle;

    if(devparm)
    	printf("         adding %s\n", filename);

    handle = W_AddFile(filename);

    return handle != NULL;
}
/*
// Copyright message banners
// Some dehacked mods replace these.  These are only displayed if they are 
// replaced by dehacked.
// haleyjd 08/22/2010: [STRIFE] altered to match strings from binary
static char *copyright_banners[] =
{
    "===========================================================================\n"
    "ATTENTION:  This version of STRIFE has extra files added to it.\n"
    "        You will not receive technical support for modified games.\n"
    "===========================================================================\n",

    "===========================================================================\n"
    "             This version is NOT SHAREWARE, do not distribute!\n"
    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"
    "===========================================================================\n",

    "===========================================================================\n"
    "                                Shareware!\n"
    "===========================================================================\n"
};
*/
// Prints a message only if it has been modified by dehacked.
/*
void PrintDehackedBanners(void)
{
    size_t i;

    for (i=0; i<arrlen(copyright_banners); ++i)
    {
        char *deh_s;

        deh_s = (copyright_banners[i]);

        if (deh_s != copyright_banners[i])
        {
            printf("%s", deh_s);

            // Make sure the modified banner always ends in a newline character.
            // If it doesn't, add a newline.  This fixes av.wad.

            if (deh_s[strlen(deh_s) - 1] != '\n')
            {
                printf("\n");
            }
        }
    }
}
*/
static struct 
{
    char *description;
    char *cmdline;
    GameVersion_t version;
} gameversions[] = {
    { "Strife 1.2",          "1.2",       exe_strife_1_2  },
    { "Strife 1.31",         "1.31",      exe_strife_1_31 },
    { NULL,                  NULL,        0               }
};

// Initialize the game version

static void InitGameVersion(void)
{
/*
    int p;
    int i;

    // haleyjd: we support emulating either the 1.2 or the 1.31 versions of 
    // Strife, which are the most significant. 1.2 is the most mature version
    // that still has the single saveslot restriction, whereas 1.31 is the
    // final revision. The differences between the two are barely worth
    // mentioning aside from that main one.

    //!
    // @arg <version>
    // @category compat
    //
    // Emulate a specific version of Strife. Valid values are "1.2" and "1.31".
    //

    p = M_CheckParmWithArgs("-gameversion", 1);

    if (p)
    {
        for (i=0; gameversions[i].description != NULL; ++i)
        {
            if (!strcmp(myargv[p+1], gameversions[i].cmdline))
            {
                gameversion = gameversions[i].version;
                break;
            }
        }

        if (gameversions[i].description == NULL) 
        {
            printf("Supported game versions:\n");

            for (i=0; gameversions[i].description != NULL; ++i)
            {
                printf("\t%s (%s)\n", gameversions[i].cmdline,
                        gameversions[i].description);
            }

            I_Error("Unknown game version '%s'", myargv[p+1]);
        }
    }
    else
*/
    {
        gameversion = exe_strife_1_31;
    }
}

void PrintGameVersion(void)
{
    int i;

    for (i=0; gameversions[i].description != NULL; ++i)
    {
        if (gameversions[i].version == gameversion)
        {
	    if(devparm)
    	        printf(" Emulating the behavior of the "
            	       "'%s' executable.\n", gameversions[i].description);
            break;
        }
    }
}

// Function called at exit to display the ENDOOM screen

static void D_Endoom(void)
{
    byte *endoom;

    // Don't show ENDOOM if we have it disabled, or we're running
    // in screensaver or control test mode. Only show it once the
    // game has actually started.


    if (!show_endoom || !main_loop_started)// || screensaver_mode || testcontrols)
    {
        return;
    }

    // haleyjd 08/27/10: [STRIFE] ENDOOM -> ENDSTRF
    endoom = W_CacheLumpName(("ENDSTRF"), PU_STATIC);

    I_Endoom(endoom);
}

//=============================================================================
//
// haleyjd: Chocolate Strife Specifics
//
// None of the code in here is from the original executable, but is needed for
// other reasons.

//
// D_PatchClipCallback
//
// haleyjd 08/28/10: Clip patches to the framebuffer without errors.
// Returns false if V_DrawPatch should return without drawing.
//
boolean D_PatchClipCallback(patch_t *patch, int x, int y)
{
    // note that offsets were already accounted for in V_DrawPatch
    return (x >= 0 && y >= 0 /*&& screens[0]*/
            && x + SHORT(patch->width) <= SCREENWIDTH 
            && y + SHORT(patch->height) <= SCREENHEIGHT);
}

//
// D_InitChocoStrife
//
// haleyjd 08/28/10: Take care of some Strife-specific initialization
// that is necessitated by Chocolate Doom issues, such as setting global 
// callbacks.
//
static void D_InitChocoStrife(void)
{
    // set the V_DrawPatch clipping callback
    V_SetPatchClipCallback(D_PatchClipCallback);
}


//
// STRIFE Graphical Intro Sequence
//

#define MAXINTROPROGRESS 69

static int introprogress;        // track the progress of the intro

static byte *rawgfx_startup0;    // raw linear gfx for intro
static byte *rawgfx_startp[4];
static byte *rawgfx_startlz[2];
static byte *rawgfx_startbot;

//
// D_IntroBackground
//
// [STRIFE] New function
// haleyjd 20110206: Strife only drew this once, but for supporting double-
// buffered or page-flipped surfaces it is best to redraw the entire screen
// every frame.
//
static void D_IntroBackground(void)
{
    if(!showintro)
        return;

    // Fill the background entirely (wasn't needed in vanilla)
    V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);

    // Strife cleared the screen somewhere in the low-level code between the
    // intro and the titlescreen, so this is to take care of that and get
    // proper fade-in behavior on the titlescreen
    if(introprogress >= MAXINTROPROGRESS)
    {
//        I_FinishUpdate();
	I_FinishUpdate(1);
        return;
    }

    // Draw a 95-pixel rect from STARTUP0 starting at y=57 to (0,41) on the
    // screen (this was a memcpy directly to 0xA3340 in low DOS memory)
    V_DrawBlock(0, 41, 0, 320, 95, rawgfx_startup0 + (320*57));
}

//
// D_InitIntroSequence
//
// [STRIFE] New function
// haleyjd 20110206: Initialize the graphical introduction sequence
//

static void D_InitIntroSequence(void)
{
    if(showintro)
    {
        // In vanilla Strife, Mode 13h was initialized directly in D_DoomMain.
        // We have to be a little more courteous of the low-level code here.
//        I_SetWindowTitle(gamedescription);
//        I_SetGrabMouseCallback(D_StartupGrabCallback);
        I_InitGraphics();

        V_RestoreBuffer(); // make the V_ routines work

        // Load all graphics
        rawgfx_startup0   = W_CacheLumpName("STARTUP0", PU_STATIC);
        rawgfx_startp[0]  = W_CacheLumpName("STRTPA1",  PU_STATIC);
        rawgfx_startp[1]  = W_CacheLumpName("STRTPB1",  PU_STATIC);
        rawgfx_startp[2]  = W_CacheLumpName("STRTPC1",  PU_STATIC);
        rawgfx_startp[3]  = W_CacheLumpName("STRTPD1",  PU_STATIC);
        rawgfx_startlz[0] = W_CacheLumpName("STRTLZ1",  PU_STATIC);
        rawgfx_startlz[1] = W_CacheLumpName("STRTLZ2",  PU_STATIC);
        rawgfx_startbot   = W_CacheLumpName("STRTBOT",  PU_STATIC);

        // Draw the background
        D_IntroBackground();
    }
    /*
    // STRIFE-FIXME: This was actually displayed on a special textmode
    // screen with ANSI color codes... would require use of textlib to
    // emulate properly...
    else
    {
        puts(("Conversation ON"));init
        puts(("Rogue Entertainment"));
        puts(("and"));
        puts(("Velocity Games"));
        puts(("present"));
        puts(("S T R I F E"));
        puts(("Loading..."));
    }
    */
}

//
// D_DrawIntroSequence
//
// [STRIFE] New function
// haleyjd 20110206: Refresh the intro sequence
//
static void D_DrawIntroSequence(void)
{
    int laserpos;
    int robotpos;

    if(!showintro)
        return;

    D_IntroBackground(); // haleyjd: refresh the background

    // Laser position
    laserpos = (200 * introprogress / MAXINTROPROGRESS) + 60;

    // BUG: (?) Due to this clip, the laser never even comes close to
    // touching the peasant; confirmed with vanilla. This MAY have been
    // intentional, for effect, however, since no death frames are shown 
    // either... kind of a black-out death.
    if(laserpos > 200)
        laserpos = 200;

    // Draw the laser
    // Blitted 16 bytes for 16 rows starting at 705280 + laserpos
    // (705280 - 0xA0000) / 320 == 156
    V_DrawBlock(laserpos, 156, 0, 16, 16, rawgfx_startlz[laserpos % 2]);

    // Robot position
    robotpos = laserpos % 5 - 2;

    // Draw the robot
    // Blitted 48 bytes for 48 rows starting at 699534 + (320*robotpos)
    // 699534 - 0xA0000 == 44174, which % 320 == 14, / 320 == 138
    V_DrawBlock(14, 138 + robotpos, 0, 48, 48, rawgfx_startbot);

    // Draw the peasant
    // Blitted 32 bytes for 64 rows starting at 699142
    // 699142 - 0xA0000 == 43782, which % 320 == 262, / 320 == 136
    V_DrawBlock(262, 136, 0, 32, 64, rawgfx_startp[laserpos % 4]);

//    I_FinishUpdate();
    I_FinishUpdate(1);
}

//
// D_IntroTick
//
// Advance the intro sequence
//
void D_IntroTick(void)
{
    static boolean didsound = false; // haleyjd 20120209
    
    if(devparm)
        return;

    ++introprogress;
    if(introprogress >= MAXINTROPROGRESS)
    {
        D_IntroBackground(); // haleyjd: clear the bg anyway
        
        // haleyjd 20120209: This isn't 100% true to vanilla because vanilla
        // would play this sound a half-dozen times. BUT, in vanilla, for 
        // whatever reason, under DMX, playing the same sound multiple times
        // doesn't add up violently like it does under SDL_mixer. This means
        // that without this one-time limitation, the sound is far too loud.
        if(!didsound)
        {
            S_StartSound(NULL, sfx_psdtha);
            didsound = true;
        }
    }
    else
        D_DrawIntroSequence();
}

//
// End Chocolate Strife Specifics
//
//=============================================================================

//
// D_DoomMain
//
void D_DoomMain (void)
{
/*
    int             p;
    char            file[256];
    char            demolumpname[9];
*/
    W_CheckSize();
#ifdef SHAREWARE
    if(filesize_strife0_wad == 10493652)	// FILE SIZE OF STRIFE0.WAD FOR v1.0
    {
	STRIFE_1_0_REGISTERED = false;
	STRIFE_1_X_REGISTERED = false;
	STRIFE_1_0_SHAREWARE = true;
	STRIFE_1_1_SHAREWARE = false;
    	gamemode = commercial;
    	gamemission = strife;
	isdemoversion = true;
	isregistered = false;
    }
    else if(filesize_strife0_wad == 9934413)	// FILE SIZE OF STRIFE0.WAD FOR v1.1
    {
	STRIFE_1_0_REGISTERED = false;
	STRIFE_1_X_REGISTERED = false;
	STRIFE_1_0_SHAREWARE = false;
	STRIFE_1_1_SHAREWARE = true;
    	gamemode = commercial;
    	gamemission = strife;
	isdemoversion = true;
	isregistered = false;
    }
#endif
    if(filesize_strife1_wad == 28372168)	// FILE SIZE OF STRIFE1.WAD FOR v1.0
    {
	STRIFE_1_0_REGISTERED = true;
	STRIFE_1_X_REGISTERED = false;
	gamemode = commercial;				// ADDED HERE FOR PSP
	gamemission = strife;
	isregistered = true;
	isdemoversion = false;
#ifdef SHAREWARE
	STRIFE_1_0_SHAREWARE = false;
	STRIFE_1_1_SHAREWARE = false;
#endif
    }
    else if(filesize_strife1_wad == 28377364)	// FILE SIZE OF STRIFE1.WAD FOR v1.2, v1.3, v1.31
    {
	STRIFE_1_0_REGISTERED = false;
	STRIFE_1_X_REGISTERED = true;
	gamemode = commercial;				// ADDED HERE FOR PSP
	gamemission = strife;
	isregistered = true;
	isdemoversion = false;
#ifdef SHAREWARE
	STRIFE_1_0_SHAREWARE = false;
	STRIFE_1_1_SHAREWARE = false;
#endif
    }

    if(filesize_voices_wad == 27319149)		// FILE SIZE OF VOICES.WAD FOR v1.2, v1.3, v1.31
	VOICES_1_X_REGISTERED = true;
    else
	VOICES_1_X_REGISTERED = false;
#ifdef SHAREWARE
    if(devparm || STRIFE_1_0_SHAREWARE || STRIFE_1_1_SHAREWARE)
#else
    if(devparm)
#endif
    {
	printf(" ");
	psvDebugScreenSetFgColor(0x000000);	// black
	psvDebugScreenSetBgColor(0x009E00);	// green

        printf("                   STRIFE:  Quest for the Sigil                   ");

	psvDebugScreenSetFgColor(0xD0D0D0);	// grey
	psvDebugScreenSetBgColor(0x000000);	// black
#ifdef SHAREWARE
	if(STRIFE_1_0_SHAREWARE || STRIFE_1_1_SHAREWARE)
	    devparm = true;
#endif
    }
#ifdef SHAREWARE
    if(strife0_wad_exists == 0 && strife1_wad_exists == 0)
#else
    if(strife1_wad_exists == 0)
#endif
    {
	psvDebugScreenClear(0x000000);

	printf(" ");

	psvDebugScreenSetFgColor(0x000000);	// black
	psvDebugScreenSetBgColor(0x009E00);	// green

//	if(STRIFE_1_0_SHAREWARE)
	    printf("                   STRIFE:  Quest for the Sigil                   ");
//	else if(STRIFE_1_1_SHAREWARE)
//	    printf("                STRIFE:  Quest for the Sigil v1.1                 ");

	psvDebugScreenSetFgColor(0xD0D0D0);	// grey
	psvDebugScreenSetBgColor(0x000000);	// black

//	if(devparm)
	    DEH_printf(D_DEVSTR);
	    
//	if(devparm)
	    DEH_printf(" Reading config from: strife.cfg\n");

//	if(devparm)
	    DEH_printf(" DPMI memory: 32.282624, 8.0M allocated for zone\n");

//	if(devparm)
	    DEH_printf(" Play Voices = %d\n", disable_voices == 0);

//	if(devparm) // [STRIFE] Devparm only
	    DEH_printf(" W_Init: Init WADfiles.\n");

	printf(" ");
        printf("\n");
        printf("\n");
        printf(" ================================================================== ");
		printf("\n");
	printf("             !!! STRIFE MAIN IWAD FILE IS MISSING !!!               ");
    	printf("\n");
	printf("   PLEASE COPY STRIFE'S MAIN IWAD FILE INTO THE GAME DIRECTORY.     ");
    	printf("\n");
	printf("                         QUITTING NOW ...                           ");
	printf("\n");
	printf(" ================================================================== ");

	wad_missing = 1;

	sceKernelDelayThread(5000*1000);

	I_QuitSerialFail();
    }
#ifdef SHAREWARE
    if(	(strife0_wad_exists == 1 && filesize_strife0_wad != 9934413
				 && filesize_strife0_wad != 10493652) ||
	(strife1_wad_exists == 1 && filesize_strife1_wad != 28372168
				 && filesize_strife1_wad != 28377364))
#else
    if(	(strife1_wad_exists == 1 && filesize_strife1_wad != 28372168
				 && filesize_strife1_wad != 28377364))
#endif
    {
	printf("\n WARNING: Wrong Version of STRIFE IWAD file detected.\n");
	printf("\n");		
	printf(" Please get an official STRIFE IWAD file.\n");
	printf("\n");
	printf("\n Quitting now...\n");
	sceKernelDelayThread(5000*1000);
	I_QuitSerialFail();
    }

    if(voices_wad_exists == 1 && filesize_voices_wad != 27319149)
    {
	printf("\n WARNING: Wrong Version of VOICES.WAD file detected.\n");
	printf("\n");		
	printf(" Please get an official v1.1 VOICES.WAD file.\n");
	printf("\n");
	printf(" Voices will be disabled.\n");
	printf("\n");
	disable_voices = 1;
    }

    I_AtExit(D_Endoom, false);

    // haleyjd 20110206 [STRIFE]: -nograph parameter

    //!
    // @vanilla
    //
    // Disable graphical introduction sequence
    //
/*
    if (M_ParmExists("-nograph"))
        showintro = false;

    // Undocumented:
    // Invoked by setup to test the controls.

    if (M_ParmExists("-testcontrols"))
    {
        testcontrols = true;
        showintro = false;
    }
*/
    // haleyjd 20110206: Moved up -devparm for max visibility

    //!
    // @vanilla
    //
    // Developer mode. Implies -nograph.
    //

//    devparm = M_CheckParm ("-devparm");

    // print banner

//    if(devparm)
//    	I_PrintBanner("Chocolate Doom 2.0.0"/*PACKAGE_STRING*/);

    //DEH_printf("Z_Init: Init zone memory allocation daemon. \n"); [STRIFE] removed
    Z_Init ();
/*
#ifdef FEATURE_MULTIPLAYER
    //!
    // @category net
    //
    // Start a dedicated server, routing packets but not participating
    // in the game itself.
    //

    if (M_CheckParm("-dedicated") > 0)
    {
        printf("Dedicated server mode.\n");
        NET_DedicatedServer();

        // Never returns
    }

    //!
    // @category net
    //
    // Query the Internet master server for a global list of active
    // servers.
    //

    if (M_CheckParm("-search"))
    {
        NET_MasterQuery();
        exit(0);
    }

    //!
    // @arg <address>
    // @category net
    //
    // Query the status of the server running on the given IP
    // address.
    //

    p = M_CheckParmWithArgs("-query", 1);

    if (p)
    {
        NET_QueryAddress(myargv[p+1]);
        exit(0);
    }

    //!
    // @category net
    //
    // Search the local LAN for running servers.
    //

    if (M_CheckParm("-localsearch"))
    {
        NET_LANQuery();
        exit(0);
    }

#endif
*/            
/*
#ifdef FEATURE_DEHACKED
    if(devparm)
        printf("DEH_Init: Init Dehacked support.\n");
    DEH_Init();
#endif
*/
    //!
    // @vanilla
    //
    // Disable monsters.
    //
/*
    nomonsters = M_CheckParm ("-nomonsters");

    //!
    // @vanilla
    //
    // Set Rogue playtesting mode (godmode, noclip toggled by backspace)
    //

    workparm = M_CheckParm ("-work");

    //!
    // @vanilla
    //
    // Flip player gun sprites (broken).
    //

    flipparm = M_CheckParm ("-flip");

    //!
    // @vanilla
    //
    // Respawn monsters after they are killed.
    //

    respawnparm = M_CheckParm ("-respawn");

    //!
    // @vanilla
    //
    // Items respawn at random locations
    //

    randomparm = M_CheckParm ("-random");

    //!
    // @vanilla
    //
    // Monsters move faster.
    //

    fastparm = M_CheckParm ("-fast");
*/
//    I_DisplayFPSDots(devparm);
/*
    // haleyjd 20110206 [STRIFE]: -devparm implies -nograph
    if(devparm)
        showintro = false;
*/
    // Note: Vanilla Strife does not understand the -deathmatch command
    // line parameter. deathmatch=1 is the default behavior when
    // playing a netgame.

    //!
    // @category net
    // @vanilla
    //
    // Start a deathmatch game.  Weapons do not stay in place and
    // all items respawn after 30 seconds.
    //
/*
    if (M_CheckParm ("-altdeath"))
        deathmatch = 2;
*/
    if (devparm)
        DEH_printf(D_DEVSTR);
    
    if(devparm)
        DEH_printf(" Reading config from: strife.cfg\n");

    if(devparm)
        DEH_printf(" DPMI memory: 32.282624, 8.0M allocated for zone\n");

    // haleyjd 20110220: This stuff was done in I_StartupSound in vanilla, but 
    // we'll do it here instead so we don't have to modify the low-level shared
    // code with Strife-specific stuff.

    //!
    // @vanilla
    //
    // Disable voice dialog and show dialog as text instead,
    // even if voices.wad can be found.
    //

    if(disable_voices/* || M_CheckParm("-novoice")*/)
    {
        dialogshowtext = disable_voices = 1;
    }

    if(devparm)
        DEH_printf(" Play Voices = %d\n", disable_voices == 0);

    // find which dir to use for config files
/*
#ifdef _WIN32

    //!
    // @platform windows
    // @vanilla
    //
    // Save configuration data and savegames in c:\strife.cd,
    // allowing play from CD.
    //

    if (M_CheckParm("-cdrom") > 0)
    {
        printf(D_CDROM);

        // haleyjd 08/22/2010: [STRIFE] Use strife.cd folder for -cdrom
        M_SetConfigDir("c:\\strife.cd\\");
    }
    else
#endif
*/
    {
        // Auto-detect the configuration dir.

        M_SetConfigDir(NULL);
    }
    
    //!
    // @arg <x>
    // @vanilla
    //
    // Turbo mode.  The player's speed is multiplied by x%.  If unspecified,
    // x defaults to 200.  Values are rounded up to 10 and down to 400.
    //
/*
    if ( (p=M_CheckParm ("-turbo")) )
    {
        int     scale = 200;
        extern int forwardmove[2];
        extern int sidemove[2];

        if (p<myargc-1)
            scale = atoi (myargv[p+1]);
        if (scale < 10)
            scale = 10;
        if (scale > 400)
            scale = 400;
        DEH_printf("turbo scale: %i%%\n", scale);
        forwardmove[0] = forwardmove[0]*scale/100;
        forwardmove[1] = forwardmove[1]*scale/100;
        sidemove[0] = sidemove[0]*scale/100;
        sidemove[1] = sidemove[1]*scale/100;
    }
*/    
    // init subsystems
    // DEH_printf("V_Init: allocate screens.\n"); [STRIFE] removed
    V_Init ();

    // Load configuration files before initialising other subsystems.
    // haleyjd 08/22/2010: [STRIFE] - use strife.cfg
    // DEH_printf("M_LoadDefaults: Load system defaults.\n"); [STRIFE] removed
    M_SetConfigFilenames("strife.cfg");//, PROGRAM_PREFIX "strife.cfg");
    D_BindVariables();

    M_LoadDefaults();

    if (!graphical_startup)
    {
        showintro = false;
    }

    // Save configuration at exit.
    I_AtExit(M_SaveDefaults, false);

    // Find the main IWAD file and load it.
    iwadfile = D_FindIWAD(IWAD_MASK_STRIFE, &gamemission);

    // None found?
    if (iwadfile == NULL)
    {
        I_Error("Game mode indeterminate.  No IWAD file was found.  Try\n"
                "specifying one with the '-iwad' command line parameter.\n");
    }
/*
//    modifiedgame = false;

    if(devparm) // [STRIFE] Devparm only
        DEH_printf(" W_Init: Init WADfiles.\n");
    D_IdentifyVersion();
*/
    while(1)
    {
        sceCtrlReadBufferPositive(0, &pad, 1);
#ifdef SHAREWARE
        if(strife0_wad_exists == 1 && strife1_wad_exists == 1)	// FOR PSP: (PICK AN IWAD)
        {
    	    if(iwad_message_has_been_shown == 1)
    	        goto skip_ask_for_iwad;

		
	    psvDebugScreenClear(0x000000);

	    printf(" ");

	    psvDebugScreenSetFgColor(0x000000);	// black
	    psvDebugScreenSetBgColor(0x009E00);	// green

//	    if(STRIFE_1_0_SHAREWARE)
	        printf("                   STRIFE:  Quest for the Sigil                   ");
//	    else if(STRIFE_1_1_SHAREWARE)
//	        printf("                STRIFE:  Quest for the Sigil v1.1                 ");

	    psvDebugScreenSetFgColor(0xD0D0D0);	// grey
	    psvDebugScreenSetBgColor(0x000000);	// black

//	    if(devparm)
	        DEH_printf(D_DEVSTR);
	    
//	    if(devparm)
	        DEH_printf(" Reading config from: strife.cfg\n");

//	    if(devparm)
	        DEH_printf(" DPMI memory: 32.282624, 8.0M allocated for zone\n");

//	    if(devparm)
	        DEH_printf(" Play Voices = %d\n", disable_voices == 0);

//	    if(devparm) // [STRIFE] Devparm only
	        DEH_printf(" W_Init: Init WADfiles.\n");

	    printf(" ");
            printf("\n");
            printf("\n");
            printf(" ================================================================== ");
	    printf("          !!! TWO MAIN IWAD FILES HAVE BEEN DETECTED !!!            ");
    	    printf("\n");
	    printf("             PLEASE CHOOSE WHICH ONE TO LOAD FOR GAME               ");
    	    printf("\n");
	    printf("      PRESS SQUARE FOR STRIFE0.WAD OR CIRCLE FOR STRIFE1.WAD        ");
	    printf(" ================================================================== ");

	    skip_ask_for_iwad:
	    {
	    }

	    if(pad.buttons != lastpad.buttons)
	    {
		lastpad = pad;

		if(pad.buttons & SCE_CTRL_SQUARE)
		{
		    if(filesize_strife0_wad == 10493652)	// FILE SIZE OF STRIFE0.WAD FOR v1.0
		    {
			STRIFE_1_0_REGISTERED = false;
			STRIFE_1_X_REGISTERED = false;
			STRIFE_1_0_SHAREWARE = true;
			STRIFE_1_1_SHAREWARE = false;
			gamemode = commercial;				// ADDED HERE FOR PSP
			gamemission = strife;
			isdemoversion = true;
			isregistered = false;
		    }
		    else if(filesize_strife0_wad == 9934413)	// FILE SIZE OF STRIFE0.WAD FOR v1.1
		    {
			STRIFE_1_0_REGISTERED = false;
			STRIFE_1_X_REGISTERED = false;
			STRIFE_1_0_SHAREWARE = false;
			STRIFE_1_1_SHAREWARE = true;
			gamemode = commercial;				// ADDED HERE FOR PSP
			gamemission = strife;
			isdemoversion = true;
			isregistered = false;
		    }

		    psvDebugScreenClear(0x000000);

		    printf(" ");

		    psvDebugScreenSetFgColor(0x000000);	// black
		    psvDebugScreenSetBgColor(0x009E00);	// green

		    if(STRIFE_1_0_SHAREWARE)
		        printf("                 STRIFE: Quest for the Sigil v1.0                 ");
		    else if(STRIFE_1_1_SHAREWARE)
		        printf("                 STRIFE: Quest for the Sigil v1.1                 ");

		    psvDebugScreenSetFgColor(0xD0D0D0);	// grey
		    psvDebugScreenSetBgColor(0x000000);	// black

	    	    if(devparm)
	    	        DEH_printf(D_DEVSTR);
	    
	    	    if(devparm)
	    	        DEH_printf(" Reading config from: strife.cfg\n");

	    	    if(devparm)
	    	        DEH_printf(" DPMI memory: 32.282624, 8.0M allocated for zone\n");

	    	    if(devparm)
	    	        DEH_printf(" Play Voices = %d\n", disable_voices == 0);

	    	    if(devparm) // [STRIFE] Devparm only
	    	        DEH_printf(" W_Init: Init WADfiles.\n");

		    D_AddFile("strife0.wad");

		    break;
	        }
		else if(pad.buttons & SCE_CTRL_CIRCLE)
		{
		    if(filesize_strife1_wad == 28372168)	// FILE SIZE OF STRIFE1.WAD FOR v1.0
		    {
			STRIFE_1_0_REGISTERED = true;
			STRIFE_1_X_REGISTERED = false;
			STRIFE_1_0_SHAREWARE = false;
			STRIFE_1_1_SHAREWARE = false;
			gamemode = commercial;				// ADDED HERE FOR PSP
			gamemission = strife;
			isdemoversion = false;
			isregistered = true;
		    }
		    else if(filesize_strife1_wad == 28377364)	// FILE SIZE OF STRIFE1.WAD FOR v1.2, v1.3, v1.31
		    {
			STRIFE_1_0_REGISTERED = false;
			STRIFE_1_X_REGISTERED = true;
			STRIFE_1_0_SHAREWARE = false;
			STRIFE_1_1_SHAREWARE = false;
			gamemode = commercial;				// ADDED HERE FOR PSP
			gamemission = strife;
			isdemoversion = false;
			isregistered = true;
		    }

		    psvDebugScreenClear(0x000000);

		    printf(" ");

		    psvDebugScreenSetFgColor(0x000000);	// black
		    psvDebugScreenSetBgColor(0x009E00);	// green

		    if(devparm)
		    {
		    	if(STRIFE_1_0_REGISTERED)
		    	    printf("                 STRIFE: Quest for the Sigil v1.0                 ");
		    	else if(STRIFE_1_X_REGISTERED)
		    	    printf("                STRIFE:  Quest for the Sigil v1.31                ");
		    }

		    psvDebugScreenSetFgColor(0xD0D0D0);	// grey
		    psvDebugScreenSetBgColor(0x000000);	// black

	    	    if(devparm)
	    	        DEH_printf(D_DEVSTR);
	    
	    	    if(devparm)
	    	        DEH_printf(" Reading config from: strife.cfg\n");

	    	    if(devparm)
	    	        DEH_printf(" DPMI memory: 32.282624, 8.0M allocated for zone\n");

	    	    if(devparm)
	    	        DEH_printf(" Play Voices = %d\n", disable_voices == 0);

	    	    if(devparm) // [STRIFE] Devparm only
	    	        DEH_printf(" W_Init: Init WADfiles.\n");

		    if(voices_wad_exists == 1)
		    	D_AddFile("voices.wad");
		    else
			disable_voices = 1;

		    D_AddFile("strife1.wad");

		    break;
	        }
	    }
	    iwad_message_has_been_shown = 1;
	}

	else if ((strife0_wad_exists == 1 && strife1_wad_exists == 0) || (strife0_wad_exists == 0 && strife1_wad_exists == 1))
#else
	if (strife1_wad_exists == 1)
#endif
	{
#ifdef SHAREWARE
	    if(filesize_strife0_wad == 10493652)	// FILE SIZE OF STRIFE0.WAD FOR v1.0
	    {
		STRIFE_1_0_REGISTERED = false;
		STRIFE_1_X_REGISTERED = false;
		STRIFE_1_0_SHAREWARE = true;
		STRIFE_1_1_SHAREWARE = false;
		gamemode = commercial;				// ADDED HERE FOR PSP
		gamemission = strife;
		isdemoversion = true;
		isregistered = false;
	    }
	    else if(filesize_strife0_wad == 9934413)	// FILE SIZE OF STRIFE0.WAD FOR v1.1
	    {
		STRIFE_1_0_REGISTERED = false;
		STRIFE_1_X_REGISTERED = false;
		STRIFE_1_0_SHAREWARE = false;
		STRIFE_1_1_SHAREWARE = true;
		gamemode = commercial;				// ADDED HERE FOR PSP
		gamemission = strife;
		isdemoversion = true;
		isregistered = false;
	    }
#endif
	    if(filesize_strife1_wad == 28372168)	// FILE SIZE OF STRIFE1.WAD FOR v1.0
	    {
		STRIFE_1_0_REGISTERED = true;
		STRIFE_1_X_REGISTERED = false;
		gamemode = commercial;				// ADDED HERE FOR PSP
		gamemission = strife;
		isregistered = true;
		isdemoversion = false;
#ifdef SHAREWARE
		STRIFE_1_0_SHAREWARE = false;
		STRIFE_1_1_SHAREWARE = false;
#endif
	    }
	    else if(filesize_strife1_wad == 28377364)	// FILE SIZE OF STRIFE1.WAD FOR v1.2, v1.3, v1.31
	    {
		STRIFE_1_0_REGISTERED = false;
		STRIFE_1_X_REGISTERED = true;
		gamemode = commercial;				// ADDED HERE FOR PSP
		gamemission = strife;
		isregistered = true;
		isdemoversion = false;
#ifdef SHAREWARE
		STRIFE_1_0_SHAREWARE = false;
		STRIFE_1_1_SHAREWARE = false;
#endif
	    }

	    psvDebugScreenClear(0x000000);

	    printf(" ");

	    psvDebugScreenSetFgColor(0x000000);	// black
	    psvDebugScreenSetBgColor(0x009E00);	// green

//	    if(devparm)
	    {
#ifdef SHAREWARE
	        if(STRIFE_1_0_SHAREWARE || (devparm && STRIFE_1_0_REGISTERED))
#else
	        if(devparm && STRIFE_1_0_REGISTERED)
#endif
	    	    printf("                 STRIFE: Quest for the Sigil v1.0                 ");
#ifdef SHAREWARE
	    	else if(STRIFE_1_1_SHAREWARE)
	    	    printf("                 STRIFE: Quest for the Sigil v1.1                 ");
#endif
	    	else if(devparm && STRIFE_1_X_REGISTERED)
	    	    printf("                STRIFE:  Quest for the Sigil v1.31                ");
	    }

	    psvDebugScreenSetFgColor(0xD0D0D0);	// grey
	    psvDebugScreenSetBgColor(0x000000);	// black

    	    if(devparm)
    	        DEH_printf(D_DEVSTR);
	    
    	    if(devparm)
    	        DEH_printf(" Reading config from: strife.cfg\n");

    	    if(devparm)
    	        DEH_printf(" DPMI memory: 32.282624, 8.0M allocated for zone\n");

    	    if(devparm)
    	        DEH_printf(" Play Voices = %d\n", disable_voices == 0);

    	    if(devparm) // [STRIFE] Devparm only
    	        DEH_printf(" W_Init: Init WADfiles.\n");

	    if(voices_wad_exists && (STRIFE_1_0_REGISTERED || STRIFE_1_X_REGISTERED))
	    	D_AddFile("voices.wad");
	    else
		disable_voices = 1;

	    D_AddFile(iwadfile);

	    break;
	}
    }

//    D_AddFile(iwadfile);
    D_AddFile("pspstrife.wad");			// REQUIRED FOR SPECIAL PSP STUFF
//    W_CheckCorrectIWAD(strife);		// DISABLED FOR PSP - sorry :-( I'LL TRY TO FIND A FIX
//    modifiedgame = W_ParseCommandLine();

    // [STRIFE] serial number check and output	// ADDED SERIAL CHECK FOR PSP SOURCE PORT
//    if(devparm)
    {
//        char msgbuf[80];
        char *serial  = W_CacheLumpName("SERIAL", PU_CACHE);
        int serialnum = atoi(serial);

	while(1)
	{
	    sceCtrlReadBufferPositive(0, &pad, 1);
#ifdef SHAREWARE
	    if(serialnum == 960222)		// FOR PSP: SHAREWARE VERSION v1.0 (NOT YET SUPPORTED)
	    {
		if(warning_message_has_been_shown == 1)
		    goto skip_show_message_v10;

	    	psvDebugScreenClear(0x000000);

	    	printf(" ");

	    	psvDebugScreenSetFgColor(0x000000);	// black
	    	psvDebugScreenSetBgColor(0x009E00);	// green

//	    	if(devparm)
	    	{
	    	    if(STRIFE_1_0_SHAREWARE || (devparm && STRIFE_1_0_REGISTERED))
	    	    	printf("                 STRIFE: Quest for the Sigil v1.0                 ");
	    	    else if(STRIFE_1_1_SHAREWARE)
	    	    	printf("                 STRIFE: Quest for the Sigil v1.1                 ");
	    	    else if(devparm && STRIFE_1_X_REGISTERED)
	    	    	printf("                STRIFE:  Quest for the Sigil v1.31                ");
	    	}

		psvDebugScreenSetFgColor(0xD0D0D0);	// grey
		psvDebugScreenSetBgColor(0x000000);	// black

//	    	if(devparm)
	    	    DEH_printf(D_DEVSTR);
	    
//	    	if(devparm)
	    	    DEH_printf(" Reading config from: strife.cfg\n");

//	    	if(devparm)
	    	    DEH_printf(" DPMI memory: 32.282624, 8.0M allocated for zone\n");

//	    	if(devparm)
	    	    DEH_printf(" Play Voices = %d\n", disable_voices == 0);

//	    	if(devparm) // [STRIFE] Devparm only
	    	    DEH_printf(" W_Init: Init WADfiles.\n");

	        printf("\n");
	        printf("\n");
	        printf(" ================================================================== ");
			printf("\n");
    	        printf("                          !!! WARNING !!!                           ");
	        printf("\n");
	        printf("                 YOU ARE MOST LIKELY TRYING TO USE                  ");
			printf("\n");
	        printf("            SHAREWARE VERSION 1.0 OF STRIFE'S IWAD FILE             ");
			printf("\n");
	        printf("         WHICH IS NOT EVEN SUPPORTED BY THIS PORT (YET)!!!          ");
	        printf("\n");
	        printf("            PLEASE GET A KNOWN PROPER STRIFE IWAD FILE.             ");
	        printf("\n");
	        printf("          THERE DO EXIST THE FOLLOWING STRIFE IWAD FILES:           ");
	        printf("\n");
	        printf("                 (SHAREWARE VERSION): v1.0 & v1.1                   ");
			printf("\n");
	        printf("         (REGISTERED VERSION):  v1.0 & v1.2 & v1.3 & v1.31          ");
	        printf("\n");
	        printf("            PLEASE GET A KNOWN PROPER STRIFE IWAD FILE.             ");
	        printf("\n");
	    	printf("                 PRESS CROSS TO QUIT THE GAME NOW                   ");
			printf("\n");
	        printf(" ================================================================== ");
			printf("\n");

		skip_show_message_v10:
		{
		}

	    	if(pad.buttons != lastpad.buttons)
	    	{
		    lastpad = pad;
		    if(pad.buttons & SCE_CTRL_CROSS)
		    {
			I_QuitSerialFail();
	            }
	        }
		warning_message_has_been_shown = 1;
	    }

	    else if(serialnum == 960313)	// FOR PSP: SHAREWARE VERSION v1.1 (PARTIAL SUPPORT)
	    {
		if(warning_message_has_been_shown == 1)
		    goto skip_show_message_v11;

	    	psvDebugScreenClear(0x000000);

	    	printf(" ");

	    	psvDebugScreenSetFgColor(0x000000);	// black
	    	psvDebugScreenSetBgColor(0x009E00);	// green

//	    	if(devparm)
	    	{
	    	    if(STRIFE_1_0_SHAREWARE || (devparm && STRIFE_1_0_REGISTERED))
	    	    	printf("                 STRIFE: Quest for the Sigil v1.0                 ");
	    	    else if(STRIFE_1_1_SHAREWARE)
	    	    	printf("                 STRIFE: Quest for the Sigil v1.1                 ");
	    	    else if(devparm && STRIFE_1_X_REGISTERED)
	    	    	printf("                STRIFE:  Quest for the Sigil v1.31                ");
	    	}

		psvDebugScreenSetFgColor(0xD0D0D0);	// grey
		psvDebugScreenSetBgColor(0x000000);	// black

//	    	if(devparm)
	    	    DEH_printf(D_DEVSTR);
	    
//	    	if(devparm)
	    	    DEH_printf(" Reading config from: strife.cfg\n");

//	    	if(devparm)
	    	    DEH_printf(" DPMI memory: 32.282624, 8.0M allocated for zone\n");

//	    	if(devparm)
	    	    DEH_printf(" Play Voices = %d\n", disable_voices == 0);

//	    	if(devparm) // [STRIFE] Devparm only
	    	    DEH_printf(" W_Init: Init WADfiles.\n");

//		printf(" ");
		printf("\n");
		printf("\n");
		printf(" ================================================================== ");
    		printf("                          !!! WARNING !!!                           ");
		printf("\n");
		printf("                 YOU ARE MOST LIKELY TRYING TO USE                  ");
		printf("            SHAREWARE VERSION 1.1 OF STRIFE'S IWAD FILE             ");
		printf("         WHICH IS NOT YET FULLY SUPPORTED BY THIS PORT !!!          ");
		printf("\n");
		printf("          THERE DO EXIST THE FOLLOWING STRIFE IWAD FILES:           ");
		printf("\n");
		printf("                 (SHAREWARE VERSION): v1.0 & v1.1                   ");
		printf("         (REGISTERED VERSION):  v1.0 & v1.2 & v1.3 & v1.31          ");
		printf("\n");
		printf("          BUT YOU STILL CAN TAKE A LOOK AT THIS OF COURSE           ");
		printf("\n");
	    	printf("                      PRESS CROSS TO CONTINUE                       ");
		printf(" ================================================================== ");

		skip_show_message_v11:
		{
		}

	    	if(pad.buttons != lastpad.buttons)
	    	{
		    lastpad = pad;
		    if(pad.buttons & SCE_CTRL_CROSS)
		    {

		    	if(filesize_strife0_wad == 10493652)	// FILE SIZE OF STRIFE0.WAD FOR v1.0
		    	{
			    STRIFE_1_0_REGISTERED = false;
			    STRIFE_1_X_REGISTERED = false;
			    STRIFE_1_0_SHAREWARE = true;
			    STRIFE_1_1_SHAREWARE = false;
			    gamemode = commercial;				// ADDED HERE FOR PSP
			    gamemission = strife;
			    isdemoversion = true;
			    isregistered = false;
		    	}
		    	else if(filesize_strife0_wad == 9934413)	// FILE SIZE OF STRIFE0.WAD FOR v1.1
		    	{
			    STRIFE_1_0_REGISTERED = false;
			    STRIFE_1_X_REGISTERED = false;
			    STRIFE_1_0_SHAREWARE = false;
			    STRIFE_1_1_SHAREWARE = true;
			    gamemode = commercial;				// ADDED HERE FOR PSP
			    gamemission = strife;
			    isdemoversion = true;
			    isregistered = false;
		    	}

			devparm = true;

		    	psvDebugScreenClear(0x000000);

		    	printf(" ");

		    	psvDebugScreenSetFgColor(0x000000);	// black
		    	psvDebugScreenSetBgColor(0x009E00);	// green

			if(devparm)
			{
			    if(STRIFE_1_0_SHAREWARE)
		    	    	printf("                 STRIFE: Quest for the Sigil v1.0                 ");
		    	    else if(STRIFE_1_1_SHAREWARE)
		    	    	printf("                 STRIFE: Quest for the Sigil v1.1                 ");
			}

		    	psvDebugScreenSetFgColor(0xD0D0D0);	// grey
		    	psvDebugScreenSetBgColor(0x000000);	// black

//	    	    	if(devparm)
	    	    	    DEH_printf(D_DEVSTR);
	    
//	    	    	if(devparm)
	    	    	    DEH_printf(" Reading config from: strife.cfg\n");

//	    	    	if(devparm)
	    	    	    DEH_printf(" DPMI memory: 32.282624, 8.0M allocated for zone\n");

//	    	    	if(devparm)
	    	    	    DEH_printf(" Play Voices = %d\n", disable_voices == 0);

//	    	    	if(devparm) // [STRIFE] Devparm only
	    	    	    DEH_printf(" W_Init: Init WADfiles.\n");

//			if(devparm)
			{
			    printf("         adding strife0.wad");
			}

//			if(devparm)
			{
			    printf("\n");
			    printf("         adding pspstrife.wad");
			    printf("\n");
			}

			break;
	            }
	        }
		warning_message_has_been_shown = 1;
	    }

	    else if(serialnum == 960416)	// FOR PSP: REGISTERED VERSION v1.0, v1.2, v1.3, v1.31
#else
	    if(serialnum == 960416)	// FOR PSP: REGISTERED VERSION v1.0, v1.2, v1.3, v1.31
#endif
    	    	goto load_the_game;

	    else if(!serialnum)			// FOR PSP: UNKNOWN IWAD DETECTED
	    {
		if(warning_message_has_been_shown == 1)
		    goto skip_show_message_unknown;

	    	printf(" ");
	    	printf(" ================================================================== ");
    	    	printf("                          !!! WARNING !!!                           ");
	    	printf("\n");
	    	printf("                 THE IWAD FILE THAT YOU'RE TRYING                   ");
	    	printf("            TO USE IS NOT A KNOWN STRIFE IWAD FILE !!!              ");
	    	printf("\n");
	    	printf("            PLEASE GET A KNOWN PROPER STRIFE IWAD FILE              ");
	    	printf("\n");
	    	printf("                 PRESS CROSS TO QUIT THE GAME NOW                   ");
	    	printf(" ================================================================== ");

		skip_show_message_unknown:
		{
		}

	    	if(pad.buttons != lastpad.buttons)
	    	{
		    lastpad = pad;
		    if(pad.buttons & SCE_CTRL_CROSS)
		    {
			I_QuitSerialFail();
	            }
	        }
		warning_message_has_been_shown = 1;
	    }
	}

	load_the_game:
	{
	    if(devparm && (STRIFE_1_0_REGISTERED || STRIFE_1_X_REGISTERED))
	    {
//    	    	DEH_snprintf(msgbuf, sizeof(msgbuf),
		printf(" Wad Serial Number: %d:          retail version.\n", serialnum);
		printf(" ================================================================== ");
	    	printf("          This version is NOT SHAREWARE, do not distribute!         ");
	    	printf("      Please report software piracy to the SPA:  1-800-388-PIR8     ");
	    	printf(" ================================================================== ");
	    }
#ifdef SHAREWARE
	    else if(devparm && (STRIFE_1_0_SHAREWARE || STRIFE_1_1_SHAREWARE))
	    {
//    	    	DEH_snprintf(msgbuf, sizeof(msgbuf),
		printf(" Wad Serial Number: %d:          demo version.\n", serialnum);	
	    }
#endif
//	    printf("%s\n", msgbuf);
	}
    }

    // haleyjd 20110206 [STRIFE]: -devparm implies -nograph
#ifdef SHAREWARE
    if(devparm || STRIFE_1_0_SHAREWARE || STRIFE_1_1_SHAREWARE)	// FOR PSP: (NOT AVAIL. IN SHAREWARE)
#else
    if(devparm)	// FOR PSP: (NOT AVAIL. IN SHAREWARE)
#endif
        showintro = false;

    // add any files specified on the command line with -file wadfile
    // to the wad list
    //

    // Debug:
/*
    if(devparm_print_dir)
    {
	W_PrintDirectory();
    }
*/
    //!
    // @arg <demo>
    // @category demo
    // @vanilla
    //
    // Play back the demo named demo.lmp.
    //
/*
    p = M_CheckParmWithArgs ("-playdemo", 1);

    if (!p)

    {
        //!
        // @arg <demo>
        // @category demo
        // @vanilla
        //
        // Play back the demo named demo.lmp, determining the framerate
        // of the screen.
        //
	p = M_CheckParmWithArgs("-timedemo", 1);

    }

    if (p)
    {
        // With Vanilla you have to specify the file without extension,
        // but make that optional.
        if (M_StringEndsWith(myargv[p + 1], ".lmp"))
        {
            M_StringCopy(file, myargv[p + 1], sizeof(file));
        }
        else
        {
            DEH_snprintf(file, sizeof(file), "%s.lmp", myargv[p+1]);
        }

        if (D_AddFile (file))
        {
            M_StringCopy(demolumpname, lumpinfo[numlumps - 1].name,
                         sizeof(demolumpname));
        }
        else
        {
            // If file failed to load, still continue trying to play
            // the demo in the same way as Vanilla Doom.  This makes
            // tricks like "-playdemo demo1" possible.

            M_StringCopy(demolumpname, myargv[p + 1], sizeof(demolumpname));
        }

        printf("Playing demo %s.\n", file);
    }
*/
    I_AtExit((atexit_func_t) G_CheckDemoStatus, true);

    // Generate the WAD hash table.  Speed things up a bit.

    W_GenerateHashTable();
    
//    D_IdentifyVersion();
    InitGameVersion();
    D_SetGameDescription();
    savegamedir = M_GetSaveGameDir("/strife1.wad/");
/*
    // fraggle 20130405: I_InitTimer is needed here for the netgame
    // startup. Start low-level sound init here too.
    I_InitTimer();
    I_InitSound(true);
    I_InitMusic();
*/
/*
#ifdef FEATURE_MULTIPLAYER
    if(devparm) // [STRIFE]
        printf ("NET_Init: Init network subsystem.\n");
    NET_Init();
#endif

    D_ConnectNetGame();
*/
    // haleyjd 20110210: Create Strife hub save folders
    M_CreateSaveDirs(savegamedir);

//    I_GraphicsCheckCommandLine();

    // haleyjd 20110206 [STRIFE] Startup the introduction sequence
    if(STRIFE_1_0_REGISTERED || STRIFE_1_X_REGISTERED)
    	D_InitIntroSequence();
/*
    // haleyjd 20110924: moved S_Init up to here
    if(devparm) // [STRIFE]
        DEH_printf(" S_Init: Setting up sound.\n");
    S_Init (sfxVolume * 8, musicVolume * 8, voiceVolume * 8); // [STRIFE]: voice
    D_IntroTick(); // [STRIFE]
*/
    // Check for -file in shareware
/*
    if (modifiedgame)
    {
        // These are the lumps that will be checked in IWAD,
        // if any one is not present, execution will be aborted.
        // haleyjd 08/22/2010: [STRIFE] Check for Strife lumps.
        char name[3][8]=
        {
            "map23", "map30", "ROB3E1"
        };
        int i;

        // haleyjd 08/22/2010: [STRIFE] Changed string to match binary
        // STRIFE-FIXME: Needs to test isdemoversion variable
        if ( gamemode == shareware)
            I_Error(("\nYou cannot -file with the demo "
                               "version. You must buy the real game!"));

        // Check for fake IWAD with right name,
        // but w/o all the lumps of the registered version. 
        // STRIFE-FIXME: Needs to test isregistered variable
        if (gamemode == registered)
            for (i = 0; i < 3; i++)
                if (W_CheckNumForName(name[i])<0)
                    I_Error(("\nThis is not the registered version."));
    }
*/    
    D_IntroTick(); // [STRIFE]
    
    // get skill / episode / map from parms
    startskill = sk_medium;
    startepisode = 1;
    startmap = 1;
//    autostart = false;

    //!
    // @arg <skill>
    // @vanilla
    //
    // Set the game skill, 1-5 (1: easiest, 5: hardest).  A skill of
    // 0 disables all monsters.
    //
/*
    p = M_CheckParmWithArgs("-skill", 1);

    if (p)
    {
        startskill = myargv[p+1][0]-'1';
        autostart = true;
    }
*/
    // [STRIFE] no such thing in Strife
    //
    // // @arg <n>
    // // @vanilla
    // //
    // // Start playing on episode n (1-4)
    // //

    // p = M_CheckParmWithArgs("-episode", 1);

    // if (p)
    // {
    //     startepisode = myargv[p+1][0]-'0';
    //     startmap = 1;
    //     autostart = true;
    // }

    timelimit = 0;

    //! 
    // @arg <n>
    // @category net
    // @vanilla
    //
    // For multiplayer games: exit each level after n minutes.
    //
/*
    p = M_CheckParmWithArgs("-timer", 1);

    if (p)
    {
        timelimit = atoi(myargv[p+1]);
        printf("timer: %i\n", timelimit);
    }
*/
    //!
    // @category net
    // @vanilla
    //
    // Austin Virtual Gaming: end levels after 20 minutes.
    //
/*
    p = M_CheckParm ("-avg");

    if (p)
    {
        timelimit = 20;
    }
*/
    //!
    // @arg x
    // @vanilla
    //
    // Start a game immediately, warping to level x.
    //
/*
    p = M_CheckParmWithArgs("-warp", 1);

    if (p)
    {
        if (gamemode == commercial)
            startmap = atoi (myargv[p+1]);
        else
        {
            startepisode = myargv[p+1][0]-'0';

            if (p + 2 < myargc)
            {
                startmap = myargv[p+2][0]-'0';
            }
            else
            {
                startmap = 1;
            }
        }
        autostart = true;
    }

    if (testcontrols)
    {
        startepisode = 1;
        startmap = 3;
        autostart = true;
    }
*/
    // Check for load game parameter
    // We do this here and save the slot number, so that the network code
    // can override it or send the load slot to other players.

    //!
    // @arg <s>
    // @vanilla
    //
    // Load the game in slot s.
    //
/*
    p = M_CheckParmWithArgs("-loadgame", 1);
    
    if (p)
    {
        startloadgame = atoi(myargv[p+1]);
    }
    else
*/
    {
        // Not loading a game
        startloadgame = -1;
    }
/*
    if (W_CheckNumForName("SS_START") >= 0
     || W_CheckNumForName("FF_END") >= 0)
    {
        I_PrintDivider();
        printf(" WARNING: The loaded WAD file contains modified sprites or\n"
               " floor textures.  You may want to use the '-merge' command\n"
               " line option instead of '-file'.\n");
    }

    if(devparm)
    {
    	I_PrintStartupBanner(gamedescription);
    	PrintDehackedBanners();
    }
*/
    // haleyjd 08/28/10: Init Choco Strife stuff.
    D_InitChocoStrife();

    // haleyjd 08/22/2010: [STRIFE] Modified string to match binary
    if(devparm) // [STRIFE]
       DEH_printf(" R_Init: Loading Graphics - ");

    R_Init ();
    D_IntroTick(); // [STRIFE]

    if(devparm) // [STRIFE]
        DEH_printf("\n P_Init: Init Playloop state.\n");
    P_Init ();
    D_IntroTick(); // [STRIFE]

    if(devparm) // [STRIFE]
        DEH_printf(" I_Init: Setting up machine state.\n");
//    I_CheckIsScreensaver();
    D_IntroTick(); // [STRIFE]

    if(devparm) // [STRIFE]
    {
        DEH_printf(" I_StartupDPMI\n");
        DEH_printf(" I_StartupMouse\n");
        DEH_printf(" Mouse: not detected\n");
        DEH_printf(" I_StartupJoystick\n");
        DEH_printf(" Joystick: detected\n");
        DEH_printf(" I_StartupKeyboard\n");
        DEH_printf(" Keyboard: detected\n");
    }
//    I_InitJoystick();
    D_IntroTick(); // [STRIFE]

    // fraggle 20130405: I_InitTimer is needed here for the netgame
    // startup. Start low-level sound init here too.

    if(devparm) // [STRIFE]
        DEH_printf(" I_StartupSound\n");
    I_InitSound(true);
    I_InitMusic();

    if(devparm) // [STRIFE]
        DEH_printf(" I_StartupTimer()\n");
    I_InitTimer();

    if(devparm)
        DEH_printf("  Play Voices = %d\n", disable_voices == 0);

    if(devparm) // [STRIFE]
        DEH_printf(" D_CheckNetGame: Checking network game status.\n");
    D_CheckNetGame ();

    if(devparm) // [STRIFE]
        DEH_printf(" M_Init: Init Menu.\n");
    M_Init ();
    D_IntroTick(); // [STRIFE]

    // haleyjd 20110924: moved S_Init up to here
    if(devparm) // [STRIFE]
        DEH_printf(" S_Init: Setting up sound.\n");
    S_Init (sfxVolume * 8, musicVolume * 8, voiceVolume * 8); // [STRIFE]: voice
    D_IntroTick(); // [STRIFE]

    // haleyjd 20110924: Moved S_Init up.
    D_IntroTick();
/*
    // haleyjd 20110220: This stuff was done in I_StartupSound in vanilla, but 
    // we'll do it here instead so we don't have to modify the low-level shared
    // code with Strife-specific stuff.

    //!
    // @vanilla
    //
    // Disable voice dialog and show dialog as text instead,
    // even if voices.wad can be found.
    //

    if(disable_voices)// || M_CheckParm("-novoice"))
    {
        dialogshowtext = disable_voices = 1;
    }

    if(devparm)
        DEH_printf(" Play Voices = %d\n", disable_voices == 0);
*/
/*
    if(devparm) // [STRIFE]
        DEH_printf(" D_CheckNetGame: Checking network game status.\n");
    D_CheckNetGame ();
*/
    PrintGameVersion();

    if(devparm)
        DEH_printf(" HU_Init: Setting up heads up display.\n");
    HU_Init ();
    D_IntroTick(); // [STRIFE]

    if(devparm)
        DEH_printf(" ST_Init: Init status bar.\n");
    ST_Init ();
    D_IntroTick(); // [STRIFE]

    // haleyjd [STRIFE] -statcopy used to be here...
    D_IntroTick(); // [STRIFE]

    // If Doom II without a MAP01 lump, this is a store demo.  
    // Moved this here so that MAP01 isn't constantly looked up
    // in the main loop.
    // haleyjd 08/23/2010: [STRIFE] There is no storedemo version of Strife
    /*
    if (gamemode == commercial && W_CheckNumForName("map01") < 0)
        storedemo = true;
    */

    //!
    // @arg <x>
    // @category demo
    // @vanilla
    //
    // Record a demo named x.lmp.
    //
/*
    p = M_CheckParmWithArgs("-record", 1);

    if (p)
    {
        G_RecordDemo (myargv[p+1]);
        autostart = true;
    }
*/
    D_IntroTick(); // [STRIFE]
/*
    p = M_CheckParmWithArgs("-playdemo", 1);
    if (p)
    {
        singledemo = true;              // quit after one demo
        G_DeferedPlayDemo (demolumpname);
        D_DoomLoop ();  // never returns
    }
*/
    D_IntroTick(); // [STRIFE]
/*
    p = M_CheckParmWithArgs("-timedemo", 1);
    if (p)
    {
        G_TimeDemo (demolumpname);
        D_DoomLoop ();  // never returns
    }
*/
    D_IntroTick(); // [STRIFE]

    if (startloadgame >= 0)
    {
        // [STRIFE]: different, for hubs
        M_LoadSelect(startloadgame);
    }
    D_IntroTick(); // [STRIFE]


    if (gameaction != ga_loadgame )
    {
        if (autostart/* || netgame*/)
	{					// FOR PSP: POSSIBILITY TO RECORD A NEW DEMO...
            G_InitNew (startskill, 2);		// ...SINCE CHANGING CODE MAY GETS ALMOST RECORDED...
	    G_RecordDemo ("demo1");		// ...DEMOS "OUT OF SYNC" (ONLY NEEDS BOOLEAN...
            G_BeginRecording ();		// ..."AUTOSTART" SET TO TRUE)
	}
        else
            D_StartTitle ();                	// start up intro loop
    }

    D_DoomLoop ();  // never returns
}

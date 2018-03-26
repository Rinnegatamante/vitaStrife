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
//	DOOM selection menu, options, episode etc.
//	Sliders and icons. Kinda widget stuff.
//


#include <stdlib.h>
#include <ctype.h>

#include "doomdef.h"
#include "doomkeys.h"
#include "dstrings.h"

#include "d_main.h"
#include "deh_str.h"
#include "deh_misc.h"

#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"

#include "r_local.h"

#include "hu_stuff.h"

#include "g_game.h"

//#include "m_argv.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_saves.h"    // [STRIFE]
#include "p_saveg.h"
#include "p_inter.h"

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "sounds.h"

#include "m_menu.h"
#include "p_dialog.h"

#include "doomfeatures.h"

#include <vitasdk.h>

extern void		M_QuitStrife(int);

extern default_t	doom_defaults_list[];		// FOR PSP: KEY BINDINGS

extern patch_t*         hu_font[HU_FONTSIZE];
extern boolean          message_dontfuckwithme;

//extern boolean          chat_on;        // in heads-up code
extern boolean          sendsave;       // [STRIFE]

#ifdef SHAREWARE
extern boolean		STRIFE_1_0_SHAREWARE;
extern boolean		STRIFE_1_1_SHAREWARE;
#endif

extern boolean		STRIFE_1_0_REGISTERED;
extern boolean		STRIFE_1_X_REGISTERED;

extern boolean		display_ticker;

extern int		cheating;
extern int		mspeed;
extern int		mouselook;

extern int		dots_enabled;
extern int		fps_enabled;
extern int		dont_show;
extern int		display_fps;

extern int		allocated_ram_size;

extern uint32_t		tickResolution;
extern uint64_t		fpsTickLast;
extern uint64_t		systemTime;

static int		FirstKey = 0;		// FOR PSP: SPECIAL MENU FUNCTIONS (ITEMCOUNT) ;-)
static int		keyaskedfor;
static boolean		askforkey = false;

int			cheeting;
int			map = 0;
int			spot = 0;
int			musnum = 1;
int			crosshair = 0;
int			button_layout = 0;

int			mhz333 = 0;
int			frame_limit = 0;
int			memory_info = 0;
int			battery_info = 0;
int			cpu_info = 0;
int			coordinates_info = 0;
int			other_info = 0;
int			fps = 0;		// FOR PSP: calculating the frames per second

int			followplayer = 1;
int			drawgrid;
int			show_stats = 0;

int			show_talk = 1;
int			messages_disabled = 0;
/*
						// (PSP): USED TO BE FOR SET CFG VARS 'TICKER' & 'FPS',
						// BUT CAUSES A CRASH AS SOON AS THE SCREEN WIPES
int			key_controls_start_in_cfg_at_pos = 19;	// FOR PSP: ACTUALLY IT'S +2 !!!
int			key_controls_end_in_cfg_at_pos = 32;	// FOR PSP: ACTUALLY IT'S +2 !!!
*/
int			key_controls_start_in_cfg_at_pos = 18;	// FOR PSP: ACTUALLY IT'S +2 !!!
int			key_controls_end_in_cfg_at_pos = 31;	// FOR PSP: ACTUALLY IT'S +2 !!!
int			max_free_ram = 0;

boolean			am_rotate;

char			allocated_ram_textbuffer[50];
char			free_ram_textbuffer[50];
char			max_free_ram_textbuffer[50];
char			fpsDisplay[100];
char			map_coordinates_textbuffer[50];
uint64_t			fpsTickNow;

//
// defaulted values
//
int			mouseSensitivity;

// [STRIFE]: removed this entirely
// Show messages has default, 0 = off, 1 = on
int			showMessages = 1;
	

// Blocky mode, has default, 0 = high, 1 = normal
int			detailLevel = 0;
int			screenblocks = 9;

// temp for screenblocks (0-9)
int			screenSize;

// -1 = no quicksave slot picked!
int			quickSaveSlot;

 // 1 = message to be printed
int			messageToPrint;
// ...and here is the message string!
char*			messageString;

// message x & y
int			messx;
int			messy;
int			messageLastMenuActive;

// timed message = no input from user
boolean			messageNeedsInput;

void			(*messageRoutine)(int response);

char gammamsg[5][26] =
{
    GAMMALVL0,
    GAMMALVL1,
    GAMMALVL2,
    GAMMALVL3,
    GAMMALVL4
};

// we are going to be entering a savegame string
int			saveStringEnter;              
int             	saveSlot;	// which slot to save in
int			saveCharIndex;	// which char we're editing
// old save description before edit
char			saveOldString[SAVESTRINGSIZE];  

boolean                 inhelpscreens;
boolean                 menuactive;
boolean                 menupause;      // haleyjd 08/29/10: [STRIFE] New global
int                     menupausetime;  // haleyjd 09/04/10: [STRIFE] New global
boolean                 menuindialog;   // haleyjd 09/04/10: ditto

// haleyjd 08/27/10: [STRIFE] SKULLXOFF == -28, LINEHEIGHT == 19
#define CURSORXOFF		-28
#define LINEHEIGHT		19

#define CURSORXOFF_SMALL	-20
#define LINEHEIGHT_SMALL	10
#define FIRSTKEY_MAX		0

#define FIRSTKEYSETAMOUNT	16

extern boolean		sendpause;
char			savegamestrings[10][SAVESTRINGSIZE];

char			endstring[160];

// haleyjd 09/04/10: [STRIFE] Moved menuitem / menu structures into header
// because they are needed externally by the dialog engine.

// haleyjd 08/27/10: [STRIFE] skull* stuff changed to cursor* stuff
short			itemOn;			// menu item skull is on
short			cursorAnimCounter;	// skull animation counter
short			whichCursor;		// which skull to draw

// graphic name of cursors
// haleyjd 08/27/10: [STRIFE] M_SKULL* -> M_CURS*
char    		*cursorName[8] = {"M_CURS1", "M_CURS2", "M_CURS3", "M_CURS4", 
                          		"M_CURS5", "M_CURS6", "M_CURS7", "M_CURS8" };

char    		*cursorNameSmall[8] = {"M_CURS1S", "M_CURS2S", "M_CURS3S", "M_CURS4S", 
                          		"M_CURS5S", "M_CURS6S", "M_CURS7S", "M_CURS8S" };

// haleyjd 20110210 [STRIFE]: skill level for menus
int			menuskill;

// current menudef
menu_t*			currentMenu;

// haleyjd 03/01/13: [STRIFE] v1.31-only:
// Keeps track of whether the save game menu is being used to name a new
// character slot, or to just save the current game. In the v1.31 disassembly
// this was the new dword_8632C variable.
boolean			namingCharacter; 

//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_ReadThis3(int choice); // [STRIFE]

//void M_ChangeMessages(int choice); [STRIFE]
void M_ChangeSensitivity(int choice);
void M_ChangeDetail(int choice);

void M_GameFiles(int choice);
void M_Brightness(int choice);
void M_Freelook(int choice);
void M_FreelookSpeed(int choice);
void M_KeyBindingsClearControls (int key);
void M_KeyBindingsClearAll (int choice);
void M_KeyBindingsReset (int choice);
void M_KeyBindingsButtonLayout(int choice);
void M_KeyBindingsSetKey(int choice);
void M_KeyBindings(int choice);
void M_CpuSpeed(int choice);
void M_FPS(int choice);
void M_DisplayTicker(int choice);
void M_FrameLimiter(int choice);
void M_Battery(int choice);
void M_CPU(int choice);
void M_Coordinates(int choice);
void M_Other(int choice);
void M_ShowMemory(int choice);
void M_MapGrid(int choice);
void M_MapRotation(int choice);
void M_FollowMode(int choice);
void M_Statistics(int choice);
void M_Messages(int choice);
void M_DialogText(int choice);
void M_Crosshair(int choice);

void M_SfxVol(int choice);
void M_Voices(int choice);
void M_VoiceVol(int choice); // [STRIFE]
void M_MusicVol(int choice);
void M_God(int choice);
void M_Noclip(int choice);
void M_Weapons(int choice);
void M_Keys(int choice);
void M_Artifacts(int choice);
void M_Stonecold(int choice);
void M_Lego(int choice);
void M_Pumpup(int choice);
void M_Topo(int choice);
//void M_GPS(int choice);
void M_Gripper(int choice);
//void M_Dots(int choice);
void M_Rift(int choice);
void M_RiftNow(int choice);
void M_Spin(int choice);
/*
void M_Scoot(int choice);
void M_ScootNow(int choice);
*/
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Screen(int choice);
void M_FPSCounter(int display_fps);
void M_Controls(int choice);
void M_System(int choice);
void M_Sound(int choice);
void M_Game(int choice);
void M_Debug(int choice);
void M_Cheats(int choice);

//void M_FinishReadThis(int choice); - [STRIFE] unused
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawFilesMenu(void);
void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawReadThis3(void); // [STRIFE]
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);

void M_DrawScreen(void);
void M_DrawKeyBindings(void);
void M_DrawControls(void);
void M_DrawSystem(void);
void M_DrawGame(void);
void M_DrawDebug(void);

void M_DrawSound(void);
void M_DrawCheats(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
int  M_StringWidth(char *string);
int  M_StringHeight(char *string);
void M_StartMessage(char *string,void *routine,boolean input);
void M_StopMessage(void);


char *maptext[] =			// ADDED FOR PSP
{
	" ",
	"SANCTUARY",
	"TOWN",
	"FRONT BASE",
	"POWER STATION",
	"PRISON",
	"SEWERS",
	"CASTLE",
	"AUDIENCE CHAMBER",
	"CASTLE: PROGRAMMER'S KEEP",
	"NEW FRONT BASE",
	"BORDERLANDS",
	"THE TEMPLE OF THE ORACLE",
	"CATACOMBS",
	"MINES",
	"FORTRESS: ADMINISTRATION",
	"FORTRESS: BISHOP'S TOWER",
	"FORTRESS: THE BAILEY",
	"FORTRESS: STORES",
	"FORTRESS: SECURITY COMPLEX",
	"FACTORY: RECEIVING",
	"FACTORY: MANUFACTURING",
	"FACTORY: FORGE",
	"ORDER COMMONS",
	"FACTORY: CONVERSION CHAPEL",
	"CATACOMBS: RUINED TEMPLE",
	"PROVING GROUNDS",
	"THE LAB",
	"ALIEN SHIP",
	"ENTITY'S LAIR",
	"ABANDONED FRONT BASE",
	"TRAINING FACILITY",
//					DEMO ADDITIONS ???
	"SANCTUARY (DEMO VER.)",
	"TOWN (DEMO VER.)",
	"MOVEMENT BASE (DEMO VER.)",
};

char *mustext[] =			// ADDED FOR PSP
{
	" ",
	"TITLE MUSIC",
	"SANCTUARY",
	"TOWN",
	"FRONT BASE",
	"POWER STATION",
	"PRISON",
	"SEWERS",
	"CASTLE",
	"AUDIENCE CHAMBER",
	"CASTLE: PROGRAMMER'S KEEP",
	"NEW FRONT BASE",
	"BORDERLANDS",
	"THE TEMPLE OF THE ORACLE",
	"CATACOMBS",
	"MINES",
	"FORTRESS: ADMINISTRATION",
	"FORTRESS: BISHOP'S TOWER",
	"FORTRESS: THE BAILEY",
	"FORTRESS: STORES",
	"FORTRESS: SECURITY COMPLEX",
	"FACTORY: RECEIVING",
	"FACTORY: MANUFACTURING",
	"FACTORY: FORGE",
	"ORDER COMMONS",
	"FACTORY: CONVERSION CHAPEL",
	"CATACOMBS: RUINED TEMPLE",
	"PROVING GROUNDS",
	"THE LAB",
	"ALIEN SHIP",
	"ENTITY'S LAIR",
	"ABANDONED FRONT BASE",
	"TRAINING FACILITY",
	"UNKNOWN MUSIC TITLE #1",	// LEFTOVERS FROM ROGUE???
	"UNKNOWN MUSIC TITLE #2",	// LEFTOVERS FROM ROGUE???
};

char *spottext[] =			// FOR PSP (BUT DOESN'T WORK (YET))
{
	" ",
	"01",
	"02",
	"03",
	"04",
	"05",
	"06",
	"07",
	"08",
	"09",
	"10",
};


char *stupidtable[] =
{
    "A","B","C","D","E",
    "F","G","H","I","J",
    "K","L","M","N","O",
    "P","Q","R","S","T",
    "U","V","W","X","Y",
    "Z"
};

char *Key2String (int key)
{
/* S.A.: return "[" or "]" or "\"" doesn't work
 * because there are no lumps for these chars,
 * therefore we have to go with "RIGHT BRACKET"
 * and similar for much punctuation.  Probably
 * won't work with international keyboards and
 * dead keys, either.
 */
    switch (key)
    {
	case KEY_UPARROW:	return "UP ARROW";
	case KEY_DOWNARROW:	return "DOWN ARROW";
	case KEY_LEFTARROW:	return "LEFT ARROW";
	case KEY_RIGHTARROW:	return "RIGHT ARROW";
	case KEY_TRIANGLE:	return "TRIANGLE";
	case KEY_CROSS:		return "CROSS";
	case KEY_SQUARE:	return "SQUARE";
	case KEY_CIRCLE:	return "CIRCLE";
	case KEY_SELECT:	return "SELECT";
	case KEY_START:		return "START";
	case KEY_LEFTTRIGGER:	return "LEFT TRIGGER";
	case KEY_RIGHTTRIGGER:	return "RIGHT TRIGGER";
    }
    /* Handle letter keys */
    /* S.A.: could also be done with toupper */
    if (key >= 'a' && key <= 'z')
	return stupidtable[(key - 'a')];

    return "?";		/* Everything else */
}

//
// DOOM MENU
//
enum
{
    newgame = 0,
    options,
/*
    loadgame,
    savegame,
*/
    gamefiles,
    readthis,
    quitdoom,
    main_end
} main_e;

menuitem_t MainMenu[]=
{
    {1,"M_NGAME",M_NewGame,'n'},
    {1,"M_OPTION",M_Options,'o'},
/*
    {1,"M_LOADG",M_LoadGame,'l'},
    {1,"M_SAVEG",M_SaveGame,'s'},
*/
    {1,"M_GFILES",M_GameFiles,'f'},
    // Another hickup with Special edition.
    {1,"M_RDTHIS",M_ReadThis,'h'}, // haleyjd 08/28/10: 'r' -> 'h'
    {1,"M_QUITG",M_QuitStrife,'q'}
};

menu_t  MainDef =
{
    main_end,
    NULL,
    MainMenu,
    M_DrawMainMenu,
    97,55, // haleyjd 08/28/10: [STRIFE] changed y coord
    0
};

enum
{
    loadgame,
    savegame,
    endgame,
    cheats,
    files_end
} files_e;

menuitem_t FilesMenu[]=
{
    {1,"M_LOADG",M_LoadGame,'l'},
    {1,"M_SAVEG",M_SaveGame,'s'},
    {1,"M_ENDGAM",M_EndGame,'e'},
    {1,"M_CHEATS",M_Cheats,'c'}
};

menu_t  FilesDef =
{
    files_end,
    NULL,
    FilesMenu,
    M_DrawFilesMenu,
    97,45, // haleyjd 08/28/10: [STRIFE] changed y coord
    0
};

//
// EPISODE SELECT
//
/*
enum
{
    ep1,
    ep2,
    ep3,
    ep4,
    ep_end
} episodes_e;

menuitem_t EpisodeMenu[]=
{
    {1,"M_EPI1", M_Episode,'k'},
    {1,"M_EPI2", M_Episode,'t'},
    {1,"M_EPI3", M_Episode,'i'},
    {1,"M_EPI4", M_Episode,'t'}
};

menu_t  EpiDef =
{
    ep_end,		// # of menu items
    &MainDef,		// previous menu
    EpisodeMenu,	// menuitem_t ->
    M_DrawEpisode,	// drawing routine ->
    48,63,              // x,y
    ep1			// lastOn
};
*/

//
// NEW GAME
//
enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end
} newgame_e;

menuitem_t NewGameMenu[]=
{
    // haleyjd 08/28/10: [STRIFE] changed all shortcut letters
    {1,"M_JKILL",   M_ChooseSkill, 't'},
    {1,"M_ROUGH",   M_ChooseSkill, 'r'},
    {1,"M_HURT",    M_ChooseSkill, 'v'},
    {1,"M_ULTRA",   M_ChooseSkill, 'e'},
    {1,"M_NMARE",   M_ChooseSkill, 'b'}
};

menu_t  NewDef =
{
    newg_end,           // # of menu items
    &MainDef,           // previous menu - haleyjd [STRIFE] changed to MainDef
    NewGameMenu,        // menuitem_t ->
    M_DrawNewGame,      // drawing routine ->
    48,63,              // x,y
    toorough            // lastOn - haleyjd [STRIFE]: default to skill 1
};

//
// OPTIONS MENU
//
enum
{
/*
    // haleyjd 08/28/10: [STRIFE] Removed messages, mouse sens., detail.
    endgame,
    scrnsize,
    option_empty1,
    soundvol,
    cheats,
*/
    screen,
    controls,
    sound,
    sys,
    game,
    debug,
    opt_end
} options_e;

menuitem_t OptionsMenu[]=
{
    {1,"M_SCRSET",	M_Screen,'s'},
    {1,"M_CTLSET",	M_Controls,'c'},
    {1,"M_SNDSET",	M_Sound,'v'},
    {1,"M_SYSSET",	M_System,'y'},
    {1,"M_GMESET",	M_Game,'g'},
    {1,"M_DBGSET",	M_Debug,'d'}
/*
    // haleyjd 08/28/10: [STRIFE] Removed messages, mouse sens., detail.
    {1,"M_ENDGAM",	M_EndGame,'e'},
    {2,"M_SCRNSZ",	M_SizeDisplay,'s'},
    {-1,"",0,'\0'},
    {1,"M_SVOL",	M_Sound,'s'},
    {1,"M_CHEATS",	M_Cheats,'c'}
*/
};

menu_t  OptionsDef =
{
    opt_end,
    &MainDef,
    OptionsMenu,
    M_DrawOptions,
    60,57,
    0
};

//
// Read This! MENU 1 & 2 & [STRIFE] 3
//
enum
{
    rdthsempty1,
    read1_end
} read_e;

menuitem_t ReadMenu1[] =
{
    {1,"",M_ReadThis2,0}
};

menu_t  ReadDef1 =
{
    read1_end,
    &MainDef,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
    0
};

enum
{
    rdthsempty2,
    read2_end
} read_e2;

menuitem_t ReadMenu2[]=
{
    {1,"",M_ReadThis3,0} // haleyjd 08/28/10: [STRIFE] Go to ReadThis3
};

menu_t  ReadDef2 =
{
    read2_end,
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    250,185, // haleyjd 08/28/10: [STRIFE] changed coords
    0
};

// haleyjd 08/28/10: Added Read This! menu 3
enum
{
    rdthsempty3,
    read3_end
} read_e3;

menuitem_t ReadMenu3[]=
{
    {1,"",M_ClearMenus,0}
};

menu_t  ReadDef3 =
{
    read3_end,
    &ReadDef2,
    ReadMenu3,
    M_DrawReadThis3,
    250, 185,
    0
};

enum
{
    gamma,
    screen_empty1,
    scrnsize,
    screen_empty2,
    screen_detail,
    screen_end
} screen_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t ScreenMenu[]=
{
    {2,"M_BRGTNS",M_Brightness,'b'},
    {-1,"",0,'\0'},
    {2,"M_SCRNSZ",M_SizeDisplay,'s'},
    {-1,"",0,'\0'},
    {2,"M_DETAIL",M_ChangeDetail,'d'}
};

menu_t  ScreenDef =
{
    screen_end,
    &OptionsDef,
    ScreenMenu,
    M_DrawScreen,
    80,55,       // [STRIFE] changed y coord 64 -> 35
    0
};

enum
{
    keybindings_up,
    keybindings_down,
    keybindings_left,
    keybindings_right,
    keybindings_triangle,
    keybindings_cross,
    keybindings_square,
    keybindings_circle,
    keybindings_select,
    keybindings_start,
    keybindings_lefttrigger,
    keybindings_righttrigger,
    keybindings_empty,
    keybindings_layout,
    keybindings_clearall,
    keybindings_reset,
    keybindings_end
} keybindings_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t KeyBindingsMenu[]=
{
    {5,"",M_KeyBindingsSetKey,0},
    {5,"",M_KeyBindingsSetKey,1},
    {5,"",M_KeyBindingsSetKey,2},
    {5,"",M_KeyBindingsSetKey,3},
    {5,"",M_KeyBindingsSetKey,4},
    {5,"",M_KeyBindingsSetKey,5},
    {5,"",M_KeyBindingsSetKey,6},
    {5,"",M_KeyBindingsSetKey,7},
    {5,"",M_KeyBindingsSetKey,8},
    {5,"",M_KeyBindingsSetKey,9},
    {5,"",M_KeyBindingsSetKey,10},
    {5,"",M_KeyBindingsSetKey,11},
    {-1,"",0,'\0'},
    {2,"",M_KeyBindingsButtonLayout,'l'},
    {5,"",M_KeyBindingsClearAll,'c'},
    {5,"",M_KeyBindingsReset,'r'}
};

menu_t  KeyBindingsDef =
{
    keybindings_end,
    &OptionsDef,
    KeyBindingsMenu,
    M_DrawKeyBindings,
    45,40,       // [STRIFE] changed y coord 64 -> 35
    0
};

enum
{
    mousesensi,
    controls_empty1,
    controls_freelook,
    mousespeed,
    controls_empty2,
    controls_keybindings,
    controls_end
} controls_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t ControlsMenu[]=
{
    {2,"M_MVESPD",M_ChangeSensitivity,'m'},
    {-1,"",0,'\0'},
    {2,"M_FRLOOK",M_Freelook,'f'},
    {2,"M_FLKSPD",M_FreelookSpeed,'s'},
    {-1,"",0,'\0'},
    {1,"M_KBNDGS",M_KeyBindings,'b'}
};

menu_t  ControlsDef =
{
    controls_end,
    &OptionsDef,
    ControlsMenu,
    M_DrawControls,
    55,40,       // [STRIFE] changed y coord 64 -> 35
    0
};

enum
{
    system_cpuspeed,
    system_fps,
    system_ticker,
    system_framelimiter,
    system_end
} system_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t SystemMenu[]=
{
    {2,"M_CPUSPD",M_CpuSpeed,'c'},
    {2,"M_FRAMES",M_FPS,'f'},
    {2,"M_TICKER",M_DisplayTicker,'t'},
    {2,"M_FLIMIT",M_FrameLimiter,'l'}
};

menu_t  SystemDef =
{
    system_end,
    &OptionsDef,
    SystemMenu,
    M_DrawSystem,
    40,65,       // [STRIFE] changed y coord 64 -> 35
    0
};

enum
{
    game_mapgrid,
    game_maprotation,
    game_followmode,
    game_statistics,
    game_empty1,
    game_messages,
    game_dialogtext,
    game_crosshair,
    game_end
} game_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t GameMenu[]=
{
    {2,"M_MAPGRD",M_MapGrid,'g'},
    {2,"M_MAPROT",M_MapRotation,'r'},
    {2,"M_FLWMDE",M_FollowMode,'f'},
    {2,"M_AMSTAT",M_Statistics,'s'},
    {-1,"",0,'\0'},
    {2,"",M_Messages,'m'},
    {2,"M_DLGTXT",M_DialogText,'t'},
    {2,"M_XHAIR",M_Crosshair,'c'}
};

menu_t  GameDef =
{
    game_end,
    &OptionsDef,
    GameMenu,
    M_DrawGame,
    45,45,       // [STRIFE] changed y coord 64 -> 35
    0
};

enum
{
    debug_battery,
    debug_cpu,
    debug_memory,
    debug_coordinates,
    debug_other,
    debug_end
} debug_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t DebugMenu[]=
{
    {2,"M_BATNFO",M_Battery,'b'},
    {2,"M_CPUNFO",M_CPU,'p'},
    {2,"M_MEMORY",M_ShowMemory,'m'},
    {2,"M_CRDNTS",M_Coordinates,'c'},
    {2,"M_OTHNFO",M_Other,'o'}
};

menu_t  DebugDef =
{
    debug_end,
    &OptionsDef,
    DebugMenu,
    M_DrawDebug,
    45,55,       // [STRIFE] changed y coord 64 -> 35
    0
};

//
// SOUND VOLUME MENU
//
enum
{
    sfx_vol,
    sfx_empty1,
    music_vol,
    sfx_empty2,
    sfx_voices,
    voice_vol,
    sfx_empty3,
/*
    sfx_mouse,
    sfx_empty4,
*/
    sound_end
} sound_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t SoundMenu[]=
{
    {2,"M_SFXVOL",M_SfxVol,'s'},
    {-1,"",0,'\0'},
    {2,"M_MUSVOL",M_MusicVol,'m'},
    {-1,"",0,'\0'},
    {2,"M_VOICES",M_Voices,'d'}, 
    {2,"M_VOIVOL",M_VoiceVol,'v'}, 
    {-1,"",0,'\0'}
/*
    {2,"M_MSENS",M_ChangeSensitivity,'m'},
    {-1,"",0,'\0'}
*/
};

menu_t  SoundDef =
{
    sound_end,
    &OptionsDef,
    SoundMenu,
    M_DrawSound,
    80,50,       // [STRIFE] changed y coord 64 -> 35
    0
};

//
// CHEATS MENU
//
enum
{
    cheats_god,
    cheats_noclip,
    cheats_weapons,
    cheats_keys,
    cheats_artifacts,
    cheats_stonecold,
    cheats_lego,
    cheats_pumpup,
    cheats_topo,
//    cheats_gps,
    cheats_gripper,
//    cheats_dots,
    cheats_empty1,
    cheats_rift,
    cheats_empty2,
    cheats_riftnow,
    cheats_empty3,
/*
    cheats_scoot,
    cheats_scootnow,
*/
    cheats_spin,
    cheats_end
} cheats_e;

menuitem_t CheatsMenu[]=
{
    {2,"",M_God,'g'},
    {2,"",M_Noclip,'n'},
    {2,"",M_Weapons,'w'}, 
    {2,"",M_Keys,'k'},
    {2,"",M_Artifacts,'a'},
    {2,"",M_Stonecold,'c'},
    {2,"",M_Lego,'l'},
    {2,"",M_Pumpup,'p'},
    {2,"",M_Topo,'t'},
//    {2,"",M_GPS,'c'},
    {2,"",M_Gripper,'b'},
//    {2,"",M_Dots,'d'},
    {-1,"",0,'\0'},
    {2,"",M_Rift,'r'},
    {-1,"",0,'\0'},
    {2,"",M_RiftNow,'e'},
    {-1,"",0,'\0'},
/*
    {2,"",M_Scoot,'j'},
    {2,"",M_ScootNow,'s'},
*/
    {2,"",M_Spin,'m'}
};

menu_t  CheatsDef =
{
    cheats_end,
    &OptionsDef,
    CheatsMenu,
    M_DrawCheats,
    80,30,       // [STRIFE] changed y coord 64 -> 35
    0
};

//
// LOAD GAME MENU
//
enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load_end
} load_e;

menuitem_t LoadMenu[]=
{
    {1,"", M_LoadSelect,'1'},
    {1,"", M_LoadSelect,'2'},
    {1,"", M_LoadSelect,'3'},
    {1,"", M_LoadSelect,'4'},
    {1,"", M_LoadSelect,'5'},
    {1,"", M_LoadSelect,'6'}
};

menu_t  LoadDef =
{
    load_end,
    &MainDef,
    LoadMenu,
    M_DrawLoad,
    80,54,
    0
};

//
// SAVE GAME MENU
//
menuitem_t SaveMenu[]=
{
    {1,"", M_SaveSelect,'1'},
    {1,"", M_SaveSelect,'2'},
    {1,"", M_SaveSelect,'3'},
    {1,"", M_SaveSelect,'4'},
    {1,"", M_SaveSelect,'5'},
    {1,"", M_SaveSelect,'6'}
};

menu_t  SaveDef =
{
    load_end,
    &MainDef,
    SaveMenu,
    M_DrawSave,
    80,54,
    0
};

void M_DrawNameChar(void);

//
// NAME CHARACTER MENU
//
// [STRIFE]
// haleyjd 20110210: New "Name Your Character" Menu
//
menu_t NameCharDef =
{
    load_end,
    &NewDef,
    SaveMenu,
    M_DrawNameChar,
    80,54,
    0
};

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
// [STRIFE]
// haleyjd 20110210: Rewritten to read "name" file in each slot directory
//
void M_ReadSaveStrings(void)
{
    FILE *handle;
    int   i;
    char *fname = NULL;

    for(i = 0; i < load_end; i++)
    {
        if(fname)
            Z_Free(fname);
        fname = M_SafeFilePath(savegamedir, M_MakeStrifeSaveDir(i, "\\name"));

		char flname[256];
		sprintf(flname, "ux0:/data/vitaStrife/%s", fname);
		
        handle = fopen(flname, "rb");
        if(handle == NULL)
        {
            M_StringCopy(savegamestrings[i], EMPTYSTRING,
                         sizeof(savegamestrings[i]));
            LoadMenu[i].status = 0;
            continue;
        }
        fread(savegamestrings[i], 1, SAVESTRINGSIZE, handle);
        fclose(handle);
        LoadMenu[i].status = 1;
    }

    if(fname)
        Z_Free(fname);
}

//
// M_DrawNameChar
//
// haleyjd 09/22/10: [STRIFE] New function
// Handler for drawing the "Name Your Character" menu.
//
void M_DrawNameChar(void)
{
    int i;

    M_WriteText(72, 28, DEH_String("Name Your Character"));

    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    if (saveStringEnter)
    {
        i = M_StringWidth(savegamestrings[quickSaveSlot]);
        M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*quickSaveSlot,"_");
    }
}

//
// M_DoNameChar
//
// haleyjd 09/22/10: [STRIFE] New function
// Handler for items in the "Name Your Character" menu.
//
void M_DoNameChar(int choice)
{
    int map;

    // 20130301: clear naming character flag for 1.31 save logic
    if(gameversion == exe_strife_1_31)
        namingCharacter = false;
    sendsave = 1;
    ClearTmp();
    G_WriteSaveName(choice, savegamestrings[choice]);
    quickSaveSlot = choice;  
    SaveDef.lastOn = choice;
    ClearSlot();
    FromCurr();
    
    if(isdemoversion)
        map = 33;
    else
        map = 2;

    G_DeferedInitNew(menuskill, map);
    M_ClearMenus(0);
}

//
// M_LoadGame & Cie.
//
void M_DrawLoad(void)
{
    int             i;

    V_DrawPatch(72, 28, 0, 
                      W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE));

    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }
}



//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x,int y)
{
    int             i;

    V_DrawPatch(x - 8, y + 7, 0,
                      W_CacheLumpName(DEH_String("M_LSLEFT"), PU_CACHE));

    for (i = 0;i < 24;i++)
    {
        V_DrawPatch(x, y + 7, 0,
                          W_CacheLumpName(DEH_String("M_LSCNTR"), PU_CACHE));
        x += 8;
    }

    V_DrawPatch(x, y + 7, 0,
                      W_CacheLumpName(DEH_String("M_LSRGHT"), PU_CACHE));
}



//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    // [STRIFE]: completely rewritten
    char *name = NULL;

    G_WriteSaveName(choice, savegamestrings[choice]);
    ToCurr();

    // use safe & portable filepath concatenation for Choco
    name = M_SafeFilePath(savegamedir, M_MakeStrifeSaveDir(choice, ""));

    G_ReadCurrent(name);
    quickSaveSlot = choice;
    M_ClearMenus(0);

    Z_Free(name);
}

//
// Selected from DOOM menu
//
// [STRIFE] Verified unmodified
//
void M_LoadGame (int choice)
{
/*
    if (netgame)
    {
        M_StartMessage(DEH_String(LOADNET), NULL, false);
        return;
    }
*/
    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
    int             i;

    V_DrawPatch(72, 28, 0, W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE));
    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    if (saveStringEnter)
    {
        i = M_StringWidth(savegamestrings[quickSaveSlot]);
        M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*quickSaveSlot,"_");
    }
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    // [STRIFE]: completely rewritten
    if(slot >= 0)
    {
        sendsave = 1;
        G_WriteSaveName(slot, savegamestrings[slot]);
        M_ClearMenus(0);
        quickSaveSlot = slot;        
        // haleyjd 20130922: slight divergence. We clear the destination slot 
        // of files here, which vanilla did not do. As a result, 1.31 had 
        // broken save behavior to the point of unusability. fraggle agrees 
        // this is detrimental enough to be fixed - unconditionally, for now.
        ClearSlot();        
        FromCurr();
    }
    else
        M_StartMessage(DEH_String(QSAVESPOT), NULL, false);
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    int realmap;

    if(gamemap == 0)
	realmap = gamemap + 2;
    else
	realmap = gamemap;

    // we are going to be intercepting all chars
    saveStringEnter = 1;

    // [STRIFE]
    quickSaveSlot = choice;
    //saveSlot = choice;

    SceDateTime savegame_time;				// FOR PSP: always save a new savegame name

    sceRtcGetCurrentClockLocalTime(&savegame_time);
	

//    sprintf(savegamestrings[choice], "map%2.2d s%d %d/%d/%d %2.2d:%2.2d",
    sprintf(savegamestrings[choice], "map%2.2d %d/%d/%d %2.2d:%2.2d",
	realmap,
//	gameskill+1,
	savegame_time.year,
	savegame_time.month,
	savegame_time.day,
	savegame_time.hour,
	savegame_time.minute);

    M_StringCopy(saveOldString, savegamestrings[choice], sizeof(saveOldString));
    if (!strcmp(savegamestrings[choice],EMPTYSTRING))
        savegamestrings[choice][0] = 0;
    saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
/*
    // [STRIFE]
    if (netgame)
    {
        // haleyjd 20110211: Hooray for Rogue's awesome multiplayer support...
        M_StartMessage(DEH_String("You can't save a netgame"), NULL, false);
        return;
    }
*/
    if (!usergame)
    {
        M_StartMessage(DEH_String(SAVEDEAD),NULL,false);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    // [STRIFE]
    if(gameversion == exe_strife_1_31)
    {
        // haleyjd 20130301: in 1.31, we can choose a slot again.
        M_SetupNextMenu(&SaveDef);
        M_ReadSaveStrings();
    }
    else
    {
        // In 1.2 and lower, you save over your character slot exclusively
        M_ReadSaveStrings();
        M_DoSave(quickSaveSlot);
    }
}



//
//      M_QuickSave
//
char    tempstring[80];

void M_QuickSaveResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_DoSave(quickSaveSlot);
        S_StartSound(NULL, sfx_mtalht); // [STRIFE] sound
    }
}

void M_QuickSave(void)
{
/*
    if (netgame)
    {
        // haleyjd 20110211 [STRIFE]: More fun...
        M_StartMessage(DEH_String("You can't save a netgame"), NULL, false);
        return;
    }
*/
    if (!usergame)
    {
        S_StartSound(NULL, sfx_oof);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    if (quickSaveSlot < 0)
    {
        M_StartControlPanel();
        M_ReadSaveStrings();
        M_SetupNextMenu(&SaveDef);
        quickSaveSlot = -2;	// means to pick a slot now
        return;
    }
    DEH_snprintf(tempstring, 80, QSPROMPT, savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickSaveResponse,true);
}



//
// M_QuickLoadResponse
//
void M_QuickLoadResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_LoadSelect(quickSaveSlot);
        S_StartSound(NULL, sfx_mtalht); // [STRIFE] sound
    }
}

//
// M_QuickLoad
//
// [STRIFE] Verified unmodified
//
void M_QuickLoad(void)
{
/*
    if (netgame)
    {
        M_StartMessage(DEH_String(QLOADNET),NULL,false);
        return;
    }
*/
    if (quickSaveSlot < 0)
    {
        M_StartMessage(DEH_String(QSAVESPOT),NULL,false);
        return;
    }
    DEH_snprintf(tempstring, 80, QLPROMPT, savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickLoadResponse,true);
}




//
// Read This Menus
// Had a "quick hack to fix romero bug"
// haleyjd 08/28/10: [STRIFE] Draw HELP1, unconditionally.
//
void M_DrawReadThis1(void)
{
    inhelpscreens = true;

    V_DrawPatch (0, 0, 0, W_CacheLumpName(DEH_String("HELP1"), PU_CACHE));
}



//
// Read This Menus
// haleyjd 08/28/10: [STRIFE] Not optional, draws HELP2
//
void M_DrawReadThis2(void)
{
    inhelpscreens = true;

    V_DrawPatch(0, 0, 0, W_CacheLumpName(DEH_String("HELP2"), PU_CACHE));
}


//
// Read This Menus
// haleyjd 08/28/10: [STRIFE] New function to draw HELP3.
//
void M_DrawReadThis3(void)
{
    inhelpscreens = true;
    
    V_DrawPatch(0, 0, 0, W_CacheLumpName(DEH_String("HELP3"), PU_CACHE));
}

void M_GameFiles(int choice)
{
    M_SetupNextMenu(&FilesDef);
}

void M_DrawFilesMenu(void)
{
}

void M_Brightness(int choice)
{
    switch(choice)
    {
    case 0:
        if (usegamma)
            usegamma--;
        break;
    case 1:
        if (usegamma < 4)
            usegamma++;
        break;
    }
    players[consoleplayer].message = DEH_String(gammamsg[usegamma]);
    I_SetPalette (W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE));
}

void M_Screen(int choice)
{
    M_SetupNextMenu(&ScreenDef);
}

void M_DrawScreen(void)
{
    // haleyjd 08/27/10: [STRIFE] M_OPTTTL -> M_OPTION
    V_DrawPatch(60, 15, 0, 
                      W_CacheLumpName(DEH_String("M_SCRSET"), PU_CACHE));

    // haleyjd 08/26/10: [STRIFE] Removed messages, sensitivity, detail.

    M_DrawThermo(OptionsDef.x+62,OptionsDef.y+LINEHEIGHT*(gamma+1),
                 5,usegamma);
    M_DrawThermo(OptionsDef.x+45,OptionsDef.y+LINEHEIGHT*(scrnsize+1),
                 9,screenSize);

    if(detailLevel)
	V_DrawPatch (180, 134, 0, W_CacheLumpName(DEH_String("M_LOW"), PU_CACHE));
    else
	V_DrawPatch (180, 135, 0, W_CacheLumpName(DEH_String("M_HIGH"), PU_CACHE));
}

void M_KeyBindingsButtonLayout(int choice)
{
    switch(choice)
    {
    case 0:
        if (button_layout)
            button_layout--;
        break;
    case 1:
        if (button_layout < 1)
            button_layout++;
        break;
    }
}

void M_KeyBindingsSetKey(int choice)
{
    askforkey = true;
    keyaskedfor = choice;
/*
    if (!netgame && !demoplayback)
    {
	paused = true;
    }
*/
}

void M_KeyBindingsClearControls (int key)	// XXX (FOR PSP): NOW THIS IS RATHER IMPORTANT: IF...
{						// ...THE CONFIG VARIABLES IN THIS SOURCE EVER GET...
    int i;					// ...SOMEWHAT REARRANGED, THEN IT'S IMPORTANT TO...
						// ...CHANGE THE START- & END-POS INTEGERS AS WELL...
						// ...TO THEIR NEW CORRESPONDING POSITIONS OR ELSE...
						// ...THE KEY BINDINGS MENU WILL BE VERY BUGGY!!!

    for (i = key_controls_start_in_cfg_at_pos; i < key_controls_end_in_cfg_at_pos; i++)
    {
	if (*doom_defaults_list[i].location == key)
	    *doom_defaults_list[i].location = 0;
    }
}

void M_KeyBindingsClearAll (int choice)
{
/*						// (PSP): USED TO BE FOR SET CFG VARS 'TICKER' & 'FPS',
						// BUT CAUSES A CRASH AS SOON AS THE SCREEN WIPES
    *doom_defaults_list[20].location = 0;
    *doom_defaults_list[21].location = 0;
    *doom_defaults_list[22].location = 0;
    *doom_defaults_list[23].location = 0;
    *doom_defaults_list[24].location = 0;
    *doom_defaults_list[25].location = 0;
    *doom_defaults_list[26].location = 0;
    *doom_defaults_list[27].location = 0;
    *doom_defaults_list[28].location = 0;
    *doom_defaults_list[29].location = 0;
    *doom_defaults_list[20].location = 0;
    *doom_defaults_list[31].location = 0;
*/
    *doom_defaults_list[19].location = 0;
    *doom_defaults_list[20].location = 0;
    *doom_defaults_list[21].location = 0;
    *doom_defaults_list[22].location = 0;
    *doom_defaults_list[23].location = 0;
    *doom_defaults_list[24].location = 0;
    *doom_defaults_list[25].location = 0;
    *doom_defaults_list[26].location = 0;
    *doom_defaults_list[27].location = 0;
    *doom_defaults_list[28].location = 0;
    *doom_defaults_list[29].location = 0;
    *doom_defaults_list[30].location = 0;
}

void M_KeyBindingsReset (int choice)
{
/*						// (PSP): USED TO BE FOR SET CFG VARS 'TICKER' & 'FPS',
						// BUT CAUSES A CRASH AS SOON AS THE SCREEN WIPES
    *doom_defaults_list[20].location = 173;
    *doom_defaults_list[21].location = 9;
    *doom_defaults_list[22].location = 91;
    *doom_defaults_list[23].location = 93;
    *doom_defaults_list[24].location = 175;
    *doom_defaults_list[25].location = 157;
    *doom_defaults_list[26].location = 47;
    *doom_defaults_list[27].location = 32;
    *doom_defaults_list[28].location = 27;
    *doom_defaults_list[29].location = 13;
    *doom_defaults_list[30].location = 44;
    *doom_defaults_list[31].location = 46;
*/
    *doom_defaults_list[19].location = 173;
    *doom_defaults_list[20].location = 9;
    *doom_defaults_list[21].location = 91;
    *doom_defaults_list[22].location = 93;
    *doom_defaults_list[23].location = 175;
    *doom_defaults_list[24].location = 157;
    *doom_defaults_list[25].location = 47;
    *doom_defaults_list[26].location = 32;
    *doom_defaults_list[27].location = 27;
    *doom_defaults_list[28].location = 13;
    *doom_defaults_list[29].location = 44;
    *doom_defaults_list[30].location = 46;
}

void M_DrawKeyBindings(void)
{
    int i;

    V_DrawPatch (80, 15, 0, W_CacheLumpName(DEH_String("M_KBNDGS"), PU_CACHE));

    M_WriteText(40, 40, DEH_String("MOVE FORWARDS"));
    M_WriteText(40, 50, DEH_String("MOVE BACKWARDS"));
    M_WriteText(40, 60, DEH_String("TURN LEFT"));
    M_WriteText(40, 70, DEH_String("TURN RIGHT"));
    M_WriteText(40, 80, DEH_String("INVENTORY RIGHT"));
    M_WriteText(40, 90, DEH_String("JUMP"));
    M_WriteText(40, 100, DEH_String("OBJ'S / GUNS / KEYS"));
    M_WriteText(40, 110, DEH_String("INVENTORY DROP"));
    M_WriteText(40, 120, DEH_String("MAIN MENU"));
    M_WriteText(40, 130, DEH_String("INVENTORY USE"));
    M_WriteText(40, 140, DEH_String("USE / OPEN"));
    M_WriteText(40, 150, DEH_String("FIRE"));

    M_WriteText(40, 165, DEH_String("BUTTON LAYOUT:"));

    if(button_layout == 0)
    	M_WriteText(195, 165, DEH_String("PS VITA"));
    else if(button_layout == 1)
    	M_WriteText(195, 165, DEH_String("PSP"));

    M_WriteText(40, 180, DEH_String("CLEAR ALL CONTROLS"));
    M_WriteText(40, 190, DEH_String("RESET TO DEFAULTS"));

    for (i = 0; i < 12; i++)
    {
	if (askforkey && keyaskedfor == i)
	{
	    M_WriteText(195, (i*10+40), "???");
	}
	else
	{
	    M_WriteText(195, (i*10+40),
		Key2String(*(doom_defaults_list[i+FirstKey+19].location)));
	}
    }
}

void M_Freelook(int choice)
{
    switch(choice)
    {
    case 0:
        if (mouselook)
            mouselook--;
        break;
    case 1:
        if (mouselook < 2)
            mouselook++;
        break;
    }
}

void M_FreelookSpeed(int choice)
{
    switch(choice)
    {
    case 0:
        if (mspeed)
            mspeed--;
        break;
    case 1:
        if (mspeed < 10)
            mspeed++;
        break;
    }
}

void M_KeyBindings(int choice)
{
    M_SetupNextMenu(&KeyBindingsDef);
}

void M_Controls(int choice)
{
    M_SetupNextMenu(&ControlsDef);
}

void M_DrawControls(void)
{
    V_DrawPatch (50, 15, 0, W_CacheLumpName(DEH_String("M_CTLSET"), PU_CACHE));

    if(mouselook == 0)
    V_DrawPatch(190, 80, 0, 
                      W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));
    else if(mouselook == 1)
    V_DrawPatch(190, 80, 0, 
                      W_CacheLumpName(DEH_String("M_NORMAL"), PU_CACHE));
    else if(mouselook == 2)
    V_DrawPatch(190, 80, 0, 
                      W_CacheLumpName(DEH_String("M_INVRSE"), PU_CACHE));
    M_DrawThermo(OptionsDef.x+38,OptionsDef.y-18+LINEHEIGHT*(mousesensi+1),
                 14,mouseSensitivity);
    M_DrawThermo(OptionsDef.x+48,OptionsDef.y-18+LINEHEIGHT*(mousespeed+1),
                 11,mspeed);
}

void M_CpuSpeed(int choice)
{
    mhz333 ^= 1;
    if (mhz333)
    {
	players[consoleplayer].message = DEH_String("CLOCK NOW AT 444MHZ");
	scePowerSetArmClockFrequency(444);
    }
    else
    {
	players[consoleplayer].message = DEH_String("CLOCK NOW AT 333MHZ");
	scePowerSetArmClockFrequency(333);
    }
}

void M_FPS(int choice)
{
    if(display_fps < 1)
    {
	display_fps++;
	fps_enabled = 1;
	players[consoleplayer].message = DEH_String("FPS COUNTER ON");
    }
    else if(display_fps)
    {
	display_fps--;
	fps_enabled = 0;
	players[consoleplayer].message = DEH_String("FPS COUNTER OFF");
    }
}

void M_FPSCounter(int display_fps)
{
    fps++;
	SceRtcTick t;
    sceRtcGetCurrentTick( &t );
	fpsTickNow = t.tick;
	
    if( ((fpsTickNow - fpsTickLast)/((float)tickResolution)) >= 1.0f )
    {
	fpsTickLast = fpsTickNow;
	sprintf( fpsDisplay, "FPS: %d", fps );
	fps = 0;
    }
    if(display_fps)
    {
	M_WriteText(0, 30, fpsDisplay);
    }
}

void M_DisplayTicker(int choice)
{
    display_ticker = !display_ticker;
    if (display_ticker)
    {
	dots_enabled = 1;
	players[consoleplayer].message = DEH_String("TICKER ON");
    }
    else
    {
	dots_enabled = 0;
	players[consoleplayer].message = DEH_String("TICKER OFF");
    }
}

void M_FrameLimiter(int choice)
{
    switch(choice)
    {
    case 0:
        if (frame_limit)
            frame_limit--;
        break;
    case 1:
        if (frame_limit < 3)
            frame_limit++;
        break;
    }
}

void M_Battery(int choice)
{
    switch(choice)
    {
    case 0:
        if (battery_info)
            battery_info--;
        break;
    case 1:
        if (battery_info < 1)
            battery_info++;
        break;
    }
}

void M_CPU(int choice)
{
    switch(choice)
    {
    case 0:
        if (cpu_info)
            cpu_info--;
        break;
    case 1:
        if (cpu_info < 1)
            cpu_info++;
        break;
    }
}

void M_Coordinates(int choice)
{
    switch(choice)
    {
    case 0:
        if (coordinates_info)
            coordinates_info--;
        break;
    case 1:
        if (coordinates_info < 1)
            coordinates_info++;
        break;
    }
}

void M_Other(int choice)
{
    switch(choice)
    {
    case 0:
        if (other_info)
            other_info--;
        break;
    case 1:
        if (other_info < 1)
            other_info++;
        break;
    }
}

void M_ShowMemory(int choice)
{
    switch(choice)
    {
    case 0:
        if (memory_info)
            memory_info--;
        break;
    case 1:
        if (memory_info < 1)
            memory_info++;
        break;
    }
}

void M_System(int choice)
{
    M_SetupNextMenu(&SystemDef);
}

char unit_plugged_textbuffer[50];
char battery_present_textbuffer[50];
char battery_charging_textbuffer[50];
char battery_charging_status_textbuffer[50];
char battery_low_textbuffer[50];
char battery_lifetime_percent_textbuffer[50];
char battery_lifetime_int_textbuffer[50];
char battery_temp_textbuffer[50];
char battery_voltage_textbuffer[50];
char processor_clock_textbuffer[50];
char processor_bus_textbuffer[50];
char idle_time_textbuffer[50];

void M_DrawSystem(void)
{
    V_DrawPatch (55, 20, 0, W_CacheLumpName(DEH_String("M_SYSSET"), PU_CACHE));

    if(mhz333)
    V_DrawPatch(188, 67, 0, 
                      W_CacheLumpName(DEH_String("M_333MHZ"), PU_CACHE));
    else
    V_DrawPatch(188, 67, 0, 
                      W_CacheLumpName(DEH_String("M_222MHZ"), PU_CACHE));

    if(display_fps)
	V_DrawPatch (244, 85, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    else
	V_DrawPatch (244, 85, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(display_ticker)
	V_DrawPatch (244, 106, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    else
	V_DrawPatch (244, 106, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(frame_limit == 0)
	V_DrawPatch (244, 127, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));
    else if(frame_limit == 1)
	V_DrawPatch (249, 127, 0, W_CacheLumpName(DEH_String("M_ONE"), PU_CACHE));
    else if(frame_limit == 2)
	V_DrawPatch (246, 127, 0, W_CacheLumpName(DEH_String("M_TWO"), PU_CACHE));
    else if(frame_limit == 3)
	V_DrawPatch (246, 127, 0, W_CacheLumpName(DEH_String("M_THREE"), PU_CACHE));
}

void M_MapGrid(int choice)
{
    switch(choice)
    {
    case 0:
        if (drawgrid)
            drawgrid--;
        players[consoleplayer].message = DEH_String(AMSTR_GRIDOFF);
        break;
    case 1:
        if (drawgrid < 1)
            drawgrid++;
        players[consoleplayer].message = DEH_String(AMSTR_GRIDON);
        break;
    }
}

void M_MapRotation(int choice)
{
    switch(choice)
    {
    case 0:
        if (am_rotate)
            am_rotate = false;
        players[consoleplayer].message = DEH_String(AMSTR_ROTATIONOFF);
        break;
    case 1:
        if (am_rotate == false)
            am_rotate = true;
        players[consoleplayer].message = DEH_String(AMSTR_ROTATIONON);
        break;
    }
}

void M_FollowMode(int choice)
{
    switch(choice)
    {
    case 0:
        if (followplayer)
            followplayer--;
        players[consoleplayer].message = DEH_String(AMSTR_FOLLOWOFF);
        break;
    case 1:
        if (followplayer < 1)
            followplayer++;
        players[consoleplayer].message = DEH_String(AMSTR_FOLLOWON);
        break;
    }
}

void M_Statistics(int choice)
{
    switch(choice)
    {
    case 0:
        if (show_stats)
            show_stats--;
	players[consoleplayer].message = DEH_String("LEVEL STATISTICS OFF");
        break;
    case 1:
        if (show_stats < 1)
            show_stats++;
	players[consoleplayer].message = DEH_String("LEVEL STATISTICS ON");
        break;
    }
}

void M_Messages(int choice)
{
    switch(choice)
    {
    case 0:
        if (showMessages)
            showMessages--;
	    messages_disabled = 1;
        players[consoleplayer].message = DEH_String(MSGOFF);
        break;
    case 1:
        if (showMessages < 1)
            showMessages++;
	    messages_disabled = 0;
	    dont_show = 0;
        players[consoleplayer].message = DEH_String(MSGON);
        break;
    }
}

void M_DialogText(int choice)
{
    switch(choice)
    {
    case 0:
        if (dialogshowtext)
            dialogshowtext--;
        players[consoleplayer].message = DEH_String(DLGOFF);
        break;
    case 1:
        if (dialogshowtext < 1)
            dialogshowtext++;
        players[consoleplayer].message = DEH_String(DLGON);
        break;
    }
}

void M_Crosshair(int choice)
{
    switch(choice)
    {
    case 0:
        if (crosshair)
            crosshair--;
        break;
    case 1:
        if (crosshair < 1)
            crosshair++;
        break;
    }
}

void M_Game(int choice)
{
    M_SetupNextMenu(&GameDef);
}

void M_DrawGame(void)
{
    V_DrawPatch (70, 15, 0, W_CacheLumpName(DEH_String("M_GMESET"), PU_CACHE));
//    V_DrawPatch (45, 45, 0, W_CacheLumpName(DEH_String("M_AMOPTS"), PU_CACHE));
    V_DrawPatch (47, 126, 0, W_CacheLumpName(DEH_String("M_SLASHS"), PU_CACHE));
    V_DrawPatch (45, 136, 0, W_CacheLumpName(DEH_String("M_MESSGS"), PU_CACHE));

    if(drawgrid == 1)
	V_DrawPatch (234, 45, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    else if(drawgrid == 0)
	V_DrawPatch (234, 45, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(am_rotate == true)
	V_DrawPatch (234, 65, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    else if(am_rotate == false)
	V_DrawPatch (234, 65, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(followplayer == 1)
	V_DrawPatch (234, 85, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    else if(followplayer == 0)
	V_DrawPatch (234, 85, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(show_stats == 1)
	V_DrawPatch (234, 104, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    else if (show_stats == 0)
	V_DrawPatch (234, 104, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(showMessages == 1)
	V_DrawPatch (234, 138, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    else if (showMessages == 0)
	V_DrawPatch (234, 138, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(dialogshowtext == 1)
	V_DrawPatch (234, 158, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    else if (dialogshowtext == 0)
	V_DrawPatch (234, 158, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(crosshair == 1)
	V_DrawPatch (234, 178, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    else if (crosshair == 0)
	V_DrawPatch (234, 178, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));
}

void M_Debug(int choice)
{
    M_SetupNextMenu(&DebugDef);
}

void M_DrawDebug(void)
{
    V_DrawPatch (62, 15, 0, W_CacheLumpName(DEH_String("M_DBGSET"), PU_CACHE));

    if(battery_info)
    {
	sprintf(unit_plugged_textbuffer,"Unit Is Plugged In: %d \n",scePowerIsPowerOnline());
	sprintf(battery_present_textbuffer,"Battery Is Present: %d \n",scePowerIsBatteryExist());
	sprintf(battery_charging_textbuffer,"Battery Is Charging: %d \n",scePowerIsBatteryCharging());
	sprintf(battery_charging_status_textbuffer,"Battery Charging Status: %d \n",scePowerGetBatteryChargingStatus());
	sprintf(battery_low_textbuffer,"Battery Is Low: %d \n",scePowerIsLowBattery());
	sprintf(battery_lifetime_percent_textbuffer,"Battery Lifetime (Perc.): %d \n",scePowerGetBatteryLifePercent());
	sprintf(battery_lifetime_int_textbuffer,"Battery Lifetime (Int.): %d \n",scePowerGetBatteryLifeTime());
	sprintf(battery_temp_textbuffer,"Battery Temp.: %d \n",scePowerGetBatteryTemp());
	sprintf(battery_voltage_textbuffer,"Battery Voltage: %d \n",scePowerGetBatteryVolt());

	V_DrawPatch (234, 55, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    }
    else
	V_DrawPatch (234, 55, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(cpu_info)
    {
	sprintf(processor_clock_textbuffer,"Processor Clock Freq.: %d \n",scePowerGetArmClockFrequency());
	sprintf(processor_bus_textbuffer,"Processor Bus Freq.: %d \n",scePowerGetBusClockFrequency());

	V_DrawPatch (234, 75, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    }
    else
	V_DrawPatch (234, 75, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(memory_info)
    {
    	sprintf(allocated_ram_textbuffer,"Allocated RAM: %d Bytes\n",allocated_ram_size);

    	sprintf(free_ram_textbuffer,"Curr. Free RAM: UNKNOWN\n");

    	sprintf(max_free_ram_textbuffer,"Max. Free RAM: UNKNOWN\n");

	V_DrawPatch (234, 95, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    }
    else
	V_DrawPatch (234, 95, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(coordinates_info)
	V_DrawPatch (234, 115, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    else
	V_DrawPatch (234, 115, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(other_info)
    {
	sprintf(idle_time_textbuffer,"Idle Time: UNKNOWN \n");

	V_DrawPatch (234, 135, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    }
    else
	V_DrawPatch (234, 135, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));
}

//
// Change Sfx & Music volumes
//
// haleyjd 08/29/10: [STRIFE]
// * Changed title graphic coordinates
// * Added voice volume and sensitivity sliders
//
void M_DrawSound(void)
{
    V_DrawPatch (60, 15, 0, W_CacheLumpName(DEH_String("M_SNDSET"), PU_CACHE));

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1),
                 16,sfxVolume);

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1),
                 16,musicVolume);

    if(disable_voices == 1)
	V_DrawPatch (207, 126, 0, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));
    else
	V_DrawPatch (207, 126, 0, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(voice_vol+1),
                 16,voiceVolume);
/*
    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT+10*(sfx_mouse+1),
                 16,mouseSensitivity);
*/
}

void M_Sound(int choice)
{
    M_SetupNextMenu(&SoundDef);
}

void M_SfxVol(int choice)
{
    switch(choice)
    {
    case 0:
        if (sfxVolume)
            sfxVolume--;
        break;
    case 1:
        if (sfxVolume < 15)
            sfxVolume++;
        break;
    }

    S_SetSfxVolume(sfxVolume * 8);
}

void M_Voices(int choice)
{
    switch(choice)
    {
    case 0:
        if (disable_voices)
            disable_voices--;
        break;
    case 1:
        if (disable_voices < 1 && (STRIFE_1_0_REGISTERED || STRIFE_1_X_REGISTERED))
            disable_voices++;
#ifdef SHAREWARE
	else if(STRIFE_1_0_SHAREWARE || STRIFE_1_1_SHAREWARE)	// FOR PSP: (NOT POSSIBLE IN SHAREWARE)
	    M_StartMessage(DEH_String("VOICES CAN'T BE DISABLED IN DEMO"), NULL, false);
#endif
        break;
    }
}

//
// M_VoiceVol
//
// haleyjd 08/29/10: [STRIFE] New function
// Sets voice volume level.
//
void M_VoiceVol(int choice)
{
    switch(choice)
    {
    case 0:
        if (voiceVolume)
            voiceVolume--;
        break;
    case 1:
        if (voiceVolume < 15)
            voiceVolume++;
        break;
    }

    S_SetVoiceVolume(voiceVolume * 8);
}

void M_MusicVol(int choice)
{
    switch(choice)
    {
    case 0:
        if (musicVolume)
            musicVolume--;
        break;
    case 1:
        if (musicVolume < 15)
            musicVolume++;
        break;
    }

    S_SetMusicVolume(musicVolume * 8);
}

void DetectState(void)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_DEAD)
    {
	M_StartMessage(DEH_String("CHEATING NOT ALLOWED - YOU'RE DEAD"), NULL, false);
    }
    else if(/*!netgame && */demoplayback && gamestate == GS_LEVEL
	&& players[consoleplayer].playerstate == PST_LIVE)
    {
	M_StartMessage(DEH_String("CHEATING NOT ALLOWED IN DEMO MODE"), NULL, false);
    }
    else if(/*!netgame && */demoplayback && gamestate == GS_LEVEL
	&& players[consoleplayer].playerstate == PST_DEAD)
    {
	M_StartMessage(DEH_String("CHEATING NOT ALLOWED IN DEMO MODE"), NULL, false);
    }
    else if(/*!netgame && */demoplayback && gamestate != GS_LEVEL)
    {
	M_StartMessage(DEH_String("CHEATING NOT ALLOWED IN DEMO MODE"), NULL, false);
    }
    else if(/*!netgame && */!demoplayback && gamestate != GS_LEVEL)
    {
	M_StartMessage(DEH_String("CHEATING NOT ALLOWED IN DEMO MODE"), NULL, false);
    }
/*
    else if(netgame)
    {
	M_StartMessage(DEH_String("CHEATING NOT ALLOWED FOR NET GAME"), NULL, false);
    }
*/
    if(gameskill == sk_nightmare)
    {
	M_StartMessage(DEH_String("CHEATING DISABLED - NIGHTMARE SKILL"), NULL, false);
    }
}

void M_God(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
        players[consoleplayer].cheats ^= CF_GODMODE;
	if (players[consoleplayer].cheats & CF_GODMODE)
	{
            if (players[consoleplayer].mo)
                players[consoleplayer].mo->health = 100;
            players[consoleplayer].health = deh_god_mode_health;
            players[consoleplayer].st_update = true; // [STRIFE]
	    players[consoleplayer].message = DEH_String(STSTR_DQDON);
    	}
    	else
    	{
            players[consoleplayer].st_update = true; // [STRIFE]
	    players[consoleplayer].message = DEH_String(STSTR_DQDOFF);
    	}
    }
    DetectState();
}

void M_Noclip(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
        players[consoleplayer].cheats ^= CF_NOCLIP;
    	if (players[consoleplayer].cheats & CF_NOCLIP)
    	{
	    players[consoleplayer].message = DEH_String(STSTR_NCON);
            players[consoleplayer].mo->flags |= MF_NOCLIP;
    	}
    	else
    	{
	    players[consoleplayer].message = DEH_String(STSTR_NCOFF);
	    players[consoleplayer].mo->flags &= ~MF_NOCLIP;
    	}
    }
    DetectState();
}

void M_Weapons(int choice)
{
    int i;

    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	// [STRIFE]: "BOOMSTIX" cheat for all normal weapons
    	players[consoleplayer].armorpoints = deh_idkfa_armor;
    	players[consoleplayer].armortype = deh_idkfa_armor_class;

    	for (i = 0; i < NUMWEAPONS; i++)
	    if(!isdemoversion || weaponinfo[i].availabledemo)
            	players[consoleplayer].weaponowned[i] = true;
        
    	// Takes away the Sigil, even if you already had it...
    	players[consoleplayer].weaponowned[wp_sigil] = false;

    	for (i=0;i<NUMAMMO;i++)
	    players[consoleplayer].ammo[i] = players[consoleplayer].maxammo[i];

    	players[consoleplayer].message = DEH_String(STSTR_FAADDED);
    }
    DetectState();
}

void M_Keys(int choice)
{
    int i;

    // villsa [STRIFE]: "JIMMY" cheat for all keys
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	if(players[consoleplayer].cards[FIRSTKEYSETAMOUNT - 1])
    	{
            if(players[consoleplayer].cards[NUMCARDS - 1] || isdemoversion)
            {
            	for(i = 0; i < NUMCARDS; i++)
                    players[consoleplayer].cards[i] = false;

            	players[consoleplayer].message = DEH_String("Keys removed");
            }
            else
            {
            	for(i = 0; i < NUMCARDS; i++)
                    players[consoleplayer].cards[i] = true;

            	players[consoleplayer].message = DEH_String("Even More Cheater Keys Added");
            }
    	}
    	else
    	{
            for(i = 0; i < FIRSTKEYSETAMOUNT; i++)
            	players[consoleplayer].cards[i] = true;

            players[consoleplayer].message = DEH_String("Cheater Keys Added");
    	}
    }
    DetectState();
}

void M_Artifacts(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	players[consoleplayer].message = DEH_String("YOU GOT THE MIDAS TOUCH, BABY");
    	P_GiveItemToPlayer(&players[0], SPR_HELT, MT_TOKEN_TOUGHNESS);
    }
    DetectState();
}

void M_Stonecold(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	stonecold = true;
    	players[consoleplayer].message = DEH_String("Kill 'em.  Kill 'em All");
    }
    DetectState();
}

void M_Lego(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	if(!isdemoversion)
    	{
            players[consoleplayer].st_update = true;
            if(players[consoleplayer].weaponowned[wp_sigil])
            {
    	        if(++players[consoleplayer].sigiltype > 4)
    	        {
                    players[consoleplayer].sigiltype = -1;
                    players[consoleplayer].pendingweapon = wp_fist;
                    players[consoleplayer].weaponowned[wp_sigil] = false;
                }
            }
            else
            {
                players[consoleplayer].weaponowned[wp_sigil] = true;
                players[consoleplayer].sigiltype = 0;
    	    }
    	    // BUG: This brings up a bad version of the Sigil (sigiltype -1) which
    	    // causes some VERY interesting behavior, when you type LEGO for the
    	    // sixth time. This shouldn't be done when taking it away, and yet it
    	    // is here... verified with vanilla.
//    	    players[consoleplayer].pendingweapon = wp_sigil;	// FOR PSP: DEACTIVATED TO BE ABLE... 
    	}							// ...TO GET BACK TO THE FISTS WHEN...
    }								// ...USING 'LEGO' CHEAT IN CHEAT-MENU
    DetectState();
}

void M_Pumpup(int choice)
{
    int i;

    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	for(i = 0; i < 0 + 3; ++i)
    	{
    	    // [STRIFE]: Handle berserk, invisibility, and envirosuit
//  	      if(cht_CheckCheat(&cheat_powerup[i], ev->data2))
    	    {
    	        if(players[consoleplayer].powers[i])
    	            players[consoleplayer].powers[i] = (i != 1);
    	        else
    	            P_GivePower(&players[0], i);
    	    }
    	}

    	// [STRIFE]: PUMPUPH gives medical inventory items
    	P_GiveItemToPlayer(&players[0], SPR_STMP, MT_INV_MED1);
    	P_GiveItemToPlayer(&players[0], SPR_MDKT, MT_INV_MED2);
    	P_GiveItemToPlayer(&players[0], SPR_FULL, MT_INV_MED3);

    	// [STRIFE]: PUMPUPP gives backpack
    	if(!&players[consoleplayer].backpack)
    	{
	    for(i = 0; i < NUMAMMO; ++i)
	    players[consoleplayer].maxammo[i] = 2 * players[consoleplayer].maxammo[i];
    	}
    	players[consoleplayer].backpack = true;

    	for(i = 0; i < NUMAMMO; ++i)
	    P_GiveAmmo(&players[0], i, 1);

    	// [STRIFE]: PUMPUPS gives stamina and accuracy upgrades
    	P_GiveItemToPlayer(&players[0], SPR_TOKN, MT_TOKEN_STAMINA);
    	P_GiveItemToPlayer(&players[0], SPR_TOKN, MT_TOKEN_NEW_ACCURACY);

    	// [STRIFE] PUMPUPT gives targeter
    	P_GivePower(&players[0], pw_targeter);
    	players[consoleplayer].message = DEH_String("you got the stuff!");
    }
    DetectState();
}

void M_Topo(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	cheating = (cheating+1) % 3;
    	cheeting = (cheeting+1) % 3;
    }
    DetectState();
}
/*
void M_GPS(int choice)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare
	&& players[consoleplayer].playerstate == PST_LIVE)
    {
    	// [STRIFE] 'GPS' for player position
    	static char buf[52];
    	M_snprintf(buf, sizeof(buf),
    	           "ang=0x%x;x,y=(0x%x,0x%x)",
    	           players[consoleplayer].mo->angle,
    	           players[consoleplayer].mo->x,
    	           players[consoleplayer].mo->y);
    	players[consoleplayer].message = buf;
    }
    DetectState();
}
*/
void M_Gripper(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	players[consoleplayer].cheats ^= CF_NOMOMENTUM;
    	if(players[consoleplayer].cheats & CF_NOMOMENTUM)
    	    players[consoleplayer].message = DEH_String("STEALTH BOOTS ON");
    	else
    	    players[consoleplayer].message = DEH_String("STEALTH BOOTS OFF");
    }
    DetectState();
}
/*
void M_Dots(int choice)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare
	&& players[consoleplayer].playerstate == PST_LIVE)
    {
    	devparm = !devparm;
    	if (devparm)
    	    players[consoleplayer].message = DEH_String("devparm ON");
    	else
    	    players[consoleplayer].message = DEH_String("devparm OFF");
    }
    DetectState();
}
*/

void M_Rift(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	switch(choice)
    	{
    	case 0:
#ifdef SHAREWARE
    	    if   ((map >=  2 && (STRIFE_1_0_REGISTERED || STRIFE_1_X_REGISTERED))
	    ||    (map >= 33 && (STRIFE_1_0_SHAREWARE  || STRIFE_1_1_SHAREWARE)))
#else
    	    if    (map >=  2 && (STRIFE_1_0_REGISTERED || STRIFE_1_X_REGISTERED))
#endif
		   map--;
    	    break;
    	case 1:
	    if    (map <  34)	// FOR PSP: STRIFE v1.0 Shareware includes MAP35 & MAP36, but...
	    {			// ...the IWAD is missing some textures (maybe they are DEV-MAPS?)
#ifdef SHAREWARE
		if(map ==  0 && (STRIFE_1_0_SHAREWARE  || STRIFE_1_1_SHAREWARE))
		   map  = 32;
#endif
    	    	   map++;
	    }
    	    break;
    	}
    }
    DetectState();
}

void M_RiftNow(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	// So be it.
    	players[consoleplayer].message = DEH_String(STSTR_CLEV);
    	G_RiftExitLevel(map, 0, players[consoleplayer].mo->angle);
    }
    DetectState();
}
					// THIS WAS SUPPOSED TO BE FOR THE PSP SOURCE PORT, BUT...
void M_Scoot(int choice)		// ...IT DOESN'T WORK AS EXPECTED (YET)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare
	&& players[consoleplayer].playerstate == PST_LIVE)
    {
    	switch(choice)
    	{
    	case 0:
    	    break;
    	case 1:
    	    break;
    	}
    }
    DetectState();
}

					// THIS WAS SUPPOSED TO BE FOR THE PSP SOURCE PORT, BUT...
void M_ScootNow(int choice)		// ...IT DOESN'T WORK AS EXPECTED (YET)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare
	&& players[consoleplayer].playerstate == PST_LIVE)
    {
    	// BUG: should be <= 9. Shouldn't do anything bad though...
    	if(spot <= 10) 
    	{
    	    players[consoleplayer].message = DEH_String("Spawning to spot");
	    G_RiftCheat(spot);
    	}
    }
    DetectState();
}

void M_Spin(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	switch(choice)
    	{
    	case 0:
    	    if	   (musnum >  1)
    	            musnum--;
    	    break;
    	case 1:
	    if	   (STRIFE_1_0_REGISTERED || STRIFE_1_X_REGISTERED)
	    {
		if (musnum < 34)
		    musnum++;
	    }
#ifdef SHAREWARE
	    else if(STRIFE_1_0_SHAREWARE)
	    {
		if (musnum < 10)
		    musnum++;
	    }
	    else if(STRIFE_1_1_SHAREWARE)
	    {
		if (musnum <  9)
		    musnum++;
	    }
#endif
    	    break;
    	}

        players[consoleplayer].message = DEH_String(STSTR_MUS);
        S_ChangeMusic(musnum, 1);
    }
    DetectState();
}

void M_DrawCheats(void)
{
    V_DrawPatch (110, 10, 0, W_CacheLumpName(DEH_String("M_CHEATS"), PU_CACHE));

    M_WriteText(72, 30, DEH_String("OMNIPOTENT (GOD)"));

    if (players[consoleplayer].cheats & CF_GODMODE)
	M_WriteText(215, 30, DEH_String("ON"));
    else
	M_WriteText(215, 30, DEH_String("OFF"));

    M_WriteText(72, 40, DEH_String("ELVIS (NOCLIP)"));

    if (players[consoleplayer].cheats & CF_NOCLIP)
	M_WriteText(215, 40, DEH_String("ON"));
    else
	M_WriteText(215, 40, DEH_String("OFF"));

    M_WriteText(72, 110, DEH_String("TOPO (MAP)"));

    if(!cheating)
	M_WriteText(215, 110, DEH_String("OFF"));
    else if (cheating && cheeting!=2)
	M_WriteText(195, 110, DEH_String("WALLS"));
    else if (cheating && cheeting==2)	  
	M_WriteText(215, 110, DEH_String("ALL"));

    M_WriteText(72, 50, DEH_String("BOOMSTIX"));
    M_WriteText(193, 50, DEH_String("(GUNS)"));

    M_WriteText(72, 60, DEH_String("JIMMY"));
    M_WriteText(194, 60, DEH_String("(KEYS)"));

    M_WriteText(72, 70, DEH_String("DONNYTRUMP"));
    M_WriteText(193, 70, DEH_String("(GOLD)"));

    M_WriteText(72, 80, DEH_String("STONECOLD"));
    M_WriteText(171, 80, DEH_String("(KILL ALL)"));

    M_WriteText(72, 90, DEH_String("LEGO"));
    M_WriteText(148, 90, DEH_String("(SIGIL PARTS)"));

    M_WriteText(72, 100, DEH_String("PUMPUP"));
    M_WriteText(188, 100, DEH_String("(ITEMS)"));

//    M_WriteText(72, 110, DEH_String("GPS"));		// FOR PSP: IT WORKS, BUT IS NOT REALLY NEEDED

    M_WriteText(72, 120, DEH_String("GRIPPER"));
    M_WriteText(143, 120, DEH_String("(SLOW SPEED)"));

//    M_WriteText(72, 130, DEH_String("DOTS"));		// FOR PSP: IT WORKS, BUT IS NOT REALLY NEEDED

    M_WriteText(72, 136, DEH_String("RIFT (WARP TO MAP):"));
    M_WriteText(72, 160, DEH_String("EXECUTE WARPING"));

//    M_WriteText(72, 160, DEH_String("WARP TO SPOT:"));
//    M_WriteText(72, 170, DEH_String("EXECUTE WARPING TO SPOT"));	// FOR PSP: (DOESN'T WORK)

    M_WriteText(72, 180, DEH_String("SPIN (PLAY MUSIC TITLE):"));

    if     (map == 0 && (STRIFE_1_0_REGISTERED || STRIFE_1_X_REGISTERED))
	    map =  1;
#ifdef SHAREWARE
    else if(map == 0 && (STRIFE_1_0_SHAREWARE  || STRIFE_1_1_SHAREWARE ))
	    map = 32;
#endif
    M_WriteText(72, 145, maptext[map]);

    if( spot == 0)
	spot =  1;

//    M_WriteText(230, 160, spottext[spot]);				// FOR PSP: (DOESN'T WORK (YET))

    if (musnum == 0)
	musnum =  1;

    M_WriteText(72, 190, mustext[musnum]);
}

void M_Cheats(int choice)
{
    M_SetupNextMenu(&CheatsDef);
}

//
// M_DrawMainMenu
//
// haleyjd 08/27/10: [STRIFE] Changed x coordinate; M_DOOM -> M_STRIFE
//
void M_DrawMainMenu(void)
{
    V_DrawPatch(84, 15, 0,
                      W_CacheLumpName(DEH_String("M_STRIFE"), PU_CACHE));
}




//
// M_NewGame
//
// haleyjd 08/31/10: [STRIFE] Changed M_NEWG -> M_NGAME
//
void M_DrawNewGame(void)
{
    V_DrawPatch(96, 14, 0, W_CacheLumpName(DEH_String("M_NGAME"), PU_CACHE));
    V_DrawPatch(54, 38, 0, W_CacheLumpName(DEH_String("M_SKILL"), PU_CACHE));
}

void M_NewGame(int choice)
{
/*
    if (netgame && !demoplayback)
    {
        M_StartMessage(DEH_String(NEWGAME),NULL,false);
        return;
    }
*/
    // haleyjd 09/07/10: [STRIFE] Removed Chex Quest and DOOM gamemodes
    if(gameversion == exe_strife_1_31)
       namingCharacter = true; // for 1.31 save logic
    M_SetupNextMenu(&NewDef);
}


//
//      M_Episode
//

// haleyjd: [STRIFE] Unused
/*
int     epi;

void M_DrawEpisode(void)
{
    V_DrawPatch(54, 38, W_CacheLumpName(DEH_String("M_EPISOD"), PU_CACHE));
}

void M_VerifyNightmare(int key)
{
    if (key != key_menu_confirm)
        return;

    G_DeferedInitNew(nightmare, 1);
    M_ClearMenus (0);
}
*/

void M_ChooseSkill(int choice)
{
    // haleyjd 09/07/10: Removed nightmare confirmation
    // [STRIFE]: start "Name Your Character" menu
    menuskill = choice;
    currentMenu = &NameCharDef;
    itemOn = NameCharDef.lastOn;
    M_ReadSaveStrings();
}

/*
// haleyjd [STRIFE] Unused
void M_Episode(int choice)
{
    if ( (gamemode == shareware)
	 && choice)
    {
	M_StartMessage(DEH_String(SWSTRING),NULL,false);
	M_SetupNextMenu(&ReadDef1);
	return;
    }

    // Yet another hack...
    if ( (gamemode == registered)
	 && (choice > 2))
    {
      fprintf( stderr,
	       "M_Episode: 4th episode requires UltimateDOOM\n");
      choice = 0;
    }
	 
    epi = choice;
    M_SetupNextMenu(&NewDef);
}
*/


//
// M_Options
//
char    detailNames[2][9]	= {"M_GDHIGH","M_GDLOW"};
char	msgNames[2][9]		= {"M_MSGOFF","M_MSGON"};


void M_DrawOptions(void)
{
    // haleyjd 08/27/10: [STRIFE] M_OPTTTL -> M_OPTION
    V_DrawPatch(108, 15, 0, 
                      W_CacheLumpName(DEH_String("M_OPTION"), PU_CACHE));

    // haleyjd 08/26/10: [STRIFE] Removed messages, sensitivity, detail.
/*
    M_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(scrnsize+1),
                 9,screenSize);
*/
}

void M_Options(int choice)
{
    M_SetupNextMenu(&OptionsDef);
}

//
// M_AutoUseHealth
//
// [STRIFE] New function
// haleyjd 20110211: toggle autouse health state
//
void M_AutoUseHealth(void)
{
    if(/*!netgame && */usergame)
    {
        players[consoleplayer].cheats ^= CF_AUTOHEALTH;

        if(players[consoleplayer].cheats & CF_AUTOHEALTH)
            players[consoleplayer].message = DEH_String("Auto use health ON");
        else
            players[consoleplayer].message = DEH_String("Auto use health OFF");
    }
}
/*
//
// M_ChangeShowText
//
// [STRIFE] New function
//
void M_ChangeShowText(void)
{
    dialogshowtext ^= true;

    if(dialogshowtext)
        players[consoleplayer].message = DEH_String("Conversation Text On");
    else
        players[consoleplayer].message = DEH_String("Conversation Text Off");
}
*/
//
//      Toggle messages on/off
//
// [STRIFE] Messages cannot be disabled in Strife
/*
void M_ChangeMessages(int choice)
{
    // warning: unused parameter `int choice'
    choice = 0;
    showMessages = 1 - showMessages;

    if (!showMessages)
        players[consoleplayer].message = DEH_String(MSGOFF);
    else
        players[consoleplayer].message = DEH_String(MSGON);

    message_dontfuckwithme = true;
}
*/


//
// M_EndGame
//
void M_EndGameResponse(int key)
{
    if (key != key_menu_confirm)
        return;

    currentMenu->lastOn = itemOn;
    M_ClearMenus (0);
    D_StartTitle ();
}

void M_EndGame(int choice)
{
    choice = 0;
    if (!usergame)
    {
        S_StartSound(NULL,sfx_oof);
        return;
    }
/*
    if (netgame)
    {
        M_StartMessage(DEH_String(NETEND),NULL,false);
        return;
    }
*/
    M_StartMessage(DEH_String(ENDGAME),M_EndGameResponse,true);
}




//
// M_ReadThis
//
void M_ReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef1);
}

//
// M_ReadThis2
//
// haleyjd 08/28/10: [STRIFE] Eliminated DOOM stuff.
//
void M_ReadThis2(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef2);
}

//
// M_ReadThis3
//
// haleyjd 08/28/10: [STRIFE] New function.
//
void M_ReadThis3(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef3);
}

/*
// haleyjd 08/28/10: [STRIFE] Not used.
void M_FinishReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&MainDef);
}
*/

#if 0
extern void F_StartCast(void);

//
// M_CheckStartCast
//
// [STRIFE] New but unused function. Was going to start a cast
//   call from within the menu system... not functional even in
//   the earliest demo version.
//
void M_CheckStartCast()
{
    if(usergame)
    {
        M_StartMessage(DEH_String("You have to end your game first."), NULL, false);
        return;
    }

    F_StartCast();
    M_ClearMenus(0);
}
#endif

//
// M_QuitResponse
//
// haleyjd 09/11/10: [STRIFE] Modifications to start up endgame
// demosequence.
//
void M_QuitResponse(int key)
{
    char buffer[20];
/*
    if (key != key_menu_confirm)
        return;

    if(netgame)
        I_Quit();
    else
*/
    {
        S_StartMusic(mus_fast);
        DEH_snprintf(buffer, sizeof(buffer), "qfmrm%i", gametic % 8 + 1);
        I_StartVoice(buffer);
        D_QuitGame();
    }
}

/*
// haleyjd 09/11/10: [STRIFE] Unused
static char *M_SelectEndMessage(void)
{
}
*/

//
// M_QuitStrife
//
// [STRIFE] Renamed from M_QuitDOOM
// haleyjd 09/11/10: No randomized text message; that's taken care of
// by the randomized voice message after confirmation.
//
void M_QuitStrife(int choice)
{
    DEH_snprintf(endstring, sizeof(endstring),
                 "Do you really want to leave?\n\n" DOSY);
  
    M_StartMessage(endstring, M_QuitResponse, true);
}

void M_ChangeSensitivity(int choice)
{
    switch(choice)
    {
    case 0:
        if (mouseSensitivity)
            mouseSensitivity--;
        break;
    case 1:
        if (mouseSensitivity < 13)
            mouseSensitivity++;
        break;
    }
}


// haleyjd [STRIFE] Unused
void M_ChangeDetail(int choice)
{
    choice = 0;
    detailLevel = 1 - detailLevel;

    R_SetViewSize (screenblocks, detailLevel);

    if (!detailLevel)
	players[consoleplayer].message = DEH_String(DETAILHI);
    else
	players[consoleplayer].message = DEH_String(DETAILLO);
}


// [STRIFE] Verified unmodified
void M_SizeDisplay(int choice)
{
    switch(choice)
    {
    case 0:
        if (screenSize > 0)
        {
            screenblocks--;
            screenSize--;
        }
        break;
    case 1:
        if (screenSize < 8)
        {
            screenblocks++;
            screenSize++;
        }
        break;
    }

    R_SetViewSize (screenblocks, detailLevel);
}




//
//      Menu Functions
//

//
// M_DrawThermo
//
// haleyjd 08/28/10: [STRIFE] Changes to some patch coordinates.
//
void
M_DrawThermo
( int	x,
  int	y,
  int	thermWidth,
  int	thermDot )
{
    int         xx;
    int         yy; // [STRIFE] Needs a temp y coordinate variable
    int         i;

    xx = x;
    yy = y + 6; // [STRIFE] +6 to y coordinate
    V_DrawPatch(xx, yy, 0, W_CacheLumpName(DEH_String("M_THERML"), PU_CACHE));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
        V_DrawPatch(xx, yy, 0, W_CacheLumpName(DEH_String("M_THERMM"), PU_CACHE));
        xx += 8;
    }
    V_DrawPatch(xx, yy, 0, W_CacheLumpName(DEH_String("M_THERMR"), PU_CACHE));

    // [STRIFE] +2 to initial y coordinate
    V_DrawPatch((x + 8) + thermDot * 8, y + 2, 0,
                      W_CacheLumpName(DEH_String("M_THERMO"), PU_CACHE));
}


// haleyjd: These are from DOOM v0.5 and the prebeta! They drew those ugly red &
// blue checkboxes... preserved for historical interest, as not in Strife.
void
M_DrawEmptyCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatch(menu->x - 10, menu->y + item * LINEHEIGHT - 1, 0, 
                      W_CacheLumpName(DEH_String("M_CELL1"), PU_CACHE));
}

void
M_DrawSelCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatch(menu->x - 10, menu->y + item * LINEHEIGHT - 1, 0,
                      W_CacheLumpName(DEH_String("M_CELL2"), PU_CACHE));
}


void
M_StartMessage
( char*		string,
  void*		routine,
  boolean	input )
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = routine;
    messageNeedsInput = input;
    menuactive = true;
    return;
}



void M_StopMessage(void)
{
    menuactive = messageLastMenuActive;
    messageToPrint = 0;
}



//
// Find string width from hu_font chars
//
int M_StringWidth(char* string)
{
    size_t             i;
    int             w = 0;
    int             c;

    for (i = 0;i < strlen(string);i++)
    {
        c = toupper(string[i]) - HU_FONTSTART;
        if (c < 0 || c >= HU_FONTSIZE)
            w += 4;
        else
            w += SHORT (hu_font[c]->width);
    }

    return w;
}



//
//      Find string height from hu_font chars
//
int M_StringHeight(char* string)
{
    size_t             i;
    int             h;
    int             height = SHORT(hu_font[0]->height);

    h = height;
    for (i = 0;i < strlen(string);i++)
        if (string[i] == '\n')
            h += height;

    return h;
}


//
// M_WriteText
//
// Write a string using the hu_font
// haleyjd 09/04/10: [STRIFE]
// * Rogue made a lot of changes to this for the dialog system.
//
int
M_WriteText
( int           x,
  int           y,
  const char*   string) // haleyjd: made const for safety w/dialog engine
{
    int	        w;
    const char* ch;
    int         c;
    int         cx;
    int         cy;

    ch = string;
    cx = x;
    cy = y;

    while(1)
    {
        c = *ch++;
        if (!c)
            break;

        // haleyjd 09/04/10: [STRIFE] Don't draw spaces at the start of lines.
        if(c == ' ' && cx == x)
            continue;

        if (c == '\n')
        {
            cx = x;
            cy += 11; // haleyjd 09/04/10: [STRIFE]: Changed 12 -> 11
            continue;
        }

        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c>= HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font[c]->width);

        // haleyjd 09/04/10: [STRIFE] Different linebreak handling
        if (cx + w > SCREENWIDTH - 20)
        {
            cx = x;
            cy += 11;
            --ch;
        }
        else
        {
            V_DrawPatch(cx, cy, 0, hu_font[c]);
            cx += w;
        }
    }

    // [STRIFE] Return final y coordinate.
    return cy + 12;
}

//
// M_DialogDimMsg
//
// [STRIFE] New function
// haleyjd 09/04/10: Painstakingly transformed from the assembly code, as the
// decompiler could not touch it. Redimensions a string to fit on screen, leaving
// at least a 20 pixel margin on the right side. The string passed in must be
// writable.
//
void M_DialogDimMsg(int x, int y, char *str, boolean useyfont)
{
    int rightbound = (SCREENWIDTH - 20) - x;
    patch_t **fontarray;  // ebp
    int linewidth = 0;    // esi
    int i = 0;            // edx
    char *message = str;  // edi
    char  bl;             // bl

    if(useyfont)
       fontarray = yfont;
    else
       fontarray = hu_font;

    bl = toupper(*message);

    if(!bl)
        return;

    // outer loop - run to end of string
    do
    {
        if(bl != '\n')
        {
            int charwidth; // eax
            int tempwidth; // ecx

            if(bl < HU_FONTSTART || bl > HU_FONTEND)
                charwidth = 4;
            else
                charwidth = SHORT(fontarray[bl - HU_FONTSTART]->width);

            tempwidth = linewidth + charwidth;

            // Test if the line still fits within the boundary...
            if(tempwidth >= rightbound)
            {
                // Doesn't fit...
                char *tempptr = &message[i]; // ebx
                char  al;                    // al

                // inner loop - run backward til a space (or the start of the
                // string) is found, subtracting width off the current line.
                // BUG: shouldn't we stop at a previous '\n' too?
                while(*tempptr != ' ' && i > 0)
                {
                    tempptr--;
                    // BUG: they didn't add the first char to linewidth yet...
                    linewidth -= charwidth; 
                    i--;
                    al = toupper(*tempptr);
                    if(al < HU_FONTSTART || al > HU_FONTEND)
                        charwidth = 4;
                    else
                        charwidth = SHORT(fontarray[al - HU_FONTSTART]->width);
                }
                // Replace the space with a linebreak.
                // BUG: what if i is zero? ... infinite loop time!
                message[i] = '\n';
                linewidth = 0;
            }
            else
            {
                // The line does fit.
                // Spaces at the start of a line don't count though.
                if(!(bl == ' ' && linewidth == 0))
                    linewidth += charwidth;
            }
        }
        else
            linewidth = 0; // '\n' seen, so reset the line width
    }
    while((bl = toupper(message[++i])) != 0); // step to the next character
}

// These keys evaluate to a "null" key in Vanilla Doom that allows weird
// jumping in the menus. Preserve this behavior for accuracy.
/*
static boolean IsNullKey(int key)
{
    return key == KEY_PAUSE || key == KEY_CAPSLOCK
        || key == KEY_SCRLCK || key == KEY_NUMLOCK;
}
*/
//
// CONTROL PANEL
//

//
// M_Responder
//
boolean M_Responder (event_t* ev)
{
    int             ch;
    int             key;
    int             i;
/*
    static  int     joywait = 0;

    static  int     mousewait = 0;
    static  int     mousey = 0;
    static  int     lasty = 0;
    static  int     mousex = 0;
    static  int     lastx = 0;
*/
    if (askforkey && ev->type == ev_keydown)		// KEY BINDINGS
    {
	M_KeyBindingsClearControls(ev->data1);
	*doom_defaults_list[keyaskedfor + 19 + FirstKey].location = ev->data1;
	askforkey = false;
	return true;
    }

    if (askforkey && ev->type == ev_mouse)
    {
	if (ev->data1 & 1)
	    return true;
	if (ev->data1 & 2)
	    return true;
	if (ev->data1 & 4)
	    return true;
	return false;
    }

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.
/*
    if (testcontrols)
    {
        if (ev->type == ev_quit
         || (ev->type == ev_keydown
          && (ev->data1 == key_menu_activate || ev->data1 == key_menu_quit)))
        {
            I_Quit();
            return true;
        }

        return false;
    }
*/
    // "close" button pressed on window?
    if (ev->type == ev_quit)
    {
        // First click on close button = bring up quit confirm message.
        // Second click on close button = confirm quit

        if (menuactive && messageToPrint && messageRoutine == M_QuitResponse)
        {
            M_QuitResponse(key_menu_confirm);
        }
        else
        {
            S_StartSound(NULL, sfx_swtchn);
            M_QuitStrife(0);
        }

        return true;
    }

    // key is the key pressed, ch is the actual character typed
  
    ch = 0;
    key = -1;
/*
    if (ev->type == ev_joystick && joywait < I_GetTime())
    {
        if (ev->data3 < 0)
        {
            key = key_menu_up;
            joywait = I_GetTime() + 5;
        }
        else if (ev->data3 > 0)
        {
            key = key_menu_down;
            joywait = I_GetTime() + 5;
        }

        if (ev->data2 < 0)
        {
            key = key_menu_left;
            joywait = I_GetTime() + 2;
        }
        else if (ev->data2 > 0)
        {
            key = key_menu_right;
            joywait = I_GetTime() + 2;
        }

        if (ev->data1&1)
        {
            key = key_menu_forward;
            joywait = I_GetTime() + 5;
        }
        if (ev->data1&2)
        {
            key = key_menu_back;
            joywait = I_GetTime() + 5;
        }
        if (joybmenu >= 0 && (ev->data1 & (1 << joybmenu)) != 0)
        {
            key = key_menu_activate;
            joywait = I_GetTime() + 5;
        }
    }
    else

    {
        if (ev->type == ev_mouse && mousewait < I_GetTime())
        {
            mousey += ev->data3;
            if (mousey < lasty-30)
            {
                key = key_menu_down;
                mousewait = I_GetTime() + 5;
                mousey = lasty -= 30;
            }
            else if (mousey > lasty+30)
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 5;
                mousey = lasty += 30;
            }

            mousex += ev->data2;
            if (mousex < lastx-30)
            {
                key = key_menu_left;
                mousewait = I_GetTime() + 5;
                mousex = lastx -= 30;
            }
            else if (mousex > lastx+30)
            {
                key = key_menu_right;
                mousewait = I_GetTime() + 5;
                mousex = lastx += 30;
            }

            if (ev->data1&1)
            {
                key = key_menu_forward;
                mousewait = I_GetTime() + 15;
//                mouse_fire_countdown = 5;   // villsa [STRIFE]
            }

            if (ev->data1&2)
            {
                key = key_menu_back;
                mousewait = I_GetTime() + 15;
            }
        }

        else
*/
        {
            if (ev->type == ev_keydown)
            {
                key = ev->data1;
                ch = ev->data2;
            }
        }
//    }

    if (key == -1)
        return false;

    // Save Game string input
    if (saveStringEnter)
    {
        switch(key)
        {
/*					// FOR PSP (we don't need any other input than confirmation)
        case KEY_SQUARE:
            if (saveCharIndex > 0)
            {
                saveCharIndex--;
                savegamestrings[quickSaveSlot][saveCharIndex] = 0;
            }
            break;
*/
        case KEY_SELECT:
            saveStringEnter = 0;
            M_StringCopy(savegamestrings[quickSaveSlot], saveOldString,
                         sizeof(savegamestrings[quickSaveSlot]));
            break;

        case KEY_CROSS:			// CHANGED FOR PSP (WAS KEY_ENTER BEFORE)
            // [STRIFE]
            saveStringEnter = 0;
            if(gameversion == exe_strife_1_31 && !namingCharacter)
            {
               // In 1.31, we can be here as a result of normal saving again,
               // whereas in 1.2 this only ever happens when naming your
               // character to begin a new game.
               M_DoSave(quickSaveSlot);
               return true;
            }
//            if (savegamestrings[quickSaveSlot][0])			// PSP workaround
                M_DoNameChar(quickSaveSlot);
            break;

        default:
            // This is complicated.
            // Vanilla has a bug where the shift key is ignored when entering
            // a savegame name. If vanilla_keyboard_mapping is on, we want
            // to emulate this bug by using 'data1'. But if it's turned off,
            // it implies the user doesn't care about Vanilla emulation: just
            // use the correct 'data2'.
/*
            if (vanilla_keyboard_mapping)
            {
                ch = key;
            }
*/
            ch = toupper(ch);

            if (ch != ' '
                && (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE))
            {
                break;
            }

            if (ch >= 32 && ch <= 127 &&
                saveCharIndex < SAVESTRINGSIZE-1 &&
                M_StringWidth(savegamestrings[quickSaveSlot]) <
                (SAVESTRINGSIZE-2)*8)
            {
                savegamestrings[quickSaveSlot][saveCharIndex++] = ch;
                savegamestrings[quickSaveSlot][saveCharIndex] = 0;
            }
            break;
        }
        return true;
    }

    // Take care of any messages that need input
    if (messageToPrint)
    {
        if (messageNeedsInput)
        {
            if (key != ' ' && key != KEY_SELECT
                && key != key_menu_confirm && key != key_menu_abort)
            {
                return false;
            }
        }

        menuactive = messageLastMenuActive;
        messageToPrint = 0;
        if (messageRoutine)
            messageRoutine(key);

        menupause = false;                // [STRIFE] unpause
        menuactive = false;
        S_StartSound(NULL, sfx_mtalht);   // [STRIFE] sound
        return true;
    }

    // [STRIFE]:
    // * In v1.2 this is moved to F9 (quickload)
    // * In v1.31 it is moved to F12 with DM spy, and quicksave
    //   functionality is restored separate from normal saving
    /*
    if (devparm && key == key_menu_help)
    {
        G_ScreenShot ();
        return true;
    }
    */
/*
    // F-Keys
    if (!menuactive)
    {

        if (key == key_menu_decscreen)      // Screen size down
        {
            if (automapactive || chat_on)
                return false;
            M_SizeDisplay(0);
            S_StartSound(NULL, sfx_stnmov);
            return true;
        }
        else if (key == key_menu_incscreen) // Screen size up
        {
            if (automapactive || chat_on)
                return false;
            M_SizeDisplay(1);
            S_StartSound(NULL, sfx_stnmov);
            return true;
        }
        else if (key == key_menu_help)     // Help key
        {
            M_StartControlPanel ();
            // haleyjd 08/29/10: [STRIFE] always ReadDef1
            currentMenu = &ReadDef1; 

            itemOn = 0;
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_save)     // Save
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
                namingCharacter = false; // just saving normally, in 1.31

            if(netgame || players[consoleplayer].health <= 0 ||
                players[consoleplayer].cheats & CF_ONFIRE)
            {
                S_StartSound(NULL, sfx_oof);
            }
            else
            {
                M_StartControlPanel();
                S_StartSound(NULL, sfx_swtchn);
                M_SaveGame(0);
            }
            return true;
        }
        else if (key == key_menu_load)     // Load
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
            {
                // 1.31: normal save loading
                namingCharacter = false;
                M_StartControlPanel();
                M_LoadGame(0);
                S_StartSound(NULL, sfx_swtchn);
            }
            else
            {
                // Pre 1.31: quickload only
                S_StartSound(NULL, sfx_swtchn);
                M_QuickLoad();
            }
            return true;
        }
        else if (key == key_menu_volume)   // Sound Volume
        {
            M_StartControlPanel ();
            currentMenu = &SoundDef;
            itemOn = sfx_vol;
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_detail)   // Detail toggle
        {
            //M_ChangeDetail(0);
            M_AutoUseHealth(); // [STRIFE]
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qsave)    // Quicksave
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
                namingCharacter = false; // for 1.31 save changes

            if(netgame || players[consoleplayer].health <= 0 ||
               players[consoleplayer].cheats & CF_ONFIRE)
            {
                S_StartSound(NULL, sfx_oof);
            }
            else
            {
                S_StartSound(NULL, sfx_swtchn);
                M_QuickSave();
            }
            return true;
        }
        else if (key == key_menu_endgame)  // End game
        {
            S_StartSound(NULL, sfx_swtchn);
            M_EndGame(0);
            return true;
        }
        else if (key == key_menu_messages) // Toggle messages
        {
            //M_ChangeMessages(0);
            M_ChangeShowText(); // [STRIFE]
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qload)    // Quickload
        {
            // [STRIFE]
            // * v1.2: takes a screenshot
            // * v1.31: does quickload again
            if(gameversion == exe_strife_1_31)
            {
                namingCharacter = false;
                S_StartSound(NULL, sfx_swtchn);
                M_QuickLoad();
            }
            else
                G_ScreenShot();
            return true;
        }
        else if (key == key_menu_quit)     // Quit DOOM
        {
            S_StartSound(NULL, sfx_swtchn);
            M_QuitStrife(0);
            return true;
        }
        else if (key == key_menu_gamma)    // gamma toggle
        {
            usegamma++;
            if (usegamma > 4)
                usegamma = 0;
            players[consoleplayer].message = DEH_String(gammamsg[usegamma]);
            I_SetPalette (W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE));
            return true;
        }
        else if(gameversion == exe_strife_1_31 && key == key_spy)
        {
            // haleyjd 20130301: 1.31 moved screenshots to F12.
            G_ScreenShot();
            return true;
        }
        else if (key != 0 && key == key_menu_screenshot)
        {
            G_ScreenShot();
            return true;
        }
    }
*/
    // Pop-up menu?
    if (!menuactive)
    {
        if (key == key_menu_activate)
        {
            M_StartControlPanel ();
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        return false;
    }

    // NEW STATUS TYPES	VS. OLD STATUS TYPES:
    //
    //		-1	=	ITT_EMPTY
    //		 1	=	ITT_EFUNC
    //		 2	=	ITT_LRFUNC
    //
    // NOT AVAILABLE IN CHOCOLATE (YET):
    //
    //		 3	=	ITT_SETMENU
    //		 4	=	ITT_INERT
    //
    //		 5	=	ITT_SETKEY	// SPECIAL FUNCTION FOR CUSTOMIZING KEY BINDINGS


    // Keys usable within menu

    if (key == key_menu_down)
    {
        // Move down to next item

        do
        {
            if (itemOn+1 > currentMenu->numitems-1)
	    {
		if (FirstKey == FIRSTKEY_MAX)	// FOR PSP (if too menu items) ;-)
		{
	            itemOn = 0;
		    FirstKey = 0;
		}
		else
		{
		    FirstKey++;
		}
	    }
            else itemOn++;
            S_StartSound(NULL, sfx_pstop);
        } while(currentMenu->menuitems[itemOn].status==-1);

        return true;
    }
    else if (key == key_menu_up)
    {
        // Move back up to previous item

        do
        {
            if (!itemOn)
		if (FirstKey == 0)		// FOR PSP (if too menu items) ;-)
		{
                    itemOn = currentMenu->numitems-1;
		    FirstKey = FIRSTKEY_MAX;
		}
		else
		{
		    FirstKey--;
		}
            else itemOn--;
            S_StartSound(NULL, sfx_pstop);
        } while(currentMenu->menuitems[itemOn].status==-1);

        return true;
    }
    else if (key == key_menu_left)
    {
        // Slide slider left

        if (currentMenu->menuitems[itemOn].routine &&
            currentMenu->menuitems[itemOn].status == 2)
        {
            S_StartSound(NULL, sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(0);
        }
        return true;
    }
    else if (key == key_menu_right)
    {
        // Slide slider right

        if (currentMenu->menuitems[itemOn].routine &&
            currentMenu->menuitems[itemOn].status == 2)
        {
            S_StartSound(NULL, sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(1);
        }
        return true;
    }
    else if (key == key_menu_forward)
    {
        // Activate menu item

        if (currentMenu->menuitems[itemOn].routine &&
            currentMenu->menuitems[itemOn].status)
        {
            currentMenu->lastOn = itemOn;
            if (currentMenu->menuitems[itemOn].status == 2)
            {
                currentMenu->menuitems[itemOn].routine(1);      // right arrow
                S_StartSound(NULL, sfx_stnmov);
            }
            else
            {
                currentMenu->menuitems[itemOn].routine(itemOn);
                //S_StartSound(NULL, sfx_swish); [STRIFE] No sound is played here.
            }
        }
        return true;
    }
    else if (key == key_menu_activate)
    {
        // Deactivate menu
        if(gameversion == exe_strife_1_31) // [STRIFE]: 1.31 saving
            namingCharacter = false;

        if(menuindialog) // [STRIFE] - Get out of dialog engine semi-gracefully
            P_DialogDoChoice(-1);

        currentMenu->lastOn = itemOn;
        M_ClearMenus (0);
        S_StartSound(NULL, sfx_mtalht); // villsa [STRIFE]: sounds

        return true;
    }
    else if (key == key_menu_back)
    {
        // Go back to previous menu

        currentMenu->lastOn = itemOn;
        if (currentMenu->prevMenu)
        {
            currentMenu = currentMenu->prevMenu;
            itemOn = currentMenu->lastOn;
            S_StartSound(NULL, sfx_swtchn);
        }
        return true;
    }

    // Keyboard shortcut?
    // Vanilla Strife has a weird behavior where it jumps to the scroll bars
    // when certain keys are pressed, so emulate this.

    else if (ch != 0/* || IsNullKey(key)*/)
    {
        // Keyboard shortcut?

        for (i = itemOn+1;i < currentMenu->numitems;i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL, sfx_pstop);
                return true;
            }
        }

        for (i = 0;i <= itemOn;i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL, sfx_pstop);
                return true;
            }
        }
    }

    return false;
}



//
// M_StartControlPanel
//
void M_StartControlPanel (void)
{
    // intro might call this repeatedly
    if (menuactive)
        return;
    
    menuactive = 1;
    menupause = true;
    currentMenu = &MainDef;         // JDC
    itemOn = currentMenu->lastOn;   // JDC
}

#include "st_stuff.h"

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
    static short	x;
    static short	y;
    unsigned int	i;
    unsigned int	max;
    char		string[80];
    char               *name;
    int			start;

    inhelpscreens = false;

    if(display_fps == 1)
    {
	M_FPSCounter(1);
    }
    else if(display_fps == 0)
    {
	M_FPSCounter(0);
    }

    if(memory_info)
    {
    	M_WriteText(0, 40, allocated_ram_textbuffer);
    	M_WriteText(0, 50, free_ram_textbuffer);
    	M_WriteText(0, 60, max_free_ram_textbuffer);
    }

    if(battery_info)
    {
  	M_WriteText(0, 70, unit_plugged_textbuffer);
    	M_WriteText(0, 80, battery_present_textbuffer);
    	M_WriteText(0, 90, battery_charging_textbuffer);
    	M_WriteText(0, 100, battery_charging_status_textbuffer);
    	M_WriteText(0, 110, battery_low_textbuffer);
    	M_WriteText(0, 120, battery_lifetime_percent_textbuffer);
    	M_WriteText(0, 130, battery_lifetime_int_textbuffer);
    	M_WriteText(0, 140, battery_temp_textbuffer);
    	M_WriteText(0, 150, battery_voltage_textbuffer);
    }

    if(cpu_info)
    {
    	M_WriteText(0, 160, processor_clock_textbuffer);
    	M_WriteText(0, 170, processor_bus_textbuffer);
    }

    if(coordinates_info)
    {
    	if(gamestate == GS_LEVEL)
    	{
	    static player_t* player;
	    player = &players[consoleplayer];

    	    sprintf(map_coordinates_textbuffer, "MAP: %d - X:%5d - Y:%5d - Z:%5d",
		    gamemap,
		    player->mo->x >> FRACBITS,
		    player->mo->y >> FRACBITS,
		    player->mo->z >> FRACBITS);

            M_WriteText(40, 24, map_coordinates_textbuffer);
    	}
    }

    if(other_info)
    	M_WriteText(0, 180, idle_time_textbuffer);

    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
        start = 0;
        y = 100 - M_StringHeight(messageString) / 2;
        while (messageString[start] != '\0')
        {
            int foundnewline = 0;

            for (i = 0; i < strlen(messageString + start); i++)
            {
                if (messageString[start + i] == '\n')
                {
                    M_StringCopy(string, messageString + start,
                                 sizeof(string));
                    if (i < sizeof(string))
                    {
                        string[i] = '\0';
                    }

                    foundnewline = 1;
                    start += i + 1;
                    break;
                }
            }

            if (!foundnewline)
            {
                M_StringCopy(string, messageString + start,
                             sizeof(string));
                start += strlen(string);
            }

            x = 160 - M_StringWidth(string) / 2;
            M_WriteText(x, y, string);
            y += SHORT(hu_font[0]->height);
        }

        return;
    }

    if (!menuactive)
        return;

    if (currentMenu->routine)
        currentMenu->routine();         // call Draw routine
    
    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;

    if (currentMenu->menuitems[itemOn].status == 5)		// FOR PSP (if too menu items) ;-)
	max += FirstKey;

    if(!devparm && currentMenu == &OptionsDef)
	currentMenu->numitems = 5;

    for (i=0;i<max;i++)
    {
        name = DEH_String(currentMenu->menuitems[i].name);

        if (name[0])
        {
            V_DrawPatch (x, y, 0, W_CacheLumpName(name, PU_CACHE));
        }
	if(currentMenu == &CheatsDef || currentMenu == &KeyBindingsDef)
	{
            y += LINEHEIGHT_SMALL;
	    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
	    // DRAW SIGIL
	    V_DrawPatch(x + CURSORXOFF_SMALL, currentMenu->y - 5 + itemOn*LINEHEIGHT_SMALL, 0,
		              W_CacheLumpName(DEH_String(cursorNameSmall[whichCursor]),
		                              PU_CACHE));
	}
        else if(currentMenu)
	{
	    y += LINEHEIGHT;
	    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
	    // DRAW SIGIL
	    V_DrawPatch(x + CURSORXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT, 0,
		              W_CacheLumpName(DEH_String(cursorName[whichCursor]),
		                              PU_CACHE));
	}
    }
}


//
// M_ClearMenus
//
// haleyjd 08/28/10: [STRIFE] Added an int param so this can be called by menus.
//         09/08/10: Added menupause.
//
void M_ClearMenus (int choice)
{
    choice = 0;     // haleyjd: for no warning; not from decompilation.
    menuactive = 0;
    menupause = 0;
}




//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;
}


//
// M_Ticker
//
// haleyjd 08/27/10: [STRIFE] Rewritten for Sigil cursor
//
void M_Ticker (void)
{
    if (--cursorAnimCounter <= 0)
    {
        whichCursor = (whichCursor + 1) % 8;
        cursorAnimCounter = 5;
    }
}


//
// M_Init
//
// haleyjd 08/27/10: [STRIFE] Removed DOOM gamemode stuff
//
void M_Init (void)
{
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichCursor = 0;
    cursorAnimCounter = 10;
    screenSize = screenblocks - 3;
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive; // STRIFE-FIXME: assigns 0 here...
    quickSaveSlot = -1;

    // [STRIFE]: Initialize savegame paths and clear temporary directory
    G_WriteSaveName(5, "ME");
    ClearTmp();

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.
}


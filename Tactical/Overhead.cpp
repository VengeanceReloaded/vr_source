#ifdef PRECOMPILEDHEADERS
#include "Tactical All.h"
#include "PreBattle Interface.h"
#include "creature spreading.h"
#include "Lua Interpreter.h"
#else
#include <stdio.h>
#include <string.h>
#include "wcheck.h"
#include "stdlib.h"
#include "debug.h"
#include "math.h"
#include "worlddef.h"
#include "worldman.h"
#include "renderworld.h"
#include "Assignments.h"
//#include "Soldier Control.h"
#include "Animation Control.h"
#include "Animation Data.h"
#include "Isometric Utils.h"
#include "Event Pump.h"
#include "Timer Control.h"
#include "Render Fun.h"
#include "Render Dirty.h"
#include "mousesystem.h"
#include "interface.h"
#include "sysutil.h"
#include "FileMan.h"
#include "points.h"
#include "Random.h"
#include "ai.h"
#include "Interactive Tiles.h"
#include "soldier ani.h"
#include "english.h"
#include "overhead.h"
#include "opplist.h"
#include "Sound Control.h"
#include "Font Control.h"
#include "lighting.h"
#include "pathai.h"
#include "screenids.h"
#include "interface cursors.h"
#include "weapons.h"
#include "rotting corpses.h"
#include "lighting.h"
#include "Handle UI Plan.h"
#include "structure.h"
#include "interface panels.h"
#include "message.h"
#include "items.h"
#include "Soldier Profile.h"
#include "fov.h"
#include "Soldier macros.h"
#include "EditorMercs.h"
#include "soldier tile.h"
#ifdef	NETWORKED
#include "Networking.h"
#include "NetworkEvent.h"
#endif
#include "structure wrap.h"
#include "tile animation.h"
#include "Strategic Merc Handler.h"
#include "strategic turns.h"
#include "Squads.h"
#include "Morale.h"
#include "Campaign.h"
#include "music control.h"
#include "faces.h"
#include "dialogue control.h"
#include "Queen Command.h"
#include "Quests.h"
#include "NPC.h"
#include "strategicmap.h"
#include "soldier profile.h"
#include "soldier functions.h"
#include "auto bandage.h"
#include "Game Event Hook.h"
#include "Explosion Control.h"
#include "SkillCheck.h"
#include "World Items.h"
#include "smell.h"
#include "Player Command.h"
#include "GameSettings.h"
#include "messageboxscreen.h"
#include "Game Clock.h"
#include "Strategic Town Loyalty.h"
#include "Strategic Mines.h"
#include "interface items.h"
#include "text.h"
#include "keys.h"
#include "Boxing.h"
#include "Town Militia.h"
#include "meanwhile.h"
#include "Map Screen Helicopter.h"
#include "interface control.h"
#include "exit grids.h"
#include "fade screen.h"
#include "Game Init.h"
#include "jascreens.h"
#include "strategic.h"
#include "arms dealer init.h"
#include "interface utils.h"
#include "Air Raid.h"
#include "civ quotes.h"
#include "drugs and alcohol.h"
#include "history.h"
#include "los.h"
#include "interface dialogue.h"
#include "Strategic AI.h"
#include "end game.h"
#include "Strategic Status.h"
#include "PreBattle Interface.h"
#include "Militia Control.h"
#include "Lua Interpreter.h"
#include "bullets.h"
#endif
#include "connect.h"

#include "Luaglobal.h"
#include "LuaInitNPCs.h"
#include "Interface.h"

// OJW - 20090419
UINT8	giMAXIMUM_NUMBER_OF_PLAYER_MERCS = CODE_MAXIMUM_NUMBER_OF_PLAYER_MERCS;
UINT8	giMAXIMUM_NUMBER_OF_PLAYER_VEHICLES = CODE_MAXIMUM_NUMBER_OF_PLAYER_VEHICLES;
UINT8	giMAXIMUM_NUMBER_OF_PLAYER_SLOTS = CODE_MAXIMUM_NUMBER_OF_PLAYER_SLOTS;
UINT8	giMAXIMUM_NUMBER_OF_ENEMIES = CODE_MAXIMUM_NUMBER_OF_ENEMIES;
UINT8	giMAXIMUM_NUMBER_OF_CREATURES = CODE_MAXIMUM_NUMBER_OF_CREATURES;
UINT8	giMAXIMUM_NUMBER_OF_REBELS = CODE_MAXIMUM_NUMBER_OF_REBELS;
UINT8	giMAXIMUM_NUMBER_OF_CIVS = CODE_MAXIMUM_NUMBER_OF_CIVS;


//forward declarations of common classes to eliminate includes
class OBJECTTYPE;
class SOLDIERTYPE;

extern void HandleBestSightingPositionInRealtime();

extern UINT8	gubAICounter;

#include "fresh_header.h"
#define RT_DELAY_BETWEEN_AI_HANDLING 50
#define RT_AI_TIMESLICE 10

INT32 giRTAILastUpdateTime = 0;
UINT32 guiAISlotToHandle = 0;
#define HANDLE_OFF_MAP_MERC 0xFFFF
#define RESET_HANDLE_OF_OFF_MAP_MERCS 0xFFFF
UINT32 guiAIAwaySlotToHandle = RESET_HANDLE_OF_OFF_MAP_MERCS;

#define PAUSE_ALL_AI_DELAY 1500

BOOLEAN gfPauseAllAI = FALSE;
INT32		giPauseAllAITimer = 0;

BOOLEAN sniperwarning;
BOOLEAN biggunwarning;
BOOLEAN gogglewarning;
//BOOLEAN airstrikeavailable;

TacticalStatusType	gTacticalStatus;

extern void RecalculateOppCntsDueToNoLongerNeutral( SOLDIERTYPE * pSoldier );
extern void SetSoldierAniSpeed( SOLDIERTYPE *pSoldier );
extern void HandleExplosionQueue( void );
extern void UpdateForContOverPortrait( SOLDIERTYPE *pSoldier, BOOLEAN fOn );
extern void HandleSystemNewAISituation( SOLDIERTYPE *pSoldier, BOOLEAN fResetABC );

extern BOOLEAN NPCInRoom( UINT8 ubProfileID, UINT8 ubRoomID );

extern	INT8		gbInvalidPlacementSlot[ NUM_INV_SLOTS ];


void ResetAllMercSpeeds( );
void HandleBloodForNewGridNo( SOLDIERTYPE *pSoldier );
BOOLEAN HandleAtNewGridNo( SOLDIERTYPE *pSoldier, BOOLEAN *pfKeepMoving );
void DoCreatureTensionQuote( SOLDIERTYPE *pSoldier );
void HandleCreatureTenseQuote( );


void RemoveSoldierFromTacticalSector( SOLDIERTYPE *pSoldier, BOOLEAN fAdjustSelected );
void HandleEndDemoInCreatureLevel( );
void DeathTimerCallback( void );
void CaptureTimerCallback( void );

// HEADROCK HAM 3.6: Define now.
void MilitiaChangesSides( void );

extern void CheckForAlertWhenEnemyDies( SOLDIERTYPE * pDyingSoldier );
extern void PlaySoldierFootstepSound( SOLDIERTYPE *pSoldier );
extern void HandleKilledQuote( SOLDIERTYPE *pKilledSoldier, SOLDIERTYPE *pKillerSoldier, INT32 sGridNo, INT8 bLevel );
extern UINT16 PickSoldierReadyAnimation( SOLDIERTYPE *pSoldier, BOOLEAN fEndReady );


extern void PlayStealthySoldierFootstepSound( SOLDIERTYPE *pSoldier );

extern BOOLEAN gfSurrendered;

// GLOBALS
#define		 START_DEMO_SCENE				3
#define	NUM_RANDOM_SCENES	4

CHAR8 gDebugStr[128];

#ifdef NETWORKED
extern	BYTE					gfAmIHost;
extern BOOLEAN					gfAmINetworked;
#endif

#define	NEW_FADE_DELAY					60

// ATE: GLOBALS FOR E3
UINT8							gubCurrentScene = 0;
CHAR8					 gzLevelFilenames[ ][ 50 ] =
{
	"A9.dat",
	"ScotTBMines.dat",
	"LindaTBCaves.dat",
	"LindaRTDesert.dat",
	"IanRTNight.dat",
	"LindaRTCave1.dat",
	"LindaRTCave2.dat"

};


INT8						ubLevelMoveLink[ 10 ] =
{
	1,
	2,
	3,
	4,
	0,
	0,
	0,
	0,
	0,
	0
};


// Soldier List used for all soldier overhead interaction
SOLDIERTYPE		Menptr[ TOTAL_SOLDIERS ];
SOLDIERTYPE		*MercPtrs[ TOTAL_SOLDIERS ];

SOLDIERTYPE		*MercSlots[ TOTAL_SOLDIERS ];
UINT32					guiNumMercSlots = 0;

SOLDIERTYPE		*AwaySlots[ TOTAL_SOLDIERS ];
UINT32					guiNumAwaySlots = 0;

// DEF: changed to have client wait for gPlayerNum assigned from host
UINT8							gbPlayerNum = 0;

// Global for current selected soldier
UINT16                                                                  gusSelectedSoldier = NOBODY;
INT8																			gbShowEnemies = FALSE;

BOOLEAN																 gfMovingAnimation = FALSE;

CHAR8					gzAlertStr[][ 30 ] =
{
	"GREEN",
	"YELLOW",
	"RED",
	"BLACK"
};

CHAR8					gzActionStr[][ 30 ] =
{
	"NONE",

	"RANDOM PATROL",
	"SEEK FRIEND",
	"SEEK OPPONENT",
	"TAKE COVER",
	"GET CLOSER",

	"POINT PATROL",
	"LEAVE WATER GAS",
	"SEEK NOISE",
	"ESCORTED MOVE",
	"RUN AWAY",

	"KNIFE MOVE",
	"APPROACH MERC",
	"TRACK",
	"EAT",
	"PICK UP ITEM",

	"SCHEDULE MOVE",
	"WALK",
	"RUN",
	"WITHDRAW",
	"FLANK LEFT",
	"FLANK RIGHT",
	"MOVE TO CLIMB",
	"CHG FACING",

	"CHG STANCE",
	"YELLOW ALERT",
	"RED ALERT",
	"CREATURE CALL",
	"PULL TRIGGER",

	"USE DETONATOR",
	"FIRE GUN",
	"TOSS PROJECTILE",
	"KNIFE STAB",
	"THROW KNIFE",

	"GIVE AID",
	"WAIT",
	"PENDING ACTION",
	"DROP ITEM",
	"COWER",

	"STOP COWERING",
	"OPEN/CLOSE DOOR",
	"UNLOCK DOOR",
	"LOCK DOOR",
	"LOWER GUN",

	"ABSOLUTELY NONE",
	"CLIMB ROOF",
	"END TURN",
	"EC&M",
	"TRAVERSE DOWN",
	"OFFER SURRENDER",
	"RAISE GUN",
};

CHAR8					gzDirectionStr[][ 30 ] =
{
	"NORTHEAST",
	"EAST",
	"SOUTHEAST",
	"SOUTH",
	"SOUTHWEST",
	"WEST",
	"NORTHWEST",
	"NORTH"
};

// TEMP VALUES FOR TEAM DEFAULT POSITIONS
UINT8 bDefaultTeamRangesMP[ MAXTEAMS ][ 2 ] = 

{
	0,              19,                     //20  US
	20,             51,                      //32  ENEMY
	52,             52,      //kulled off to make room      ;) //32    CREATURE
	53,             84,                        //32    REBELS ( OUR GUYS )
	85,             116,                   //32  CIVILIANS
	117,             119,			  // PLANNING SOLDIERS (reduced)
	120,             126,				//1 //new sides //hayden // 7 each
	127,             133,				//2
	134,             140,				//3
	141,             MAX_NUM_SOLDIERS - 1, 			//4
	MAX_NUM_SOLDIERS, TOTAL_SOLDIERS - 1		        // PLANNING SOLDIERS
};

UINT8 bDefaultTeamRanges[ MAXTEAMS_SP ][ 2 ] = 
{
	0,
	  CODE_MAXIMUM_NUMBER_OF_PLAYER_MERCS+CODE_MAXIMUM_NUMBER_OF_PLAYER_VEHICLES-1,
	CODE_MAXIMUM_NUMBER_OF_PLAYER_MERCS+CODE_MAXIMUM_NUMBER_OF_PLAYER_VEHICLES,
      CODE_MAXIMUM_NUMBER_OF_PLAYER_MERCS+CODE_MAXIMUM_NUMBER_OF_PLAYER_VEHICLES+CODE_MAXIMUM_NUMBER_OF_ENEMIES-1,
	CODE_MAXIMUM_NUMBER_OF_PLAYER_MERCS+CODE_MAXIMUM_NUMBER_OF_PLAYER_VEHICLES+CODE_MAXIMUM_NUMBER_OF_ENEMIES,
	  CODE_MAXIMUM_NUMBER_OF_PLAYER_MERCS+CODE_MAXIMUM_NUMBER_OF_PLAYER_VEHICLES+CODE_MAXIMUM_NUMBER_OF_ENEMIES+CODE_MAXIMUM_NUMBER_OF_CREATURES-1,
	CODE_MAXIMUM_NUMBER_OF_PLAYER_MERCS+CODE_MAXIMUM_NUMBER_OF_PLAYER_VEHICLES+CODE_MAXIMUM_NUMBER_OF_ENEMIES+CODE_MAXIMUM_NUMBER_OF_CREATURES,
	  CODE_MAXIMUM_NUMBER_OF_PLAYER_MERCS+CODE_MAXIMUM_NUMBER_OF_PLAYER_VEHICLES+CODE_MAXIMUM_NUMBER_OF_ENEMIES+CODE_MAXIMUM_NUMBER_OF_CREATURES+CODE_MAXIMUM_NUMBER_OF_REBELS-1,
	CODE_MAXIMUM_NUMBER_OF_PLAYER_MERCS+CODE_MAXIMUM_NUMBER_OF_PLAYER_VEHICLES+CODE_MAXIMUM_NUMBER_OF_ENEMIES+CODE_MAXIMUM_NUMBER_OF_CREATURES+CODE_MAXIMUM_NUMBER_OF_REBELS,	
	  MAX_NUM_SOLDIERS -1,
	MAX_NUM_SOLDIERS, 
	  TOTAL_SOLDIERS - 1			// PLANNING SOLDIERS
};

COLORVAL bDefaultTeamColors[ MAXTEAMS ] = 
{
	FROMRGB( 255, 255, 0 ),
	FROMRGB( 255, 0, 0 ),
	FROMRGB( 255, 0, 255 ),
	FROMRGB( 0, 255, 0 ),
	FROMRGB( 255, 255, 255 ),
	FROMRGB( 0, 0, 255 ),
	FROMRGB( 255, 120, 0 ), //hayden //team 1 (radar colours)	// orange
	FROMRGB( 62, 140, 240 ), //2									// light blue							
	FROMRGB( 180, 50, 255 ), //3								// violett
	FROMRGB( 0, 180, 20 ) //4									// green

};


// UTILITY FUNCTIONS
INT8	NumActiveAndConsciousTeamMembers( UINT8 ubTeam );
UINT8 NumEnemyInSector( );
UINT8 NumEnemyInSectorExceptCreatures();
UINT8 NumCapableEnemyInSector( );

BOOLEAN KillIncompacitatedEnemyInSector( );
BOOLEAN CheckForLosingEndOfBattle( );
void	EndBattleWithUnconsciousGuysCallback( UINT8 bExitValue );
UINT8 NumEnemyInSectorNotDeadOrDying( );
UINT8 NumBloodcatsInSectorNotDeadOrDying( );


UINT8		gubWaitingForAllMercsToExitCode			= 0;
INT8		gbNumMercsUntilWaitingOver					= 0;
UINT32	guiWaitingForAllMercsToExitData[3];
UINT32	guiWaitingForAllMercsToExitTimer = 0;
BOOLEAN	gfKillingGuysForLosingBattle = FALSE;

INT32 GetFreeMercSlot(void)
{
	UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumMercSlots; uiCount++)
	{
		if(( MercSlots[uiCount] == NULL ) )
			return((INT32)uiCount);
	}

	if(guiNumMercSlots < TOTAL_SOLDIERS )
		return((INT32)guiNumMercSlots++);

	return(-1);
}


void RecountMercSlots(void)
{
	INT32 iCount;

	if ( guiNumMercSlots > 0 )
	{
		// set equal to 0 as a default
		for (iCount = guiNumMercSlots - 1; (iCount >=0) ; iCount--)
		{
			if ( ( MercSlots[iCount] != NULL ) )
			{
				guiNumMercSlots=(UINT32)(iCount+1);
				return;
			}
		}
		// no mercs found
		guiNumMercSlots = 0;
	}
}


INT32	AddMercSlot( SOLDIERTYPE *pSoldier )
{
	INT32					iMercIndex;

	if( ( iMercIndex = GetFreeMercSlot() )==(-1) )
		return(-1);

	MercSlots[ iMercIndex ] = pSoldier;

	return( iMercIndex );
}


BOOLEAN RemoveMercSlot( SOLDIERTYPE *pSoldier )
{
	UINT32 uiCount;

	CHECKF( pSoldier != NULL );

	for( uiCount=0; uiCount < guiNumMercSlots; uiCount++)
	{
		if ( MercSlots[ uiCount ] == pSoldier )
		{
			MercSlots[ uiCount ] = NULL;
			RecountMercSlots( );
			return( TRUE );
		}
	}

	// TOLD TO DELETE NON-EXISTANT SOLDIER
	return( FALSE );
}

INT32 GetFreeAwaySlot(void)
{
	UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumAwaySlots; uiCount++)
	{
		if(( AwaySlots[uiCount] == NULL ) )
			return((INT32)uiCount);
	}

	if(guiNumAwaySlots < TOTAL_SOLDIERS )
		return((INT32)guiNumAwaySlots++);

	return(-1);
}


void RecountAwaySlots(void)
{
	INT32 iCount;

	if ( guiNumAwaySlots > 0 )
	{
		for (iCount = guiNumAwaySlots - 1; (iCount >=0) ; iCount--)
		{
			if ( ( AwaySlots[iCount] != NULL ) )
			{
				guiNumAwaySlots = (UINT32)(iCount + 1);
				return;
			}
		}
		// no mercs found
		guiNumAwaySlots = 0;
	}
}


INT32	AddAwaySlot( SOLDIERTYPE *pSoldier )
{
	INT32					iAwayIndex;

	if( ( iAwayIndex = GetFreeAwaySlot() )==(-1) )
		return(-1);

	AwaySlots[ iAwayIndex ] = pSoldier;

	return( iAwayIndex );
}


BOOLEAN RemoveAwaySlot( SOLDIERTYPE *pSoldier )
{
	UINT32 uiCount;

	CHECKF( pSoldier != NULL );

	for( uiCount=0; uiCount < guiNumAwaySlots; uiCount++)
	{
		if ( AwaySlots[ uiCount ] == pSoldier )
		{
			AwaySlots[ uiCount ] = NULL;
			RecountAwaySlots( );
			return( TRUE );
		}
	}

	// TOLD TO DELETE NON-EXISTANT SOLDIER
	return( FALSE );
}

INT32 MoveSoldierFromMercToAwaySlot( SOLDIERTYPE * pSoldier )
{
	BOOLEAN fRet;

	fRet = RemoveMercSlot( pSoldier );
	if (!fRet)
	{
		return( -1 );
	}

	if ( !(pSoldier->flags.uiStatusFlags & SOLDIER_OFF_MAP) )
	{
		RemoveManFromTeam( pSoldier->bTeam );
	}

	pSoldier->bInSector = FALSE;
	pSoldier->flags.uiStatusFlags |= SOLDIER_OFF_MAP;
	return( AddAwaySlot( pSoldier ) );
}


INT32 MoveSoldierFromAwayToMercSlot( SOLDIERTYPE * pSoldier )
{
	BOOLEAN fRet;

	fRet = RemoveAwaySlot( pSoldier );
	if (!fRet)
	{
		return( -1 );
	}

	AddManToTeam( pSoldier->bTeam );

	pSoldier->bInSector = TRUE;
	pSoldier->flags.uiStatusFlags &= (~SOLDIER_OFF_MAP);
	return( AddMercSlot( pSoldier ) );
}




BOOLEAN InitTacticalEngine( )
{
	// Init renderer
	InitRenderParams( 0 );

	// Init dirty queue system
	InitializeBaseDirtyRectQueue( );

	// Init Interface stuff
	InitializeTacticalInterface(	);

	// Init system objects
	InitializeGameVideoObjects( );

	// Init palette system
	LoadPaletteData( );

	if( !LoadLockTable() )
		return(FALSE);

	InitInteractiveTileManagement( );

	// init path code
	if (!InitPathAI())
	{
		return( FALSE );
	}

	// init AI
	if (!InitAI())
	{
		return( FALSE );
	}

	// Init Overhead
	if ( !InitOverhead( ) )
	{
		return( FALSE );
	}

#ifdef NETWORKED
	if ( !gfAmINetworked )
		gfAmIHost = TRUE;
#endif

	return( TRUE );
}

void ShutdownTacticalEngine( )
{
	DeletePaletteData( );

	ShutdownStaticExternalNPCFaces( );

	ShutDownPathAI();
	ShutdownInteractiveTileManagement( );
	UnLoadCarPortraits( );

	ShutdownNPCQuotes();

}

BOOLEAN InitOverhead( )
{
	UINT32	cnt;
	UINT8		cnt2;

	//memset( MercSlots, 0, sizeof( MercSlots ) );
	//memset( AwaySlots, 0, sizeof( AwaySlots ) );

	// Set pointers list
	for( cnt = 0; cnt < TOTAL_SOLDIERS; cnt++ )
	{
		MercPtrs[ cnt ] = &Menptr[ cnt ];
		MercPtrs[ cnt ]->bActive = FALSE;
	}

	memset( &gTacticalStatus, 0, sizeof( TacticalStatusType ) );

	UINT8 maxteams;
	if (!is_networked)
		maxteams = MAXTEAMS_SP;
	else
		maxteams = MAXTEAMS;

	// Set team values
	for( cnt = 0; cnt < maxteams; cnt++ )
	{
		// For now, set hard-coded values
		if (!is_networked)
		{
			gTacticalStatus.Team[ cnt ].bFirstID = bDefaultTeamRanges[ cnt ][0];
			gTacticalStatus.Team[ cnt ].bLastID =	bDefaultTeamRanges[ cnt ][1];
		}
		else
		{
			gTacticalStatus.Team[ cnt ].bFirstID = bDefaultTeamRangesMP[ cnt ][0];
			gTacticalStatus.Team[ cnt ].bLastID =	bDefaultTeamRangesMP[ cnt ][1];
		}
		// WDS - make number of mercenaries, etc. be configurable
		unsigned max = 0;
		switch (cnt) {
			case OUR_TEAM:
				max = gGameExternalOptions.ubGameMaximumNumberOfPlayerMercs + gGameExternalOptions.ubGameMaximumNumberOfPlayerVehicles;
				break;
			case ENEMY_TEAM:
				max = gGameExternalOptions.ubGameMaximumNumberOfEnemies;
				break;
			case CREATURE_TEAM:
				max = gGameExternalOptions.ubGameMaximumNumberOfCreatures;
				break;
			case MILITIA_TEAM:
				max = gGameExternalOptions.ubGameMaximumNumberOfRebels;
				break;
			case CIV_TEAM:
				max = gGameExternalOptions.ubGameMaximumNumberOfCivilians;
				break;
			// Don't worry about the others
			case PLAYER_PLAN:
			case LAN_TEAM_ONE:
			case LAN_TEAM_TWO:
			case LAN_TEAM_THREE:
			case LAN_TEAM_FOUR:
				max = 9999;
				break;
		}
		if (max < (unsigned)(gTacticalStatus.Team[ cnt ].bLastID - gTacticalStatus.Team[ cnt ].bFirstID + 1)) {
			gTacticalStatus.Team[ cnt ].bLastID = gTacticalStatus.Team[ cnt ].bFirstID + max - 1;
		}

		gTacticalStatus.Team[ cnt ].RadarColor = bDefaultTeamColors[ cnt ];

		if ( cnt == gbPlayerNum || cnt == PLAYER_PLAN )
		{
			gTacticalStatus.Team[ cnt ].bSide = 0;
			gTacticalStatus.Team[ cnt ].bHuman = TRUE;
		}
		else
		{
			if (cnt == MILITIA_TEAM )
			{
				// militia guys on our side!
				gTacticalStatus.Team[ cnt ].bSide = 0;
			}
			else if ( cnt == CREATURE_TEAM )
			{
				// creatures are on no one's side but their own
				// NB side 2 is used for hostile rebels....
				gTacticalStatus.Team[ cnt ].bSide = 3;
			}
			else
			{
				// hostile (enemies, or civilians; civs are potentially hostile but neutral)
				gTacticalStatus.Team[ cnt ].bSide = 1;
			}
			gTacticalStatus.Team[ cnt ].bHuman = FALSE;
		}

		gTacticalStatus.Team[ cnt ].ubLastMercToRadio = NOBODY;
		gTacticalStatus.Team[ cnt ].bTeamActive = FALSE;
		gTacticalStatus.Team[ cnt ].bAwareOfOpposition = FALSE;

		// set team values in soldier structures for all who are on this team
		for ( cnt2 = gTacticalStatus.Team[ cnt ].bFirstID; cnt2 <= gTacticalStatus.Team[ cnt ].bLastID; cnt2++ )
		{
			MercPtrs[ cnt2 ]->bTeam = (INT8) cnt;
		}
	}

	// Zero out merc slots!
	for ( cnt = 0; cnt < TOTAL_SOLDIERS; cnt++ )
	{
		MercSlots[ cnt ] = NULL;
	}

	// Set other tactical flags
	gTacticalStatus.uiFlags = TURNBASED | TRANSLUCENCY_TYPE;
	gTacticalStatus.sSlideTarget = NOWHERE;
	gTacticalStatus.uiTimeOfLastInput = GetJA2Clock();
	gTacticalStatus.uiTimeSinceDemoOn = GetJA2Clock();
	gTacticalStatus.uiCountdownToRestart = GetJA2Clock();
	gTacticalStatus.fGoingToEnterDemo				= FALSE;
	gTacticalStatus.fNOTDOLASTDEMO					= FALSE;
	
	if (is_networked)
		gTacticalStatus.fDidGameJustStart               = FALSE; //hayden
	else
	gTacticalStatus.fDidGameJustStart				= TRUE;

	gTacticalStatus.ubLastRequesterTargetID					= NO_PROFILE;
	gTacticalStatus.ubLastRequesterSurgeryTargetID = NOBODY; // SANDRO - reset surgery requester too

	for ( cnt = 0; cnt < NUM_PANIC_TRIGGERS; cnt++ )
	{
		gTacticalStatus.sPanicTriggerGridNo[ cnt ] = NOWHERE;
	}
	/*	for ( cnt = 0; cnt < NUM_TOPTIONS; cnt++ )
	{
	gGameSettings.fOptions[ cnt ] = 1;
	}

	gGameSettings.fOptions[ TOPTION_RTCONFIRM ] = 0;
	gGameSettings.fOptions[ TOPTION_HIDE_BULLETS ] = 0;
	*/
	gTacticalStatus.bRealtimeSpeed					= MAX_REALTIME_SPEED_VAL / 2;

	gfInAirRaid = FALSE;
	gpCustomizableTimerCallback = NULL;

	// Reset cursor
	gpItemPointer = NULL;
	gpItemPointerSoldier = NULL;
	memset( gbInvalidPlacementSlot, 0, sizeof( gbInvalidPlacementSlot ) );

	InitCivQuoteSystem( );

	ZeroAnimSurfaceCounts( );

	InitializeLua();

	return( TRUE );
}

BOOLEAN ShutdownOverhead( )
{
	UINT32 cnt;

	ShutdownLua( );

	// Delete any soldiers which have been created!
	for( cnt = 0; cnt < TOTAL_SOLDIERS; cnt++ )
	{
		if( MercPtrs[ cnt ] != NULL )
		{
			if ( MercPtrs[ cnt ]->bActive )
			{
				MercPtrs[ cnt ]->DeleteSoldier(	);
			}
		}
	}

	return( TRUE );
}


BOOLEAN GetSoldier( SOLDIERTYPE **ppSoldier, UINT16 usSoldierIndex )
{
	// Check range of index given
	*ppSoldier = NULL;

	if ( usSoldierIndex < 0 || usSoldierIndex > TOTAL_SOLDIERS-1 )
	{
		// Set debug message
		return( FALSE );
	}

	// Check if a guy exists here
	// Does another soldier exist here?
	if ( MercPtrs[ usSoldierIndex ]->bActive )
	{
		// Set Existing guy
		*ppSoldier = MercPtrs[ usSoldierIndex ];
		return( TRUE);
	}
	else
	{
		return( FALSE );
	}

}

BOOLEAN	NextAIToHandle( UINT32 uiCurrAISlot )
{
	UINT32	cnt;

	if (uiCurrAISlot >= guiNumMercSlots)
	{
		// last person to handle was an off-map merc, so now we start looping at the beginning
		// again
		cnt = 0;
	}
	else
	{
		// continue on from the last person we handled
		cnt = uiCurrAISlot + 1;
	}

	for ( ; cnt < guiNumMercSlots; cnt++ )
	{
		if ( MercSlots[ cnt ] && ( (MercSlots[ cnt ]->bTeam != gbPlayerNum) || (MercSlots[ cnt ]->flags.uiStatusFlags & SOLDIER_PCUNDERAICONTROL) ) )
		{
			// aha! found an AI guy!
			guiAISlotToHandle = cnt;
			return( TRUE );
		}
	}

	// set so that even if there are no off-screen mercs to handle, we will loop back to
	// the start of the array
	guiAISlotToHandle = HANDLE_OFF_MAP_MERC;

	// didn't find an AI guy to handle after the last one handled and the # of slots
	// it's time to check for an off-map merc... maybe
	if (guiNumAwaySlots > 0)
	{
		if ( (guiAIAwaySlotToHandle + 1) >= guiNumAwaySlots)
		{
			// start looping from the beginning
			cnt = 0;
		}
		else
		{
			// continue on from the last person we handled
			cnt = guiAIAwaySlotToHandle + 1;
		}

		for ( ; cnt < guiNumAwaySlots; cnt++ )
		{
			if (AwaySlots[ cnt ] && AwaySlots[ cnt ]->bTeam != gbPlayerNum)
			{
				// aha! found an AI guy!
				guiAIAwaySlotToHandle = cnt;
				return( FALSE );
			}
		}

		// reset awayAISlotToHandle, but DON'T loop again, away slots not that important
		guiAIAwaySlotToHandle = RESET_HANDLE_OF_OFF_MAP_MERCS;
	}

	return( FALSE );
}

void PauseAITemporarily( void )
{
	gfPauseAllAI = TRUE;
	giPauseAllAITimer = GetJA2Clock();
}

void PauseAIUntilManuallyUnpaused( void )
{
	gfPauseAllAI = TRUE;
	giPauseAllAITimer = 0;
}

void UnPauseAI( void )
{
	// overrides any timer too
	gfPauseAllAI = FALSE;
	giPauseAllAITimer = 0;
}

FLOAT gdRadiansForAngle[ ] =
{
	(FLOAT)PI,
	(FLOAT)( 3 * PI / 4 ),
	(FLOAT)( PI / 2 ),
	(FLOAT)( ( PI ) / 4 ),

	(FLOAT)0,
	(FLOAT)( ( -PI ) / 4 ),
	(FLOAT)( -PI / 2 ),
	(FLOAT)( -3 * PI / 4 ),

};


BOOLEAN ExecuteOverhead( )
{

	UINT32							cnt;
	SOLDIERTYPE			 *pSoldier;
	INT16									sAPCost;
	INT16									sBPCost;
	FLOAT									dXPos , dYPos;
	FLOAT									dAngle;
	BOOLEAN						 fKeepMoving;
	INT8							bShadeLevel;
	BOOLEAN						 fNoAPsForPendingAction;
	INT32 sGridNo;
	STRUCTURE						*pStructure;
	BOOLEAN													fHandleAI = FALSE;


	// Diagnostic Stuff
	static INT32									iTimerTest = 0;
	static INT32									iTimerVal = 0;


	gfMovingAnimation = FALSE;

	if(GetSoldier(&pSoldier, gusSelectedSoldier))
	{
		if(pSoldier->bActive)
		{
			if(pSoldier->flags.uiStatusFlags&SOLDIER_GREEN_RAY)
				LightShowRays((INT16)(pSoldier->dXPos/CELL_X_SIZE), (INT16)(pSoldier->dYPos/CELL_Y_SIZE), FALSE);
		}
	}


	if ( COUNTERDONE( TOVERHEAD ) )
	{
		// Reset counter
		RESETCOUNTER( TOVERHEAD );

		// Diagnostic Stuff
		iTimerVal = GetJA2Clock();
		giTimerDiag = iTimerVal - iTimerTest;
		iTimerTest = iTimerVal;

		// ANIMATED TILE STUFF
		UpdateAniTiles( );

		// BOMBS!!!
		HandleExplosionQueue();

		HandleCreatureTenseQuote( );

		CheckHostileOrSayQuoteList();

		if ( gfPauseAllAI && giPauseAllAITimer && ( iTimerVal - giPauseAllAITimer > PAUSE_ALL_AI_DELAY ) )
		{
			// ok, stop pausing the AI!
			gfPauseAllAI = FALSE;
		}

		if (!gfPauseAllAI)
		{
			// AI limiting crap
			gubAICounter = 0;
			if (!((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT)))
			{
				if ((iTimerVal - giRTAILastUpdateTime ) > RT_DELAY_BETWEEN_AI_HANDLING)
				{
					giRTAILastUpdateTime = iTimerVal;
					// figure out which AI guy to handle this time around,
					// starting with the slot AFTER the current AI guy
					fHandleAI = NextAIToHandle( guiAISlotToHandle );
				}
			}
		}

		for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
		{
			pSoldier = MercSlots[ cnt ];

			if ( pSoldier && pSoldier->bActive && (pSoldier->bTeam == OUR_TEAM) ) {
				if (!gogglewarning && BadGoggles(pSoldier)) {
					gogglewarning = TRUE;
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, New113Message[MSG113_CHECK_GOGGLES] );
				}
			}

			// Syncronize for upcoming soldier counters
			SYNCTIMECOUNTER( );

			if ( pSoldier != NULL )
			{
				HandlePanelFaceAnimations( pSoldier );

				// Handle damage counters
				if ( pSoldier->flags.fDisplayDamage )
				{
					if ( TIMECOUNTERDONE( pSoldier->timeCounters.DamageCounter, DAMAGE_DISPLAY_DELAY ) )
					{
						pSoldier->bDisplayDamageCount++;
						pSoldier->sDamageX+=1;
						pSoldier->sDamageY-=1;

						RESETTIMECOUNTER( pSoldier->timeCounters.DamageCounter, DAMAGE_DISPLAY_DELAY );
					}

					if ( pSoldier->bDisplayDamageCount >= 8 )
					{
						pSoldier->bDisplayDamageCount = 0;
						pSoldier->sDamage = 0;
						pSoldier->flags.fDisplayDamage = FALSE;
					}

				}

				// Handle reload counters
				if ( pSoldier->flags.fReloading )
				{
					if ( TIMECOUNTERDONE( pSoldier->timeCounters.ReloadCounter, pSoldier->sReloadDelay ) )
					{
						pSoldier->flags.fReloading = FALSE;
						pSoldier->flags.fPauseAim	= FALSE;
						/*
						DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("@@@@@@@ Freeing up attacker - realtime reloading") );
						FreeUpAttacker( pSoldier->ubID );
						*/
					}
				}

				// Checkout fading
				if ( pSoldier->flags.fBeginFade )
				{
					if ( TIMECOUNTERDONE( pSoldier->timeCounters.FadeCounter, NEW_FADE_DELAY ) )
					{
						RESETTIMECOUNTER( pSoldier->timeCounters.FadeCounter, NEW_FADE_DELAY );

						// Fade out....
						if ( pSoldier->flags.fBeginFade == 1 )
						{
							bShadeLevel = (pSoldier->ubFadeLevel&0x0f);
							bShadeLevel=__min(bShadeLevel+1, SHADE_MIN);

							if ( bShadeLevel >= ( SHADE_MIN - 3 ) )
							{
								pSoldier->flags.fBeginFade = FALSE;
								pSoldier->bVisible = -1;

								// Set levelnode shade level....
								if ( pSoldier->pLevelNode )
								{
									pSoldier->pLevelNode->ubShadeLevel = bShadeLevel;
								}

								// Set Anim speed accordingly!
								SetSoldierAniSpeed( pSoldier );
							}

							bShadeLevel|=(pSoldier->ubFadeLevel&0x30);
							pSoldier->ubFadeLevel = bShadeLevel;
						}
						else if ( pSoldier->flags.fBeginFade == 2 )
						{
							bShadeLevel = (pSoldier->ubFadeLevel&0x0f);
							//ubShadeLevel =__max(ubShadeLevel-1, gpWorldLevelData[ pSoldier->sGridNo ].pLandHead->ubShadeLevel );

							bShadeLevel = bShadeLevel-1;

							if ( bShadeLevel <= 0 )
							{
								bShadeLevel = 0;
							}

							if ( bShadeLevel <= (gpWorldLevelData[ pSoldier->sGridNo ].pLandHead->ubShadeLevel ) )
							{
								bShadeLevel = (gpWorldLevelData[ pSoldier->sGridNo ].pLandHead->ubShadeLevel );

								pSoldier->flags.fBeginFade = FALSE;
								//pSoldier->bVisible = -1;
								//pSoldier->ubFadeLevel = gpWorldLevelData[ pSoldier->sGridNo ].pLandHead->ubShadeLevel;

								// Set levelnode shade level....
								if ( pSoldier->pLevelNode )
								{
									pSoldier->pLevelNode->ubShadeLevel = bShadeLevel;
								}

								// Set Anim speed accordingly!
								SetSoldierAniSpeed( pSoldier );
							}

							bShadeLevel|=(pSoldier->ubFadeLevel&0x30);
							pSoldier->ubFadeLevel = bShadeLevel;
						}
					}
				}

				// Check if we have a new visiblity and shade accordingly down
				if ( pSoldier->bLastRenderVisibleValue != pSoldier->bVisible	)
				{
					HandleCrowShadowVisibility( pSoldier );

					// Check for fade out....
					if ( pSoldier->bVisible == -1 && pSoldier->bLastRenderVisibleValue >= 0 )
					{						
						if (!TileIsOutOfBounds(pSoldier->sGridNo))
						{
							pSoldier->ubFadeLevel = gpWorldLevelData[ pSoldier->sGridNo ].pLandHead->ubShadeLevel;
						}
						
						pSoldier->flags.fBeginFade	= TRUE;
						pSoldier->sLocationOfFadeStart = pSoldier->sGridNo;

						// OK, re-evaluate guy's roof marker
						HandlePlacingRoofMarker( pSoldier, pSoldier->sGridNo, FALSE, FALSE );

						pSoldier->bVisible = -2;
					}

					// Check for fade in.....
					if ( pSoldier->bVisible != -1 && pSoldier->bLastRenderVisibleValue == -1 && pSoldier->bTeam != gbPlayerNum )
					{
						pSoldier->ubFadeLevel = ( SHADE_MIN - 3 );
						pSoldier->flags.fBeginFade	= 2;
						pSoldier->sLocationOfFadeStart = pSoldier->sGridNo;

						// OK, re-evaluate guy's roof marker
						HandlePlacingRoofMarker( pSoldier, pSoldier->sGridNo, TRUE, FALSE );
					}

				}
				pSoldier->bLastRenderVisibleValue = pSoldier->bVisible;


				// Handle stationary polling...
				if ( ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_STATIONARY ) || pSoldier->flags.fNoAPToFinishMove )
				{
					// Are are stationary....
					// Were we once moving...?
					if ( pSoldier->flags.fSoldierWasMoving && pSoldier->bVisible > -1 )
					{
						pSoldier->flags.fSoldierWasMoving = FALSE;

						HandlePlacingRoofMarker( pSoldier, pSoldier->sGridNo, TRUE, FALSE );

						if ( !gGameSettings.fOptions[ TOPTION_MERC_ALWAYS_LIGHT_UP ] )
						{
							pSoldier->DeleteSoldierLight( );

							pSoldier->SetCheckSoldierLightFlag( );
						}
					}
				}
				else
				{
					// We are moving....
					// Were we once stationary?
					if ( !pSoldier->flags.fSoldierWasMoving )
					{
						pSoldier->flags.fSoldierWasMoving = TRUE;

						HandlePlacingRoofMarker( pSoldier, pSoldier->sGridNo, FALSE, FALSE );
					}
				}

				// Handle animation update counters
				// ATE: Added additional check here for special value of anispeed that pauses all updates
				{
#ifndef BOUNDS_CHECKER
				if ( TIMECOUNTERDONE( pSoldier->timeCounters.UpdateCounter, pSoldier->sAniDelay ) && pSoldier->sAniDelay != 10000 )
#endif
				{

#ifdef NETWORKED
					// DEF:
					// Check for TIMING delay here only if in Realtime
					if( gTacticalStatus.uiFlags & REALTIME)
						if ( pSoldier->flags.fIsSoldierMoving )
							CheckForSlowSoldier( pSoldier );
#endif

					// Check if we need to look for items
					if ( pSoldier->flags.uiStatusFlags & SOLDIER_LOOKFOR_ITEMS )
					{
						RevealRoofsAndItems(pSoldier, TRUE, FALSE, pSoldier->pathing.bLevel, FALSE );
						pSoldier->flags.uiStatusFlags &= (~SOLDIER_LOOKFOR_ITEMS);
					}


					// Check if we need to reposition light....
					if ( pSoldier->flags.uiStatusFlags & SOLDIER_RECHECKLIGHT )
					{
						pSoldier->PositionSoldierLight( );
						pSoldier->flags.uiStatusFlags &= (~SOLDIER_RECHECKLIGHT);
					}

					RESETTIMECOUNTER( pSoldier->timeCounters.UpdateCounter, pSoldier->sAniDelay );

					fNoAPsForPendingAction = FALSE;

#ifdef NETWORKED
					// Get the path update, if there is 1
					if (pSoldier->flags.fSoldierUpdatedFromNetwork)
						UpdateSoldierFromNetwork(pSoldier);
#endif

					// Check if we are moving and we deduct points and we have no points
					if ( !( ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ( ANIM_MOVING | ANIM_SPECIALMOVE ) ) && pSoldier->flags.fNoAPToFinishMove	) && !pSoldier->flags.fPauseAllAnimation	)
					{
						if ( !AdjustToNextAnimationFrame( pSoldier ) )
						{
							continue;
						}
						//hayden - holt at scheduled grid
						
						//if (is_networked)
						//{
						//	if(pSoldier->sGridNo==pSoldier->sScheduledStop)
						//	{
						//		pSoldier->HaultSoldierFromSighting( 1 );
						//		pSoldier->sScheduledStop=NULL;
						//	}
						//}

						if ( !( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_SPECIALMOVE ) )
						{
							// Check if we are waiting for an opened path
							HandleNextTileWaiting( pSoldier );
						}

						// Update world data with new position, etc
						// Determine gameworld cells corrds of guy
						if ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ( ANIM_MOVING | ANIM_SPECIALMOVE ) && !( pSoldier->flags.uiStatusFlags & SOLDIER_PAUSEANIMOVE ) )
						{
							fKeepMoving = TRUE;

							pSoldier->flags.fPausedMove	= FALSE;

							// CHECK TO SEE IF WE'RE ON A MIDDLE TILE
							if ( pSoldier->flags.fPastXDest && pSoldier->flags.fPastYDest )
							{
								pSoldier->flags.fPastXDest = pSoldier->flags.fPastYDest = FALSE;
								// assign X/Y values back to make sure we are at the center of the tile
								// (to prevent mercs from going through corners of tiles and producing
								// structure data complaints)

								//pSoldier->dXPos = pSoldier->pathing.sDestXPos;
								//pSoldier->dYPos = pSoldier->pathing.sDestYPos;

								HandleBloodForNewGridNo( pSoldier );

								if ( ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_SPECIALMOVE ) && pSoldier->sGridNo != pSoldier->pathing.sFinalDestination )
								{

								}
								else
								{
									//OK, we're at the MIDDLE of a new tile...
									HandleAtNewGridNo( pSoldier, &fKeepMoving );
								}

								if ( gTacticalStatus.bBoxingState != NOT_BOXING && (gTacticalStatus.bBoxingState == BOXING_WAITING_FOR_PLAYER || gTacticalStatus.bBoxingState == PRE_BOXING || gTacticalStatus.bBoxingState == BOXING) )
								{
									BoxingMovementCheck( pSoldier );
								}

								// Are we at our final destination?
								if ( pSoldier->pathing.sFinalDestination == pSoldier->sGridNo )
								{
									// Cancel path....
									pSoldier->pathing.usPathIndex = pSoldier->pathing.usPathDataSize = 0;

									// Cancel reverse
									pSoldier->bReverse = FALSE;

									// OK, if we are the selected soldier, refresh some UI stuff
									if ( pSoldier->ubID == (UINT8)gusSelectedSoldier )
									{
										gfUIRefreshArrows = TRUE;
									}

									// ATE: Play landing sound.....
									if ( pSoldier->usAnimState == JUMP_OVER_BLOCKING_PERSON )
									{
										PlaySoldierFootstepSound( pSoldier );
									}

									// If we are a robot, play stop sound...
									if ( pSoldier->flags.uiStatusFlags & SOLDIER_ROBOT )
									{
										PlaySoldierJA2Sample( pSoldier->ubID, ROBOT_STOP, RATE_11025, SoundVolume( HIGHVOLUME, pSoldier->sGridNo ), 1, SoundDir( pSoldier->sGridNo ), TRUE );
									}

									// Update to middle if we're on destination
									dXPos = pSoldier->pathing.sDestXPos;
									dYPos = pSoldier->pathing.sDestYPos;
									pSoldier->EVENT_SetSoldierPosition( dXPos, dYPos );
#ifdef NETWORKED
									// DEF: Test Code
									StopSoldierMovementTime(pSoldier);
#endif

									// Handle New sight
									//HandleSight(pSoldier,SIGHT_LOOK | SIGHT_RADIO );

									// CHECK IF WE HAVE A PENDING ANIMATION
									if ( pSoldier->usPendingAnimation != NO_PENDING_ANIMATION )
									{
										pSoldier->ChangeSoldierState( pSoldier->usPendingAnimation, 0 , FALSE );
										pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;

										if ( pSoldier->ubPendingDirection != NO_PENDING_DIRECTION )
										{
											pSoldier->EVENT_SetSoldierDesiredDirection( pSoldier->ubPendingDirection );
											pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
										}

									}

									// CHECK IF WE HAVE A PENDING ACTION
									if ( pSoldier->ubWaitActionToDo	)
									{
										if ( pSoldier->ubWaitActionToDo == 2 )
										{
											pSoldier->ubWaitActionToDo = 1;

											if ( gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALKOFF_SCREEN )
											{
												// ATE wanted this line here...
												pSoldier->pathing.usPathIndex--;
												AdjustSoldierPathToGoOffEdge( pSoldier, pSoldier->sGridNo, (UINT8)pSoldier->aiData.uiPendingActionData1 );
												continue;
											}
										}
										else if ( pSoldier->ubWaitActionToDo == 1 )
										{
											pSoldier->ubWaitActionToDo = 0;

											gbNumMercsUntilWaitingOver--;

											pSoldier->SoldierGotoStationaryStance( );

											// If we are at an exit-grid, make disappear.....
											if ( gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO )
											{
												// Remove!
												RemoveSoldierFromTacticalSector( pSoldier, TRUE );
											}
										}
									}
									else if ( pSoldier->aiData.ubPendingAction != NO_PENDING_ACTION )
									{
										DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("We are inside the IF PENDING Animation with soldier #%d", pSoldier->ubID) );

										if ( pSoldier->aiData.ubPendingAction == MERC_OPENDOOR ||	pSoldier->aiData.ubPendingAction == MERC_OPENSTRUCT )
										{
											sGridNo								 = pSoldier->aiData.sPendingActionData2;
											//usStructureID			= (UINT16)pSoldier->aiData.uiPendingActionData1;
											//pStructure = FindStructureByID( sGridNo, usStructureID );

											// LOOK FOR STRUCT OPENABLE
											pStructure = FindStructure( sGridNo, STRUCTURE_OPENABLE );

											if ( pStructure == NULL )
											{
#ifdef JA2BETAVERSION
												ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Told to open struct at %d and none was found", sGridNo );
#endif
												fKeepMoving = FALSE;
											}
											else
											{
												CalcInteractiveObjectAPs( pSoldier, sGridNo, pStructure, &sAPCost, &sBPCost ); // SANDRO - added argument

												if ( EnoughPoints( pSoldier, sAPCost, sBPCost , TRUE ) )
												{
													InteractWithInteractiveObject( pSoldier, pStructure, pSoldier->aiData.bPendingActionData3 );
												}
												else
												{
													fNoAPsForPendingAction = TRUE;
												}
											}
										}
										if ( pSoldier->aiData.ubPendingAction == MERC_PICKUPITEM	)
										{
											sGridNo								 = pSoldier->aiData.sPendingActionData2;

											if ( sGridNo == pSoldier->sGridNo )
											{
												// OK, now, if in realtime
												if ( !( gTacticalStatus.uiFlags & INCOMBAT ) )
												{
													// If the two gridnos are not the same, check to see if we can
													// now go into it
													if ( sGridNo != (pSoldier->aiData.uiPendingActionData4 ))
													{
														if ( NewOKDestination( pSoldier, pSoldier->aiData.uiPendingActionData4, TRUE, pSoldier->pathing.bLevel ) )
														{
															// GOTO NEW TILE!
															SoldierPickupItem( pSoldier, pSoldier->aiData.uiPendingActionData1, pSoldier->aiData.uiPendingActionData4, pSoldier->aiData.bPendingActionData3 );
															continue;
														}
													}
												}

												// OK MORON, DOUBLE CHECK THAT THE ITEM EXISTS HERE...
												if ( pSoldier->aiData.uiPendingActionData1 != ITEM_PICKUP_ACTION_ALL )
												{
													//if ( ItemExistsAtLocation( (INT16)( pSoldier->aiData.uiPendingActionData4 ), pSoldier->aiData.uiPendingActionData1, pSoldier->pathing.bLevel ) )
													{
														PickPickupAnimation( pSoldier, pSoldier->aiData.uiPendingActionData1, pSoldier->aiData.uiPendingActionData4, pSoldier->aiData.bPendingActionData3 );
													}
												}
												else
												{
													PickPickupAnimation( pSoldier, pSoldier->aiData.uiPendingActionData1, pSoldier->aiData.uiPendingActionData4, pSoldier->aiData.bPendingActionData3 );
												}
											}
											else
											{
												pSoldier->SoldierGotoStationaryStance( );
											}
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_PUNCH	)
										{
											// for the benefit of the AI
											pSoldier->aiData.bAction = AI_ACTION_KNIFE_STAB;

											pSoldier->EVENT_SoldierBeginPunchAttack( pSoldier->aiData.sPendingActionData2, pSoldier->aiData.bPendingActionData3 );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_TALK )
										{
											pSoldier->PlayerSoldierStartTalking( (UINT8)pSoldier->aiData.uiPendingActionData1, TRUE );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_DROPBOMB	)
										{
											pSoldier->EVENT_SoldierBeginDropBomb( );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_STEAL	)
										{
											//pSoldier->pathing.bDesiredDirection = pSoldier->aiData.bPendingActionData3;
											pSoldier->EVENT_SetSoldierDesiredDirection( pSoldier->aiData.bPendingActionData3 );

											pSoldier->EVENT_InitNewSoldierAnim( STEAL_ITEM, 0 , FALSE );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_KNIFEATTACK)
										{
											// for the benefit of the AI
											pSoldier->aiData.bAction = AI_ACTION_KNIFE_STAB;

											pSoldier->EVENT_SoldierBeginBladeAttack( pSoldier->aiData.sPendingActionData2, pSoldier->aiData.bPendingActionData3 );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_GIVEAID	)
										{
											pSoldier->EVENT_SoldierBeginFirstAid( pSoldier->aiData.sPendingActionData2, pSoldier->aiData.bPendingActionData3 );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_REPAIR	)
										{
											pSoldier->EVENT_SoldierBeginRepair( pSoldier->aiData.sPendingActionData2, pSoldier->aiData.bPendingActionData3 );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_FUEL_VEHICLE	)
										{
											pSoldier->EVENT_SoldierBeginRefuel( pSoldier->aiData.sPendingActionData2, pSoldier->aiData.bPendingActionData3 );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_RELOADROBOT	)
										{
											pSoldier->EVENT_SoldierBeginReloadRobot( pSoldier->aiData.sPendingActionData2, pSoldier->aiData.bPendingActionData3, (INT8)pSoldier->aiData.uiPendingActionData1 );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_TAKEBLOOD	)
										{
											pSoldier->EVENT_SoldierBeginTakeBlood( pSoldier->aiData.sPendingActionData2, pSoldier->aiData.bPendingActionData3 );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_ATTACH_CAN )
										{
											pSoldier->EVENT_SoldierBeginAttachCan( pSoldier->aiData.sPendingActionData2, pSoldier->aiData.bPendingActionData3 );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_ENTER_VEHICLE	)
										{
											pSoldier->EVENT_SoldierEnterVehicle( pSoldier->aiData.sPendingActionData2, pSoldier->aiData.bPendingActionData3 );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
											continue;
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_CUTFFENCE )
										{
											pSoldier->EVENT_SoldierBeginCutFence( pSoldier->aiData.sPendingActionData2, pSoldier->aiData.bPendingActionData3 );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
										}
										else if ( pSoldier->aiData.ubPendingAction == MERC_GIVEITEM	)
										{
											pSoldier->EVENT_SoldierBeginGiveItem( );
											pSoldier->aiData.ubPendingAction = NO_PENDING_ACTION;
										}

										if ( fNoAPsForPendingAction )
										{
											// Change status of guy to waiting
											HaltMoveForSoldierOutOfPoints( pSoldier );
											fKeepMoving = FALSE;
											pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
											pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
										}

									}
									else
									{
										// OK, ADJUST TO STANDING, WE ARE DONE
										// DO NOTHING IF WE ARE UNCONSCIOUS
										if ( pSoldier->stats.bLife >= OKLIFE )
										{
											if ( pSoldier->ubBodyType == CROW )
											{
												// If we are flying, don't stop!
												if ( pSoldier->sHeightAdjustment == 0 )
												{
													pSoldier->SoldierGotoStationaryStance( );
												}
											}
											else
											{
												UnSetUIBusy( pSoldier->ubID );

												pSoldier->SoldierGotoStationaryStance( );
											}
										}
									}

									// RESET MOVE FAST FLAG
									if ( (pSoldier->ubProfile == NO_PROFILE)	)
									{
										pSoldier->flags.fUIMovementFast = FALSE;
									}

									// if AI moving and waiting to process something at end of
									// move, have them handled the very next frame
									if (pSoldier->ubQuoteActionID == QUOTE_ACTION_ID_CHECKFORDEST)
									{
										pSoldier->aiData.fAIFlags |= AI_HANDLE_EVERY_FRAME;
									}

									fKeepMoving = FALSE;
								}
								else if ( !pSoldier->flags.fNoAPToFinishMove )
								{
									// Increment path....
									pSoldier->pathing.usPathIndex++;

									if ( pSoldier->pathing.usPathIndex > pSoldier->pathing.usPathDataSize )
									{
										pSoldier->pathing.usPathIndex = pSoldier->pathing.usPathDataSize;
									}

									// Are we at the end?
									if ( pSoldier->pathing.usPathIndex == pSoldier->pathing.usPathDataSize )
									{
										// ATE: Pop up warning....
										if ( pSoldier->pathing.usPathDataSize != MAX_PATH_LIST_SIZE )
										{
#ifdef JA2BETAVERSION
											ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Path for %s ( %d ) did not make merc get to dest (%d spaces away).", pSoldier->name, pSoldier->ubID, PythSpacesAway( pSoldier->pathing.sFinalDestination, pSoldier->sGridNo) );
#endif
										}

										// In case this is an AI person with the path-stored flag set,
										// turn it OFF since we have exhausted our stored path
										pSoldier->pathing.bPathStored = FALSE;										
										if (!TileIsOutOfBounds(pSoldier->sAbsoluteFinalDestination))
										{
											// We have not made it to our dest... but it's better to let the AI handle this itself,
											// on the very next fram
											pSoldier->aiData.fAIFlags |= AI_HANDLE_EVERY_FRAME;
										}
										else
										{
											// ATE: Added this to fcalilitate the fact
											// that our final dest may now have people on it....
											// 0verhaul:	But what if this is turn-based?	The soldier may be waiting forever for
											// the other one to leave the position, but since it's turn-based he never will.
											// Is that an issue here?
											// Yes, I think it is.	I found a situation where the soldier's path led through another
											// soldier.	The other soldier wasn't even at the destination but his path invariably
											// led through that soldier.	Since this was set up to allow that, he ended up deadlocked
											// in "take cover" mode waiting for the soldier right next to him to leave.
											// So in turn-based mode, we will just allow the soldier to re-think his next move.
											if ( gTacticalStatus.uiFlags & TURNBASED && gTacticalStatus.uiFlags & INCOMBAT)
											{
												ActionDone( pSoldier);
												pSoldier->SoldierGotoStationaryStance( );
												continue;
											}
											else
											{
												if ( FindBestPath( pSoldier, pSoldier->pathing.sFinalDestination, pSoldier->pathing.bLevel, pSoldier->usUIMovementMode, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) != 0 )
												{
												INT32 sNewGridNo;

												sNewGridNo = NewGridNo( pSoldier->sGridNo, DirectionInc( (UINT8)guiPathingData[ 0 ] ) );

													SetDelayedTileWaiting( pSoldier, sNewGridNo, 1 );
												}

												// We have not made it to our dest... set flag that we are waiting....
												if ( !pSoldier->EVENT_InternalGetNewSoldierPath( pSoldier->pathing.sFinalDestination, pSoldier->usUIMovementMode, 2, FALSE ) )
												{
													// ATE: To do here.... we could not get path, so we have to stop
													// 0verhaul:	May also need to clear the action type so that the soldier will know
													// to re-think another move instead of waiting for nothing to finish happening.
													pSoldier->SoldierGotoStationaryStance( );
													continue;
												}
											}
										}
									}
									else
									{
										// OK, Now find another dest grindo....
										if ( !( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_SPECIALMOVE ) )
										{
											// OK, now we want to see if we can continue to another tile...
											if ( !HandleGotoNewGridNo( pSoldier, &fKeepMoving, FALSE, pSoldier->usAnimState ) )
											{
												continue;
											}
										}
										else
										{
											// Change desired direction
											// Just change direction
											pSoldier->EVENT_InternalSetSoldierDestination( (UINT8) pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex ], FALSE, pSoldier->usAnimState );
										}

										if ( gTacticalStatus.bBoxingState != NOT_BOXING && (gTacticalStatus.bBoxingState == BOXING_WAITING_FOR_PLAYER || gTacticalStatus.bBoxingState == PRE_BOXING || gTacticalStatus.bBoxingState == BOXING) )
										{
											BoxingMovementCheck( pSoldier );
										}
									}
								}
							}

							if ( ( pSoldier->flags.uiStatusFlags & SOLDIER_PAUSEANIMOVE ) )
							{
								fKeepMoving = FALSE;
							}

							BOOLEAN executeCondition = FALSE;

							if (is_networked)
							{
								if ( !pSoldier->flags.fPausedMove && fKeepMoving && !pSoldier->flags.fNoAPToFinishMove )
									executeCondition = TRUE;
							}	
							else
							{
								if ( !pSoldier->flags.fPausedMove && fKeepMoving )
									executeCondition = TRUE;
							}

							// DO WALKING
							if ( executeCondition )
							{
								// Determine deltas
								//	dDeltaX = pSoldier->pathing.sDestXPos - pSoldier->dXPos;
								//dDeltaY = pSoldier->pathing.sDestYPos - pSoldier->dYPos;

								// Determine angle
								//	dAngle = (FLOAT)atan2( dDeltaX, dDeltaY );


								dAngle = gdRadiansForAngle[ pSoldier->bMovementDirection ];

								// For walking, base it on body type!
								if ( pSoldier->usAnimState == WALKING )
								{
									pSoldier->MoveMerc( gubAnimWalkSpeeds[ pSoldier->ubBodyType ].dMovementChange, dAngle, TRUE );

								}
								else
								{
									pSoldier->MoveMerc( gAnimControl[ pSoldier->usAnimState ].dMovementChange, dAngle, TRUE );
								}
							}

						}

						// Check for direction change
						if ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_TURNING )
						{
							pSoldier->TurnSoldier( );
						}
					}


#ifdef NETWORKED
					if(!pSoldier->flags.fNoAPToFinishMove )
						pSoldier->usLastUpdateTime = GetJA2Clock();
					if (pSoldier->flags.fSoldierUpdatedFromNetwork)
						UpdateSoldierFromNetwork(pSoldier);
#endif
//haydens network soldier update ->>
					if(is_client)UpdateSoldierToNetwork ( pSoldier );
//
				}

				}
				if ( !gfPauseAllAI &&
					( ((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT)) || (fHandleAI && guiAISlotToHandle == cnt) || (pSoldier->aiData.fAIFlags & AI_HANDLE_EVERY_FRAME) || gTacticalStatus.fAutoBandageMode ) )
				{
					HandleSoldierAI( pSoldier );
					if ( !((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT)) )
					{
						if (GetJA2Clock() - iTimerVal > RT_AI_TIMESLICE)
						{
							// don't do any more AI this time!
							fHandleAI = FALSE;
						}
						else
						{
							// we still have time to handle AI; skip to the next person
							fHandleAI = NextAIToHandle( guiAISlotToHandle );
						}
					}
				}

			}

		}

		if ( guiNumAwaySlots > 0 && !gfPauseAllAI && !(gTacticalStatus.uiFlags & INCOMBAT) && guiAISlotToHandle == HANDLE_OFF_MAP_MERC && guiAIAwaySlotToHandle != RESET_HANDLE_OF_OFF_MAP_MERCS )
		{
			pSoldier = AwaySlots[ guiAIAwaySlotToHandle ];

			// Syncronize for upcoming soldier counters
			SYNCTIMECOUNTER( );

			if ( pSoldier != NULL )
			{
				// the ONLY thing to do with away soldiers is process their schedule if they have one
				// and there is an action for them to do (like go on-sector)
				if (pSoldier->aiData.fAIFlags & AI_CHECK_SCHEDULE)
				{
					HandleSoldierAI( pSoldier );
				}
			}
		}

		// Turn off auto bandage if we need to...
		if ( gTacticalStatus.fAutoBandageMode )
		{
			if ( !CanAutoBandage( TRUE) )
			{
				SetAutoBandageComplete( );
			}
		}

		// Check if we should be doing a special event once guys get to a location...
		if ( gubWaitingForAllMercsToExitCode != 0 )
		{
			// Check if we have gone past our time...
			if ( ( GetJA2Clock( ) - guiWaitingForAllMercsToExitTimer ) > 2500 )
			{
				// OK, set num waiting to 0
#ifdef JA2BETAVERSION
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_DEBUG, L"Waiting too long for Mercs to exit...forcing entry." );
#endif
				gbNumMercsUntilWaitingOver = 0;

				// Reset all waitng codes
				for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
				{
					pSoldier = MercSlots[ cnt ];
					if ( pSoldier != NULL )
					{
						pSoldier->ubWaitActionToDo = 0;
					}
				}

			}
		}

		if ( gbNumMercsUntilWaitingOver == 0 )
		{
			// ATE: Unset flag to ignore sight...
			gTacticalStatus.uiFlags &= ( ~DISALLOW_SIGHT );


			// OK cheif, do something here....
			switch( gubWaitingForAllMercsToExitCode )
			{
			case WAIT_FOR_MERCS_TO_WALKOFF_SCREEN:

				if ( ( gTacticalStatus.ubCurrentTeam == gbPlayerNum ) )
				{
					guiPendingOverrideEvent = LU_ENDUILOCK;
					HandleTacticalUI( );
				}
				AllMercsHaveWalkedOffSector( );
				break;

			case WAIT_FOR_MERCS_TO_WALKON_SCREEN:

				// OK, unset UI
				if ( ( gTacticalStatus.ubCurrentTeam == gbPlayerNum ) )
				{
					guiPendingOverrideEvent = LU_ENDUILOCK;
					HandleTacticalUI( );
				}
				break;

			case WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO:

				// OK, unset UI
				if ( ( gTacticalStatus.ubCurrentTeam == gbPlayerNum ) )
				{
					guiPendingOverrideEvent = LU_ENDUILOCK;
					HandleTacticalUI( );
				}
				AllMercsWalkedToExitGrid( );
				break;

			}

			// ATE; Turn off tactical status flag...
			gTacticalStatus.uiFlags &= ( ~IGNORE_ALL_OBSTACLES );

			gubWaitingForAllMercsToExitCode = 0;
		}
	}


	// reset these AI-related global variables to 0 to ensure they don't interfere with the UI
	gubNPCAPBudget = 0;
	gubNPCDistLimit = 0;

	return( TRUE );

}

void HaltGuyFromNewGridNoBecauseOfNoAPs( SOLDIERTYPE *pSoldier )
{
	HaltMoveForSoldierOutOfPoints( pSoldier );
	pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
	pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
	pSoldier->aiData.ubPendingAction				= NO_PENDING_ACTION;

	UnMarkMovementReserved( pSoldier );

	// Display message if our merc...
	if ( pSoldier->bTeam == gbPlayerNum && ( gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ GUY_HAS_RUN_OUT_OF_APS_STR ], pSoldier->name );
	}

	UnSetUIBusy( pSoldier->ubID );

	// OK, Unset engaged in CONV, something changed...
	UnSetEngagedInConvFromPCAction( pSoldier );

}


void HandleLocateToGuyAsHeWalks( SOLDIERTYPE *pSoldier )
{
	// Our guys if option set,
	if ( pSoldier->bTeam == gbPlayerNum )
	{
		// IF tracking on, center on guy....
		if ( gGameSettings.fOptions[ TOPTION_TRACKING_MODE ] )
		{
			LocateSoldier( pSoldier->ubID, FALSE );
		}
	}
	else
	{
		// Others if visible...
		if ( pSoldier->bVisible != -1 )
		{
			// ATE: If we are visible, and have not already removed roofs, goforit
			if ( pSoldier->pathing.bLevel > 0 )
			{
				if ( !( gTacticalStatus.uiFlags & SHOW_ALL_ROOFS ) )
				{
					gTacticalStatus.uiFlags |= SHOW_ALL_ROOFS;
					SetRenderFlags( RENDER_FLAG_FULL );
				}
			}

			LocateSoldier( pSoldier->ubID, FALSE );
		}
	}
}

#pragma optimize("gpt",on)
__forceinline
BOOLEAN HandleGotoNewGridNo( SOLDIERTYPE *pSoldier, BOOLEAN *pfKeepMoving, BOOLEAN fInitialMove, UINT16 usAnimState )
{
	INT16							sAPCost;
	INT16							sBPCost;
	INT32                          usNewGridNo, sOverFenceGridNo, sMineGridNo;

	if (gTacticalStatus.uiFlags & INCOMBAT && fInitialMove )
	{
		HandleLocateToGuyAsHeWalks( pSoldier );
	}

	// Default to TRUE
	(*pfKeepMoving ) = TRUE;

	// Check for good breath....
	// if ( pSoldier->bBreath < OKBREATH && !fInitialMove )
	if ( pSoldier->bBreath < OKBREATH )
	{
		// OK, first check for b== 0
		// If our currentd state is moving already....( this misses the first tile, so the user
		// Sees some change in their click, but just one tile
		if ( pSoldier->bBreath == 0 )
		{
			// Collapse!
			pSoldier->bBreathCollapsed = TRUE;
			pSoldier->bEndDoorOpenCode = FALSE;

			if ( fInitialMove )
			{
				UnSetUIBusy( pSoldier->ubID );
			}

			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("HandleGotoNewGridNo() Failed: Out of Breath") );
			return( FALSE );
		}

		// OK, if we are collapsed now, check for OK breath instead...
		if ( pSoldier->bCollapsed )
		{
			// Collapse!
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("HandleGotoNewGridNo() Failed: Has Collapsed") );
			pSoldier->bBreathCollapsed = TRUE;
			pSoldier->bEndDoorOpenCode = FALSE;
			return( FALSE );
		}

	}

	usNewGridNo = NewGridNo( pSoldier->sGridNo, DirectionInc( (UINT8)pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex ] ) );

	// OK, check if this is a fence cost....
	if ( gubWorldMovementCosts[ usNewGridNo ][ (UINT8)pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex ] ][ pSoldier->pathing.bLevel ] == TRAVELCOST_FENCE )
	{
		// We have been told to jump fence....

		// Do we have APs?
		// CHRISL: Added penalty for jumping a fence while wearing a backpack
		// Do we have APs?
		// SANDRO - changed the static values to precise calculation here
		if((UsingNewInventorySystem() == true) && pSoldier->inv[BPACKPOCKPOS].exists() == true)
		{
			sAPCost = GetAPsToJumpFence( pSoldier, TRUE );
			sBPCost = GetBPsToJumpFence( pSoldier, TRUE );
		}
		else
		{
			sAPCost = GetAPsToJumpFence( pSoldier, FALSE );
			sBPCost = GetBPsToJumpFence( pSoldier, FALSE );
		}


		if ( EnoughPoints( pSoldier, sAPCost, sBPCost, FALSE )	)
		{
			// ATE: Check for tile being clear....
			sOverFenceGridNo = NewGridNo( usNewGridNo, DirectionInc( (UINT8)pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex + 1 ] ) );

			if ( HandleNextTile( pSoldier, (INT8)pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex + 1 ], sOverFenceGridNo, pSoldier->pathing.sFinalDestination ) )
			{
				// We do, adjust path data....
				pSoldier->pathing.usPathIndex++;
				// We go two, because we really want to start moving towards the NEXT gridno,
				// if we have any...

				// LOCK PENDING ACTION COUNTER
				pSoldier->flags.uiStatusFlags |= SOLDIER_LOCKPENDINGACTIONCOUNTER;

				pSoldier->SoldierGotoStationaryStance( );

				// OK, jump!
				pSoldier->BeginSoldierClimbFence( );

				pSoldier->flags.fContinueMoveAfterStanceChange = 2;
			}

		}
		else
		{
			HaltGuyFromNewGridNoBecauseOfNoAPs( pSoldier );
			(*pfKeepMoving ) = FALSE;
		}

		return( FALSE );
	}
	else if ( InternalDoorTravelCost( pSoldier, usNewGridNo, gubWorldMovementCosts[ usNewGridNo ][ (UINT8)pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex ] ][ pSoldier->pathing.bLevel ], (BOOLEAN) (pSoldier->bTeam == gbPlayerNum), NULL, TRUE ) == TRAVELCOST_DOOR )
	{
		STRUCTURE * pStructure;
		INT8				bDirection;
		INT32				sDoorGridNo;

		// OK, if we are here, we have been told to get a pth through a door.

		// No need to check if for AI

		// No need to check for right key ( since the path checks for that? )

		// Just for now play the $&&% animation
		bDirection = (UINT8)pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex ];

		// OK, based on the direction, get door gridno
		if ( bDirection == NORTH || bDirection == WEST )
		{
			sDoorGridNo = NewGridNo( pSoldier->sGridNo, DirectionInc( (UINT8)pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex ] ) );
		}
		else if ( bDirection == SOUTH || bDirection == EAST )
		{
			sDoorGridNo = pSoldier->sGridNo;
		}
		else
		{
#ifdef JA2TESTVERSION
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"ERROR: Invalid Direction to approach door. (Soldier loc: %d, dir: %d).", pSoldier->sGridNo, bDirection );
#endif
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("HandleGotoNewGridNo() Failed: Open door - invalid approach direction") );

			HaltGuyFromNewGridNoBecauseOfNoAPs( pSoldier );
			pSoldier->bEndDoorOpenCode = FALSE;
			(*pfKeepMoving ) = FALSE;
			return( FALSE );
		}

		// Get door
		pStructure = FindStructure( sDoorGridNo, STRUCTURE_ANYDOOR );

		if ( pStructure == NULL )
		{
#ifdef JA2TESTVERSION
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"ERROR: Told to open door that does not exist at %d.", sDoorGridNo );
#endif
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("HandleGotoNewGridNo() Failed: Door does not exist") );
			HaltGuyFromNewGridNoBecauseOfNoAPs( pSoldier );
			pSoldier->bEndDoorOpenCode = FALSE;
			(*pfKeepMoving ) = FALSE;
			return( FALSE );
		}

		// OK, open!
		StartInteractiveObject( sDoorGridNo, pStructure->usStructureID, pSoldier, bDirection );
		InteractWithInteractiveObject( pSoldier, pStructure, bDirection );

		// One needs to walk after....
		if ( (pSoldier->bTeam != gbPlayerNum) || (gTacticalStatus.fAutoBandageMode) || ( pSoldier->flags.uiStatusFlags & SOLDIER_PCUNDERAICONTROL ) )
		{
			pSoldier->bEndDoorOpenCode = 1;
			pSoldier->sEndDoorOpenCodeData = sDoorGridNo;
		}
		(*pfKeepMoving ) = FALSE;
		return( FALSE );
	}


	// Find out how much it takes to move here!
	sAPCost = ActionPointCost( pSoldier, usNewGridNo, (INT8)pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex ], usAnimState );
	sBPCost = TerrainBreathPoints( pSoldier, usNewGridNo, (INT8)pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex ], usAnimState );

	// CHECK IF THIS TILE IS A GOOD ONE!
	if ( !HandleNextTile( pSoldier, (INT8)pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex ], usNewGridNo, pSoldier->pathing.sFinalDestination ) )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "HandleGotoNewGridNo() Failed: Tile %d Was blocked", sNewGridNo ) );

		// ATE: If our own guy and an initial move.. display message
		//if ( fInitialMove && pSoldier->bTeam == gbPlayerNum	)
		//{
		//	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ NO_PATH_FOR_MERC ], pSoldier->name );
		//}

		pSoldier->bEndDoorOpenCode = FALSE;
		// GO on to next guy!
		return( FALSE );
	}


	// just check the tile we're going to walk into
	if ( NearbyGroundSeemsWrong( pSoldier, usNewGridNo, FALSE, &sMineGridNo ) )
	{
		if ( pSoldier->flags.uiStatusFlags & SOLDIER_PC )
		{
			// NearbyGroundSeemsWrong returns true with gridno NOWHERE if
			// we find something by metal detector... we should definitely stop
			// but we won't place a locator or say anything

			// IF not in combat, stop them all
			if ( !( gTacticalStatus.uiFlags & INCOMBAT ) )
			{
				INT32 cnt2;
				SOLDIERTYPE			 *pSoldier2;

				cnt2 = gTacticalStatus.Team[ gbPlayerNum ].bLastID;

				// look for all mercs on the same team,
				for ( pSoldier2 = MercPtrs[ cnt2 ]; cnt2 >= gTacticalStatus.Team[ gbPlayerNum ].bFirstID; cnt2-- ,pSoldier2-- )
				{
					if ( pSoldier2->bActive )
					{
						pSoldier2->EVENT_StopMerc( pSoldier2->sGridNo, pSoldier2->ubDirection );
					}
				}
			}
			else
			{
				pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );
			}

			(*pfKeepMoving) = FALSE;

			if (!TileIsOutOfBounds(sMineGridNo))
			{
				LocateGridNo( sMineGridNo );
				// we reuse the boobytrap gridno variable here
				gsBoobyTrapGridNo = sMineGridNo;
				gpBoobyTrapSoldier = pSoldier;
				// silversurfer: if TRUE the merc won't comment that he found a mine, otherwise old behaviour
				if (gGameExternalOptions.fMineSpottedNoTalk )
					MineSpottedDialogueCallBack ();
				else
				{
					SetStopTimeQuoteCallback( MineSpottedDialogueCallBack );
					TacticalCharacterDialogue( pSoldier, QUOTE_SUSPICIOUS_GROUND );
				}
			}
		}
		else
		{			
			if (!TileIsOutOfBounds(sMineGridNo))
			{
				pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );
				(*pfKeepMoving) = FALSE;

				if (pSoldier->bSide != 0)
				{
					gpWorldLevelData[ sMineGridNo ].uiFlags |= MAPELEMENT_ENEMY_MINE_PRESENT;
				}
				else
				{
					gpWorldLevelData[ sMineGridNo ].uiFlags |= MAPELEMENT_PLAYER_MINE_PRESENT;
				}

				// better stop and reconsider what to do...
				SetNewSituation( pSoldier );
				ActionDone( pSoldier );
				// HEADROCK HAM 3.6: Militia can now place flags when they spot landmines.
				if (gGameExternalOptions.fMilitiaPlaceBlueFlags &&
					pSoldier->bTeam == MILITIA_TEAM)
				{
					// This line causes the screen to focus on the gridno if it is not currently visible. 
					// Is it desirable when militia spot mines? Probably not. Turned off for now.
					//LocateGridNo( sMineGridNo );

					// Flash gridno
					ITEM_POOL *pItemPool = NULL;
					GetItemPool( sMineGridNo, &pItemPool, 0 );
					SetItemPoolLocator( pItemPool );
					// Add flag
					AddBlueFlag(sMineGridNo,1);
				}
			}
		}
	}


	// ATE: Check if we have sighted anyone, if so, don't do anything else...
	// IN other words, we have stopped from sighting...
	if ( pSoldier->flags.fNoAPToFinishMove && !fInitialMove )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("HandleGotoNewGridNo() Failed: No APs to finish move set") );
		pSoldier->bEndDoorOpenCode = FALSE;
		(*pfKeepMoving ) = FALSE;
	}
	else if ( pSoldier->pathing.usPathIndex == pSoldier->pathing.usPathDataSize && pSoldier->pathing.usPathDataSize == 0 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("HandleGotoNewGridNo() Failed: No Path") );
		pSoldier->bEndDoorOpenCode = FALSE;
		(*pfKeepMoving ) = FALSE;
	}
	//else if ( gTacticalStatus.fEnemySightingOnTheirTurn )
	//{
	// Hault guy!
	//	pSoldier->AdjustNoAPToFinishMove( TRUE );
	//	(*pfKeepMoving ) = FALSE;
	//}
	else if ( EnoughPoints( pSoldier, sAPCost, 0, FALSE )	)
	{
		BOOLEAN fDontContinue = FALSE;

		if ( pSoldier->pathing.usPathIndex > 0 )
		{
			// check for running into gas

			// note: this will have to use the minimum types of structures for tear/creature gas
			// since there isn't a way to retrieve the smoke effect structure
			if ( (gpWorldLevelData[ pSoldier->sGridNo ].ubExtFlags[pSoldier->pathing.bLevel] & ANY_SMOKE_EFFECT && PreRandom( 4 ) == 0 ) || gpWorldLevelData[ pSoldier->sGridNo ].ubExtFlags[pSoldier->pathing.bLevel] & MAPELEMENT_EXT_BURNABLEGAS )
			{
				EXPLOSIVETYPE *		pExplosive = NULL;
				INT8				bPosOfMask;

				bPosOfMask = FindGasMask (pSoldier);

				if(!DoesSoldierWearGasMask(pSoldier))//dnl ch40 200909
					bPosOfMask = NO_SLOT;

				// WANNE: Only apply the following code for soldiers and not the ROBOT!!
				// TODO: Madd: This next section is pretty lame because it can't figure out which explosive was used to actually cause a gas effect
				// so for now, the first explosive to use a gas effect decides the health and breath damage for all of the gasses of that type
				// (this was hard coded before by the developers - i guess they figured they didn't need to look for the actual explosion item,
				// since they only had one of each gas item?!?)
				// anyway, it means that we can only have one set of health/breath damage values for each gas type, until someone has time
				// to dig into this further and actually make it find the original item that caused the gas
				if ( !AM_A_ROBOT( pSoldier ) )
				{

					if ( gpWorldLevelData[ pSoldier->sGridNo ].ubExtFlags[pSoldier->pathing.bLevel] & MAPELEMENT_EXT_SMOKE )
					{
						if ( bPosOfMask == NO_SLOT && !(pSoldier->flags.fHitByGasFlags & HIT_BY_SMOKEGAS) )//dnl ch40 200909
						{
							pExplosive = &( Explosive[ Item[ GetFirstExplosiveOfType(EXPLOSV_SMOKE) ].ubClassIndex ]);
						}
					}

					//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Overhead pExplosive: %d", pExplosive->ubType );
					if ( gpWorldLevelData[ pSoldier->sGridNo ].ubExtFlags[pSoldier->pathing.bLevel] & MAPELEMENT_EXT_TEARGAS )
					{
						if ( !(pSoldier->flags.fHitByGasFlags & HIT_BY_TEARGAS) && bPosOfMask == NO_SLOT )
						{
							pExplosive = &( Explosive[ Item[ GetFirstExplosiveOfType(EXPLOSV_TEARGAS) ].ubClassIndex ]);
						}
					}

					//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Overhead pExplosive: %d", pExplosive->ubType );
					if ( gpWorldLevelData[ pSoldier->sGridNo ].ubExtFlags[pSoldier->pathing.bLevel] & MAPELEMENT_EXT_MUSTARDGAS )
					{
						if ( !(pSoldier->flags.fHitByGasFlags & HIT_BY_MUSTARDGAS) && bPosOfMask == NO_SLOT )
						{
							pExplosive = &(Explosive[ Item[ GetFirstExplosiveOfType(EXPLOSV_MUSTGAS) ].ubClassIndex ]);
						}
					}
		
					//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Overhead pExplosive: %d", pExplosive->ubType );
					if ( gpWorldLevelData[ pSoldier->sGridNo ].ubExtFlags[pSoldier->pathing.bLevel] & MAPELEMENT_EXT_CREATUREGAS )
					{
						if ( !(pSoldier->flags.fHitByGasFlags & HIT_BY_CREATUREGAS) ) // gas mask doesn't help vs creaturegas
						{
							pExplosive = &(Explosive[ Item[ GetFirstExplosiveOfType(EXPLOSV_CREATUREGAS) ].ubClassIndex ]);
						}
					}

					//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Overhead pExplosive: %d", pExplosive->ubType );
					if ( gpWorldLevelData[ pSoldier->sGridNo ].ubExtFlags[pSoldier->pathing.bLevel] & MAPELEMENT_EXT_BURNABLEGAS )
					{
						if ( !(pSoldier->flags.fHitByGasFlags & HIT_BY_BURNABLEGAS) )
						{
							pExplosive = &(Explosive[ Item[ GetFirstExplosiveOfType(EXPLOSV_BURNABLEGAS) ].ubClassIndex ]);
						}
					}

					//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Overhead pSoldier->flags.fHitByGasFlags: %d", pSoldier->flags.fHitByGasFlags );
					//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Overhead pExplosive: %d", pExplosive->ubType );

					if ( !(gpWorldLevelData[ pSoldier->sGridNo ].ubExtFlags[pSoldier->pathing.bLevel] & MAPELEMENT_EXT_SMOKE ))
					{
						if ( pExplosive )
						{
							pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );
							fDontContinue = TRUE;

							DishOutGasDamage( pSoldier, pExplosive, TRUE, FALSE,
								(INT16) (pExplosive->ubDamage + (UINT8)PreRandom( pExplosive->ubDamage ) ),
								(INT16) (100 * ( pExplosive->ubStunDamage + (INT16)PreRandom( ( pExplosive->ubStunDamage / 2 ) ) ) ), NOBODY );
						}
					}
				}
			}

			if ( !fDontContinue )
			{

				if ( (pSoldier->bOverTerrainType == FLAT_FLOOR || pSoldier->bOverTerrainType == PAVED_ROAD) && pSoldier->pathing.bLevel == 0 )
				{
					INT32	iMarblesIndex;

					if ( MarblesExistAtLocation( pSoldier->sGridNo, 0, &iMarblesIndex ) )
					{
						// Slip on marbles!
						pSoldier->DoMercBattleSound( BATTLE_SOUND_CURSE1 );
						if ( pSoldier->bTeam == gbPlayerNum )
						{
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, Message[ STR_SLIPPED_MARBLES ], pSoldier->name );
						}
						RemoveItemFromPool( pSoldier->sGridNo, iMarblesIndex, 0 );
						SoldierCollapse( pSoldier );
						if (pSoldier->bActionPoints > 0)
						{
							pSoldier->bActionPoints -= (INT8) (Random( pSoldier->bActionPoints ) + 1);
						}
						return( FALSE );
					}
				}

				if ( (pSoldier->bBlindedCounter > 0) && (pSoldier->usAnimState == RUNNING) && (Random( 5 ) == 0) &&
					OKFallDirection( pSoldier, (pSoldier->sGridNo + DirectionInc( pSoldier->ubDirection ) ), pSoldier->pathing.bLevel, pSoldier->ubDirection, pSoldier->usAnimState ) )
				{
					// 20% chance of falling over!
					pSoldier->DoMercBattleSound( BATTLE_SOUND_CURSE1 );
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[ 37 ], pSoldier->name );
					SoldierCollapse( pSoldier );
					if (pSoldier->bActionPoints > 0)
					{
						pSoldier->bActionPoints -= (INT8) (Random( pSoldier->bActionPoints ) + 1);
					}
					return( FALSE );
				}
				else if ( ( GetDrunkLevel( pSoldier ) == DRUNK ) && (Random( 5 ) == 0) &&
					OKFallDirection( pSoldier, (pSoldier->sGridNo + DirectionInc( pSoldier->ubDirection ) ), pSoldier->pathing.bLevel, pSoldier->ubDirection, pSoldier->usAnimState ) )
				{
					// 20% chance of falling over!
					pSoldier->DoMercBattleSound( BATTLE_SOUND_CURSE1 );
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[ 37 ], pSoldier->name );
					SoldierCollapse( pSoldier );
					if (pSoldier->bActionPoints > 0)
					{
						pSoldier->bActionPoints -= (INT8) (Random( pSoldier->bActionPoints ) + 1);
					}
					return( FALSE );
				}
				else
					// ATE; First check for profile
					// Forgetful guy might forget his path
					if ( (pSoldier->bTeam == gbPlayerNum) && ( pSoldier->ubProfile != NO_PROFILE ) && gMercProfiles[pSoldier->ubProfile].bDisability == FORGETFUL )
					{
						if ( pSoldier->ubNumTilesMovesSinceLastForget < 255 )
						{
							pSoldier->ubNumTilesMovesSinceLastForget++;
						}

						if ( pSoldier->pathing.usPathIndex > 2 && (Random( 100 ) == 0) && pSoldier->ubNumTilesMovesSinceLastForget > 200 )
						{
							pSoldier->ubNumTilesMovesSinceLastForget = 0;

							TacticalCharacterDialogue( pSoldier, QUOTE_PERSONALITY_TRAIT );
							pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );
							if (pSoldier->bActionPoints > 0)
							{
								pSoldier->bActionPoints -= (INT8) (Random( pSoldier->bActionPoints ) + 1);
							}

							fDontContinue = TRUE;

							UnSetUIBusy( pSoldier->ubID	);
						}
					}

			}
		}

		if ( !fDontContinue )
		{
			// Don't apply the first deduction in points...
			if ( usAnimState == CRAWLING && pSoldier->flags.bTurningFromPronePosition > TURNING_FROM_PRONE_ON )
			{
			}
			else
			{
				// Adjust AP/Breathing points to move
				DeductPoints( pSoldier, sAPCost, sBPCost );
			}

			// OK, let's check for monsters....
			if (pSoldier->flags.uiStatusFlags & SOLDIER_MONSTER)
			{
				if ( !ValidCreatureTurn( pSoldier, ( INT8 ) ( pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex ] ) ) )
				{
					if ( !pSoldier->bReverse )
					{
						pSoldier->bReverse = TRUE;

						if ( pSoldier->ubBodyType == INFANT_MONSTER )
						{
							pSoldier->ChangeSoldierState( WALK_BACKWARDS, 1, TRUE );
						}
						else
						{
							pSoldier->ChangeSoldierState( MONSTER_WALK_BACKWARDS, 1, TRUE );
						}
					}
				}
				else
				{
					pSoldier->bReverse = FALSE;
				}
			}

			// OK, let's check for monsters....
			if (pSoldier->ubBodyType == BLOODCAT )
			{
				if ( !ValidCreatureTurn( pSoldier, ( INT8 ) ( pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex ] ) ) )
				{
					if ( !pSoldier->bReverse )
					{
						pSoldier->bReverse = TRUE;
						pSoldier->ChangeSoldierState( BLOODCAT_WALK_BACKWARDS, 1, TRUE );
					}
				}
				else
				{
					pSoldier->bReverse = FALSE;
				}
			}

			// Change desired direction
			pSoldier->EVENT_InternalSetSoldierDestination( (UINT8) pSoldier->pathing.usPathingData[ pSoldier->pathing.usPathIndex ], fInitialMove, usAnimState );		

			// CONTINUE
			// IT'S SAVE TO GO AGAIN, REFRESH flag
			pSoldier->AdjustNoAPToFinishMove( FALSE );
		}
	}
	else
	{
		// HALT GUY HERE
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("HandleGotoNewGridNo() Failed: No APs %d %d", sAPCost, pSoldier->bActionPoints ) );
		HaltGuyFromNewGridNoBecauseOfNoAPs( pSoldier );
		pSoldier->bEndDoorOpenCode = FALSE;
		(*pfKeepMoving ) = FALSE;
	}

	return( TRUE );
}

void HandleMaryArrival( SOLDIERTYPE * pSoldier )
{
	INT32 sDist;

	if ( !pSoldier )
	{
		pSoldier = FindSoldierByProfileID( MARY, TRUE );
		if ( !pSoldier )
		{
			return;
		}
	}

	if ( CheckFact( FACT_JOHN_ALIVE, 0 ) )
	{
		return;
	}
	// new requirements: player close by

	// TODO.WANNE: Hardcoded grid number
	else if (PythSpacesAway( pSoldier->sGridNo, 8228 ) < 40)
	{		
		if ( !TileIsOutOfBounds(ClosestPC( pSoldier, &sDist )) && sDist > NPC_TALK_RADIUS * 2 )
		{
			// too far away
			return;
		}

		// Mary has arrived
		SetFactTrue( FACT_MARY_OR_JOHN_ARRIVED );

		pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );

		TriggerNPCRecord( MARY, 13 );
	}

}

void HandleJohnArrival( SOLDIERTYPE * pSoldier )
{
	SOLDIERTYPE * pSoldier2 = NULL;
	INT32 sDist;

	if ( !pSoldier )
	{
		pSoldier = FindSoldierByProfileID( JOHN, TRUE );
		if ( !pSoldier )
		{
			return;
		}
	}
	
	// TODO.WANNE: Hardcoded grid number
	if (PythSpacesAway( pSoldier->sGridNo, 8228 ) < 40)
	{		
		if (!TileIsOutOfBounds(ClosestPC( pSoldier, &sDist )) && sDist > NPC_TALK_RADIUS * 2 )
		{
			// too far away
			return;
		}

		if ( CheckFact( FACT_MARY_ALIVE, 0 ) )
		{
			pSoldier2 = FindSoldierByProfileID( MARY, FALSE );
			if ( pSoldier2 )
			{
				if ( PythSpacesAway( pSoldier->sGridNo, pSoldier2->sGridNo ) > 8 )
				{
					// too far away!
					return;
				}
			}
		}

		SetFactTrue( FACT_MARY_OR_JOHN_ARRIVED );

		pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );

		// if Mary is alive/dead
		if ( pSoldier2 )
		{
			pSoldier2->EVENT_StopMerc( pSoldier2->sGridNo, pSoldier2->ubDirection );
			TriggerNPCRecord( JOHN, 13 );
		}
		else
		{
			TriggerNPCRecord( JOHN, 12 );
		}
	}
}


BOOLEAN HandleAtNewGridNo( SOLDIERTYPE *pSoldier, BOOLEAN *pfKeepMoving )
{
	INT32								sMineGridNo;
	UINT8							 ubVolume;

		//uses Lua
		PROFILLUA_sSectorX = pSoldier->sSectorX;
		PROFILLUA_sSectorY = pSoldier->sSectorY;
		PROFILLUA_bSectorZ = pSoldier->bSectorZ;
		PROFILLUA_Level = pSoldier->pathing.bLevel;
		PROFILLUA_ubID = pSoldier->ubID;
		PROFILLUA_sGridNo = pSoldier->sGridNo;
		PROFILLUA_ubDirectiono = pSoldier->ubDirection;
		PROFILLUA_bTeam = pSoldier->bTeam;
	
	// ATE; Handle bad guys, as they fade, to cancel it if
	// too long...
	// ONLY if fading IN!
	if ( pSoldier->flags.fBeginFade	== 1 )
	{
		if ( pSoldier->sLocationOfFadeStart != pSoldier->sGridNo )
		{
			// Turn off
			pSoldier->flags.fBeginFade = FALSE;

			if ( pSoldier->pathing.bLevel > 0 && gpWorldLevelData[pSoldier->sGridNo].pRoofHead != NULL )
			{
				pSoldier->ubFadeLevel = gpWorldLevelData[ pSoldier->sGridNo ].pRoofHead->ubShadeLevel;
			}
			else
			{
				pSoldier->ubFadeLevel = gpWorldLevelData[ pSoldier->sGridNo ].pLandHead->ubShadeLevel;
			}

			// Set levelnode shade level....
			if ( pSoldier->pLevelNode )
			{
				pSoldier->pLevelNode->ubShadeLevel = pSoldier->ubFadeLevel;
			}
			pSoldier->bVisible = -1;

		}
	}

	if (gTacticalStatus.uiFlags & INCOMBAT )
	{
		HandleLocateToGuyAsHeWalks( pSoldier );
	}

	// Default to TRUE
	(*pfKeepMoving ) = TRUE;

	pSoldier->bTilesMoved++;
	if ( pSoldier->usAnimState == RUNNING )
	{
		// count running as double
		pSoldier->bTilesMoved++;
	}

	// First if we are in realtime combat or noncombat
	if ( (gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		// Update value for RT breath update
		pSoldier->ubTilesMovedPerRTBreathUpdate++;
		// Update last anim
		pSoldier->usLastMovementAnimPerRTBreathUpdate = pSoldier->usAnimState;
	}

	// Update path if showing path in RT
	if ( gGameSettings.fOptions[ TOPTION_ALWAYS_SHOW_MOVEMENT_PATH ] )
	{
		if ( !(gTacticalStatus.uiFlags & INCOMBAT ) )
		{
			gfPlotNewMovement	= TRUE;
		}
	}

	// ATE: Put some stuff in here to not handle certain things if we are
	// trversing...
	if ( gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALKOFF_SCREEN || gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO )
	{
		return( TRUE );
	}

	// Check if they are out of breath
	if ( pSoldier->CheckForBreathCollapse( ) )
	{
		(*pfKeepMoving ) = TRUE;
		return( FALSE );
	}

	// see if a mine gets set off...
	if (SetOffBombsInGridNo( pSoldier->ubID, pSoldier->sGridNo, FALSE, pSoldier->pathing.bLevel ) )
	{
		(*pfKeepMoving) = FALSE;
		pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );
		return( FALSE );
	}


	// Set "interrupt occurred" flag to false so that we will know whether *this
	// particular call* to HandleSight caused an interrupt
	gTacticalStatus.fInterruptOccurred = FALSE;

	if ( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		ubVolume = MovementNoise( pSoldier );
		if (ubVolume > 0)
		{
			MakeNoise( pSoldier->ubID, pSoldier->sGridNo, pSoldier->pathing.bLevel, pSoldier->bOverTerrainType, ubVolume, NOISE_MOVEMENT );
			if ( (pSoldier->flags.uiStatusFlags & SOLDIER_PC) && (pSoldier->bStealthMode) )
			{
				PlayStealthySoldierFootstepSound( pSoldier );
			}
		}
	}

	// ATE: Make sure we don't make another interrupt...
	if ( !gTacticalStatus.fInterruptOccurred )
	{
		// Handle New sight
		HandleSight(pSoldier,SIGHT_LOOK | SIGHT_RADIO | SIGHT_INTERRUPT);
	}

	// ATE: Check if we have sighted anyone, if so, don't do anything else...
	// IN other words, we have stopped from sighting...
	if (gTacticalStatus.fInterruptOccurred)
	{
		// Unset no APs value
		pSoldier->AdjustNoAPToFinishMove( TRUE );

		(*pfKeepMoving ) = FALSE;
		pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
		pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;

		// ATE: Cancel only if our final destination
		if ( pSoldier->sGridNo == pSoldier->pathing.sFinalDestination )
		{
			pSoldier->aiData.ubPendingAction				= NO_PENDING_ACTION;
		}

		// this flag is set only to halt the currently moving guy; reset it now
		gTacticalStatus.fInterruptOccurred = FALSE;

		// ATE: Remove this if we were stopped....
		if ( gTacticalStatus.fEnemySightingOnTheirTurn )
		{
			if ( gTacticalStatus.ubEnemySightingOnTheirTurnEnemyID == pSoldier->ubID )
			{
				pSoldier->flags.fPauseAllAnimation = FALSE;
				gTacticalStatus.fEnemySightingOnTheirTurn = FALSE;
			}
		}
	}
	else if ( pSoldier->flags.fNoAPToFinishMove )
	{
		(*pfKeepMoving ) = FALSE;
	}
	else if ( pSoldier->pathing.usPathIndex == pSoldier->pathing.usPathDataSize && pSoldier->pathing.usPathDataSize == 0 )
	{
		(*pfKeepMoving ) = FALSE;
	}
	else if ( gTacticalStatus.fEnemySightingOnTheirTurn )
	{
		// Hault guy!
		pSoldier->AdjustNoAPToFinishMove( TRUE );
		(*pfKeepMoving ) = FALSE;
	}

	// OK, check for other stuff like mines...
	if (NearbyGroundSeemsWrong( pSoldier, pSoldier->sGridNo, TRUE, &sMineGridNo ))
	{
		if (pSoldier->flags.uiStatusFlags & SOLDIER_PC)
		{
			// NearbyGroundSeemsWrong returns true with gridno NOWHERE if
			// we find something by metal detector... we should definitely stop
			// but we won't place a locator or say anything

			// IF not in combat, stop them all
			if ( !( gTacticalStatus.uiFlags & INCOMBAT ) )
			{
				INT32 cnt2;
				SOLDIERTYPE			 *pSoldier2;

				cnt2 = gTacticalStatus.Team[ gbPlayerNum ].bLastID;

				// look for all mercs on the same team,
				for ( pSoldier2 = MercPtrs[ cnt2 ]; cnt2 >= gTacticalStatus.Team[ gbPlayerNum ].bFirstID; cnt2-- ,pSoldier2-- )
				{
					if ( pSoldier2->bActive )
					{
						pSoldier2->EVENT_StopMerc( pSoldier2->sGridNo, pSoldier2->ubDirection );
					}
				}
			}
			else
			{
				pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );
			}

			(*pfKeepMoving) = FALSE;
			
			if (!TileIsOutOfBounds(sMineGridNo))
			{
				LocateGridNo( sMineGridNo );
				// we reuse the boobytrap gridno variable here
				gsBoobyTrapGridNo = sMineGridNo;
				gpBoobyTrapSoldier = pSoldier;
				// silversurfer: if TRUE the merc won't comment that he found a mine, otherwise old behaviour
				if (gGameExternalOptions.fMineSpottedNoTalk )
					MineSpottedDialogueCallBack ();
				else
				{
					SetStopTimeQuoteCallback( MineSpottedDialogueCallBack );
					TacticalCharacterDialogue( pSoldier, QUOTE_SUSPICIOUS_GROUND );
				}
			}
		}
		else
		{			
			if (!TileIsOutOfBounds(sMineGridNo))
			{

				pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );
				(*pfKeepMoving) = FALSE;

				gpWorldLevelData[ sMineGridNo ].uiFlags |= MAPELEMENT_ENEMY_MINE_PRESENT;

				// better stop and reconsider what to do...
				SetNewSituation( pSoldier );
				ActionDone( pSoldier );
			}
		}
	}

	HandleSystemNewAISituation( pSoldier, FALSE );

	
#ifdef LUA_OVERHEAD
	LetLuaMyCustomHandleAtNewGridNo(NULL,pSoldier->ubProfile, 0);
#else
	//----------------Lua------------------------
	
	
	if ( pSoldier->bTeam == gbPlayerNum )
	{
		if ( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC)
		{
			// are we there yet?
			if (pSoldier->sSectorX == 13 && pSoldier->sSectorY == MAP_ROW_B && pSoldier->bSectorZ == 0)
			{
				switch( pSoldier->ubProfile )
				{
				case SKYRIDER:
					if (PythSpacesAway( pSoldier->sGridNo, 8842 ) < 11)
					{
						// Skyrider has arrived!
						pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );
						SetFactTrue( FACT_SKYRIDER_CLOSE_TO_CHOPPER );
						TriggerNPCRecord( SKYRIDER, 15 );
						SetUpHelicopterForPlayer( 13, MAP_ROW_B, SKYRIDER );
					}
					break;

				case MARY:
					HandleMaryArrival( pSoldier );
					break;

				case JOHN:
					HandleJohnArrival( pSoldier );
					break;
				}
			}
			else if ( pSoldier->ubProfile == MARIA && (pSoldier->sSectorX == 6 && pSoldier->sSectorY == MAP_ROW_C && pSoldier->bSectorZ == 0) && CheckFact( FACT_MARIA_ESCORTED_AT_LEATHER_SHOP, MARIA ) == TRUE )
			{
				// check that Angel is there!
				if ( NPCInRoom( ANGEL, 2 ) ) // room 2 is leather shop
				{
					//	UnRecruitEPC( MARIA );
					TriggerNPCRecord( ANGEL, 12 );
				}
			}
			else if ( ( pSoldier->ubProfile == JOEY ) && (pSoldier->sSectorX == 8 && pSoldier->sSectorY == MAP_ROW_G && pSoldier->bSectorZ == 0 ) )
			{
				// if Joey walks near Martha then trigger Martha record 7
				if ( CheckFact( FACT_JOEY_NEAR_MARTHA, 0 ) )
				{
					pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );
					TriggerNPCRecord( JOEY, 9 );
				}
			}

		}
		// Drassen stuff for John & Mary
		else if ( gubQuest[ QUEST_ESCORT_TOURISTS ] == QUESTINPROGRESS && pSoldier->sSectorX == 13 && pSoldier->sSectorY == MAP_ROW_B && pSoldier->bSectorZ == 0 )
		{
			if ( CheckFact( FACT_JOHN_ALIVE, 0 ) )
			{
				HandleJohnArrival( NULL );
			}
			else
			{
				HandleMaryArrival( NULL );
			}
		}

	}
	else if ( pSoldier->bTeam == CIV_TEAM && pSoldier->ubProfile != NO_PROFILE && pSoldier->aiData.bNeutral )
	{
		switch( pSoldier->ubProfile )
		{
		case JIM:
		case JACK:
		case OLAF:
		case RAY:
		case OLGA:
		case TYRONE:
			{
				INT32 sDesiredMercDist;
				
				if (!TileIsOutOfBounds(ClosestPC( pSoldier, &sDesiredMercDist )))
				{
					if ( sDesiredMercDist <= NPC_TALK_RADIUS * 2 )
					{
						// stop
						CancelAIAction( pSoldier, TRUE );
						// aaaaaaaaaaaaaaaaaaaaatttaaaack!!!!
						AddToShouldBecomeHostileOrSayQuoteList( pSoldier->ubID );
						//MakeCivHostile( pSoldier, 2 );
						//TriggerNPCWithIHateYouQuote( pSoldier->ubProfile );
					}
				}
			}
			break;
		default:
			break;
		}
	}

#endif

	return( TRUE );
}


void SelectNextAvailSoldier( SOLDIERTYPE *pSoldier )
{
	INT32							cnt;
	SOLDIERTYPE			 *pTeamSoldier;
	BOOLEAN				 fSoldierFound = FALSE;
	DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("SelectNextAvailSoldier"));

	// IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
	cnt = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID;

	// look for all mercs on the same team,
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++,pTeamSoldier++)
	{
		if ( OK_CONTROLLABLE_MERC( pTeamSoldier )	)
		{
			fSoldierFound = TRUE;
			break;
		}
	}

	if ( fSoldierFound )
	{
		DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("SelectNextAvailSoldier: selectsoldier"));
		SelectSoldier( (INT16)cnt, FALSE, FALSE );
	}
	else
	{
		gusSelectedSoldier = NOBODY;
		// Change UI mode to reflact that we are selected
		guiPendingOverrideEvent = I_ON_TERRAIN;
	}

	DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("SelectNextAvailSoldier: done"));
}




void InternalSelectSoldier( UINT16 usSoldierID, BOOLEAN fAcknowledge, BOOLEAN fForceReselect, BOOLEAN fFromUI )
{
	SOLDIERTYPE			 *pSoldier, *pOldSoldier;

	// ARM: can't call SelectSoldier() in mapscreen, that will initialize interface panels!!!
	// ATE: Adjusted conditions a bit ( sometimes were not getting selected )
	if ( guiCurrentScreen == LAPTOP_SCREEN || guiCurrentScreen == MAP_SCREEN )
	{
		return;
	}

	if ( usSoldierID == NOBODY )
	{
		return;
	}

	//if we are in the shop keeper interface
	if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
	{
		//dont allow the player to change the selected merc
		return;
	}


	// Get guy
	pSoldier = MercPtrs[ usSoldierID ];


	// If we are dead, ignore
	if ( !OK_CONTROLLABLE_MERC( pSoldier) )
	{
		return;
	}

	// Don't do it if we don't have an interrupt
	if ( !OK_INTERRUPT_MERC( pSoldier ) )
	{
		// OK, we want to display message that we can't....
		if ( fFromUI )
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ MERC_IS_UNAVAILABLE_STR ], pSoldier->name );
		}
		return;
	}

	if ( pSoldier->ubID == gusSelectedSoldier )
	{
		if ( !fForceReselect )
		{
			return;
		}
	}

	// CANCEL FROM PLANNING MODE!
	if ( InUIPlanMode( ) )
	{
		EndUIPlan( );
	}

	// Unselect old selected guy
	if ( gusSelectedSoldier != NOBODY )
	{
		// Get guy
		pOldSoldier = MercPtrs[ gusSelectedSoldier ];
		pOldSoldier->flags.fShowLocator		= FALSE;
		pOldSoldier->flags.fFlashLocator = FALSE;

		// DB This used to say pSoldier... I fixed it
		if ( pOldSoldier->pathing.bLevel == 0 )
		{
			//ConcealWalls((INT16)(pSoldier->dXPos/CELL_X_SIZE), (INT16)(pSoldier->dYPos/CELL_Y_SIZE), REVEAL_WALLS_RADIUS);
			//	ApplyTranslucencyToWalls((INT16)(pOldSoldier->dXPos/CELL_X_SIZE), (INT16)(pOldSoldier->dYPos/CELL_Y_SIZE));
			//LightHideTrees((INT16)(pOldSoldier->dXPos/CELL_X_SIZE), (INT16)(pOldSoldier->dYPos/CELL_Y_SIZE));
		}
		//DeleteSoldierLight( pOldSoldier );

		if(pOldSoldier->flags.uiStatusFlags&SOLDIER_GREEN_RAY)
		{
			LightHideRays((INT16)(pOldSoldier->dXPos/CELL_X_SIZE), (INT16)(pOldSoldier->dYPos/CELL_Y_SIZE));
			pOldSoldier->flags.uiStatusFlags &= (~SOLDIER_GREEN_RAY);
		}

		UpdateForContOverPortrait( pOldSoldier, FALSE );
	}

	gusSelectedSoldier = (UINT16)usSoldierID;

	// find which squad this guy is, then set selected squad to this guy
	SetCurrentSquad( pSoldier -> bAssignment, FALSE );

	if ( pSoldier->pathing.bLevel == 0 )
	{
		//RevealWalls((INT16)(pSoldier->dXPos/CELL_X_SIZE), (INT16)(pSoldier->dYPos/CELL_Y_SIZE), REVEAL_WALLS_RADIUS);
		//	CalcTranslucentWalls((INT16)(pSoldier->dXPos/CELL_X_SIZE), (INT16)(pSoldier->dYPos/CELL_Y_SIZE));
		//LightTranslucentTrees((INT16)(pSoldier->dXPos/CELL_X_SIZE), (INT16)(pSoldier->dYPos/CELL_Y_SIZE));
	}

	//pSoldier->SetCheckSoldierLightFlag( );

	// Set interface to reflect new selection!
	SetCurrentTacticalPanelCurrentMerc( (UINT8)usSoldierID );

	// PLay ATTN SOUND
	if ( fAcknowledge )
	{
		if( !gGameSettings.fOptions[ TOPTION_MUTE_CONFIRMATIONS ] )
			pSoldier->DoMercBattleSound( BATTLE_SOUND_ATTN1 );
	}

	// Change UI mode to reflact that we are selected
	// NOT if we are locked inthe UI
	if ( gTacticalStatus.ubCurrentTeam == OUR_TEAM && gCurrentUIMode != LOCKUI_MODE && gCurrentUIMode != LOCKOURTURN_UI_MODE )
	{
		guiPendingOverrideEvent = M_ON_TERRAIN;
	}

	ChangeInterfaceLevel( pSoldier->pathing.bLevel );

	if( pSoldier->flags.fMercAsleep )
	{
		PutMercInAwakeState( pSoldier );
	}

	// possibly say personality quote
	if ( (pSoldier->bTeam == gbPlayerNum) && (pSoldier->ubProfile != NO_PROFILE && pSoldier->ubWhatKindOfMercAmI != MERC_TYPE__PLAYER_CHARACTER) && !( pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_PERSONALITY) )
	{
		switch ( gMercProfiles[ pSoldier->ubProfile ].bDisability )
		{
		case PSYCHO:
			if ( Random( 50 ) == 0 )
			{
				TacticalCharacterDialogue( pSoldier, QUOTE_PERSONALITY_TRAIT );
				pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_PERSONALITY;
			}
			break;
		default:
			break;
		}
	}

	UpdateForContOverPortrait( pSoldier, TRUE );

	// Remove any interactive tiles we could be over!
	BeginCurInteractiveTileCheck( INTILE_CHECK_SELECTIVE );

}

void SelectSoldier( UINT16 usSoldierID, BOOLEAN fAcknowledge, BOOLEAN fForceReselect )
{
	InternalSelectSoldier( usSoldierID, fAcknowledge, fForceReselect, FALSE );
}


BOOLEAN ResetAllAnimationCache(	)
{
	UINT32							cnt;
	SOLDIERTYPE			 *pSoldier;

	// Loop through all mercs and make go
	for ( pSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pSoldier++, cnt++ )
	{
		if ( pSoldier != NULL )
		{
			InitAnimationCache( (UINT16)cnt, &(pSoldier->AnimCache) );

		}

	}

	return( TRUE );
}




void LocateSoldier( UINT16 usID, BOOLEAN fSetLocator)
{
	SOLDIERTYPE *pSoldier;
	INT16 sNewCenterWorldX, sNewCenterWorldY;

	//if (!bCenter && SoldierOnScreen(usID))
	//return;

	// do we need to move the screen?
	//ATE: Force this baby to locate if told to
	if (!SoldierOnScreen(usID) || fSetLocator == 10 )
	{
		// Get pointer of soldier
		pSoldier = MercPtrs[ usID ];

		// Center on guy
		sNewCenterWorldX = (INT16)pSoldier->dXPos;
		sNewCenterWorldY = (INT16)pSoldier->dYPos;

		SetRenderCenter( sNewCenterWorldX, sNewCenterWorldY );

		// Plot new path!
		gfPlotNewMovement = TRUE;
	}


	// do we flash the name & health bars/health string above?
	if (fSetLocator)
	{
		if ( fSetLocator == SETLOCATOR || fSetLocator == 10 )
		{
			ShowRadioLocator((UINT8)usID, SHOW_LOCATOR_NORMAL );
		}
		else
		{
			ShowRadioLocator((UINT8)usID, SHOW_LOCATOR_FAST );
		}
	}
}


void InternalLocateGridNo( INT32 sGridNo, BOOLEAN fForce )
{
	INT16 sNewCenterWorldX, sNewCenterWorldY;

	ConvertGridNoToCenterCellXY( sGridNo, &sNewCenterWorldX, &sNewCenterWorldY );

	// FIRST CHECK IF WE ARE ON SCREEN
	if ( GridNoOnScreen( sGridNo ) && !fForce )
	{
		return;
	}

	SetRenderCenter( sNewCenterWorldX, sNewCenterWorldY );
}

void LocateGridNo( INT32 sGridNo )
{
	InternalLocateGridNo( sGridNo, FALSE );
}



void SlideTo(INT32 sGridNo, UINT16 usSoldierID , UINT16 usReasonID, BOOLEAN fSetLocator)
{
	INT32 cnt;


	if ( usSoldierID == NOBODY )
	{
		return;
	}

	if ( fSetLocator == SETANDREMOVEPREVIOUSLOCATOR )
	{
		for ( cnt = 0; cnt < TOTAL_SOLDIERS; cnt++ )
		{
			if ( MercPtrs[ cnt ]->bActive && MercPtrs[ cnt ]->bInSector )
			{
				// Remove all existing locators...
				MercPtrs[ cnt ]->flags.fFlashLocator = FALSE;
			}
		}
	}

	// Locate even if on screen
	if (fSetLocator)
		ShowRadioLocator((UINT8) usSoldierID, SHOW_LOCATOR_NORMAL );

	// FIRST CHECK IF WE ARE ON SCREEN
	if ( GridNoOnScreen( MercPtrs[ usSoldierID ]->sGridNo ) )
	{
		return;
	}

	// sGridNo here for DG compatibility
	gTacticalStatus.sSlideTarget = MercPtrs[ usSoldierID ]->sGridNo;
	gTacticalStatus.sSlideReason = usReasonID;

	// Plot new path!
	gfPlotNewMovement = TRUE;
}


void SlideToLocation( UINT16 usReasonID, INT32 sDestGridNo )
{	
	if (TileIsOutOfBounds(sDestGridNo))
	{
		return;
	}

	// FIRST CHECK IF WE ARE ON SCREEN
	if ( GridNoOnScreen( sDestGridNo ) )
	{
		return;
	}

	// sGridNo here for DG compatibility
	gTacticalStatus.sSlideTarget = sDestGridNo;
	gTacticalStatus.sSlideReason = usReasonID;

	// Plot new path!
	gfPlotNewMovement = TRUE;
}


void RebuildAllSoldierShadeTables( )
{
	UINT32							cnt;
	SOLDIERTYPE			 *pSoldier;

	// Loop through all mercs and make go
	for ( pSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pSoldier++, cnt++ )
	{
		if ( pSoldier->bActive )
		{
			pSoldier->CreateSoldierPalettes( );
		}
	}

}

void HandlePlayerTeamMemberDeath( SOLDIERTYPE *pSoldier )
{
	INT32					cnt;
	INT32					iNewSelectedSoldier = -1;
	SOLDIERTYPE			 *pTeamSoldier;
	BOOLEAN				 fMissionFailed = TRUE;
	INT8										bBuddyIndex;

	VerifyPublicOpplistDueToDeath( pSoldier );

	pSoldier->ReceivingSoldierCancelServices( );


	// IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
	cnt = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID;

	// look for all mercs on the same team,
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++,pTeamSoldier++)
	{
		if ( pTeamSoldier->stats.bLife >= OKLIFE && pTeamSoldier->bActive && pTeamSoldier->bInSector )
		{
			iNewSelectedSoldier = cnt;
			fMissionFailed = FALSE;
			break;
		}
	}


	if ( !fMissionFailed )
	{
		if (gTacticalStatus.fAutoBandageMode)
		{
			if ( pSoldier->ubAutoBandagingMedic != NOBODY )
			{
				CancelAIAction( MercPtrs[ pSoldier->ubAutoBandagingMedic ], TRUE );
			}
		}

		// see if this was the friend of a living merc
		cnt = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID;
		for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++,pTeamSoldier++)
		{
			if ( pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->stats.bLife >= OKLIFE )
			{
				bBuddyIndex = WhichBuddy( pTeamSoldier->ubProfile, pSoldier->ubProfile );
				switch( bBuddyIndex )
				{
				case 0:
					// buddy #1 died!
					TacticalCharacterDialogue( pTeamSoldier, QUOTE_BUDDY_ONE_KILLED );
					break;
				case 1:
					// buddy #2 died!
					TacticalCharacterDialogue( pTeamSoldier, QUOTE_BUDDY_TWO_KILLED );
					break;
				case 2:
					// learn to like buddy died!
					TacticalCharacterDialogue( pTeamSoldier, QUOTE_LEARNED_TO_LIKE_MERC_KILLED );
					break;
				default:
					break;
				}
			}
		}

		// handle stuff for Carmen if Slay is killed
		switch( pSoldier->ubProfile )
		{
		case SLAY:
			pTeamSoldier = FindSoldierByProfileID( CARMEN, FALSE );			
			if (pTeamSoldier && pTeamSoldier->aiData.bAttitude == ATTACKSLAYONLY && !TileIsOutOfBounds(ClosestPC( pTeamSoldier, NULL )) )
			{
				// Carmen now becomes friendly again
				TriggerNPCRecord( CARMEN, 29 );
			}
			break;
		case ROBOT:
			if (CheckFact( FACT_FIRST_ROBOT_DESTROYED, 0 ) == FALSE )
			{
				SetFactTrue( FACT_FIRST_ROBOT_DESTROYED );
				SetFactFalse( FACT_ROBOT_READY );
			}
			else
			{
				SetFactTrue( FACT_SECOND_ROBOT_DESTROYED );
			}
			break;
		}

	}

	//Make a call to handle the strategic things, such as Life Insurance, record it in history file etc.
	StrategicHandlePlayerTeamMercDeath( pSoldier );

	CheckForEndOfBattle( FALSE );


	if ( gusSelectedSoldier == pSoldier->ubID )
	{
		if ( !fMissionFailed )
		{
			SelectSoldier( (INT16)iNewSelectedSoldier, FALSE, FALSE );
		}
		else
		{
			gusSelectedSoldier = NOBODY;
			// Change UI mode to reflact that we are selected
			guiPendingOverrideEvent = I_ON_TERRAIN;
		}

	}
}


void HandleNPCTeamMemberDeath( SOLDIERTYPE *pSoldierOld )
{
	SOLDIERTYPE *pKiller = NULL;
	BOOLEAN bVisible;

	pSoldierOld->flags.uiStatusFlags |= SOLDIER_DEAD;
	bVisible = pSoldierOld->bVisible;

	VerifyPublicOpplistDueToDeath( pSoldierOld );

	if ( pSoldierOld->ubProfile != NO_PROFILE )
	{
		// mark as dead!
		gMercProfiles[ pSoldierOld->ubProfile ].bMercStatus = MERC_IS_DEAD;
		//
		gMercProfiles[ pSoldierOld->ubProfile ].bLife				= 0;

		if ( !(pSoldierOld->flags.uiStatusFlags & SOLDIER_VEHICLE) && !TANK( pSoldierOld) )
		{
			if ( pSoldierOld->ubAttackerID != NOBODY )
			{
				pKiller = MercPtrs[ pSoldierOld->ubAttackerID ];
			}
			if( pKiller && pKiller->bTeam == OUR_TEAM )
			{
				AddHistoryToPlayersLog( HISTORY_MERC_KILLED_CHARACTER, pSoldierOld->ubProfile, GetWorldTotalMin(), gWorldSectorX, gWorldSectorY );
			}
			else
			{
				AddHistoryToPlayersLog( HISTORY_NPC_KILLED, pSoldierOld->ubProfile, GetWorldTotalMin(), gWorldSectorX, gWorldSectorY );
			}
		}
	}

	if (pSoldierOld->bTeam == CIV_TEAM )
	{
		SOLDIERTYPE * pOther;

		// ATE: Added string to player
		if ( bVisible != -1 && pSoldierOld->ubProfile != NO_PROFILE )
		{
			ScreenMsg( FONT_RED, MSG_INTERFACE, pMercDeadString[ 0 ], pSoldierOld->name );
		}

		switch( pSoldierOld->ubProfile )
		{
		case BRENDA:
			SetFactTrue( FACT_BRENDA_DEAD );
			{
				pOther = FindSoldierByProfileID( HANS, FALSE );
				if (pOther && pOther->stats.bLife >= OKLIFE && pOther->aiData.bNeutral && (SpacesAway( pSoldierOld->sGridNo, pOther->sGridNo ) <= 12) )
				{

					TriggerNPCRecord( HANS, 10 );
				}
			}
			break;
		case PABLO:
			AddFutureDayStrategicEvent( EVENT_SECOND_AIRPORT_ATTENDANT_ARRIVED, 480 + Random( 60 ), 0, 1 );
			break;
		case ROBOT:
			if (CheckFact( FACT_FIRST_ROBOT_DESTROYED, 0 ) == FALSE )
			{
				SetFactTrue( FACT_FIRST_ROBOT_DESTROYED );
			}
			else
			{
				SetFactTrue( FACT_SECOND_ROBOT_DESTROYED );
			}
			break;
		case DRUGGIST:
		case SLAY:
		case ANNIE:
		case CHRIS:
		case TIFFANY:
		case T_REX:
			MakeRemainingTerroristsTougher();
			if ( pSoldierOld->ubProfile == DRUGGIST )
			{
				pOther = FindSoldierByProfileID( MANNY, 0 );
				if (pOther && pOther->bActive && pOther->bInSector && pOther->stats.bLife >= OKLIFE )
				{
					// try to make sure he isn't cowering etc
					pOther->aiData.sNoiseGridno = NOWHERE;
					pOther->aiData.bAlertStatus = STATUS_GREEN;
					TriggerNPCRecord( MANNY, 10 );
				}
			}
			break;
		case JIM:
		case JACK:
		case OLAF:
		case RAY:
		case OLGA:
		case TYRONE:
			MakeRemainingAssassinsTougher();
			break;

		case ELDIN:
			// the security guard...	Results in an extra loyalty penalty for Balime (in addition to civilian murder)

			/* Delayed loyalty effects elimininated.	Sep.12/98.	ARM
			// create the event value, for town BALIME
			uiLoyaltyValue = BuildLoyaltyEventValue( BALIME, LOYALTY_PENALTY_ELDIN_KILLED, FALSE );
			// post the event, 30 minute delay
			AddStrategicEvent( EVENT_TOWN_LOYALTY_UPDATE, GetWorldTotalMin() + 30, uiLoyaltyValue );
			*/
			DecrementTownLoyalty( BALIME, LOYALTY_PENALTY_ELDIN_KILLED );
			break;
		case JOEY:
			// check to see if Martha can see this
			pOther = FindSoldierByProfileID( MARTHA, FALSE );
			if ( pOther && (PythSpacesAway( pOther->sGridNo, pSoldierOld->sGridNo ) < 10 || SoldierToSoldierLineOfSightTest( pOther, pSoldierOld, TRUE ) != 0 ) )
			{
				// Martha has a heart attack and croaks
				TriggerNPCRecord( MARTHA, 17 );

				/* Delayed loyalty effects elimininated.	Sep.12/98.	ARM
				// create the event value, for town CAMBRIA
				uiLoyaltyValue = BuildLoyaltyEventValue( CAMBRIA, LOYALTY_PENALTY_MARTHA_HEART_ATTACK, FALSE );
				// post the event, 30 minute delay
				AddStrategicEvent( EVENT_TOWN_LOYALTY_UPDATE, GetWorldTotalMin() + 30, uiLoyaltyValue );
				*/
				DecrementTownLoyalty( CAMBRIA, LOYALTY_PENALTY_MARTHA_HEART_ATTACK );
			}
			else	// Martha doesn't see it.	She lives, but Joey is found a day or so later anyways
			{
				/*
				// create the event value, for town CAMBRIA
				uiLoyaltyValue = BuildLoyaltyEventValue( CAMBRIA, LOYALTY_PENALTY_JOEY_KILLED, FALSE );
				// post the event, 30 minute delay
				AddStrategicEvent( EVENT_TOWN_LOYALTY_UPDATE, GetWorldTotalMin() + ( ( 12 + Random(13)) * 60 ), uiLoyaltyValue );
				*/
				DecrementTownLoyalty( CAMBRIA, LOYALTY_PENALTY_JOEY_KILLED );
			}
			break;
		case DYNAMO:
			// check to see if dynamo quest is on
			if ( gubQuest[ QUEST_FREE_DYNAMO ] == QUESTINPROGRESS)
			{
				// SANDRO - quest failed if Dynamo is killed
				InternalEndQuest( QUEST_FREE_DYNAMO, pSoldierOld->sSectorX, pSoldierOld->sSectorY, FALSE );
			}
			break;
		// SANDRO - if we kill Carmen with Slay in our team, end the terrorists quest
		case CARMEN :
			// Carmen must have seen Slay, to finish the quest properly, he must know we betrayd him
			if ( pSoldierOld->aiData.bAttitude == ATTACKSLAYONLY )
			{
				pOther = FindSoldierByProfileID( SLAY, FALSE );			
				if (pOther && pOther->stats.bLife && pOther->bTeam == gbPlayerNum &&
					pSoldierOld->sSectorX == pOther->sSectorX && pSoldierOld->sSectorY == pOther->sSectorY)
				{
					// Slay is in sector and alive, Carmen dead, end quest, award some exp
					EndQuest( QUEST_KILL_TERRORISTS, pSoldierOld->sSectorX, pSoldierOld->sSectorY );
				}
			}
			break;
		case KINGPIN:
			// check to see if Kingpin money quest is on
			if ( gubQuest[ QUEST_KINGPIN_MONEY ] == QUESTINPROGRESS)
			{
				EndQuest( QUEST_KINGPIN_MONEY, pSoldierOld->sSectorX, pSoldierOld->sSectorY );
				HandleNPCDoAction( KINGPIN, NPC_ACTION_GRANT_EXPERIENCE_3, 0 );
			}
			SetFactTrue( FACT_KINGPIN_DEAD );
			ExecuteStrategicAIAction( STRATEGIC_AI_ACTION_KINGPIN_DEAD, 0, 0 );
			break;
		case DOREEN:
			// Doreen's dead
			if ( CheckFact( FACT_DOREEN_HAD_CHANGE_OF_HEART, 0 ) )
			{
				// tsk tsk, player killed her after getting her to reconsider, lose the bonus for sparing her
				DecrementTownLoyalty( DRASSEN, LOYALTY_BONUS_CHILDREN_FREED_DOREEN_SPARED );
			}	// then get the points for freeing the kids though killing her
			IncrementTownLoyalty( DRASSEN, LOYALTY_BONUS_CHILDREN_FREED_DOREEN_KILLED );
			// set the fact true so we have a universal check for whether the kids can go
			SetFactTrue( FACT_DOREEN_HAD_CHANGE_OF_HEART );
			EndQuest( QUEST_FREE_CHILDREN, gWorldSectorX, gWorldSectorY );
			if ( CheckFact( FACT_KIDS_ARE_FREE , 0 ) == FALSE )
			{
				HandleNPCDoAction( DOREEN, NPC_ACTION_FREE_KIDS, 0 );
			}
			break;
		// SANDRO - Check if queen bitch is dead 
		case QUEEN:
			// Muhahahahaaa, QUEST COMPLETED! Give us everything!! Exp, glory, fame!
			EndQuest( QUEST_KILL_DEIDRANNA, pSoldierOld->sSectorX, pSoldierOld->sSectorY );
			break;
		}

		// Are we looking at the queen?
		if ( pSoldierOld->ubProfile == QUEEN )
		{
			if ( pSoldierOld->ubAttackerID != NOBODY )
			{
				pKiller = MercPtrs[ pSoldierOld->ubAttackerID ];
			}

			BeginHandleDeidrannaDeath( pKiller, pSoldierOld->sGridNo, pSoldierOld->pathing.bLevel );
		}

		// crows/cows are on the civilian team, but none of the following applies to them
		if ( ( pSoldierOld->ubBodyType != CROW ) && ( pSoldierOld->ubBodyType != COW ) )
		{
			// If the civilian's killer is known
			if ( pSoldierOld->ubAttackerID != NOBODY )
			{
				// handle death of civilian..and if it was intentional
				HandleMurderOfCivilian( pSoldierOld, pSoldierOld->flags.fIntendedTarget );
			}
		}
	}
	else if ( pSoldierOld->bTeam == MILITIA_TEAM )
	{
		INT8 bMilitiaRank;

		bMilitiaRank = SoldierClassToMilitiaRank( pSoldierOld->ubSoldierClass );

		if (bMilitiaRank != -1)
		{
			BOOLEAN NeedReset = gfStrategicMilitiaChangesMade;
			// remove this militia from the strategic records
			StrategicRemoveMilitiaFromSector( gWorldSectorX, gWorldSectorY, bMilitiaRank, 1 );
			gfStrategicMilitiaChangesMade = NeedReset;
		}

		// If the militia's killer is known
		// silversurfer: did the player team kill the militia? If not, militia shouldn't become hostile towards the player.
		if ( pSoldierOld->ubAttackerID != NOBODY && MercPtrs[ pSoldierOld->ubAttackerID ]->bTeam == OUR_TEAM )
		{
			// also treat this as murder - but player will never be blamed for militia death he didn't cause
			// HEADROCK HAM 3.6: Actually this function never runs for militia (see function for details)
			//HandleMurderOfCivilian( pSoldierOld, pSoldierOld->flags.fIntendedTarget );

			// HEADROCK HAM 3.6: INI setting can cause militia to turn ONLY of they are killed intentionally
			if (pSoldierOld->flags.fIntendedTarget // Must be intentional
				&& gGameExternalOptions.ubCanMilitiaBecomeHostile > 0 // INI setting
				&& pSoldierOld->bSide == gbPlayerNum // Must not be hostile by now
				)
			{
				MilitiaChangesSides(); // Militia turn on you.
			}
		}

		HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_NATIVE_KILLED, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
	}
	else	// enemies and creatures... should any of this stuff not be called if a creature dies?
	{
		if ( pSoldierOld->ubBodyType == QUEENMONSTER )
		{
			SOLDIERTYPE *pKiller = NULL;

			if ( pSoldierOld->ubAttackerID != NOBODY )
			{
				pKiller = MercPtrs[ pSoldierOld->ubAttackerID ];

				BeginHandleQueenBitchDeath( pKiller, pSoldierOld->sGridNo, pSoldierOld->pathing.bLevel );
			}
		}

		if ( pSoldierOld->bTeam == ENEMY_TEAM )
		{
			gTacticalStatus.ubArmyGuysKilled++;
			TrackEnemiesKilled( ENEMY_KILLED_IN_TACTICAL, pSoldierOld->ubSoldierClass );
		}
		// If enemy guy was killed by the player, give morale boost to player's team!
		if (pSoldierOld->ubAttackerID != NOBODY && MercPtrs[ pSoldierOld->ubAttackerID ]->bTeam == gbPlayerNum )
		{
			HandleMoraleEvent( MercPtrs[pSoldierOld->ubAttackerID], MORALE_KILLED_ENEMY, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
		}

		HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_ENEMY_KILLED, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

		CheckForAlertWhenEnemyDies( pSoldierOld );

		if ( gTacticalStatus.ubTheChosenOne == pSoldierOld->ubID )
		{
			// reset the chosen one!
			gTacticalStatus.ubTheChosenOne = NOBODY;
		}

		if ( pSoldierOld->ubProfile == QUEEN )
		{
			HandleMoraleEvent( NULL, MORALE_DEIDRANNA_KILLED, gWorldSectorX, gWorldSectorY, gbWorldSectorZ	);
			MaximizeLoyaltyForDeidrannaKilled( );
		}
		else if ( pSoldierOld->ubBodyType == QUEENMONSTER )
		{
			HandleMoraleEvent( NULL, MORALE_MONSTER_QUEEN_KILLED, gWorldSectorX, gWorldSectorY, gbWorldSectorZ	);
			IncrementTownLoyaltyEverywhere( LOYALTY_BONUS_KILL_QUEEN_MONSTER );

			// Grant experience gain.....
			HandleNPCDoAction( 0, NPC_ACTION_GRANT_EXPERIENCE_5, 0 );

		}
	}


	// killing crows/cows is not worth any experience!
	if ( ( pSoldierOld->ubBodyType != CROW ) && ( pSoldierOld->ubBodyType != COW ) ) //&& pSoldierOld->ubLastDamageReason != TAKE_DAMAGE_BLOODLOSS ) // SANDRO - why not give exp for bleeding out?
	{
		UINT8	ubAssister = NOBODY;

		// if it was a kill by a player's merc
		if (pSoldierOld->ubAttackerID != NOBODY && MercPtrs[ pSoldierOld->ubAttackerID ]->bTeam == gbPlayerNum )
		{
			// SANDRO - for special NPCs, you gain more experiences
			UINT16 usNumExpChances = ( 10 * pSoldierOld->stats.bExpLevel ); // basic value
			switch( pSoldierOld->ubProfile )
			{
				case CARMEN:
				case QUEEN:
				case JOE:
				case ANNIE:
				case CHRIS:
				case KINGPIN:
				case TIFFANY:
				case T_REX:
				case DRUGGIST:
				case GENERAL:
				case JACK:
				case OLAF:
				case RAY:
				case OLGA:
				case TYRONE:
				case MIKE:
					usNumExpChances *= 2; // make it double for special guys
					if( gGameExternalOptions.usSpecialNPCStronger > 0 ) // if set stronger, adjust the value
					{
						usNumExpChances += (usNumExpChances * gGameExternalOptions.usSpecialNPCStronger / 100);
					}
					break;
			}
			// EXPERIENCE CLASS GAIN:	Earned a kill
			StatChange( MercPtrs[ pSoldierOld->ubAttackerID ], EXPERAMT, usNumExpChances, FALSE );
		}

		// JA2 Gold: if previous and current attackers are the same, the next-to-previous attacker gets the assist
		if (pSoldierOld->ubPreviousAttackerID == pSoldierOld->ubAttackerID)
		{
			if (pSoldierOld->ubNextToPreviousAttackerID != pSoldierOld->ubAttackerID ) // SANDRO - carefully with this, if we are still one person don't do this
				ubAssister = pSoldierOld->ubNextToPreviousAttackerID;
		}
		else
		{
			ubAssister = pSoldierOld->ubPreviousAttackerID;
		}

		// if it was assisted by a player's merc
		if (ubAssister != NOBODY && MercPtrs[ ubAssister ]->bTeam == gbPlayerNum )
		{
			// EXPERIENCE CLASS GAIN:	Earned an assist
			StatChange( MercPtrs[ ubAssister ], EXPERAMT, (UINT16)( 5 * pSoldierOld->stats.bExpLevel ), FALSE );
		}
	}

	if (pSoldierOld->ubAttackerID != NOBODY && MercPtrs[ pSoldierOld->ubAttackerID ]->bTeam == MILITIA_TEAM )
	{
		MercPtrs[ pSoldierOld->ubAttackerID ]->ubMilitiaKills++;
	}

	//if the NPC is a dealer, add the dealers items to the ground
	AddDeadArmsDealerItemsToWorld( pSoldierOld->ubProfile );

	//The queen AI layer must process the event by subtracting forces, etc.
	ProcessQueenCmdImplicationsOfDeath( pSoldierOld );

	// OK, check for existence of any more badguys!
	CheckForEndOfBattle( FALSE );
}

UINT8 LastActiveTeamMember( UINT8 ubTeam )
{
	INT32 cnt;
	SOLDIERTYPE			 *pSoldier;

	cnt = gTacticalStatus.Team[ ubTeam ].bLastID;

	// look for all mercs on the same team,
	for ( pSoldier = MercPtrs[ cnt ]; cnt >= gTacticalStatus.Team[ ubTeam ].bFirstID; cnt-- ,pSoldier--)
	{
		if ( pSoldier->bActive )
		{
			return( (INT8)cnt );
		}
	}

	return( gTacticalStatus.Team[ ubTeam ].bLastID );
}

void CheckForPotentialAddToBattleIncrement( SOLDIERTYPE *pSoldier )
{
UINT16 iCounter2;
	// Check if we are a threat!
	if ( !pSoldier->aiData.bNeutral && (pSoldier->bSide != gbPlayerNum ) )
	{
		//if ( FindObjClass( pSoldier, IC_WEAPON ) != NO_SLOT )
		// We need to exclude cases where a kid is not neutral anymore, but is defenceless!
		if ( pSoldier->bTeam == CIV_TEAM )
		{
		/*	// maybe increment num enemy attacked
			switch ( pSoldier->ubCivilianGroup )
			{
			case REBEL_CIV_GROUP:
			case KINGPIN_CIV_GROUP:
			case HICKS_CIV_GROUP:
				if ( FindObjClass( pSoldier, IC_WEAPON ) != NO_SLOT )
				{
					gTacticalStatus.bNumFoughtInBattle[ pSoldier->bTeam ]++;
				}
				break;
			default:
				// nope!
				break;
			}
			
			//New Group by Jazz
			for( iCounter2 = UNNAMED_CIV_GROUP_15; iCounter2 < NUM_CIV_GROUPS; iCounter2++ )
				{	
					if (pSoldier->ubCivilianGroup == iCounter2)
					{
							if ( FindObjClass( pSoldier, IC_WEAPON ) != NO_SLOT )
							{
							gTacticalStatus.bNumFoughtInBattle[ pSoldier->bTeam ]++;
							}
					
					}	
				}
			*/
			
			//New Group by Jazz
			for( iCounter2 = REBEL_CIV_GROUP; iCounter2 < NUM_CIV_GROUPS; iCounter2++ )
				{	
					if ( pSoldier->ubCivilianGroup == iCounter2 && zCivGroupName[iCounter2].AddToBattle == TRUE )
					{
						if ( FindObjClass( pSoldier, IC_WEAPON ) != NO_SLOT )
						{
							gTacticalStatus.bNumFoughtInBattle[ pSoldier->bTeam ]++;
						}
					}	
				}
			
		}
		else
		{
			// Increment num enemy attacked
			gTacticalStatus.bNumFoughtInBattle[ pSoldier->bTeam ]++;
		}
	}
}

// internal function for turning neutral to FALSE
void SetSoldierNonNeutral( SOLDIERTYPE * pSoldier )
{
	pSoldier->aiData.bNeutral = FALSE;

	if ( gTacticalStatus.bBoxingState == NOT_BOXING )
	{
		// Special code for strategic implications
		CalculateNonPersistantPBIInfo();
	}
}

// internal function for turning neutral to TRUE
void SetSoldierNeutral( SOLDIERTYPE * pSoldier )
{
	pSoldier->aiData.bNeutral = TRUE;

	if ( gTacticalStatus.bBoxingState == NOT_BOXING )
	{
		// Special code for strategic implications
		// search through civ team looking for non-neutral civilian!
		if ( !HostileCiviliansPresent() )
		{
			CalculateNonPersistantPBIInfo();
		}
	}
}
void MakeCivHostile( SOLDIERTYPE *pSoldier, INT8 bNewSide )
{
	if ( pSoldier->ubBodyType == COW )
	{
		return;
	}

	// override passed-in value; default is hostile to player, allied to army
	bNewSide = 1;

	switch( pSoldier->ubProfile )
	{
	case IRA:
	case DIMITRI:
	case MIGUEL:
	case CARLOS:
	case MADLAB:
	case DYNAMO:
	case SHANK:
		// rebels and rebel sympathizers become hostile to player and enemy
		bNewSide = 2;
		break;
	case MARIA:
	case ANGEL:
		if ( gubQuest[ QUEST_RESCUE_MARIA ] == QUESTINPROGRESS || gubQuest[ QUEST_RESCUE_MARIA ] == QUESTDONE )
		{
			bNewSide = 2;
		}
		break;
	default:
		switch( pSoldier->ubCivilianGroup )
		{
		case REBEL_CIV_GROUP:
			bNewSide = 2;
			break;
		default:
			break;
		}
		break;
	}

	if ( !pSoldier->aiData.bNeutral && bNewSide == pSoldier->bSide )
	{
		// already hostile!
		return;
	}

	if (pSoldier->ubProfile == CONRAD || pSoldier->ubProfile == GENERAL)
	{
		// change to enemy team
		SetSoldierNonNeutral( pSoldier );
		pSoldier->bSide = bNewSide;
		pSoldier = ChangeSoldierTeam( pSoldier, ENEMY_TEAM );
	}
	else
	{
		if ( pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP )
		{
			// if Maria is in the sector and escorted, set fact that the escape has
			// been noticed
			if ( gubQuest[ QUEST_RESCUE_MARIA ] == QUESTINPROGRESS && gTacticalStatus.bBoxingState == NOT_BOXING )
			{
				SOLDIERTYPE * pMaria = FindSoldierByProfileID( MARIA, FALSE );
				if ( pMaria && pMaria->bActive && pMaria->bInSector )
				{
					SetFactTrue( FACT_MARIA_ESCAPE_NOTICED );
				}
			}
		}
		if (pSoldier->ubProfile == BILLY)
		{
			// change orders
			pSoldier->aiData.bOrders = FARPATROL;
		}
		if (bNewSide != -1)
		{
			pSoldier->bSide = bNewSide;
		}
		if ( pSoldier->aiData.bNeutral )
		{
			// HEADROCK HAM 3.6: INI Setting decides whether non-combat civs can become hostile
			if (gGameExternalOptions.fCanTrueCiviliansBecomeHostile ||
				!IS_CIV_BODY_TYPE(pSoldier))
			{
				SetSoldierNonNeutral( pSoldier );
			}
			RecalculateOppCntsDueToNoLongerNeutral( pSoldier );
		}
	}


	// If we are already in combat...
	if ( ( gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		CheckForPotentialAddToBattleIncrement( pSoldier );
	}
}

UINT8 CivilianGroupMembersChangeSidesWithinProximity( SOLDIERTYPE * pAttacked )
{
	SOLDIERTYPE *		pSoldier;
	UINT8						ubFirstProfile = NO_PROFILE;
	UINT8						cnt;

	if ( pAttacked->ubCivilianGroup == NON_CIV_GROUP )
	{
		return( pAttacked->ubProfile );
	}

	cnt = gTacticalStatus.Team[ CIV_TEAM ].bFirstID;
	for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ CIV_TEAM ].bLastID; cnt++ ,pSoldier++ )
	{
		if ( pSoldier->bActive && pSoldier->bInSector && pSoldier->stats.bLife && pSoldier->aiData.bNeutral )
		{
			if ( pSoldier->ubCivilianGroup == pAttacked->ubCivilianGroup && pSoldier->ubBodyType != COW )
			{
				// if in LOS of this guy's attacker
				if ( (pAttacked->ubAttackerID != NOBODY && pSoldier->aiData.bOppList[pAttacked->ubAttackerID] == SEEN_CURRENTLY)
					|| ( PythSpacesAway( pSoldier->sGridNo, pAttacked->sGridNo ) < pAttacked->GetMaxDistanceVisible(pSoldier->sGridNo, pSoldier->pathing.bLevel) )
					|| ( pAttacked->ubAttackerID != NOBODY && PythSpacesAway( pSoldier->sGridNo, MercPtrs[ pAttacked->ubAttackerID ]->sGridNo ) < pAttacked->GetMaxDistanceVisible(MercPtrs[ pAttacked->ubAttackerID ]->sGridNo, MercPtrs[ pAttacked->ubAttackerID ]->pathing.bLevel) ) )
				{
					MakeCivHostile( pSoldier, 2 );
					if ( pSoldier->aiData.bOppCnt > 0 )
					{
						AddToShouldBecomeHostileOrSayQuoteList( pSoldier->ubID );
					}

					if ( pSoldier->ubProfile != NO_PROFILE && pSoldier->aiData.bOppCnt > 0 && ( ubFirstProfile == NO_PROFILE || Random( 2 ) ) )
					{
						ubFirstProfile = pSoldier->ubProfile;
					}
				}

			}
		}
	}

	return( ubFirstProfile );
}

SOLDIERTYPE * CivilianGroupMemberChangesSides( SOLDIERTYPE * pAttacked )
{
	SOLDIERTYPE *		pNew;
	SOLDIERTYPE *		pNewAttacked = pAttacked;
	SOLDIERTYPE *		pSoldier;
	UINT8						cnt;
	UINT8						ubFirstProfile = NO_PROFILE;

	if ( pAttacked->ubCivilianGroup == NON_CIV_GROUP )
	{
		// abort
		return( pNewAttacked );
	}

	// remove anyone (rebels) on our team and put them back in the civ team
	cnt = gTacticalStatus.Team[ OUR_TEAM ].bFirstID;
	for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ OUR_TEAM ].bLastID; cnt++ ,pSoldier++)
	{
		if (pSoldier->bActive && pSoldier->bInSector && pSoldier->stats.bLife)
		{
			if (pSoldier->ubCivilianGroup == pAttacked->ubCivilianGroup)
			{
				// should become hostile
				if ( pSoldier->ubProfile != NO_PROFILE && ( ubFirstProfile == NO_PROFILE || Random( 2 ) ) )
				{
					ubFirstProfile = pSoldier->ubProfile;
				}

				pNew = ChangeSoldierTeam( pSoldier, CIV_TEAM );
				if ( pSoldier == pAttacked )
				{
					pNewAttacked = pNew;
				}

			}
		}
	}

	// now change sides for anyone on the civ team within proximity
	if ( ubFirstProfile == NO_PROFILE )
	{
		// get first profile value
		ubFirstProfile = CivilianGroupMembersChangeSidesWithinProximity( pNewAttacked );
	}
	else
	{
		// just call
		CivilianGroupMembersChangeSidesWithinProximity( pNewAttacked );
	}

	/*
	if ( ubFirstProfile != NO_PROFILE )
	{
	TriggerFriendWithHostileQuote( ubFirstProfile );
	}
	*/

	if ( gTacticalStatus.fCivGroupHostile[ pNewAttacked->ubCivilianGroup ] == CIV_GROUP_NEUTRAL )
	{
		// if the civilian group turning hostile is the Rebels
		if (pAttacked->ubCivilianGroup == REBEL_CIV_GROUP)
		{
			// we haven't already reduced the loyalty back when we first set the flag to BECOME hostile
			ReduceLoyaltyForRebelsBetrayed();
		}

		AddStrategicEvent( EVENT_MAKE_CIV_GROUP_HOSTILE_ON_NEXT_SECTOR_ENTRANCE, GetWorldTotalMin() + 300, pNewAttacked->ubCivilianGroup );
		gTacticalStatus.fCivGroupHostile[ pNewAttacked->ubCivilianGroup ] = CIV_GROUP_WILL_EVENTUALLY_BECOME_HOSTILE;
	}

	// HEADROCK HAM 3.6: If this sector has affiliated bloodcats, make them all hostile.
	if ( gBloodcatPlacements[SECTOR(pNewAttacked->sSectorX,pNewAttacked->sSectorY)][0].PlacementType == BLOODCAT_PLACEMENT_STATIC &&
		gBloodcatPlacements[SECTOR(pNewAttacked->sSectorX,pNewAttacked->sSectorY)][ gGameOptions.ubDifficultyLevel-1 ].ubFactionAffiliation == pNewAttacked->ubCivilianGroup )
	{
		MakeBloodcatsHostile();
	}

	return( pNewAttacked );
}

void CivilianGroupChangesSides( UINT8 ubCivilianGroup )
{
	// change civ group side due to external event (wall blowing up)
	INT32										cnt;
	SOLDIERTYPE	*						pSoldier;

	gTacticalStatus.fCivGroupHostile[ ubCivilianGroup ] = CIV_GROUP_HOSTILE;

	// now change sides for anyone on the civ team
	cnt = gTacticalStatus.Team[ CIV_TEAM ].bFirstID;
	for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ CIV_TEAM ].bLastID; cnt++ ,pSoldier++)
	{
		if (pSoldier->bActive && pSoldier->bInSector && pSoldier->stats.bLife && pSoldier->aiData.bNeutral)
		{
			if ( pSoldier->ubCivilianGroup == ubCivilianGroup && pSoldier->ubBodyType != COW )
			{
				MakeCivHostile( pSoldier, 2 );
				if ( pSoldier->aiData.bOppCnt > 0 )
				{
					AddToShouldBecomeHostileOrSayQuoteList( pSoldier->ubID );
				}
				/*
				if ( (pSoldier->ubProfile != NO_PROFILE) && (pSoldier->aiData.bOppCnt > 0) && ( ubFirstProfile == NO_PROFILE || Random( 2 ) ) )
				{
				ubFirstProfile = pSoldier->ubProfile;
				}
				*/
			}
		}
	}

	/*
	if ( ubFirstProfile != NO_PROFILE )
	{
	TriggerFriendWithHostileQuote( ubFirstProfile );
	}
	*/
}

void HickCowAttacked( SOLDIERTYPE * pNastyGuy, SOLDIERTYPE * pTarget )
{
	INT32										cnt;
	SOLDIERTYPE *						pSoldier;

	// now change sides for anyone on the civ team
	cnt = gTacticalStatus.Team[ CIV_TEAM ].bFirstID;
	for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ CIV_TEAM ].bLastID; cnt++ ,pSoldier++)
	{
		if ( pSoldier->bActive && pSoldier->bInSector && pSoldier->stats.bLife && pSoldier->aiData.bNeutral && pSoldier->ubCivilianGroup == HICKS_CIV_GROUP )
		{
			if ( SoldierToSoldierLineOfSightTest( pSoldier, pNastyGuy, TRUE ) )
			{
				CivilianGroupMemberChangesSides( pSoldier );
				break;
			}
		}
	}
}

void MilitiaChangesSides( void )
{
	// make all the militia change sides

	INT32						cnt;
	SOLDIERTYPE *		pSoldier;

	if ( gTacticalStatus.Team[ MILITIA_TEAM ].bMenInSector == 0 )
	{
		return;
	}

	// remove anyone (rebels) on our team and put them back in the civ team
	cnt = gTacticalStatus.Team[ MILITIA_TEAM ].bFirstID;
	for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ MILITIA_TEAM ].bLastID; cnt++ ,pSoldier++)
	{
		if (pSoldier->bActive && pSoldier->bInSector && pSoldier->stats.bLife)
		{
			if ( (gWorldSectorX ==0 && gWorldSectorY == 0) || !NumEnemiesInSector( gWorldSectorX, gWorldSectorY ) )
				MakeCivHostile( pSoldier, 2 );
			RecalculateOppCntsDueToNoLongerNeutral( pSoldier );
		}
	}
}

/*
void MakePotentiallyHostileCivGroupsHostile( void )
{
UINT8 ubLoop;

// loop through all civ groups that might become hostile and set them
// to hostile
for ( ubLoop = REBEL_CIV_GROUP; ubLoop < NUM_CIV_GROUPS; ubLoop++ )
{
if (gTacticalStatus.fCivGroupHostile[ ubLoop ] == CIV_GROUP_WILL_BECOME_HOSTILE)
{
gTacticalStatus.fCivGroupHostile[ ubLoop ] = CIV_GROUP_HOSTILE;
}
}
}
*/

INT8 NumActiveAndConsciousTeamMembers( UINT8 ubTeam )
{
	INT32 cnt;
	SOLDIERTYPE			 *pSoldier;
	UINT8									ubCount = 0;

	cnt = gTacticalStatus.Team[ ubTeam ].bFirstID;

	// look for all mercs on the same team,
	for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ ubTeam ].bLastID; cnt++,pSoldier++)
	{
		if ( OK_CONTROLLABLE_MERC( pSoldier) )
		{
			ubCount++;
		}
	}

	return( ubCount );
}


UINT8 FindNextActiveAndAliveMerc( SOLDIERTYPE *pSoldier, BOOLEAN fGoodForLessOKLife, BOOLEAN fOnlyRegularMercs )
{
	UINT8	bLastTeamID;
	INT32 cnt;
	SOLDIERTYPE			 *pTeamSoldier;

	cnt = pSoldier->ubID + 1;
	bLastTeamID = gTacticalStatus.Team[ pSoldier->bTeam ].bLastID;

	// look for all mercs on the same team,
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= bLastTeamID; cnt++,pTeamSoldier++)
	{
		if ( fOnlyRegularMercs )
		{
			if ( pTeamSoldier->bActive && ( AM_AN_EPC( pTeamSoldier ) || AM_A_ROBOT( pTeamSoldier ) ) )
			{
				continue;
			}
		}

		if ( fGoodForLessOKLife )
		{
			if ( pTeamSoldier->stats.bLife > 0 && pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->bTeam == gbPlayerNum && pTeamSoldier->bAssignment < ON_DUTY	&& OK_INTERRUPT_MERC( pTeamSoldier ) && pSoldier->bAssignment == pTeamSoldier->bAssignment )
			{
				return( (UINT8)cnt );
			}
		}
		else
		{
			if ( OK_CONTROLLABLE_MERC( pTeamSoldier) && OK_INTERRUPT_MERC( pTeamSoldier ) && pSoldier->bAssignment == pTeamSoldier->bAssignment )
			{
				return( (UINT8)cnt );
			}
		}
	}

	// none found,
	// Now loop back
	cnt = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID;
	bLastTeamID = pSoldier->ubID;

	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= bLastTeamID; cnt++,pTeamSoldier++)
	{
		if ( fOnlyRegularMercs )
		{
			if ( pTeamSoldier->bActive && ( AM_AN_EPC( pTeamSoldier ) || AM_A_ROBOT( pTeamSoldier ) ) )
			{
				continue;
			}
		}

		if ( fGoodForLessOKLife )
		{
			if ( pTeamSoldier->stats.bLife > 0 && pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->bTeam == gbPlayerNum && pTeamSoldier->bAssignment < ON_DUTY	&& OK_INTERRUPT_MERC( pTeamSoldier ) && pSoldier->bAssignment == pTeamSoldier->bAssignment )
			{
				return( (UINT8)cnt );
			}
		}
		else
		{
			if ( OK_CONTROLLABLE_MERC( pTeamSoldier) && OK_INTERRUPT_MERC( pTeamSoldier ) && pSoldier->bAssignment == pTeamSoldier->bAssignment )
			{
				return( (UINT8)cnt );
			}
		}
	}

	// IF we are here, keep as we always were!
	return( pSoldier->ubID );

}


SOLDIERTYPE *FindNextActiveSquad( SOLDIERTYPE *pSoldier )
{
	INT32 cnt, cnt2;


	for( cnt = pSoldier->bAssignment + 1 ; cnt <	NUMBER_OF_SQUADS; cnt++ )
	{
		for( cnt2 =0; cnt2 < NUMBER_OF_SOLDIERS_PER_SQUAD; cnt2++ )
		{
			if ( Squad[ cnt ][ cnt2 ] != NULL && Squad[ cnt ][ cnt2 ]->bInSector && OK_INTERRUPT_MERC( Squad[ cnt ][ cnt2 ] ) && OK_CONTROLLABLE_MERC( Squad[ cnt ][ cnt2 ] ) && !(Squad[ cnt ][ cnt2 ]->flags.uiStatusFlags & SOLDIER_VEHICLE) )
			{
				return( Squad[ cnt ][ cnt2 ] );
			}
		}
	}

	// none found,
	// Now loop back
	for( cnt = 0; cnt <= pSoldier->bAssignment; cnt++ )
	{
		for( cnt2 =0; cnt2 < NUMBER_OF_SOLDIERS_PER_SQUAD; cnt2++ )
		{
			if ( Squad[ cnt ][ cnt2 ] != NULL	&& Squad[ cnt ][ cnt2 ]->bInSector && OK_INTERRUPT_MERC( Squad[ cnt ][ cnt2 ] ) && OK_CONTROLLABLE_MERC( Squad[ cnt ][ cnt2 ] ) && !(Squad[ cnt ][ cnt2 ]->flags.uiStatusFlags & SOLDIER_VEHICLE) )
			{
				return( Squad[ cnt ][ cnt2 ] );
			}
		}
	}

	// IF we are here, keep as we always were!
	return( pSoldier );

}


UINT8 FindPrevActiveAndAliveMerc( SOLDIERTYPE *pSoldier, BOOLEAN fGoodForLessOKLife,	BOOLEAN fOnlyRegularMercs )
{
	UINT8	bLastTeamID;
	INT32 cnt;
	SOLDIERTYPE			 *pTeamSoldier;


	// loop back
	bLastTeamID = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID;
	cnt = pSoldier->ubID - 1;

	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt >= bLastTeamID; cnt--,pTeamSoldier-- )
	{
		if ( fOnlyRegularMercs )
		{
			if ( AM_AN_EPC( pTeamSoldier ) || AM_A_ROBOT( pTeamSoldier ) )
			{
				continue;
			}
		}

		if ( fGoodForLessOKLife )
		{
			// Check for bLife > 0
			if ( pTeamSoldier->stats.bLife > 0 && pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->bTeam == gbPlayerNum && pTeamSoldier->bAssignment < ON_DUTY	&& OK_INTERRUPT_MERC( pTeamSoldier ) && pSoldier->bAssignment == pTeamSoldier->bAssignment )
			{
				return( (UINT8)cnt );
			}
		}
		else
		{
			if ( OK_CONTROLLABLE_MERC( pTeamSoldier ) && OK_INTERRUPT_MERC( pTeamSoldier ) && pSoldier->bAssignment == pTeamSoldier->bAssignment )
			{
				return( (UINT8)cnt );
			}
		}
	}

	bLastTeamID = pSoldier->ubID;
	cnt = gTacticalStatus.Team[ pSoldier->bTeam ].bLastID;

	// look for all mercs on the same team,
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt >= bLastTeamID; cnt--,pTeamSoldier-- )
	{
		if ( fOnlyRegularMercs )
		{
			if ( AM_AN_EPC( pTeamSoldier ) || AM_A_ROBOT( pTeamSoldier ) )
			{
				continue;
			}
		}

		if ( fGoodForLessOKLife )
		{
			if ( pTeamSoldier->stats.bLife > 0 && pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->bTeam == gbPlayerNum && pTeamSoldier->bAssignment < ON_DUTY	&& OK_INTERRUPT_MERC( pTeamSoldier ) && pSoldier->bAssignment == pTeamSoldier->bAssignment )
			{
				return( (UINT8)cnt );
			}
		}
		else
		{
			if ( OK_CONTROLLABLE_MERC( pTeamSoldier) && OK_INTERRUPT_MERC( pTeamSoldier ) && pSoldier->bAssignment == pTeamSoldier->bAssignment )
			{
				return( (UINT8)cnt );
			}
		}
	}

	// none found,
	// IF we are here, keep as we always were!
	return( pSoldier->ubID );

}

BOOLEAN CheckForPlayerTeamInMissionExit( )
{
	INT32 cnt;
	SOLDIERTYPE			 *pSoldier;
	UINT8									bGuysIn = 0;

	// End the turn of player charactors
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	// look for all mercs on the same team,
	for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pSoldier++)
	{
		if ( pSoldier->bActive && pSoldier->stats.bLife >= OKLIFE )
		{
			if ( pSoldier->flags.fInMissionExitNode )
			{
				bGuysIn++;
			}
		}
	}

	if ( bGuysIn == 0 )
	{
		return( FALSE );
	}

	if ( NumActiveAndConsciousTeamMembers( gbPlayerNum ) == 0 )
	{
		return( FALSE );
	}

	if ( bGuysIn == NumActiveAndConsciousTeamMembers( gbPlayerNum ) )
	{
		return( TRUE );
	}

	return( FALSE );
}

void EndTacticalDemo( )
{
	gTacticalStatus.uiFlags &= (~DEMOMODE );
	gTacticalStatus.fGoingToEnterDemo = FALSE;
}

UINT32 EnterTacticalDemoMode()
{
	UINT8 ubNewScene = gubCurrentScene;
	UINT8	ubNumScenes = NUM_RANDOM_SCENES;

	gTacticalStatus.uiTimeOfLastInput = GetJA2Clock();

	// REMOVE ALL EVENTS!
	DequeAllGameEvents( FALSE );

	// Switch into realtime/demo
	gTacticalStatus.uiFlags |= ( REALTIME | DEMOMODE );
	gTacticalStatus.uiFlags &= (~TURNBASED);
	gTacticalStatus.uiFlags &= (~NPC_TEAM_DEAD);
	gTacticalStatus.uiFlags &= (~PLAYER_TEAM_DEAD);

	// Force load of guys
	SetLoadOverrideParams( TRUE, FALSE, NULL );

	// Load random level
	// Dont't do first three levels
	if ( gTacticalStatus.fNOTDOLASTDEMO )
	{
		ubNumScenes--;
	}

	do
	{
		ubNewScene = START_DEMO_SCENE + (UINT8)Random( ubNumScenes );

	} while( ubNewScene == gubCurrentScene );

	gubCurrentScene = ubNewScene;

	// Set demo timer
	gTacticalStatus.uiTimeSinceDemoOn = GetJA2Clock();

	gfSGPInputReceived = FALSE;

	gTacticalStatus.fGoingToEnterDemo = FALSE;

	return( INIT_SCREEN );

}

CHAR8 *GetSceneFilename(	)
{
	return( gzLevelFilenames[ gubCurrentScene ] );
}

extern BOOLEAN InternalOkayToAddStructureToWorld( INT32 sBaseGridNo, INT8 bLevel, DB_STRUCTURE_REF * pDBStructureRef, INT16 sExclusionID, BOOLEAN fIgnorePeople );

// NB if making changes don't forget to update NewOKDestinationAndDirection
INT16 NewOKDestination( SOLDIERTYPE * pCurrSoldier, INT32 sGridNo, BOOLEAN fPeopleToo, INT8 bLevel )
{
	UINT8					bPerson;
	STRUCTURE *		pStructure;
	INT16		 sDesiredLevel;
	BOOLEAN				fOKCheckStruct;

	if ( !GridNoOnVisibleWorldTile( sGridNo ) )
	{
		return( TRUE );
	}

	if (fPeopleToo && ( bPerson = WhoIsThere2( sGridNo, bLevel ) ) != NOBODY )
	{
		// we could be multitiled... if the person there is us, and the gridno is not
		// our base gridno, skip past these checks
		if ( ! (bPerson == pCurrSoldier->ubID && sGridNo != pCurrSoldier->sGridNo) )
		{
			if ( pCurrSoldier->bTeam == gbPlayerNum )
			{
				if ( ( Menptr[ bPerson ].bVisible >= 0) || ( gTacticalStatus.uiFlags & SHOW_ALL_MERCS ) )
					return( FALSE );				 // if someone there it's NOT OK
			}
			else
			{
				return( FALSE );				 // if someone there it's NOT OK
			}
		}
	}

	// Check structure database
	if ( (pCurrSoldier->flags.uiStatusFlags & SOLDIER_MULTITILE) && !(gfEstimatePath) )
	{
		UINT16									usAnimSurface;
		STRUCTURE_FILE_REF *		pStructureFileRef;
		BOOLEAN									fOk;
		INT8										bLoop;
		UINT16									usStructureID = INVALID_STRUCTURE_ID;

		// this could be kinda slow...

		// Get animation surface...
		usAnimSurface = DetermineSoldierAnimationSurface( pCurrSoldier, pCurrSoldier->usUIMovementMode );
		// Get structure ref...
		pStructureFileRef = GetAnimationStructureRef( pCurrSoldier->ubID, usAnimSurface, pCurrSoldier->usUIMovementMode );

		// opposite directions should be mirrors, so only check 4
		if ( pStructureFileRef )
		{
			// if ANY direction is valid, consider moving here valid
			for (bLoop = 0; bLoop < NUM_WORLD_DIRECTIONS; bLoop++)
			{
				// ATE: Only if we have a levelnode...
				if ( pCurrSoldier->pLevelNode != NULL && pCurrSoldier->pLevelNode->pStructureData != NULL )
				{
					usStructureID = pCurrSoldier->pLevelNode->pStructureData->usStructureID;
				}
				else
				{
					usStructureID = INVALID_STRUCTURE_ID;
				}

				fOk = InternalOkayToAddStructureToWorld( sGridNo, bLevel, &(pStructureFileRef->pDBStructureRef[bLoop]), usStructureID, (BOOLEAN)!fPeopleToo );
				if (fOk)
				{
					return( TRUE );
				}
			}
		}
		return( FALSE );
	}
	else
	{
		// quick test
		if ( gpWorldLevelData[ sGridNo ].pStructureHead != NULL )
		{
			// Something is here, check obstruction in future
			if ( bLevel == 0)
			{
				sDesiredLevel = STRUCTURE_ON_GROUND;
			}
			else
			{
				sDesiredLevel = STRUCTURE_ON_ROOF;
			}

			pStructure = FindStructure( sGridNo, STRUCTURE_BLOCKSMOVES );

			// ATE: If we are trying to get a path to an exit grid AND
			// we are a cave....still allow this..
			//if ( pStructure && gfPlotPathToExitGrid && pStructure->fFlags & STRUCTURE_CAVEWALL )
			if ( pStructure && gfPlotPathToExitGrid )
			{
				pStructure = NULL;
			}

			while (pStructure != NULL)
			{
				if (!(pStructure->fFlags & STRUCTURE_PASSABLE) )
				{
					fOKCheckStruct = TRUE;

					// Check if this is a multi-tile
					if ( ( pStructure->fFlags & STRUCTURE_MOBILE ) && ( pCurrSoldier->flags.uiStatusFlags & SOLDIER_MULTITILE ) )
					{
						// Check IDs with soldier's ID
						if ( pCurrSoldier->pLevelNode != NULL && pCurrSoldier->pLevelNode->pStructureData != NULL && pCurrSoldier->pLevelNode->pStructureData->usStructureID == pStructure->usStructureID )
						{
							fOKCheckStruct = FALSE;
						}
					}

					if ( fOKCheckStruct )
					{
						if (pStructure->sCubeOffset == sDesiredLevel)
						{
							return( FALSE );
						}
					}
				}

				pStructure = FindNextStructure( pStructure, STRUCTURE_BLOCKSMOVES );
			}
		}
	}
	return( TRUE );
}

// NB if making changes don't forget to update NewOKDestination
INT16 NewOKDestinationAndDirection( SOLDIERTYPE * pCurrSoldier, INT32 sGridNo, INT8 bDirection, BOOLEAN fPeopleToo, INT8 bLevel )
{
	UINT8					bPerson;
	STRUCTURE *		pStructure;
	INT16		 sDesiredLevel;
	BOOLEAN				fOKCheckStruct;

	if (fPeopleToo && ( bPerson = WhoIsThere2( sGridNo, bLevel ) ) != NOBODY )
	{
		// we could be multitiled... if the person there is us, and the gridno is not
		// our base gridno, skip past these checks
		if ( ! (bPerson == pCurrSoldier->ubID && sGridNo != pCurrSoldier->sGridNo) )
		{
			if ( pCurrSoldier->bTeam == gbPlayerNum )
			{
				if ( ( Menptr[ bPerson ].bVisible >= 0) || ( gTacticalStatus.uiFlags & SHOW_ALL_MERCS ) )
					return( FALSE );				 // if someone there it's NOT OK
			}
			else
			{
				return( FALSE );				 // if someone there it's NOT OK
			}
		}
	}

	// Check structure database
	if ( ( pCurrSoldier->flags.uiStatusFlags & SOLDIER_MULTITILE ) && !(gfEstimatePath) )
	{
		UINT16									usAnimSurface;
		STRUCTURE_FILE_REF *		pStructureFileRef;
		BOOLEAN									fOk;
		INT8										bLoop;
		UINT16									usStructureID = INVALID_STRUCTURE_ID;

		// this could be kinda slow...

		// Get animation surface...
		usAnimSurface = DetermineSoldierAnimationSurface( pCurrSoldier, pCurrSoldier->usUIMovementMode );
		// Get structure ref...
		pStructureFileRef = GetAnimationStructureRef( pCurrSoldier->ubID, usAnimSurface, pCurrSoldier->usUIMovementMode );

		if ( pStructureFileRef )
		{

			// use the specified direction for checks
			bLoop = bDirection;

			{
				// ATE: Only if we have a levelnode...
				if ( pCurrSoldier->pLevelNode != NULL && pCurrSoldier->pLevelNode->pStructureData != NULL )
				{
					usStructureID = pCurrSoldier->pLevelNode->pStructureData->usStructureID;
				}

				fOk = InternalOkayToAddStructureToWorld( sGridNo, pCurrSoldier->pathing.bLevel, &(pStructureFileRef->pDBStructureRef[ gOneCDirection[ bLoop ] ]), usStructureID, (BOOLEAN)!fPeopleToo );
				if (fOk)
				{
					return( TRUE );
				}
			}

		}
		return( FALSE );
	}
	else
	{
		// quick test
		if ( gpWorldLevelData[ sGridNo ].pStructureHead != NULL )
		{
			// Something is here, check obstruction in future
			if ( bLevel == 0)
			{
				sDesiredLevel = STRUCTURE_ON_GROUND;
			}
			else
			{
				sDesiredLevel = STRUCTURE_ON_ROOF;
			}

			pStructure = FindStructure( sGridNo, STRUCTURE_BLOCKSMOVES );

			// ATE: If we are trying to get a path to an exit grid AND
			// we are a cave....still allow this..
			//if ( pStructure && gfPlotPathToExitGrid && pStructure->fFlags & STRUCTURE_CAVEWALL )
			if ( pStructure && gfPlotPathToExitGrid )
			{
				pStructure = NULL;
			}

			while (pStructure != NULL)
			{
				if (!(pStructure->fFlags & STRUCTURE_PASSABLE) )
				{
					fOKCheckStruct = TRUE;

					// Check if this is a multi-tile
					if ( ( pStructure->fFlags & STRUCTURE_MOBILE ) && ( pCurrSoldier->flags.uiStatusFlags & SOLDIER_MULTITILE ) )
					{
						// Check IDs with soldier's ID
						if ( pCurrSoldier->pLevelNode != NULL && pCurrSoldier->pLevelNode->pStructureData != NULL && pCurrSoldier->pLevelNode->pStructureData->usStructureID == pStructure->usStructureID )
						{
							fOKCheckStruct = FALSE;
						}
					}

					if ( fOKCheckStruct )
					{
						if (pStructure->sCubeOffset == sDesiredLevel)
						{
							return( FALSE );
						}
					}
				}

				pStructure = FindNextStructure( pStructure, STRUCTURE_BLOCKSMOVES );
			}
		}
	}
	return( TRUE );
}



//Kris:
BOOLEAN FlatRoofAboveGridNo( INT32 iMapIndex )
{
	LEVELNODE *pRoof;
	UINT32 uiTileType;
	pRoof = gpWorldLevelData[ iMapIndex ].pRoofHead;
	while( pRoof )
	{
		if( pRoof->usIndex != NO_TILE )
		{
			GetTileType( pRoof->usIndex, &uiTileType );
			if( uiTileType >= FIRSTROOF && uiTileType <= LASTROOF )
				return TRUE;
		}
		pRoof = pRoof->pNext;
	}
	return FALSE;
}


//Kris:
//ASSUMPTION:	This function assumes that we are checking on behalf of a single tiled merc.	This function
//						should not be used for checking on behalf of multi-tiled "things".
//I wrote this function for editor use.	I don't personally care about multi-tiled stuff.	All I want
//to know is whether or not I can put a merc here.	In most cases, I won't be dealing with multi-tiled
//mercs, and the rarity doesn't justify the needs.	I just wrote this to be quick and dirty, and I don't
//expect it to perform perfectly in all situations.
BOOLEAN IsLocationSittable( INT32 iMapIndex, BOOLEAN fOnRoof )
{
	STRUCTURE *pStructure;
	INT16 sDesiredLevel;
	if( WhoIsThere2( iMapIndex, 0 ) != NOBODY )
		return FALSE;
	//Locations on roofs without a roof is not possible, so
	//we convert the onroof intention to ground.
	if( fOnRoof && !FlatRoofAboveGridNo( iMapIndex ) )
		fOnRoof = FALSE;
	// Check structure database
	if( gpWorldLevelData[ iMapIndex ].pStructureHead )
	{
		// Something is here, check obstruction in future
		sDesiredLevel = fOnRoof ? STRUCTURE_ON_ROOF : STRUCTURE_ON_GROUND;
		pStructure = FindStructure( iMapIndex, STRUCTURE_BLOCKSMOVES );
		while( pStructure )
		{
			if( !(pStructure->fFlags & STRUCTURE_PASSABLE) && pStructure->sCubeOffset == sDesiredLevel )
				return FALSE;
			pStructure = FindNextStructure( pStructure, STRUCTURE_BLOCKSMOVES );
		}
	}
	return TRUE;
}


BOOLEAN IsLocationSittableExcludingPeople( INT32 iMapIndex, BOOLEAN fOnRoof )
{
	STRUCTURE *pStructure;
	INT16 sDesiredLevel;

	//Locations on roofs without a roof is not possible, so
	//we convert the onroof intention to ground.
	if( fOnRoof && !FlatRoofAboveGridNo( iMapIndex ) )
		fOnRoof = FALSE;
	// Check structure database
	if( gpWorldLevelData[ iMapIndex ].pStructureHead )
	{
		// Something is here, check obstruction in future
		sDesiredLevel = fOnRoof ? STRUCTURE_ON_ROOF : STRUCTURE_ON_GROUND;
		pStructure = FindStructure( iMapIndex, STRUCTURE_BLOCKSMOVES );
		while( pStructure )
		{
			if( !(pStructure->fFlags & STRUCTURE_PASSABLE) && pStructure->sCubeOffset == sDesiredLevel )
				return FALSE;
			pStructure = FindNextStructure( pStructure, STRUCTURE_BLOCKSMOVES );
		}
	}
	return TRUE;
}


BOOLEAN TeamMemberNear(INT8 bTeam, INT32 sGridNo, INT32 iRange)
{
	UINT8 bLoop;
	SOLDIERTYPE * pSoldier;

	for (bLoop=gTacticalStatus.Team[bTeam].bFirstID, pSoldier=MercPtrs[bLoop]; bLoop <= gTacticalStatus.Team[bTeam].bLastID; bLoop++, pSoldier++)
	{
		if (pSoldier->bActive && pSoldier->bInSector && (pSoldier->stats.bLife >= OKLIFE) && !( pSoldier->flags.uiStatusFlags & SOLDIER_GASSED ) )
		{
			if (PythSpacesAway(pSoldier->sGridNo,sGridNo) <= iRange)
			{
				return(TRUE);
			}
		}
	}

	return(FALSE);
}

INT32 FindAdjacentGridEx( SOLDIERTYPE *pSoldier, INT32 sGridNo, UINT8 *pubDirection, INT32 *psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor )
{
	// psAdjustedGridNo gets the original gridno or the new one if updated
	// It will ONLY be updated IF we were over a merc, ( it's updated to their gridno )
	// pubDirection gets the direction to the final gridno
	// fForceToPerson: forces the grid under consideration to be the one occupiedby any target
	// in that location, because we could be passed a gridno based on the overlap of soldier's graphic
	// fDoor determines whether special door-handling code should be used (for interacting with doors)

	INT32 sFourGrids[4], sDistance=0;
	static const UINT8 sDirs[4] = { NORTH, EAST, SOUTH, WEST };
	//INT32 cnt;
	//INT32 sClosest=NOWHERE, sSpot, sOkTest;
	INT32 sClosest = MAX_MAP_POS, sSpot, sOkTest; //Lalien: changed to ensure compability with new definition of NOWHERE
	//INT32 sCloseGridNo=NOWHERE;
	INT32 sCloseGridNo = MAX_MAP_POS; //Lalien: changed to ensure compability with new definition of NOWHERE
	UINT32										 uiMercFlags;
	UINT16										 usSoldierIndex;
	UINT8											ubDir;
	STRUCTURE								*pDoor;
	//STRUCTURE							*pWall;
	UINT8											ubWallOrientation;
	BOOLEAN																	fCheckGivenGridNo = TRUE;
	UINT8																		ubTestDirection;
	EXITGRID																ExitGrid;

	// Set default direction
	if (pubDirection)
	{
		*pubDirection = pSoldier->ubDirection;
	}

	// CHECK IF WE WANT TO FORCE GRIDNO TO PERSON
	if ( psAdjustedGridNo != NULL )
	{
		*psAdjustedGridNo = sGridNo;
	}

	// CHECK IF IT'S THE SAME ONE AS WE'RE ON, IF SO, RETURN THAT!
	if ( pSoldier->sGridNo == sGridNo && !FindStructure( sGridNo, ( STRUCTURE_SWITCH ) ) )
	{
		// OK, if we are looking for a door, it may be in the same tile as us, so find the direction we
		// have to face to get to the door, not just our initial direction...
		// If we are in the same tile as a switch, we can NEVER pull it....
		if( fDoor )
		{
			// This can only happen if a door was to the south to east of us!

			// Do south!
			//sSpot = NewGridNo( sGridNo, DirectionInc( SOUTH ) );

			// ATE: Added: Switch behave EXACTLY like doors
			pDoor = FindStructure( sGridNo, ( STRUCTURE_ANYDOOR ) );

			if ( pDoor != NULL )
			{
				// Get orinetation
				ubWallOrientation = pDoor->ubWallOrientation;

				if ( ubWallOrientation == OUTSIDE_TOP_LEFT || ubWallOrientation == INSIDE_TOP_LEFT )
				{
					// To the south!
					sSpot = NewGridNo( sGridNo, DirectionInc( SOUTH ) );
					if (pubDirection)
					{
						(*pubDirection) = (UINT8)GetDirectionFromGridNo( sSpot, pSoldier );
					}
				}

				if ( ubWallOrientation == OUTSIDE_TOP_RIGHT || ubWallOrientation == INSIDE_TOP_RIGHT )
				{
					// TO the east!
					sSpot = NewGridNo( sGridNo, DirectionInc( EAST ) );
					if (pubDirection)
					{
						(*pubDirection) = (UINT8)GetDirectionFromGridNo( sSpot, pSoldier );
					}
				}
			}
		}

		// Use soldier's direction
		return( sGridNo );
	}

	// Look for a door!
	if (fDoor)
	{
		pDoor = FindStructure( sGridNo, ( STRUCTURE_ANYDOOR | STRUCTURE_SWITCH ) );
	}
	else
	{
		pDoor = NULL;
	}

	if ( fForceToPerson )
	{
		if ( FindSoldier( sGridNo, &usSoldierIndex, &uiMercFlags, FIND_SOLDIER_GRIDNO )	)
		{
			sGridNo = MercPtrs[ usSoldierIndex ]->sGridNo;
			if ( psAdjustedGridNo != NULL )
			{
				*psAdjustedGridNo = sGridNo;

				// Use direction to this guy!
				if (pubDirection)
				{
					(*pubDirection) = (UINT8)GetDirectionFromGridNo( sGridNo, pSoldier );
				}
			}
		}
	}


	if ( ( sOkTest = NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->pathing.bLevel ) ) > 0)	// no problem going there! nobody on it!
	{
		// OK, if we are looking to goto a switch, ignore this....
		if ( pDoor )
		{
			if ( pDoor->fFlags & STRUCTURE_SWITCH )
			{
				// Don't continuel
				fCheckGivenGridNo = FALSE;
			}
		}

		// If there is an exit grid....
		if ( GetExitGrid( sGridNo, &ExitGrid ) )
		{
			// Don't continuel
			fCheckGivenGridNo = FALSE;
		}


		if ( fCheckGivenGridNo )
		{
			sDistance = PlotPath( pSoldier, sGridNo,	NO_COPYROUTE, NO_PLOT, TEMPORARY, (INT16)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints );

			if ( sDistance > 0 )
		{

			if ( sDistance < sClosest )
			{
				sClosest						= sDistance;
				sCloseGridNo	= sGridNo;
			}
		}
		}
	}


	for (INT8 cnt = 0; cnt < 4; cnt++)
	{
		// MOVE OUT TWO DIRECTIONS
		sFourGrids[cnt] = sSpot = NewGridNo( sGridNo, DirectionInc( sDirs[ cnt ] ) );

		ubTestDirection = sDirs[ cnt ];

		// For switches, ALLOW them to walk through walls to reach it....
		if ( pDoor && pDoor->fFlags & STRUCTURE_SWITCH )
		{
			ubTestDirection = gOppositeDirection[ ubTestDirection ];
		}

		if( fDoor )
		{
			if ( gubWorldMovementCosts[ sSpot ][ ubTestDirection ][ pSoldier->pathing.bLevel ] >= TRAVELCOST_BLOCKED )
			{
				// obstacle or wall there!
				continue;
			}
		}
		else
		{
			// this function returns original MP cost if not a door cost
			if ( DoorTravelCost( pSoldier, sSpot, gubWorldMovementCosts[ sSpot ][ ubTestDirection ][ pSoldier->pathing.bLevel ], FALSE, NULL ) >= TRAVELCOST_BLOCKED )
		{
			// obstacle or wall there!
			continue;
		}
		}

		// Eliminate some directions if we are looking at doors!
		if ( pDoor != NULL )
		{
			// Get orinetation
			ubWallOrientation = pDoor->ubWallOrientation;

			// Refuse the south and north and west	directions if our orientation is top-right
			if ( ubWallOrientation == OUTSIDE_TOP_RIGHT || ubWallOrientation == INSIDE_TOP_RIGHT )
			{
				if ( sDirs[ cnt ] == NORTH || sDirs[ cnt ] == WEST || sDirs[ cnt ] == SOUTH )
					continue;
			}

			// Refuse the north and west and east directions if our orientation is top-right
			if ( ubWallOrientation == OUTSIDE_TOP_LEFT || ubWallOrientation == INSIDE_TOP_LEFT )
			{
				if ( sDirs[ cnt ] == NORTH || sDirs[ cnt ] == WEST || sDirs[ cnt ] == EAST )
					continue;
			}
		}

		// If this spot is our soldier's gridno use that!
		if ( sSpot == pSoldier->sGridNo )
		{
			// Use default diurection ) soldier's direction )

			// OK, at least get direction to face......
			// Defaults to soldier's facing dir unless we change it!
			//if ( pDoor != NULL )
		{
			// Use direction to the door!
			if (pubDirection)
				{
					(*pubDirection) = (UINT8)GetDirectionFromGridNo( sGridNo, pSoldier );
				}
		}
		return( sSpot );
		}

		// don't store path, just measure it
		ubDir = (UINT8)GetDirectionToGridNoFromGridNo( sSpot, sGridNo );

		if ( ( NewOKDestinationAndDirection( pSoldier, sSpot, ubDir, TRUE, pSoldier->pathing.bLevel ) > 0 ) &&
			( ( sDistance = PlotPath( pSoldier, sSpot,	NO_COPYROUTE, NO_PLOT, TEMPORARY, (INT16)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints ) ) > 0 ) )
		{
			if ( sDistance < sClosest )
			{
				sClosest						= sDistance;
				sCloseGridNo  = sSpot;
			}
		}
	}
	
	if ( !TileIsOutOfBounds( sClosest ) )
	{
		// Take last direction and use opposite!
		// This will be usefull for ours and AI mercs

		// If our gridno is the same ( which can be if we are look at doors )
		if ( sGridNo == sCloseGridNo )
		{
			if (pubDirection)
			{
				// ATE: Only if we have a valid door!
				if ( pDoor )
				{
					switch( pDoor->pDBStructureRef->pDBStructure->ubWallOrientation )
					{
					case OUTSIDE_TOP_LEFT:
					case INSIDE_TOP_LEFT:

						*pubDirection = SOUTH;
						break;

					case OUTSIDE_TOP_RIGHT:
					case INSIDE_TOP_RIGHT:

						*pubDirection = EAST;
						break;
					}
				}
			}
		}
		else
		{
			// Calculate direction if our gridno is different....
			ubDir = (UINT8)GetDirectionToGridNoFromGridNo( sCloseGridNo, sGridNo );
			if (pubDirection)
			{
				*pubDirection = ubDir;
			}
		}
		//if ( psAdjustedGridNo != NULL )
		//{
		//		(*psAdjustedGridNo) = sCloseGridNo;
		//}		
		if ( TileIsOutOfBounds( sCloseGridNo ) )
		{
			return( -1 );
		}
		return( sCloseGridNo );
	}
	else
		return( -1 );
}


INT32 FindNextToAdjacentGridEx( SOLDIERTYPE *pSoldier, INT32 sGridNo, UINT8 *pubDirection, INT32 *psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor )
{
	// This function works in a similar way as FindAdjacentGridEx, but looks for a location 2 tiles away

	// psAdjustedGridNo gets the original gridno or the new one if updated
	// pubDirection gets the direction to the final gridno
	// fForceToPerson: forces the grid under consideration to be the one occupiedby any target
	// in that location, because we could be passed a gridno based on the overlap of soldier's graphic
	// fDoor determines whether special door-handling code should be used (for interacting with doors)

 INT32 sFourGrids[4], sDistance=0;
	static const UINT8 sDirs[4] = { NORTH, EAST, SOUTH, WEST };
	//INT32 cnt;
 INT32 sClosest=WORLD_MAX, sSpot, sSpot2, sOkTest;
 INT32 sCloseGridNo=NOWHERE;
	UINT32										 uiMercFlags;
	UINT16										 usSoldierIndex;
	UINT8											ubDir;
	STRUCTURE								*pDoor;
	UINT8											ubWallOrientation;
	BOOLEAN																	fCheckGivenGridNo = TRUE;
	UINT8																		ubTestDirection;
	UINT8																		ubWhoIsThere;

	// CHECK IF WE WANT TO FORCE GRIDNO TO PERSON
	if ( psAdjustedGridNo != NULL )
	{
		*psAdjustedGridNo = sGridNo;
	}

	// CHECK IF IT'S THE SAME ONE AS WE'RE ON, IF SO, RETURN THAT!
	if ( pSoldier->sGridNo == sGridNo )
	{
		*pubDirection = pSoldier->ubDirection;
		return( sGridNo );
	}

	// Look for a door!
	if (fDoor)
	{
		pDoor = FindStructure( sGridNo, ( STRUCTURE_ANYDOOR | STRUCTURE_SWITCH ) );
	}
	else
	{
		pDoor = NULL;
	}

	if ( fForceToPerson )
	{
		if ( FindSoldier( sGridNo, &usSoldierIndex, &uiMercFlags, FIND_SOLDIER_GRIDNO )	)
		{
			sGridNo = MercPtrs[ usSoldierIndex ]->sGridNo;
			if ( psAdjustedGridNo != NULL )
			{
				*psAdjustedGridNo = sGridNo;
			}
		}
	}


	if ( ( sOkTest = NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->pathing.bLevel ) ) > 0)	// no problem going there! nobody on it!
	{
		// OK, if we are looking to goto a switch, ignore this....
		if ( pDoor )
		{
			if ( pDoor->fFlags & STRUCTURE_SWITCH )
			{
				// Don't continuel
				fCheckGivenGridNo = FALSE;
			}
		}

		if ( fCheckGivenGridNo )
		{
			sDistance = PlotPath( pSoldier, sGridNo,	NO_COPYROUTE, NO_PLOT, TEMPORARY, (INT16)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints );

			if ( sDistance > 0 )
		{

			if ( sDistance < sClosest )
			{
				sClosest			= sDistance;
					sCloseGridNo  = sGridNo;
			}
		}
		}
	}


	for (INT8 cnt = 0; cnt < 4; cnt++)
	{
		// MOVE OUT TWO DIRECTIONS
		sFourGrids[cnt] = sSpot = NewGridNo( sGridNo, DirectionInc( sDirs[ cnt ] ) );

		ubTestDirection = sDirs[ cnt ];

		if ( pDoor && pDoor->fFlags & STRUCTURE_SWITCH )
		{
			ubTestDirection = gOppositeDirection[ ubTestDirection ];
		}

		if ( gubWorldMovementCosts[ sSpot ][ ubTestDirection ][ pSoldier->pathing.bLevel ] >= TRAVELCOST_BLOCKED )
		{
			// obstacle or wall there!
			continue;
		}

		ubWhoIsThere = WhoIsThere2( sSpot, pSoldier->pathing.bLevel );
		if ( ubWhoIsThere != NOBODY && ubWhoIsThere != pSoldier->ubID )
		{
			// skip this direction b/c it's blocked by another merc!
			continue;
		}

		// Eliminate some directions if we are looking at doors!
		if ( pDoor != NULL )
		{
			// Get orinetation
			ubWallOrientation = pDoor->ubWallOrientation;

			// Refuse the south and north and west	directions if our orientation is top-right
			if ( ubWallOrientation == OUTSIDE_TOP_RIGHT || ubWallOrientation == INSIDE_TOP_RIGHT )
			{
				if ( sDirs[ cnt ] == NORTH || sDirs[ cnt ] == WEST || sDirs[ cnt ] == SOUTH )
					continue;
			}

			// Refuse the north and west and east directions if our orientation is top-right
			if ( ubWallOrientation == OUTSIDE_TOP_LEFT || ubWallOrientation == INSIDE_TOP_LEFT )
			{
				if ( sDirs[ cnt ] == NORTH || sDirs[ cnt ] == WEST || sDirs[ cnt ] == EAST )
					continue;
			}
		}

		// first tile is okay, how about the second?
		sSpot2 = NewGridNo( sSpot, DirectionInc( sDirs[ cnt ] ) );
		if ( gubWorldMovementCosts[ sSpot2 ][ sDirs[cnt] ][ pSoldier->pathing.bLevel ] >= TRAVELCOST_BLOCKED ||
			DoorTravelCost( pSoldier, sSpot2, gubWorldMovementCosts[ sSpot2 ][ sDirs[cnt] ][ pSoldier->pathing.bLevel ], (BOOLEAN) (pSoldier->bTeam == gbPlayerNum), NULL ) == TRAVELCOST_DOOR ) // closed door blocks!
		{
			// obstacle or wall there!
			continue;
		}

		ubWhoIsThere = WhoIsThere2( sSpot2, pSoldier->pathing.bLevel );
		if ( ubWhoIsThere != NOBODY && ubWhoIsThere != pSoldier->ubID )
		{
			// skip this direction b/c it's blocked by another merc!
			continue;
		}

		sSpot = sSpot2;

		// If this spot is our soldier's gridno use that!
		if ( sSpot == pSoldier->sGridNo )
		{
			if (pubDirection)
		{
			(*pubDirection) = (UINT8)GetDirectionFromGridNo( sGridNo, pSoldier );
		}
			//*pubDirection = pSoldier->ubDirection;
			return( sSpot );
		}

		ubDir = (UINT8)GetDirectionToGridNoFromGridNo( sSpot, sGridNo );

		// don't store path, just measure it
		if ( ( NewOKDestinationAndDirection( pSoldier, sSpot, ubDir, TRUE , pSoldier->pathing.bLevel ) > 0 ) &&
			( ( sDistance = PlotPath( pSoldier, sSpot,	NO_COPYROUTE, NO_PLOT, TEMPORARY, (INT16)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints ) ) > 0 ) )
		{
			if ( sDistance < sClosest )
			{
				sClosest			= sDistance;
				sCloseGridNo = sSpot;
			}
		}
	}
	
	if (!TileIsOutOfBounds(sClosest))
	{
		// Take last direction and use opposite!
		// This will be usefull for ours and AI mercs

		// If our gridno is the same ( which can be if we are look at doors )
		if ( sGridNo == sCloseGridNo )
		{
			if (pubDirection)
			{
				// ATE: Only if we have a valid door!
				if ( pDoor )
				{
					switch( pDoor->pDBStructureRef->pDBStructure->ubWallOrientation )
					{
					case OUTSIDE_TOP_LEFT:
					case INSIDE_TOP_LEFT:

						*pubDirection = SOUTH;
						break;

					case OUTSIDE_TOP_RIGHT:
					case INSIDE_TOP_RIGHT:

						*pubDirection = EAST;
						break;
					}
				}
			}
		}
		else
		{
			// Calculate direction if our gridno is different....
			ubDir = (UINT8)GetDirectionToGridNoFromGridNo( sCloseGridNo, sGridNo );
			if (pubDirection)
			{
				*pubDirection = ubDir;
			}
		}
		
		if (TileIsOutOfBounds(sCloseGridNo))
		{
			return( -1 );
		}
		return( sCloseGridNo );
	}
	else
		return( -1 );



	/*
	if ( !TileIsOutOfBounds(sCloseGridNo))
	{
	// Take last direction and use opposite!
	// This will be usefull for ours and AI mercs

	// If our gridno is the same ( which can be if we are look at doors )
	if ( sGridNo == sCloseGridNo )
	{
	switch( pDoor->pDBStructureRef->pDBStructure->ubWallOrientation )
	{
	case OUTSIDE_TOP_LEFT:
	case INSIDE_TOP_LEFT:

	*pubDirection = SOUTH;
	break;

	case OUTSIDE_TOP_RIGHT:
	case INSIDE_TOP_RIGHT:

	*pubDirection = EAST;
	break;
	}
	}
	else
	{
	// Calculate direction if our gridno is different....
	ubDir = (UINT8)GetDirectionToGridNoFromGridNo( sCloseGridNo, sGridNo );
	*pubDirection = ubDir;
	}
	return( sCloseGridNo );
	}
	else
	return( -1 );
	*/
}

INT32 FindAdjacentPunchTarget( SOLDIERTYPE * pSoldier, SOLDIERTYPE * pTargetSoldier, INT32 * psAdjustedTargetGridNo, UINT8 * pubDirection )
{
	INT32	sSpot;	
	UINT8	ubGuyThere;

	for ( INT8 cnt = 0; cnt < NUM_WORLD_DIRECTIONS; cnt++ )
	{
		sSpot = NewGridNo( pSoldier->sGridNo, DirectionInc( cnt ) );

		if ( DoorTravelCost( pSoldier, sSpot, gubWorldMovementCosts[ sSpot ][ cnt ][ pSoldier->pathing.bLevel ], FALSE, NULL ) >= TRAVELCOST_BLOCKED )
		{
			// blocked!
			continue;
		}

		// Check for who is there...
		ubGuyThere = WhoIsThere2( sSpot, pSoldier->pathing.bLevel );

		if ( pTargetSoldier != NULL && ubGuyThere == pTargetSoldier->ubID )
		{
			// We've got a guy here....
			// Who is the one we want......
			*psAdjustedTargetGridNo	= pTargetSoldier->sGridNo;
			*pubDirection		= ( UINT8 )cnt;
			return( sSpot );
		}
	}

	return( NOWHERE );
}


BOOLEAN UIOKMoveDestination( SOLDIERTYPE *pSoldier, INT32 usMapPos )
{
	BOOLEAN fVisible;

	// Check if a hidden tile exists but is not revealed
	if ( DoesGridNoContainHiddenStruct( usMapPos, &fVisible ) )
	{
		if ( !fVisible )
		{
			// The player thinks this is OK!
			return( TRUE );
		}
	}


	if ( !NewOKDestination( pSoldier, usMapPos, FALSE, (INT8) gsInterfaceLevel ) )
	{
		return( FALSE );
	}

	// ATE: If we are a robot, see if we are being validly controlled...
	if ( pSoldier->flags.uiStatusFlags & SOLDIER_ROBOT )
	{
		if ( ! pSoldier->CanRobotBeControlled( ) )
		{
			// Display message that robot cannot be controlled....
			return( 2 );
		}
	}

	// ATE: Experiment.. take out
	//else if ( IsRoofVisible( sMapPos ) && gsInterfaceLevel == 0 )
	//{
	//	return( FALSE );
	//}


	return( TRUE);
}


void HandleTeamServices( UINT8 ubTeamNum )
{
	INT32							cnt;
	SOLDIERTYPE			 *pTeamSoldier, *pTargetSoldier;
	UINT32					uiPointsUsed;
	UINT16					usSoldierIndex, usInHand;
	UINT16					usKitPts;
	INT8										bSlot;
	BOOLEAN									fDone;

	// IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
	cnt = gTacticalStatus.Team[ ubTeamNum ].bFirstID;

	// look for all mercs on the same team,
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ ubTeamNum ].bLastID; cnt++,pTeamSoldier++)
	{
		if ( pTeamSoldier->stats.bLife >= OKLIFE && pTeamSoldier->bActive && pTeamSoldier->bInSector )
		{
			fDone = FALSE;
			// Check for different events!
			// FOR DOING AID
			if ( pTeamSoldier->usAnimState == GIVING_AID )
			{
				// Get medkit info
				usInHand = pTeamSoldier->inv[ HANDPOS ].usItem;

				// Get victim pointer
				usSoldierIndex = WhoIsThere2( pTeamSoldier->sTargetGridNo, pTeamSoldier->pathing.bLevel );
				if ( usSoldierIndex != NOBODY )
				{
					pTargetSoldier = MercPtrs[ usSoldierIndex ];

					if ( pTargetSoldier->ubServiceCount )
					{
						BOOLEAN fThrowMessage = (pTargetSoldier->bBleeding ? TRUE : FALSE); // added by SANDRO

						usKitPts = TotalPoints( &(pTeamSoldier->inv[ HANDPOS ] ) );

						uiPointsUsed = pTeamSoldier->SoldierDressWound( pTargetSoldier, usKitPts, usKitPts );

						// Determine if they are all banagded
						if ( !pTargetSoldier->bBleeding && pTargetSoldier->stats.bLife >= OKLIFE && !(pTargetSoldier->iHealableInjury >= 100 && pTeamSoldier->fDoingSurgery)) // check for surgery added by SANDRO
						{
							if ( fThrowMessage ) // throw message "all bandaged" only if there was something to bandage - SANDRO
								ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ MERC_IS_ALL_BANDAGED_STR ], pTargetSoldier->name );

							// Cancel all services for this guy!
							pTargetSoldier->ReceivingSoldierCancelServices( );
							fDone = TRUE;
						}

						UseKitPoints( &(pTeamSoldier->inv[ HANDPOS ] ), (UINT16)uiPointsUsed, pTeamSoldier );

						// Get new total
						usKitPts = TotalPoints( &(pTeamSoldier->inv[ HANDPOS ] ) );

						// WHETHER OR NOT recipient is all bandaged, check if we've used them up!
						if ( usKitPts <= 0)	 // no more bandages
						{
							if ( fDone )
							{
								// don't swap if we're done
								bSlot = NO_SLOT;
							}
							else
							{
								bSlot = FindFirstAidKit ( pTeamSoldier );
								if ( bSlot == NO_SLOT )
								{
									bSlot = FindMedKit( pTeamSoldier );
								}
							}
							if ( bSlot != NO_SLOT )
							{
								SwapObjs( pTeamSoldier, HANDPOS, bSlot, TRUE );
							}
							else
							{
								ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ MERC_IS_OUT_OF_BANDAGES_STR ], pTeamSoldier->name );
								pTeamSoldier->GivingSoldierCancelServices( );

								if ( !gTacticalStatus.fAutoBandageMode )
								{
									pTeamSoldier->DoMercBattleSound( (INT8)( BATTLE_SOUND_CURSE1 ) );
								}

							}
						}
					}
				}
			}
		}

	}
}

void HandlePlayerServices( SOLDIERTYPE *pTeamSoldier )
{
	SOLDIERTYPE	 *pTargetSoldier;
	UINT32					uiPointsUsed;
	UINT16					usSoldierIndex, usInHand;
	UINT16					usKitPts;
	INT8										bSlot;
	BOOLEAN									fDone = FALSE;

	if ( pTeamSoldier->stats.bLife >= OKLIFE && pTeamSoldier->bActive )
	{
		// Check for different events!
		// FOR DOING AID
		if ( pTeamSoldier->usAnimState == GIVING_AID )
		{
			// Get medkit info
			usInHand = pTeamSoldier->inv[ HANDPOS ].usItem;

			// Get victim pointer
			usSoldierIndex = WhoIsThere2( pTeamSoldier->sTargetGridNo, pTeamSoldier->pathing.bLevel );

			if ( usSoldierIndex != NOBODY )
			{
				pTargetSoldier = MercPtrs[ usSoldierIndex ];

				if ( pTargetSoldier->ubServiceCount )
				{
					BOOLEAN fThrowMessage = (pTargetSoldier->bBleeding ? TRUE : FALSE); // added by SANDRO 

					usKitPts = TotalPoints( &(pTeamSoldier->inv[ HANDPOS ] ) );

					uiPointsUsed = pTeamSoldier->SoldierDressWound( pTargetSoldier, usKitPts, usKitPts );

					// Determine if they are all banagded
					if ( !pTargetSoldier->bBleeding && pTargetSoldier->stats.bLife >= OKLIFE && !(pTargetSoldier->iHealableInjury >= 100 && pTeamSoldier->fDoingSurgery)) // check for surgery added by SANDRO
					{
						if ( fThrowMessage ) // throw message "all bandaged" only if there was something to bandage - SANDRO
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ MERC_IS_ALL_BANDAGED_STR ], pTargetSoldier->name );

						// Cancel all services for this guy!
						pTargetSoldier->ReceivingSoldierCancelServices( );
						fDone = TRUE;
					}

					UseKitPoints( &(pTeamSoldier->inv[ HANDPOS ] ), (UINT16)uiPointsUsed, pTeamSoldier );

					// Get new total
					usKitPts = TotalPoints( &(pTeamSoldier->inv[ HANDPOS ] ) );

					// WHETHER OR NOT recipient is all bandaged, check if we've used them up!
					if ( usKitPts <= 0)	 // no more bandages
					{
						if ( fDone )
						{
							// don't swap if we're done
							bSlot = NO_SLOT;
						}
						else
						{
							// SANDRO - on surgery, only search for medic bags
							if (pTeamSoldier->fDoingSurgery)
							{
								bSlot = FindMedKit ( pTeamSoldier );
							}
							else
							{
								bSlot = FindFirstAidKit ( pTeamSoldier);
								if ( bSlot == NO_SLOT )
								{
									bSlot = FindMedKit ( pTeamSoldier );
								}
							}
						}

						if ( bSlot != NO_SLOT )
						{
							SwapObjs( pTeamSoldier, HANDPOS, bSlot, FALSE ); // SANDRO - changed to FALSE, so we can continue in the service (???)
						}
						else
						{
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ MERC_IS_OUT_OF_BANDAGES_STR ], pTeamSoldier->name );
							pTeamSoldier->GivingSoldierCancelServices( );

							if ( !gTacticalStatus.fAutoBandageMode )
							{
								pTeamSoldier->DoMercBattleSound( (INT8)( BATTLE_SOUND_CURSE1 ) );
							}
						}
					}
				}
			}
		}
	}
}

void CommonEnterCombatModeCode( )
{
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"CommonEnterCombatMode");
	UINT32					cnt;
	SOLDIERTYPE			 *pSoldier;

	gTacticalStatus.uiFlags |= INCOMBAT;

	//gTacticalStatus.ubAttackBusyCount = 0;

	// Reset num enemies fought flag...
	// SANDRO - removed from here
	//memset( &(gTacticalStatus.bNumFoughtInBattle), 0, MAXTEAMS );
	gTacticalStatus.ubLastBattleSectorX = (UINT8) gWorldSectorX;
	gTacticalStatus.ubLastBattleSectorY = (UINT8) gWorldSectorY;
	gTacticalStatus.fLastBattleWon		= FALSE;
	gTacticalStatus.fItemsSeenOnAttack	= FALSE;


	// ATE: If we have an item pointer end it!
	CancelItemPointer( );

	ResetInterfaceAndUI( );
	ResetMultiSelection( );

	// OK, loop thorugh all guys and stop them!
	// Loop through all mercs and make go
	for ( pSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pSoldier++, cnt++ )
	{
		if ( pSoldier->bActive )
		{
			if ( pSoldier->bInSector && pSoldier->ubBodyType != CROW )
			{
				// Set some flags for quotes
				pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_IN_SHIT );
				pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_MULTIPLE_CREATURES);

				// Hault!
				pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );

				// END AI actions
				CancelAIAction( pSoldier, TRUE );

				// turn off AI controlled flag
				pSoldier->flags.uiStatusFlags &= ~SOLDIER_UNDERAICONTROL;

				// Check if this guy is an enemy....
				// SANDRO - don't check this here, we've already checked that once
				//CheckForPotentialAddToBattleIncrement( pSoldier );

				// If guy is sleeping, wake him up!
				if ( pSoldier->flags.fMercAsleep == TRUE )
				{
					pSoldier->ChangeSoldierState( WKAEUP_FROM_SLEEP, 1, TRUE );
				}

				// ATE: Refresh APs
				pSoldier->CalcNewActionPoints( );
				// Either I just noticed this, or it just hasn't been a problem before.	But
				// if the last battle left a soldier at 0 points, he will not gain full points with
				// carryover for this battle.	So we'll hit it again.
				pSoldier->CalcNewActionPoints( );

				if ( pSoldier->ubProfile != NO_PROFILE )
				{
					if ( pSoldier->bTeam == CIV_TEAM && pSoldier->aiData.bNeutral )
					{
						// only set precombat gridno if unset						
						if ( gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo == 0 || TileIsOutOfBounds(gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo))
						{
							gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo = pSoldier->sGridNo;
						}
					}
					else
					{
						gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo = NOWHERE;
					}
				}

				if ( !gTacticalStatus.fHasEnteredCombatModeSinceEntering )
				{
					// ATE: reset player's movement mode at the very start of
					// combat
					//if ( pSoldier->bTeam == gbPlayerNum )
					//{
					//pSoldier->usUIMovementMode = RUNNING;
					//}
				}
			}
		}
	}

	gTacticalStatus.fHasEnteredCombatModeSinceEntering = TRUE;

	SyncStrategicTurnTimes( );

	// Play tune..
	PlayJA2Sample( ENDTURN_1, RATE_11025, MIDVOLUME, 1, MIDDLEPAN );

	// Say quote.....

	// Change music modes
	SetMusicMode( MUSIC_TACTICAL_BATTLE );

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"CommonEnterCombatMode done");

}


void EnterCombatMode( UINT8 ubStartingTeam )
{
	UINT32				cnt;
	SOLDIERTYPE		*pTeamSoldier;
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"EnterCombatMode");

	if ( gTacticalStatus.uiFlags & INCOMBAT )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Can't enter combat when already in combat" );
		// we're already in combat!
		return;
	}

	// Alrighty, don't do this if no enemies in sector
	if ( NumCapableEnemyInSector( ) == 0 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Can't enter combat when no capable enemies" );
		//ScreenMsg( MSG_FONT_RED, MSG_DEBUG, L"Trying to init combat when no enemies around!." );
		return;
	}

	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Entering combat mode" );

	// ATE: Added here to guarentee we have fEnemyInSector
	// Mostly this was not getting set if:
	// 1 ) we see bloodcats ( which makes them hostile )
	// 2 ) we make civs hostile
	// only do this once they are seen.....
	if ( !gTacticalStatus.fEnemyInSector )
	{
		SetEnemyPresence( );
	}

	CommonEnterCombatModeCode( );

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"EnterCombatMode continuing...");

	if (ubStartingTeam == gbPlayerNum)
	{
		DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("EnterCombatMode continuing... start player turn, selected soldier = %d",gusSelectedSoldier));
		// OK, make sure we have a selected guy
		// Madd: this was causing a weird crash becuase gusSelectedSoldier was 156 (out of the array bounds) for some reason
		//if ( MercPtrs[ gusSelectedSoldier ]->aiData.bOppCnt == 0 )
		if ( gusSelectedSoldier != NOBODY && MercPtrs[ gusSelectedSoldier ]->aiData.bOppCnt == 0 )
		{
			DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"EnterCombatMode continuing... nobody selected");
			// OK, look through and find one....
			for ( cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID, pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++, pTeamSoldier++ )
			{
				if ( OK_CONTROLLABLE_MERC( pTeamSoldier ) && pTeamSoldier->aiData.bOppCnt > 0 )
				{
					DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"EnterCombatMode continuing... select soldier");
					SelectSoldier( pTeamSoldier->ubID, FALSE, TRUE );
				}
			}
		}
		DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"EnterCombatMode continuing... calling startplayerteamturn");

		if (!is_client || is_server) //hayden
		{
		StartPlayerTeamTurn( FALSE, TRUE );
			if(is_client)send_EndTurn( ubStartingTeam ); //hayden
	}
	else
	{
			//ScreenMsg( FONT_YELLOW, MSG_CHAT, L"client skipped EnterCombatMode");	
			if(is_client)startCombat(ubStartingTeam);//clients other than server meet and send request for combat for server
		}
	}
	else
	{
		DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"EnterCombatMode continuing... end turn");
		// have to call EndTurn so that we freeze the interface etc
		EndTurn( ubStartingTeam );
	}

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"EnterCombatMode done");

}



void ExitCombatMode( )
{
	UINT32							cnt;
	SOLDIERTYPE			 *pSoldier;

	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Exiting combat mode" );

	// Leave combat mode
	gTacticalStatus.uiFlags &= (~INCOMBAT);

	EndTopMessage( );

	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_FTM_EXIT_COMBAT ] );// ary-05/05/2009 : add forced turn mode

	// OK, we have exited combat mode.....
	// Reset some flags for no aps to move, etc

	// Set virgin sector to true....
	gTacticalStatus.fVirginSector = TRUE;

	// Loop through all mercs and make go
	for ( pSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pSoldier++, cnt++ )
	{
		if ( pSoldier->bActive )
		{
			if ( pSoldier->bInSector )
			{
				// Reset some flags
				if ( pSoldier->flags.fNoAPToFinishMove && pSoldier->stats.bLife >= OKLIFE )
				{
					pSoldier->AdjustNoAPToFinishMove( FALSE );

					// ary-05/05/2009 : fix lower ready weapons
					//previously "ready weapon" state was being dropped in a couple of cases
					//the fix involves bypassing the reset animation state for the various "ready weapon" types
					//since this is a reset animation function, we should be VERY specific about when and what we dont reset

					UINT16	test;
					test = pSoldier->usAnimState; 
					if (!( 	test == AIM_RIFLE_STAND ||	test == AIM_RIFLE_CROUCH ||
							test == AIM_RIFLE_PRONE ||	test == AIM_DUAL_STAND 	 ||
							test == AIM_DUAL_CROUCH ||	test == AIM_DUAL_PRONE
						)) 
					{
						pSoldier->SoldierGotoStationaryStance( );
					}				
				}

				//Cancel pending events
				pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
				pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
				pSoldier->aiData.ubPendingAction	= NO_PENDING_ACTION;

				// Reset moved flag
				pSoldier->aiData.bMoved = FALSE;

				// Set final destination
				pSoldier->pathing.sFinalDestination	= pSoldier->sGridNo;

				// remove AI controlled flag
				pSoldier->flags.uiStatusFlags &= ~SOLDIER_UNDERAICONTROL;
			}
		}
	}

	// Change music modes
	// unused
	//gfForceMusicToTense = TRUE;

	SetMusicMode( MUSIC_TACTICAL_ENEMYPRESENT );

	BetweenTurnsVisibilityAdjustments();

	// pause the AI for a bit
	PauseAITemporarily();

	// reset muzzle flashes
	TurnOffEveryonesMuzzleFlashes();

	// zap interrupt list
	ClearIntList();

	// dirty interface
	DirtyMercPanelInterface( MercPtrs[ 0 ], DIRTYLEVEL2 );

	// ATE: If we are IN_CONV - DONT'T DO THIS!
	if ( !( gTacticalStatus.uiFlags & ENGAGED_IN_CONV ) )
	{
		HandleStrategicTurnImplicationsOfExitingCombatMode();
	}

	// Make sure next opplist decay DOES happen right after we go to RT
	// since this would be the same as what would happen at the end of the turn
	gTacticalStatus.uiTimeSinceLastOpplistDecay = __max( 0, GetWorldTotalSeconds() - TIME_BETWEEN_RT_OPPLIST_DECAYS );
	NonCombatDecayPublicOpplist( GetWorldTotalSeconds() );
}


void SetEnemyPresence( )
{
	// We have an ememy present....
	DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("SetEnemyPresence"));

	// Check if we previously had no enemys present and we are in a virgin secotr ( no enemys spotted yet )
	if ( !gTacticalStatus.fEnemyInSector && gTacticalStatus.fVirginSector )
	{
		// If we have a guy selected, say quote!
		// For now, display ono status message
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ ENEMY_IN_SECTOR_STR ] );

		// Change music modes..

		// If we are just starting game, don't do this!
		if ( !DidGameJustStart() && !AreInMeanwhile( ) )
		{
			SetMusicMode( MUSIC_TACTICAL_ENEMYPRESENT );
			DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("SetEnemyPresence: warnings = false"));
			sniperwarning = FALSE;
			biggunwarning = FALSE;
			gogglewarning = FALSE;
			//			airstrikeavailable = TRUE;
		}
		else
		{
			DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("SetEnemyPresence: warnings = true"));
			sniperwarning = TRUE;
			biggunwarning = TRUE;
			//gogglewarning = TRUE;
			//			airstrikeavailable = FALSE;
		}

		// Say quote...
		//SayQuoteFromAnyBodyInSector( QUOTE_ENEMY_PRESENCE );

		gTacticalStatus.fEnemyInSector = TRUE;

	}
}


extern BOOLEAN gfLastMercTalkedAboutKillingID;

BOOLEAN SoldierHasSeenEnemiesLastFewTurns( SOLDIERTYPE *pTeamSoldier )
{
	INT32					cnt2;
	SOLDIERTYPE		*pSoldier;
	INT32					cnt;

	for ( cnt = 0; cnt < MAXTEAMS; cnt++ )
	{

		if ( gTacticalStatus.Team[ cnt ].bSide != pTeamSoldier->bSide )
		{

			// check this team for possible enemies
			cnt2 = gTacticalStatus.Team[ cnt ].bFirstID;
			for ( pSoldier = MercPtrs[ cnt2 ]; cnt2 <= gTacticalStatus.Team[ cnt ].bLastID; cnt2++, pSoldier++ )
			{
				if ( pSoldier->bActive && pSoldier->bInSector && ( pSoldier->bTeam == gbPlayerNum || pSoldier->stats.bLife >= OKLIFE ) )
				{
					if ( !CONSIDERED_NEUTRAL( pTeamSoldier, pSoldier ) && ( pTeamSoldier->bSide != pSoldier->bSide ) )
					{
						// Have we not seen this guy.....
						if ( ( pTeamSoldier->aiData.bOppList[ cnt2 ] >= SEEN_CURRENTLY ) &&
							( pTeamSoldier->aiData.bOppList[ cnt2 ] <= SEEN_THIS_TURN ) )
						{
							gTacticalStatus.bConsNumTurnsNotSeen = 0;
							return( TRUE );
						}
					}
				}
			}
		}
	}

	return( FALSE );
}

BOOLEAN WeSeeNoOne( void )
{
	UINT32		uiLoop;
	SOLDIERTYPE * pSoldier;

	for ( uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++ )
	{
		pSoldier = MercSlots[ uiLoop ];
		if ( pSoldier != NULL )
		{
			if ( pSoldier->bTeam == gbPlayerNum )
			{
				if ( pSoldier->aiData.bOppCnt > 0 )
				{
					return( FALSE );
				}
			}
		}
	}

	return( TRUE );
}

BOOLEAN NobodyAlerted( void )
{
	UINT32		uiLoop;
	SOLDIERTYPE * pSoldier;

	for ( uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++ )
	{
		pSoldier = MercSlots[ uiLoop ];
		if ( pSoldier != NULL )
		{
			if ( ( pSoldier->bTeam != gbPlayerNum ) && ( ! pSoldier->aiData.bNeutral ) && (pSoldier->stats.bLife >= OKLIFE) && (pSoldier->aiData.bAlertStatus >= STATUS_RED) )
			{
				return( FALSE );
			}
		}
	}

	return( TRUE );
}


BOOLEAN WeSawSomeoneThisTurn( void )
{
	UINT32		uiLoop, uiLoop2;
	SOLDIERTYPE * pSoldier;

	for ( uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++ )
	{
		pSoldier = MercSlots[ uiLoop ];
		if ( pSoldier != NULL )
		{
			if ( pSoldier->bTeam == gbPlayerNum )
			{
				for ( uiLoop2 = gTacticalStatus.Team[ ENEMY_TEAM ].bFirstID; uiLoop2 < TOTAL_SOLDIERS; uiLoop2++ )
				{
					if ( pSoldier->aiData.bOppList[ uiLoop2 ] == SEEN_THIS_TURN )
					{
						return( TRUE );
					}
				}
			}
		}
	}
	return( FALSE );
}


void SayBattleSoundFromAnyBodyInSector( INT32 iBattleSnd )
{
// WDS - make number of mercenaries, etc. be configurable
//	UINT8	ubMercsInSector[ CODE_MAXIMUM_NUMBER_OF_PLAYER_SLOTS ] = { 0 };
	std::vector<UINT8>	ubMercsInSector (CODE_MAXIMUM_NUMBER_OF_PLAYER_SLOTS, 0);
	UINT8	ubNumMercs = 0;
	UINT8	ubChosenMerc;
	SOLDIERTYPE *pTeamSoldier;
	INT32 cnt;

	// Loop through all our guys and randomly say one from someone in our sector

	// set up soldier ptr as first element in mercptrs list
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	// run through list
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pTeamSoldier++ )
	{
		// Add guy if he's a candidate...
		if ( OK_INSECTOR_MERC( pTeamSoldier ) && !AM_AN_EPC( pTeamSoldier ) && !( pTeamSoldier->flags.uiStatusFlags & SOLDIER_GASSED ) && !(AM_A_ROBOT( pTeamSoldier )) && !pTeamSoldier->flags.fMercAsleep )
		{
			ubMercsInSector[ ubNumMercs ] = (UINT8)cnt;
			ubNumMercs++;
		}
	}

	// If we are > 0
	if ( ubNumMercs > 0 )
	{
		ubChosenMerc = (UINT8)Random( ubNumMercs );

		MercPtrs[ ubChosenMerc ]->DoMercBattleSound( (UINT8)iBattleSnd );
	}

}


BOOLEAN CheckForEndOfCombatMode( BOOLEAN fIncrementTurnsNotSeen )
{
	SOLDIERTYPE *pTeamSoldier;
	UINT32 cnt = 0;
	BOOLEAN	fWeSeeNoOne, fNobodyAlerted;
	BOOLEAN	fSayQuote = FALSE;
	BOOLEAN fWeSawSomeoneRecently = FALSE, fSomeoneSawSomeoneRecently = FALSE;




		//if(is_server && check_status())
		//{
		//	return(FALSE);
		//}

	// First check for end of battle....
	// If there are no enemies at all in the sector
	// Battle end should take precedence!
	if ( CheckForEndOfBattle( FALSE ) )
	{
		return( TRUE );
	}

	// We can only check for end of combat if in combat mode
	if ( ! ( gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		return( FALSE );
	}
	//if (is_server || is_client)
	//{
	//	return(FALSE); // cheap band-aid to prevent return from turn based action in multiplayer, atleast until further dev... : hayden.
	//}

	// if we're boxing NEVER end combat mode
	if ( gTacticalStatus.bBoxingState == BOXING )
	{
		return( FALSE );
	}


	fWeSeeNoOne = WeSeeNoOne();
	fNobodyAlerted = NobodyAlerted();
	if ( fWeSeeNoOne && fNobodyAlerted )
	{
		// hack!!!
		gTacticalStatus.bConsNumTurnsNotSeen = 5;
	}
	else
	{

		// we have to loop through EVERYONE to see if anyone sees a hostile... if so, stay in turnbased...

		for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
		{
			pTeamSoldier = MercSlots[ cnt ];
			if ( pTeamSoldier && pTeamSoldier->stats.bLife >= OKLIFE && !pTeamSoldier->aiData.bNeutral )
			{
				if ( SoldierHasSeenEnemiesLastFewTurns( pTeamSoldier ) )
				{
					gTacticalStatus.bConsNumTurnsNotSeen = 0;
					fSomeoneSawSomeoneRecently = TRUE;
					if ( pTeamSoldier->bTeam == gbPlayerNum || (pTeamSoldier->bTeam == MILITIA_TEAM && pTeamSoldier->bSide == 0 ) ) // or friendly militia
					{
						fWeSawSomeoneRecently = TRUE;
						break;
					}
				}
			}
		}

		if ( fSomeoneSawSomeoneRecently )
		{
			if ( fWeSawSomeoneRecently )
			{
				gTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes = 0;
			}
			else
			{
				// start tracking this
				gTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes++;
			}
			return( FALSE );
		}

		// IF here, we don;t see anybody.... increment count for end check
		if( fIncrementTurnsNotSeen )
		{
			gTacticalStatus.bConsNumTurnsNotSeen++;
		}
	}

	gTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes = 0;

	// If we have reach a point where a cons. number of turns gone by....
	//if ( gTacticalStatus.bConsNumTurnsNotSeen > 1 )
	// ary-05/05/2009 : add forced turn mode : note : no forced turn mode option for Multi Player
	if ( is_networked ) gGameSettings.fOptions[ TOPTION_TOGGLE_TURN_MODE ] = FALSE;
	if ( gTacticalStatus.bConsNumTurnsNotSeen > 1 && !gGameSettings.fOptions[ TOPTION_TOGGLE_TURN_MODE ])
	{

		if(is_networked && !getReal)//hayden
		{
			//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, MPClientMessage[32] );

			sendRT();

			return(FALSE);
		}
		gTacticalStatus.bConsNumTurnsNotSeen = 0;
		
		// Exit mode!
		ExitCombatMode();


		if ( fNobodyAlerted )
		{
			// if we don't see anyone currently BUT we did see someone this turn, THEN don't
			// say quote
			if ( fWeSeeNoOne && WeSawSomeoneThisTurn() )
			{
				// don't say quote
			}
			else
			{
				fSayQuote = TRUE;
			}
		}
		else
		{
			fSayQuote = TRUE;
		}

		// ATE: Are there creatures here? If so, say another quote...
		if ( fSayQuote && ( gTacticalStatus.uiFlags & IN_CREATURE_LAIR ) )
		{
			SayQuoteFromAnyBodyInSector( QUOTE_WORRIED_ABOUT_CREATURE_PRESENCE );
		}
		// Are we fighting bloodcats?
		else if ( NumBloodcatsInSectorNotDeadOrDying( ) > 0 )
		{

		}
		else
		{
			if ( fSayQuote )
			{
				// Double check by seeing if we have any active enemies in sector!
				if ( NumEnemyInSectorNotDeadOrDying( ) > 0 )
				{
					//comm by ddd SayQuoteFromAnyBodyInSector( QUOTE_WARNING_OUTSTANDING_ENEMY_AFTER_RT );
					if(Chance(gGameExternalOptions.iChanceSayAnnoyingPhrase) )
						SayQuoteFromAnyBodyInSector( QUOTE_WARNING_OUTSTANDING_ENEMY_AFTER_RT );
				}
			}
		}
		/*
		if ( (!fWeSeeNoOne || !fNobodyAlerted) && WeSawSomeoneThisTurn() )
		{
		// Say quote to the effect that the battle has lulled
		SayQuoteFromAnyBodyInSector( QUOTE_WARNING_OUTSTANDING_ENEMY_AFTER_RT );
		}
		*/

		// Begin tense music....
		// unused
		//gfForceMusicToTense = TRUE;
		SetMusicMode( MUSIC_TACTICAL_ENEMYPRESENT );

		return( TRUE );
	}

	return( FALSE );
}


void DeathNoMessageTimerCallback( void )
{
	//CheckAndHandleUnloadingOfCurrentWorld();
	if(!is_client)
		CheckAndHandleUnloadingOfCurrentWorld();
	else	
	{
		if (!isOwnTeamWipedOut)
		{
			ScreenMsg( FONT_LTGREEN, MSG_CHAT, MPClientMessage[40] );
			if(!cDisableSpectatorMode)
			{
				gTacticalStatus.uiFlags |= SHOW_ALL_MERCS;//hayden
				ScreenMsg( FONT_YELLOW, MSG_CHAT, MPClientMessage[41] );
			}
			else 
				ScreenMsg( FONT_LTBLUE, MSG_CHAT, L"spectator mode disabled");
			
			teamwiped();
		}
	}
}

void RemoveStaticEnemiesFromSectorInfo( INT16 sMapX, INT16 sMapY, INT8 bMapZ )
{
    if (!bMapZ) // Battle ended Above-ground
    {
        SECTORINFO *pSectorInfo = &( SectorInfo[ SECTOR( sMapX, sMapY ) ] );

        pSectorInfo->ubNumAdmins = pSectorInfo->ubNumTroops = pSectorInfo->ubNumElites = 0;
        pSectorInfo->ubAdminsInBattle = pSectorInfo->ubTroopsInBattle = pSectorInfo->ubElitesInBattle = 0;
    }
    else
    {
        UNDERGROUND_SECTORINFO *pSectorInfo;
        
        pSectorInfo = FindUnderGroundSector( sMapX, sMapY, bMapZ );
        pSectorInfo->ubNumAdmins = pSectorInfo->ubNumTroops = pSectorInfo->ubNumElites = 0;
        pSectorInfo->ubAdminsInBattle = pSectorInfo->ubTroopsInBattle = pSectorInfo->ubElitesInBattle = 0;
    }
}


//!!!!
//IMPORTANT NEW NOTE:
//Whenever returning TRUE, make sure you clear gfBlitBattleSectorLocator;
BOOLEAN CheckForEndOfBattle( BOOLEAN fAnEnemyRetreated )
{
	SOLDIERTYPE *pTeamSoldier;
	BOOLEAN			fBattleWon = TRUE;
	BOOLEAN			fBattleLost = FALSE;
	INT32				cnt = 0;
	UINT16			usAnimState;

	if ( gTacticalStatus.bBoxingState == BOXING )
	{
		// no way are we going to exit boxing prematurely, thank you! :-)
		return( FALSE );
	}

	// OJW - 090212 - Fix end conditions for multiplayer - TeamDM
	if(is_server)
	{
		// check the server's conditions for continueing the game, if the server wants to continue the game it returns true
		// hence we return false that the battle has ended. If not, when this function returns below we will force the game to end.
		if ( check_status() )
			return(FALSE);
		// the block of code below would always cause this function to exit before checking the servers desires
		// in cases of team deathmatch where there were no more enemies
	}
	else
	{
		// We can only check for end of battle if in combat mode or there are enemies
		// present (they might bleed to death or run off the map!)
		if ( ! ( gTacticalStatus.uiFlags & INCOMBAT ) )
		{
			if ( ! (gTacticalStatus.fEnemyInSector) )
			{
				return( FALSE );
			}
		}
	}

	// ATE: If attack busy count.. get out...
	if ( (gTacticalStatus.ubAttackBusyCount > 0 ) )
	{
		return( FALSE );
	}


	// OK, this is to releave infinate looping...becasue we can kill guys in this function
	if ( gfKillingGuysForLosingBattle )
	{
		return( FALSE );
	}

	// Check if the battle is won!
	if ( NumCapableEnemyInSector( ) > 0 )
	{
		fBattleWon = FALSE;
	}

	if ( CheckForLosingEndOfBattle( ) == TRUE )
	{
		fBattleLost = TRUE;
	}

	//NEW (Nov 24, 98)	by Kris
	if( !gbWorldSectorZ && fBattleWon )
	{ //Check to see if more enemy soldiers exist in the strategic layer
		//It is possible to have more than 20 enemies in a sector.	By failing here,
		//it gives the engine a chance to add these soldiers as reinforcements.	This
		//is naturally handled.
		AddPossiblePendingEnemiesToBattle();
		if( gfPendingEnemies )
		{
			fBattleWon = FALSE;
		}
	}

	if( ( fBattleLost ) || ( fBattleWon ) )
	{
		if( !gbWorldSectorZ )
		{
			SectorInfo[ SECTOR( gWorldSectorX, gWorldSectorY) ].bLastKnownEnemies = NumEnemiesInSector( gWorldSectorX, gWorldSectorY );
		}
	}

	// We should NEVER have a battle lost and won at the same time...

	if ( fBattleLost )
	{
		// CJC: End AI's turn here.... first... so that UnSetUIBusy will succeed if militia win
		// battle for us
		EndAllAITurns( );

		// Set enemy presence to false
		// This is safe 'cause we're about to unload the friggen sector anyway....
		gTacticalStatus.fEnemyInSector = FALSE;
		// SANDRO - reset number of enemies here
		memset( &(gTacticalStatus.bNumFoughtInBattle), 0, MAXTEAMS );

		// If here, the battle has been lost!
		UnSetUIBusy( (UINT8)gusSelectedSoldier );

		if ( gTacticalStatus.uiFlags & INCOMBAT )
		{
			// Exit mode!
			ExitCombatMode();
		}

		HandleMoraleEvent( NULL, MORALE_HEARD_BATTLE_LOST, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
		HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_BATTLE_LOST, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

		// SANDRO - end quest if cleared the sector after interrogation (sector N7 by Meduna)
		if ( gWorldSectorX == 7 && gWorldSectorY == 14 && gbWorldSectorZ == 0 && gubQuest[ QUEST_INTERROGATION ] == QUESTINPROGRESS )
		{
			// Quest failed
			InternalEndQuest( QUEST_INTERROGATION, gWorldSectorX, gWorldSectorY, FALSE );
		}

		// Play death music
		SetMusicMode( MUSIC_TACTICAL_DEATH );

		SetCustomizableTimerCallbackAndDelay( 10000, DeathNoMessageTimerCallback, FALSE );
		
		if (is_networked)
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, MPClientMessage[42] );

		if ( CheckFact( FACT_FIRST_BATTLE_BEING_FOUGHT, 0 ) )
		{
			// this is our first battle... and we lost it!
			SetFactTrue( FACT_FIRST_BATTLE_FOUGHT );
			SetFactFalse( FACT_FIRST_BATTLE_BEING_FOUGHT );
			SetTheFirstBattleSector( (INT16) (gWorldSectorX + gWorldSectorY * MAP_WORLD_X) );
			HandleFirstBattleEndingWhileInTown( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, FALSE );
		}

		if( NumEnemyInSectorExceptCreatures() )
		{
			SetThisSectorAsEnemyControlled( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, TRUE );
		}

		// ATE: Important! THis is delayed until music ends so we can have proper effect!
		// CheckAndHandleUnloadingOfCurrentWorld();

		//Whenever returning TRUE, make sure you clear gfBlitBattleSectorLocator;
		LogBattleResults( LOG_DEFEAT );
		gfBlitBattleSectorLocator = FALSE;
		// If we are the server, we escape this function at the top if we think the game should still be running
		// hence if we get here the game is over for all clients and we should report it
		if (is_networked && is_server)
			game_over();
		return( TRUE );
	}


	// If battle won, do stuff right away!
	if ( fBattleWon )
	{

		if ( gTacticalStatus.bBoxingState == NOT_BOXING ) // if boxing don't do any of this stuff
		{
			gTacticalStatus.fLastBattleWon = TRUE;

			// OK, KILL any enemies that are incompacitated
			if ( KillIncompacitatedEnemyInSector( ) )
			{
				return( FALSE );
			}
		}

		// Kill all enemies. Sometime even after killing all the enemies, there appeares "in battle" enemies in sector info
		RemoveStaticEnemiesFromSectorInfo( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );


		// If here, the battle has been won!
		// hurray! a glorious victory!

		// Set enemy presence to false
		gTacticalStatus.fEnemyInSector = FALSE;

		// CJC: End AI's turn here.... first... so that UnSetUIBusy will succeed if militia win
		// battle for us
		EndAllAITurns( );

		UnSetUIBusy( (UINT8)gusSelectedSoldier );

		// ATE:
		// If we ended battle in any team other than the player's
		// we need to end the UI lock using this method....
		guiPendingOverrideEvent = LU_ENDUILOCK;
		HandleTacticalUI( );

		if ( gTacticalStatus.uiFlags & INCOMBAT )
		{
			// Exit mode!
			ExitCombatMode();
		}

		if ( gTacticalStatus.bBoxingState == NOT_BOXING ) // if boxing don't do any of this stuff
		{

			// Only do some stuff if we actually faught a battle
			if ( gTacticalStatus.bNumFoughtInBattle[ ENEMY_TEAM ] + gTacticalStatus.bNumFoughtInBattle[ CREATURE_TEAM ] + gTacticalStatus.bNumFoughtInBattle[ CIV_TEAM ] > 0 )
				//if ( gTacticalStatus.bNumEnemiesFoughtInBattle > 0 )
			{
				// Loop through all mercs and make go
				for ( pTeamSoldier = MercPtrs[ gTacticalStatus.Team[ gbPlayerNum ].bFirstID ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pTeamSoldier++)
				{
					if ( pTeamSoldier->bActive )
					{
						if ( pTeamSoldier->bInSector )
						{
							if( pTeamSoldier->bTeam == gbPlayerNum )
							{
								// SANDRO - records - num tactical battles
								gMercProfiles[pTeamSoldier->ubProfile].records.usBattlesTactical++;
								// largest battle experienced
								if (gMercProfiles[pTeamSoldier->ubProfile].records.usLargestBattleFought < ( gTacticalStatus.bNumFoughtInBattle[ ENEMY_TEAM ] + gTacticalStatus.bNumFoughtInBattle[ CREATURE_TEAM ] + gTacticalStatus.bNumFoughtInBattle[ CIV_TEAM ] ))
								{
									gMercProfiles[pTeamSoldier->ubProfile].records.usLargestBattleFought = ( gTacticalStatus.bNumFoughtInBattle[ ENEMY_TEAM ] + gTacticalStatus.bNumFoughtInBattle[ CREATURE_TEAM ] + gTacticalStatus.bNumFoughtInBattle[ CIV_TEAM ] );
								}

								// If this guy is OKLIFE & not standing, make stand....
								if ( pTeamSoldier->stats.bLife >= OKLIFE && !pTeamSoldier->bCollapsed )
								{
									if ( pTeamSoldier->bAssignment < ON_DUTY )
									{
										// Reset some quote flags....
										pTeamSoldier->usQuoteSaidExtFlags &= (~SOLDIER_QUOTE_SAID_BUDDY_1_WITNESSED);
										pTeamSoldier->usQuoteSaidExtFlags &= (~SOLDIER_QUOTE_SAID_BUDDY_2_WITNESSED);
										pTeamSoldier->usQuoteSaidExtFlags &= (~SOLDIER_QUOTE_SAID_BUDDY_3_WITNESSED);

										// toggle stealth mode....
										gfUIStanceDifferent = TRUE;
										pTeamSoldier->bStealthMode = FALSE;
										fInterfacePanelDirty = DIRTYLEVEL2;
										//DBrot: Stance change
										if (gGameExternalOptions.fStandUpAfterBattle){
											if ( gAnimControl[ pTeamSoldier->usAnimState ].ubHeight != ANIM_STAND )
											{
												pTeamSoldier->ChangeSoldierStance( ANIM_STAND );
											}
											else
											{
												// If they are aiming, end aim!
												usAnimState = PickSoldierReadyAnimation( pTeamSoldier, TRUE );

												if ( usAnimState != INVALID_ANIMATION )
												{
													pTeamSoldier->EVENT_InitNewSoldierAnim( usAnimState, 0, FALSE );
												}
											}
										}
									}
								}
							}
						}
					}
				}

				HandleMoraleEvent( NULL, MORALE_BATTLE_WON, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
				HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_BATTLE_WON, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

				// Change music modes
				if ( gfLastMercTalkedAboutKillingID == NOBODY || ( gfLastMercTalkedAboutKillingID != NOBODY && !( MercPtrs[ gfLastMercTalkedAboutKillingID ]->flags.uiStatusFlags & SOLDIER_MONSTER ) ) )
				{
					SetMusicMode( MUSIC_TACTICAL_VICTORY );

					// OJW - 20081222 - dont auto-bandage if networked
					if (!is_networked)
						ShouldBeginAutoBandage( );
				}
				else if ( gfLastMercTalkedAboutKillingID != NOBODY && ( MercPtrs[ gfLastMercTalkedAboutKillingID ]->flags.uiStatusFlags & SOLDIER_MONSTER ) )
				{
					// OJW - 20081222 - dont auto-bandage if networked
					if (!is_networked)
						ShouldBeginAutoBandage( );
				}
				// SANDRO - end quest if cleared the sector after interrogation (sector N7 by Meduna)
				if ( gWorldSectorX == 7 && gWorldSectorY == 14 && gbWorldSectorZ == 0 && gubQuest[ QUEST_INTERROGATION ] == QUESTINPROGRESS )
				{
					// Complete quest
					EndQuest( QUEST_INTERROGATION, gWorldSectorX, gWorldSectorY );
				}

				// Say battle end quote....

				if (fAnEnemyRetreated)
				{
					SayQuoteFromAnyBodyInSector( QUOTE_ENEMY_RETREATED );
				}
				else
				{
					// OK, If we have just finished a battle with creatures........ play killed creature quote...
					//
					if ( gfLastMercTalkedAboutKillingID != NOBODY && ( MercPtrs[ gfLastMercTalkedAboutKillingID ]->flags.uiStatusFlags & SOLDIER_MONSTER ) )
					{

					}
					else if ( gfLastMercTalkedAboutKillingID != NOBODY && ( MercPtrs[ gfLastMercTalkedAboutKillingID ]->ubBodyType == BLOODCAT ) )
					{
						SayBattleSoundFromAnyBodyInSector( BATTLE_SOUND_COOL1 );
					}
					else
					{
						SayQuoteFromAnyBodyInSector( QUOTE_SECTOR_SAFE );
					}
				}

				// SANDRO - reset number of enemies here
				memset( &(gTacticalStatus.bNumFoughtInBattle), 0, MAXTEAMS );
			}
			else
			{
				// Change to nothing music...
				SetMusicMode( MUSIC_TACTICAL_NOTHING );
				// OJW - 20081222 - dont auto bandage if networked
				if (!is_networked)
					ShouldBeginAutoBandage();
			}

			HandleMilitiaStatusInCurrentMapBeforeLoadingNewMap();
			//gfStrategicMilitiaChangesMade = TRUE;


			// Loop through all militia and restore them to peaceful status
			cnt = gTacticalStatus.Team[ MILITIA_TEAM ].bFirstID;
			for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ MILITIA_TEAM ].bLastID; cnt++,pTeamSoldier++)
			{
				if ( pTeamSoldier->bActive && pTeamSoldier->bInSector )
				{
					pTeamSoldier->aiData.bAlertStatus = STATUS_GREEN;
					CheckForChangingOrders( pTeamSoldier );
					pTeamSoldier->aiData.sNoiseGridno = NOWHERE;
					pTeamSoldier->aiData.ubNoiseVolume = 0;
					pTeamSoldier->aiData.bNewSituation = FALSE;
					pTeamSoldier->aiData.bOrders = STATIONARY;
					if ( pTeamSoldier->stats.bLife < OKLIFE )
					{
						// SANDRO - the insta-healable value for doctor trait check
						pTeamSoldier->iHealableInjury -= ((OKLIFE - pTeamSoldier->stats.bLife) * 100);
						if (pTeamSoldier->iHealableInjury < 0)
							pTeamSoldier->iHealableInjury = 0;

						pTeamSoldier->stats.bLife = OKLIFE;
					}

					pTeamSoldier->bBleeding = 0; // let's think, the autobandage was done for the militia too
				}
			}
			gTacticalStatus.Team[ MILITIA_TEAM ].bAwareOfOpposition = FALSE;

			// Loop through all civs and restore them to peaceful status
			cnt = gTacticalStatus.Team[ CIV_TEAM ].bFirstID;
			for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ CIV_TEAM ].bLastID; cnt++,pTeamSoldier++)
			{
				if ( pTeamSoldier->bActive && pTeamSoldier->bInSector )
				{
					pTeamSoldier->aiData.bAlertStatus = STATUS_GREEN;
					pTeamSoldier->aiData.sNoiseGridno = NOWHERE;
					pTeamSoldier->aiData.ubNoiseVolume = 0;
					pTeamSoldier->aiData.bNewSituation = FALSE;
					CheckForChangingOrders( pTeamSoldier );
				}
			}
			gTacticalStatus.Team[ CIV_TEAM ].bAwareOfOpposition = FALSE;

		}

		fInterfacePanelDirty = DIRTYLEVEL2;

		if ( gTacticalStatus.bBoxingState == NOT_BOXING ) // if boxing don't do any of this stuff
		{
			LogBattleResults( LOG_VICTORY );

			SetThisSectorAsPlayerControlled( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, TRUE );
			HandleVictoryInNPCSector( gWorldSectorX, gWorldSectorY,( INT16 ) gbWorldSectorZ );
			if ( CheckFact( FACT_FIRST_BATTLE_BEING_FOUGHT, 0 ) )
			{
				// ATE: Need to trigger record for this event .... for NPC scripting
				TriggerNPCRecord( PACOS, 18 );

				// this is our first battle... and we won!
				SetFactTrue( FACT_FIRST_BATTLE_FOUGHT );
				SetFactTrue( FACT_FIRST_BATTLE_WON );
				SetFactFalse( FACT_FIRST_BATTLE_BEING_FOUGHT );
				SetTheFirstBattleSector( (INT16) (gWorldSectorX + gWorldSectorY * MAP_WORLD_X) );
				HandleFirstBattleEndingWhileInTown( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, FALSE );
			}
		}

		//Whenever returning TRUE, make sure you clear gfBlitBattleSectorLocator;
		gfBlitBattleSectorLocator = FALSE;


		// Kaiden: More UB Reveal All Items after combat code.
		//When all the enemy gets killed, reveal the items they dropped
		//But only if the option is turned ON.
		if(gGameExternalOptions.gfRevealItems)
			RevealAllDroppedEnemyItems();

		// If we are the server, we escape this function at the top if we think the game should still be running
		// hence if we get here the game is over for all clients and we should report it
		if (is_networked && is_server)
			game_over();
		return( TRUE );
	}
	// If we are the server, we escape this function at the top if we think the game should still be running
	// hence if we get here the game is over for all clients and we should report it
	if (is_networked && is_server)
		game_over();

	return( FALSE );
}


void CycleThroughKnownEnemies( )
{
	// static to indicate last position we were at:
	SOLDIERTYPE		*pSoldier;
	static BOOLEAN fFirstTime = TRUE;
	static UINT16	usStartToLook;
	UINT32				cnt;
	BOOLEAN				fEnemyBehindStartLook = FALSE;
	BOOLEAN				fEnemiesFound = FALSE;

	if ( fFirstTime )
	{
		fFirstTime = FALSE;

		usStartToLook = gTacticalStatus.Team[ gbPlayerNum ].bLastID;
	}

	for ( cnt = gTacticalStatus.Team[ gbPlayerNum ].bLastID, pSoldier = MercPtrs[ cnt ]; cnt < TOTAL_SOLDIERS; cnt++, pSoldier++ )
	{
		// try to find first active, OK enemy
		if ( pSoldier->bActive && pSoldier->bInSector && !pSoldier->aiData.bNeutral && (pSoldier->bSide != gbPlayerNum) && (pSoldier->stats.bLife > 0) )
		{
			if ( pSoldier->bVisible != -1 )
			{
				fEnemiesFound = TRUE;

				// If we are > ok start, this is the one!
				if ( cnt > usStartToLook )
				{
					usStartToLook = (UINT16)cnt;

					// Locate to!
					//LocateSoldier( pSoldier->ubID, 1 );

					//ATE: Change to Slide To...
					SlideTo( 0, pSoldier->ubID, 0, SETANDREMOVEPREVIOUSLOCATOR );
					return;
				}
				else
				{
					fEnemyBehindStartLook = TRUE;
				}
			}
		}
	}

	if ( !fEnemiesFound )
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_ENEMIES_IN_SIGHT_STR ] );
	}

	// If here, we found nobody, but there may be someone behind
	// If to, recurse!
	if ( fEnemyBehindStartLook )
	{
		usStartToLook = gTacticalStatus.Team[ gbPlayerNum ].bLastID;

		CycleThroughKnownEnemies( );
	}
}


void CycleVisibleEnemies( SOLDIERTYPE *pSrcSoldier )
{
	// static to indicate last position we were at:
	SOLDIERTYPE		*pSoldier;
	UINT16	usStartToLook;
	UINT32				cnt;

	usStartToLook = gTacticalStatus.Team[ gbPlayerNum ].bLastID;

	for ( cnt = gTacticalStatus.Team[ gbPlayerNum ].bLastID, pSoldier = MercPtrs[ cnt ]; cnt < TOTAL_SOLDIERS; cnt++, pSoldier++ )
	{
		// try to find first active, OK enemy
		if ( pSoldier->bActive && pSoldier->bInSector && !pSoldier->aiData.bNeutral && (pSoldier->bSide != gbPlayerNum) && (pSoldier->stats.bLife > 0) )
		{
			if ( pSrcSoldier->aiData.bOppList[ pSoldier->ubID ] == SEEN_CURRENTLY	)
			{
				// If we are > ok start, this is the one!
				if ( cnt > pSrcSoldier->ubLastEnemyCycledID )
				{
					pSrcSoldier->ubLastEnemyCycledID = (UINT8)cnt;

					//ATE: Change to Slide To...
					SlideTo( 0, pSoldier->ubID, 0, SETANDREMOVEPREVIOUSLOCATOR );

					ChangeInterfaceLevel( pSoldier->pathing.bLevel );
					return;
				}
			}
		}
	}

	// If here.. reset to zero...
	pSrcSoldier->ubLastEnemyCycledID = 0;


	usStartToLook = gTacticalStatus.Team[ gbPlayerNum ].bLastID;
	for ( cnt = gTacticalStatus.Team[ gbPlayerNum ].bLastID, pSoldier = MercPtrs[ cnt ]; cnt < TOTAL_SOLDIERS; cnt++, pSoldier++ )
	{
		// try to find first active, OK enemy
		if ( pSoldier->bActive && pSoldier->bInSector && !pSoldier->aiData.bNeutral && (pSoldier->bSide != gbPlayerNum) && (pSoldier->stats.bLife > 0) )
		{
			if ( pSrcSoldier->aiData.bOppList[ pSoldier->ubID ] == SEEN_CURRENTLY	)
			{

				// If we are > ok start, this is the one!
				if ( cnt > pSrcSoldier->ubLastEnemyCycledID )
				{
					pSrcSoldier->ubLastEnemyCycledID = (UINT8)cnt;

					//ATE: Change to Slide To...
					SlideTo( 0, pSoldier->ubID, 0, SETANDREMOVEPREVIOUSLOCATOR );

					ChangeInterfaceLevel( pSoldier->pathing.bLevel );
					return;
				}
			}
		}
	}


}


UINT32 CountNonVehiclesOnPlayerTeam( void )
{
	UINT32				cnt;
	SOLDIERTYPE		*pSoldier;
	UINT32					bNumber = 0;

	for ( cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID, pSoldier = MercPtrs[ cnt ]; cnt <= (UINT32)( gTacticalStatus.Team[ gbPlayerNum ].bLastID ); cnt++, pSoldier++ )
	{
		if ( pSoldier->bActive && !(pSoldier->flags.uiStatusFlags & SOLDIER_VEHICLE ) )
		{
			bNumber++;
		}
	}

	return( bNumber );

}


BOOLEAN PlayerTeamFull( )
{
	// last ID for the player team is 19, so long as we have at most 17 non-vehicles...
	if ( CountNonVehiclesOnPlayerTeam() < gGameExternalOptions.ubGameMaximumNumberOfPlayerMercs /*gTacticalStatus.Team[ gbPlayerNum ].bLastID - 2*/ )
	{
		return( FALSE );
	}

	return( TRUE );
}

UINT8 NumPCsInSector( void )
{
	SOLDIERTYPE *pTeamSoldier;
	UINT32				cnt = 0;
	UINT8				ubNumPlayers = 0;

	// Check if the battle is won!
	// Loop through all mercs and make go
	for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
	{
		if ( MercSlots[ cnt ] )
		{
			pTeamSoldier = MercSlots[ cnt ];
			if ( pTeamSoldier->bTeam == gbPlayerNum && pTeamSoldier->stats.bLife > 0 )
			{
				ubNumPlayers++;
			}
		}
	}

	return( ubNumPlayers );

}


UINT8 NumEnemyInSector( )
{
	SOLDIERTYPE *pTeamSoldier;
	INT32				cnt = 0;
	UINT8				ubNumEnemies = 0;

	// Check if the battle is won!
	// Loop through all mercs and make go
	for ( pTeamSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pTeamSoldier++, cnt++ )
	{
		if ( pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->stats.bLife > 0 )
		{
			// Checkf for any more bacguys
			if ( !pTeamSoldier->aiData.bNeutral && (pTeamSoldier->bSide != 0 ) )
			{
				ubNumEnemies++;
			}
		}
	}

	return( ubNumEnemies );

}

UINT8 NumEnemyInSectorExceptCreatures()
{
	SOLDIERTYPE *pTeamSoldier;
	INT32				cnt = 0;
	UINT8				ubNumEnemies = 0;

	// Check if the battle is won!
	// Loop through all mercs and make go
	for ( pTeamSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pTeamSoldier++, cnt++ )
	{
		if ( pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->stats.bLife > 0 && pTeamSoldier->bTeam != CREATURE_TEAM )
		{
			// Checkf for any more bacguys
			if ( !pTeamSoldier->aiData.bNeutral && (pTeamSoldier->bSide != 0 ) )
			{
				ubNumEnemies++;
			}
		}
	}

	return( ubNumEnemies );

}


UINT8 NumEnemyInSectorNotDeadOrDying( )
{
	SOLDIERTYPE *pTeamSoldier;
	INT32				cnt = 0;
	UINT8				ubNumEnemies = 0;

	// Check if the battle is won!
	// Loop through all mercs and make go
	for ( pTeamSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pTeamSoldier++, cnt++ )
	{
		// Kill those not already dead.,...
		if ( pTeamSoldier->bActive && pTeamSoldier->bInSector )
		{
			// For sure for flag thet they are dead is not set
			if ( !( pTeamSoldier->flags.uiStatusFlags & SOLDIER_DEAD ) )
			{
				// Also, we want to pick up unconcious guys as NOT being capable,
				// but we want to make sure we don't get those ones that are in the
				// process of dying
				if ( pTeamSoldier->stats.bLife >= OKLIFE )
				{
					// Check for any more badguys
					if ( !pTeamSoldier->aiData.bNeutral && (pTeamSoldier->bSide != 0 ) )
					{
						ubNumEnemies++;
					}
				}
			}
		}
	}

	return( ubNumEnemies );

}

UINT8 NumBloodcatsInSectorNotDeadOrDying( )
{
	SOLDIERTYPE *pTeamSoldier;
	INT32				cnt = 0;
	UINT8				ubNumEnemies = 0;

	// Check if the battle is won!
	// Loop through all mercs and make go
	for ( pTeamSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pTeamSoldier++, cnt++ )
	{
		// Kill those not already dead.,...
		if ( pTeamSoldier->bActive && pTeamSoldier->bInSector )
		{
			if ( pTeamSoldier->ubBodyType == BLOODCAT )
			{
				// For sure for flag thet they are dead is not set
				if ( !( pTeamSoldier->flags.uiStatusFlags & SOLDIER_DEAD ) )
				{
					// Also, we want to pick up unconcious guys as NOT being capable,
					// but we want to make sure we don't get those ones that are in the
					// process of dying
					if ( pTeamSoldier->stats.bLife >= OKLIFE )
					{
						// Check for any more badguys
						if ( !pTeamSoldier->aiData.bNeutral && (pTeamSoldier->bSide != 0 ) )
						{
							ubNumEnemies++;
						}
					}
				}
			}
		}
	}

	return( ubNumEnemies );

}


UINT8 NumCapableEnemyInSector( )
{
	SOLDIERTYPE *pTeamSoldier;
	INT32				cnt = 0;
	UINT8				ubNumEnemies = 0;

	// Check if the battle is won!
	// Loop through all mercs and make go
	for ( pTeamSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pTeamSoldier++, cnt++ )
	{
		// Kill those not already dead.,...
		if ( pTeamSoldier->bActive && pTeamSoldier->bInSector )
		{
			// For sure for flag thet they are dead is not set
			if ( !( pTeamSoldier->flags.uiStatusFlags & SOLDIER_DEAD ) )
			{
				// Also, we want to pick up unconcious guys as NOT being capable,
				// but we want to make sure we don't get those ones that are in the
				// process of dying
				if ( pTeamSoldier->stats.bLife < OKLIFE && pTeamSoldier->stats.bLife != 0 )
				{

				}
				else
				{
					// Check for any more badguys
					if ( !pTeamSoldier->aiData.bNeutral && (pTeamSoldier->bSide != 0 ) )
					{
						ubNumEnemies++;
					}
				}
			}
		}
	}

	return( ubNumEnemies );

}


BOOLEAN CheckForLosingEndOfBattle( )
{
	SOLDIERTYPE *pTeamSoldier;
	INT32				cnt = 0;
	INT8				bNumDead = 0, bNumNotOK = 0, bNumInBattle = 0, bNumNotOKRealMercs = 0;
	BOOLEAN			fMadeCorpse;
	BOOLEAN			fDoCapture = FALSE;
	BOOLEAN	 fOnlyEPCsLeft = TRUE;
	BOOLEAN	 fMilitiaInSector = FALSE;


	// ATE: Check for MILITIA - we won't lose if we have some.....
	cnt = gTacticalStatus.Team[ MILITIA_TEAM ].bFirstID;
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ MILITIA_TEAM ].bLastID; cnt++,pTeamSoldier++)
	{
		if ( pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->bSide == gbPlayerNum )
		{
			if ( pTeamSoldier->stats.bLife >= OKLIFE )
			{
				// We have at least one poor guy who will still fight....
				// we have not lost ( yet )!
				fMilitiaInSector = TRUE;
			}
		}
	}


	// IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	// look for all mercs on the same team,
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pTeamSoldier++)
	{
		// Are we active and in sector.....
		if ( pTeamSoldier->bActive && pTeamSoldier->bInSector && !( pTeamSoldier->flags.uiStatusFlags & SOLDIER_VEHICLE ) )
		{
			bNumInBattle++;

			if ( pTeamSoldier->stats.bLife == 0 )
			{
				bNumDead++;
			}
			else if ( pTeamSoldier->stats.bLife < OKLIFE )
			{
				bNumNotOK++;

				if ( !AM_AN_EPC( pTeamSoldier ) && !AM_A_ROBOT( pTeamSoldier ) )
				{
					bNumNotOKRealMercs++;
				}
			}
			else
			{
				if ( !AM_A_ROBOT( pTeamSoldier ) || !AM_AN_EPC( pTeamSoldier ) )
				{
					fOnlyEPCsLeft = FALSE;
				}
			}
		}
	}


	// OK< check ALL in battle, if that matches SUM of dead, incompacitated, we're done!
	if ( ( bNumDead + bNumNotOK ) == bNumInBattle || fOnlyEPCsLeft )
	{
		// Are there militia in sector?
		if ( fMilitiaInSector )
		{
			if( guiCurrentScreen != AUTORESOLVE_SCREEN )
			{
				// if here, check if we should autoresolve.
				// if we have at least one guy unconscious, call below function...
				if ( HandlePotentialBringUpAutoresolveToFinishBattle(  gWorldSectorX, gWorldSectorY, gbWorldSectorZ  ) )
				{
					// return false here as we are autoresolving...
					return( FALSE );
				}
			}
			else
			{
				return( FALSE );
			}
		}


		// Bring up box if we have ANY guy incompaciteded.....
		if ( bNumDead != bNumInBattle )
		{
			// If we get captured...
			// Your unconscious mercs are captured!

			// Check if we should get captured....
			if ( bNumNotOKRealMercs < 4 && bNumNotOKRealMercs > 1 )
			{
				// Check if any enemies exist....
				if ( gTacticalStatus.Team[ ENEMY_TEAM ].bMenInSector > 0 )
				{
					//if( GetWorldDay() > STARTDAY_ALLOW_PLAYER_CAPTURE_FOR_RESCUE && !( gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE ))
					{
						if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTNOTSTARTED || ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTDONE && gubQuest[ QUEST_INTERROGATION ] == QUESTNOTSTARTED ) )
						{
							fDoCapture = TRUE;
							// CJC Dec 1 2002: fix capture sequences
							BeginCaptureSquence();
						}
					}
				}
			}

			gfKillingGuysForLosingBattle = TRUE;

			// KIll them now...
			// IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
			cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

			for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pTeamSoldier++)
			{
				// Are we active and in sector.....
				if ( pTeamSoldier->bActive && pTeamSoldier->bInSector )
				{
					if ( pTeamSoldier->stats.bLife != 0 && pTeamSoldier->stats.bLife < OKLIFE || AM_AN_EPC( pTeamSoldier ) || AM_A_ROBOT( pTeamSoldier ) )
					{
						// Captured EPCs or ROBOTS will be kiiled in capture routine....
						if ( !fDoCapture )
						{
							// Kill!
							pTeamSoldier->stats.bLife = 0;

							HandleSoldierDeath( pTeamSoldier, &fMadeCorpse );

							//HandlePlayerTeamMemberDeath( pTeamSoldier );
							// Make corpse..
							//TurnSoldierIntoCorpse( pTeamSoldier, TRUE, TRUE );
						}
					}

					// ATE: if we are told to do capture....
					if ( pTeamSoldier->stats.bLife != 0 && fDoCapture )
					{
						EnemyCapturesPlayerSoldier( pTeamSoldier );

						RemoveSoldierFromTacticalSector( pTeamSoldier, TRUE );
					}

				}
			}

			gfKillingGuysForLosingBattle = FALSE;

			// WANNE - MP: Only do special scenes (capture, ...) in a single player game
			if (!is_networked)
			{
				if ( fDoCapture )
				{
					EndCaptureSequence( );
					SetCustomizableTimerCallbackAndDelay( 3000, CaptureTimerCallback, FALSE );
				}
				else
				{
					SetCustomizableTimerCallbackAndDelay( 10000, DeathTimerCallback, FALSE );
				}
			}
		}
		return( TRUE );
	}

	return( FALSE );
}


BOOLEAN KillIncompacitatedEnemyInSector( )
{
	SOLDIERTYPE *pTeamSoldier;
	INT32				cnt = 0;
	BOOLEAN			fReturnVal = FALSE;

	// Check if the battle is won!
	// Loop through all mercs and make go
	for ( pTeamSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pTeamSoldier++, cnt++ )
	{
		if ( pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->stats.bLife < OKLIFE && !( pTeamSoldier->flags.uiStatusFlags & SOLDIER_DEAD ) )
		{
			// Checkf for any more bacguys
			if ( !pTeamSoldier->aiData.bNeutral && (pTeamSoldier->bSide != gbPlayerNum ) )
			{
				// KIll......
				// SANDRO - if the soldier is bleeding out, consider this damage as done by the last attacker
				if ( pTeamSoldier->ubAttackerID != NOBODY )
					pTeamSoldier->SoldierTakeDamage( ANIM_CROUCH, pTeamSoldier->stats.bLife, 100, TAKE_DAMAGE_BLOODLOSS, pTeamSoldier->ubAttackerID, NOWHERE, 0, TRUE );
				else if ( pTeamSoldier->ubPreviousAttackerID != NOBODY )
					pTeamSoldier->SoldierTakeDamage( ANIM_CROUCH, pTeamSoldier->stats.bLife, 100, TAKE_DAMAGE_BLOODLOSS, pTeamSoldier->ubPreviousAttackerID, NOWHERE, 0, TRUE );
				else if ( pTeamSoldier->ubNextToPreviousAttackerID != NOBODY )
					pTeamSoldier->SoldierTakeDamage( ANIM_CROUCH, pTeamSoldier->stats.bLife, 100, TAKE_DAMAGE_BLOODLOSS, pTeamSoldier->ubNextToPreviousAttackerID, NOWHERE, 0, TRUE );
				else 
					pTeamSoldier->SoldierTakeDamage( ANIM_CROUCH, pTeamSoldier->stats.bLife, 100, TAKE_DAMAGE_BLOODLOSS, NOBODY, NOWHERE, 0, TRUE );

				fReturnVal = TRUE;
			}
		}
	}
	return( fReturnVal );
}



BOOLEAN AttackOnGroupWitnessed( SOLDIERTYPE * pSoldier, SOLDIERTYPE * pTarget )
{
	UINT32					uiSlot;
	SOLDIERTYPE *		pGroupMember;

	// look for all group members... rebels could be on the civ team or ours!
	for ( uiSlot = 0; uiSlot < guiNumMercSlots; uiSlot++ )
	{
		pGroupMember = MercSlots[ uiSlot ];
		if (pGroupMember && (pGroupMember->ubCivilianGroup == pTarget->ubCivilianGroup) && pGroupMember != pTarget)
		{
			if (pGroupMember->aiData.bOppList[pSoldier->ubID] == SEEN_CURRENTLY || pGroupMember->aiData.bOppList[pTarget->ubID] == SEEN_CURRENTLY)
			{
				return( TRUE );
			}
			if ( SpacesAway( pGroupMember->sGridNo, pSoldier->sGridNo ) < 12 || SpacesAway( pGroupMember->sGridNo, pTarget->sGridNo ) < 12 )
			{
				return( TRUE );
			}
		}
	}

	return( FALSE );
}

INT8 CalcSuppressionTolerance( SOLDIERTYPE * pSoldier )
{
	INT8		bTolerance;

	// Calculate basic tolerance value
	bTolerance = pSoldier->stats.bExpLevel * 2;
	if (pSoldier->flags.uiStatusFlags & SOLDIER_PC)
	{
		// give +1 for every 10% morale from 50, for a maximum bonus/penalty of 5.
		bTolerance += ( pSoldier->aiData.bMorale - 50 ) / 10;
	}
	else
	{
		// give +2 for every morale category from normal, for a max change of 4
		bTolerance += ( pSoldier->aiData.bAIMorale - MORALE_NORMAL ) * 2;
	}

	if ( pSoldier->ubProfile != NO_PROFILE )
	{
		// SANDRO - check for character traits
		if ( gGameOptions.fNewTraitSystem )
		{
			if ( gMercProfiles[ pSoldier->ubProfile ].bCharacterTrait == CHAR_TRAIT_INTELLECTUAL )
				bTolerance += -2;
			else if  ( gMercProfiles[ pSoldier->ubProfile ].bCharacterTrait == CHAR_TRAIT_DAUNTLESS )
				bTolerance += 2;
		}
		else
		{
			// change tolerance based on attitude
			switch ( gMercProfiles[ pSoldier->ubProfile ].bAttitude )
			{
			case ATT_AGGRESSIVE:
				bTolerance += 2;
				break;
			case ATT_COWARD:
				bTolerance += -2;
				break;
			default:
				break;
			}
		}
	}
	else
	{
		// generic NPC/civvie; change tolerance based on attitude
		switch ( pSoldier->aiData.bAttitude )
		{
		case BRAVESOLO:
		case BRAVEAID:
			bTolerance +=	2;
			break;
		case AGGRESSIVE:
			bTolerance +=	1;
			break;
		case DEFENSIVE:
			bTolerance += -1;
			break;
		default:
			break;
		}
	}

	// HEADROCK HAM 3.2: This is actually a feature from HAM 2.9. It adds bonuses/penalties for nearby friends.
	if (gGameExternalOptions.fFriendliesAffectTolerance)
	{
		bTolerance += CheckStatusNearbyFriendlies( pSoldier );
	}

	// HEADROCK HAM 3.3: Moving rapidly makes one less prone to suppression.
	if (gGameExternalOptions.ubTilesMovedPerBonusTolerancePoint > 0)
	{
		bTolerance += pSoldier->bTilesMoved / gGameExternalOptions.ubTilesMovedPerBonusTolerancePoint;
	}
	// HEADROCK HAM 3.6: This value has moved here. It reduces tolerance if the character is massively shocked.
	if (gGameExternalOptions.ubCowerEffectOnSuppression != 0)
	{
		if (pSoldier->aiData.bShock > bTolerance)
		{
			bTolerance -= gGameExternalOptions.ubCowerEffectOnSuppression;
		}
	}
	
	// SANDRO - STOMP traits - squadleader's bonus to suppression tolerance
	if ( gGameOptions.fNewTraitSystem && IS_MERC_BODY_TYPE(pSoldier) && 
		(pSoldier->bTeam == ENEMY_TEAM || pSoldier->bTeam == MILITIA_TEAM || pSoldier->bTeam == gbPlayerNum) )
	{
		UINT8 ubNumberOfSL = GetSquadleadersCountInVicinity( pSoldier, FALSE, FALSE );
		// Also take ourselves into account
		if ((ubNumberOfSL < gSkillTraitValues.ubSLMaxBonuses) && HAS_SKILL_TRAIT( pSoldier, SQUADLEADER_NT ))
		{
			ubNumberOfSL = max( gSkillTraitValues.ubSLMaxBonuses, (ubNumberOfSL + NUM_SKILL_TRAITS( pSoldier, SQUADLEADER_NT )) );
		}
		bTolerance += (bTolerance * gSkillTraitValues.ubSLOverallSuppresionBonusPercent * ubNumberOfSL / 100);
	}
	
	bTolerance = __max(bTolerance, gGameExternalOptions.ubSuppressionToleranceMin);
	bTolerance = __min(bTolerance, gGameExternalOptions.ubSuppressionToleranceMax);

	return( bTolerance );
}

void HandleSuppressionFire( UINT8 ubTargetedMerc, UINT8 ubCausedAttacker )
{
	///////////////////////////////////////////////////////////////////////////////
	// 
	// HEADROCK HAM B2: This entire function has been completely revamped.
	// HEADROCK HAM 3.5: Revamped again.
	//
	///////////////////////////////////////////////////////////////////////////////

	// This function runs after very attack is completed. It calculates the number
	// of "Suppression Points" any character has received during the attack, and
	// inflicts various penalties accordingly.
	// The most important result of this function is AP loss.

	//INT8 SUPPRESSION_AP_LIMIT = gGameExternalOptions.iMinAPLimitFromSuppression;

	INT8									bTolerance;
	INT32									sClosestOpponent, sClosestOppLoc;
	UINT8									ubPointsLost, ubNewStance;
	UINT32								uiLoop;
	UINT8 ubLoop2;
	// Flag to determine if the target is cowering (if allowed)
	BOOLEAN								fCower;
	SOLDIERTYPE *					pSoldier;
	// External options
	
	// JA2_OPTIONS.INI
	INT8 MAXIMUM_SUPPRESSION_SHOCK = gGameExternalOptions.ubMaxSuppressionShock;
	
	// APBPConstants.INI
	UINT16 usLimitSuppressionAPsLostPerAttack = APBPConstants[AP_MAX_SUPPRESSED];
	UINT16 usLimitSuppressionAPsLostPerTurn = APBPConstants[AP_MAX_TURN_SUPPRESSED];
	//HEADROCK HAM 3.5: Ratio between AP Loss and Suppression Shock
	UINT16 uiShockPerAPLossDivisor = APBPConstants[AP_SUPPRESSION_SHOCK_DIVISOR];

	// Loop through every character.
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("HandleSuppressionFire: loop = %d, numslots = %d ",uiLoop, guiNumMercSlots));
		pSoldier = MercSlots[uiLoop];

		// Has this character received any Suppression Points since the last attack?
		// HEADROCK: Suppression Points accumulate by bullets flying near the character. It includes
		// friendly fire at a certain distance. As of HAM 3.2, it also happens with nearby explosions.
		// The number of points accumulated resets to 0 at the end of this function.
		
		if (pSoldier && IS_MERC_BODY_TYPE( pSoldier) && pSoldier->stats.bLife >= OKLIFE && pSoldier->ubSuppressionPoints > 0)
		{
			DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("HandleSuppressionFire: soldier id = %d, life = %d, suppression points = %d",pSoldier->ubID,pSoldier->stats.bLife, pSoldier->ubSuppressionPoints));
			DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("HandleSuppressionFire: calc suppression tolerance"));

			// Calculate the character's tolerance to suppression. Helps reduce the severity of the penalties inflicted
			// during this function.
			bTolerance = CalcSuppressionTolerance( pSoldier );

			DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("HandleSuppressionFire: figure out aps lost"));

			// This formula gives a linear increase in AP loss relative to Suppression Points. The most Suppression
			// Points we have, the most APs we're going to lose. Tolerance mitigates this by making the graph angle
			// more shallow. 
			// The relation between AP Loss and Suppression Points is LINEAR.
			ubPointsLost = ( ( (pSoldier->ubSuppressionPoints * APBPConstants[AP_SUPPRESSION_MOD]) / (bTolerance + 6) ) * 2 + 1 ) / 2;
			
			// INI-Controlled intensity. SuppressionEffectiveness acts as a percentage applied to the number of lost APs. 
			// To turn off the entire Suppression system, simply set the INI value to 0. (0% AP Loss)
			// The default is obviously 100%. You can increase or decrease it, at will.
			// PLEASE NOTE that AP loss governs ALL OTHER SUPPRESSION EFFECTS.
			ubPointsLost = ( ubPointsLost * gGameExternalOptions.sSuppressionEffectiveness ) / 100;

			// This is an upper cap for the number of APs we can lose per attack.
			if (usLimitSuppressionAPsLostPerAttack > 0)
			{
				if (ubPointsLost > usLimitSuppressionAPsLostPerAttack)
				{
					ubPointsLost = __max(255,(UINT8)usLimitSuppressionAPsLostPerAttack);
				}
			}

			// This makes sure that we never lose more APs than we're allowed per turn.
			if (usLimitSuppressionAPsLostPerTurn > 0)
			{
				if (pSoldier->ubAPsLostToSuppression + ubPointsLost > APBPConstants[AP_MAX_TURN_SUPPRESSED])
				{
					ubPointsLost = usLimitSuppressionAPsLostPerTurn - pSoldier->ubAPsLostToSuppression;
				}
			}

			// This nifty little bit gives suppression an "extra kick". Soldiers affected by suppression (I.E. lost APs) 
			// will also suffer from SHOCK. As shock accumulates, the soldier becomes less accurate and may find it 
			// difficult to perform certain manual tasks. Additionally, he also becomes harder to hit, because the fear 
			// causes him to hide as best as he can from incoming fire.
			// Shock is sliced in half at the start of every turn. Also note that shock may cause "cowering" (see below).
			if (gGameExternalOptions.usSuppressionShockEffect > 0)
			{
				// Can't get shock if we haven't lost APs.
				if (ubPointsLost > 0)
				{
					INT8 bShockValue, bShockLimit;
					// Limit defined by INI.
					bShockLimit = MAXIMUM_SUPPRESSION_SHOCK;
					// The amount of shock received depends on how many APs we've lost - Every AP lost will cause one 
					// point of shock. This is then divided by 4 if using the 100AP system.
					if (uiShockPerAPLossDivisor == 0) // SANDRO - check if we are not going to divide by zero
						uiShockPerAPLossDivisor = 4; // set to default value in this case
					bShockValue = ubPointsLost / uiShockPerAPLossDivisor;

					bShockValue = __max(0,bShockValue);
					bShockLimit = __max(0,bShockLimit);

					// use external value to determine how effective SHOCK really is.
					bShockValue = (bShockValue * gGameExternalOptions.usSuppressionShockEffect) / 100;

					// Make sure total shock doesn't go TOO high. Maximum is around 30, including previous shock 
					// from suppression and/or wounds. It is possible to breach the maximum after a good suppressive
					// attack.

					if ( pSoldier->aiData.bShock + bShockValue <= bShockLimit )
					{
						// Shock limit not yet breached. Add shock to character.
						pSoldier->aiData.bShock = __min(127, pSoldier->aiData.bShock + bShockValue);
					}
					else if ( pSoldier->aiData.bShock < bShockLimit ) // Shock limit will be breached.
					{
						// Original shock was lower than the limit, so add extra shock and breach the limit.
						pSoldier->aiData.bShock = __min(127, pSoldier->aiData.bShock + bShockValue);
					}
					// Else, original shock was already over the limit. No more shock is added.
				}
			}
			// HEADROCK: Cowering is the panic that grips a character due to suffering too much suppression shock. If
			// enough shock has been accumulated, the soldier goes into this panic. Generally, cowering will cause
			// the character to drop a stance if he can, overriding other conditions for a stance-change (see below).
			// Cowering characters may become considerably easier to suppress with additional firepower. In other
			// words, if you're cowering, you've effectively turned from a bad-ass to a wimp.

			fCower = false;
			if ( gGameExternalOptions.usSuppressionShockEffect > 0 )
			{
				// SANDRO - STOMP traits
				INT8 bShockForCower = pSoldier->aiData.bShock;
				if ( gGameOptions.fNewTraitSystem )
				{
					// Squadleader's resistance to cowering
					if ( HAS_SKILL_TRAIT( pSoldier, SQUADLEADER_NT ))
					{
						bShockForCower = (INT8)((bShockForCower * (100 - gSkillTraitValues.ubSLFearResistance * NUM_SKILL_TRAITS( pSoldier, SQUADLEADER_NT )) /100) + 0.5);
					}
					// Check for character traits
					if ( gMercProfiles[pSoldier->ubProfile].bCharacterTrait == CHAR_TRAIT_INTELLECTUAL )
					{
						bShockForCower = (INT8)((bShockForCower * 23 / 20 ) + 0.5); // +15% as shock
					}
					else if ( gMercProfiles[pSoldier->ubProfile].bCharacterTrait == CHAR_TRAIT_DAUNTLESS )
					{
						bShockForCower = (INT8)((bShockForCower * 17 / 20 ) + 0.5); // -15% as shock				
					}
				}
				if (bShockForCower >= bTolerance)
				{ 
					fCower = true; 

					// If soldier is visible on-screen, report to player that they are cowering.
					if ( pSoldier->bVisible != -1 )
						ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, New113HAMMessage[0], pSoldier->name );				
				}
			}

			// Suppression reduces morale. For every X APs lost, morale goes down by a point. X is defined by INI.
			DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("HandleSuppressionFire: check for morale effects"));
			if (APBPConstants[AP_LOST_PER_MORALE_DROP] > 0 && ubPointsLost > 0)
			{
				for ( ubLoop2 = 0; ubLoop2 < (ubPointsLost / APBPConstants[AP_LOST_PER_MORALE_DROP]); ubLoop2++ )
				{
					HandleMoraleEvent( pSoldier, MORALE_SUPPRESSED, pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ );
				}
			}
	

			ubNewStance = 0;

			DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("HandleSuppressionFire: check for reaction"));

			// HEADROCK HAM 3.5: Characters who have enough APs to drop WILL DROP. This is strictly for survival reasons,
			// because upright characters can easily get themselves killed. Soldiers drop stance for "free", using up
			// the APs that they've just lost to do so.

			switch (gAnimControl[ pSoldier->usAnimState ].ubEndHeight)
			{
				case ANIM_PRONE:
					// can't change stance below prone!
					break;
				case ANIM_CROUCH:
					if (ubPointsLost >= GetAPsProne(pSoldier, TRUE) && IsValidStance( pSoldier, ANIM_PRONE ) && gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_PRONE ) // SANDRO - changed to GetAPsProne
					{
						sClosestOpponent = ClosestKnownOpponent( pSoldier, &sClosestOppLoc, NULL );
						// HEADROCK: Added cowering.						
						if (TileIsOutOfBounds(sClosestOpponent) || SpacesAway( pSoldier->sGridNo, sClosestOppLoc ) > 8 || fCower)
						{
							ubPointsLost -= GetAPsProne(pSoldier, TRUE);
							ubNewStance = ANIM_PRONE;
						}
					}
					break;
				default: // standing!
					if ( pSoldier->MercInWater())
					{
						// can't change stance here!
						break;
					}
					else if (ubPointsLost >= (GetAPsCrouch(pSoldier, TRUE) + GetAPsProne(pSoldier, TRUE)) && ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_PRONE ) && IsValidStance( pSoldier, ANIM_PRONE ) ) // SANDRO - changed to GetAPsCrouch/Prone
					{
						sClosestOpponent = ClosestKnownOpponent( pSoldier, &sClosestOppLoc, NULL );
						// HEADROCK: Added cowering.
						if (TileIsOutOfBounds(sClosestOpponent) || SpacesAway( pSoldier->sGridNo, sClosestOppLoc ) > 8 ||fCower )
						{
							if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_STAND )
							{
								// can only crouch for now
								ubNewStance = ANIM_CROUCH;
							}
							else
							{
								// lie prone!
								ubNewStance = ANIM_PRONE;
							}
						}
						else if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_STAND && IsValidStance( pSoldier, ANIM_CROUCH ) )
						{
							// crouch, at least!
							ubNewStance = ANIM_CROUCH;
						}
					}
					else if ( ubPointsLost >= GetAPsCrouch(pSoldier, TRUE) && ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_CROUCH ) && IsValidStance( pSoldier, ANIM_CROUCH ) ) // SANDRO - changed to GetAPsCrouch
					{
						// crouch!
						ubNewStance = ANIM_CROUCH;
					}
					break;
				}


			DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("HandleSuppressionFire: reduce action points"));

			// HEADROCK HAM 3.1: If this setting is enabled, it will show an on-screen message that tells us the
			// character has lost his entire next turn. This only fires once per turn, the moment a character drops
			// to the minimum AP limit.

			if (gGameExternalOptions.fShowSuppressionShutdown)
			{
				// If we're about the hit the lower limit
				if (pSoldier->bActionPoints > APBPConstants[AP_MIN_LIMIT] && pSoldier->bActionPoints - ubPointsLost <= APBPConstants[AP_MIN_LIMIT])
				{
					// And soldier is visible
					if ( pSoldier->bVisible != -1 )
					{
						// "Soldier is pinned down!"
						ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, New113HAMMessage[1], pSoldier->name );
						// HEADROCK HAM 3.2: Added a radio locator!
						ShowRadioLocator( (UINT8)pSoldier->ubID, SHOW_LOCATOR_NORMAL );
					}
				}
			}

			// Reduce action points!
			// HEADROCK HAM Beta 2.2: Enforce a minimum limit via INI.
			if (pSoldier->bActionPoints - ubPointsLost <= APBPConstants[AP_MIN_LIMIT] )
			{
				pSoldier->bActionPoints = APBPConstants[AP_MIN_LIMIT];
			}
			else
			{
				pSoldier->bActionPoints -= ubPointsLost;
			}
			// Remember how many APs were lost. This prevents us from losing more and more APs without receiving
			// extra suppression. Note that a specific HAM setting will reset this value after every attack,
			// but also resets ubSuppressionPoints.
			pSoldier->ubAPsLostToSuppression = __min(255, pSoldier->ubAPsLostToSuppression + ubPointsLost);

			DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("HandleSuppressionFire: check for quote"));
			if ( (pSoldier->flags.uiStatusFlags & SOLDIER_PC) && (pSoldier->ubSuppressionPoints > 8) && (pSoldier->ubID == ubTargetedMerc) )
			{
				if ( !(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_BEING_PUMMELED ) )
				{
					DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("HandleSuppressionFire: say quote"));
					pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_BEING_PUMMELED;
					// say we're under heavy fire!

					// ATE: For some reason, we forgot #53!
					if ( pSoldier->ubProfile != 53 )
					{
						TacticalCharacterDialogue( pSoldier, QUOTE_UNDER_HEAVY_FIRE );
					}
				}
			}

			if ( ubNewStance != 0 )
			{
				DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("HandleSuppressionFire: setup change stance"));
				// This person is going to change stance

				// This person will be busy while they crouch or go prone
				if ((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT))
				{
					DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("!!!!!!! Starting suppression, on %d", pSoldier->ubID ) );
					DebugAttackBusy( String("!!!!!!! Starting suppression, on %d\n", pSoldier->ubID ) );
					//gTacticalStatus.ubAttackBusyCount++;

					// make sure supressor ID is the same!
					pSoldier->ubSuppressorID = ubCausedAttacker;
				}

				// AI people will have to have their actions cancelled
				if (!(pSoldier->flags.uiStatusFlags & SOLDIER_PC))
				{
					CancelAIAction( pSoldier, TRUE );
#if 0
					pSoldier->aiData.bAction = AI_ACTION_CHANGE_STANCE;
					pSoldier->aiData.usActionData = ubNewStance;
					pSoldier->aiData.bActionInProgress = TRUE;
#endif
				}

				// go for it!
				// ATE: Cancel any PENDING ANIMATIONS...
				pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
				// ATE: Turn off non-interrupt flag ( this NEEDS to be done! )
				pSoldier->flags.fInNonintAnim = FALSE;
				pSoldier->flags.fRTInNonintAnim = FALSE;
				gTacticalStatus.ubAttackBusyCount++;
				DebugAttackBusy( String( "Attack busy %d due to suppression fire on %d\n", gTacticalStatus.ubAttackBusyCount, pSoldier->ubID ));

				DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("HandleSuppressionFire: change stance"));
				pSoldier->ChangeSoldierStance( ubNewStance );

				pSoldier->flags.fChangingStanceDueToSuppression = TRUE;
				pSoldier->flags.fDontChargeAPsForStanceChange = TRUE;
			}

			// HEADROCK HAM 3.5: After sufficient testing, suppression clearing now works immediately at the end of
			// the attack. ubAPsLostToSuppression is only cleared at the end of the turn, but no longer plays a role
			// in affecting the number of APs lost, so it is largely irrelevant now.

			pSoldier->ubSuppressionPoints = 0;

		} // end of examining one soldier
	} // end of loop

	DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("HandleSuppressionFire done"));
}

BOOLEAN ProcessImplicationsOfPCAttack( SOLDIERTYPE * pSoldier, SOLDIERTYPE ** ppTarget, INT8 bReason )
{
	INT16		sTargetXPos, sTargetYPos;
	BOOLEAN	fEnterCombat = TRUE;
	SOLDIERTYPE * pTarget = *ppTarget;

	if ( pTarget->aiData.fAIFlags & AI_ASLEEP )
	{
		// waaaaaaaaaaaaake up!
		pTarget->aiData.fAIFlags &= (~AI_ASLEEP);
	}

	if (pTarget->ubProfile == PABLO && CheckFact( FACT_PLAYER_FOUND_ITEMS_MISSING, 0 ) )
	{
		SetFactTrue( FACT_PABLO_PUNISHED_BY_PLAYER );
	}

	if ( gTacticalStatus.bBoxingState == BOXING )
	{
		// should have a check for "in boxing ring", no?
		if ( ( pSoldier->usAttackingWeapon != NOTHING && !Item[pSoldier->usAttackingWeapon].brassknuckles ) || !( pSoldier->flags.uiStatusFlags & SOLDIER_BOXER ) )
		{
			// someone's cheating!
			if ( (Item[ pSoldier->usAttackingWeapon ].usItemClass == IC_BLADE || Item[ pSoldier->usAttackingWeapon ].usItemClass == IC_PUNCH) && (pTarget->flags.uiStatusFlags & SOLDIER_BOXER) )
			{
				// knife or brass knuckles disqualify the player!
				BoxingPlayerDisqualified( pSoldier, BAD_ATTACK );
			}
			else
			{
				// anything else is open war!
				//gTacticalStatus.bBoxingState = NOT_BOXING;
				SetBoxingState( NOT_BOXING );
				// if we are attacking a boxer we should set them to neutral (temporarily) so that the rest of the civgroup code works...
				if ( (pTarget->bTeam == CIV_TEAM) && (pTarget->flags.uiStatusFlags & SOLDIER_BOXER) )
				{
					SetSoldierNeutral( pTarget );
				}
			}

		}
	}

	// silversurfer: if militia was attacked and not already hostile make them hostile towards the player
	// but only IF the player was the attacker!
	if ( (pTarget->bTeam == MILITIA_TEAM) && (pSoldier->bTeam == OUR_TEAM) )
	{
		// HEADROCK HAM 3.6: INI setting controls their response
		{
			if (pTarget->flags.fIntendedTarget // Must be intentional
				&& gGameExternalOptions.ubCanMilitiaBecomeHostile == 2 // INI setting
				&& pTarget->bSide == gbPlayerNum // Must not be hostile by now
				)
			{
				// rebel militia attacked by the player!
				MilitiaChangesSides();
			}
		}
	}
	// JA2 Gold: fix Slay
	else if ( (pTarget->bTeam == CIV_TEAM && pTarget->aiData.bNeutral) && pTarget->ubProfile == SLAY && pTarget->stats.bLife >= OKLIFE && CheckFact( 155, 0 ) == FALSE )
	{
		TriggerNPCRecord( SLAY, 1 );
	}
	else if ( (pTarget->bTeam == CIV_TEAM) && (pTarget->ubCivilianGroup == 0) && (pTarget->aiData.bNeutral) && !( pTarget->flags.uiStatusFlags & SOLDIER_VEHICLE ) )
	{
		if ( pTarget->ubBodyType == COW && gWorldSectorX == 10 && gWorldSectorY == MAP_ROW_F )
		{
			// hicks could get mad!!!
			HickCowAttacked( pSoldier, pTarget );
		}
		else if (pTarget->ubProfile == PABLO && pTarget->stats.bLife >= OKLIFE && CheckFact( FACT_PABLO_PUNISHED_BY_PLAYER, 0 ) && !CheckFact( 38, 0 ) )
		{
			TriggerNPCRecord( PABLO, 3 );
			// Nailed Pablo.	So reset the fact that he stole from the last shipment since he's supposed to be returning
			// the goods now.
			SetFactFalse( FACT_PABLOS_STOLE_FROM_LATEST_SHIPMENT);
			SetFactFalse( FACT_PLAYER_FOUND_ITEMS_MISSING);
		}
		else
		{
			// regular civ attacked, turn non-neutral
			AddToShouldBecomeHostileOrSayQuoteList( pTarget->ubID );

			if ( pTarget->ubProfile == NO_PROFILE || !(gMercProfiles[ pTarget->ubProfile ].ubMiscFlags3 & PROFILE_MISC_FLAG3_TOWN_DOESNT_CARE_ABOUT_DEATH) )
			{
				//Kaiden: Added to keep Militia from going Hostile if you attack a Crow or Cow with an explosive.
				if ( (pTarget->ubBodyType != CROW) && (pTarget->ubBodyType != COW) )
					// militia, if any, turn hostile
					MilitiaChangesSides();
			}
		}
	}
	else if (pTarget->bTeam == CREATURE_TEAM && pTarget->ubBodyType == BLOODCAT && pTarget->aiData.bNeutral)
	{
		// Attacked a bloodcat.
		MakeBloodcatsHostile();
		// Are bloodcats in this sector affiliated with a faction?
		if ( gBloodcatPlacements[SECTOR(pTarget->sSectorX,pTarget->sSectorY)][0].PlacementType == BLOODCAT_PLACEMENT_STATIC &&
			gBloodcatPlacements[SECTOR(pTarget->sSectorX,pTarget->sSectorY)][ gGameOptions.ubDifficultyLevel-1 ].ubFactionAffiliation > NON_CIV_GROUP )
		{
			// Temporarily change bloodcat's civilian group
			UINT8 ubFaction = pTarget->ubCivilianGroup;
			pTarget->ubCivilianGroup = gBloodcatPlacements[SECTOR(pTarget->sSectorX,pTarget->sSectorY)][ gGameOptions.ubDifficultyLevel-1 ].ubFactionAffiliation;
			// Make entire faction hostile
			CivilianGroupMembersChangeSidesWithinProximity( pTarget );
			// Change back
			pTarget->ubCivilianGroup = ubFaction;
		}
	}
	else
	{
		if (pTarget->ubProfile == CARMEN)// Carmen
		{
			// Special stuff for Carmen the bounty hunter
			if (pSoldier->ubProfile != SLAY) // attacked by someone other than Slay
			{
				// change attitude
				pTarget->aiData.bAttitude = AGGRESSIVE;
			}
		}

		if ( pTarget->ubCivilianGroup && ( (pTarget->bTeam == gbPlayerNum) || pTarget->aiData.bNeutral ) )
		{
#ifdef JA2TESTVERSION
			if (pTarget->flags.uiStatusFlags & SOLDIER_PC)
			{
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"%s is changing teams", pTarget->name );
			}
#endif
			// member of a civ group, either recruited or neutral, so should
			// change sides individually or all together

			CivilianGroupMemberChangesSides( pTarget );

			if (pTarget->ubProfile != NO_PROFILE && pTarget->stats.bLife >= OKLIFE && pTarget->bVisible == TRUE)
			{
				// trigger quote!
				PauseAITemporarily();
				AddToShouldBecomeHostileOrSayQuoteList( pTarget->ubID );
				//TriggerNPCWithIHateYouQuote( pTarget->ubProfile );
			}
		}
		else if ( pTarget->ubCivilianGroup != NON_CIV_GROUP && !( pTarget->flags.uiStatusFlags & SOLDIER_BOXER ) )
		{
			// Firing at a civ in a civ group who isn't hostile... if anyone in that civ group can see this
			// going on they should become hostile.
			CivilianGroupMembersChangeSidesWithinProximity( pTarget );

			// HEADROCK HAM 3.6: If there are bloodcats affiliated with his group...
			if ( gBloodcatPlacements[SECTOR(pTarget->sSectorX,pTarget->sSectorY)][0].PlacementType == BLOODCAT_PLACEMENT_STATIC &&
				gBloodcatPlacements[SECTOR(pTarget->sSectorX,pTarget->sSectorY)][ gGameOptions.ubDifficultyLevel-1 ].ubFactionAffiliation == pTarget->ubCivilianGroup )
			{
				// Make them hostile.
				MakeBloodcatsHostile();
			}
		}
		else if ( pTarget->bTeam == gbPlayerNum && !(gTacticalStatus.uiFlags & INCOMBAT) )
		{
			// firing at one of our own guys who is not a rebel etc
			if ( pTarget->stats.bLife >= OKLIFE && !(pTarget->bCollapsed) && !AM_A_ROBOT( pTarget ) && (bReason == REASON_NORMAL_ATTACK ) )
			{
				// OK, sturn towards the prick
				// Change to fire ready animation
				ConvertGridNoToXY( pSoldier->sGridNo, &sTargetXPos, &sTargetYPos );

				pTarget->flags.fDontChargeReadyAPs = TRUE;
				// Ready weapon
				pTarget->SoldierReadyWeapon( sTargetXPos, sTargetYPos, FALSE );

				// ATE: Depending on personality, fire back.....

				// Do we have a gun in a\hand?
				if ( Item[ pTarget->inv[ HANDPOS ].usItem ].usItemClass == IC_GUN )
				{
					// Toggle burst capable...
					if ( !pTarget->bDoBurst )
					{
						// Changed by ADB, 1513
						//if ( IsGunBurstCapable( pTarget, HANDPOS , FALSE ) )
						if ( IsGunBurstCapable( &pTarget->inv[HANDPOS], FALSE, pTarget ) )
						{
							ChangeWeaponMode( pTarget );
						}
					}

					// Fire back!
					HandleItem( pTarget, pSoldier->sGridNo, pSoldier->pathing.bLevel, pTarget->inv[ HANDPOS ].usItem, FALSE );

				}

			}

			// don't enter combat on attack on one of our own mercs
			fEnterCombat = FALSE;
		}

		// if we've attacked a miner
		if ( IsProfileAHeadMiner( pTarget->ubProfile ) )
		{
			PlayerAttackedHeadMiner( pTarget->ubProfile );
		}
	}

	*ppTarget = pTarget;
	return( fEnterCombat );
}

SOLDIERTYPE *InternalReduceAttackBusyCount( )
{
	// Strange as this may seem, this function returns a pointer to
	// the *target* in case the target has changed sides as a result
	// of being attacked
	SOLDIERTYPE *				pSoldier;
	SOLDIERTYPE *				pTarget;
	BOOLEAN							fEnterCombat = FALSE;
	UINT32						cnt;
	UINT8						ubID;

#if 0
// 0verhaul:	None of this is necessary anymore with the new attack busy system
	if (ubID == NOBODY)
	{
		pSoldier = NULL;
		pTarget = NULL;
	}
	else
	{
		pSoldier = MercPtrs[ ubID ];
		if ( ubTargetID != NOBODY)
		{
			pTarget = MercPtrs[ ubTargetID ];
		}
		else
		{
			pTarget = NULL;
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String(">>Target ptr is null!" ) );
		}
	}

	if (fCalledByAttacker)
	{
		if (pSoldier && Item[pSoldier->inv[HANDPOS].usItem].usItemClass & IC_GUN)
		{
			if (pSoldier->bBulletsLeft > 0)
			{
				return( pTarget );
			}
		}
	}
#endif

//	if ((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT))
//	{

	if ( gTacticalStatus.ubAttackBusyCount == 0 )
	{
		// ATE: We have a problem here... if testversion, report error......
		// But for all means.... DON'T wrap!
		if ( (gTacticalStatus.uiFlags & INCOMBAT) )
		{
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("!!!!!!! &&&&&&& Problem with attacker busy count decrementing past 0.... preventing wrap-around." ) );
#ifdef JA2BETAVERSION
			if(!is_networked)ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Attack busy problem. Save, exit and send debug.txt + save file to Sir-Tech." );
			DebugAttackBusy( "Attack Busy Problem\n");
			//cheap hack to suppress message in MP - hayden.
#endif
		}
	}
	else
	{
		gTacticalStatus.ubAttackBusyCount--;
	}

	DebugAttackBusy( String( "New attack busy %d\n", gTacticalStatus.ubAttackBusyCount));

	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("!!!!!!! Ending attack, attack count now %d", gTacticalStatus.ubAttackBusyCount) );
	//	}

	if (gTacticalStatus.ubAttackBusyCount > 0)
	{
		return( NULL );
	}

	// I'm going to go out on	limb here and decide that from this point on we really care about the current soldier who
	// started this chain of events.	So we'll grab that one instead of relying on the correct one passed in.
	// This should help with collapses and other 'attacks' with
	// no attackers that happened due to actions from other soldiers or turns, shots against dead bodies, etc.

	pSoldier = NULL;

	if (gTacticalStatus.ubCurrentTeam == gbPlayerNum)
	{
		pSoldier = MercPtrs[ gusSelectedSoldier ];
	}
	else
	{
		for (cnt = gTacticalStatus.Team[ gTacticalStatus.ubCurrentTeam ].bFirstID;
			cnt <= gTacticalStatus.Team[ gTacticalStatus.ubCurrentTeam ].bLastID;
			cnt++)
		{
			if (MercPtrs[ cnt ] &&
				MercPtrs[ cnt ]->flags.uiStatusFlags & SOLDIER_UNDERAICONTROL)
			{
				pSoldier = MercPtrs[ cnt ];
				break;
			}
		}
	}
	// If we still haven't figured out who last acted, it could be that the team number changed during the attack.	Unfortunately this
	// can happen during a switch from real-time.	For now we will assume the last actor was a PC, but a real "Who started this?" pointer
	// would work quite well.	If only I could close all the holes that the UI opens so that one routine could handle everything.
	if (!pSoldier)
	{
		if (is_networked)
		{
			if(gusSelectedSoldier==156)
				return( NULL );
		}

		pSoldier = MercPtrs[ gusSelectedSoldier ];
	}

	if (!pSoldier)
		return (NULL);

	//if(!pSoldier && AreInMeanwhile())//dnl fix for preventing game crash in meanwhile after Queen slap Elliot
	//	return(NULL);

	//if (!pSoldier)

	ubID = pSoldier->ubID;

	DebugAttackBusy( String( "Ending action for %d\n", ubID ) );
	// Get the intended target info
	pTarget = NULL;
	if (pSoldier->ubTargetID != NOBODY)
	{
		pTarget = MercPtrs[ pSoldier->ubTargetID ];
	}

	if ((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT))
	{

		// Check to see if anyone was suppressed
		if (pTarget)
		{
			HandleSuppressionFire( pTarget->ubID, ubID );
		}
		else
		{
			HandleSuppressionFire( NOBODY, ubID );
		}


		//HandleAfterShootingGuy( pSoldier, pTarget );

		// suppression fire might cause the count to be increased, so check it again
		if (gTacticalStatus.ubAttackBusyCount > 0)
		{
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("!!!!!!! Starting suppression, attack count now %d", gTacticalStatus.ubAttackBusyCount) );
			return( NULL );
		}
	}

	// ATE: IN MEANWHILES, we have 'combat' in realtime....
	// this is so we DON'T call freeupattacker() which will cancel
	// the AI guy's meanwhile NPC stuff.
	// OK< let's NOT do this if it was the queen attacking....
	if ( AreInMeanwhile( ) && pSoldier != NULL && pSoldier->ubProfile != QUEEN )
	{
		return( NULL );
	}

#if 0
// 0verhaul:	This is moved to the end loop where everybody's state is reset for the next action
	if (pTarget)
	{
		// reset # of shotgun pellets hit by
		pTarget->bNumPelletsHitBy = 0;
		// reset flag for making "ow" sound on being shot
	}
#endif

	if (pSoldier)
	{
		// reset attacking hand
		pSoldier->ubAttackingHand = HANDPOS;

		// if there is a valid target, and our attack was noticed
		if ( pTarget && (pSoldier->flags.uiStatusFlags & SOLDIER_ATTACK_NOTICED) )
		{
			// stuff that only applies to when we attack
			if ( pTarget->ubBodyType != CROW )
			{
				if ( pSoldier->bTeam == gbPlayerNum )
				{
					fEnterCombat = ProcessImplicationsOfPCAttack( pSoldier, &pTarget, REASON_NORMAL_ATTACK );
					if ( !fEnterCombat )
					{
						DebugMsg( TOPIC_JA2, DBG_LEVEL_3, ">>Not entering combat as a result of PC attack" );
					}
				}
			}

			// global

			// ATE: If we are an animal, etc, don't change to hostile...
			// ( and don't go into combat )
			if ( pTarget->ubBodyType == CROW )
			{
				// Loop through our team, make guys who can see this fly away....
				{
					UINT32				cnt;
					SOLDIERTYPE		*pTeamSoldier;
					UINT8					ubTeam;

					ubTeam = pTarget->bTeam;

					for ( cnt = gTacticalStatus.Team[ ubTeam ].bFirstID, pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ ubTeam ].bLastID; cnt++, pTeamSoldier++ )
					{
						if ( pTeamSoldier->bActive && pTeamSoldier->bInSector )
						{
							if ( pTeamSoldier->ubBodyType == CROW )
							{
								if ( pTeamSoldier->aiData.bOppList[ pSoldier->ubID ] == SEEN_CURRENTLY )
								{
									//ZEROTIMECOUNTER( pTeamSoldier->timeCounters.AICounter );

									//MakeCivHostile( pTeamSoldier, 2 );

									HandleCrowFlyAway( pTeamSoldier );

								}
							}
						}
					}
				}

				// Don't enter combat...
				fEnterCombat = FALSE;
			}

			if ( gTacticalStatus.bBoxingState == BOXING )
			{
				if ( pTarget && pTarget->stats.bLife <= 0 )
				{
					// someone has won!
					EndBoxingMatch( pTarget );
				}
			}


			// if soldier and target were not both players and target was not under fire before...
			if ( ( pSoldier->bTeam != gbPlayerNum || pTarget->bTeam != gbPlayerNum ) )
			{
				if (pTarget->aiData.bOppList[ pSoldier->ubID ] != SEEN_CURRENTLY )
				{
					NoticeUnseenAttacker( pSoldier, pTarget, 0 );
				}
				// "under fire" lasts for 2 turns
				pTarget->aiData.bUnderFire = 2;
			}

		}
		else if ( pTarget )
		{
			// something is wrong here!
			if ( !pTarget->bActive || !pTarget->bInSector )
			{
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, ">>Invalid target attacked!" );
			}
			else if ( ! (pSoldier->flags.uiStatusFlags & SOLDIER_ATTACK_NOTICED) )
			{
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, ">>Attack not noticed" );
			}
		}
		else
		{
			// no target, don't enter combat
			fEnterCombat = FALSE;
		}

		if ( pSoldier->flags.fSayAmmoQuotePending )
		{
			pSoldier->flags.fSayAmmoQuotePending = FALSE;
			TacticalCharacterDialogue( pSoldier, QUOTE_OUT_OF_AMMO );
		}

		if (pSoldier->flags.uiStatusFlags & SOLDIER_PC)
		{
			UnSetUIBusy( ubID );
		}
		else
		{
			FreeUpNPCFromAttacking( ubID );
		}

		if ( !fEnterCombat )
		{
			// It's still possible that something happened
			if (!(gTacticalStatus.uiFlags & INCOMBAT) )
			{
				HandleBestSightingPositionInRealtime();
			}
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, ">>Not to enter combat from this attack" );
		}


		if ( fEnterCombat && !( gTacticalStatus.uiFlags & INCOMBAT ) )
		{
			// Go into combat!

			// If we are in a meanwhile... don't enter combat here...
			if ( !AreInMeanwhile( ) )
			{
				EnterCombatMode( pSoldier->bTeam );
			}
		}

		pSoldier->flags.uiStatusFlags &= (~SOLDIER_ATTACK_NOTICED);
	}

	if ( gTacticalStatus.fKilledEnemyOnAttack )
	{
		// Check for death quote...
		HandleKilledQuote( MercPtrs[ gTacticalStatus.ubEnemyKilledOnAttack ], MercPtrs[ gTacticalStatus.ubEnemyKilledOnAttackKiller ], gTacticalStatus.ubEnemyKilledOnAttackLocation, gTacticalStatus.bEnemyKilledOnAttackLevel );
		gTacticalStatus.fKilledEnemyOnAttack = FALSE;
	}

	// ATE: Check for stat changes....
	HandleAnyStatChangesAfterAttack( );


	if ( gTacticalStatus.fItemsSeenOnAttack	&& gTacticalStatus.ubCurrentTeam == gbPlayerNum )
	{
		gTacticalStatus.fItemsSeenOnAttack = FALSE;

		// Display quote!
		if ( !AM_AN_EPC( MercPtrs[ gTacticalStatus.ubItemsSeenOnAttackSoldier ] ) )
		{
			//ddd TacticalCharacterDialogueWithSpecialEvent( MercPtrs[ gTacticalStatus.ubItemsSeenOnAttackSoldier ], (UINT16)( QUOTE_SPOTTED_SOMETHING_ONE + Random( 2 ) ), DIALOGUE_SPECIAL_EVENT_SIGNAL_ITEM_LOCATOR_START, gTacticalStatus.sItemsSeenOnAttackGridNo, 0 );
			if(Chance(gGameExternalOptions.iChanceSayAnnoyingPhrase) )
				TacticalCharacterDialogueWithSpecialEvent( MercPtrs[ gTacticalStatus.ubItemsSeenOnAttackSoldier ], (UINT16)( QUOTE_SPOTTED_SOMETHING_ONE + Random( 2 ) ), DIALOGUE_SPECIAL_EVENT_SIGNAL_ITEM_LOCATOR_START, gTacticalStatus.usItemsSeenOnAttackGridNo, 0 );
		}
		else
		{
			// Turn off item lock for locators...
			gTacticalStatus.fLockItemLocators = FALSE;
			// Slide to location!
			SlideToLocation( 0,	gTacticalStatus.usItemsSeenOnAttackGridNo );
		}
	}

	if ( gTacticalStatus.uiFlags & CHECK_SIGHT_AT_END_OF_ATTACK )
	{
		UINT8 ubLoop;
		SOLDIERTYPE * pSightSoldier;

		AllTeamsLookForAll( FALSE );

		// call fov code
		ubLoop = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;
		for ( pSightSoldier = MercPtrs[ ubLoop ]; ubLoop <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; ubLoop++, pSightSoldier++ )
		{
			if ( pSightSoldier->bActive && pSightSoldier->bInSector )
			{
				RevealRoofsAndItems( pSightSoldier, TRUE, FALSE, pSightSoldier->pathing.bLevel, FALSE );
			}
		}
		gTacticalStatus.uiFlags &= ~CHECK_SIGHT_AT_END_OF_ATTACK;
	}

	DequeueAllDemandGameEvents( TRUE );

	// if we're in realtime, turn off the attacker's muzzle flash at this point
	if ( !(gTacticalStatus.uiFlags & INCOMBAT) && pSoldier )
	{
		EndMuzzleFlash( pSoldier );
	}

	if ( pSoldier && (pSoldier->bWeaponMode == WM_ATTACHED_GL || pSoldier->bWeaponMode == WM_ATTACHED_GL_BURST || pSoldier->bWeaponMode == WM_ATTACHED_GL_AUTO ))
	{
		if ( !Weapon[pSoldier->inv[HANDPOS].usItem].NoSemiAuto )
		{
			// change back to single shot
			pSoldier->bWeaponMode = WM_NORMAL;
			pSoldier->bDoAutofire = 0;
			pSoldier->bDoBurst = 0;
		}
		else
		{
			// change back to autofire
			pSoldier->bWeaponMode = WM_AUTOFIRE;
			pSoldier->bDoAutofire = 1;
			pSoldier->bDoBurst = TRUE;
		}
		DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2 );
	}

	// record last target
	// Check for valid target!
	if ( pSoldier )
	{
		pSoldier->sLastTarget = pSoldier->sTargetGridNo;
	}

	// Reset various flags and values that should be 0 once the action is overwith
	for (cnt = 0; cnt < guiNumMercSlots; cnt++)
	{
		pSoldier = MercSlots[ cnt ];
		if ( pSoldier )
		{
			pSoldier->bNumPelletsHitBy = 0;
			if ( !( pSoldier->flags.uiStatusFlags & SOLDIER_TURNINGFROMHIT ) )
			{
				// 0verhaul:  This is an ugly hack.  I don't like it, but until I figure out a better solution
				// for turning from a hit, it's the only way to not stop a soldier in mid-turn.
				pSoldier->flags.fGettingHit = FALSE;
			}

			if (pSoldier->ubAttackerID != NOBODY )
			{
				if (pSoldier->ubPreviousAttackerID != pSoldier->ubAttackerID)
				{
					pSoldier->ubNextToPreviousAttackerID = pSoldier->ubPreviousAttackerID;
				}

				pSoldier->ubPreviousAttackerID = pSoldier->ubAttackerID;

				// Why not keep the attacker ID for a dead queen monster?
				if ( pSoldier->stats.bLife != 0 && pSoldier->ubBodyType != QUEENMONSTER )
				{
					pSoldier->ubAttackerID = NOBODY;
				}
			}
		}
	}

	CheckForEndOfBattle( FALSE );

	return( pTarget );
}

SOLDIERTYPE * ReduceAttackBusyCount( )
{
#if 0
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("ReduceAttackBusyCount") );
	if ( ubID == NOBODY )
	{
		return( InternalReduceAttackBusyCount( ubID, fCalledByAttacker, NOBODY ) );
	}
	else
	{
		return( InternalReduceAttackBusyCount( ubID, fCalledByAttacker, MercPtrs[ ubID ]->ubTargetID ) );
	}
#endif
// 0verhaul:	This is now a simple subroutine.
	return InternalReduceAttackBusyCount( );
}

SOLDIERTYPE * FreeUpAttacker( )
{
	// Okay, so the objective here is not to simply decrease the ABC, but to twiddle it and see what
	// falls out.	So there's an increase followed immediately by a decrease.
	// If the ABC is 0 at this point, such as an aborted action might give, this will finalize the
	// action and allow the soldier to try something else.
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("FreeUpAttacker") );
	DebugAttackBusy( "Freeing up attacker\n");
	gTacticalStatus.ubAttackBusyCount++;
	return( ReduceAttackBusyCount( ) );
}

#if 0
// 0verhaul:	These routines are declared obsolete.	Call ReduceAttackBusyCount instead.
SOLDIERTYPE * FreeUpAttackerGivenTarget( UINT8 ubID, UINT8 ubTargetID )
{
	// Strange as this may seem, this function returns a pointer to
	// the *target* in case the target has changed sides as a result
	// of being attacked
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("FreeUpAttackerGivenTarget") );
	return( InternalReduceAttackBusyCount( ubID, TRUE, ubTargetID ) );
}

SOLDIERTYPE * ReduceAttackBusyGivenTarget( UINT8 ubID, UINT8 ubTargetID )
{
	// Strange as this may seem, this function returns a pointer to
	// the *target* in case the target has changed sides as a result
	// of being attacked
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("ReduceAttackBusyGivenTarget") );
	return( InternalReduceAttackBusyCount( ubID, FALSE, ubTargetID ) );
}
#endif


void StopMercAnimation( BOOLEAN fStop )
{
	static INT8 bOldRealtimeSpeed;

	if ( fStop )
	{
		if ( !(gTacticalStatus.uiFlags & SLOW_ANIMATION ) )
		{
			bOldRealtimeSpeed = gTacticalStatus.bRealtimeSpeed;
			gTacticalStatus.bRealtimeSpeed = -1;

			gTacticalStatus.uiFlags |=(SLOW_ANIMATION );

			ResetAllMercSpeeds( );

		}
	}
	else
	{
		if ( gTacticalStatus.uiFlags & SLOW_ANIMATION )
		{
			gTacticalStatus.bRealtimeSpeed = bOldRealtimeSpeed;

			gTacticalStatus.uiFlags &=(~SLOW_ANIMATION );

			ResetAllMercSpeeds( );
		}

	}


}

void ResetAllMercSpeeds( )
{
	SOLDIERTYPE *pSoldier;
	UINT32			cnt;

	for ( cnt = 0; cnt < TOTAL_SOLDIERS; cnt++ )
	{
		pSoldier = MercPtrs[ cnt ];

		if ( pSoldier->bActive && pSoldier->bInSector )
		{
			SetSoldierAniSpeed( pSoldier );
		}

	}

}

void SetActionToDoOnceMercsGetToLocation( UINT8 ubActionCode,	INT8 bNumMercsWaiting, UINT32 uiData1, UINT32 uiData2, UINT32 uiData3 )
{
	gubWaitingForAllMercsToExitCode				= ubActionCode;
	gbNumMercsUntilWaitingOver						= bNumMercsWaiting;
	guiWaitingForAllMercsToExitData[ 0 ]	= uiData1;
	guiWaitingForAllMercsToExitData[ 1 ]	= uiData2;
	guiWaitingForAllMercsToExitData[ 2 ]	= uiData3;

	// Setup timer counter and report back if too long....
	guiWaitingForAllMercsToExitTimer			= GetJA2Clock( );

	// ATE: Set flag to ignore sight...
	gTacticalStatus.uiFlags |= ( DISALLOW_SIGHT );

}

void HandleBloodForNewGridNo( SOLDIERTYPE *pSoldier )
{
	// Handle bleeding...
	if ( ( pSoldier->bBleeding > MIN_BLEEDING_THRESHOLD ) )
	{
		INT8 bBlood;

		bBlood = ( ( pSoldier->bBleeding-MIN_BLEEDING_THRESHOLD ) / BLOODDIVISOR);

		if ( bBlood > MAXBLOODQUANTITY )
		{
			bBlood = MAXBLOODQUANTITY;
		}

		// now, he shouldn't ALWAYS bleed the same amount; LOWER it perhaps. If it
		// goes less than zero, then no blood!
		bBlood -= (INT8)Random(7);

		if ( bBlood >= 0 )
		{
			// this handles all soldiers' dropping blood during movement
			DropBlood( pSoldier, bBlood, pSoldier->bVisible );
		}
	}
}

void CencelAllActionsForTimeCompression( void )
{
	SOLDIERTYPE *pSoldier;
	INT32 cnt;

	for ( pSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pSoldier++, cnt++ )
	{
		if ( pSoldier->bActive )
		{
			if ( pSoldier->bInSector )
			{
				// Hault!
				pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );

				// END AI actions
				CancelAIAction( pSoldier, TRUE );
			}
		}
	}
}

void AddManToTeam( INT8 bTeam )
{
	// ATE: If not loading game!
	if( !( gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		// Increment men in sector number!
		if (gTacticalStatus.Team[ bTeam ].bMenInSector == 0)
		{
			gTacticalStatus.Team[ bTeam ].bTeamActive = TRUE;
		}
		gTacticalStatus.Team[ bTeam ].bMenInSector++;
		if ( bTeam == ENEMY_TEAM )
		{
			gTacticalStatus.bOriginalSizeOfEnemyForce++;
		}
	}
}

void RemoveManFromTeam( INT8 bTeam )
{
	// ATE; if not loading game!
	if( !( gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		// Decrement men in sector number!
		gTacticalStatus.Team[ bTeam ].bMenInSector--;
		if (gTacticalStatus.Team[ bTeam ].bMenInSector == 0)
		{
			gTacticalStatus.Team[ bTeam ].bTeamActive = FALSE;
		}
		else if (gTacticalStatus.Team[ bTeam ].bMenInSector < 0)
		{
#ifdef JA2BETAVERSION
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Number of people on team %d dropped to %d", bTeam, gTacticalStatus.Team[ bTeam ].bMenInSector );
#endif
			// reset!
			gTacticalStatus.Team[ bTeam ].bMenInSector = 0;
		}
	}
}

void RemoveSoldierFromTacticalSector( SOLDIERTYPE *pSoldier, BOOLEAN fAdjustSelected )
{
	UINT8	ubID;
	SOLDIERTYPE *pNewSoldier;

	// reset merc's opplist
	InitSoldierOppList( pSoldier );

	// Remove!
	pSoldier->RemoveSoldierFromGridNo( );

	RemoveMercSlot( pSoldier );

	pSoldier->bInSector = FALSE;

	// Select next avialiable guy....
	if ( fAdjustSelected )
	{
		if ( guiCurrentScreen == GAME_SCREEN )
		{
			if ( gusSelectedSoldier == pSoldier->ubID	)
			{
				ubID = FindNextActiveAndAliveMerc( pSoldier, FALSE, FALSE );

				if ( ubID != NOBODY && ubID != gusSelectedSoldier )
				{
					SelectSoldier( ubID, FALSE, FALSE );
				}
				else
				{
					// OK - let's look for another squad...
					pNewSoldier = FindNextActiveSquad( pSoldier );

					if ( pNewSoldier != pSoldier )
					{
						// Good squad found!
						SelectSoldier( pNewSoldier->ubID, FALSE, FALSE );
					}
					else
					{
						// if here, make nobody
						gusSelectedSoldier = NOBODY;
					}
				}
			}
			UpdateTeamPanelAssignments( );
		}
		else
		{
			gusSelectedSoldier = NOBODY;

			if ( guiCurrentScreen == GAME_SCREEN )
			{
				// otherwise, make sure interface is team panel...
				UpdateTeamPanelAssignments( );
				SetCurrentInterfacePanel( (UINT8)TEAM_PANEL );
			}
		}
	}

}


void	DoneFadeOutDueToDeath( void )
{
	// Quit game....
	InternalLeaveTacticalScreen( MAINMENU_SCREEN );
	//SetPendingNewScreen( MAINMENU_SCREEN );
}

void EndBattleWithUnconsciousGuysCallback( UINT8 bExitValue )
{
	// Enter mapscreen.....
	if(!is_client)CheckAndHandleUnloadingOfCurrentWorld();
	else	
	{
		ScreenMsg( FONT_LTGREEN, MSG_CHAT, MPClientMessage[40] );
		if(!cDisableSpectatorMode)
		{
			gTacticalStatus.uiFlags |= SHOW_ALL_MERCS;//hayden
			ScreenMsg( FONT_YELLOW, MSG_CHAT, MPClientMessage[41] );
		}
		else 
			ScreenMsg( FONT_LTBLUE, MSG_CHAT, L"spectator mode disabled");
		
		teamwiped();
	}
}


void InitializeTacticalStatusAtBattleStart( void )
{
	INT8							bLoop;
	INT32							cnt;
	SOLDIERTYPE *			pSoldier;

	gTacticalStatus.ubArmyGuysKilled = 0;
	gTacticalStatus.bOriginalSizeOfEnemyForce = 0;

	gTacticalStatus.fPanicFlags = 0;
	gTacticalStatus.fEnemyFlags = 0;
	for ( bLoop = 0; bLoop < NUM_PANIC_TRIGGERS; bLoop++ )
	{
		gTacticalStatus.sPanicTriggerGridNo[ bLoop ] = NOWHERE;
		gTacticalStatus.bPanicTriggerIsAlarm[ bLoop ] = FALSE;
		gTacticalStatus.ubPanicTolerance[ bLoop ] = 0;
	}

	for( cnt = 0; cnt < MAXTEAMS; cnt++ )
	{
		gTacticalStatus.Team[ cnt ].ubLastMercToRadio = NOBODY;
		gTacticalStatus.Team[ cnt ].bAwareOfOpposition = FALSE;
	}

	gTacticalStatus.ubTheChosenOne	= NOBODY;

	ClearIntList();

	// make sure none of our guys have leftover shock values etc
	for ( cnt = gTacticalStatus.Team[ 0 ].bFirstID; cnt <= gTacticalStatus.Team[ 0 ].bLastID; cnt++ )
	{
		pSoldier = MercPtrs[ cnt ];
		pSoldier->aiData.bShock = 0;
		pSoldier->bTilesMoved = 0;
	}

	// loop through everyone; clear misc flags
	for ( cnt = 0; cnt <= gTacticalStatus.Team[ CIV_TEAM ].bLastID; cnt++ )
	{
		MercPtrs[ cnt ]->ubMiscSoldierFlags = 0;
	}

}


void	DoneFadeOutDemoCreatureLevel( void )
{
	// OK, insertion data found, enter sector!
	SetCurrentWorldSector( 1, 16, 0 );

	FadeInGameScreen( );
}


void DemoEndOKCallback( INT8 bExitCode )
{
}

void HandleEndDemoInCreatureLevel( )
{
}


void DeathTimerCallback( void )
{
	if (gTacticalStatus.Team[ CREATURE_TEAM ].bMenInSector > gTacticalStatus.Team[ ENEMY_TEAM ].bMenInSector )
	{
		DoMessageBox( MSG_BOX_BASIC_STYLE, LargeTacticalStr[ LARGESTR_NOONE_LEFT_CAPABLE_OF_BATTLE_AGAINST_CREATURES_STR ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_OK, EndBattleWithUnconsciousGuysCallback, NULL );
	}
	else
	{
		DoMessageBox( MSG_BOX_BASIC_STYLE, LargeTacticalStr[ LARGESTR_NOONE_LEFT_CAPABLE_OF_BATTLE_STR ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_OK, EndBattleWithUnconsciousGuysCallback, NULL );
	}
}

void CaptureTimerCallback( void )
{
	if( gfSurrendered )
	{
		DoMessageBox( MSG_BOX_BASIC_STYLE, LargeTacticalStr[ 3 ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_OK, EndBattleWithUnconsciousGuysCallback, NULL );
	}
	else
	{
		DoMessageBox( MSG_BOX_BASIC_STYLE, LargeTacticalStr[ LARGESTR_HAVE_BEEN_CAPTURED ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_OK, EndBattleWithUnconsciousGuysCallback, NULL );
	}
	gfSurrendered = FALSE;
}

void DoPOWPathChecks( void )
{
	INT32						iLoop;
	SOLDIERTYPE *		pSoldier;

	// loop through all mercs on our team and if they are POWs in sector, do POW path check and
	// put on a squad if available
	for ( iLoop = gTacticalStatus.Team[ gbPlayerNum ].bFirstID; iLoop <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; iLoop++ )
	{
		pSoldier = MercPtrs[ iLoop ];

		if ( pSoldier->bActive && pSoldier->bInSector && pSoldier->bAssignment == ASSIGNMENT_POW )
		{
			// check to see if POW has been freed!
			// this will be true if a path can be made from the POW to either of 3 gridnos
			// 10492 (hallway) or 10482 (outside), or 9381 (outside)
			if ( FindBestPath( pSoldier, 10492, 0, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) )//dnl!!!
			{
				// drop out of if
			}
			else if ( FindBestPath( pSoldier, 10482, 0, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) )//dnl!!!
			{
				// drop out of if
			}
			else if ( FindBestPath( pSoldier, 9381, 0, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) )//dnl!!!
			{
				// drop out of if
			}
			else
			{
				continue;
			}
			// free! free!
			// put them on any available squad
			pSoldier->aiData.bNeutral = FALSE;
			AddCharacterToAnySquad( pSoldier );
			pSoldier->DoMercBattleSound( BATTLE_SOUND_COOL1 );
		}
	}
}

BOOLEAN HostileCiviliansPresent( void )
{
	INT32						iLoop;
	SOLDIERTYPE *		pSoldier;

	if ( gTacticalStatus.Team[ CIV_TEAM ].bTeamActive == FALSE )
	{
		return( FALSE );
	}

	for ( iLoop = gTacticalStatus.Team[ CIV_TEAM ].bFirstID; iLoop <= gTacticalStatus.Team[ CIV_TEAM ].bLastID; iLoop++ )
	{
		pSoldier = MercPtrs[ iLoop ];

		if ( pSoldier->bActive && pSoldier->bInSector && pSoldier->stats.bLife > 0 && !pSoldier->aiData.bNeutral )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}

BOOLEAN HostileCiviliansWithGunsPresent( void )
{
	INT32						iLoop;
	SOLDIERTYPE *		pSoldier;

	if ( gTacticalStatus.Team[ CIV_TEAM ].bTeamActive == FALSE )
	{
		return( FALSE );
	}

	for ( iLoop = gTacticalStatus.Team[ CIV_TEAM ].bFirstID; iLoop <= gTacticalStatus.Team[ CIV_TEAM ].bLastID; iLoop++ )
	{
		pSoldier = MercPtrs[ iLoop ];

		if ( pSoldier->bActive && pSoldier->bInSector && pSoldier->stats.bLife > 0 && !pSoldier->aiData.bNeutral )
		{
			if ( FindAIUsableObjClass( pSoldier, IC_WEAPON ) == -1 )
			{
				return( TRUE );
			}

		}
	}

	return( FALSE );
}

BOOLEAN HostileBloodcatsPresent( void )
{
	INT32						iLoop;
	SOLDIERTYPE *		pSoldier;

	if ( gTacticalStatus.Team[ CREATURE_TEAM ].bTeamActive == FALSE )
	{
		return( FALSE );
	}

	for ( iLoop = gTacticalStatus.Team[ CREATURE_TEAM ].bFirstID; iLoop <= gTacticalStatus.Team[ CREATURE_TEAM ].bLastID; iLoop++ )
	{
		pSoldier = MercPtrs[ iLoop ];

		//KM : Aug 11, 1999 -- Patch fix:	Removed the check for bNeutral.	Bloodcats automatically become hostile
		//		on site.	Because the check used to be there, it was possible to get into a 2nd battle elsewhere
		//	 which is BAD BAD BAD!
		if ( pSoldier->bActive && pSoldier->bInSector && pSoldier->stats.bLife > 0 && pSoldier->ubBodyType == BLOODCAT )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


void HandleCreatureTenseQuote( )
{
// WDS - make number of mercenaries, etc. be configurable
//	UINT8	ubMercsInSector[ CODE_MAXIMUM_NUMBER_OF_PLAYER_SLOTS ] = { 0 };
	std::vector<UINT8>	ubMercsInSector (CODE_MAXIMUM_NUMBER_OF_PLAYER_SLOTS, 0);
	UINT8	ubNumMercs = 0;
	UINT8	ubChosenMerc;
	SOLDIERTYPE *pTeamSoldier;
	INT32 cnt;
	INT32 uiTime;


	// Check for quote seeing creature attacks....
	if ( gubQuest[ QUEST_CREATURES ] != QUESTDONE )
	{
		if ( gTacticalStatus.uiFlags & IN_CREATURE_LAIR )
		{
			if( !(gTacticalStatus.uiFlags & INCOMBAT ) )
			{
				uiTime = GetJA2Clock( );

				if ( ( uiTime - gTacticalStatus.uiCreatureTenseQuoteLastUpdate ) > (UINT32)( gTacticalStatus.sCreatureTenseQuoteDelay * 1000 ) )
				{
					gTacticalStatus.uiCreatureTenseQuoteLastUpdate = uiTime;

					// set up soldier ptr as first element in mercptrs list
					cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

					// run through list
					for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pTeamSoldier++ )
					{
						// Add guy if he's a candidate...
						if ( OK_INSECTOR_MERC( pTeamSoldier ) && !AM_AN_EPC( pTeamSoldier ) && !( pTeamSoldier->flags.uiStatusFlags & SOLDIER_GASSED ) && !(AM_A_ROBOT( pTeamSoldier )) && !pTeamSoldier->flags.fMercAsleep )
						{
							ubMercsInSector[ ubNumMercs ] = (UINT8)cnt;
							ubNumMercs++;
						}
					}

					// If we are > 0
					if ( ubNumMercs > 0 )
					{
						ubChosenMerc = (UINT8)Random( ubNumMercs );

						DoCreatureTensionQuote ( MercPtrs[ ubMercsInSector[ ubChosenMerc ] ] );
					}

					// Adjust delay....
					gTacticalStatus.sCreatureTenseQuoteDelay = (INT16)( 60 + Random( 60 ) );

				}
			}
		}
	}
}


void DoCreatureTensionQuote( SOLDIERTYPE *pSoldier )
{
	INT32	iRandomQuote;
	BOOLEAN fCanDoQuote = TRUE;
	INT32	iQuoteToUse;

	// Check for playing smell quote....
	iRandomQuote = Random( 3 );

	switch( iRandomQuote )
	{
	case 0:

		iQuoteToUse = QUOTE_SMELLED_CREATURE;
		if ( !( pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_SMELLED_CREATURE ) )
		{
			// set flag
			pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_SMELLED_CREATURE;
		}
		else
		{
			fCanDoQuote = FALSE;
		}
		break;

	case 1:

		iQuoteToUse = QUOTE_TRACES_OF_CREATURE_ATTACK;
		if ( !( pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_SPOTTING_CREATURE_ATTACK ) )
		{
			// set flag
			pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_SPOTTING_CREATURE_ATTACK;
		}
		else
		{
			fCanDoQuote = FALSE;
		}
		break;

	case 2:

		iQuoteToUse = QUOTE_WORRIED_ABOUT_CREATURE_PRESENCE;
		if ( !( pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_WORRIED_ABOUT_CREATURES ) )
		{
			// set flag
			pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_WORRIED_ABOUT_CREATURES;
		}
		else
		{
			fCanDoQuote = FALSE;
		}
		break;
	default:
		AssertMsg(0, "Invalid quote");
		return;
	}

	if ( fCanDoQuote )
	{
		TacticalCharacterDialogue( pSoldier, (INT16)iQuoteToUse );
	}
}

// Kaiden: Taken from UB to reveal all items after combat.
void RevealAllDroppedEnemyItems()
{
	UINT32 uiCnt=0;

	//loop through all the items
	for(uiCnt=0; uiCnt<guiNumWorldItems; uiCnt++ )
	{
		//if the item exists AND the item was droppped from an enemy
		if( gWorldItems[ uiCnt ].fExists && gWorldItems[ uiCnt ].usFlags & WORLD_ITEM_DROPPED_FROM_ENEMY )
		{
			gWorldItems[ uiCnt ].bVisible = VISIBLE;
		}
	}

	// Make team look for items
	AllSoldiersLookforItems( TRUE );

}


// HEADROCK HAM 3.2: This function from HAM 2.9 makes a character look around himself and try to find friends. Dead
// friends lower the result, while more friends (ESP good leaders) will raise the result. The result can then be used
// for any purpose, although this was written specifically to alter a character's suppression tolerance.

INT8 CheckStatusNearbyFriendlies( SOLDIERTYPE *pSoldier )
{
	SOLDIERTYPE * pLeader;
	UINT8 sModifier = 0;
	INT16 usEffectiveLeadership = 0;
	UINT16 usEffectiveRangeToLeader = 0;
	INT16 usBestLeader = 0;
	INT16 usFriendBonus = 0;
	UINT8 ubLevelDifference = 0;

	// Run through each friendly.
	for ( UINT8 iCounter = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID ; iCounter <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID ; iCounter ++ )
	{
		pLeader = MercPtrs[ iCounter ];
		// Make sure that character is alive, not too shocked, and conscious, and of higher experience level
		// than the character being suppressed.
		if (pLeader != pSoldier && pLeader->bActive && pLeader->aiData.bShock < pLeader->stats.bLeadership/5 && 
			pLeader->stats.bLife >= OKLIFE )
		{
			ubLevelDifference = (pLeader->stats.bExpLevel - pSoldier->stats.bExpLevel );
			// Calculate character's leadership and range/3
			usEffectiveLeadership = ((EffectiveLeadership( pLeader ) - 25) / 15);
			usEffectiveRangeToLeader = PythSpacesAway( pSoldier->sGridNo, pLeader->sGridNo ) / 3;

			// SANDRO - add effective leadership and level to determine if we can help our friend to feel better :)
			if ( gGameOptions.fNewTraitSystem && HAS_SKILL_TRAIT( pLeader, SQUADLEADER_NT ) ) 
			{
				ubLevelDifference += (gSkillTraitValues.ubSLEffectiveLevelAsStandby * NUM_SKILL_TRAITS( pLeader, SQUADLEADER_NT ));
				usEffectiveLeadership += (gSkillTraitValues.ubSLEffectiveLevelAsStandby * NUM_SKILL_TRAITS( pLeader, SQUADLEADER_NT ));
			}

			if ( ubLevelDifference >= 0 )
			{
				// If leader is within range of his leadership stat
				if (usEffectiveRangeToLeader <= usEffectiveLeadership+1)
				{
					// The difference in experience level is important!
					usEffectiveLeadership += ubLevelDifference;
					// Reduce effective leadership with every 3 tiles of distance
					usEffectiveLeadership -= usEffectiveRangeToLeader-1;

					// If this is the best leader we've seen so far,
					if (usEffectiveLeadership > usBestLeader)
					{
						// Set this as the best leader
						usBestLeader = usEffectiveLeadership;
					}
					// Friends within range always give at least one tolerance bonus point.
					usFriendBonus += 1;
				}
			}
		}
		// Incapacitated or heavily suppressed friends will not be good for our tolerance!
		else if (pLeader != pSoldier && pLeader->bActive && (pLeader->aiData.bShock > pSoldier->aiData.bShock || pLeader->stats.bLife <= OKLIFE) )
		{
			usEffectiveRangeToLeader = PythSpacesAway( pSoldier->sGridNo, pLeader->sGridNo );
			// If they are no more than 5 tiles away,
			if (usEffectiveRangeToLeader <= 5)
			{	
				// Penalty is based on the difference between experience levels, and the range between them,
				// and is never less than 1 point.
				usEffectiveLeadership = (pLeader->stats.bExpLevel - pSoldier->stats.bExpLevel) / __max(1,(usEffectiveRangeToLeader/2));
				usFriendBonus -= __max(1, usEffectiveLeadership);
			}
			// SANDRO - however, dead squadleader is very bad for our psychics
			if ( gGameOptions.fNewTraitSystem && HAS_SKILL_TRAIT( pLeader, SQUADLEADER_NT ) && 
				( pLeader->flags.uiStatusFlags & SOLDIER_DEAD ) && usEffectiveRangeToLeader <= 10 ) 
			{
				usFriendBonus -= ( gSkillTraitValues.ubSLDeathMoralelossMultiplier * NUM_SKILL_TRAITS( pLeader, SQUADLEADER_NT ));
			}
		}
	}

	// If we did find someone who's a good enough leader to help us out,
	if (usBestLeader > 0)
	{
		// Add his leadership bonus, minus the point we got for him before. He'll give at least one
		// point, like anybody else.
		usFriendBonus += __max(usBestLeader-1, 1);
	}

	// Add no more than five points for nearby friends.
	usFriendBonus = __min(usFriendBonus, 5);
	usFriendBonus = __max(usFriendBonus, -5);
	sModifier += usFriendBonus;

	return(sModifier);

}

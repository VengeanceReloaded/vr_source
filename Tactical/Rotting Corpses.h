#ifndef _ROTTING_CORPSES_H
#define _ROTTING_CORPSES_H

#include "tile animation.h"

#define	NUM_CORPSE_SHADES		17

enum RottingCorpseDefines
{
	NO_CORPSE,
	SMERC_JFK,
	SMERC_BCK,
	SMERC_FWD,
	SMERC_DHD,
	SMERC_PRN,
	SMERC_WTR,
	SMERC_FALL,
	SMERC_FALLF,

	MMERC_JFK,
	MMERC_BCK,
	MMERC_FWD,
	MMERC_DHD,
	MMERC_PRN,
	MMERC_WTR,
	MMERC_FALL,
	MMERC_FALLF,

	FMERC_JFK,
	FMERC_BCK,
	FMERC_FWD,
	FMERC_DHD,
	FMERC_PRN,
	FMERC_WTR,
	FMERC_FALL,
	FMERC_FALLF,

	// CIVS
	M_DEAD1,
	K_DEAD1,
	H_DEAD1,
	FT_DEAD1,
	S_DEAD1,
	W_DEAD1,
	C_DEAD1,
	M_DEAD2,
	K_DEAD2,
	H_DEAD2,

	FT_DEAD2,
	S_DEAD2,
	W_DEAD2,
	C_DEAD2,
	BLOODCAT_DEAD,
	COW_DEAD,
	ADULTMONSTER_DEAD,
	INFANTMONSTER_DEAD,
	LARVAEMONSTER_DEAD,
	ROTTING_STAGE2,

	TANK1_DEAD,
	TANK2_DEAD,
	HUMMER_DEAD,
	ICECREAM_DEAD,
	QUEEN_MONSTER_DEAD,
	ROBOT_DEAD,
	BURNT_DEAD,
	EXPLODE_DEAD,

	NUM_CORPSES,

} ;



#define		ROTTING_CORPSE_FIND_SWEETSPOT_FROM_GRIDNO			0x001		//Find the closest spot to the given gridno
#define		ROTTING_CORPSE_USE_NORTH_ENTRY_POINT					0x002		//Find the spot closest to the north entry grid
#define		ROTTING_CORPSE_USE_SOUTH_ENTRY_POINT					0x004		//Find the spot closest to the south entry grid
#define		ROTTING_CORPSE_USE_EAST_ENTRY_POINT						0x008		//Find the spot closest to the east entry grid
#define		ROTTING_CORPSE_USE_WEST_ENTRY_POINT						0x010		//Find the spot closest to the west entry grid
#define		ROTTING_CORPSE_USE_CAMO_PALETTE							0x020		//We use cammo palette here....
#define		ROTTING_CORPSE_VEHICLE												0x040		//Vehicle Corpse
#define		ROTTING_CORPSE_USE_STEALTH_PALETTE							0x080		//We use stealth palette here....
#define		ROTTING_CORPSE_USE_URBAN_CAMO_PALETTE							0x100		//We use urban palette here....
#define		ROTTING_CORPSE_USE_DESERT_CAMO_PALETTE							0x200		//We use desert palette here....
#define		ROTTING_CORPSE_USE_SNOW_CAMO_PALETTE							0x400		//We use snow palette here....

typedef struct
{
	UINT8												ubType;
	UINT8												ubBodyType;
	INT32												sGridNo;
	FLOAT												dXPos;
	FLOAT												dYPos;
	INT16												sHeightAdjustment;

	PaletteRepID								HeadPal;			// Palette reps
	PaletteRepID								PantsPal;	
	PaletteRepID								VestPal;	
	PaletteRepID								SkinPal;	

	UINT8												ubDirection;
	UINT32											uiTimeOfDeath;

	UINT16											usFlags;

	INT8												bLevel;

	INT8												bVisible;
	INT8												bNumServicingCrows;
	UINT8												ubProfile;
	BOOLEAN					 fHeadTaken;
		UINT8												ubAIWarningValue;

	UINT8												ubFiller[ 12 ];

} ROTTING_CORPSE_DEFINITION;


typedef struct
{
	ROTTING_CORPSE_DEFINITION		def;
	BOOLEAN											fActivated;

	ANITILE											*pAniTile;

	SGPPaletteEntry							*p8BPPPalette; 
	UINT16											*p16BPPPalette;
	UINT16											*pShades[ NUM_CORPSE_SHADES ]; 
	INT16												sGraphicNum;
	INT32												iCachedTileID;
	FLOAT												dXPos;
	FLOAT												dYPos;

	BOOLEAN											fAttractCrowsOnlyWhenOnScreen;
	INT32												iID;

} ROTTING_CORPSE;



INT32	AddRottingCorpse( ROTTING_CORPSE_DEFINITION *pCorpseDef );

void RemoveCorpse( INT32 iCorpseID );
void RemoveCorpses( );


BOOLEAN TurnSoldierIntoCorpse( SOLDIERTYPE *pSoldier, BOOLEAN fRemoveMerc, BOOLEAN fCheckForLOS );

INT16 FindNearestRottingCorpse( SOLDIERTYPE *pSoldier );

void AllMercsOnTeamLookForCorpse( ROTTING_CORPSE *pCorpse, INT8 bTeam );
void MercLooksForCorpses( SOLDIERTYPE *pSoldier );
void RebuildAllCorpseShadeTables( );

UINT16				CreateCorpsePaletteTables( ROTTING_CORPSE *pCorpse );

INT32 FindNearestAvailableGridNoForCorpse( ROTTING_CORPSE_DEFINITION *pCorpseDef, INT8 ubRadius );


void HandleRottingCorpses( );
void AddCrowToCorpse( ROTTING_CORPSE *pCorpse );

void VaporizeCorpse( INT32 sGridNo, UINT16 usStructureID );
void CorpseHit( INT32 sGridNo, UINT16 usStructureID );

void HandleCrowLeave( SOLDIERTYPE *pSoldier );

void HandleCrowFlyAway( SOLDIERTYPE *pSoldier );

// WDS - increase number of corpses
// HEADROCK HAM 3.6: Increase again (250->500).
#define		MAX_ROTTING_CORPSES		500

//extern ROTTING_CORPSE	gRottingCorpse[ MAX_ROTTING_CORPSES ];
extern std::vector<ROTTING_CORPSE> gRottingCorpse;
extern INT32					giNumRottingCorpse;
extern UINT8					gb4DirectionsFrom8[8];

ROTTING_CORPSE *GetCorpseAtGridNo( INT32 sGridNo , INT8 bLevel );
BOOLEAN IsValidDecapitationCorpse( ROTTING_CORPSE *pCorpse );
void DecapitateCorpse( SOLDIERTYPE *pSoldier, INT32 sGridNo,  INT8 bLevel );

void GetBloodFromCorpse( SOLDIERTYPE *pSoldier );

UINT16 GetCorpseStructIndex( ROTTING_CORPSE_DEFINITION *pCorpseDef, BOOLEAN fForImage );

void LookForAndMayCommentOnSeeingCorpse( SOLDIERTYPE *pSoldier, INT32 sGridNo, UINT8 ubLevel );

INT32 GetGridNoOfCorpseGivenProfileID( UINT8 ubProfileID );

void DecayRottingCorpseAIWarnings( void );
UINT8 GetNearestRottingCorpseAIWarning( INT32 sGridNo );

#endif
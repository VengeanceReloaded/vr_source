#ifdef PRECOMPILEDHEADERS
	#include "Tactical All.h"
#else
	#include <memory.h>
	#include "Inventory Choosing.h"
	#include "animation data.h"
	#include "Items.h"
	#include "Random.h"
	#include "Weapons.h"
	#include "Strategic Status.h"
	#include "Campaign.h"
	#include "GameSettings.h"
	#include "AIInternals.h"
	#include "strategicmap.h"
	#include "Auto Resolve.h"
	#include "Map Screen Interface Map.h"
	#include "EnemyItemDrops.h"
	#include "strategic.h"
	#include "Game Clock.h"
#endif

/*
#define ENEMYAMMODROPRATE       100 //Madd 50      // % of time enemies drop ammunition
#define ENEMYGRENADEDROPRATE    100 //Madd 25      // % of time enemies drop grenades
#define ENEMYEQUIPDROPRATE       50 //Madd 15      // % of stuff enemies drop equipment

// only 1/10th of what enemies drop...
#define MILITIAAMMODROPRATE      100 //Madd 5      // % of time enemies drop ammunition
#define MILITIAGRENADEDROPRATE	 100 //Madd 3      // % of time enemies drop grenades
#define MILITIAEQUIPDROPRATE      50 //Madd 2      // % of stuff enemies drop equipment

*/
int ENEMYAMMODROPRATE;       //100 //Madd 50      // % of time enemies drop ammunition
int ENEMYGRENADEDROPRATE;    //100 //Madd 25      // % of time enemies drop grenades
int ENEMYEQUIPDROPRATE;       //50 //Madd 15      // % of stuff enemies drop equipment

// only 1/10th of what enemies drop...
int MILITIAAMMODROPRATE;      //100 //Madd 5      // % of time enemies drop ammunition
int MILITIAGRENADEDROPRATE;	 //100 //Madd 3      // % of time enemies drop grenades
int MILITIAEQUIPDROPRATE;      //50 //Madd 2      // % of stuff enemies drop equipment

UINT16 itemMortar;
UINT16 itemGrenadeLauncher;
UINT16 itemLAW;
UINT16 itemRPG;

#define MAX_MORTARS_PER_TEAM			1			// one team can't randomly roll more than this many mortars per sector


UINT32 guiMortarsRolledByTeam = 0;

//
// WDS note 08/06/2008:  This is no longer used
// ARMY_GUN_CHOICE_TYPE gRegularArmyGunChoices[ARMY_GUN_LEVELS] =
// {	// INDEX		CLASS				 #CHOICES
// 	{ /* 0 - lo pistols			*/	2,	SW38,					DESERTEAGLE,		-1,						-1,					-1					},
// 	{ /* 1 - hi pistols			*/	2,	GLOCK_17,			BERETTA_93R,		-1,						-1,					-1					},
// 	{ /* 2 - lo SMG/shotgun	*/	2,	M870,					MP5K,						-1,						-1,					-1					},
// 	{ /* 3 - lo rifles			*/	1,	MINI14,				-1,							-1,						-1,					-1					},
// 	{ /* 4 - hi SMGs				*/	2,	MAC10,				COMMANDO,				-1,						-1,					-1					},
// 	{ /* 5 - med rifles  		*/	1,	G41,					-1,							-1,						-1,					-1					},
// 	{ /* 6 - sniper rifles	*/	1,	M24,					-1,							-1,						-1,					-1					},
// 	{	/* 7 - hi rifles			*/	2,	M14,					C7,							-1,						-1,					-1					},
// 	{ /* 8 - best rifle			*/	1,	FNFAL,				-1,							-1,						-1,					-1					},
// 	{ /* 9 - machine guns		*/	1,	MINIMI,				-1,							-1,						-1,					-1					},
// 	{ /* 10- rocket rifle		*/	2,	ROCKET_RIFLE,	MINIMI,							-1,						-1,					-1					},
// };


ARMY_GUN_CHOICE_TYPE gExtendedArmyGunChoices[ARMY_GUN_LEVELS];// =
//{	// INDEX		CLASS				 #CHOICES
//	{ /* 0 - lo pistols			*/	6,	SW38,					BARRACUDA,			DESERTEAGLE,	GLOCK_17,		M1911,	BERETTA_92F,-1,-1,-1,-1 ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1		},
//	{ /* 1 - hi pist/shtgn	*/	6,	GLOCK_18,			BERETTA_93R,		P7M8,	M870,				M950		,FIVE7,-1,-1,-1,-1	,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1		},
//	{ /* 2 - lo SMGs/shtgn	*/	6,	 TYPE85,				THOMPSON,			MP5, MP53,					MP5K, MP5SD,-1,-1,-1,-1	,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
//	{ /* 3 - lo rifles    	*/	4,	MINI14,				SKS,						BENNELLI,						SPAS15,					-1		,-1,-1,-1,-1,-1		,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1	},
//	{ /* 4 - hi SMGs				*/	7,	MAC10,				AKSU74,		MICRO_UZI,		COMMANDO,			M960,					UMP45		,MP5N,-1,-1,-1	,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1		},
//	{ /* 5 - med rifles  		*/	6,	AKM,					G3A3,						G41,					AK74,				M900	,AK103,-1,-1,-1,-1	,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1		},
//	{ /* 6 - sniper rifles	*/	6,	DRAGUNOV,			M24,						PSG,						VAL_SILENT,					BARRETT		,MSG90A1,-1,-1,-1,-1	,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1	},
//	{	/* 7 - hi rifles			*/	7,	FAMAS,				M14,						AUG,					C7,				ENFIELD,		G36K,G36C,-1,-1,-1	,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1	},
//	{ /* 8 - best rifle			*/	4,	FNFAL,				P90,						G11,						TAR21,					-1,		-1,-1,-1,-1,-1	,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1		},
//	{ /* 9 - machine guns		*/	3,	MINIMI,				RPK74,					HK21E,				-1,					-1,-1,-1,-1,-1,-1	,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1		},
//	{ /* 10- rocket rifle		*/	5,	ROCKET_RIFLE,	AUTO_ROCKET_RIFLE ,	 RPK74,				HK21E,		MINIMI , -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
//};

ARMY_GUN_CHOICE_TYPE gArmyItemChoices[MAX_ITEM_TYPES];

void RandomlyChooseWhichItemsAreDroppable( SOLDIERCREATE_STRUCT *pp, INT8 bSoldierClass );
void EquipTank( SOLDIERCREATE_STRUCT *pp );

void ChooseKitsForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp, INT8 bKitClass );
void ChooseMiscGearForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp, INT8 bMiscClass );
void ChooseBombsForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp, INT8 bBombClass );
// Headrock: Added function definition for LBE chooser
void ChooseLBEsForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp, INT8 bLBEClass );

UINT16 PickARandomItem(UINT8 typeIndex);
UINT16 PickARandomItem(UINT8 typeIndex, UINT8 maxCoolness);
UINT16 PickARandomItem(UINT8 typeIndex, UINT8 maxCoolness, BOOLEAN getMatchingCoolness);
UINT16 PickARandomAttachment(UINT8 typeIndex, UINT16 usBaseItem, UINT8 maxCoolness, BOOLEAN getMatchingCoolness);


void InitArmyGunTypes(void)
{
	ARMY_GUN_CHOICE_TYPE *pGunChoiceTable;
	UINT32 uiGunLevel;
	UINT32 uiChoice;
	UINT16 bItemNo;
	UINT16 ubWeapon;

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"InitArmyGunTypes ");

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"InitArmyGunTypes set gun choice table");
	// depending on selection of the gun nut option
	//if (gGameOptions.fGunNut)
	//{
		// use table of extended gun choices
		pGunChoiceTable = &(gExtendedArmyGunChoices[0]);
	//}
	//else
	//{
	//	// use table of regular gun choices
	//	pGunChoiceTable = &(gRegularArmyGunChoices[0]);
	//}

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"InitArmyGunTypes pick a gun type");
	// for each gun category
	for (uiGunLevel = 0; uiGunLevel <	ARMY_GUN_LEVELS; uiGunLevel++)
	{
		// choose one the of the possible gun choices to be used by the army for this game & store it
		uiChoice = Random(pGunChoiceTable[ uiGunLevel ].ubChoices);

		bItemNo = pGunChoiceTable[ uiGunLevel ].bItemNo[ uiChoice ];
		AssertMsg(bItemNo != -1, "Invalid army gun choice in table");
		gStrategicStatus.ubStandardArmyGunIndex[uiGunLevel] = (UINT8) bItemNo;
	}

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"InitArmyGunTypes mark weapons as not dropped");
	// set all flags that track whether this weapon type has been dropped before to FALSE
	for (ubWeapon = 0; ubWeapon < MAXITEMS; ubWeapon++)
	{
		DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("InitArmyGunTypes marking weapons %d", ubWeapon));
		gStrategicStatus.fWeaponDroppedAlready[ubWeapon] = FALSE;
	}

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"InitArmyGunTypes marking 38s as dropped");
	// avoid auto-drops for the gun class with the crappiest guns in it
	MarkAllWeaponsOfSameGunClassAsDropped( SW38 );

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"InitArmyGunTypes done");
}



INT8 GetWeaponClass( UINT16 usGun )
{
	UINT32		uiGunLevel, uiLoop;

	// always use the extended list since it contains all guns...
	if ( IsWeapon (usGun) )
	{
		for (uiGunLevel = 0; uiGunLevel <	ARMY_GUN_LEVELS; uiGunLevel++)
		{
			for ( uiLoop = 0; uiLoop < gExtendedArmyGunChoices[ uiGunLevel ].ubChoices; uiLoop++ )
			{
				if ( gExtendedArmyGunChoices[ uiGunLevel ].bItemNo[ uiLoop ] == usGun )
				{
					return( (INT8) uiGunLevel );
				}
			}
		}
	}
	return( -1 );
}



void MarkAllWeaponsOfSameGunClassAsDropped( UINT16 usWeapon )
{
	INT8 bGunClass;
	UINT32 uiLoop;

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"MarkAllWeaponsOfSameGunClassAsDropped");


	// mark that item itself as dropped, whether or not it's part of a gun class
	gStrategicStatus.fWeaponDroppedAlready[ usWeapon ] = TRUE;

	bGunClass = GetWeaponClass( usWeapon );

	// if the gun belongs to a gun class (mortars, GLs, LAWs, etc. do not and are handled independently)
	if ( bGunClass != -1 )
	{
		// then mark EVERY gun in that class as dropped
		for ( uiLoop = 0; uiLoop < gExtendedArmyGunChoices[ bGunClass ].ubChoices; uiLoop++ )
		{
			gStrategicStatus.fWeaponDroppedAlready[ gExtendedArmyGunChoices[ bGunClass ].bItemNo[ uiLoop ] ] = TRUE;
		}
	}

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"MarkAllWeaponsOfSameGunClassAsDropped done");
}



//Chooses equipment based on the relative equipment level (0-4) with best being 4.  It allocates a range
//of equipment to choose from.
//NOTE:  I'm just winging it for the decisions on which items that different groups can have.  Basically,
// there are variations, so a guy at a certain level may get a better gun and worse armour or vice versa.
void GenerateRandomEquipment( SOLDIERCREATE_STRUCT *pp, INT8 bSoldierClass, INT8 bEquipmentRating )
{
	DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("GenerateRandomEquipment"));
	OBJECTTYPE *pItem;
	//general rating information
	INT8 bRating = 0;
	//numbers of items
	INT8 bAmmoClips = 0;
	INT8 bGrenades = 0;
	BOOLEAN fAttachment = FALSE;
	//item levels
	INT8 bWeaponClass = 0;
	INT8 bHelmetClass = 0;
	INT8 bVestClass = 0;
	INT8 bLeggingClass = 0;
	INT8 bAttachClass = 0;
	INT8 bGrenadeClass = 0;
	INT8 bKnifeClass = 0;
	INT8 bKitClass = 0;
	INT8 bMiscClass = 0;
	INT8 bBombClass = 0;
	// Headrock: Added Zeroed LBE integer
	INT8 bLBEClass = 0;
	//special weapons
	BOOLEAN fMortar = FALSE;
	BOOLEAN fGrenadeLauncher = FALSE;
	BOOLEAN fLAW = FALSE;
	BOOLEAN fRPG = FALSE;
	UINT32 i;
	INT8 bEquipmentModifier;
	UINT8 ubMaxSpecialWeaponRoll;


	Assert( pp );

	// kids don't get anything 'cause they don't have any weapon animations and the rest is inappropriate
	if ( ( pp->bBodyType == HATKIDCIV ) || ( pp->bBodyType == KIDCIV ) )
	{
		return;
	}


	if ( ( pp->bBodyType == TANK_NE ) || ( pp->bBodyType == TANK_NW ) )
	{
		EquipTank( pp );
		return;
	}


	Assert( ( bSoldierClass >= SOLDIER_CLASS_NONE ) && ( bSoldierClass <= SOLDIER_CLASS_ELITE_MILITIA ) );
	Assert( ( bEquipmentRating >= 0 ) && ( bEquipmentRating <= 4 ) );


	// equipment level is modified by 1/10 of the difficulty percentage, -5, so it's between -5 to +5
	// (on normal, this is actually -4 to +4, easy is -5 to +3, and hard is -3 to +5)
	//bEquipmentModifier = bEquipmentRating + ( ( CalcDifficultyModifier( bSoldierClass ) / 10 ) - 5 );

	// Lesh 1.13 mod notes
	// My idea of enemy choice balancing is based on bEquipmentModifier, which is influenced by
	// game progress and difficulty. You can see that I throw out bEquipmentRating from the formulae.
	// This variable has a very big influence to result. Also as you can see below I commented
	// out such constants as BAD_ADMINISTRATOR_EQUIPMENT_RATING. They have big influence too,
	// maybe because of enemy choices table was incorrectly assembled. For example:
	// BAD_ADMINISTRATOR_EQUIPMENT_RATING is equal 1 and BAD_ELITE_EQUIPMENT_RATING is equal 5.
	// So in the same time when admins hold crappy weapons, elites get stuff up to nearly 6 level
	// in Drassen counter-attack. In this situation it is very hard to make balanced xml-file(IMHO).
	// Made in optional form

	// SANDRO - new behaviour of progress setting
	bEquipmentModifier = bEquipmentRating + ( ( CalcDifficultyModifier( bSoldierClass ) / 10 ) - 5 );
	switch( gGameOptions.ubProgressSpeedOfItemsChoices )
	{
		case ITEM_PROGRESS_VERY_SLOW:
			// On "Very Slow", at the beginning of the game, the modifier is -5 and goes up to -1 at +-100% game progress
			bEquipmentModifier -= (1 + (4 - (HighestPlayerProgressPercentage() / 25) ));
			break;
		case ITEM_PROGRESS_SLOW:
			// On "Slow", it's -3 until hit at least 10% game progress, then -2 till 40%, then -1 till 80%
			if (HighestPlayerProgressPercentage() < 10 )
				bEquipmentModifier -= 3;
			else if (HighestPlayerProgressPercentage() < 40 )
				bEquipmentModifier -= 2;
			else if (HighestPlayerProgressPercentage() < 80 )
				bEquipmentModifier -= 1;
			break;
		case ITEM_PROGRESS_NORMAL:
			// Nothing needed here
			break;
		case ITEM_PROGRESS_FAST:
			// On "Fast", it's just +1 at the beginning and rise up through out the game
			if (HighestPlayerProgressPercentage() < 25 )
				bEquipmentModifier += 1;
			else if (HighestPlayerProgressPercentage() < 60 )
				bEquipmentModifier += 2;
			else if (HighestPlayerProgressPercentage() < 90 )
				bEquipmentModifier += 3;
			else
				bEquipmentModifier += 4;
			break;
		case ITEM_PROGRESS_VERY_FAST:
			// "Very Fast" just make everyone loaded with top gear quickly
			if (HighestPlayerProgressPercentage() < 10 )
				bEquipmentModifier += 2;
			else if (HighestPlayerProgressPercentage() < 25 )
				bEquipmentModifier += 3;
			else if (HighestPlayerProgressPercentage() < 50 )
				bEquipmentModifier += 4;
			else if (HighestPlayerProgressPercentage() < 80 )
				bEquipmentModifier += 5;
			else
				bEquipmentModifier += 6;
			break;
		default:
			break;
	}

	switch( bSoldierClass )
	{
		case SOLDIER_CLASS_NONE:
			// ammo is here only so that civilians with pre-assigned ammo will get some clips for it!
			bAmmoClips = (INT8)(2 + Random( 2 ));

			// civilians get nothing, anyone who should get something should be preassigned by Linda
			break;

		// SANDRO - I've split the item choosing below, so the item choices for militia can be
		// handled differently than for enemies.
		case SOLDIER_CLASS_ADMINISTRATOR:
			//if ( gGameOptions.fSlowProgressForEnemyItemsChoice )
			//	bRating = bEquipmentModifier - ( bSoldierClass == SOLDIER_CLASS_ADMINISTRATOR );
			//else
			bRating = BAD_ADMINISTRATOR_EQUIPMENT_RATING + bEquipmentModifier;

			bRating = (INT8)max( MIN_EQUIPMENT_CLASS, min( MAX_EQUIPMENT_CLASS, bRating ) );

			bWeaponClass = bRating;
			//Headrocktest, remove for release
			bLBEClass = bRating;

			//Note:  in some cases the class of armour and/or helmet won't be high enough to make
			//			 the lowest level.
			bVestClass = bRating;
			bHelmetClass = bRating;
			// no leggings

			if( Chance( 30 ) )
				bKnifeClass = bRating;

			bAmmoClips = (INT8)(3 + Random( 2 ));

			if( bRating >= GOOD_ADMINISTRATOR_EQUIPMENT_RATING )
			{
				bAmmoClips++;

				bKitClass = bRating;
				bMiscClass = bRating;
				// Headrock: Low Level LBEs possible on Administrators:
				bLBEClass = bRating;
			}

			if( bRating >= GREAT_ADMINISTRATOR_EQUIPMENT_RATING )
			{
				bGrenades = 1, bGrenadeClass = bRating;
			}

			if( ( bRating > MIN_EQUIPMENT_CLASS ) && bRating < MAX_EQUIPMENT_CLASS )
			{ //Randomly decide if there is to be an upgrade of guns vs armour (one goes up, the other down)
				switch( Random( 5 ) )
				{
					case 0:	bWeaponClass++, bVestClass--;					break;	//better gun, worse armour
					case 1:	bWeaponClass--, bVestClass++;					break;	//worse gun, better armour
				}
			}
			break;
		case SOLDIER_CLASS_GREEN_MILITIA:

			bRating = BAD_ADMINISTRATOR_EQUIPMENT_RATING + bEquipmentModifier;

			///////////////////////////////////////////////////////////////////////////////////////////
			// SANDRO - Militia equipment quality bonus
			if ( gGameExternalOptions.bGreenMilitiaEquipmentQualityModifier != 0 )
				bRating += gGameExternalOptions.bGreenMilitiaEquipmentQualityModifier;
			///////////////////////////////////////////////////////////////////////////////////////////

			bRating = (INT8)max( MIN_EQUIPMENT_CLASS, min( MAX_EQUIPMENT_CLASS, bRating ) );

			bWeaponClass = bRating;
			//Headrocktest, remove for release
			//bLBEClass = bRating;

			//Note:  in some cases the class of armour and/or helmet won't be high enough to make
			//			 the lowest level.
			bVestClass = bRating;
			bHelmetClass = bRating;

			if( Chance( 15 ) )
				bKnifeClass = bRating;

			bAmmoClips = (INT8)(3 + Random( 2 ));

			if( bRating >= GOOD_ADMINISTRATOR_EQUIPMENT_RATING )
			{
				bAmmoClips++;

				bMiscClass = bRating;
			}

			if( bRating >= GREAT_ADMINISTRATOR_EQUIPMENT_RATING )
			{
				bGrenades = 1, bGrenadeClass = bRating;
			}

			if( ( bRating > MIN_EQUIPMENT_CLASS ) && bRating < MAX_EQUIPMENT_CLASS )
			{ //Randomly decide if there is to be an upgrade of guns vs armour (one goes up, the other down)
				switch( Random( 5 ) )
				{
					case 0:	bWeaponClass++, bVestClass--;					break;	//better gun, worse armour
					case 1:	bWeaponClass--, bVestClass++;					break;	//worse gun, better armour
				}
			}
			break;

		case SOLDIER_CLASS_ARMY:
			//army guys tend to have a broad range of equipment
			//if ( gGameOptions.fSlowProgressForEnemyItemsChoice )
			//	bRating = bEquipmentModifier - ( bSoldierClass == SOLDIER_CLASS_ARMY );
			//else
				bRating = BAD_ARMY_EQUIPMENT_RATING + bEquipmentModifier;
			bRating = (INT8)max( MIN_EQUIPMENT_CLASS, min( MAX_EQUIPMENT_CLASS, bRating ) );

			bWeaponClass = bRating;
			bVestClass = bRating;
			bHelmetClass = bRating;
			bGrenadeClass = bRating;
			// Headrock: Added LBE set to Coolness Rating
			bLBEClass = bRating;
			//WarmSteel - attachments don't need to be as high a class, controversional and might be better to externalize?
			bAttachClass = bRating*3/8;


			if( ( bRating >= GOOD_ARMY_EQUIPMENT_RATING ) && Chance(33) )
			{
				fAttachment = TRUE;
				bAttachClass = bRating*5/8;
			}

			bAmmoClips = (INT8)(3 + Random( 2 ));

			if( bRating >= AVERAGE_ARMY_EQUIPMENT_RATING )
			{
				bGrenades = (INT8)Random( 2 );
				bKitClass = bRating;
				bMiscClass = bRating;
			}

			if( bRating >= GOOD_ARMY_EQUIPMENT_RATING )
			{
				bGrenades++;
			}

			if( bRating >= GREAT_ARMY_EQUIPMENT_RATING )
			{
				bGrenades++;

				bLeggingClass = bRating;

				if ( Chance( 25 ) )
				{
					bBombClass = bRating;
				}
			}

			if( Chance( 35 ) )
				bKnifeClass = bRating;

			// Headrock: Chance for soldier to carry better LBE
			//if( Chance( 50 ) )
			//	bLBEClass++;

			if( ( bRating > MIN_EQUIPMENT_CLASS ) && bRating < MAX_EQUIPMENT_CLASS )
			{
				switch( Random( 7 ) )
				{
					case 3:	bWeaponClass++, bVestClass--;		break;	//better gun, worse armour
					case 4: bWeaponClass--, bVestClass++;		break;	//worse gun, better armour
					case 5: bVestClass++, bHelmetClass--;		break;	//better armour, worse helmet
					case 6: bVestClass--, bHelmetClass++;		break;	//worse armour, better helmet
				}
			}

			// if well-enough equipped, 1/5 chance of something really special
			if( ( bRating >= GREAT_ARMY_EQUIPMENT_RATING ) && ( Random( 100 ) < 20 ) )
			{
				//give this man a special weapon!  No mortars if underground, however
				ubMaxSpecialWeaponRoll = ( !IsAutoResolveActive() && ( gbWorldSectorZ != 0 ) ) ? 6 : 7;
				switch ( Random ( ubMaxSpecialWeaponRoll ) )
				{
					case 0:
					case 1:
					case 2:
						if ( pp->bExpLevel >= 3 )
						{
							//grenade launcher
							fGrenadeLauncher = TRUE;
							bGrenades = 3 + (INT8)(Random( 3 )); //3-5
						}
						break;

					case 3:
					case 4:
						if ( pp->bExpLevel >= 4 )
						{
							// LAW rocket launcher
							fLAW = TRUE;
						}
						break;
					case 5:
						if ( pp->bExpLevel >= 5 )
						{
							// RPG rocket launcher
							fRPG = TRUE;
							bGrenades = 2 + (INT8)(Random( 3 )); //2-4
							bGrenadeClass = RPG_GRENADE_CLASS;
						}
						break;

					case 6:
						// one per team maximum!
						if ( ( pp->bExpLevel >= 5 ) && ( guiMortarsRolledByTeam < MAX_MORTARS_PER_TEAM ) )
						{
							//mortar
							fMortar = TRUE;
							guiMortarsRolledByTeam++;

							// the grenades will actually represent mortar shells in this case
							bGrenades = 2 + (INT8)(Random( 3 )); //2-4
							bGrenadeClass = MORTAR_GRENADE_CLASS;
						}
						break;
				}
			}
			break;
		case SOLDIER_CLASS_REG_MILITIA:
			
			bRating = BAD_ARMY_EQUIPMENT_RATING + bEquipmentModifier;

			///////////////////////////////////////////////////////////////////////////////////////////
			// SANDRO - Militia equipment quality bonus
			if ( gGameExternalOptions.bRegularMilitiaEquipmentQualityModifier != 0 )
				bRating += gGameExternalOptions.bRegularMilitiaEquipmentQualityModifier;
			///////////////////////////////////////////////////////////////////////////////////////////

			bRating = (INT8)max( MIN_EQUIPMENT_CLASS, min( MAX_EQUIPMENT_CLASS, bRating ) );

			bWeaponClass = bRating;
			bVestClass = bRating;
			bHelmetClass = bRating;
			bGrenadeClass = bRating;
			//WarmSteel - attachments don't need to be as high a class, controversional and might be better to externalize?
			bAttachClass = bRating*3/8;


			if( ( bRating >= GOOD_ARMY_EQUIPMENT_RATING ) && Chance(33) )
			{
				fAttachment = TRUE;
				bAttachClass = bRating*5/8;
			}

			bAmmoClips = (INT8)(3 + Random( 2 ));

			if( bRating >= AVERAGE_ARMY_EQUIPMENT_RATING )
			{
				bGrenades = (INT8)Random( 2 );
				bKitClass = bRating;
				bMiscClass = bRating;
			}

			if( bRating >= GOOD_ARMY_EQUIPMENT_RATING )
			{
				bGrenades++;
			}

			if( bRating >= GREAT_ARMY_EQUIPMENT_RATING )
			{
				bGrenades++;

				bLeggingClass = bRating;

				if ( Chance( 25 ) )
				{
					bBombClass = bRating;
				}
			}

			if( Chance( 20 ) )
				bKnifeClass = bRating;

			if( ( bRating > MIN_EQUIPMENT_CLASS ) && bRating < MAX_EQUIPMENT_CLASS )
			{
				switch( Random( 7 ) )
				{
					case 3:	bWeaponClass++, bVestClass--;		break;	//better gun, worse armour
					case 4: bWeaponClass--, bVestClass++;		break;	//worse gun, better armour
					case 5: bVestClass++, bHelmetClass--;		break;	//better armour, worse helmet
					case 6: bVestClass--, bHelmetClass++;		break;	//worse armour, better helmet
				}
			}

			// if well-enough equipped, 1/5 chance of something really special
			if( ( bRating >= GREAT_ARMY_EQUIPMENT_RATING ) && ( Random( 100 ) < 20 ) )
			{
				//give this man a special weapon!  No mortars if underground, however
				ubMaxSpecialWeaponRoll = ( !IsAutoResolveActive() && ( gbWorldSectorZ != 0 ) ) ? 6 : 7;
				switch ( Random ( ubMaxSpecialWeaponRoll ) )
				{
					case 0:
					case 1:
					case 2:
						if ( pp->bExpLevel >= 3 )
						{
							//grenade launcher
							fGrenadeLauncher = TRUE;
							bGrenades = 3 + (INT8)(Random( 3 )); //3-5
						}
						break;

					case 3:
					case 4:
						if ( pp->bExpLevel >= 4 )
						{
							// LAW rocket launcher
							fLAW = TRUE;
						}
						break;
					case 5:
						if ( pp->bExpLevel >= 5 )
						{
							// RPG rocket launcher
							fRPG = TRUE;
							bGrenades = 2 + (INT8)(Random( 3 )); //2-4
							bGrenadeClass = RPG_GRENADE_CLASS;
						}
						break;
				}
			}
			break;

		case SOLDIER_CLASS_ELITE:
			//if ( gGameOptions.fSlowProgressForEnemyItemsChoice )
			//	bRating = bEquipmentModifier - ( bSoldierClass == SOLDIER_CLASS_ELITE );
			//else
				bRating = BAD_ELITE_EQUIPMENT_RATING + bEquipmentModifier;
			bRating = (INT8)max( MIN_EQUIPMENT_CLASS, min( MAX_EQUIPMENT_CLASS, bRating ) );

			bWeaponClass = bRating;
			bHelmetClass = bRating;
			bVestClass = bRating;
			bLeggingClass = bRating;
			bAttachClass = bRating*7/8;
			bGrenadeClass = bRating;
			bKitClass = bRating;
			bMiscClass = bRating;

			// Headrock: Elite LBEs
			bLBEClass = bRating;

			if ( Chance( 25 ) )
			{
				bBombClass = bRating;
			}

			bAmmoClips = (INT8)(3 + Random( 2 ));
			bGrenades = (INT8)(2 + Random( 3 ));

			if( ( bRating >= AVERAGE_ELITE_EQUIPMENT_RATING ) && Chance(75) )
			{
				fAttachment = TRUE;
				bAttachClass = bRating;
			}

			if( Chance( 35 ) )
				bKnifeClass = bRating;

			if( ( bRating > MIN_EQUIPMENT_CLASS ) && bRating < MAX_EQUIPMENT_CLASS )
			{
				UINT32 uiRange = ((UsingNewInventorySystem() == false)) ? Random(11) : Random(12);
				switch( uiRange )
				{
					case 4:		bWeaponClass++, bVestClass--;		break;
					case 5:		bWeaponClass--, bVestClass--;		break;
					case 6:		bVestClass++, bHelmetClass--;		break;
					case 7:		bGrenades += 2;						break;
					case 8:		bHelmetClass++;						break;
					case 9:		bVestClass++;						break;
					case 10:	bWeaponClass++;						break;
					case 11:	bLBEClass++;						break;
				}
			}

			// if well-enough equipped, 1/3 chance of something really special
			if( ( bRating >= GOOD_ELITE_EQUIPMENT_RATING ) && ( Random( 100 ) < 33 ) )
			{
				//give this man a special weapon!  No mortars if underground, however
				ubMaxSpecialWeaponRoll = ( !IsAutoResolveActive() && ( gbWorldSectorZ != 0 ) ) ? 6 : 7;
				switch ( Random ( ubMaxSpecialWeaponRoll ) )
				{
					case 0:
					case 1:
					case 2:
						//grenade launcher
						fGrenadeLauncher = TRUE;
						bGrenades = 4 + (INT8)(Random( 4 )); //4-7
						break;
					case 3:
					case 4:
						// LAW rocket launcher
						fLAW = TRUE;
						break;
					case 5:
						// RPG rocket launcher
						fRPG = TRUE;
						bGrenades = 3 + (INT8)(Random( 5 )); //3-7
						bGrenadeClass = RPG_GRENADE_CLASS;
						break;
					case 6:
						// one per team maximum!
						if ( guiMortarsRolledByTeam < MAX_MORTARS_PER_TEAM )
						{
							//mortar
							fMortar = TRUE;
							guiMortarsRolledByTeam++;

							// the grenades will actually represent mortar shells in this case
							bGrenades = 3 + (INT8)(Random( 5 )); //3-7
							bGrenadeClass = MORTAR_GRENADE_CLASS;
						}
						break;
				}
			}
			break;
		case SOLDIER_CLASS_ELITE_MILITIA:
			
			bRating = BAD_ELITE_EQUIPMENT_RATING + bEquipmentModifier;

			///////////////////////////////////////////////////////////////////////////////////////////
			// SANDRO - Militia equipment quality bonus
			if ( gGameExternalOptions.bVeteranMilitiaEquipmentQualityModifier != 0 )
				bRating += gGameExternalOptions.bVeteranMilitiaEquipmentQualityModifier;
			///////////////////////////////////////////////////////////////////////////////////////////

			bRating = (INT8)max( MIN_EQUIPMENT_CLASS, min( MAX_EQUIPMENT_CLASS, bRating ) );

			bWeaponClass = bRating;
			bHelmetClass = bRating;
			bVestClass = bRating;
			bLeggingClass = bRating;
			bAttachClass = bRating*7/8;
			bGrenadeClass = bRating;
			bKitClass = bRating;
			bMiscClass = bRating;

			if ( Chance( 25 ) )
			{
				bBombClass = bRating;
			}

			bAmmoClips = (INT8)(3 + Random( 2 ));
			bGrenades = (INT8)(2 + Random( 3 ));
			
			if( ( bRating >= AVERAGE_ELITE_EQUIPMENT_RATING ) && Chance(75) )
			{
				fAttachment = TRUE;
				bAttachClass = bRating;
			}

			if( Chance( 25 ) )
				bKnifeClass = bRating;

			if( ( bRating > MIN_EQUIPMENT_CLASS ) && bRating < MAX_EQUIPMENT_CLASS )
			{
				UINT32 uiRange = ((UsingNewInventorySystem() == false)) ? Random(10) : Random(11);
				switch( uiRange )
				{
					case 4:		bWeaponClass++, bVestClass--;		break;
					case 5:		bWeaponClass--, bVestClass--;		break;
					case 6:		bVestClass++, bHelmetClass--;		break;
					case 7:		bGrenades += 2;						break;
					case 8:		bHelmetClass++;						break;
					case 9:		bVestClass++;						break;
					case 10:	bWeaponClass++;						break;
					//case 11:	bLBEClass++;						break;
				}
			}

			// if well-enough equipped, 1/3 chance of something really special
			if( ( bRating >= GOOD_ELITE_EQUIPMENT_RATING ) && ( Random( 100 ) < 60 ) )
			{
				//give this man a special weapon!  No mortars if underground, however
				ubMaxSpecialWeaponRoll = ( !IsAutoResolveActive() && ( gbWorldSectorZ != 0 ) ) ? 4 : 5;
				switch ( Random ( ubMaxSpecialWeaponRoll ) )
				{
					case 0:
						//grenade launcher
						fGrenadeLauncher = TRUE;
						bGrenades = 4 + (INT8)(Random( 4 )); //4-7
						break;
					case 1:
					case 2:
						// LAW rocket launcher
						fLAW = TRUE;
						break;
					case 3:
						// RPG rocket launcher
						fRPG = TRUE;
						bGrenades = 3 + (INT8)(Random( 5 )); //3-7
						bGrenadeClass = RPG_GRENADE_CLASS;
						break;
					case 4:
						// one per team maximum!
						if ( guiMortarsRolledByTeam < MAX_MORTARS_PER_TEAM )
						{
							//mortar
							fMortar = TRUE;
							guiMortarsRolledByTeam++;

							// the grenades will actually represent mortar shells in this case
							bGrenades = 3 + (INT8)(Random( 5 )); //3-7
							bGrenadeClass = MORTAR_GRENADE_CLASS;
						}
						else
						{
							// RPG rocket launcher
							fRPG = TRUE;
							bGrenades = 3 + (INT8)(Random( 5 )); //3-7
							bGrenadeClass = RPG_GRENADE_CLASS;
						}
						break;
				}
			}
			break;
	}

	for ( i = 0; i < pp->Inv.size(); i++ )
	{ //clear items, but only if they have write status.
		if( !(pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE) )
		{
			DeleteObj(&pp->Inv[ i ]);
			//ADB this is ok, delete only inits it
			pp->Inv[ i ].fFlags |= OBJECT_UNDROPPABLE;
		}
		else
		{ //check to see what kind of item is here.  If we find a gun, for example, it'll make the
			//bWeaponClass negative to signify that a gun has already been specified, and later
			//code will use that to determine that and to pick ammo for it.
			pItem = &pp->Inv[ i ];
			if( !pItem )
				continue;
			switch( Item[ pItem->usItem ].usItemClass )
			{
				case IC_GUN:
					if ( !Item[pItem->usItem].rocketlauncher )
					{
						bWeaponClass *= -1;
					}
					else	// rocket launcher!
					{
						fLAW = FALSE;
					}
					(*pItem)[0]->data.gun.ubGunState |= GS_CARTRIDGE_IN_CHAMBER;
					break;
				case IC_AMMO:
					bAmmoClips = 0;
					break;
				case IC_BLADE:
				case IC_THROWING_KNIFE:
					bKnifeClass = 0;
					break;
				case IC_LAUNCHER:
					(*pItem)[0]->data.gun.ubGunState |= GS_CARTRIDGE_IN_CHAMBER;
					fGrenadeLauncher = FALSE;
					if (fMortar || fRPG)
					{
						bGrenadeClass = 0;
						bGrenades = 0;
					}
					fMortar = FALSE;
					fRPG = FALSE;
					break;
				case IC_ARMOUR:
					if( i == HELMETPOS )
						bHelmetClass = 0;
					else if( i == VESTPOS )
						bVestClass = 0;
					else if( i == LEGPOS )
						bLeggingClass = 0;
					break;
				case IC_GRENADE:
					bGrenades = 0;
					bGrenadeClass = 0;
					break;
				case IC_MEDKIT:
				case IC_KIT:
					bKitClass = 0;
					break;
				case IC_MISC:
					bMiscClass = 0;
				case IC_BOMB:
					bBombClass = 0;
					break;
				// Headrock: Added failsafe for LBEs
				case IC_LBEGEAR:
					bLBEClass = 0;
					break;
			}
		}
	}



	// special: militia shouldn't drop bombs
	if ( !( SOLDIER_CLASS_ENEMY( bSoldierClass ) ) )
	{
		bBombClass = 0;
	}


	//Now actually choose the equipment!
	ChooseWeaponForSoldierCreateStruct( pp, bWeaponClass, bAmmoClips, bAttachClass, fAttachment );
	// Headrock: This is where the program calls LBE choosing
	if ( SOLDIER_CLASS_ENEMY( bSoldierClass )) // BUT ONLY FOR ENEMIES, NOT MILITIA - SANDRO
	{
		if (Chance(50))	// Make EDB gear rare
			ChooseLBEsForSoldierCreateStruct( pp, bLBEClass );
	}
	ChooseSpecialWeaponsForSoldierCreateStruct( pp, bKnifeClass, fGrenadeLauncher, fLAW, fMortar, fRPG );
	ChooseGrenadesForSoldierCreateStruct( pp, bGrenades, bGrenadeClass, fGrenadeLauncher );
	ChooseArmourForSoldierCreateStruct( pp, bHelmetClass, bVestClass, bLeggingClass );
	ChooseFaceGearForSoldierCreateStruct( pp );
	if (Chance(65))
	{ ChooseKitsForSoldierCreateStruct( pp, bKitClass ); }	// Kits not so often - SANDRO
	ChooseMiscGearForSoldierCreateStruct( pp, bMiscClass );
	ChooseBombsForSoldierCreateStruct( pp, bBombClass );
	//ADB why is this here twice?
	// Headrock: This is where the program calls LBE choosing
	//ChooseLBEsForSoldierCreateStruct( pp, bLBEClass ); // SANDRO - removed duplicated 
	ChooseLocationSpecificGearForSoldierCreateStruct( pp );
	RandomlyChooseWhichItemsAreDroppable( pp, bSoldierClass );

	DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("GenerateRandomEquipment done"));
}


//When using the class values, they should all range from 0-11, 0 meaning that there will be no
//selection for that particular type of item, and 1-11 means to choose an item if possible.  1 is
//the worst class of item, while 11 is the best.

void ChooseWeaponForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp, INT8 bWeaponClass,
																				 INT8 bAmmoClips, INT8 bAttachClass, BOOLEAN fAttachment )
{
	//INVTYPE *pItem;
	UINT16 i;
	//UINT16 usRandom;
	UINT16 usGunIndex = 0;
	UINT16 usAmmoIndex = 0;
	UINT16 usAttachIndex = 0;
	UINT16 usAttachIndex2 = 0;
	UINT16 usScopeIndex = 0;
	UINT8 ubChanceStandardAmmo;
	INT8 bStatus;

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"ChooseWeaponForSoldierCreateStruct");

	// Choose weapon:
	// WEAPONS are very important, and are therefore handled differently using special pre-generated tables.
	// It was requested that enemies use only a small subset of guns with a lot duplication of the same gun type.

	// if gun was pre-selected (rcvd negative weapon class) and needs ammo
	if( bWeaponClass < 0 && bAmmoClips )
	{ //Linda has added a specific gun to the merc's inventory, but no ammo.  So, we
		//will choose ammunition that works with the gun.
		for( i = 0; i < pp->Inv.size(); i++ )
		{
			if( Item[ pp->Inv[ i ].usItem ].usItemClass == IC_GUN && pp->Inv[ i ].exists() == true)
			{
				usGunIndex = pp->Inv[ i ].usItem;
				ubChanceStandardAmmo = 100 - (bWeaponClass * -9);		// weapon class is negative!
				usAmmoIndex = RandomMagazine( usGunIndex, ubChanceStandardAmmo, max(Item[usGunIndex].ubCoolness, HighestPlayerProgressPercentage() / 10 + 3));
		
				if ( usAmmoIndex <= 0 )
					usAmmoIndex = DefaultMagazine(usGunIndex);

				pp->Inv[ i ][0]->data.gun.ubGunAmmoType = Magazine[Item[usAmmoIndex].ubClassIndex].ubAmmoType;
				pp->Inv[ i ][0]->data.gun.usGunAmmoItem = usAmmoIndex;

				if ( Item[usGunIndex].fingerprintid )
				{
					pp->Inv[ i ][0]->data.ubImprintID = (NO_PROFILE + 1);
				}

				break;
			}
		}
		if( bAmmoClips && usAmmoIndex )
		{
			CreateItems( usAmmoIndex, 100, bAmmoClips, &gTempObject );
			gTempObject.fFlags |= OBJECT_UNDROPPABLE;
			PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
		}

		return;
	}

	if (bWeaponClass < 1)
		return; //empty handed / pre-selected

	// reduce anything over 11 to 11
	if (bWeaponClass > ARMY_GUN_LEVELS)
	{
		bWeaponClass = ARMY_GUN_LEVELS;
	}


	// the weapon class here ranges from 1 to 11, so subtract 1 to get a gun level
	usGunIndex = SelectStandardArmyGun( (UINT8) (bWeaponClass - 1));

	//WarmSteel - Removed the attachment check here. why check twice anyway?
	//Now, we have chosen all of the correct items.  Now, we will assign them into the slots.
	//Because we are dealing with enemies, automatically give them full ammo in their weapon.
	switch( pp->ubSoldierClass )
	{
		case SOLDIER_CLASS_ADMINISTRATOR:
		case SOLDIER_CLASS_ARMY:
		case SOLDIER_CLASS_GREEN_MILITIA:
		case SOLDIER_CLASS_REG_MILITIA:
			//Admins/Troops: 60-75% + 1% every 4% progress
			bStatus = (INT8)(60 + Random( 16 ));
			bStatus += (INT8)(HighestPlayerProgressPercentage() / 4);
			bStatus = (INT8)min( 100, bStatus );
			break;
		case SOLDIER_CLASS_ELITE:
		case SOLDIER_CLASS_ELITE_MILITIA:
			//85-90% +  1% every 10% progress
			bStatus = (INT8)(85 + Random( 6 ));
			bStatus += (INT8)(HighestPlayerProgressPercentage() / 10);
			bStatus = (INT8)min( 100, bStatus );
			break;
		default:
			bStatus = (INT8)(50 + Random( 51 ) );
			break;
	}
	// don't allow it to be lower than marksmanship, we don't want it to affect their chances of hitting
	bStatus = (INT8)max( pp->bMarksmanship, bStatus );


	CreateItem( usGunIndex, bStatus, &(pp->Inv[ HANDPOS ]) );
	pp->Inv[ HANDPOS ].fFlags |= OBJECT_UNDROPPABLE;

	// Rocket Rifles must come pre-imprinted, in case carrier gets killed without getting a shot off
	if ( Item[usGunIndex].fingerprintid )
	{
		pp->Inv[ HANDPOS ][0]->data.ubImprintID = (NO_PROFILE + 1);
	}

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"ChooseWeaponForSoldierCreateStruct: Gun Created");
	if( !(pp->Inv[ HANDPOS ].fFlags & OBJECT_NO_OVERWRITE) )
	{ //slot not locked, so add attachments to it
		if(UsingNewAttachmentSystem()==false){
			if ( Weapon[usGunIndex].ubWeaponType == GUN_SN_RIFLE )
			{
				usScopeIndex = PickARandomAttachment(SCOPE,usGunIndex,bAttachClass,TRUE);
				if ( usScopeIndex == 0 )
				{
					// find any one that works
					usScopeIndex = PickARandomAttachment(SCOPE,usGunIndex,bAttachClass,FALSE);
				}
			}

			if( ValidItemAttachment(&(pp->Inv[ HANDPOS ]),usScopeIndex,TRUE,FALSE) )
			{
				DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("ChooseWeaponForSoldierCreateStruct: Create Scope %d",usScopeIndex));
				CreateItem( usScopeIndex, 100, &gTempObject );
				gTempObject.fFlags |= OBJECT_UNDROPPABLE;
				pp->Inv[ HANDPOS ].AttachObject( NULL, &gTempObject, FALSE );
			} else {
				usScopeIndex = 0;
			}

			//Choose attachment
			if( bAttachClass && ( fAttachment ))
				usAttachIndex = PickARandomAttachment(ATTACHMENTS,usGunIndex,bAttachClass,FALSE);

			if( ValidItemAttachment(&(pp->Inv[ HANDPOS ]),usAttachIndex,TRUE,FALSE))
			{
				DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("ChooseWeaponForSoldierCreateStruct: Create Attachment %d",usAttachIndex));
				CreateItem( usAttachIndex, 100, &gTempObject );
				gTempObject.fFlags |= OBJECT_UNDROPPABLE;
				pp->Inv[ HANDPOS ].AttachObject( NULL, &gTempObject, FALSE );
			} else {
				usAttachIndex = 0;
			}

			//check for chance of second attachment
			if ( ( bAttachClass - Item[usAttachIndex].ubCoolness ) > 0 && Random(2) )
				usAttachIndex2 = PickARandomAttachment(ATTACHMENTS,usGunIndex,bAttachClass,FALSE);


			if( ValidItemAttachment(&(pp->Inv[ HANDPOS ]),usAttachIndex2,TRUE,FALSE))
			{
				DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("ChooseWeaponForSoldierCreateStruct: Create 2nd Attachment %d",usAttachIndex2));
				CreateItem( usAttachIndex2, 100, &gTempObject );
				gTempObject.fFlags |= OBJECT_UNDROPPABLE;
				pp->Inv[ HANDPOS ].AttachObject( NULL, &gTempObject, FALSE );
			} else {
				usAttachIndex2 = 0;
			}
		} else {
			//The total pool of coolness we have for all the attachments. If fAttachment is true, the chance of getting an attachment is boosted.
			INT8 iMiscAttachmentChance = 40 + (fAttachment * 20);
			//Add a value equal to the avarage amount of attachments that will be on this gun.
			//Because the guns scope does not subtract from this value, -1.
			INT16 iAttachmentCoolnessPool = (bAttachClass * (gGameExternalOptions.iMaxEnemyAttachments-1) * iMiscAttachmentChance) / 100;

			BOOLEAN fFoundValidAttachment = FALSE;
			BOOLEAN fRandomPassed = FALSE;
			BOOLEAN fGetMatchingCoolness = FALSE;
			UINT16	usTryCount = 0;

			//When using NAS, we want more attachments.
			if ( Weapon[usGunIndex].ubWeaponType == GUN_SN_RIFLE )
			{
				usScopeIndex = PickARandomAttachment(SCOPE,usGunIndex,bAttachClass,TRUE);
				if ( usScopeIndex == 0 )
				{
					// find any one that works
					usScopeIndex = PickARandomAttachment(SCOPE,usGunIndex,bAttachClass,FALSE);
				}
			//Guns should have a fairly good chance of having a scope. Even when they're not sniper rifles.
			//They're likely to be crappier, though.
			} else if (Chance(75) && fAttachment){
				usScopeIndex = PickARandomAttachment(SCOPE,usGunIndex,bAttachClass-1,FALSE);
			}

			if( ValidItemAttachmentSlot(&(pp->Inv[ HANDPOS ]),usScopeIndex, TRUE,FALSE) )
			{
				DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("ChooseWeaponForSoldierCreateStruct: Create Scope %d",usScopeIndex));
				CreateItem( usScopeIndex, 100, &gTempObject );
				gTempObject.fFlags |= OBJECT_UNDROPPABLE;
				pp->Inv[ HANDPOS ].AttachObject( NULL, &gTempObject, FALSE );
			}

			//Loop for the maximum amount of attachments (-1 because we can already have a scope by here)
			//cnt = -1 because we start with ++ing it
			for(INT16 cnt = -1; cnt < (gGameExternalOptions.iMaxEnemyAttachments-1); ){
				usTryCount++;
				usAttachIndex = 0;

				//Our coolness pool was depleted, we won't find any more attachments anyway.
				if(iAttachmentCoolnessPool <= 0)
					break;

				//Only move on to the next attachment if 1: we found a good one. 2: randomness decided this attachment won't be. 3: We've exceeded the maximum number of tries.
				if(fFoundValidAttachment || !fRandomPassed || usTryCount > 20){
					cnt++;
					usTryCount = 0;
					fFoundValidAttachment = FALSE;

					//Determine if this attachments deserves to be.
					fRandomPassed = Chance(iMiscAttachmentChance);
					//Sometimes allow lower grade attachments.
					fGetMatchingCoolness = Chance(70);
				}

				if (fRandomPassed)
					usAttachIndex = PickARandomAttachment(ATTACHMENTS,usGunIndex,bAttachClass,fGetMatchingCoolness);

				if(!usAttachIndex){
					continue;
				}

				if( ValidItemAttachmentSlot(&(pp->Inv[ HANDPOS ]),usAttachIndex, TRUE,FALSE))
				{
					fFoundValidAttachment = TRUE;
					DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("ChooseWeaponForSoldierCreateStruct: Created Attachment %d, num: %d",usAttachIndex, cnt));
					CreateItem( usAttachIndex, 100, &gTempObject );
					gTempObject.fFlags |= OBJECT_UNDROPPABLE;
					pp->Inv[ HANDPOS ].AttachObject( NULL, &gTempObject, FALSE );
					iAttachmentCoolnessPool -= Item[usAttachIndex].ubCoolness;
				}
			}
		}
	}
	

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("ChooseWeaponForSoldierCreateStruct: Set bullets"));
	//set bullets = to magsize including any attachments (c-mag adapters, etc)
	pp->Inv[ HANDPOS ][0]->data.gun.ubGunShotsLeft = GetMagSize(&pp->Inv[ HANDPOS ]);

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("ChooseWeaponForSoldierCreateStruct: choose ammo"));
	if( bAmmoClips )
	{ //We have a gun, so choose ammo clips

		// check default ammo first...
		usAmmoIndex = DefaultMagazine( usGunIndex );
		if ( AmmoTypes[Magazine[ Item[ usAmmoIndex ].ubClassIndex ].ubAmmoType].standardIssue )
			ubChanceStandardAmmo = 80;
		else
			ubChanceStandardAmmo = 100 - (bWeaponClass * 9);

		//switch( Magazine[ Item[ usAmmoIndex ].ubClassIndex ].ubAmmoType )
		//{
		//	case AMMO_AP:
		//	case AMMO_SUPER_AP:
		//		// assault rifle, rocket rifle (etc) - high chance of having AP ammo
		//		ubChanceStandardAmmo = 80;
		//		break;
		//	default:
		//		ubChanceStandardAmmo = 100 - (bWeaponClass * 9);
		//		break;
		//}

		usAmmoIndex = RandomMagazine( &pp->Inv[HANDPOS], ubChanceStandardAmmo, max(Item[usGunIndex].ubCoolness, HighestPlayerProgressPercentage() / 10 + 3 ));

		if ( usAmmoIndex <= 0 )
			usAmmoIndex = DefaultMagazine(usGunIndex);

		pp->Inv[ HANDPOS ][0]->data.gun.ubGunAmmoType = Magazine[Item[usAmmoIndex].ubClassIndex].ubAmmoType;
		pp->Inv[ HANDPOS ][0]->data.gun.usGunAmmoItem = usAmmoIndex;
	}

	//Ammo
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("ChooseWeaponForSoldierCreateStruct: create ammo"));
	if( bAmmoClips && usAmmoIndex )
	{
		//Madd: ensure a minimum # of bullets to make sure enemies don't run out and run away
		while ( ( bAmmoClips * Weapon[usGunIndex].ubMagSize - Weapon[usGunIndex].ubMagSize) < 20 ) // each soldier should have at least 20 bullets, ie: 2 9mm 15rd clips, 3 7rd shotgun shells, 4 6rd speedloaders, etc.
		{
			bAmmoClips++;
		}

		CreateItems( usAmmoIndex, 100, bAmmoClips, &gTempObject );
		gTempObject.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
	}

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("ChooseWeaponForSoldierCreateStruct: done"));
}

void ChooseGrenadesForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp, INT8 bGrenades, INT8 bGrenadeClass, BOOLEAN fGrenadeLauncher )
{
	INT16 sNumPoints;
	UINT16 usItem;
	UINT8 ubBaseQuality;
	UINT8 ubQualityVariation;
	//numbers of each type the player will get!
	UINT8 count = 0;

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"ChooseGrenadesForSoldierCreateStruct");

	//determine how many *points* the enemy will get to spend on grenades...
	sNumPoints = bGrenades * bGrenadeClass;

	//no points, no grenades!
	if( !sNumPoints )
		return;

	// special mortar shell handling
	if (bGrenadeClass == MORTAR_GRENADE_CLASS)
	{
		// 0verhaul:  itemMortar can be 0 if the previous function
		// 1) failed to find a Mortar that the soldier would want to use (due to XML prefs)
		// 2) randomly chose not to supply a Mortar (always possible, even if choices exist).
		// Since we should not go beyond this point in the case where Mortar Rounds are desired,
		// return here in any case
		if (itemMortar > 0 )
		{
			usItem = PickARandomLaunchable ( itemMortar );
			if ( usItem > 0 )
			{
				CreateItems( usItem, (INT8) (80 + Random(21)), bGrenades, &gTempObject );
				gTempObject.fFlags |= OBJECT_UNDROPPABLE;
				PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
			}
		}

		return;
	}

	// special rpg rocket handling
	if (bGrenadeClass == RPG_GRENADE_CLASS)
	{
		// 0verhaul:  itemRPG can be 0 if the previous function
		// 1) failed to find an RPG that the soldier would want to use (due to XML prefs)
		// 2) randomly chose not to supply an RPG (always possible, even if choices exist).
		// Since we should not go beyond this point in the case where RPG grenades are desired,
		// return here in any case
		if (itemRPG > 0 )
		{
			usItem = PickARandomLaunchable ( itemRPG );
			if ( usItem > 0 )
			{
				for ( int i = 0; i < bGrenades; i++ )
				{
					CreateItem( usItem, (INT8) (70 + Random(31)), &gTempObject ); 
					gTempObject.fFlags |= OBJECT_UNDROPPABLE;
					PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
				}

			}
		}
		return;
	}

	Assert( bGrenadeClass <= 11 );

	//determine the quality of grenades.  The elite guys get the best quality, while the others
	//get progressively worse.
	ubBaseQuality = (UINT8)min( 45 + bGrenadeClass * 5, 90 );
	ubQualityVariation = 101 - ubBaseQuality;

	// Madd: GL guys don't get hand grenades anymore
	if ( itemGrenadeLauncher > 0 )
	{
		//do this for every 1-2 grenades so that we can get more variety
		while ( bGrenades > 0 )
		{
			count = Random(3);
			if ( count > bGrenades )
				count = bGrenades;

			usItem = PickARandomLaunchable ( itemGrenadeLauncher );
			if ( usItem > 0 && count > 0 )
			{
				CreateItems( usItem, (INT8)(ubBaseQuality + Random( ubQualityVariation )), count, &gTempObject );
				gTempObject.fFlags |= OBJECT_UNDROPPABLE;
				PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
			}
			bGrenades -= count;
		}

		return;
	}


	//Madd: screw the original code; it's impossible to externalize and too complicated anyway
	//do this for every 1-2 grenades so that we can get more variety
	while ( bGrenades > 0 )
	{
		count = Random(3);
		if ( count > bGrenades )
			count = bGrenades;

		usItem = PickARandomItem ( GRENADE , bGrenadeClass, FALSE );
		if ( usItem > 0 && count > 0 )
		{
			CreateItems( usItem, (INT8)(ubBaseQuality + Random( ubQualityVariation )), count, &gTempObject );
			gTempObject.fFlags |= OBJECT_UNDROPPABLE;
			PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
		}
		bGrenades -= count;
	}
	return;

#ifdef obsoleteCode
	////now, purchase the grenades.
	//while( sNumPoints > 0 )
	//{
	//	if( sNumPoints >= 20 )
	//	{ //Choose randomly between mustard and regular
	//		if( Random( 2 ) && !fGrenadeLauncher )
	//			ubNumMustard++, sNumPoints -= 10;
	//		else
	//			ubNumReg++, sNumPoints -= 9;
	//	}

	//	if( sNumPoints >= 10 )
	//	{ //Choose randomly between any
	//		switch( Random( 7 ) )
	//		{
	//			case 0:	if ( !fGrenadeLauncher )
	//							{
	//								ubNumMustard++;		sNumPoints -= 10;	break;
	//							}
	//							// if grenade launcher, pick regular instead
	//			case 1: ubNumReg++;				sNumPoints -= 9;		break;
	//			case 2: if ( !fGrenadeLauncher )
	//							{
	//								ubNumMini++;			sNumPoints -= 7;		break;
	//							}
	//							// if grenade launcher, pick tear instead
	//			case 3: ubNumTear++;			sNumPoints -= 6;		break;
	//			case 4: ubNumStun++;			sNumPoints -= 5;		break;
	//			case 5: ubNumSmoke++;			sNumPoints -= 4;		break;
	//			case 6: if (!fGrenadeLauncher )
	//					{
	//						ubNumFlare++;			sNumPoints -= 3;
	//					}
	//					break;
	//		}
	//	}

	//	// JA2 Gold: don't make mini-grenades take all points available, and add chance of break lights
	//	if( sNumPoints >= 1 && sNumPoints < 10 )
	//	{
	//		switch( Random( 10 ) )
	//		{
	//			case 0:
	//			case 1:
	//			case 2:
	//				ubNumSmoke++;
	//				sNumPoints -= 4;
	//				break;
	//			case 3:
	//			case 4:
	//				ubNumTear++;
	//				sNumPoints -= 6;
	//				break;
	//			case 5:
	//			case 6:
	//				if (!fGrenadeLauncher)
	//				{
	//					ubNumFlare++;
	//					sNumPoints -= 3;
	//				}
	//				break;
	//			case 7:
	//			case 8:
	//				ubNumStun++;
	//				sNumPoints -= 5;
	//				break;
	//			case 9:
	//				if (!fGrenadeLauncher)
	//				{
	//					ubNumMini++;
	//					sNumPoints -= 7;
	//				}
	//				break;
	//		}
	//	}
	//	/*
	//	if( usNumPoints >= 1 && usNumPoints < 10 )
	//	{ //choose randomly between either stun or tear, (normal with rare chance)
	//		switch( Random( 10 ) )
	//		{
	//			case 0:
	//			case 1:
	//			case 2:
	//			case 3:
	//				ubNumSmoke++;
	//				if( usNumPoints > 4 )
	//					usNumPoints -= 4;
	//				else
	//					usNumPoints = 0;
	//				break;
	//			case 4:
	//			case 5:
	//			case 6:
	//				ubNumTear++;
	//				if( usNumPoints > 6 )
	//					usNumPoints -= 6;
	//				else
	//					usNumPoints = 0;
	//				break;
	//			case 7:
	//			case 8:
	//				ubNumStun++;
	//				if( usNumPoints > 5 )
	//					usNumPoints -= 5;
	//				else
	//					usNumPoints = 0;
	//				break;
	//			case 9:
	//				ubNumMini++;
	//				usNumPoints = 0;
	//				break;
	//		}
	//	}
	//	*/
	//}


	////Create the grenades and add them to the soldier

	//if( ubNumSmoke )
	//{
	//	if ( fGrenadeLauncher )
	//	{
	//		usItem = GL_SMOKE_GRENADE;
	//	}
	//	else
	//	{
	//		usItem = SMOKE_GRENADE;
	//	}
	//	CreateItems( usItem, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumSmoke, &gTempObject );
	//	gTempObject.fFlags |= OBJECT_UNDROPPABLE;
	//	PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
	//}
	//if( ubNumTear )
	//{
	//	if ( fGrenadeLauncher )
	//	{
	//		usItem = GL_TEARGAS_GRENADE;
	//	}
	//	else
	//	{
	//		usItem = TEARGAS_GRENADE;
	//	}
	//	CreateItems( usItem, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumTear, &gTempObject );
	//	gTempObject.fFlags |= OBJECT_UNDROPPABLE;
	//	PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
	//}
	//if( ubNumStun )
	//{
	//	if ( fGrenadeLauncher )
	//	{
	//		usItem = GL_STUN_GRENADE;
	//	}
	//	else
	//	{
	//		usItem = STUN_GRENADE;
	//	}
	//	CreateItems( usItem, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumStun, &gTempObject );
	//	gTempObject.fFlags |= OBJECT_UNDROPPABLE;
	//	PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
	//}
	//if( ubNumReg )
	//{
	//	if ( fGrenadeLauncher )
	//	{
	//		usItem = GL_HE_GRENADE;
	//	}
	//	else
	//	{
	//		usItem = HAND_GRENADE;
	//	}
	//	CreateItems( usItem, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumReg, &gTempObject );
	//	gTempObject.fFlags |= OBJECT_UNDROPPABLE;
	//	PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
	//}

	//if( ubNumMini )
	//{
	//	CreateItems( MINI_GRENADE, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumMini, &gTempObject );
	//	gTempObject.fFlags |= OBJECT_UNDROPPABLE;
	//	PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
	//}
	//if( ubNumMustard )
	//{
	//	CreateItems( MUSTARD_GRENADE, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumMustard, &gTempObject );
	//	gTempObject.fFlags |= OBJECT_UNDROPPABLE;
	//	PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
	//}
	//if( ubNumFlare )
	//{
	//	CreateItems( BREAK_LIGHT, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumFlare, &gTempObject );
	//	gTempObject.fFlags |= OBJECT_UNDROPPABLE;
	//	PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
	//}
#endif //obsoleteCode
}

void ChooseArmourForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp, INT8 bHelmetClass, INT8 bVestClass, INT8 bLeggingsClass )
{
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"ChooseArmourForSoldierCreateStruct");
	//UINT16 i;
	//INVTYPE *pItem;
	//UINT16 usRandom;
	UINT16 usItem = 0, usHelmetItem = 0, usVestItem = 0, usLeggingsItem = 0;
	//UINT16 usNumMatches;
	//INT8 bOrigVestClass = bVestClass;
	INT8 i;

	//tais: always get any armor... smeagol doesnt like naked people..
	if(gGameExternalOptions.fSoldiersWearAnyArmour)
	{
		if(bHelmetClass < 1) bHelmetClass = 1;
		//search for a non-empty class with items we need
		for(i=bHelmetClass;i<=10;i++)
		{
			usHelmetItem = PickARandomItem(HELMET,i );
			//if we find a non-empty class change to that and break
			if(usHelmetItem > 0)
			{
				bHelmetClass = i;
				break;
			}
		}
		if(bVestClass < 1) bVestClass = 1;
		//search for a non-empty class with items we need
		for(i=bVestClass;i<=10;i++)
		{
			usVestItem = PickARandomItem(VEST,i );
			//if we find a non-empty class change to that and break
			if(usVestItem > 0)
			{
				bVestClass = i;
				break;
			}
		}
		if(bLeggingsClass < 1) bLeggingsClass = 1;
		//search for a non-empty class with items we need
		for(i=bLeggingsClass;i<=10;i++)
		{
			usLeggingsItem = PickARandomItem(LEGS,i );
			//if we find a non-empty class change to that and break
			if(usLeggingsItem > 0)
			{
				bLeggingsClass = i;
				break;
			}
		}
	}

	//Madd: added minimum protection of 10 for armours to be used by enemies

	//Choose helmet
	if( bHelmetClass )
	{
		if(!gGameExternalOptions.fSoldiersWearAnyArmour) usHelmetItem = PickARandomItem(HELMET,bHelmetClass );
		if ( usHelmetItem > 0 && Item[usHelmetItem].usItemClass == IC_ARMOUR && !(pp->Inv[ HELMETPOS ].fFlags & OBJECT_NO_OVERWRITE) && Armour[ Item[usHelmetItem].ubClassIndex ].ubArmourClass == ARMOURCLASS_HELMET )
		{
			CreateItem( usHelmetItem, (INT8)(70+Random(31)), &(pp->Inv[ HELMETPOS ]) );
			pp->Inv[ HELMETPOS ].fFlags |= OBJECT_UNDROPPABLE;

			// roll to see if he gets an attachment, too.  Higher chance the higher his entitled helmet class is
			if (( INT8 ) Random( 100 ) < ( 15 * ( bHelmetClass - Item[usHelmetItem].ubCoolness ) ) )
			{
				UINT16 usAttachment = PickARandomAttachment(ARMOURATTACHMENT,usHelmetItem, bHelmetClass, FALSE);
				if ( usAttachment > 0 )
				{
					CreateItem( usAttachment, (INT8)(70+Random(31)), &gTempObject );
					gTempObject.fFlags |= OBJECT_UNDROPPABLE;
					pp->Inv[ HELMETPOS ].AttachObject( NULL, &gTempObject, FALSE );
				}
			}

		}

#ifdef obsoleteCode
		//usNumMatches = 0;
		//while( bHelmetClass && !usNumMatches )
		//{ //First step is to count the number of helmets in the helmet class range.  If we
		//	//don't find one, we keep lowering the class until we do.
		//	for( i = 0; i < MAXITEMS; i++ )
		//	{
		//		if ( Item[i].usItemClass  == 0 )
		//			break;
		//		pItem = &Item[ i ];
		//		// NOTE: This relies on treated armor to have a coolness of 0 in order for enemies not to be equipped with it
		//		if( pItem->usItemClass == IC_ARMOUR && pItem->ubCoolness == bHelmetClass )
		//		{
		//			if( Armour[ pItem->ubClassIndex ].ubArmourClass == ARMOURCLASS_HELMET && Armour[ pItem->ubClassIndex ].ubProtection >= 10 )
		//				usNumMatches++;
		//		}
		//	}
		//	if( !usNumMatches )
		//		bHelmetClass--;
		//}
		//if( usNumMatches )
		//{ //There is a helmet that we can choose.
		//	usRandom = (UINT16)Random( usNumMatches );
		//	for( i = 0; i < MAXITEMS; i++ )
		//	{
		//		if ( Item[i].usItemClass  == 0 )
		//			break;
		//		pItem = &Item[ i ];
		//		if( pItem->usItemClass == IC_ARMOUR && pItem->ubCoolness == bHelmetClass )
		//		{
		//			if( Armour[ pItem->ubClassIndex ].ubArmourClass == ARMOURCLASS_HELMET && Armour[ pItem->ubClassIndex ].ubProtection >= 10 )
		//			{
		//				if( usRandom )
		//					usRandom--;
		//				else
		//				{
		//					if( !(pp->Inv[ HELMETPOS ].fFlags & OBJECT_NO_OVERWRITE) )
		//					{
		//						CreateItem( i, (INT8)(70+Random(31)), &(pp->Inv[ HELMETPOS ]) );
		//						pp->Inv[ HELMETPOS ].fFlags |= OBJECT_UNDROPPABLE;
		//					}
		//					break;
		//				}
		//			}
		//		}
		//	}
		//}
#endif //obsoleteCode
	}

	//Choose vest
	if( bVestClass )
	{
		if(!gGameExternalOptions.fSoldiersWearAnyArmour) usVestItem = PickARandomItem(VEST,bVestClass );
		if ( usVestItem > 0 && Item[usVestItem].usItemClass == IC_ARMOUR && !(pp->Inv[ VESTPOS ].fFlags & OBJECT_NO_OVERWRITE) && Armour[ Item[usVestItem].ubClassIndex ].ubArmourClass == ARMOURCLASS_VEST )
		{
			CreateItem( usVestItem, (INT8)(70+Random(31)), &(pp->Inv[ VESTPOS ]) );
			pp->Inv[ VESTPOS ].fFlags |= OBJECT_UNDROPPABLE;

			// roll to see if he gets a CERAMIC PLATES, too.  Higher chance the higher his entitled vest class is
			if (( INT8 ) Random( 100 ) < ( 15 * ( bVestClass - Item[usVestItem].ubCoolness ) ) )
			{
				UINT16 usAttachment = PickARandomAttachment(ARMOURATTACHMENT,usVestItem, bVestClass, FALSE);
				if ( usAttachment > 0 )
				{
					CreateItem( usAttachment, (INT8)(70+Random(31)), &gTempObject );
					gTempObject.fFlags |= OBJECT_UNDROPPABLE;
					pp->Inv[ VESTPOS ].AttachObject( NULL, &gTempObject, FALSE );
				}
			}

		}

#ifdef obsoleteCode
		//usNumMatches = 0;
		//while( bVestClass && !usNumMatches )
		//{ //First step is to count the number of armours in the armour class range.  If we
		//	//don't find one, we keep lowering the class until we do.
		//	for( i = 0; i < MAXITEMS; i++ )
		//	{
		//		if ( Item[i].usItemClass  == 0 )
		//			break;
		//		// these 3 have a non-zero coolness, and would otherwise be selected, so skip them
		//		if ( Item[i].leatherjacket)
		//			continue;

		//		pItem = &Item[ i ];
		//		// Madd: don't pick crappy armour like t-shirts, must be a minimum protection of ten
		//		// NOTE: This relies on treated armor to have a coolness of 0 in order for enemies not to be equipped with it
		//		if( pItem->usItemClass == IC_ARMOUR && pItem->ubCoolness == bVestClass )
		//		{
		//			if( Armour[ pItem->ubClassIndex ].ubArmourClass == ARMOURCLASS_VEST && Armour[pItem->ubClassIndex].ubProtection >= 10 )
		//				usNumMatches++;
		//		}
		//	}
		//	if( !usNumMatches )
		//		bVestClass--;
		//}
		//if( usNumMatches )
		//{ //There is an armour that we can choose.
		//	usRandom = (UINT16)Random( usNumMatches );
		//	for( i = 0; i < MAXITEMS; i++ )
		//	{
		//		if ( Item[i].usItemClass  == 0 )
		//			break;

		//		pItem = &Item[ i ];
		//		if( pItem->usItemClass == IC_ARMOUR && pItem->ubCoolness == bVestClass )
		//		{
		//			if ( Item[i].leatherjacket )
		//				continue;

		//			// Madd: don't pick crappy armour like t-shirts, must be a minimum protection of ten
		//			if( Armour[ pItem->ubClassIndex ].ubArmourClass == ARMOURCLASS_VEST && Armour[ pItem->ubClassIndex ].ubProtection >= 10 )
		//			{
		//				if( usRandom )
		//					usRandom--;
		//				else
		//				{
		//					if( !(pp->Inv[ VESTPOS ].fFlags & OBJECT_NO_OVERWRITE) )
		//					{
		//						CreateItem( i, (INT8)(70+Random(31)), &(pp->Inv[ VESTPOS ]) );
		//						pp->Inv[ VESTPOS ].fFlags |= OBJECT_UNDROPPABLE;

		//						// roll to see if he gets a CERAMIC PLATES, too.  Higher chance the higher his entitled vest class is
		//						if (( INT8 ) Random( 100 ) < ( 15 * ( bOrigVestClass - pItem->ubCoolness ) ) )
		//						{
		//							UINT16 usAttachment = PickARandomItem(ARMOURATTACHMENT, bOrigVestClass);
		//							if ( usAttachment > 0 && ValidAttachment(usAttachment, i ) )
		//							{
		//								CreateItem( usAttachment, (INT8)(70+Random(31)), &gTempObject );
		//								gTempObject.fFlags |= OBJECT_UNDROPPABLE;
		//								AttachObject( NULL, &(pp->Inv[ VESTPOS ]), &gTempObject );
		//							}
		//						}
		//					}
		//					break;
		//				}
		//			}
		//		}
		//	}
		//}
#endif //obsoleteCode
	}
	//Choose Leggings
	if( bLeggingsClass )
	{
		if(!gGameExternalOptions.fSoldiersWearAnyArmour) usLeggingsItem = PickARandomItem(LEGS,bLeggingsClass );
		if ( usLeggingsItem > 0 && Item[usLeggingsItem].usItemClass == IC_ARMOUR && !(pp->Inv[ LEGPOS ].fFlags & OBJECT_NO_OVERWRITE) && Armour[ Item[usLeggingsItem].ubClassIndex ].ubArmourClass == ARMOURCLASS_LEGGINGS )
		{
			CreateItem( usLeggingsItem, (INT8)(70+Random(31)), &(pp->Inv[ LEGPOS ]) );
			pp->Inv[ LEGPOS ].fFlags |= OBJECT_UNDROPPABLE;

			// roll to see if he gets an attachment, too.  Higher chance the higher his entitled Leggings class is
			if (( INT8 ) Random( 100 ) < ( 15 * ( bLeggingsClass - Item[usLeggingsItem].ubCoolness ) ) )
			{
				UINT16 usAttachment = PickARandomAttachment(ARMOURATTACHMENT,usLeggingsItem, bLeggingsClass, FALSE);
				if ( usAttachment > 0 )
				{
					CreateItem( usAttachment, (INT8)(70+Random(31)), &gTempObject );
					gTempObject.fFlags |= OBJECT_UNDROPPABLE;
					pp->Inv[ LEGPOS ].AttachObject( NULL, &gTempObject, FALSE);
				}
			}

		}


#ifdef obsoleteCode
		//usNumMatches = 0;
		//while( bLeggingsClass && !usNumMatches )
		//{ //First step is to count the number of Armours in the Armour class range.  If we
		//	//don't find one, we keep lowering the class until we do.
		//	for( i = 0; i < MAXITEMS; i++ )
		//	{
		//		if ( Item[i].usItemClass  == 0 )
		//			break;
		//		pItem = &Item[ i ];
		//		// NOTE: This relies on treated armor to have a coolness of 0 in order for enemies not to be equipped with it
		//		if( pItem->usItemClass == IC_ARMOUR && pItem->ubCoolness == bLeggingsClass )
		//		{
		//			if( Armour[ pItem->ubClassIndex ].ubArmourClass == ARMOURCLASS_LEGGINGS && Armour[ pItem->ubClassIndex ].ubProtection >= 10 )
		//				usNumMatches++;
		//		}
		//	}
		//	if( !usNumMatches )
		//		bLeggingsClass--;
		//}
		//if( usNumMatches )
		//{ //There is a legging that we can choose.
		//	usRandom = (UINT16)Random( usNumMatches );
		//	for( i = 0; i < MAXITEMS; i++ )
		//	{
		//		if ( Item[i].usItemClass  == 0 )
		//			break;
		//		pItem = &Item[ i ];
		//		if( pItem->usItemClass == IC_ARMOUR && pItem->ubCoolness == bLeggingsClass )
		//		{
		//			if( Armour[ pItem->ubClassIndex ].ubArmourClass == ARMOURCLASS_LEGGINGS && Armour[ pItem->ubClassIndex ].ubProtection >= 10 )
		//			{
		//				if( usRandom )
		//					usRandom--;
		//				else
		//				{
		//					if( !(pp->Inv[ LEGPOS ].fFlags & OBJECT_NO_OVERWRITE) )
		//					{
		//						CreateItem( i, (INT8)(70+Random(31)), &(pp->Inv[ LEGPOS ]) );
		//						pp->Inv[ LEGPOS ].fFlags |= OBJECT_UNDROPPABLE;
		//						break;
		//					}
		//				}
		//			}
		//		}
		//	}
		//}
#endif //obsoleteCode
	}
}

void ChooseSpecialWeaponsForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp, INT8 bKnifeClass, BOOLEAN fGrenadeLauncher, BOOLEAN fLAW, BOOLEAN fMortar, BOOLEAN fRPG )
{
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"ChooseSpecialWeaponsForSoldierCreateStruct");
	//UINT16 i;
	//INVTYPE *pItem;
	//UINT16 usRandom;
	UINT16 usKnifeIndex = 0;

	itemGrenadeLauncher = 0;
	itemLAW = 0;
	itemRPG = 0;
	itemMortar=0;

#ifdef obsoleteCode
	//Choose knife
	//while( bKnifeClass && !usNumMatches )
	//{ //First step is to count the number of weapons in the weapon class range.  If we
	//	//don't find one, we keep lowering the class until we do.
	//	for( i = 0; i < MAXITEMS; i++ )
	//	{
	//		if ( Item[i].usItemClass  == 0 )
	//			break;
	//		pItem = &Item[ i ];
	//		if( ( pItem->usItemClass == IC_BLADE || pItem->usItemClass == IC_THROWING_KNIFE ) && pItem->ubCoolness == bKnifeClass )
	//		{
	//			usNumMatches++;
	//		}
	//	}
	//	if( !usNumMatches )
	//		bKnifeClass--;
	//}
	//if( usNumMatches )
	//{ //There is a knife that we can choose.
	//	usRandom = (UINT16)Random( usNumMatches );
	//	for( i = 0; i < MAXITEMS; i++ )
	//	{
	//		if ( Item[i].usItemClass  == 0 )
	//			break;
	//		pItem = &Item[ i ];
	//		if( ( pItem->usItemClass == IC_BLADE || pItem->usItemClass == IC_THROWING_KNIFE ) && pItem->ubCoolness == bKnifeClass )
	//		{
	//			if( usRandom )
	//				usRandom--;
	//			else
	//			{
	//				usKnifeIndex = i;
	//				break;
	//			}
	//		}
	//	}
	//}
#endif //obsoleteCode

	if ( bKnifeClass )
	{
		usKnifeIndex = PickARandomItem(KNIVES,bKnifeClass);

		if( usKnifeIndex > 0 )
		{
			CreateItem( usKnifeIndex, (INT8)(70 + Random( 31 )), &gTempObject );
			gTempObject.fFlags |= OBJECT_UNDROPPABLE;
			PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
		}
	}


////test code
//		CreateItem( 347, (INT8)(50 + Random( 51 )), &gTempObject );
//		PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
//
//		CreateItem( 298, (INT8)(50 + Random( 51 )), &gTempObject );
//		PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
//
//		CreateItem( 298, (INT8)(50 + Random( 51 )), &gTempObject );
//		PlaceObjectInSoldierCreateStruct( pp, &gTempObject );

	if ( IsGrenadeLauncherAttached(&pp->Inv[HANDPOS]) ) //Madd: if there's a grenade launcher attached to the gun, then use that instead of giving him another one
	{
		itemGrenadeLauncher = GetAttachedGrenadeLauncher(&pp->Inv[HANDPOS]);
	}
	else if (fGrenadeLauncher)
	{
		itemGrenadeLauncher = PickARandomItem ( GRENADELAUNCHER );
		// give grenade launcher
		if ( itemGrenadeLauncher > 0 )
		{
			CreateItem( itemGrenadeLauncher, (INT8)(50 + Random( 51 )), &gTempObject );
			gTempObject.fFlags |= OBJECT_UNDROPPABLE;
			PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
		}
	}

	if (fLAW)
	{
		// give rocket launcher
		itemLAW = PickARandomItem (SINGLESHOTROCKETLAUNCHER);
		if ( itemLAW > 0 )
		{
			CreateItem( itemLAW, (INT8)(50 + Random( 51 )), &gTempObject );
			gTempObject.fFlags |= OBJECT_UNDROPPABLE;
			PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
		}
	}
	if (fRPG)
	{
		// give rpg
		itemRPG = PickARandomItem (ROCKETLAUNCHER);
		if ( itemRPG > 0 )
		{
			CreateItem( itemRPG, (INT8)(50 + Random( 51 )), &gTempObject );
			gTempObject.fFlags |= OBJECT_UNDROPPABLE;
			PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
		}
	}

	if (fMortar)
	{
		// make sure we're not distributing them underground!
		Assert( IsAutoResolveActive() || ( gbWorldSectorZ == 0 ) );

		// give mortar
		itemMortar = PickARandomItem (MORTARLAUNCHER);
		if ( itemMortar > 0 )
		{
			CreateItem( itemMortar, (INT8)(50 + Random( 51 )), &gTempObject );
			gTempObject.fFlags |= OBJECT_UNDROPPABLE;
			PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
		}
	}


}


void ChooseFaceGearForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp )
{
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"ChooseFaceGearForSoldierCreateStruct");
	INT32 i;
	INT8 bDifficultyRating = CalcDifficultyModifier( pp->ubSoldierClass );
	UINT16 usItem=0;

	if ( gWorldSectorX == TIXA_SECTOR_X && gWorldSectorY == TIXA_SECTOR_Y && StrategicMap[ TIXA_SECTOR_X + TIXA_SECTOR_Y * MAP_WORLD_X ].fEnemyControlled )
	{ //Tixa is a special case that is handled differently.
		return;
	}

	//Look for any face item in the big pocket positions (the only place they can be added in the editor)
	//If any are found, then don't assign any
	// CHRISL: Change static inventory pocket definition to dynamic
	for( i = BIGPOCKSTART; i < BIGPOCKFINAL; i++ )
	{
		if( Item[ pp->Inv[ i ].usItem ].usItemClass == IC_FACE )
		{
			return;
		}
	}

	//KM: (NEW)
	//Note the lack of overwritable item checking here.  This is because faceitems are not
	//supported in the editor, hence they can't have this status.
	switch( pp->ubSoldierClass )
	{
		case SOLDIER_CLASS_ELITE:
		case SOLDIER_CLASS_ELITE_MILITIA:
			if ( Chance( bDifficultyRating ) )
			{
				//All elites get a gasmask and either night goggles or uv goggles.
				if( Chance( 75 ) )
				{
					usItem = PickARandomItem(GASMASKS);
					if ( usItem > 0 )
					{
						CreateItem( usItem, (INT8)(70+Random(31)), &(pp->Inv[ HEAD1POS ]) );
						pp->Inv[ HEAD1POS ].fFlags |= OBJECT_UNDROPPABLE;
					}
				}
				else
				{
					usItem = PickARandomItem(HEARINGAIDS);
					if ( usItem > 0 )
					{
						CreateItem( usItem, (INT8)(70+Random(31)), &(pp->Inv[ HEAD1POS ]) );
						pp->Inv[ HEAD1POS ].fFlags |= OBJECT_UNDROPPABLE;
					}
				}
			}
			if ( Chance( bDifficultyRating / 3 ) )
			{
				if( Chance( 75 ) )
				{
					usItem = PickARandomItem(NVGLOW);
					if ( usItem > 0 )
					{
						CreateItem( usItem, (INT8)(70+Random(31)), &(pp->Inv[ HEAD2POS ]) );
						pp->Inv[ HEAD2POS ].fFlags |= OBJECT_UNDROPPABLE;
					}
				}
				else
				{
					usItem = PickARandomItem(NVGHIGH);
					if ( usItem > 0 )
					{
						CreateItem( usItem, (INT8)(70+Random(31)), &(pp->Inv[ HEAD2POS ]) );
						pp->Inv[ HEAD2POS ].fFlags |= OBJECT_UNDROPPABLE;
					}
				}
			}
			break;
		case SOLDIER_CLASS_ARMY:
		case SOLDIER_CLASS_REG_MILITIA:
			if( Chance( bDifficultyRating / 2 ) )
			{ //chance of getting a face item
				if( Chance( 50 ) )
				{
					usItem = PickARandomItem(GASMASKS);
					if ( usItem > 0 )
					{
						CreateItem( usItem, (INT8)(70+Random(31)), &(pp->Inv[ HEAD1POS ]) );
						pp->Inv[ HEAD1POS ].fFlags |= OBJECT_UNDROPPABLE;
					}
				}
				else
				{
					usItem = PickARandomItem(NVGLOW);
					if ( usItem > 0 )
					{
						CreateItem( usItem, (INT8)(70+Random(31)), &(pp->Inv[ HEAD1POS ]) );
						pp->Inv[ HEAD1POS ].fFlags |= OBJECT_UNDROPPABLE;
					}
				}
			}
			if( Chance( bDifficultyRating / 3 ) )
			{ //chance of getting a extended ear
				usItem = PickARandomItem(HEARINGAIDS);
				if ( usItem > 0 )
				{
					CreateItem( usItem, (INT8)(70+Random(31)), &(pp->Inv[ HEAD2POS ]) );
					pp->Inv[ HEAD2POS ].fFlags |= OBJECT_UNDROPPABLE;
				}
			}
			break;
		case SOLDIER_CLASS_ADMINISTRATOR:
		case SOLDIER_CLASS_GREEN_MILITIA:
			break;
	}
}



void ChooseKitsForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp, INT8 bKitClass )
{
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"ChooseKitsForSoldierCreateStruct");
	//UINT16 i;
	//INVTYPE *pItem;
	//UINT16 usRandom;
	//UINT16 usNumMatches = 0;
	UINT16 usKitItem = 0;

	//Madd: specify kit items in xml instead

#ifdef obsoleteCode
	// we want these mostly to be first aid and medical kits, and for those kit class doesn't matter, they're always useful
	//if ( Chance( 50 ) )
	//{
	//	usKitItem = FIRSTAIDKIT;
	//}
	//else
	//if ( Chance( 25 ) )
	//{
	//	usKitItem = MEDICKIT;
	//}
	//else
	//{
	//	// count how many non-medical KITS are eligible ( of sufficient or lower coolness)
	//	for( i = 0; i < MAXITEMS; i++ )
	//	{
	//		if ( Item[i].usItemClass  == 0 )
	//			break;
	//		pItem = &Item[ i ];
	//		// skip toolkits
	//		if( ( pItem->usItemClass == IC_KIT ) && ( pItem->ubCoolness > 0 ) && pItem->ubCoolness <= bKitClass && ( !Item[i].toolkit ) )
	//		{
	//			usNumMatches++;
	//		}
	//	}

	//	// if any are eligible, pick one of them at random
	//	if( usNumMatches )
	//	{
	//		usRandom = (UINT16)Random( usNumMatches );
	//		for( i = 0; i < MAXITEMS; i++ )
	//		{
	//			if ( Item[i].usItemClass  == 0 )
	//				break;
	//			pItem = &Item[ i ];
	//			// skip toolkits
	//			if( ( pItem->usItemClass == IC_KIT ) && ( pItem->ubCoolness > 0 ) && pItem->ubCoolness <= bKitClass && ( !Item[i].toolkit ) )
	//			{
	//				if( usRandom )
	//					usRandom--;
	//				else
	//				{
	//					usKitItem = i;
	//					break;
	//				}
	//			}
	//		}
	//	}
	//}
#endif //obsoleteCode

	// if we still want more medical and first aid kits, we can just add extras of those item #s to the array
	usKitItem = PickARandomItem( KIT , bKitClass, FALSE );
	if ( usKitItem > 0 )
	{
		CreateItem( usKitItem, (INT8)(80 + Random( 21 )), &gTempObject );
		gTempObject.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
	}
}


void ChooseMiscGearForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp, INT8 bMiscClass )
{
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"ChooseMiscGearForSoldierCreateStruct");
	//UINT16 i;
	//INVTYPE *pItem;
	UINT16 usRandom;

	//Madd: let's do this a couple times, so we can have > 1 misc item per soldier
	usRandom = Random(3);

	if ( usRandom )
	{
		UINT16 usItem = 0;
		for ( UINT16 i=0;i<usRandom;i++ )
		{
			usItem = PickARandomItem (MISCITEMS , bMiscClass, FALSE);
			if ( usItem > 0 )
			{
				int bStatus = 80 + Random( 21 );
				if ( Item[usItem].usItemClass == IC_MONEY )
					bStatus = 1 + Random( 20 );

				CreateItem( usItem, (INT8)(bStatus), &gTempObject );
				gTempObject.fFlags |= OBJECT_UNDROPPABLE;
				PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
			}
		}
	}

	// Madd: code above is externalized
#ifdef obsoleteCode
	//// not all of these are IC_MISC, some are IC_PUNCH (not covered anywhere else)
	//INT32 iMiscItemsList[] =
	//{
	//	CANTEEN,
	//	CANTEEN,
	//	CANTEEN,
	//	CANTEEN,
	//	ALCOHOL,
	//	ALCOHOL,
	//	ADRENALINE_BOOSTER,
	//	ADRENALINE_BOOSTER,
	//	REGEN_BOOSTER,
	//	BRASS_KNUCKLES,
	//	CHEWING_GUM,
	//	CIGARS,
	//	GOLDWATCH,
	//	-1
	//};


	//// count how many are eligible
	//i = 0;
	//while ( iMiscItemsList[ i ] != -1 )
	//{
	//	pItem = &Item[ iMiscItemsList [ i ] ];
	//	if( ( pItem->ubCoolness > 0 ) && ( pItem->ubCoolness <= bMiscClass ) )
	//	{
	//		// exclude REGEN_BOOSTERs unless Sci-Fi flag is on
	//		if ( ( iMiscItemsList [ i ] != REGEN_BOOSTER ) || ( gGameOptions.ubGameStyle == STYLE_SCIFI ) )
	//		{
	//			usNumMatches++;
	//		}
	//	}

	//	i++;
	//}


	//// if any are eligible, pick one of them at random
	//if( usNumMatches )
	//{
	//	usRandom = (UINT16)Random( usNumMatches );

	//	i = 0;
	//	while ( iMiscItemsList[ i ] != -1 )
	//	{
	//		pItem = &Item[ iMiscItemsList [ i ] ];
	//		if( ( pItem->ubCoolness > 0 ) && ( pItem->ubCoolness <= bMiscClass ) )
	//		{
	//			// exclude REGEN_BOOSTERs unless Sci-Fi flag is on
	//			if ( ( iMiscItemsList [ i ] != REGEN_BOOSTER ) || ( gGameOptions.ubGameStyle == STYLE_SCIFI ) )
	//			{
	//				if( usRandom )
	//					usRandom--;
	//				else
	//				{
	//					CreateItem( ( UINT16 ) iMiscItemsList[ i ], (INT8)(80 + Random( 21 )), &gTempObject );
	//					gTempObject.fFlags |= OBJECT_UNDROPPABLE;
	//					PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
	//					break;
	//				}
	//			}
	//		}

	//		i++;
	//	}
	//}
#endif //obsoleteCode
}


void ChooseBombsForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp, INT8 bBombClass )
{
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"ChooseBombsForSoldierCreateStruct");
	//UINT16 i;
	//INVTYPE *pItem;
	//UINT16 usRandom;
	UINT16 usItem = 0;

	usItem = PickARandomItem( BOMB , bBombClass, FALSE );
	if ( usItem > 0 )
	{
		CreateItem( usItem, (INT8)(80 + Random( 21 )), &gTempObject );
		gTempObject.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
	}

	// Madd: externalized again
#ifdef obsoleteCode
	//// count how many are eligible
	//for( i = 0; i < MAXITEMS; i++ )
	//{
	//	if ( Item[i].usItemClass  == 0 )
	//		break;
	//	pItem = &Item[ i ];
	//	if( ( pItem->usItemClass == IC_BOMB ) && ( pItem->ubCoolness > 0 ) && ( pItem->ubCoolness <= bBombClass ) )
	//	{
	//		usNumMatches++;
	//	}
	//}


	//// if any are eligible, pick one of them at random
	//if( usNumMatches )
	//{
	//	usRandom = (UINT16)Random( usNumMatches );
	//	for( i = 0; i < MAXITEMS; i++ )
	//	{
	//		if ( Item[i].usItemClass  == 0 )
	//			break;

	//		pItem = &Item[ i ];
	//		if( ( pItem->usItemClass == IC_BOMB ) && ( pItem->ubCoolness > 0 ) && ( pItem->ubCoolness <= bBombClass ) )
	//		{
	//			if( usRandom )
	//				usRandom--;
	//			else
	//			{
	//				CreateItem( i, (INT8)(80 + Random( 21 )), &gTempObject );
	//				gTempObject.fFlags |= OBJECT_UNDROPPABLE;
	//				PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
	//				break;
	//			}
	//		}
	//	}
	//}
#endif //obsoleteCode
}

// Headrock: Added a function to randomly create LBEs
void ChooseLBEsForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp, INT8 bLBEClass )
{
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"ChooseLBEsForSoldierCreateStruct");
	//UINT16 i;
	//INVTYPE *pItem;
	//UINT16 usRandom;
	UINT16 usItem = 0;

	// CHRISL: If we're using the old inventory system, just return
	if((UsingNewInventorySystem() == false))
		return;

	usItem = PickARandomItem( LBE , bLBEClass, FALSE );
	if ( usItem > 0 )
	{
		CreateItem( usItem, (INT8)(80 + Random( 21 )), &gTempObject );
		gTempObject.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &gTempObject );
	}
}


void ChooseLocationSpecificGearForSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp )
{
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"ChooseLocationSpecificGearForSoldierCreateStruct");
	UINT16 usItem = 0;
	// If this is Tixa and the player doesn't control Tixa then give all enemies gas masks,
	// but somewhere on their person, not in their face positions
	if ( gWorldSectorX == TIXA_SECTOR_X && gWorldSectorY == TIXA_SECTOR_Y && StrategicMap[ TIXA_SECTOR_X + TIXA_SECTOR_Y * MAP_WORLD_X ].fEnemyControlled )
	{
		//dnl ch40 041009
		usItem = PickARandomItem(GASMASKS);
		if(Random(100) > 80)
			usItem = NOTHING;
		if(usItem > 0)
		{
			CreateItem(usItem, (INT8)(90+Random(10)), &gTempObject);
			PlaceObjectInSoldierCreateStruct(pp, &gTempObject);
		}
	}
}



BOOLEAN PlaceObjectInSoldierCreateStruct( SOLDIERCREATE_STRUCT *pp, OBJECTTYPE *pObject )
{
	INT8 i;

	if( FitsInSmallPocket(pObject) == true ) {
		// CHRISL: Change static inventory pocket definition to dynamic
		//try to get it into a small pocket first
		for( i = SMALLPOCKSTART; i < SMALLPOCKFINAL; i++ )
		{
			if( pp->Inv[ i ].exists() == false && !(pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE) )
			{
				pp->Inv[ i ] = *pObject;
				return TRUE;
			}
		}
	}

	//NPCs skip medium pockets

	// CHRISL: Change static inventory pocket definition to dynamic
	for( i = BIGPOCKSTART; i < BIGPOCKFINAL; i++ )
	{ //no space free in small pockets, so put it into a large pocket.
		if( pp->Inv[ i ].exists() == false && !(pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE) )
		{
			pp->Inv[ i ] = *pObject;
			return TRUE;
		}
	}
	return FALSE;
}

void RandomlyChooseWhichItemsAreDroppable( SOLDIERCREATE_STRUCT *pp, INT8 bSoldierClass )
{
	UINT32 i;
	INT32 j;
//	UINT16 usRandomNum;
	UINT32 uiItemClass;
	UINT8 ubNumMatches = 0;
	UINT16 usItem;
	UINT8 ubAmmoDropRate;
	UINT8 ubGrenadeDropRate;
	UINT8 ubOtherDropRate;
	BOOLEAN fWeapon = FALSE;
	BOOLEAN fGrenades = FALSE;	// this includes all  grenades!
	BOOLEAN fAmmo = FALSE;
	BOOLEAN fArmour = FALSE;
	BOOLEAN fKnife = FALSE;
	BOOLEAN fKit = FALSE;
	BOOLEAN fFace = FALSE;
	BOOLEAN fMisc = FALSE;
	UINT32 uiRandomValue = 0;

	//Madd
//if ( gGameSettings.fOptions[TOPTION_DROP_ALL] ) // EXTERNILIZED TO THE INI INSTEAD - SANDRO
if ( gGameOptions.fEnemiesDropAllItems )
{
	ENEMYAMMODROPRATE = 100;
	ENEMYGRENADEDROPRATE = 100;
	ENEMYEQUIPDROPRATE = 100;	// WANNE: Changed from 50 to 100, because DROP ALL should mean DROP ALL!
	MILITIAAMMODROPRATE = 100;
	MILITIAGRENADEDROPRATE = 100;
	MILITIAEQUIPDROPRATE = 100;   // WANNE: Changed from 50 to 100, because DROP ALL should mean DROP ALL!
}
else
{
	// Default random drop
	if (gGameExternalOptions.ubEnemiesItemDrop == 0)
	{
		ENEMYAMMODROPRATE = 50;
		ENEMYGRENADEDROPRATE = 25;
		ENEMYEQUIPDROPRATE = 15;
		MILITIAAMMODROPRATE = 5;
		MILITIAGRENADEDROPRATE = 3;
		MILITIAEQUIPDROPRATE = 2;
	}
	// Get drop rate from XML-Files
	else
	{
		// Reset, because it is not used anymore
		ENEMYAMMODROPRATE = -1;
		ENEMYGRENADEDROPRATE = -1;
		ENEMYEQUIPDROPRATE = -1;
		MILITIAAMMODROPRATE = -1;
		MILITIAGRENADEDROPRATE = -1;
		MILITIAEQUIPDROPRATE = -1;
	}
}

#ifdef obsoleteCode
/*
	//40% of soldiers will have droppable items.
	usRandomNum = (UINT16)Random( 1000 );
	if( usRandomNum >= 400 )
		return;
	//so now the number is 0-399.  This is kind of like a D&D die roll where
	//various numbers drop different items, or even more than one item.  At this
	//point, we don't care if the enemy has anything in the slot that is made droppable.
	//Any items containing the OBJECT_NO_OVERWRITE slot is rejected for droppable
	//consideration.

	if( usRandomNum < 32 ) //3.2% of dead bodies present the possibility of several items (0-5 items : avg 3).
	{ //31 is the magic number that allows all 5 item classes to be dropped!
		if( usRandomNum & 16 )
			fWeapon = TRUE;
		if( usRandomNum & 8 )
			fAmmo = TRUE;
		if( usRandomNum & 4 )
			fGrenades = TRUE;
		if( usRandomNum & 2 )
			fArmour = TRUE;
		if( usRandomNum & 1 )
			fMisc = TRUE;
	}
	else if( usRandomNum < 100 ) //6.8% chance of getting 2-3 different items.
	{ //do a more generalized approach to dropping items.
		switch( usRandomNum / 10 )
		{
			case 3:	fWeapon = TRUE;											fAmmo = TRUE;																break;
			case 4:	fWeapon = TRUE;	fGrenades = TRUE;																							break;
			case 5:									fGrenades = TRUE;																fMisc = TRUE;	break;
			case 6:									fGrenades = TRUE;								fArmour = TRUE;								break;
			case 7:																			fAmmo = TRUE;	fArmour = TRUE;								break;
			case 8:																			fAmmo = TRUE;	fArmour = TRUE;	fMisc = TRUE;	break;
			case 9:									fGrenades = TRUE;	fAmmo = TRUE;									fMisc = TRUE;	break;
		}
	}
	else
	{
		switch( usRandomNum / 50 ) //30% chance of getting 1-2 items (no weapons)
		{
			case 2:									fGrenades = TRUE;																							break;
			case 3:																			fAmmo = TRUE;																break;
			case 4:																									fArmour = TRUE;									break;
			case 5:																																		fMisc = TRUE;	break;
			case 6:																			fAmmo = TRUE;									fMisc = TRUE;	break;
			case 7:									fGrenades = TRUE;	fAmmo = TRUE;																break;
		}
	}

	fKnife = (Random(3)) ? FALSE : TRUE;
*/
#endif //obsoleteCode


	if ( SOLDIER_CLASS_MILITIA( bSoldierClass ) )
	{
		// militia (they drop much less stuff)
		ubAmmoDropRate		= MILITIAAMMODROPRATE;
		ubGrenadeDropRate = MILITIAGRENADEDROPRATE;
		ubOtherDropRate = MILITIAEQUIPDROPRATE;
	}
	else
	{
		// enemy army
		ubAmmoDropRate  = ENEMYAMMODROPRATE;
		ubGrenadeDropRate = ENEMYGRENADEDROPRATE;
		ubOtherDropRate = ENEMYEQUIPDROPRATE;
	}

	// only enemy soldiers use auto-drop system!
	// don't use the auto-drop system in auto-resolve: player won't see what's being used & enemies will often win & keep'em
	// HEADROCK HAM B2.8: Militia now drop all their equipment... IF not killed by player (see TurnSoldierIntoCorpse() )
	if ( (SOLDIER_CLASS_ENEMY( bSoldierClass ) || ( gGameExternalOptions.ubMilitiaDropEquipment > 0 && SOLDIER_CLASS_MILITIA( bSoldierClass ) )) && !IsAutoResolveActive() )
	{
		// SPECIAL handling for weapons: we'll always drop a weapon type that has never been dropped before
		for( i = 0; i < pp->Inv.size(); i++ )
		{
			usItem = pp->Inv[ i ].usItem;
//TODO: someday maybe 			pp->Inv[i].fFlags &= ~ITEM_REPAIRABLE;

			//if ( gGameSettings.fOptions[TOPTION_DROP_ALL]  ) // EXTERNILIZED TO THE INI INSTEAD - SANDRO
			if ( gGameOptions.fEnemiesDropAllItems )
			{
				// WANNE: "Drop all" should mean "Drop all"
				/*
				if ( Item[ pp->Inv[ i ].usItem ].usItemClass == IC_FACE )
				{
					if( Random(100) < ubOtherDropRate ) // way too many gas masks, nvg and extended ears getting dropped!
						pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
				}
				else
				*/

				pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
			}
			else
			{
				// Default random drop
				if (gGameExternalOptions.ubEnemiesItemDrop == 0)
				{
						// if it's a weapon (monster parts included - they won't drop due to checks elsewhere!)
					if ((usItem > NONE) && (usItem < MAXITEMS )) // Madd -- this should be ok set to maxitems instead of max_Weapons
					{
						// and we're allowed to change its flags
						if(! (pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE ))
						{
							// and it's never been dropped before in this game
							if (!gStrategicStatus.fWeaponDroppedAlready[usItem])
							{
								// mark it as droppable, and remember we did so.  If the player never kills this particular dude, oh well,
								// tough luck, he missed his chance for an easy reward, he'll have to wait til next time and need some luck...
								pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;

								MarkAllWeaponsOfSameGunClassAsDropped( usItem );
							}
						}
					}
				}
			}
		}
	}

	//if ( gGameSettings.fOptions[TOPTION_DROP_ALL]  ) // EXTERNILIZED TO THE INI INSTEAD - SANDRO
	if ( gGameOptions.fEnemiesDropAllItems )
		return;



	// WANNE: Randomly choose which type of items should be dropped
	if (gGameExternalOptions.ubEnemiesItemDrop == 0)
	{
		if( Random(100) < ubAmmoDropRate )
			fAmmo = TRUE;

		if( Random(100) < ubOtherDropRate )
			fWeapon = TRUE;

		if( Random(100) < ubOtherDropRate )
			fArmour = TRUE;

		if( Random(100) < ubOtherDropRate )
			fKnife = TRUE;

		if( Random(100) < ubGrenadeDropRate )
			fGrenades = TRUE;

		if( Random(100) < ubOtherDropRate )
			fKit = TRUE;

		if( Random(100) < (UINT32)(ubOtherDropRate / 3) )
			fFace = TRUE;

		if( Random(100) < ubOtherDropRate )
			fMisc = TRUE;


		//Now, that the flags are set for each item, we now have to search through the item slots to
		//see if we can find a matching item, however, if we find any items in a particular class that
		//have the OBJECT_NO_OVERWRITE flag set, we will not make any items droppable for that class
		//because the editor would have specified it already.
		if( fAmmo )
		{
			// now drops ALL ammo found, not just the first slot
			for( i = 0; i < pp->Inv.size(); i++ )
			{
				uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
				if( uiItemClass == IC_AMMO )
				{
					if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
						continue;
					else
					{
						pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
					}
				}
			}
		}

		if( fWeapon )
		{
			ubNumMatches = 0;
			for( i = 0; i < pp->Inv.size(); i++ )
			{
				uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
				if( uiItemClass == IC_GUN || uiItemClass == IC_LAUNCHER )
				{
					if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
						break;
					else
						ubNumMatches++;
				}
			}
			if ( ubNumMatches > 0 )
			{
				for( i = 0; i < pp->Inv.size(); i++ )
				{
					uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
					if( uiItemClass == IC_GUN || uiItemClass == IC_LAUNCHER )
					{
						if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
							break;
						else if( !Random( ubNumMatches-- )  )
						{
							pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
							break;
						}
					}
				}
			}
		}

		if( fArmour )
		{
			ubNumMatches = 0;
			for( i = 0; i < pp->Inv.size(); i++ )
			{
				uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
				if( uiItemClass == IC_ARMOUR )
				{
					if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
						break;
					else
						ubNumMatches++;
				}
			}
			if ( ubNumMatches > 0 )
			{
				for( i = 0; i < pp->Inv.size(); i++ )
				{
					uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
					if( uiItemClass == IC_ARMOUR )
					{
						if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
							break;
						else if( !Random( ubNumMatches-- ) )
						{
							pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
							break;
						}
					}
				}
			}
		}

		if( fKnife)
		{
			for( i = 0; i < pp->Inv.size(); i++ )
			{
				// drops FIRST knife found
				uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
				if( uiItemClass == IC_BLADE || uiItemClass == IC_THROWING_KNIFE )
				{
					if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
						break;
					else
					{
						pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
						break;
					}
				}
			}
		}

		// note that they'll only drop ONE TYPE of grenade if they have multiple types (very common)
		if( fGrenades )
		{
			ubNumMatches = 0;
			for( i = 0; i < pp->Inv.size(); i++ )
			{
				uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
				if( uiItemClass == IC_GRENADE )
				{
					if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
						break;
					else
						ubNumMatches++;
				}
			}
			if ( ubNumMatches > 0 )
			{
				for( i = 0; i < pp->Inv.size(); i++ )
				{
					uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
					if( uiItemClass == IC_GRENADE )
					{
						if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
							break;
						else if( !Random( ubNumMatches-- )  )
						{
							pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
							break;
						}
					}
				}
			}
		}

		if( fKit )
		{
			ubNumMatches = 0;
			for( i = 0; i < pp->Inv.size(); i++ )
			{
				uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
				if( uiItemClass == IC_MEDKIT || uiItemClass == IC_KIT )
				{
					if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
						break;
					else
						ubNumMatches++;
				}
			}
			if ( ubNumMatches > 0 )
			{
				for( i = 0; i < pp->Inv.size(); i++ )
				{
					uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
					if( uiItemClass == IC_MEDKIT || uiItemClass == IC_KIT )
					{
						if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
							break;
						else if( !Random( ubNumMatches-- )  )
						{
							pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
							break;
						}
					}
				}
			}
		}

		if( fFace )
		{
			ubNumMatches = 0;
			for( i = 0; i < pp->Inv.size(); i++ )
			{
				uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
				if( uiItemClass == IC_FACE )
				{
					if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
						break;
					else
						ubNumMatches++;
				}
			}
			if ( ubNumMatches > 0 )
			{
				for( i = 0; i < pp->Inv.size(); i++ )
				{
					uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
					if( uiItemClass == IC_FACE )
					{
						if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
							break;
						else if( !Random( ubNumMatches-- ) )
						{
							pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
							break;
						}
					}
				}
			}
		}

		if( fMisc )
		{
			ubNumMatches = 0;
			for( i = 0; i < pp->Inv.size(); i++ )
			{
				uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
				if( uiItemClass == IC_MISC )
				{
					if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
						break;
					else
						ubNumMatches++;
				}
			}
			if ( ubNumMatches > 0 )
			{
				for( i = 0; i < pp->Inv.size(); i++ )
				{
					uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;
					if( uiItemClass == IC_MISC )
					{
						if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
							break;
						else if( !Random( ubNumMatches-- ) )
						{
							pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
							break;
						}
					}
				}
			}
		}
	}
	// WANNE: Drop-rate from different XML-Files
	else if (gGameExternalOptions.ubEnemiesItemDrop == 1)
	{
		// Loop through the enemy inter
		for( i = 0; i < pp->Inv.size(); i++ )
			{
				uiItemClass = Item[ pp->Inv[ i ].usItem ].usItemClass;

				// We are allowed to change the object and it is not the first (nothing) object
				//if(! (pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE ) && pp->Inv[ i ].usItem != 0)
				if( pp->Inv[ i ].exists() == true)
				{
					// Weapon
					if( uiItemClass == IC_GUN)
					{
						// Find matching weaponType in the XML
						for (j = 0; j < MAX_DROP_ITEMS; j++)
						{
							// We have no more weapon items->exit from loop
							if (j > 0 && gEnemyWeaponDrops[j].uiIndex == 0)
								break;

							// We found the matching weapon type
							if (Weapon[ Item[ pp->Inv[ i ].usItem ].ubClassIndex ].ubWeaponType == gEnemyWeaponDrops[j].ubWeaponType)
							{
								uiRandomValue = Random(100);

								if (uiRandomValue == 0)
									uiRandomValue++;

								// militia
								if ( SOLDIER_CLASS_MILITIA( bSoldierClass ) )
								{
									// Drop the item!
									if (uiRandomValue <= gEnemyWeaponDrops[j].ubMilitiaDropRate)
									{
										pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
									}
								}
								// enemy
								else
								{
									// Drop the item!
									if (uiRandomValue <= gEnemyWeaponDrops[j].ubEnemyDropRate)
									{
										pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
									}
								}

								break;
							}
						}
					}
					// Ammo
					else if (uiItemClass == IC_AMMO)
					{
						// Find matching ammo in the XML
						for (j = 0; j < MAX_DROP_ITEMS; j++)
						{
							// We have no more ammo items->exit from loop
							if (j > 0 && gEnemyAmmoDrops[j].uiIndex == 0)
								break;

							// We found the matching ammo type
							if (Magazine[ Item[ pp->Inv[ i ].usItem ].ubClassIndex ].ubAmmoType == gEnemyAmmoDrops[j].uiType)
							{
								uiRandomValue = Random(100);

								if (uiRandomValue == 0)
									uiRandomValue++;

								// militia
								if ( SOLDIER_CLASS_MILITIA( bSoldierClass ) )
								{
									// Drop the item!
									if (uiRandomValue <= gEnemyAmmoDrops[j].ubMilitiaDropRate)
									{
										pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
									}
								}
								else
								{
									// Drop the item!
									if (uiRandomValue <= gEnemyAmmoDrops[j].ubEnemyDropRate)
									{
										pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
									}
								}

								break;
							}
						}
					}
					// Explosive
					else if (uiItemClass == IC_GRENADE || uiItemClass == IC_BOMB)
					{
						// Find matching explosive in the XML
						for (j = 0; j < MAX_DROP_ITEMS; j++)
						{
							// We have no more explosive items->exit from loop
							if (j > 0 && gEnemyExplosiveDrops[j].uiIndex == 0)
								break;

							// We found the matching explosive type
							if (Explosive[Item[ pp->Inv[ i ].usItem ].ubClassIndex].ubType == gEnemyExplosiveDrops[j].ubType)
							{
								uiRandomValue = Random(100);

								if (uiRandomValue == 0)
									uiRandomValue++;

								// militia
								if ( SOLDIER_CLASS_MILITIA( bSoldierClass ) )
								{
									// Drop the item!
									if (uiRandomValue <= gEnemyExplosiveDrops[j].ubMilitiaDropRate)
									{
										pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
									}
								}
								// enemy
								else
								{
									// Drop the item!
									if (uiRandomValue <= gEnemyExplosiveDrops[j].ubEnemyDropRate)
									{
										pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
									}
								}


								break;
							}
						}
					}
					// Armour
					else if (uiItemClass == IC_ARMOUR)
					{
						// Find matching armour in the XML
						for (j = 0; j < MAX_DROP_ITEMS; j++)
						{
							// We have no more armour items->exit from loop
							if (j > 0 && gEnemyArmourDrops[j].uiIndex == 0)
								break;

							// We found the matching armour type
							if (Armour[ Item[ pp->Inv[ i ].usItem ].ubClassIndex ].ubArmourClass == gEnemyArmourDrops[j].ubArmourClass)
							{
								uiRandomValue = Random(100);

								if (uiRandomValue == 0)
									uiRandomValue++;

								// militia
								if ( SOLDIER_CLASS_MILITIA( bSoldierClass ) )
								{
									// Drop the item!
									if (uiRandomValue <= gEnemyArmourDrops[j].ubMilitiaDropRate)
									{
										pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
									}
								}
								// enemy
								else
								{
									// Drop the item!
									if (uiRandomValue <= gEnemyArmourDrops[j].ubEnemyDropRate)
									{
										pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
									}
								}

								break;
							}
						}
					}
					// Misc
					else if (uiItemClass == IC_BLADE || uiItemClass == IC_MEDKIT || uiItemClass == IC_KIT ||
						uiItemClass == IC_THROWING_KNIFE || uiItemClass == IC_LAUNCHER ||
						uiItemClass == IC_APPLIABLE || uiItemClass == IC_FACE || uiItemClass == IC_TENTACLES ||
						uiItemClass == IC_THROWN || uiItemClass == IC_PUNCH || uiItemClass == IC_KEY ||
						uiItemClass == IC_MISC || uiItemClass == IC_MONEY
						
						// WANNE: Fix by Roadkill: Also drop LBE Gear
						|| uiItemClass == IC_LBEGEAR || uiItemClass == IC_BELTCLIP						
						)
					{
						// Find matching armour in the XML
						for (j = 0; j < MAX_DROP_ITEMS; j++)
						{
							// We have no more armour items->exit from loop
							if (j > 0 && gEnemyMiscDrops[j].uiIndex == 0)
								break;

							// We found the matching armour type
							if (Item[ pp->Inv[ i ].usItem ].usItemClass == gEnemyMiscDrops[j].usItemClass)
							{
								uiRandomValue = Random(100);

								if (uiRandomValue == 0)
									uiRandomValue++;

								// militia
								if ( SOLDIER_CLASS_MILITIA( bSoldierClass ) )
								{
									// Drop the item!
									if (uiRandomValue <= gEnemyMiscDrops[j].ubMilitiaDropRate)
									{
										pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
									}
								}
								// enemy
								else
								{
									// Drop the item!
									if (uiRandomValue <= gEnemyMiscDrops[j].ubEnemyDropRate)
									{
										pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
									}
								}

								break;
							}
						}
					}
				}
			}
	}
}


void AssignCreatureInventory( SOLDIERTYPE *pSoldier )
{
	UINT32 uiChanceToDrop = 0;
	BOOLEAN fMaleCreature = FALSE;
	BOOLEAN fBloodcat = FALSE;

	// all creature items in this first section are only offensive/defensive placeholders, and
	// never get dropped, because they're not real items!
	switch( pSoldier->ubBodyType)
	{
		case ADULTFEMALEMONSTER:
			CreateItem( CREATURE_OLD_FEMALE_CLAWS,	100, &(pSoldier->inv[HANDPOS]) );
			CreateItem( CREATURE_OLD_FEMALE_HIDE,		100, &(pSoldier->inv[HELMETPOS]) );
			CreateItem( CREATURE_OLD_FEMALE_HIDE,		100, &(pSoldier->inv[VESTPOS]) );
			CreateItem( CREATURE_OLD_FEMALE_HIDE,		100, &(pSoldier->inv[LEGPOS]) );
			uiChanceToDrop = 30;
			break;
		case AM_MONSTER:
			CreateItem( CREATURE_OLD_MALE_CLAWS,		100, &(pSoldier->inv[HANDPOS]) );
			CreateItem( CREATURE_OLD_MALE_SPIT,			100, &(pSoldier->inv[SECONDHANDPOS]) );
			CreateItem( CREATURE_OLD_MALE_HIDE,			100, &(pSoldier->inv[HELMETPOS]) );
			CreateItem( CREATURE_OLD_MALE_HIDE,			100, &(pSoldier->inv[VESTPOS]) );
			CreateItem( CREATURE_OLD_MALE_HIDE,			100, &(pSoldier->inv[LEGPOS]) );
			uiChanceToDrop = 30;
			fMaleCreature = TRUE;
			break;
		case YAF_MONSTER:
			CreateItem( CREATURE_YOUNG_FEMALE_CLAWS,		100, &(pSoldier->inv[HANDPOS]) );
			CreateItem( CREATURE_YOUNG_FEMALE_HIDE,			100, &(pSoldier->inv[HELMETPOS]) );
			CreateItem( CREATURE_YOUNG_FEMALE_HIDE,			100, &(pSoldier->inv[VESTPOS]) );
			CreateItem( CREATURE_YOUNG_FEMALE_HIDE,			100, &(pSoldier->inv[LEGPOS]) );
			uiChanceToDrop = 15;
			break;
		case YAM_MONSTER:
			CreateItem( CREATURE_YOUNG_MALE_CLAWS,		100, &(pSoldier->inv[HANDPOS]) );
			CreateItem( CREATURE_YOUNG_MALE_SPIT,			100, &(pSoldier->inv[SECONDHANDPOS]) );
			CreateItem( CREATURE_YOUNG_MALE_HIDE,			100, &(pSoldier->inv[HELMETPOS]) );
			CreateItem( CREATURE_YOUNG_MALE_HIDE,			100, &(pSoldier->inv[VESTPOS]) );
			CreateItem( CREATURE_YOUNG_MALE_HIDE,			100, &(pSoldier->inv[LEGPOS]) );
			uiChanceToDrop = 15;
			fMaleCreature = TRUE;
			break;
		case INFANT_MONSTER:
			CreateItem( CREATURE_INFANT_SPIT,			100, &(pSoldier->inv[HANDPOS]) );
			CreateItem( CREATURE_INFANT_HIDE,			100, &(pSoldier->inv[HELMETPOS]) );
			CreateItem( CREATURE_INFANT_HIDE,			100, &(pSoldier->inv[VESTPOS]) );
			CreateItem( CREATURE_INFANT_HIDE,			100, &(pSoldier->inv[LEGPOS]) );
			uiChanceToDrop = 5;
			break;
		case LARVAE_MONSTER:
			uiChanceToDrop = 0;
			break;
		case QUEENMONSTER:
			CreateItem( CREATURE_QUEEN_SPIT,			100, &(pSoldier->inv[HANDPOS]) );
			CreateItem( CREATURE_QUEEN_TENTACLES, 100, &(pSoldier->inv[SECONDHANDPOS]) );
			CreateItem( CREATURE_QUEEN_HIDE,			100, &(pSoldier->inv[HELMETPOS]) );
			CreateItem( CREATURE_QUEEN_HIDE,			100, &(pSoldier->inv[VESTPOS]) );
			CreateItem( CREATURE_QUEEN_HIDE,			100, &(pSoldier->inv[LEGPOS]) );
			// she can't drop anything, because the items are unreachable anyways (she's too big!)
			uiChanceToDrop = 0;
			break;
		case BLOODCAT:
			CreateItem( BLOODCAT_CLAW_ATTACK,		100, &(pSoldier->inv[HANDPOS]) );
			CreateItem( BLOODCAT_BITE,		100, &(pSoldier->inv[SECONDHANDPOS]) );
			fBloodcat = TRUE;
			uiChanceToDrop = 30;
			break;

		default:
			AssertMsg( FALSE, String( "Invalid creature bodytype %d", pSoldier->ubBodyType ) );
			return;
	}

	// decide if the creature will drop any REAL bodyparts
	if (Random(100) < uiChanceToDrop)
	{
		CreateItem( (UINT16)(fBloodcat ? BLOODCAT_CLAWS : CREATURE_PART_CLAWS), (INT8) (80 + Random(21)), &(pSoldier->inv[BIGPOCK1POS]) );
	}

	if (Random(100) < uiChanceToDrop)
	{
		CreateItem( (UINT16)(fBloodcat ? BLOODCAT_TEETH : CREATURE_PART_FLESH), (INT8) (80 + Random(21)), &(pSoldier->inv[BIGPOCK2POS]) );
	}

	// as requested by ATE, males are more likely to drop their "organs" (he actually suggested this, I'm serious!)
	if (fMaleCreature)
	{
		// increase chance by 50%
		uiChanceToDrop += (uiChanceToDrop / 2);
	}

	if (Random(100) < uiChanceToDrop)
	{
		CreateItem( (UINT16)(fBloodcat ? BLOODCAT_PELT : CREATURE_PART_ORGAN), (INT8) (80 + Random(21)), &(pSoldier->inv[BIGPOCK3POS]) );
	}
}

void ReplaceExtendedGuns( SOLDIERCREATE_STRUCT *pp, INT8 bSoldierClass )
{
	UINT32				uiLoop, uiLoop2;
	INT8					bWeaponClass;
	UINT16				usItem, usNewGun, usAmmo, usNewAmmo;

	for ( uiLoop = 0; uiLoop < pp->Inv.size(); uiLoop++ )
	{
		usItem = pp->Inv[ uiLoop ].usItem;

		if ( ( Item[ usItem ].usItemClass & IC_GUN ) && ExtendedGunListGun( usItem ) )
		{
			if ( bSoldierClass == SOLDIER_CLASS_NONE )
			{
				usNewGun = StandardGunListReplacement( usItem );
			}
			else
			{
				bWeaponClass = GetWeaponClass( usItem );
				AssertMsg( bWeaponClass != -1, String( "Gun %d does not have a match in the extended gun array", usItem ) );
				usNewGun = SelectStandardArmyGun( bWeaponClass );
			}

			if ( usNewGun != NOTHING )
			{
				//We are creating a new gun, but the new gun needs the old gun's attachments
				OBJECTTYPE* pObj = &(pp->Inv[ uiLoop ]);
				CreateItem( usNewGun, (*pObj)[0]->data.gun.bGunStatus, &gTempObject );
				gTempObject.fFlags = (*pObj).fFlags;

				//init attachments
				InitItemAttachments(pObj);

				for (attachmentList::iterator iter = (*pObj)[0]->attachments.begin(); iter != (*pObj)[0]->attachments.end(); ++iter) {
					if(iter->exists()){
						gTempObject.AttachObject(0, &(*iter), FALSE);
					}
				}

				//copy it over
				*pObj = gTempObject;


				// must search through inventory and replace ammo accordingly
				for ( uiLoop2 = 0; uiLoop2 < pp->Inv.size(); uiLoop2++ )
				{
					usAmmo = pp->Inv[ uiLoop2 ].usItem;
					if ( (Item[ usAmmo ].usItemClass & IC_AMMO) )
					{
						usNewAmmo = FindReplacementMagazineIfNecessary( usItem, usAmmo, usNewGun );
						if (usNewAmmo != NOTHING )
						{
							// found a new magazine, replace...
							CreateItems( usNewAmmo, 100, pp->Inv[ uiLoop2 ].ubNumberOfObjects, &( pp->Inv[ uiLoop2 ] ) );
						}
					}
				}
			}
		}
	}
}



UINT16 SelectStandardArmyGun( UINT8 uiGunLevel )
{
	ARMY_GUN_CHOICE_TYPE *pGunChoiceTable;
	int uiChoice;
	int usGunIndex;

	// pick the standard army gun for this weapon class from table
//	usGunIndex = gStrategicStatus.ubStandardArmyGunIndex[uiGunLevel];

	// decided to randomize it afterall instead of repeating the same weapon over and over

	// depending on selection of the gun nut option
	//if (gGameOptions.fGunNut)
	//{
		// use table of extended gun choices
		pGunChoiceTable = &(gExtendedArmyGunChoices[0]);
	//}
	//else
	//{
	//	// use table of regular gun choices
	//	pGunChoiceTable = &(gRegularArmyGunChoices[0]);
	//}

	// choose one the of the possible gun choices
	usGunIndex = -1;

	while (usGunIndex == -1)
	{
		uiChoice = Random(pGunChoiceTable[ uiGunLevel ].ubChoices);
		usGunIndex = pGunChoiceTable[ uiGunLevel ].bItemNo[ uiChoice ];

		if (!ItemIsLegal(usGunIndex)) //Madd: check for tons of guns
			usGunIndex = -1;

		//Check to avoid an endless loop looking for "normal" guns
		if (usGunIndex == -1)
		{
			//Madd: there better be something from the original JA2 guns here somewhere (biggunlist=0)!
			int numTries = 0;
			//Try 5 more times...
			while (numTries < 5 && usGunIndex == -1)
			{
				uiChoice = Random(pGunChoiceTable[ uiGunLevel ].ubChoices);
				usGunIndex = pGunChoiceTable[ uiGunLevel ].bItemNo[ uiChoice ];

				if (!ItemIsLegal(usGunIndex)) //Madd: check for tons of guns
					usGunIndex = -1;

				numTries++;
			}

			if (usGunIndex == -1) //We still haven't found one!  Start just looping through the guns then
			{
				for (int i=0;i<pGunChoiceTable[uiGunLevel].ubChoices;i++)
				{
					usGunIndex = pGunChoiceTable[ uiGunLevel ].bItemNo[ i ];

					if (!ItemIsLegal(usGunIndex))
						usGunIndex = -1;
					else
						break;
				}
			}

			if ( usGunIndex == -1 )
			{
				//Still nothing?  Then he gets a glock
				usGunIndex = GLOCK_17;
			}

		}

	}

	Assert(usGunIndex);

	return(usGunIndex);
}



void EquipTank( SOLDIERCREATE_STRUCT *pp )
{
	// tanks get special equipment, and they drop nothing (MGs are hard-mounted & non-removable)

	// main cannon
	CreateItem( TANK_CANNON, ( INT8 )( 80 + Random( 21 ) ), &( pp->Inv[ HANDPOS ]) );
	pp->Inv[ HANDPOS ].fFlags |= OBJECT_UNDROPPABLE;

	// machine gun
	CreateItems( MINIMI, ( INT8 )( 80 + Random( 21 ) ), 1, &gTempObject );
	gTempObject.fFlags |= OBJECT_UNDROPPABLE;
	PlaceObjectInSoldierCreateStruct( pp, &gTempObject );

	// tanks don't deplete shells or ammo...
	CreateItems( TANK_SHELL, 100, 1, &gTempObject );
	gTempObject.fFlags |= OBJECT_UNDROPPABLE;
	PlaceObjectInSoldierCreateStruct( pp, &gTempObject );

	// armour equal to spectra all over (for vs explosives)
	CreateItem( SPECTRA_VEST, 100, &(pp->Inv[ VESTPOS ]) );
	pp->Inv[ VESTPOS ].fFlags |= OBJECT_UNDROPPABLE;
	CreateItem( SPECTRA_HELMET, 100, &(pp->Inv[ HELMETPOS ]) );
	pp->Inv[ HELMETPOS ].fFlags |= OBJECT_UNDROPPABLE;
	CreateItem( SPECTRA_LEGGINGS, 100, &(pp->Inv[ LEGPOS ]) );
	pp->Inv[ LEGPOS ].fFlags |= OBJECT_UNDROPPABLE;

}



void ResetMortarsOnTeamCount( void )
{
	guiMortarsRolledByTeam = 0;
}

UINT16 PickARandomItem(UINT8 typeIndex)
{
	return PickARandomItem(typeIndex,100,FALSE);
}
UINT16 PickARandomItem(UINT8 typeIndex, UINT8 maxCoolness)
{
	return PickARandomItem(typeIndex,maxCoolness,TRUE);
}
UINT16 PickARandomItem(UINT8 typeIndex, UINT8 maxCoolness, BOOLEAN getMatchingCoolness)
{
	//DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("PickARandomItem: typeIndex = %d, maxCoolness = %d, getMatchingCoolness = %d",typeIndex,maxCoolness,getMatchingCoolness));

	UINT16 usItem = 0;
	UINT32 uiChoice;
	UINT16 defaultItem = 0;
	BOOLEAN pickItem = FALSE;

	if ( gArmyItemChoices[ typeIndex ].ubChoices <= 0 )
		return 0;

	// check up to 10 times for an item with a matching coolness
	for (int i=0; i < 10;i++)
	{
		//if we've already tried more times then there are items + 1, limit the looping to speed up the game, and just plain give up
		if ( i > gArmyItemChoices[ typeIndex ].ubChoices )
			break;

		// a chance for nothing!
		uiChoice = Random(gArmyItemChoices[ typeIndex ].ubChoices + (int) ( gArmyItemChoices[ typeIndex ].ubChoices / 3 ));

		if ( uiChoice >= gArmyItemChoices[ typeIndex ].ubChoices )
		{
			if ( !getMatchingCoolness )
				return 0;
			else
				uiChoice = Random(gArmyItemChoices[ typeIndex ].ubChoices);
		}
		usItem = gArmyItemChoices[ typeIndex ].bItemNo[ uiChoice ];

		pickItem = FALSE;

		if (usItem >= 0 && Item[usItem].ubCoolness <= maxCoolness && ItemIsLegal(usItem))
		{
			// On day
			if (DayTime() == TRUE)
			{
				// Only pick items, that have not a negative day vision bonus range
				// So we only pick normal items and day items (sun googles, ...)
				if (Item[usItem].dayvisionrangebonus >= 0 )
				{
					pickItem = TRUE;
				}
			}
			// At night
			else
			{
				// Only pick items, that have not a negative night vision bonus range
				// So we only pick normal items and night items (NVG, ...)
				if (Item[usItem].nightvisionrangebonus >= 0 )
				{
					pickItem = TRUE;
				}
			}
		}


		//Madd: quickfix: don't use NVGs during the day, and no sungoggles at night either
		//if ( usItem >= 0 && Item[usItem].ubCoolness <= maxCoolness && ItemIsLegal(usItem) && (( DayTime() && Item[usItem].nightvisionrangebonus == 0 ) || ( NightTime() && Item[usItem].dayvisionrangebonus == 0 )))

		if (pickItem == TRUE)
		{
			// pick a default item in case we don't find anything with a matching coolness, but pick the coolest item we can find
			if ( defaultItem == 0 || Item[usItem].ubCoolness > Item[defaultItem].ubCoolness )
				defaultItem = usItem;

			// found something with the right coolness
			if ( Item[usItem].ubCoolness == maxCoolness || !getMatchingCoolness )
				return usItem;
		}
	}

	// couldn't find anything with the exact matching coolness, so return the best item we did find
	if (ItemIsLegal(defaultItem))
		return defaultItem;
	else
		return 0;
}
UINT16 PickARandomAttachment(UINT8 typeIndex, UINT16 usBaseItem, UINT8 maxCoolness, BOOLEAN getMatchingCoolness)
{
	//DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("PickARandomAttachment: baseItem = %d, typeIndex = %d, maxCoolness = %d, getMatchingCoolness = %d",usBaseItem, typeIndex,maxCoolness,getMatchingCoolness));

	UINT16 usItem = 0;
	UINT32 uiChoice;
	UINT16 defaultItem = 0;

//	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("PickARandomAttachment: # choices = %d", gArmyItemChoices[ typeIndex ].ubChoices ));
	if ( gArmyItemChoices[ typeIndex ].ubChoices <= 0 )
		return 0;

	// check up to 10 times for an item with a matching coolness
	for (int i=0; i < 50; i++)
	{
		//if we've already tried more times then there are items + 1, limit the looping to speed up the game, and just plain give up
		if ( i > gArmyItemChoices[ typeIndex ].ubChoices )
			break;

		uiChoice = Random(gArmyItemChoices[ typeIndex ].ubChoices);
		usItem = gArmyItemChoices[ typeIndex ].bItemNo[ uiChoice ];

		BOOLEAN fDefaultAttachment = FALSE;
		for(UINT8 cnt = 0; cnt < MAX_DEFAULT_ATTACHMENTS; cnt++){
			if(Item[usBaseItem].defaultattachments[cnt] == 0)
				break;

			if(Item[usBaseItem].defaultattachments[cnt] == usItem){
				fDefaultAttachment = TRUE;
			}
		}
		if ( usItem >= 0 && Item[usItem].ubCoolness <= maxCoolness && ValidAttachment(usItem,usBaseItem) && !fDefaultAttachment)
		{
			// pick a default item in case we don't find anything with a matching coolness, but pick the coolest item we can find
			if ( defaultItem == 0 || Item[usItem].ubCoolness > Item[defaultItem].ubCoolness )
				defaultItem = usItem;

			// found something with the right coolness
			if ( (Item[usItem].ubCoolness >= maxCoolness-2) || !getMatchingCoolness )
			{
//				DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("PickARandomAttachment: baseItem = %d, usItem = %d",usBaseItem, usItem));
				if (ItemIsLegal(usItem))
					return usItem;
			}
		}
	}

	// couldn't find anything with the exact matching coolness, so return the best item we did find
//	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("PickARandomAttachment: baseItem = %d, defaultItem = %d",usBaseItem, defaultItem));
	if (ItemIsLegal(defaultItem))
		return defaultItem;
	else
		return 0;
}

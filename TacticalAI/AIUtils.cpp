#ifdef PRECOMPILEDHEADERS
	#include "AI All.h"
#else
	#include "ai.h"
	#include "Weapons.h"
	#include "opplist.h"
	#include "Points.h"
	#include "PathAI.h"
	#include "WorldMan.h"
	#include "AIInternals.h"
	#include "Items.h"
	#include "message.h"
	#include "los.h"
	#include "assignments.h"
	#include "Soldier Functions.h"
	#include "Points.h"
	#include "GameSettings.h"
	#include "Buildings.h"
	#include "Soldier macros.h"
	#include "Render Fun.h"
	#include "strategicmap.h"
	#include "environment.h"
	#include "lighting.h"
	#include "Soldier Create.h"
	#include "SkillCheck.h"		// added by SANDRO
	#include "Vehicles.h"		// added by silversurfer
	// sevenfm:
	#include "Game Clock.h"
	#include "Rotting Corpses.h"
	#include "wcheck.h"
	#include "Drugs And Alcohol.h"
	#include "Sound Control.h"
	#include "SmokeEffects.h"
	#include "Structure Wrap.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// SANDRO - In this file, all APBPConstants[AP_CROUCH] and APBPConstants[AP_PRONE] were changed to GetAPsCrouch() and GetAPsProne()
//			On the bottom here, there are these functions made
//////////////////////////////////////////////////////////////////////

//
// CJC's DG->JA2 conversion notes
//
// Commented out:
//
// InWaterOrGas - gas stuff
// RoamingRange - point patrol stuff

extern UINT16 PickSoldierReadyAnimation( SOLDIERTYPE *pSoldier, BOOLEAN fEndReady, BOOLEAN fHipStance );
extern SECTOR_EXT_DATA	SectorExternalData[256][4];

//rain
extern INT8 gbCurrentRainIntensity;
extern BOOLEAN gfLightningInProgress;
extern BOOLEAN gfHaveSeenSomeone;
extern UINT8 ubRealAmbientLightLevel;
//end rain

UINT8 Urgency[NUM_STATUS_STATES][NUM_MORALE_STATES] =
{
	{URGENCY_LOW,  URGENCY_LOW,  URGENCY_LOW,  URGENCY_LOW,  URGENCY_LOW}, // green
	{URGENCY_HIGH, URGENCY_MED,  URGENCY_MED,  URGENCY_LOW,  URGENCY_LOW}, // yellow
	{URGENCY_HIGH, URGENCY_MED,  URGENCY_MED,  URGENCY_MED,  URGENCY_MED}, // red
	{URGENCY_HIGH, URGENCY_HIGH, URGENCY_HIGH, URGENCY_MED,  URGENCY_MED}  // black
};

UINT16 MovementMode[LAST_MOVEMENT_ACTION + 1][NUM_URGENCY_STATES] =
{
	{WALKING,	 WALKING,  WALKING }, // AI_ACTION_NONE

	{WALKING,  WALKING,  WALKING }, // AI_ACTION_RANDOM_PATROL
	{RUNNING,  RUNNING,  RUNNING }, // AI_ACTION_SEEK_FRIEND
	{RUNNING,  RUNNING,  RUNNING }, // AI_ACTION_SEEK_OPPONENT
	{RUNNING,  RUNNING,  RUNNING }, // AI_ACTION_TAKE_COVER
	{WALKING,  RUNNING,  RUNNING }, // AI_ACTION_GET_CLOSER

	{WALKING,  WALKING,  WALKING }, // AI_ACTION_POINT_PATROL,
	{RUNNING,  RUNNING,  RUNNING }, // AI_ACTION_LEAVE_WATER_GAS,
	{WALKING,  SWATTING, RUNNING }, // AI_ACTION_SEEK_NOISE,
	{RUNNING,  RUNNING,  RUNNING }, // AI_ACTION_ESCORTED_MOVE,
	{WALKING,  RUNNING,  RUNNING }, // AI_ACTION_RUN_AWAY,

	{RUNNING,  RUNNING,  RUNNING }, // AI_ACTION_KNIFE_MOVE
	{WALKING,  WALKING,  WALKING }, // AI_ACTION_APPROACH_MERC
	{RUNNING,  RUNNING,  RUNNING }, // AI_ACTION_TRACK
	{RUNNING,	 RUNNING,  RUNNING },	// AI_ACTION_EAT 
	{WALKING,	 RUNNING,  RUNNING},	// AI_ACTION_PICKUP_ITEM

	{WALKING,	 WALKING,  WALKING},	// AI_ACTION_SCHEDULE_MOVE
	{WALKING,	 WALKING,  WALKING},	// AI_ACTION_WALK
	{WALKING,	 RUNNING,  RUNNING},	// withdraw
	{RUNNING,	 RUNNING,  RUNNING},	// flank left
	{RUNNING,	 RUNNING,  RUNNING},	// flank right
	{RUNNING,	 RUNNING,  RUNNING},	// AI_ACTION_MOVE_TO_CLIMB
};

INT8 OKToAttack(SOLDIERTYPE * pSoldier, int target)
{
	// can't shoot yourself
	if (target == pSoldier->sGridNo)
		return(NOSHOOT_MYSELF);

	if (WaterTooDeepForAttacks(pSoldier->sGridNo, pSoldier->pathing.bLevel))
		return(NOSHOOT_WATER);

	// make sure a weapon is in hand (FEB.8 ADDITION: tossable items are also OK)
	if (!WeaponInHand(pSoldier))
	{
		return(NOSHOOT_NOWEAPON);
	}

	// JUST PUT THIS IN ON JULY 13 TO TRY AND FIX OUT-OF-AMMO SITUATIONS

	if ( Item[pSoldier->inv[HANDPOS].usItem].usItemClass == IC_GUN)
	{
		if ( Item[pSoldier->inv[HANDPOS].usItem].cannon )
		{
			// look for another tank shell ELSEWHERE IN INVENTORY
			if ( FindLaunchable( pSoldier, pSoldier->inv[HANDPOS].usItem ) == NO_SLOT )
			//if ( !ItemHasAttachments( &(pSoldier->inv[HANDPOS]) ) )
			{
				return(NOSHOOT_NOLOAD);
			}
		}
		else if (pSoldier->inv[HANDPOS][0]->data.gun.ubGunShotsLeft == 0 /*SB*/ || 
			!(pSoldier->inv[HANDPOS][0]->data.gun.ubGunState & GS_CARTRIDGE_IN_CHAMBER) ||
			(pSoldier->IsValidSecondHandShotForReloadingPurposes( ) && 
			(pSoldier->inv[SECONDHANDPOS][0]->data.gun.ubGunShotsLeft == 0 || 
			!(pSoldier->inv[SECONDHANDPOS][0]->data.gun.ubGunState & GS_CARTRIDGE_IN_CHAMBER))))
		{
			return(NOSHOOT_NOAMMO);
		}
	}
	else if (Item[pSoldier->inv[HANDPOS].usItem].usItemClass == IC_LAUNCHER)
	{
		if ( FindLaunchable( pSoldier, pSoldier->inv[HANDPOS].usItem ) == NO_SLOT )
		//if ( !ItemHasAttachments( &(pSoldier->inv[HANDPOS]) ) )
		{
			return(NOSHOOT_NOLOAD);
		}
	}

	return(TRUE);
}

BOOLEAN ConsiderProne( SOLDIERTYPE * pSoldier )
{
	INT32		sOpponentGridNo;
	//INT8		bOpponentLevel;
	//INT32		iRange;

	// sevenfm: admins/green militia go prone only when wounded or under fire
	if( pSoldier->ubSoldierClass == SOLDIER_CLASS_ADMINISTRATOR ||
		pSoldier->ubSoldierClass == SOLDIER_CLASS_GREEN_MILITIA )
	{
		if( pSoldier->stats.bLife > 3*pSoldier->stats.bLifeMax/4 &&
			!pSoldier->aiData.bUnderFire )
		{
			return( FALSE );
		}
	}

	// sevenfm: other soldiers also check range change desire (DEFENSIVE/CUNNING will go prone even with AI morale = 4)
	if ( RangeChangeDesire(pSoldier) > 3 &&
		!pSoldier->aiData.bUnderFire &&
		pSoldier->stats.bLife > 3*pSoldier->stats.bLifeMax/4 )
	{
		return( FALSE );
	}

	// We don't want to go prone if there is a nearby enemy
	sOpponentGridNo = ClosestKnownOpponent( pSoldier, NULL, NULL );
	if( !TileIsOutOfBounds(sOpponentGridNo) && 
		PythSpacesAway( pSoldier->sGridNo, sOpponentGridNo ) < DAY_VISION_RANGE / 4 )
	{
		return( FALSE );
	}

	return( TRUE );
}

UINT8 StanceChange( SOLDIERTYPE * pSoldier, INT16 ubAttackAPCost )
{
	// consider crouching or going prone

	if (PTR_STANDING)
	{
		if (pSoldier->bActionPoints - ubAttackAPCost >= GetAPsCrouch(pSoldier, TRUE))
		{
			if ( (pSoldier->bActionPoints - ubAttackAPCost >= GetAPsCrouch(pSoldier, TRUE) + GetAPsProne(pSoldier, TRUE)) && IsValidStance( pSoldier, ANIM_PRONE ) && ConsiderProne( pSoldier ) )
			{
				return( ANIM_PRONE );
			}
			else if ( IsValidStance( pSoldier, ANIM_CROUCH ) )
			{
				return( ANIM_CROUCH );
			}
		}
	}
	else if (PTR_CROUCHED)
	{
		if ( (pSoldier->bActionPoints - ubAttackAPCost >= GetAPsProne(pSoldier, TRUE)) && IsValidStance( pSoldier, ANIM_PRONE ) && ConsiderProne( pSoldier ) )
		{
			return( ANIM_PRONE );
		}
	}
	return( 0 );
}

UINT8 ShootingStanceChange( SOLDIERTYPE * pSoldier, ATTACKTYPE * pAttack, INT8 bDesiredDirection )
{
	// Figure out the best stance for this attack

	// We don't want to go through a lot of complex calculations here,
	// just compare the chance of the bullet hitting if we are
	// standing, crouched, or prone

	UINT16	usRealAnimState, usBestAnimState;
	INT8		bBestStanceDiff=-1;
	INT8		bLoop, bStanceNum, bStanceDiff, bAPsAfterAttack, bCurAimTime, bSetScopeMode;
	UINT32	uiChanceOfDamage, uiBestChanceOfDamage, uiCurrChanceOfDamage;
	UINT32	uiStanceBonus, uiMinimumStanceBonusPerChange = 20 - 3 * pAttack->ubAimTime;
	INT32		iRange;

	bStanceNum = 0;
	uiCurrChanceOfDamage = 0;

	bSetScopeMode = pSoldier->bScopeMode;
	pSoldier->bScopeMode = pAttack->bScopeMode;
	bAPsAfterAttack = pSoldier->bActionPoints - MinAPsToAttack( pSoldier, pAttack->sTarget, ADDTURNCOST, pAttack->ubAimTime, 1);
	pSoldier->bScopeMode = bSetScopeMode;
	if (bAPsAfterAttack < GetAPsCrouch(pSoldier, TRUE))
	{
		return( 0 );
	}
	// Unfortunately, to get this to work, we have to fake the AI guy's
	// animation state so we get the right height values
	usRealAnimState = pSoldier->usAnimState;
	usBestAnimState = pSoldier->usAnimState;
	uiBestChanceOfDamage = 0;
	iRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, pAttack->sTarget );

	switch( gAnimControl[usRealAnimState].ubEndHeight )
	{
		// set a stance number comparable with our loop variable so we can easily compute
		// stance differences and thus AP cost
		case ANIM_STAND:
			bStanceNum = 0;
			break;
		case ANIM_CROUCH:
			bStanceNum = 1;
			break;
		case ANIM_PRONE:
			bStanceNum = 2;
			break;
	}
	for (bLoop = 0; bLoop < 3; bLoop++)
	{
		bStanceDiff = abs( bLoop - bStanceNum );
		if (bStanceDiff == 2 && bAPsAfterAttack < GetAPsCrouch(pSoldier, TRUE) + GetAPsProne(pSoldier, TRUE))
		{
			// can't consider this!
			continue;
		}

		switch( bLoop )
		{
			case 0:
				if ( !pSoldier->InternalIsValidStance( bDesiredDirection, ANIM_STAND ) )
				{
					continue;
				}
				pSoldier->usAnimState = STANDING;
				break;
			case 1:
				if ( !pSoldier->InternalIsValidStance( bDesiredDirection, ANIM_CROUCH ) )
				{
					continue;
				}
				pSoldier->usAnimState = CROUCHING;
				break;
			default:
				if ( !pSoldier->InternalIsValidStance( bDesiredDirection, ANIM_PRONE ) )
				{
					continue;
				}
				pSoldier->usAnimState = PRONE;
				break;
		}

		// Hack:	Assumes the cost to reach the target stance from the current stance is the same as going back.	Probably true.
		bCurAimTime = __min( bAPsAfterAttack - GetAPsToChangeStance( pSoldier, bStanceNum), pAttack->ubAimTime);
		// If can't fire at all from this stance, don't bother with the chances of hitting
		if (bCurAimTime < 0)
		{
			continue;
		}

		uiChanceOfDamage = SoldierToLocationChanceToGetThrough( pSoldier, pAttack->sTarget, pSoldier->bTargetLevel, pSoldier->bTargetCubeLevel, pAttack->ubOpponent ) * CalcChanceToHitGun( pSoldier, pAttack->sTarget, bCurAimTime, AIM_SHOT_TORSO ) / 100;
		if (uiChanceOfDamage > 0)
		{
			uiStanceBonus = 0;
			// artificially augment "chance of damage" to reflect penalty to be shot at various stances
			switch( pSoldier->usAnimState )
			{
				case CROUCHING:
					if (iRange > POINT_BLANK_RANGE + 10 * (AIM_PENALTY_TARGET_CROUCHED / 3))
					{
						uiStanceBonus = AIM_BONUS_CROUCHING;
					}
					else if (iRange > POINT_BLANK_RANGE)
					{
						// reduce chance to hit with distance to the prone/immersed target
						uiStanceBonus = 3 * ((iRange - POINT_BLANK_RANGE) / CELL_X_SIZE); // penalty -3%/tile
					}
					break;
				case PRONE:
					if (iRange <= MIN_PRONE_RANGE)
					{
						// HATE being prone this close!
						uiChanceOfDamage = 0;
					}
					else //if (iRange > POINT_BLANK_RANGE)
					{
						// reduce chance to hit with distance to the prone/immersed target
						uiStanceBonus = 3 * ((iRange - POINT_BLANK_RANGE) / CELL_X_SIZE); // penalty -3%/tile
					}
					break;
				default:
					break;
			}
			// reduce stance bonus according to how much we have to change stance to get there
			//uiStanceBonus = uiStanceBonus * (4 - bStanceDiff) / 4;
			uiChanceOfDamage += uiStanceBonus;
		}

		if (bStanceDiff == 0)
		{
			uiCurrChanceOfDamage = uiChanceOfDamage;
		}
		if (uiChanceOfDamage > uiBestChanceOfDamage )
		{
			uiBestChanceOfDamage = uiChanceOfDamage;
			usBestAnimState = pSoldier->usAnimState;
			bBestStanceDiff = bStanceDiff;
		}
	}

	pSoldier->usAnimState = usRealAnimState;

	// return 0 or the best height value to be at
	if (bBestStanceDiff == 0 || ((uiBestChanceOfDamage - uiCurrChanceOfDamage) / bBestStanceDiff) < uiMinimumStanceBonusPerChange)
	{
		// better off not changing our stance!
		return( 0 );
	}
	else
	{
		return( gAnimControl[ usBestAnimState ].ubEndHeight );
	}
}


UINT16 DetermineMovementMode( SOLDIERTYPE * pSoldier, INT8 bAction )
{
	// zombies always run if they know enemy location
	if (pSoldier->IsZombie() && IS_MERC_BODY_TYPE(pSoldier))
	{
		INT32 sClosestThreat = ClosestKnownOpponent(pSoldier, NULL, NULL);

		if (!TileIsOutOfBounds(sClosestThreat))
		{
			return RUNNING;
		}
		else
		{
			return WALKING;
		}
	}

	if ( pSoldier->flags.fUIMovementFast )
	{
		return( RUNNING );
	}
	else if ( CREATURE_OR_BLOODCAT( pSoldier ) )
	{
		if (pSoldier->aiData.bAlertStatus == STATUS_GREEN)
		{
			return( WALKING );
		}
		else
		{
			return( RUNNING );
		}
	}
	else if (pSoldier->ubBodyType == COW || pSoldier->ubBodyType == CROW)
	{
		return( WALKING );
	}
	else
	{
		if ( (pSoldier->aiData.fAIFlags & AI_CAUTIOUS) )
		{
			// if soldier is already crouched/prone, use SWATTING
			if( IS_MERC_BODY_TYPE(pSoldier ) &&
				(pSoldier->bTeam == ENEMY_TEAM || pSoldier->bTeam == MILITIA_TEAM) &&
				gAnimControl[ pSoldier->usAnimState ].ubEndHeight <= ANIM_CROUCH )
			{
				return SWATTING;
			}
			// use WALKING instead of RUNNING
			if( MovementMode[bAction][Urgency[pSoldier->aiData.bAlertStatus][pSoldier->aiData.bAIMorale]] == RUNNING )
			{
				return( WALKING );
			}

			// if not RUNNING, use default movement mode
			return MovementMode[bAction][Urgency[pSoldier->aiData.bAlertStatus][pSoldier->aiData.bAIMorale]];
		}
		else if ( bAction == AI_ACTION_SEEK_NOISE && pSoldier->bTeam == CIV_TEAM && !IS_MERC_BODY_TYPE( pSoldier ) )
		{
			return( WALKING );
		}
		else if ( (pSoldier->ubBodyType == HATKIDCIV || pSoldier->ubBodyType == KIDCIV) && (pSoldier->aiData.bAlertStatus == STATUS_GREEN) && Random( 10 ) == 0 )
		{
			return( KID_SKIPPING );
		}
		else
		{
			// sevenfm: movement mode tweaks
			INT32 sClosestThreat =	ClosestKnownOpponent( pSoldier, NULL, NULL );

			// use walking mode if no enemy known
			if (pSoldier->aiData.bAlertStatus < STATUS_RED &&
				TileIsOutOfBounds(sClosestThreat) &&
				!pSoldier->aiData.bUnderFire &&
				(bAction == AI_ACTION_SEEK_FRIEND || bAction == AI_ACTION_SEEK_NOISE || bAction == AI_ACTION_TAKE_COVER))
			{
				return WALKING;
			}

			// use swatting when blinded
			if (IS_MERC_BODY_TYPE(pSoldier) &&
				pSoldier->bBlindedCounter > 0)
			{
				return SWATTING;
			}

			if ( IS_MERC_BODY_TYPE( pSoldier ) &&
				pSoldier->aiData.bAlertStatus >= STATUS_YELLOW &&
				!InWaterGasOrSmoke( pSoldier, pSoldier->sGridNo ) &&
				!(pSoldier->flags.uiStatusFlags & SOLDIER_BOXER) &&				
				!TileIsOutOfBounds(sClosestThreat) &&
				(pSoldier->bTeam == ENEMY_TEAM || pSoldier->bTeam == MILITIA_TEAM) )
			{
				// determine max visible distance
				//INT16 sDistanceVisible = DistanceVisible( pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, pSoldier->sGridNo, pSoldier->pathing.bLevel );
				INT16 sDistanceVisible = VISION_RANGE;

				// use running when in light at night (moving slowly in light at night is dangerous)
				if( NightTime() &&
					InLightAtNight( pSoldier->sGridNo, pSoldier->pathing.bLevel ) &&
					(bAction == AI_ACTION_SEEK_OPPONENT || 
					bAction == AI_ACTION_GET_CLOSER ||
					bAction == AI_ACTION_SEEK_FRIEND ||
					bAction == AI_ACTION_TAKE_COVER) &&
					pSoldier->bBreath > 25 )
				{
					return RUNNING;
				}

				// night swatting approach (more chance for AI to sneak unseen and get interrupt)
				if( NightTime() &&
					!InLightAtNight( pSoldier->sGridNo, pSoldier->pathing.bLevel) &&
					pSoldier->aiData.bAlertStatus == STATUS_RED &&
					pSoldier->aiData.bShock == 0 &&
					!GuySawEnemy(pSoldier) &&					
					CountNearbyFriends(pSoldier, pSoldier->sGridNo, DAY_VISION_RANGE / 4) < 3 &&
					PythSpacesAway(pSoldier->sGridNo, sClosestThreat) < 3*sDistanceVisible/2 &&
					CountFriendsBlack(pSoldier) == 0 &&
					(bAction == AI_ACTION_SEEK_OPPONENT) )
				{					
					return SWATTING;
				}

				// use swatting/crawling for SEEK in RED state if soldier is already crouched/prone
				// (more chance for AI to sneak unseen and get interrupt)
				if( !InLightAtNight( pSoldier->sGridNo, pSoldier->pathing.bLevel) &&
					pSoldier->aiData.bAlertStatus == STATUS_RED &&
					pSoldier->aiData.bShock == 0 &&
					!GuySawEnemy(pSoldier) &&					
					CountNearbyFriends(pSoldier, pSoldier->sGridNo, DAY_VISION_RANGE / 4) < 3 &&
					PythSpacesAway(pSoldier->sGridNo, sClosestThreat) < 3*sDistanceVisible/2 &&					
					CountFriendsBlack(pSoldier) == 0 &&
					RangeChangeDesire(pSoldier) < 4 &&					
					gAnimControl[ pSoldier->usAnimState ].ubEndHeight <= ANIM_CROUCH &&
					(bAction == AI_ACTION_SEEK_OPPONENT) )
				{
					return SWATTING;
				}

				// use swatting for taking cover
				if( pSoldier->aiData.bAlertStatus >= STATUS_RED &&
					PythSpacesAway( pSoldier->sGridNo, sClosestThreat ) > DAY_VISION_RANGE / 8 &&
					(pSoldier->aiData.bUnderFire && RangeChangeDesire(pSoldier) < 4 ||
					pSoldier->aiData.bShock > 2*RangeChangeDesire(pSoldier) ||
					pSoldier->aiData.bShock > 0 && gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_PRONE) &&
					!pSoldier->aiData.bLastAttackHit &&
					( bAction == AI_ACTION_TAKE_COVER ) )
				{
					return SWATTING;
				}

				// use SWATTING/CRAWLING when under fire
				// (use suppression fire to slow down enemy)
				if( pSoldier->aiData.bAlertStatus >= STATUS_RED &&
					(pSoldier->aiData.bShock > RangeChangeDesire(pSoldier) && PythSpacesAway( pSoldier->sGridNo, sClosestThreat ) > DAY_VISION_RANGE / 2 ||
					pSoldier->aiData.bShock > 0 && gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_PRONE && PythSpacesAway( pSoldier->sGridNo, sClosestThreat ) > DAY_VISION_RANGE / 4) &&
					PythSpacesAway(pSoldier->sGridNo, sClosestThreat) < 3*sDistanceVisible/2 &&
					gAnimControl[ pSoldier->usAnimState ].ubEndHeight <= ANIM_CROUCH &&
					!pSoldier->aiData.bLastAttackHit &&
					( bAction == AI_ACTION_SEEK_OPPONENT || 
					bAction == AI_ACTION_GET_CLOSER ||
					bAction == AI_ACTION_SEEK_FRIEND ||
					bAction == AI_ACTION_TAKE_COVER ) )
				{
					/*if( gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_PRONE &&
						!InARoom( pSoldier->sGridNo, &usRoom ) )
						RangeChangeDesire(pSoldier) < 4 )
					{
						return CRAWLING;
					}*/
					return SWATTING;
				}

				// use SWATTING when in a room and seen enemy recently or under fire
				// (better cover for AI hiding behind windows)
				if ( InARoom( pSoldier->sGridNo, NULL ) &&
					pSoldier->aiData.bAlertStatus >= STATUS_YELLOW &&
					( pSoldier->aiData.bOrders == SNIPER ||
					pSoldier->aiData.bOrders == STATIONARY ||
					(GuySawEnemy(pSoldier) || pSoldier->aiData.bShock > 0 ) && RangeChangeDesire(pSoldier) < 4 ) &&
					PythSpacesAway( pSoldier->sGridNo, sClosestThreat ) > DAY_VISION_RANGE / 4 &&
					( bAction == AI_ACTION_SEEK_OPPONENT || 
					bAction == AI_ACTION_GET_CLOSER ||
					bAction == AI_ACTION_SEEK_FRIEND ||
					bAction == AI_ACTION_TAKE_COVER ||
					bAction == AI_ACTION_SEEK_NOISE ) )
				{
					return SWATTING;
				}

				// use swatting/crawling for snipers on roof or when under fire
				// (better cover for AI, especially if there are sandbags on roof)
				if( pSoldier->pathing.bLevel > 0 &&
					pSoldier->aiData.bAlertStatus >= STATUS_YELLOW &&					
					( pSoldier->aiData.bOrders == SNIPER ||
					pSoldier->aiData.bOrders == STATIONARY ||
					pSoldier->aiData.bShock > 0 && RangeChangeDesire(pSoldier) < 4 ) &&
					PythSpacesAway( pSoldier->sGridNo, sClosestThreat ) > DAY_VISION_RANGE / 4 &&
					( bAction == AI_ACTION_SEEK_OPPONENT || 
					bAction == AI_ACTION_GET_CLOSER ||
					bAction == AI_ACTION_SEEK_FRIEND ||
					bAction == AI_ACTION_TAKE_COVER ||
					bAction == AI_ACTION_SEEK_NOISE ) )
				{
					return SWATTING;
				}

				UINT8 ubStance = gAnimControl[pSoldier->usAnimState].ubEndHeight;

				// use running for taking cover when not under attack
				if (!pSoldier->aiData.bUnderFire &&
					bAction == AI_ACTION_TAKE_COVER &&
					pSoldier->bInitialActionPoints > APBPConstants[AP_MINIMUM] &&
					!TileIsOutOfBounds(sClosestThreat) &&
					(!InARoom(pSoldier->sGridNo, NULL) || PythSpacesAway(sClosestThreat, pSoldier->sGridNo) > DAY_VISION_RANGE * 2) &&
					pSoldier->aiData.bAIMorale >= MORALE_NORMAL &&
					pSoldier->bBreath > 25 &&
					pSoldier->pathing.bLevel == 0 &&
					pSoldier->aiData.bOrders != STATIONARY &&
					pSoldier->aiData.bOrders != SNIPER &&
					(ubStance > ANIM_PRONE || pSoldier->bActionPoints > APBPConstants[AP_MINIMUM]))
				{
					return RUNNING;
				}

				// use walking/swatting when flanking in realtime
				// (better cover at night, save BP in realtime at daytime)
				if(AICheckIsFlanking(pSoldier) && !gfTurnBasedAI)
				{
					if(NightTime())
					{
						return SWATTING;
					}
					if(pSoldier->bBreath < pSoldier->bBreathMax/2)
					{
						return WALKING;
					}					
				}
			}

			return( MovementMode[bAction][Urgency[pSoldier->aiData.bAlertStatus][pSoldier->aiData.bAIMorale]] );
		}
	}
}

void NewDest(SOLDIERTYPE *pSoldier, INT32 usGridNo)
{
	// sevenfm: disable
	/*BOOLEAN fSet = FALSE;

	if ( IS_MERC_BODY_TYPE( pSoldier ) && pSoldier->aiData.bAction == AI_ACTION_TAKE_COVER && (pSoldier->aiData.bOrders == DEFENSIVE || pSoldier->aiData.bOrders == CUNNINGSOLO || pSoldier->aiData.bOrders == CUNNINGAID ) && (SoldierDifficultyLevel( pSoldier ) >= 2) )
	{
		UINT16 usMovementMode;

		// getting real movement anim for someone who is going to take cover, not just considering
		usMovementMode = MovementMode[AI_ACTION_TAKE_COVER][Urgency[pSoldier->aiData.bAlertStatus][pSoldier->aiData.bAIMorale]];

		if ( usMovementMode != SWATTING )
		{
			// really want to look at path, see how far we could get on path while swatting
			if ( EnoughPoints( pSoldier, RecalculatePathCost( pSoldier, SWATTING ), 0, FALSE ) || (pSoldier->aiData.bLastAction == AI_ACTION_TAKE_COVER && pSoldier->usUIMovementMode == SWATTING ) )
			{
				pSoldier->usUIMovementMode = SWATTING;
			}
			else
			{
				pSoldier->usUIMovementMode = usMovementMode;
			}
		}
		else
		{
			pSoldier->usUIMovementMode = usMovementMode;
		}
		fSet = TRUE;
	}
	else
	{
		if ( pSoldier->bTeam == ENEMY_TEAM && pSoldier->aiData.bAlertStatus == STATUS_RED )
		{
			switch( pSoldier->aiData.bAction )
			{

				case AI_ACTION_MOVE_TO_CLIMB:
				case AI_ACTION_RUN_AWAY:
					pSoldier->usUIMovementMode = DetermineMovementMode( pSoldier, pSoldier->aiData.bAction );
					fSet = TRUE;
					break;
				default:
					if ( !fSet )
					{
						pSoldier->usUIMovementMode = DetermineMovementMode( pSoldier, pSoldier->aiData.bAction );
						fSet = TRUE;
					}
					break;
			}

		}
		else
		{
			pSoldier->usUIMovementMode = DetermineMovementMode( pSoldier, pSoldier->aiData.bAction );
			fSet = TRUE;
		}

		if ( pSoldier->usUIMovementMode == SWATTING && !IS_MERC_BODY_TYPE( pSoldier ) )
		{
			pSoldier->usUIMovementMode = WALKING;
		}
	}*/

	// sevenfm: always use DetermineMovementMode
	pSoldier->usUIMovementMode = DetermineMovementMode( pSoldier, pSoldier->aiData.bAction );
	// check for non merc bodytypes
	if ( pSoldier->usUIMovementMode == SWATTING && !IS_MERC_BODY_TYPE( pSoldier ) )
	{
		pSoldier->usUIMovementMode = WALKING;
	}

	//pSoldier->EVENT_GetNewSoldierPath( pSoldier->pathing.sDestination, pSoldier->usUIMovementMode );
	// ATE: Using this more versatile version
	// Last parameter says whether to re-start the soldier's animation
	// This should be done if buddy was paused for fNoApstofinishMove...
	pSoldier->EVENT_InternalGetNewSoldierPath( usGridNo, pSoldier->usUIMovementMode , FALSE, pSoldier->flags.fNoAPToFinishMove );
}


BOOLEAN IsActionAffordable(SOLDIERTYPE *pSoldier, INT8 bAction)
{
	INT16	bMinPointsNeeded = 0;
	// sevenfm: r7972 fix
	INT8 bAPForStandUp = 0;
	INT8 bAPToLookAtWall = 0;

	//NumMessage("AffordableAction - Guy#",pSoldier->ubID);

	if( bAction == AI_ACTION_NONE )
	{
		bAction = pSoldier->aiData.bAction;
	}

	switch (bAction)
	{
		case AI_ACTION_NONE:                  // maintain current position & facing
			// no cost for doing nothing!
			break;
		case AI_ACTION_CHANGE_FACING:         // turn to face another direction
			bMinPointsNeeded = (INT8) GetAPsToLook( pSoldier );
			break;
		case AI_ACTION_RANDOM_PATROL:         // move towards a particular location
		case AI_ACTION_SEEK_FRIEND:           // move towards friend in trouble
		case AI_ACTION_SEEK_OPPONENT:         // move towards a reported opponent
		case AI_ACTION_TAKE_COVER:            // run for nearest cover from threat
		case AI_ACTION_GET_CLOSER:            // move closer to a strategic location
		case AI_ACTION_POINT_PATROL:          // move towards next patrol point
		case AI_ACTION_LEAVE_WATER_GAS:       // seek nearest spot of ungassed land
		case AI_ACTION_SEEK_NOISE:            // seek most important noise heard
		case AI_ACTION_ESCORTED_MOVE:         // go where told to by escortPlayer
		case AI_ACTION_RUN_AWAY:              // run away from nearby opponent(s)
		case AI_ACTION_APPROACH_MERC:
		case AI_ACTION_TRACK:
		case AI_ACTION_EAT:
		case AI_ACTION_SCHEDULE_MOVE:
		case AI_ACTION_WALK:
		case AI_ACTION_MOVE_TO_CLIMB:
			// for movement, must have enough APs to move at least 1 tile's worth
			bMinPointsNeeded = MinPtsToMove(pSoldier);
			break;
		case AI_ACTION_PICKUP_ITEM:           // grab things lying on the ground
			bMinPointsNeeded = __max( MinPtsToMove( pSoldier ), GetBasicAPsToPickupItem( pSoldier ) ); // SANDRO
			break;
		case AI_ACTION_OPEN_OR_CLOSE_DOOR:
		case AI_ACTION_UNLOCK_DOOR:
		case AI_ACTION_LOCK_DOOR:
			bMinPointsNeeded = MinPtsToMove(pSoldier);
			break;
		case AI_ACTION_DROP_ITEM:
			bMinPointsNeeded = GetBasicAPsToPickupItem( pSoldier ); // SANDRO
			break;
		case AI_ACTION_FIRE_GUN:              // shoot at nearby opponent
		case AI_ACTION_TOSS_PROJECTILE:       // throw grenade at/near opponent(s)
		case AI_ACTION_KNIFE_MOVE:            // preparing to stab adjacent opponent
		case AI_ACTION_THROW_KNIFE:
			// only FIRE_GUN currently actually pays extra turning costs!
			bMinPointsNeeded = MinAPsToAttack(pSoldier,pSoldier->aiData.usActionData,ADDTURNCOST,pSoldier->aiData.bAimTime);
#ifdef BETAVERSION
			if (ptsNeeded > pSoldier->bActionPoints)
			{
			/*
				sprintf(tempstr,"AI ERROR: %s has insufficient points for attack action %d at grid %d",
							pSoldier->name,pSoldier->aiData.bAction,pSoldier->aiData.usActionData);
				PopMessage(tempstr);
				*/
			}
#endif
			break;
		case AI_ACTION_PULL_TRIGGER:          // activate an adjacent panic trigger
			bMinPointsNeeded = APBPConstants[AP_PULL_TRIGGER];
			break;
		case AI_ACTION_USE_DETONATOR:         // grab detonator and set off bomb(s)
			bMinPointsNeeded = APBPConstants[AP_USE_REMOTE];
			break;
		case AI_ACTION_YELLOW_ALERT:          // tell friends opponent(s) heard
		case AI_ACTION_RED_ALERT:             // tell friends opponent(s) seen
		case AI_ACTION_CREATURE_CALL:				 // for now
			bMinPointsNeeded = APBPConstants[AP_RADIO];
			break;
		case AI_ACTION_CHANGE_STANCE:                // crouch
			bMinPointsNeeded = GetAPsCrouch(pSoldier, TRUE);
			break;
		case AI_ACTION_GIVE_AID:              // help injured/dying friend
			bMinPointsNeeded = 0;
			break;
		case AI_ACTION_CLIMB_ROOF:
			// sevenfm: r7972 fix
			bAPForStandUp = 0;
			bAPToLookAtWall = (FindDirectionForClimbing(pSoldier, pSoldier->sGridNo) == pSoldier->ubDirection) ? 0 : GetAPsToLook(pSoldier);

			// SANDRO - improved this a bit
			if (pSoldier->pathing.bLevel == 0)
			{
				if (PTR_CROUCHED) bAPForStandUp = (INT8)(GetAPsCrouch(pSoldier, TRUE));
				else if (PTR_PRONE) bAPForStandUp = GetAPsCrouch(pSoldier, TRUE) + GetAPsProne(pSoldier, TRUE);
				bMinPointsNeeded = GetAPsToClimbRoof(pSoldier, FALSE) + bAPForStandUp + bAPToLookAtWall;
			}
			else
			{
				if (!PTR_CROUCHED) bAPForStandUp = (INT8)(GetAPsCrouch(pSoldier, TRUE));
				bMinPointsNeeded = GetAPsToClimbRoof(pSoldier, TRUE) + bAPForStandUp + bAPToLookAtWall;
			}
			break;
		case AI_ACTION_COWER:
		case AI_ACTION_STOP_COWERING:
		case AI_ACTION_LOWER_GUN:
		case AI_ACTION_END_COWER_AND_MOVE:
		case AI_ACTION_TRAVERSE_DOWN:
		case AI_ACTION_OFFER_SURRENDER:
			bMinPointsNeeded = 0;
			break;
		case AI_ACTION_STEAL_MOVE: // added by SANDRO
			//bMinPointsNeeded = GetAPsToStealItem( pSoldier, NULL, pSoldier->aiData.usActionData );;
			break;

		case AI_ACTION_JUMP_WINDOW:
			if((UsingNewInventorySystem() == true) && pSoldier->inv[BPACKPOCKPOS].exists() == true)
				bMinPointsNeeded = GetAPsToJumpThroughWindows( pSoldier, TRUE );
			else
				bMinPointsNeeded = GetAPsToJumpFence( pSoldier, FALSE );
			break;
		case AI_ACTION_FREE_PRISONER:
			bMinPointsNeeded = APBPConstants[AP_HANDCUFF];
			break;
		case AI_ACTION_USE_SKILL:
			bMinPointsNeeded = 10;	// TODO
			break;
		case AI_ACTION_HANDLE_ITEM:
			bMinPointsNeeded = 0;
			break;
		default:
#ifdef BETAVERSION
			//NumMessage("AffordableAction - Illegal action type = ",pSoldier->aiData.bAction);
#endif
			break;
	}

	// check whether or not we can afford to do this action
	if (bMinPointsNeeded > pSoldier->bActionPoints)
	{
		return(FALSE);
	}
	else
	{
		return(TRUE);
	}
}

INT16 RandomFriendWithin(SOLDIERTYPE *pSoldier)
{
	UINT32				uiLoop;
	UINT16				usMaxDist;
	UINT8					ubFriendCount, ubFriendIDs[MAXMERCS], ubFriendID;
	UINT8				ubDirection;
	UINT8					ubDirsLeft;
	BOOLEAN				fDirChecked[8];
	BOOLEAN				fRangeRestricted = FALSE, fFound = FALSE;
	INT32				usDest, usOrigin;
	SOLDIERTYPE *	pFriend;


	// obtain maximum roaming distance from soldier's origin
	usMaxDist = RoamingRange(pSoldier,&usOrigin);

	// if our movement range is restricted

	// CJC: since RandomFriendWithin is only used in non-combat, ALWAYS restrict range.
	fRangeRestricted = TRUE;
	/*
	if (usMaxDist < MAX_ROAMING_RANGE)
	{
		fRangeRestricted = TRUE;
	}
	*/

	// if range is restricted, make sure origin is a legal gridno!
	if (fRangeRestricted && ((usOrigin < 0) || (usOrigin >= GRIDSIZE)))
	{
#ifdef BETAVERSION
		NameMessage(pSoldier,"has illegal origin, but his roaming range is restricted!",1000);
#endif
		return(FALSE);
	}

	ubFriendCount = 0;

	// build a list of the guynums of all active, eligible friendly mercs

	// go through each soldier, looking for "friends" (soldiers on same side)
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pFriend = MercSlots[ uiLoop ];

		// if this merc is inactive, not in sector, or dead
		if (!pFriend)
		{
			continue;
		}

		// skip ourselves
		if (pFriend->ubID == pSoldier->ubID)
		{
			continue;
		}

		// if this man not neutral, but is on my side, OR if he is neutral, but
		// so am I, then he's a "friend" for the purposes of random visitations
		if ((!pFriend->aiData.bNeutral && (pSoldier->bSide == pFriend->bSide)) ||
			(pFriend->aiData.bNeutral && pSoldier->aiData.bNeutral))
		{
			// if we're not already neighbors
			if (SpacesAway(pSoldier->sGridNo,pFriend->sGridNo) > 1)
			{
		// remember his guynum, increment friend counter
			ubFriendIDs[ubFriendCount++] = pFriend->ubID;
			}
		}
	}


	while (ubFriendCount && !fFound)
	{
		// randomly select one of the remaining friends in the list
		ubFriendID = ubFriendIDs[PreRandom(ubFriendCount)];

		// if our movement range is NOT restricted, or this friend's within range
		// use distance - 1, because there must be at least 1 tile 1 space closer
		if (!fRangeRestricted ||
			(SpacesAway(usOrigin,Menptr[ubFriendID].sGridNo) - 1) <= usMaxDist)
		{
			// should be close enough, try to find a legal->pathing.sDestination within 1 tile

			// clear dirChecked flag for all 8 directions
			for (ubDirection = 0; ubDirection < 8; ubDirection++)
			{
				fDirChecked[ubDirection] = FALSE;
			}

			ubDirsLeft = 8;

			// examine all 8 spots around 'ubFriendID'
			// keep looking while directions remain and a satisfactory one not found
			while ((ubDirsLeft--) && !fFound)
			{
				// randomly select a direction which hasn't been 'checked' yet
				do
				{
					ubDirection = (UINT8) Random(8);
				}
				while (fDirChecked[ubDirection]);

				fDirChecked[ubDirection] = TRUE;

				// determine the gridno 1 tile away from current friend in this direction
				usDest = NewGridNo(Menptr[ubFriendID].sGridNo,DirectionInc(ubDirection));

				// if that's out of bounds, ignore it & check next direction
				if (usDest == Menptr[ubFriendID].sGridNo)
				{
					continue;
				}

				// if our movement range is NOT restricted
				if (!fRangeRestricted || (SpacesAway(usOrigin,usDest) <= usMaxDist))
				{
					if (LegalNPCDestination(pSoldier,usDest,ENSURE_PATH,NOWATER, 0))
					{
						fFound = TRUE;			// found a spot
						pSoldier->aiData.usActionData = usDest;	// store this->pathing.sDestination
						pSoldier->pathing.bPathStored = TRUE;	// optimization - Ian
						break;					// stop checking in other directions
					}
				}
			}
		}

		if (!fFound)
		{
			ubFriendCount--;

			// if we hadn't already picked the last friend currently in the list
			if (ubFriendCount != ubFriendID)
			{
				ubFriendIDs[ubFriendID] = ubFriendIDs[ubFriendCount];
			}
		}
	}

	return(fFound);
}


INT32 RandDestWithinRange(SOLDIERTYPE *pSoldier)
{
	INT32 sRandDest = NOWHERE;
	INT32 usOrigin, usMaxDist;
	UINT8	ubTriesLeft;
	BOOLEAN fLimited = FALSE, fFound = FALSE;
	INT16 sMaxLeft, sMaxRight, sMaxUp, sMaxDown, sXRange, sYRange, sXOffset, sYOffset;
	INT16 sOrigX, sOrigY;
	INT16 sX, sY;
	//DBrot: More Rooms
	//UINT8	ubRoom = 0, ubTempRoom;
	UINT16 usRoom = 0, usTempRoom;
	sOrigX = sOrigY = -1;
	sMaxLeft = sMaxRight = sMaxUp = sMaxDown = sXRange = sYRange = -1;

	// Try to find a random->pathing.sDestination that's no more than maxDist away from
	// the given gridno of origin

	if (gfTurnBasedAI)
	{
		ubTriesLeft = 10;
	}
	else
	{
		ubTriesLeft = 1;
	}

	usMaxDist = RoamingRange(pSoldier,&usOrigin);

	if ( pSoldier->aiData.bOrders <= CLOSEPATROL && (pSoldier->bTeam == CIV_TEAM || pSoldier->ubProfile != NO_PROFILE ) )
	{
		// any other combo uses the default of ubRoom == 0, set above
		if ( !InARoom( pSoldier->aiData.sPatrolGrid[0], &usRoom ) )
		{
			usRoom = 0;
		}
	}

	// if the maxDist is truly a restriction
	if (usMaxDist < (MAXCOL - 1))
	{
		fLimited = TRUE;

		// determine maximum horizontal limits
		sOrigX = usOrigin % MAXCOL;
		sOrigY = usOrigin / MAXCOL;

		sMaxLeft	= min(usMaxDist, sOrigX);
		sMaxRight = min(usMaxDist,MAXCOL - (sOrigX + 1));

		// determine maximum vertical limits
		sMaxUp	= min(usMaxDist, sOrigY);
		sMaxDown = min(usMaxDist,MAXROW - (sOrigY + 1));

		sXRange = sMaxLeft + sMaxRight + 1;
		sYRange = sMaxUp + sMaxDown + 1;
	}

	if (pSoldier->ubBodyType == LARVAE_MONSTER)
	{
		// only crawl 1 tile, within our roaming range
		while ((ubTriesLeft--) && !fFound)
		{
			sXOffset = (INT16) Random( 3 ) - 1; // generates -1 to +1
			sYOffset = (INT16) Random( 3 ) - 1;

			if (fLimited)
			{
				sX = pSoldier->sGridNo % MAXCOL + sXOffset;
				sY = pSoldier->sGridNo / MAXCOL + sYOffset;
				if (sX < sOrigX - sMaxLeft || sX > sOrigX + sMaxRight)
				{
					continue;
				}
				if (sY < sOrigY - sMaxUp || sY > sOrigY + sMaxDown)
				{
					continue;
				}
				sRandDest = usOrigin + sXOffset + (MAXCOL * sYOffset);
			}
			else
			{
				sRandDest = usOrigin + sXOffset + (MAXCOL * sYOffset);
			}

			if (!LegalNPCDestination(pSoldier,sRandDest,ENSURE_PATH,NOWATER,0))
			{
				sRandDest = NOWHERE;
				continue;					// try again!
			}

			// passed all the tests,->pathing.sDestination is acceptable
			fFound = TRUE;
			pSoldier->pathing.bPathStored = TRUE;	// optimization - Ian
		}
	}
	else
	{
		// keep rolling random->pathing.sDestinations until one's satisfactory or retries used
		while ((ubTriesLeft--) && !fFound)
		{
			if (fLimited)
			{
				UINT8 ubTriesLeft2 = 128;
				do//dnl ch53 111009 This loop should increase search performance, but probably need some counter to prevent eventual endless loop
				{
					sXOffset = ((INT16)Random(sXRange)) - sMaxLeft;
					sYOffset = ((INT16)Random(sYRange)) - sMaxUp;
					sRandDest = usOrigin + sXOffset + (MAXCOL * sYOffset);
				}while(!GridNoOnVisibleWorldTile(sRandDest) && --ubTriesLeft2);
	#ifdef BETAVERSION
				if ((sRandDest < 0) || (sRandDest >= GRIDSIZE))
				{
					NumMessage("RandDestWithinRange: ERROR - Gridno out of range! = ",sRandDest);
					sRandDest = random(GRIDSIZE);
				}
	#endif
			}
			else
			{
				UINT8 ubTriesLeft2 = 128;
				do//dnl ch53 111009 This loop should increase search performance, but probably need some counter to prevent eventual endless loop
				{
					sRandDest = PreRandom(GRIDSIZE);
				}while(!GridNoOnVisibleWorldTile(sRandDest) && --ubTriesLeft2);
			}

			if ( usRoom && InARoom( sRandDest, &usTempRoom ) && usTempRoom != usRoom )
			{
				// outside of room available for patrol!
				sRandDest = NOWHERE;
				continue;
			}

			// sevenfm: avoid staying at doors
			if( pSoldier->aiData.bAlertStatus < STATUS_RED &&
				pSoldier->pathing.bLevel == 0 &&
				CheckDoorNearGridno(sRandDest) )
			{
				sRandDest = NOWHERE;
				continue;
			}

			// sevenfm: avoid going too close to known bombs
			if( FindBombNearby(pSoldier, sRandDest, DAY_VISION_RANGE/8, FALSE ) )
			{
				sRandDest = NOWHERE;
				continue;
			}

			// sevenfm: don't go too far from closest not SEEKENEMY friend if alert is not raised yet
			if( pSoldier->aiData.bAlertStatus < STATUS_RED &&
				DistanceToClosestNotSeekEnemyFriend(pSoldier, sRandDest) > DAY_VISION_RANGE )
			{
				continue;
			}

			if (!LegalNPCDestination(pSoldier,sRandDest,ENSURE_PATH,NOWATER,0))
			{
				sRandDest = NOWHERE;
				continue;					// try again!
			}

			// passed all the tests,->pathing.sDestination is acceptable
			fFound = TRUE;
			pSoldier->pathing.bPathStored = TRUE;	// optimization - Ian
		}
	}

	return(sRandDest); // defaults to NOWHERE
}

INT32 ClosestReachableDisturbance(SOLDIERTYPE *pSoldier, BOOLEAN * pfChangeLevel)
{
	INT32		*psLastLoc, *pusNoiseGridNo;
	INT8		*pbLastLevel;
	INT32		sGridNo = -1;
	INT8		bLevel, bClosestLevel = -1;
	BOOLEAN		fClimbingNecessary, fClosestClimbingNecessary = FALSE;
	INT32		iPathCost;
	INT32		sClosestDisturbance = NOWHERE;
	UINT32		uiLoop;
	UINT32		closestConscious = NOWHERE, closestUnconscious = NOWHERE;
	INT32		iShortestPath = 1000;
	INT32		iShortestPathConscious = 1000, iShortestPathUnconscious = 1000;
	UINT8		*pubNoiseVolume;
	INT8		*pbNoiseLevel;
	INT8		*pbPersOL, *pbPublOL;
	INT32		sClimbGridNo;
	SOLDIERTYPE *pOpponent;
	SOLDIERTYPE	*pClosestOpponent = NULL;
	INT32		sDistToEnemy, sDistToClosestEnemy = 10000;

	// sevenfm: safety check
	if (pfChangeLevel)
	{
		*pfChangeLevel = FALSE;
	}

	pubNoiseVolume = &gubPublicNoiseVolume[pSoldier->bTeam];
	pusNoiseGridNo = &gsPublicNoiseGridNo[pSoldier->bTeam];
	pbNoiseLevel = &gbPublicNoiseLevel[pSoldier->bTeam];

	// hang pointers at start of this guy's personal and public opponent opplists
	//	pbPersOL = &pSoldier->aiData.bOppList[0];
	//	pbPublOL = &(gbPublicOpplist[pSoldier->bTeam][0]);
	//	psLastLoc = &(gsLastKnownOppLoc[pSoldier->ubID][0]);

	// look through this man's personal & public opplists for opponents known
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[uiLoop];

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpponent)
		{
			continue;			// next merc
		}

		// if this merc is neutral/on same side, he's not an opponent
		if (CONSIDERED_NEUTRAL(pSoldier, pOpponent) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;			// next merc
		}

		// silversurfer: ignore empty vehicles
		if (pOpponent->ubWhatKindOfMercAmI == MERC_TYPE__VEHICLE && GetNumberInVehicle(pOpponent->bVehicleID) == 0)
		{
			continue;
		}

		pbPersOL = pSoldier->aiData.bOppList + pOpponent->ubID;
		pbPublOL = gbPublicOpplist[pSoldier->bTeam] + pOpponent->ubID;
		psLastLoc = gsLastKnownOppLoc[pSoldier->ubID] + pOpponent->ubID;
		pbLastLevel = gbLastKnownOppLevel[pSoldier->ubID] + pOpponent->ubID;

		// if this opponent is unknown personally and publicly
		if ((*pbPersOL == NOT_HEARD_OR_SEEN) && (*pbPublOL == NOT_HEARD_OR_SEEN))
		{
			continue;			// next merc
		}

		// this is possible if get here from BLACK AI in one of those rare
		// instances when we couldn't get a meaningful shot off at a guy in sight
		/*if ((*pbPersOL == SEEN_CURRENTLY) && (pOpponent->stats.bLife >= OKLIFE))
		{
		// don't allow this to return any valid values, this guy remains a
		// serious threat and the last thing we want to do is approach him!
		return(NOWHERE);
		}*/

		// if personal knowledge is more up to date or at least equal
		if ((gubKnowledgeValue[*pbPublOL - OLDEST_HEARD_VALUE][*pbPersOL - OLDEST_HEARD_VALUE] > 0) ||
			(*pbPersOL == *pbPublOL))
		{
			// using personal knowledge, obtain opponent's "best guess" gridno
			sGridNo = *psLastLoc;
			bLevel = *pbLastLevel;
		}
		else
		{
			// using public knowledge, obtain opponent's "best guess" gridno
			sGridNo = gsPublicLastKnownOppLoc[pSoldier->bTeam][pOpponent->ubID];
			bLevel = gbPublicLastKnownOppLevel[pSoldier->bTeam][pOpponent->ubID];
		}

		// if we are standing at that gridno (!, obviously our info is old...)
		if (sGridNo == pSoldier->sGridNo)
		{
			continue;			// next merc
		}

		if (TileIsOutOfBounds(sGridNo))
		{
			// huh?
			continue;
		}

		// sevenfm: if soldier is zombie and he cannot climb, skip location
		if (pSoldier->IsZombie() && pSoldier->pathing.bLevel != bLevel && !gGameExternalOptions.fZombieCanClimb)
		{
			continue;
		}

		// sevenfm: zombies do not attack vehicles
		if (pSoldier->IsZombie() && (TANK(pOpponent) || (pOpponent->flags.uiStatusFlags & SOLDIER_VEHICLE)))
		{
			continue;
		}

		// sevenfm: when in deep water, skip opponents in deep water
		if (DeepWater(pSoldier->sGridNo, pSoldier->pathing.bLevel) && DeepWater(sGridNo, bLevel))
		{
			continue;
		}

		// sevenfm: if we found reachable enemy, check other enemies only if they are closer
		sDistToEnemy = PythSpacesAway(pSoldier->sGridNo, sGridNo);
		if (sDistToEnemy < sDistToClosestEnemy || TileIsOutOfBounds(sClosestDisturbance))
		{
			iPathCost = EstimatePathCostToLocation(pSoldier, sGridNo, bLevel, FALSE, &fClimbingNecessary, &sClimbGridNo);

			// if we can get there and it's first reachable enemy or closer than other known enemies
			if (iPathCost != 0 &&
				(!pClosestOpponent || pClosestOpponent->stats.bLife < OKLIFE || pOpponent->stats.bLife >= OKLIFE) &&
				(TileIsOutOfBounds(sClosestDisturbance) ||
				iPathCost < iShortestPath ||
				pClosestOpponent && !pClosestOpponent->IsZombie() && pClosestOpponent->stats.bLife < OKLIFE && pOpponent->stats.bLife >= OKLIFE))
			{
				if (fClimbingNecessary)
				{
					sClosestDisturbance = sClimbGridNo;
				}
				else
				{
					sClosestDisturbance = sGridNo;
				}

				pClosestOpponent = pOpponent;
				sDistToClosestEnemy = sDistToEnemy;
				iShortestPath = iPathCost;
				fClosestClimbingNecessary = fClimbingNecessary;
			}
		}
	}

	// if any "misc. noise" was also heard recently	
	if (!TileIsOutOfBounds(pSoldier->aiData.sNoiseGridno) && pSoldier->aiData.sNoiseGridno != sClosestDisturbance)
	{
		// test this gridno, too
		sGridNo = pSoldier->aiData.sNoiseGridno;
		bLevel = pSoldier->bNoiseLevel;

		// if we are there (at the noise gridno)
		if (sGridNo == pSoldier->sGridNo)
		{
			for (uiLoop = 0; uiLoop<guiNumMercSlots; uiLoop++)//dnl ch58 160813
			{
				pOpponent = MercSlots[uiLoop];
				if (pOpponent && pSoldier->bSide == pOpponent->bSide && pSoldier->ubID != pOpponent->ubID&& pSoldier->aiData.sNoiseGridno == pOpponent->aiData.sNoiseGridno)
				{
					pOpponent->aiData.sNoiseGridno = NOWHERE;// Erase for all from the same team as it not useful anymore, this will avoid others to check already tested location
					pOpponent->aiData.ubNoiseVolume = 0;
				}
			}
			pSoldier->aiData.sNoiseGridno = NOWHERE;		// wipe it out, not useful anymore
			pSoldier->aiData.ubNoiseVolume = 0;
		}
		else
		{
			// get the AP cost to get to the location of the noise
			iPathCost = EstimatePathCostToLocation(pSoldier, sGridNo, bLevel, FALSE, &fClimbingNecessary, &sClimbGridNo);
			// if we can get there
			// sevenfm: only if we don't know enemy location or noise source is close and we have not seen enemy recently
			if (iPathCost != 0 &&
				!AICheckIsFlanking(pSoldier) &&
				(TileIsOutOfBounds(sClosestDisturbance) || iPathCost < iShortestPath && !GuySawEnemy(pSoldier)))
			{
				if (fClimbingNecessary)
				{
					sClosestDisturbance = sClimbGridNo;
				}
				else
				{
					sClosestDisturbance = sGridNo;
				}
				iShortestPath = iPathCost;
				fClosestClimbingNecessary = fClimbingNecessary;
			}
		}
	}


	// if any PUBLIC "misc. noise" was also heard recently	
	if (!TileIsOutOfBounds(*pusNoiseGridNo) && *pusNoiseGridNo != sClosestDisturbance)
	{
		// test this gridno, too
		sGridNo = *pusNoiseGridNo;
		bLevel = *pbNoiseLevel;

		// if we are not NEAR the noise gridno...
		if (pSoldier->pathing.bLevel != bLevel || PythSpacesAway(pSoldier->sGridNo, sGridNo) >= 6 || SoldierTo3DLocationLineOfSightTest(pSoldier, sGridNo, bLevel, 0, FALSE, NO_DISTANCE_LIMIT) == 0)
			// if we are NOT there (at the noise gridno)
			//	if (sGridNo != pSoldier->sGridNo)
		{
			// get the AP cost to get to the location of the noise
			iPathCost = EstimatePathCostToLocation(pSoldier, sGridNo, bLevel, FALSE, &fClimbingNecessary, &sClimbGridNo);
			// if we can get there
			// sevenfm: only if we don't know enemy location or noise source is close and we have not seen enemy recently
			if (iPathCost != 0 &&
				!AICheckIsFlanking(pSoldier) &&
				(TileIsOutOfBounds(sClosestDisturbance) || iPathCost < iShortestPath && !GuySawEnemy(pSoldier)))
			{
				if (fClimbingNecessary)
				{
					sClosestDisturbance = sClimbGridNo;
				}
				else
				{
					sClosestDisturbance = sGridNo;
				}
				iShortestPath = iPathCost;
				fClosestClimbingNecessary = fClimbingNecessary;
			}
		}
		else
		{
			// degrade our public noise a bit
			//dnl ch58 160813
			//*pusNoiseGridNo -= 2;
			if (*pubNoiseVolume > 1)
				(*pubNoiseVolume)--;
		}
	}

#ifdef DEBUGDECISIONS	
	if (!TileIsOutOfBounds(sClosestDisturbance))
	{
		AINumMessage("CLOSEST DISTURBANCE is at gridno ", sClosestDisturbance);
	}
#endif

	// sevenfm: safety check
	if (pfChangeLevel)
	{
		*pfChangeLevel = fClosestClimbingNecessary;
	}

	return(sClosestDisturbance);
}

INT32 ClosestKnownOpponent(SOLDIERTYPE *pSoldier, INT32 * psGridNo, INT8 * pbLevel)
{
	INT32 *psLastLoc, sGridNo, sClosestOpponent = NOWHERE;
	UINT32 uiLoop;
	INT32 iRange, iClosestRange = 1500;
	INT8	*pbPersOL, *pbPublOL;
	INT8	bLevel, bClosestLevel;
	SOLDIERTYPE *pOpponent;
	SOLDIERTYPE *pClosestOpponent = NULL;

	bClosestLevel = -1;

	// NOTE: THIS FUNCTION ALLOWS RETURN OF UNCONSCIOUS AND UNREACHABLE OPPONENTS
	psLastLoc = &(gsLastKnownOppLoc[pSoldier->ubID][0]);

	// hang pointers at start of this guy's personal and public opponent opplists
	pbPersOL = &pSoldier->aiData.bOppList[0];
	pbPublOL = &(gbPublicOpplist[pSoldier->bTeam][0]);

	// look through this man's personal & public opplists for opponents known
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[uiLoop];

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpponent)
		{
			continue;			// next merc
		}

		// if this merc is neutral/on same side, he's not an opponent
		if (CONSIDERED_NEUTRAL(pSoldier, pOpponent) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;			// next merc
		}

		// silversurfer: ignore empty vehicles
		if (pOpponent->ubWhatKindOfMercAmI == MERC_TYPE__VEHICLE && GetNumberInVehicle(pOpponent->bVehicleID) == 0)
			continue;

		// Special stuff for Carmen the bounty hunter
		if (pSoldier->aiData.bAttitude == ATTACKSLAYONLY && pOpponent->ubProfile != SLAY)
		{
			continue;	// next opponent
		}

		pbPersOL = pSoldier->aiData.bOppList + pOpponent->ubID;
		pbPublOL = gbPublicOpplist[pSoldier->bTeam] + pOpponent->ubID;
		psLastLoc = gsLastKnownOppLoc[pSoldier->ubID] + pOpponent->ubID;

		// if this opponent is unknown personally and publicly
		if ((*pbPersOL == NOT_HEARD_OR_SEEN) && (*pbPublOL == NOT_HEARD_OR_SEEN))
		{
			continue;			// next merc
		}

		// if personal knowledge is more up to date or at least equal
		if ((gubKnowledgeValue[*pbPublOL - OLDEST_HEARD_VALUE][*pbPersOL - OLDEST_HEARD_VALUE] > 0) ||
			(*pbPersOL == *pbPublOL))
		{
			// using personal knowledge, obtain opponent's "best guess" gridno
			sGridNo = gsLastKnownOppLoc[pSoldier->ubID][pOpponent->ubID];
			bLevel = gbLastKnownOppLevel[pSoldier->ubID][pOpponent->ubID];
		}
		else
		{
			// using public knowledge, obtain opponent's "best guess" gridno
			sGridNo = gsPublicLastKnownOppLoc[pSoldier->bTeam][pOpponent->ubID];
			bLevel = gbPublicLastKnownOppLevel[pSoldier->bTeam][pOpponent->ubID];
		}

		// if we are standing at that gridno(!, obviously our info is old...)
		if (sGridNo == pSoldier->sGridNo)
		{
			continue;			// next merc
		}

		// this function is used only for turning towards closest opponent or changing stance
		// as such, if they AI is in a building,
		// we should ignore people who are on the roof of the same building as the AI
		if ((bLevel != pSoldier->pathing.bLevel) && SameBuilding(pSoldier->sGridNo, sGridNo))
		{
			continue;
		}

		// I hope this will be good enough; otherwise we need a fractional/world-units-based 2D distance function
		//sRange = PythSpacesAway( pSoldier->sGridNo, sGridNo);
		iRange = GetRangeInCellCoordsFromGridNoDiff(pSoldier->sGridNo, sGridNo);

		if (sClosestOpponent == NOWHERE ||
			iRange < iClosestRange ||
			pClosestOpponent && !pClosestOpponent->IsZombie() && !(pSoldier->flags.uiStatusFlags & SOLDIER_BOXER) && pClosestOpponent->stats.bLife < OKLIFE && pOpponent->stats.bLife >= OKLIFE)
		{
			iClosestRange = iRange;
			sClosestOpponent = sGridNo;
			bClosestLevel = bLevel;
			pClosestOpponent = pOpponent;
		}
	}

#ifdef DEBUGDECISIONS	
	if (!TileIsOutOfBounds(sClosestOpponent))
	{
		AINumMessage("CLOSEST OPPONENT is at gridno ", sClosestOpponent);
	}
#endif

	if (psGridNo)
	{
		*psGridNo = sClosestOpponent;
	}
	if (pbLevel)
	{
		*pbLevel = bClosestLevel;
	}
	return(sClosestOpponent);
}

INT32 ClosestSeenOpponent(SOLDIERTYPE *pSoldier, INT32 * psGridNo, INT8 * pbLevel)
{
	INT32 sGridNo, sClosestOpponent = NOWHERE;
	UINT32 uiLoop;
	INT32 iRange, iClosestRange = 1500;
	INT8	*pbPersOL;
	INT8	bLevel, bClosestLevel;
	SOLDIERTYPE * pOpp;

	bClosestLevel = -1;

	// look through this man's personal & public opplists for opponents known
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpp = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpp)
		{
			continue;			// next merc
		}

		// if this merc is neutral/on same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpp ) || (pSoldier->bSide == pOpp->bSide))
		{
			continue;			// next merc
		}

		// silversurfer: ignore empty vehicles
		if ( pOpp->ubWhatKindOfMercAmI == MERC_TYPE__VEHICLE && GetNumberInVehicle( pOpp->bVehicleID ) == 0 )
			continue;

		// Special stuff for Carmen the bounty hunter
		if (pSoldier->aiData.bAttitude == ATTACKSLAYONLY && pOpp->ubProfile != SLAY)
		{
			continue;	// next opponent
		}

		pbPersOL = pSoldier->aiData.bOppList + pOpp->ubID;

		// if this opponent is not seen personally
		if (*pbPersOL != SEEN_CURRENTLY)
		{
			continue;			// next merc
		}

		// since we're dealing with seen people, use exact gridnos
		sGridNo = pOpp->sGridNo;
		bLevel = pOpp->pathing.bLevel;

		// if we are standing at that gridno(!, obviously our info is old...)
		if (sGridNo == pSoldier->sGridNo)
		{
			continue;			// next merc
		}

		// this function is used only for turning towards closest opponent or changing stance
		// as such, if they AI is in a building,
		// we should ignore people who are on the roof of the same building as the AI
		if ( (bLevel != pSoldier->pathing.bLevel) && SameBuilding( pSoldier->sGridNo, sGridNo ) )
		{
			continue;
		}

		// I hope this will be good enough; otherwise we need a fractional/world-units-based 2D distance function
		//sRange = PythSpacesAway( pSoldier->sGridNo, sGridNo);
		iRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, sGridNo );

		if (iRange < iClosestRange)
		{
			iClosestRange = iRange;
			sClosestOpponent = sGridNo;
			bClosestLevel = bLevel;
		}
	}

#ifdef DEBUGDECISIONS	
	if (!TileIsOutOfBounds(sClosestOpponent))
	{
		AINumMessage("CLOSEST OPPONENT is at gridno ",sClosestOpponent);
	}
#endif

	if (psGridNo)
	{
		*psGridNo = sClosestOpponent;
	}
	if (pbLevel)
	{
		*pbLevel = bClosestLevel;
	}
	return( sClosestOpponent );
}


// special variant with a minor twist
INT32 ClosestSeenOpponentWithRoof(SOLDIERTYPE *pSoldier, INT32 * psGridNo, INT8 * pbLevel)
{
	INT32 sGridNo, sClosestOpponent = NOWHERE;
	UINT32 uiLoop;
	INT32 iRange, iClosestRange = 1500;
	INT8	*pbPersOL;
	INT8	bLevel, bClosestLevel;
	SOLDIERTYPE * pOpp;

	bClosestLevel = -1;

	// look through this man's personal & public opplists for opponents known
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpp = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpp)
		{
			continue;			// next merc
		}

		// if this merc is neutral/on same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpp ) || (pSoldier->bSide == pOpp->bSide))
		{
			continue;			// next merc
		}

		// silversurfer: ignore empty vehicles
		if ( pOpp->ubWhatKindOfMercAmI == MERC_TYPE__VEHICLE && GetNumberInVehicle( pOpp->bVehicleID ) == 0 )
			continue;

		// Special stuff for Carmen the bounty hunter
		if (pSoldier->aiData.bAttitude == ATTACKSLAYONLY && pOpp->ubProfile != SLAY)
		{
			continue;	// next opponent
		}

		pbPersOL = pSoldier->aiData.bOppList + pOpp->ubID;

		// if this opponent is not seen personally
		if (*pbPersOL != SEEN_CURRENTLY)
		{
			continue;			// next merc
		}

		// since we're dealing with seen people, use exact gridnos
		sGridNo = pOpp->sGridNo;
		bLevel = pOpp->pathing.bLevel;

		// if we are standing at that gridno(!, obviously our info is old...)
		if (sGridNo == pSoldier->sGridNo)
		{
			continue;			// next merc
		}

		// special: allow zombies to also find opponents on a roof
		// otherwise they have will never decide to climb a roof while they are seeing an enemy
		// this function is used only for turning towards closest opponent or changing stance
		// as such, if they AI is in a building,
		// we should ignore people who are on the roof of the same building as the AI
		/*if ( !pSoldier->IsZombie() && (bLevel != pSoldier->pathing.bLevel) && SameBuilding( pSoldier->sGridNo, sGridNo ) )
		{
			continue;
		}*/

		// I hope this will be good enough; otherwise we need a fractional/world-units-based 2D distance function
		//sRange = PythSpacesAway( pSoldier->sGridNo, sGridNo);
		iRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, sGridNo );

		if (iRange < iClosestRange)
		{
			iClosestRange = iRange;
			sClosestOpponent = sGridNo;
			bClosestLevel = bLevel;
		}
	}

#ifdef DEBUGDECISIONS	
	if (!TileIsOutOfBounds(sClosestOpponent))
	{
		AINumMessage("CLOSEST OPPONENT is at gridno ",sClosestOpponent);
	}
#endif

	if (psGridNo)
	{
		*psGridNo = sClosestOpponent;
	}
	if (pbLevel)
	{
		*pbLevel = bClosestLevel;
	}
	return( sClosestOpponent );
}

INT32 ClosestPC( SOLDIERTYPE *pSoldier, INT32 * psDistance )
{
	// used by NPCs... find the closest PC

	// NOTE: skips EPCs!

	UINT8 ubLoop;
	SOLDIERTYPE		*pTargetSoldier;
	INT32					sMinDist = WORLD_MAX;
	INT32					sDist;
	INT32					sGridNo = NOWHERE;

	// Loop through all mercs on player team
	for (ubLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID; ubLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID; ubLoop++)
	{
		pTargetSoldier = Menptr + ubLoop;

		if (!pTargetSoldier->bActive || !pTargetSoldier->bInSector)
		{
			continue;
		}

		// if not conscious, skip him
		if (pTargetSoldier->stats.bLife < OKLIFE)
		{
		continue;
		}

		if ( AM_AN_EPC( pTargetSoldier ) )
		{
			continue;
		}

		sDist = PythSpacesAway(pSoldier->sGridNo,pTargetSoldier->sGridNo);

		// if this PC is not visible to the soldier, then add a penalty to the distance
		// so that we weight in favour of visible mercs
		if ( pTargetSoldier->bTeam != pSoldier->bTeam && pSoldier->aiData.bOppList[ ubLoop ] != SEEN_CURRENTLY )
		{
			sDist += 10;
		}

		if (sDist < sMinDist)
		{
			sMinDist = sDist;
			sGridNo = pTargetSoldier->sGridNo;
		}
	}

	if ( psDistance )
	{
		*psDistance = sMinDist;
	}

	return( sGridNo );
}

// Flugente: like ClosestPC(...), but does not account for covert or not visible mercs
INT32 ClosestUnDisguisedPC( SOLDIERTYPE *pSoldier, INT32 * psDistance )
{
	// used by NPCs... find the closest PC
	// NOTE: skips EPCs!

	UINT8 ubLoop;
	SOLDIERTYPE		*pTargetSoldier;
	INT32					sMinDist = WORLD_MAX;
	INT32					sDist;
	INT32					sGridNo = NOWHERE;

	// Loop through all mercs on player team
	ubLoop = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;
	for ( ; ubLoop <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; ubLoop++)
	{
		pTargetSoldier = Menptr + ubLoop;

		if (!pTargetSoldier->bActive || !pTargetSoldier->bInSector)
			continue;
				
		// if not conscious, skip him
		if (pTargetSoldier->stats.bLife < OKLIFE)
			continue;

		if ( AM_AN_EPC( pTargetSoldier ) )
			continue;

		if ( pTargetSoldier->usSoldierFlagMask & (SOLDIER_COVERT_CIV|SOLDIER_COVERT_SOLDIER) )
			continue;

		sDist = PythSpacesAway(pSoldier->sGridNo,pTargetSoldier->sGridNo);

		// if this PC is not visible to the soldier, then add a penalty to the distance
		// so that we weight in favour of visible mercs
		if ( pTargetSoldier->bTeam != pSoldier->bTeam && pSoldier->aiData.bOppList[ ubLoop ] != SEEN_CURRENTLY )
			continue;

		if (sDist < sMinDist)
		{
			sMinDist = sDist;
			sGridNo = pTargetSoldier->sGridNo;
		}
	}

	if ( psDistance )
	{
		*psDistance = sMinDist;
	}

	return( sGridNo );
}

INT32 FindClosestClimbPointAvailableToAI( SOLDIERTYPE * pSoldier, INT32 sStartGridNo, INT32 sDesiredGridNo, BOOLEAN fClimbUp )
{
	INT32 sGridNo;
	INT32	sRoamingOrigin;
	INT16	sRoamingRange;

	// sevenfm: safety check
	if(!pSoldier)
	{
		return NOWHERE;
	}

	if ( pSoldier->flags.uiStatusFlags & SOLDIER_PC )
	{
		sRoamingOrigin = pSoldier->sGridNo;
		sRoamingRange = MAX_ROAMING_RANGE;
	}
	else
	{
		sRoamingRange = RoamingRange( pSoldier, &sRoamingOrigin );
	}

	// since climbing necessary involves going an extra tile, we compare against 1 less than the roam range...
	// or add 1 to the distance to the climb point

	sGridNo = FindClosestClimbPoint( pSoldier, sStartGridNo, sDesiredGridNo, fClimbUp );


	if ( PythSpacesAway( sRoamingOrigin, sGridNo ) + 1 > sRoamingRange )
	{
		return( NOWHERE );
	}
	else
	{
		return( sGridNo );
	}
}

BOOLEAN ClimbingNecessary( SOLDIERTYPE * pSoldier, INT32 sDestGridNo, INT8 bDestLevel )
{
	if (pSoldier->pathing.bLevel == bDestLevel)
	{
		if ( (pSoldier->pathing.bLevel == 0) || ( gubBuildingInfo[ pSoldier->sGridNo ] == gubBuildingInfo[ sDestGridNo ] ) )
		{
			return( FALSE );
		}
		else // different buildings!
		{
			return( TRUE );
		}
	}
	else
	{
		return( TRUE );
	}
}

INT32 GetInterveningClimbingLocation( SOLDIERTYPE * pSoldier, INT32 sDestGridNo, INT8 bDestLevel, BOOLEAN * pfClimbingNecessary )
{
	if (pSoldier->pathing.bLevel == bDestLevel)
	{
		if ( (pSoldier->pathing.bLevel == 0) || ( gubBuildingInfo[ pSoldier->sGridNo ] == gubBuildingInfo[ sDestGridNo ] ) )
		{
			// on ground or same building... normal!
			*pfClimbingNecessary = FALSE;
			return( NOWHERE );
		}
		else
		{
			// different buildings!
			// yes, pass in same gridno twice... want closest climb-down spot for building we are on!
			*pfClimbingNecessary = TRUE;
			return( FindClosestClimbPointAvailableToAI( pSoldier, pSoldier->sGridNo, pSoldier->sGridNo, FALSE ) );
		}
	}
	else
	{
		*pfClimbingNecessary = TRUE;
		// different levels
		if (pSoldier->pathing.bLevel == 0)
		{
			// got to go UP onto building
			return( FindClosestClimbPointAvailableToAI( pSoldier, pSoldier->sGridNo, sDestGridNo, TRUE ) );
		}
		else
		{
			// got to go DOWN off building
			return( FindClosestClimbPointAvailableToAI( pSoldier, pSoldier->sGridNo, pSoldier->sGridNo, FALSE ) );
		}
	}
}

INT16 EstimatePathCostToLocation( SOLDIERTYPE * pSoldier, INT32 sDestGridNo, INT8 bDestLevel, BOOLEAN fAddCostAfterClimbingUp, BOOLEAN * pfClimbingNecessary, INT32 * psClimbGridNo )
{
	INT16	sPathCost;
	INT32 sClimbGridNo;

	if (pSoldier->pathing.bLevel == bDestLevel)
	{
		if ( (pSoldier->pathing.bLevel == 0) || ( gubBuildingInfo[ pSoldier->sGridNo ] == gubBuildingInfo[ sDestGridNo ] ) )
		{
			// on ground or same building... normal!
			sPathCost = EstimatePlotPath( pSoldier, sDestGridNo, FALSE, FALSE, FALSE, WALKING, FALSE, FALSE, 0);
			*pfClimbingNecessary = FALSE;
			*psClimbGridNo = NOWHERE;
		}
		else
		{
			// different buildings!
			// yes, pass in same gridno twice... want closest climb-down spot for building we are on!
			sClimbGridNo = FindClosestClimbPointAvailableToAI( pSoldier, sDestGridNo, pSoldier->sGridNo, FALSE );			
			if (TileIsOutOfBounds(sClimbGridNo))
			{
				sPathCost = 0;
			}
			else
			{
				sPathCost = PlotPath( pSoldier, sClimbGridNo, FALSE, FALSE, FALSE, WALKING, FALSE, FALSE, 0 );
				// sevenfm: check if we are already standing at climb gridno
				if (sPathCost != 0 || pSoldier->sGridNo == sClimbGridNo)
				{
					// add in cost of climbing down
					if (fAddCostAfterClimbingUp)
					{
						// add in cost of later climbing up, too
						sPathCost += APBPConstants[AP_CLIMBOFFROOF] + APBPConstants[AP_CLIMBROOF];
						// add in an estimate of getting there after climbing down
						sPathCost += (APBPConstants[AP_MOVEMENT_FLAT] + APBPConstants[AP_MODIFIER_WALK]) * PythSpacesAway( sClimbGridNo, sDestGridNo );
					}
					else
					{
						sPathCost += APBPConstants[AP_CLIMBOFFROOF];
						// add in an estimate of getting there after climbing down, *but not on top of roof*
						sPathCost += (APBPConstants[AP_MOVEMENT_FLAT] + APBPConstants[AP_MODIFIER_WALK]) * PythSpacesAway( sClimbGridNo, sDestGridNo ) / 2;
					}
					*pfClimbingNecessary = TRUE;
					*psClimbGridNo = sClimbGridNo;
				}
			}
		}
	}
	else
	{
		// sevenfm: check if zombie cannot climb
		if (pSoldier->IsZombie() && !gGameExternalOptions.fZombieCanClimb)
		{
			return 0;
		}

		// different levels
		if (pSoldier->pathing.bLevel == 0)
		{
			//got to go UP onto building
			sClimbGridNo = FindClosestClimbPointAvailableToAI(pSoldier, pSoldier->sGridNo, sDestGridNo, TRUE);
		}
		else
		{
			// got to go DOWN off building
			sClimbGridNo = FindClosestClimbPointAvailableToAI(pSoldier, sDestGridNo, pSoldier->sGridNo, FALSE);
		}
		
		if (TileIsOutOfBounds(sClimbGridNo))
		{
			sPathCost = 0;
		}
		else
		{
			sPathCost = PlotPath( pSoldier, sClimbGridNo, FALSE, FALSE, FALSE, WALKING, FALSE, FALSE, 0);
			// sevenfm: check if we are already standing at climb gridno
			if (sPathCost != 0 || pSoldier->sGridNo == sClimbGridNo)
			{
				// add in the cost of climbing up or down
				if (pSoldier->pathing.bLevel == 0)
				{
					// must climb up
					sPathCost += APBPConstants[AP_CLIMBROOF];
					if (fAddCostAfterClimbingUp)
					{
						// add to path a rough estimate of how far to go from the climb gridno to the friend
						// estimate walk cost
						sPathCost += (APBPConstants[AP_MOVEMENT_FLAT] + APBPConstants[AP_MODIFIER_WALK]) * PythSpacesAway( sClimbGridNo, sDestGridNo );
					}
				}
				else
				{
					// must climb down
					sPathCost += APBPConstants[AP_CLIMBOFFROOF];
					// add to path a rough estimate of how far to go from the climb gridno to the friend
					// estimate walk cost
					sPathCost += (APBPConstants[AP_MOVEMENT_FLAT] + APBPConstants[AP_MODIFIER_WALK]) * PythSpacesAway( sClimbGridNo, sDestGridNo );
				}
				*pfClimbingNecessary = TRUE;
				*psClimbGridNo = sClimbGridNo;
			}
		}
	}

	return( sPathCost );
}

BOOLEAN GuySawEnemy( SOLDIERTYPE * pSoldier, UINT8 ubMax )
{
	UINT8		uiLoop;
	SOLDIERTYPE *pOpponent;

	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpponent || !pOpponent->bActive || !pOpponent->bInSector)
		{
			continue;
		}

		// if this merc is neutral/on same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide) )
		{
			continue;
		}

		// if this guy SAW an enemy recently...
		if( pSoldier->aiData.bOppList[ pOpponent->ubID ] >= SEEN_CURRENTLY && 
			pSoldier->aiData.bOppList[ pOpponent->ubID ] <= ubMax )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}

BOOLEAN GuyHeardEnemy(SOLDIERTYPE * pSoldier, UINT8 ubMax)
{
	UINT8		uiLoop;
	SOLDIERTYPE *pOpponent;

	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[uiLoop];

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpponent || !pOpponent->bActive || !pOpponent->bInSector)
		{
			continue;
		}

		// if this merc is neutral/on same side, he's not an opponent
		if (CONSIDERED_NEUTRAL(pSoldier, pOpponent) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;
		}

		// if this guy HEARD an enemy recently...
		if (pSoldier->aiData.bOppList[pOpponent->ubID] >= ubMax &&
			pSoldier->aiData.bOppList[pOpponent->ubID] <= HEARD_THIS_TURN)
		{
			return(TRUE);
		}
	}

	return(FALSE);
}

INT32 ClosestReachableFriendInTrouble(SOLDIERTYPE *pSoldier, BOOLEAN * pfClimbingNecessary)
{
	UINT32 uiLoop;
	INT32 sPathCost, sClosestFriend = NOWHERE, sShortestPath = 1000, sClimbGridNo;
	BOOLEAN fClimbingNecessary, fClosestClimbingNecessary = FALSE;
	SOLDIERTYPE *pFriend;

	// civilians don't really have any "friends", so they don't bother with this
	if (PTR_CIVILIAN)
	{
		return(NOWHERE);
	}

	// consider every friend of this soldier (locations assumed to be known)
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pFriend = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, or dead
		if (!pFriend)
		{
			continue;			// next merc
		}

		// if this merc is neutral or NOT on the same side, he's not a friend
		if (pFriend->aiData.bNeutral || (pSoldier->bSide != pFriend->bSide))
		{
			continue;			// next merc
		}

		// if this "friend" is actually US
		if (pFriend->ubID == pSoldier->ubID)
		{
			continue;			// next merc
		}

		// CJC: restrict "last one to radio" to only if that guy saw us this turn or last turn

		// if this friend is not under fire, and isn't the last one to radio
		// sevenfm: also help if friend has more opponents than friends nearby
		/*if (!(	pFriend->aiData.bUnderFire ||
				(pFriend->ubID == gTacticalStatus.Team[pFriend->bTeam].ubLastMercToRadio && GuySawEnemyThisTurnOrBefore( pFriend ) ) ||
				CountSeenEnemiesLastTurn(pFriend) > CountNearbyFriendlies(pSoldier, pSoldier->sGridNo, DAY_VISION_RANGE/4) ) )*/
		// sevenfm: help if friend has more recently seen opponents than friends nearby
		if( !( CountSeenEnemiesLastTurn(pFriend) > CountNearbyFriends(pFriend, pFriend->sGridNo, DAY_VISION_RANGE/4) ) )
		{
			continue;			// next merc
		}

		// if we're already neighbors
		if (SpacesAway(pSoldier->sGridNo,pFriend->sGridNo) == 1)
		{
			continue;			// next merc
		}

		// get the AP cost to go to this friend's gridno
		sPathCost = EstimatePathCostToLocation( pSoldier, pFriend->sGridNo, pFriend->pathing.bLevel, TRUE, &fClimbingNecessary, &sClimbGridNo );

		// sevenfm: try to help friend that have more opponents than nearby friends
		// disabled
		//sPathCost = sPathCost * (1 + CountNearbyFriendlies(pSoldier, pSoldier->sGridNo, DAY_VISION_RANGE/4)) / (1 + CountSeenEnemiesLastTurn(pFriend));

		// if we can get there
		if (sPathCost != 0)
		{
			//sprintf(tempstr,"Path cost to friend %s's location is %d",pFriend->name,pathCost);
			//PopMessage(tempstr);

			if (sPathCost < sShortestPath)
			{
				if (fClimbingNecessary)
				{
					sClosestFriend = sClimbGridNo;
				}
				else
				{
					sClosestFriend = pFriend->sGridNo;
				}

				sShortestPath = sPathCost;
				fClosestClimbingNecessary = fClimbingNecessary;
			}
		}
	}

#ifdef DEBUGDECISIONS	
	if (!TileIsOutOfBounds(sClosestFriend))
	{
		AINumMessage("CLOSEST FRIEND is at gridno ",sClosestFriend);
	}
#endif

	*pfClimbingNecessary = fClosestClimbingNecessary;
	return(sClosestFriend);
}

INT16 DistanceToClosestFriend( SOLDIERTYPE * pSoldier )
{
	// find the distance to the closest person on the same team
	UINT8 ubLoop;
	SOLDIERTYPE		*pTargetSoldier;
	INT16					sMinDist = 1000;
	INT16					sDist;

	// Loop through all mercs on player team
	ubLoop = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID;

	for ( ; ubLoop <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; ubLoop++ )
	{
		if (ubLoop == pSoldier->ubID)
		{
			// same guy - continue!
			continue;
		}

		pTargetSoldier = Menptr + ubLoop;


		if (!CONSIDERED_ALLIES(pSoldier, pTargetSoldier))
		{
			continue;
		}


		if ( pSoldier->bActive && pSoldier->bInSector )
		{
			if (!pTargetSoldier->bActive || !pTargetSoldier->bInSector)
			{
				continue;
			}
			// if not conscious, skip him
			else if (pTargetSoldier->stats.bLife < OKLIFE)
			{
				continue;
			}
		}
		else
		{
			// compare sector #s
			if ( (pSoldier->sSectorX != pTargetSoldier->sSectorX) ||
				(pSoldier->sSectorY != pTargetSoldier->sSectorY) ||
				(pSoldier->bSectorZ != pTargetSoldier->bSectorZ) )
			{
				continue;
			}
			else if (pTargetSoldier->stats.bLife < OKLIFE)
			{
				continue;
			}
			else
			{
				// well there's someone who could be near
				return( 1 );
			}
		}

		sDist = SpacesAway(pSoldier->sGridNo,pTargetSoldier->sGridNo);

		if (sDist < sMinDist)
		{
			sMinDist = sDist;
		}
	}

	return( sMinDist );
}

BOOLEAN InWaterGasOrSmoke( SOLDIERTYPE *pSoldier, INT32 sGridNo )
{
	if (WaterTooDeepForAttacks( sGridNo, pSoldier->pathing.bLevel ))
	{
		return(TRUE);
	}

	// smoke
	if (gpWorldLevelData[sGridNo].ubExtFlags[ pSoldier->pathing.bLevel ] & MAPELEMENT_EXT_SMOKE)
	{
		return TRUE;
	}

	return InGas( pSoldier, sGridNo );
}

BOOLEAN InGasOrSmoke( SOLDIERTYPE *pSoldier, INT32 sGridNo )
{
	// smoke
	if (gpWorldLevelData[sGridNo].ubExtFlags[ pSoldier->pathing.bLevel ] & (MAPELEMENT_EXT_SMOKE|MAPELEMENT_EXT_SIGNAL_SMOKE) )	
		return TRUE;

	return InGas(pSoldier,sGridNo);
}


INT16 InWaterOrGas(SOLDIERTYPE *pSoldier, INT32 sGridNo)
{
	if (WaterTooDeepForAttacks( sGridNo, pSoldier->pathing.bLevel ))
	{
		return(TRUE);
	}

	return (INT16)InGas( pSoldier, sGridNo );
}

BOOLEAN InGasSpot(SOLDIERTYPE *pSoldier, INT32 sGridNo, INT8 bLevel)
{
	CHECKF(pSoldier);

	if (TileIsOutOfBounds(sGridNo))
		return FALSE;

	// tear gas
	if ((gpWorldLevelData[sGridNo].ubExtFlags[bLevel] & MAPELEMENT_EXT_TEARGAS) &&
		!DoesSoldierWearGasMask(pSoldier))
	{
		return(TRUE);
	}
	// fire/creature/mustard gas
	// sevenfm: avoid mustard gas even when wearing gas mask
	if (gpWorldLevelData[sGridNo].ubExtFlags[bLevel] & (MAPELEMENT_EXT_BURNABLEGAS | MAPELEMENT_EXT_CREATUREGAS | MAPELEMENT_EXT_MUSTARDGAS))
	{
		return(TRUE);
	}
	return FALSE;
}

BOOLEAN InGas(SOLDIERTYPE *pSoldier, INT32 sGridNo)
{
	CHECKF(pSoldier);

	if (TileIsOutOfBounds(sGridNo))
		return FALSE;

	if (InGasSpot(pSoldier, sGridNo, pSoldier->pathing.bLevel))
	{
		return TRUE;
	}

	//WarmSteel - One square away from gas is still considered in gas, because it could expand any moment.
	//Note: this only works for gas that expands with one tile, but hey it's better than nothing!
	int iNeighbourGridNo;
	for (int iDir = 0; iDir < NUM_WORLD_DIRECTIONS; ++iDir)
	{
		iNeighbourGridNo = sGridNo + DirectionInc(iDir);

		if (!TileIsOutOfBounds(iNeighbourGridNo) && InGasSpot(pSoldier, iNeighbourGridNo, pSoldier->pathing.bLevel))
		{
			return TRUE;
		}
	}

	return(FALSE);
}

BOOLEAN WearGasMaskIfAvailable( SOLDIERTYPE * pSoldier )
{
	INT8		bSlot, bNewSlot;

	bSlot = FindGasMask( pSoldier );
	if ( bSlot == NO_SLOT )
	{
		return( FALSE );
	}
	if ( bSlot == HEAD1POS || bSlot == HEAD2POS || bSlot == HELMETPOS )
	{
		return( FALSE );
	}
	if ( pSoldier->inv[ HEAD1POS ].exists() == false )
	{
		bNewSlot = HEAD1POS;
	}
	else if ( pSoldier->inv[ HEAD2POS ].exists() == false )
	{
		bNewSlot = HEAD2POS;
	}
	else
	{
		// screw it, going in position 1 anyhow
		bNewSlot = HEAD1POS;
	}

	RearrangePocket( pSoldier, bSlot, bNewSlot, TRUE );
	return( TRUE );
}

BOOLEAN InLightAtNight( INT32 sGridNo, INT8 bLevel )
{
	UINT8 ubBackgroundLightLevel;

	// do not consider us to be "in light" if we're in an underground sector
	if ( gbWorldSectorZ > 0 )
	{
		return( FALSE );
	}

	ubBackgroundLightLevel = GetTimeOfDayAmbientLightLevel();

	if ( ubBackgroundLightLevel < NORMAL_LIGHTLEVEL_DAY + 2 )
	{
		// don't consider it nighttime, too close to daylight levels
		return( FALSE );
	}
	
	// could've been placed here, ignore the light
	if ( InARoom( sGridNo, NULL ) )	
	{
		return( FALSE );
	}

	// NB light levels are backwards, so a lower light level means the
	// spot in question is BRIGHTER

	if ( LightTrueLevel( sGridNo, bLevel ) < ubBackgroundLightLevel )
	{
		return( TRUE );
	}

	return( FALSE );
}

// sevenfm: new AI morale calculation
INT8 CalcMorale(SOLDIERTYPE *pSoldier)
{
	UINT32 uiLoop, uiLoop2;
	INT32 iOurTotalThreat = 0, iTheirTotalThreat = 0;
	INT16 sOppThreatValue, sFrndThreatValue, sMorale;
	INT32 iPercent;
	INT8	bMostRecentOpplistValue;
	INT8 bMoraleCategory;
	UINT8 *pSeenOpp; //,*friendOlPtr;
	INT8	*pbPersOL, *pbPublOL;
	SOLDIERTYPE *pOpponent,*pFriend;

	// if army guy has NO weapons left then panic!
	if ( pSoldier->bTeam == ENEMY_TEAM || !pSoldier->aiData.bNeutral )
	{
		if ( FindAIUsableObjClass( pSoldier, IC_WEAPON ) == NO_SLOT )
		{
			// sevenfm: instead of leaving sector, try to attack with hands/knife
			return( MORALE_FEARLESS );
			//return( MORALE_HOPELESS );
		}
	}
	// sevenfm: neutrals always have low AI morale even if they have weapons (so they run from enemy)
	if( pSoldier->aiData.bNeutral )
	{
		return( MORALE_WORRIED );
	}

	// hang pointers to my personal opplist, my team's public opplist, and my
	// list of previously seen opponents
	pSeenOpp	= (UINT8 *) &(gbSeenOpponents[pSoldier->ubID][0]);

	// loop through every one of my possible opponents
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || (pOpponent->stats.bLife < OKLIFE))
			continue;			// next merc

		// if this merc is neutral/on same side, he's not an opponent, skip him!
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
			continue;			// next merc

		// Special stuff for Carmen the bounty hunter
		if (pSoldier->aiData.bAttitude == ATTACKSLAYONLY && pOpponent->ubProfile != SLAY)
		{
			continue;	// next opponent
		}

		pbPersOL = pSoldier->aiData.bOppList + pOpponent->ubID;
		pbPublOL = gbPublicOpplist[pSoldier->bTeam] + pOpponent->ubID;
		pSeenOpp = (UINT8 *)gbSeenOpponents[pSoldier->ubID] + pOpponent->ubID;

		// if this opponent is unknown to me personally AND unknown to my team, too
		if ((*pbPersOL == NOT_HEARD_OR_SEEN) && (*pbPublOL == NOT_HEARD_OR_SEEN))
		{
			// if I have never seen him before anywhere in this sector, either
			if (!(*pSeenOpp))
				continue;		// next merc

			// have seen him in the past, so he remains something of a threat
			bMostRecentOpplistValue = 0;		// uses the free slot for 0 opplist
		}
		else		 // decide which opplist is more current
		{
			// if personal knowledge is more up to date or at least equal
			if ((gubKnowledgeValue[*pbPublOL - OLDEST_HEARD_VALUE][*pbPersOL - OLDEST_HEARD_VALUE] > 0) || (*pbPersOL == *pbPublOL))
				bMostRecentOpplistValue = *pbPersOL;		// use personal
			else
				bMostRecentOpplistValue = *pbPublOL;		// use public
		}

		iPercent = ThreatPercent[bMostRecentOpplistValue - OLDEST_HEARD_VALUE];

		sOppThreatValue = (iPercent * CalcManThreatValue(pOpponent,pSoldier->sGridNo,FALSE,pSoldier)) / 100;

		//sprintf(tempstr,"Known opponent %s, opplist status %d, percent %d, threat = %d",
		//			ExtMen[pOpponent->ubID].name,ubMostRecentOpplistValue,ubPercent,sOppThreatValue);
		//PopMessage(tempstr);

		// ADD this to their running total threatValue (decreases my MORALE)
		iTheirTotalThreat += sOppThreatValue;
		//NumMessage("Their TOTAL threat now = ",sTheirTotalThreat);

		// NOW THE FUN PART: SINCE THIS OPPONENT IS KNOWN TO ME IN SOME WAY,
		// ANY FRIENDS OF MINE THAT KNOW ABOUT HIM BOOST MY MORALE.	SO, LET'S GO
		// THROUGH THEIR PERSONAL OPPLISTS AND CHECK WHICH OF MY FRIENDS KNOW
		// SOMETHING ABOUT HIM AND WHAT THEIR THREAT VALUE TO HIM IS.

		for (uiLoop2 = 0; uiLoop2 < guiNumMercSlots; uiLoop2++)
		{
			pFriend = MercSlots[ uiLoop2 ];

			// if this merc is inactive, at base, on assignment, dead, unconscious
			if (!pFriend || (pFriend->stats.bLife < OKLIFE))
				continue;		// next merc

			// if this merc is not on my side, then he's NOT one of my friends

			// WE CAN'T AFFORD TO CONSIDER THE ENEMY OF MY ENEMY MY FRIEND, HERE!
			// ONLY IF WE ARE ACTUALLY OFFICIALLY CO-OPERATING TOGETHER (SAME SIDE)
			if ( pFriend->aiData.bNeutral && !( pSoldier->ubCivilianGroup != NON_CIV_GROUP && pSoldier->ubCivilianGroup == pFriend->ubCivilianGroup ) )
			{
				continue;		// next merc
			}

			if ( pSoldier->bSide != pFriend->bSide )
				continue;		// next merc

			// THIS TEST IS INVALID IF A COMPUTER-TEAM IS PLAYING CO-OPERATIVELY
			// WITH A NON-COMPUTER TEAM SINCE THE OPPLISTS INVOLVED ARE NOT
			// UP-TO-DATE.	THIS SITUATION IS CURRENTLY NOT POSSIBLE IN HTH/DG.

			// ALSO NOTE THAT WE COUNT US AS OUR (BEST) FRIEND FOR THESE CALCULATIONS

			// subtract HEARD_2_TURNS_AGO (which is negative) to make values start at 0 and
			// be positive otherwise
			iPercent = ThreatPercent[pFriend->aiData.bOppList[pOpponent->ubID] - OLDEST_HEARD_VALUE];

			// reduce the percentage value based on how far away they are from the enemy, if they only hear him
			if ( pFriend->aiData.bOppList[ pOpponent->ubID ] <= HEARD_LAST_TURN )
			{
				iPercent -= PythSpacesAway( pSoldier->sGridNo, pFriend->sGridNo ) * 2;
				if ( iPercent <= 0 )
				{
					//ignore!
					continue;
				}
			}

			sFrndThreatValue = (iPercent * CalcManThreatValue(pFriend,pOpponent->sGridNo,FALSE,pSoldier)) / 100;

			//sprintf(tempstr,"Known by friend %s, opplist status %d, percent %d, threat = %d",
			//		 ExtMen[pFriend->ubID].name,pFriend->aiData.bOppList[pOpponent->ubID],ubPercent,sFrndThreatValue);
			//PopMessage(tempstr);

			// ADD this to our running total threatValue (increases my MORALE)
			// We multiply by sOppThreatValue to PRO-RATE this based on opponent's
			// threat value to ME personally.	Divide later by sum of them all.
			iOurTotalThreat += sOppThreatValue * sFrndThreatValue;
		}

		// this could get slow if I have a lot of friends...
		//KeepInterfaceGoing();
	}


	// if they are no threat whatsoever
	if (!iTheirTotalThreat)
		sMorale = 500;		// our morale is just incredible
	else
	{
		// now divide sOutTotalThreat by sTheirTotalThreat to get the REAL value
		iOurTotalThreat /= iTheirTotalThreat;

		// calculate the morale (100 is even, < 100 is us losing, > 100 is good)
		sMorale = (INT16) ((100 * iOurTotalThreat) / iTheirTotalThreat);
	}

	if (sMorale <= 25)				// odds 1:4 or worse
		bMoraleCategory = MORALE_HOPELESS;
	else if (sMorale <= 50)		 // odds between 1:4 and 1:2
		bMoraleCategory = MORALE_WORRIED;
	else if (sMorale <= 150)		// odds between 1:2 and 3:2
		bMoraleCategory = MORALE_NORMAL;
	else if (sMorale <= 300)		// odds between 3:2 and 3:1
		bMoraleCategory = MORALE_CONFIDENT;
	else							// odds better than 3:1
		bMoraleCategory = MORALE_FEARLESS;

	// make idiot administrators more aggressive
	// sevenfm: also make civilians more aggressive
	if (pSoldier->ubSoldierClass == SOLDIER_CLASS_ADMINISTRATOR || pSoldier->bTeam == CIV_TEAM && !pSoldier->aiData.bNeutral)
	{
		bMoraleCategory += 2;
	}

	// SEEKENEMY soldiers are more aggressive
	if (pSoldier->aiData.bOrders == SEEKENEMY)
	{
		bMoraleCategory++;
	}

	// if have good health
	if( pSoldier->stats.bLife > pSoldier->stats.bLifeMax/2 )
	{
		bMoraleCategory++;
	}
	// bad health
	if( pSoldier->stats.bLife < pSoldier->stats.bLifeMax/4 )
	{
		bMoraleCategory--;
	}
	// good breath
	if( pSoldier->bBreath > 50 )
	{
		bMoraleCategory++;
	}
	// bad breath
	if( pSoldier->bBreath < 25 )
	{
		bMoraleCategory--;
	}
	// if not under fire - attack
	if( !pSoldier->aiData.bUnderFire )
	{
		bMoraleCategory++;
	}

	// count friends that flank around the same spot
	if (CountFriendsFlankSameSpot(pSoldier) == 0)
	{
		bMoraleCategory++;
	}

	INT32 sClosestOpponent = ClosestKnownOpponent(pSoldier, NULL, NULL);

	// if last attack of this soldier hit enemy - increase morale
	if( pSoldier->aiData.bLastAttackHit )
	{
		bMoraleCategory++;
	}

	// if some friend hit enemy - increase morale
	if( CountNearbyFriendsLastAttackHit(pSoldier, pSoldier->sGridNo, DAY_VISION_RANGE/4 ) > 0 )
	{
		bMoraleCategory++;
	}

	// bonus if last target is unconscious or suppressed
	if( LastTargetCollapsed(pSoldier) )
	{
		bMoraleCategory++;
	}
	if( LastTargetSuppressed(pSoldier) )
	{
		bMoraleCategory++;
	}

	// bonus if weapon range is short
	if (AICheckHasGun(pSoldier) &&
		(GuySawEnemy(pSoldier, SEEN_LAST_TURN) || pSoldier->aiData.bUnderFire) &&
		AICheckShortWeaponRange(pSoldier))
	{
		bMoraleCategory++;
	}

	// limit AI morale depending on morale and suppression shock
	/*if( pSoldier->aiData.bShock )
	{
		bMoraleCategory = __min(bMoraleCategory, (20 + pSoldier->aiData.bMorale - 20*__min(3, pSoldier->aiData.bShock/5)) / 20);
	}*/

	// limit AI morale when soldier is under heavy fire
	if (ShockLevelPercent(pSoldier) > 75)
		bMoraleCategory = min(bMoraleCategory, MORALE_CONFIDENT);
	else if (ShockLevelPercent(pSoldier) > 50)
		bMoraleCategory = min(bMoraleCategory, MORALE_NORMAL);

	// prevent hopeless morale when not under attack
	if (bMoraleCategory == MORALE_HOPELESS && !pSoldier->aiData.bUnderFire)
	{
		bMoraleCategory = MORALE_WORRIED;
	}

	// check limits
	bMoraleCategory = max(bMoraleCategory, MORALE_HOPELESS);
	bMoraleCategory = min(bMoraleCategory, MORALE_FEARLESS);

	return(bMoraleCategory);
}

INT32 CalcManThreatValue( SOLDIERTYPE *pEnemy, INT32 sMyGrid, UINT8 ubReduceForCover, SOLDIERTYPE * pMe )
{
	INT32	iThreatValue = 0;
	BOOLEAN fForCreature = CREATURE_OR_BLOODCAT( pMe );

	// If man is inactive, at base, on assignment, dead, unconscious
	if (!pEnemy->bActive || !pEnemy->bInSector || !pEnemy->stats.bLife)
	{
		// he's no threat at all, return a negative number
		iThreatValue = -999;
		return(iThreatValue);
	}

	// in boxing mode, let only a boxer be considered a threat.
	if ( (gTacticalStatus.bBoxingState == BOXING) && !(pEnemy->flags.uiStatusFlags & SOLDIER_BOXER) )
	{
		iThreatValue = -999;
		return( iThreatValue );
	}

	if (fForCreature)
	{
		// health (1-100)
		iThreatValue += pEnemy->stats.bLife;
		// bleeding (more attactive!) (1-100)
		iThreatValue += pEnemy->bBleeding;
		// decrease according to distance
		iThreatValue = (iThreatValue * 10) / (10 + PythSpacesAway( sMyGrid, pEnemy->sGridNo ) );

	}
	else
	{
		// ADD twice the man's level (2-20)
		iThreatValue += EffectiveExpLevel(pEnemy); // SANDRO - find precise effective exp level

		// ADD man's total action points (10-35)
		// sevenfm: r7810 fix
		//iThreatValue += pEnemy->CalcActionPoints();
		iThreatValue += 25 * pEnemy->CalcActionPoints() / APBPConstants[AP_MAXIMUM];

		// ADD 1/2 of man's current action points (4-17)
		// sevenfm: r7810 fix
		//iThreatValue += (pEnemy->bActionPoints / 2);
		iThreatValue += 25 * pEnemy->bActionPoints / APBPConstants[AP_MAXIMUM] / 2;

		// ADD 1/10 of man's current health (0-10)
		iThreatValue += (pEnemy->stats.bLife / 10);

		if (pEnemy->bAssignment < ON_DUTY )
		{
			// ADD 1/4 of man's protection percentage (0-25)
			iThreatValue += ArmourPercent( pEnemy ) / 4;

			// ADD 1/5 of man's marksmanship skill (0-20)
			iThreatValue += (pEnemy->stats.bMarksmanship / 5);

			if ( Item[ pEnemy->inv[HANDPOS].usItem ].usItemClass & IC_WEAPON )
			{
				// ADD the deadliness of the item(weapon) he's holding (0-50)
				iThreatValue += Weapon[pEnemy->inv[HANDPOS].usItem].ubDeadliness;
			}
		}

		// SUBTRACT 1/5 of man's bleeding (0-20)
		iThreatValue -= (pEnemy->bBleeding / 5);

		// SUBTRACT 1/10 of man's breath deficiency (0-10)
		iThreatValue -= ((100 - pEnemy->bBreath) / 10);

		// SUBTRACT man's current shock value
		iThreatValue -= pEnemy->aiData.bShock;
	}

	// if I have a specifically defined spot where I'm at (sometime I don't!)	
	if (!TileIsOutOfBounds(sMyGrid))
	{
		// ADD 10% if man's already been shooting at me
		if (pEnemy->sLastTarget == sMyGrid)
		{
			iThreatValue += (iThreatValue / 10);
		}
		else
		{
			// ADD 5% if man's already facing me
			if (pEnemy->ubDirection == atan8(CenterX(pEnemy->sGridNo),CenterY(pEnemy->sGridNo),CenterX(sMyGrid),CenterY(sMyGrid)))
			{
				iThreatValue += (iThreatValue / 20);
			}
		}
	}

	// if this man is conscious
	if (pEnemy->stats.bLife >= OKLIFE)
	{
		// and we were told to reduce threat for my cover		
		if (ubReduceForCover && (!TileIsOutOfBounds(sMyGrid)))
		{
			// Reduce iThreatValue to same % as the chance HE has shoot through at ME
			//iThreatValue = (iThreatValue * ChanceToGetThrough( pEnemy, myGrid, FAKE, ACTUAL, TESTWALLS, 9999, M9PISTOL, NOT_FOR_LOS)) / 100;
			//iThreatValue = (iThreatValue * SoldierTo3DLocationChanceToGetThrough( pEnemy, myGrid, FAKE, ACTUAL, TESTWALLS, 9999, M9PISTOL, NOT_FOR_LOS)) / 100;
			iThreatValue = (iThreatValue * SoldierToLocationChanceToGetThrough( pEnemy, sMyGrid, pMe->pathing.bLevel, 0, pMe->ubID ) ) / 100;
		}
	}
	else
	{
		// if he's still something of a threat
		if (iThreatValue > 0)
		{
			// drastically reduce his threat value (divide by 5 to 18)
			iThreatValue /= (4 + (OKLIFE - pEnemy->stats.bLife));
		}
	}

	// threat value of any opponent can never drop below 1
	if (iThreatValue < 1)
	{
		iThreatValue = 1;
	}

	//sprintf(tempstr,"%s's iThreatValue = ",pEnemy->name);
	//NumMessage(tempstr,iThreatValue);

#ifdef BETAVERSION	// unnecessary for real release
	// NOTE: maximum is about 200 for a healthy Mike type with a mortar!
	if (iThreatValue > 250)
	{
		sprintf(tempstr,"CalcManThreatValue: WARNING - %d has a very high threat value of %d",pEnemy->ubID,iThreatValue);

#ifdef RECORDNET
		fprintf(NetDebugFile,"\t%s\n",tempstr);
#endif

#ifdef TESTVERSION
		PopMessage(tempstr);
#endif

	}
#endif

	return(iThreatValue);
}

// sevenfm: ONGUARD, POINTPATROL, RNDPTPATROL - max roaming if seen enemy recently or under fire
// STATIONARY/SNIPER - 5 tiles roaming
// other orders - max roaming if enemy position is known
INT16 RoamingRange(SOLDIERTYPE *pSoldier, INT32 * pusFromGridNo)
{
	BOOLEAN fOppPosKnown = FALSE;
	BOOLEAN fInCombat = FALSE;
	BOOLEAN fRedAlert = FALSE;
	//BOOLEAN fFriendsNeedHelp = FALSE;

	// sevenfm: in case we want to call this for player mercs
	if ( pSoldier->flags.uiStatusFlags & SOLDIER_PC )
	{
		*pusFromGridNo = pSoldier->sGridNo;
		return MAX_ROAMING_RANGE;
	}

	// sevenfm: for zombies, allow max range
	if (pSoldier->IsZombie())
	{
		*pusFromGridNo = pSoldier->sGridNo;
		return MAX_ROAMING_RANGE;
	}

	if ( CREATURE_OR_BLOODCAT( pSoldier ) )
	{
		if ( pSoldier->aiData.bAlertStatus == STATUS_BLACK )
		{
			*pusFromGridNo = pSoldier->sGridNo; // from current position!
			return(MAX_ROAMING_RANGE);
		}
	}
	if ( pSoldier->aiData.bOrders == POINTPATROL || pSoldier->aiData.bOrders == RNDPTPATROL )
	{
		// roam near NEXT PATROL POINT, not from where merc starts out
		*pusFromGridNo = pSoldier->aiData.sPatrolGrid[pSoldier->aiData.bNextPatrolPnt];
	}
	else
	{
		// roam around where mercs started
		//*pusFromGridNo = pSoldier->sInitialGridNo;
		*pusFromGridNo = pSoldier->aiData.sPatrolGrid[0];
	}

	if( GuyKnowsEnemyPosition(pSoldier) )
	{
		fOppPosKnown = TRUE;
	}
	if( pSoldier->aiData.bAlertStatus >= STATUS_RED )
	{
		fRedAlert = TRUE;
	}
	if( pSoldier->aiData.bUnderFire || GuySawEnemy(pSoldier))
	{
		fInCombat = TRUE;
	}

	switch (pSoldier->aiData.bOrders)
	{
		// JA2 GOLD: give non-NPCs a 5 tile roam range for cover in combat when being shot at
		case STATIONARY:	if( pSoldier->ubProfile != NO_PROFILE || !fInCombat )
							{
								return( 0 );
							}
							else
							{
								return( 5 );
							}		
		case ONGUARD:		if( !fInCombat )
							{
								return( 5 );
							}
							else
							{
								return( MAX_ROAMING_RANGE );
							}
		case CLOSEPATROL:	if( !fOppPosKnown )
							{
								return( 15 );
							}
							else
							{
								return( MAX_ROAMING_RANGE );
							}
		case POINTPATROL:	if( !fOppPosKnown )
							{
								// from nextPatrolGrid, not whereIWas
								return( 10 );
							}
							else
							{
								return( MAX_ROAMING_RANGE );
							}
		case RNDPTPATROL:	if( !fOppPosKnown )
							{
								// from nextPatrolGrid, not whereIWas
								return( 10 );
							}
							else
							{
								return( MAX_ROAMING_RANGE );
							}
		case FARPATROL:		if( !fOppPosKnown )
							{
								return( 25 );
							}
							else
							{
								return( MAX_ROAMING_RANGE );
							}
		case ONCALL:		if( !fOppPosKnown )
							{
								return( 10 );
							}
							else
							{
								return( MAX_ROAMING_RANGE );
							}
		case SEEKENEMY:		*pusFromGridNo = pSoldier->sGridNo; // from current position!
							return(MAX_ROAMING_RANGE);
		case SNIPER:		return ( 5 );
		default:
#ifdef BETAVERSION
			sprintf(tempstr,"%s has invalid orders = %d",pSoldier->GetName(),pSoldier->aiData.bOrders);
			PopMessage(tempstr);
#endif
			return(0);
	}
}

/*INT16 RoamingRange(SOLDIERTYPE *pSoldier, INT32 * pusFromGridNo)
{
	BOOL OppPosKnown = FALSE;
	if ( CREATURE_OR_BLOODCAT( pSoldier ) )
	{
		if ( pSoldier->aiData.bAlertStatus == STATUS_BLACK )
		{
			*pusFromGridNo = pSoldier->sGridNo; // from current position!
			return(MAX_ROAMING_RANGE);
		}
	}
	if ( pSoldier->aiData.bOrders == POINTPATROL || pSoldier->aiData.bOrders == RNDPTPATROL )
	{
		// roam near NEXT PATROL POINT, not from where merc starts out
		*pusFromGridNo = pSoldier->aiData.sPatrolGrid[pSoldier->aiData.bNextPatrolPnt];
	}
	else
	{
		// roam around where mercs started
		//*pusFromGridNo = pSoldier->sInitialGridNo;
		*pusFromGridNo = pSoldier->aiData.sPatrolGrid[0];
	}

	//Do we know about any opponent?
	for(UINT16 oppID = 0; oppID < MAX_NUM_SOLDIERS; oppID++)
	{
		if ( pSoldier->aiData.bOppList[oppID]  !=  NOT_HEARD_OR_SEEN &&  gbPublicOpplist[pSoldier->bTeam][oppID] != NOT_HEARD_OR_SEEN)
		{
			OppPosKnown = TRUE;
			break;
		}
	}

	//TODO: Externalize if people want?
	BOOL fLessRestrictiveRoaming = TRUE;

	switch (pSoldier->aiData.bOrders)
	{
		// JA2 GOLD: give non-NPCs a 5 tile roam range for cover in combat when being shot at
		case STATIONARY:			if (pSoldier->ubProfile != NO_PROFILE || (pSoldier->aiData.bAlertStatus < STATUS_BLACK && !(pSoldier->aiData.bUnderFire)))
									{
										return( 0 );
									}
									else
									{
										return( 5 );
									}		
		case ONGUARD:				return( 5 );
		case CLOSEPATROL:			if (pSoldier->aiData.bAlertStatus < STATUS_RED)
													{
														return( 5 );
													}
													else
													{
														if(!OppPosKnown || !fLessRestrictiveRoaming)
														{
															return( 15 );
														}
														else
														{
															return( 30 );
															//return( MAX_ROAMING_RANGE );
														}
													}
		case POINTPATROL:			if (pSoldier->aiData.bAlertStatus < STATUS_RED)
													{
														return( 10 );
													}
													else
													{
														if(!OppPosKnown || !fLessRestrictiveRoaming)
														{
															return( 20 );
														}
														else
														{
															return( 40 );
															//return( MAX_ROAMING_RANGE );
														}
													}	 // from nextPatrolGrid, not whereIWas
		case RNDPTPATROL:			if (pSoldier->aiData.bAlertStatus < STATUS_RED)
													{
														return( 10 );
													}
													else
													{
														if(!OppPosKnown || !fLessRestrictiveRoaming)
														{
															return( 20 );
														}
														else
														{
															//return( 40 );
															return( MAX_ROAMING_RANGE );
														}
													}// from nextPatrolGrid, not whereIWas
		case FARPATROL:				if (pSoldier->aiData.bAlertStatus < STATUS_RED)
													{
														return( 15 );
													}
													else
													{
														if(!OppPosKnown || !fLessRestrictiveRoaming)
														{
															return( 30 );
														}
														else
														{
															return( MAX_ROAMING_RANGE );
														}
													}
		case ONCALL:					if (pSoldier->aiData.bAlertStatus < STATUS_RED)
													{
														return( 10 );
													}
													else
													{
														if(!OppPosKnown || !fLessRestrictiveRoaming)
														{
															return( 30 );
														}
														else
														{
															//return(50);
															return( MAX_ROAMING_RANGE );
														}
													}
		case SEEKENEMY:				*pusFromGridNo = pSoldier->sGridNo; // from current position!
													return(MAX_ROAMING_RANGE);
		case SNIPER:				return ( 5 );
		default:
#ifdef BETAVERSION
			sprintf(tempstr,"%s has invalid orders = %d",pSoldier->GetName(),pSoldier->aiData.bOrders);
			PopMessage(tempstr);
#endif
			return(0);
	}
}*/


void RearrangePocket(SOLDIERTYPE *pSoldier, INT8 bPocket1, INT8 bPocket2, UINT8 bPermanent)
{
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"RearrangePocket");
	// NB there's no such thing as a temporary swap for now...
	// 0verhaul:  There is now!  If not permanent, don't lose weapon ready status because the
	// weapon will be restored after the trial situation is finished.
	//SwapObjs( &(pSoldier->inv[bPocket1]), &(pSoldier->inv[bPocket2]) );
	SwapObjs( pSoldier, bPocket1, bPocket2, bPermanent );

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"RearrangePocket done");
}

BOOLEAN FindBetterSpotForItem( SOLDIERTYPE * pSoldier, INT8 bSlot )
{
	// looks for a place in the slots to put an item in a hand or armour
	// position, and moves it there.
	if (bSlot >= BIGPOCKSTART)
	{
		return( FALSE );
	}
	if (pSoldier->inv[bSlot].exists() == false)
	{
		// well that's just fine then!
		return( TRUE );
	}

	if(FitsInSmallPocket(&pSoldier->inv[bSlot]) == false)
	{
		// then we're looking for a big pocket
		bSlot = FindEmptySlotWithin( pSoldier, BIGPOCKSTART, MEDPOCKFINAL );
	}
	else
	{
		// try a small pocket first
		bSlot = FindEmptySlotWithin( pSoldier, SMALLPOCKSTART, NUM_INV_SLOTS );
		if (bSlot == NO_SLOT)
		{
			bSlot = FindEmptySlotWithin( pSoldier, BIGPOCKSTART, MEDPOCKFINAL );
		}
	}
	if (bSlot == NO_SLOT)
	{
		return( FALSE );
	}
    DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"findbetterspotforitem: swapping items");
	RearrangePocket(pSoldier, HANDPOS, bSlot, FOREVER );		
	return( TRUE );
}

UINT8 GetTraversalQuoteActionID( INT8 bDirection )
{
	switch( bDirection )
	{
		case NORTHEAST: // east
			return( QUOTE_ACTION_ID_TRAVERSE_EAST );

		case SOUTHEAST: // south
			return( QUOTE_ACTION_ID_TRAVERSE_SOUTH );

		case SOUTHWEST: // west
			return( QUOTE_ACTION_ID_TRAVERSE_WEST );

		case NORTHWEST: // north
			return( QUOTE_ACTION_ID_TRAVERSE_NORTH );

		default:
			return( 0 );
	}
}

UINT8 SoldierDifficultyLevel( SOLDIERTYPE * pSoldier )
{
	INT8 bDifficultyBase;
	INT8 bDifficulty;

	DebugMsg(TOPIC_JA2AI,DBG_LEVEL_3,String("SoldierDifficultyLevel"));
	// difficulty modifier ranges from 0 to 100
	// and we want to end up with a number between 0 and 4 (4=hardest)
	// to a base of 1, divide by 34 to get a range from 1 to 3
	bDifficultyBase = 1 + ( CalcDifficultyModifier( pSoldier->ubSoldierClass ) / 34 );

	switch( pSoldier->ubSoldierClass )
	{
		case SOLDIER_CLASS_ADMINISTRATOR:
			bDifficulty = bDifficultyBase - 1;
			break;

		case SOLDIER_CLASS_ARMY:
			bDifficulty = bDifficultyBase;
			break;

		case SOLDIER_CLASS_ELITE:
			bDifficulty = bDifficultyBase + 1;
			break;

		// hard code militia;
		case SOLDIER_CLASS_GREEN_MILITIA:
			bDifficulty = 2;
			break;

		case SOLDIER_CLASS_REG_MILITIA:
			bDifficulty = 3;
			break;

		case SOLDIER_CLASS_ELITE_MILITIA:
			bDifficulty = 4;
			break;

#ifdef ENABLE_ZOMBIES
		case SOLDIER_CLASS_ZOMBIE:
			bDifficulty = bDifficultyBase;
			break;
#endif

		default:
			if (pSoldier->bTeam == CREATURE_TEAM)
			{
				bDifficulty = bDifficultyBase + pSoldier->pathing.bLevel / 4;
			}
			else // civ...
			{
				bDifficulty = (bDifficultyBase + pSoldier->pathing.bLevel / 4) - 1;
			}
			break;

	}

	bDifficulty = __max( bDifficulty, 0 );
	bDifficulty = __min( bDifficulty, 4 );

	return( (UINT8) bDifficulty );
}

BOOLEAN ValidCreatureTurn( SOLDIERTYPE * pCreature, INT8 bNewDirection )
{
	INT8	bDirChange;
	INT8	bTempDir;
	INT8	bLoop;
	BOOLEAN	fFound;

	bDirChange = (INT8) QuickestDirection( pCreature->ubDirection, bNewDirection );

	for( bLoop = 0; bLoop < 2; bLoop++ )
	{
		fFound = TRUE;

		bTempDir = pCreature->ubDirection;

		do
		{

			bTempDir += bDirChange;
			if (bTempDir < NORTH)
			{
				bTempDir = NORTHWEST;
			}
			else if (bTempDir > NORTHWEST)
			{
				bTempDir = NORTH;
			}
			if (!pCreature->InternalIsValidStance( bTempDir, ANIM_STAND ))
			{
				fFound = FALSE;
				break;
			}

		} while ( bTempDir != bNewDirection );

		if ( fFound )
		{
			break;
		}
		else if ( bLoop > 0 )
		{
			// can't find a dir!
			return( FALSE );
		}
		else
		{
			// try the other direction
			bDirChange = bDirChange * -1;
		}
	}

	return( TRUE );
}

INT32 RangeChangeDesire( SOLDIERTYPE * pSoldier )
{
	INT32 iRangeFactorMultiplier;

	iRangeFactorMultiplier = pSoldier->aiData.bAIMorale - 1;

	INT8 bBonus = 0;
	// sevenfm: if we have no weapons, try to get closer to enemy
	if ( !AICheckHasGun(pSoldier) )
	{
		bBonus = 2;
	}
	// bonus if weapon range is short
	else if( GuySawEnemy(pSoldier, SEEN_LAST_TURN) && AICheckShortWeaponRange(pSoldier) )
	{
		bBonus = 1;
	}

	switch (pSoldier->aiData.bAttitude)
	{
	//case DEFENSIVE:		iRangeFactorMultiplier +=	__max(-1, bBonus); break;
	case DEFENSIVE:		iRangeFactorMultiplier +=	__max(0, bBonus); break;
	case BRAVESOLO:		iRangeFactorMultiplier +=	__max(2, bBonus); break;
	case BRAVEAID:		iRangeFactorMultiplier +=	__max(2, bBonus); break;
	case CUNNINGSOLO:	iRangeFactorMultiplier +=	__max(0, bBonus); break;
	case CUNNINGAID:	iRangeFactorMultiplier +=	__max(0, bBonus); break;
	case ATTACKSLAYONLY:
	case AGGRESSIVE:	iRangeFactorMultiplier +=	__max(1, bBonus); break;
	}

	if ( (pSoldier->aiData.bOrders == SEEKENEMY || pSoldier->bTeam != ENEMY_TEAM) &&
		gTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes > 0 )
	{
		iRangeFactorMultiplier += gTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes;
	}
	
	return( iRangeFactorMultiplier );
}

BOOLEAN ArmySeesOpponents( void )
{
	INT32				cnt;
	SOLDIERTYPE *		pSoldier;

	for ( cnt = gTacticalStatus.Team[ ENEMY_TEAM ].bFirstID; cnt <= gTacticalStatus.Team[ ENEMY_TEAM ].bLastID; cnt++ )
	{
		pSoldier = MercPtrs[ cnt ];

		if ( pSoldier->bActive && pSoldier->bInSector && pSoldier->stats.bLife >= OKLIFE && pSoldier->aiData.bOppCnt > 0 )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}

#ifdef DEBUGDECISIONS
void AIPopMessage ( STR16 str )
{
	DebugAI(str);
}

void AIPopMessage ( const STR8	str )
{
	STR tempstr;
	sprintf( tempstr,"%s", str);
	DebugAI(tempstr);
}

void AINumMessage(const STR8	str, INT32 num)
{
	STR tempstr;
	sprintf( tempstr,"%s %d", str, num);
	DebugAI(tempstr);
}

void AINameMessage(SOLDIERTYPE * pSoldier,const STR8	str,INT32 num)
{
	STR tempstr;
	sprintf( tempstr,"%d %s %d",pSoldier->GetName() , str, num);
	DebugAI( tempstr );
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////
// HEADROCK:
//
// The following function(s) are part of my half-assed attempt to have the AI analyze the tactical
// situation, by comparing (known) squad sizes, the state of all combatants, and the orders of all
// friendlies. The idea is to return a value called "TacticalSituation" which can tell a combatant
// whether he should try to undertake a smarter course of action.
/////////////////////////////////////////////////////////////////////////////////////////////////
/*
INT16 AssessTacticalSituation( INT8 bTeam )
{
	UINT16 ubFriendlyTeamTacticalValue = 0;
	UINT16 ubEnemyTeamTacticalValue = 0;
	UINT8 ubSoldierTacticalThreat;
	INT16 ubTacticalSituation;
	UINT16 cnt;
	SOLDIERTYPE * pSoldier;
	
	// begin loop through all MERCs.
	for ( cnt = gTacticalStatus.Team[ OUR_TEAM ].bFirstID; cnt <= gTacticalStatus.Team[ OUR_TEAM ].bLastID; cnt++ )
	{
		pSoldier = MercPtrs[ cnt ];
		ubSoldierTacticalThreat = CalcStraightThreatValue( pSoldier );
		// Player-controlled Mercs are 1.5 times more threatening than AIs
		if (pSoldier->flags.uiStatusFlags & SOLDIER_PC)
			ubSoldierTacticalThreat = (UINT8)((float)ubSoldierTacticalThreat * 1.5);
		
		// Assess Threat
		if (bTeam == OUR_TEAM || bTeam == MILITIA_TEAM)
		{
			// Friendly!
			ubFriendlyTeamTacticalValue += ubSoldierTacticalThreat;
		}
		else
		{
			// Enemy!
			if ( TeamSeesOpponent( ENEMY_TEAM, pSoldier ) )
				ubEnemyTeamTacticalValue += ubSoldierTacticalThreat;

		}
	}

	// begin loop through all Militia.
	for ( cnt = gTacticalStatus.Team[ MILITIA_TEAM ].bFirstID; cnt <= gTacticalStatus.Team[ MILITIA_TEAM ].bLastID; cnt++ )
	{
		pSoldier = MercPtrs[ cnt ];
		ubSoldierTacticalThreat = CalcStraightThreatValue( pSoldier );
		
		// Assess Threat
		if (bTeam == OUR_TEAM || bTeam == MILITIA_TEAM)
		{
			// Friendly!
			ubFriendlyTeamTacticalValue += ubSoldierTacticalThreat;
		}
		else
		{
			// Enemy!
			if ( TeamSeesOpponent( ENEMY_TEAM, pSoldier ) )
				ubEnemyTeamTacticalValue += ubSoldierTacticalThreat;
		}
	}	

	// begin loop through all Enemies.
	for ( cnt = gTacticalStatus.Team[ ENEMY_TEAM ].bFirstID; cnt <= gTacticalStatus.Team[ ENEMY_TEAM ].bLastID; cnt++ )
	{
		pSoldier = MercPtrs[ cnt ];
		ubSoldierTacticalThreat = CalcStraightThreatValue( pSoldier );
		
		// Assess Threat

		if (bTeam == ENEMY_TEAM)
		{
			// Friendly!
			ubFriendlyTeamTacticalValue += ubSoldierTacticalThreat;
		}
		else
		{
			// Enemy!
			if ( TeamSeesOpponent( OUR_TEAM, pSoldier ) || TeamSeesOpponent ( MILITIA_TEAM, pSoldier) )
				ubEnemyTeamTacticalValue += ubSoldierTacticalThreat;
		}
		
	}

	ubTacticalSituation = ubEnemyTeamTacticalValue - ubFriendlyTeamTacticalValue;

	return (ubTacticalSituation);


}
*/

// HEADROCK: Function to check whether a team can see the specified soldier.
BOOLEAN TeamSeesOpponent( INT8 bTeam, SOLDIERTYPE * pOpponent )
{
	SOLDIERTYPE * pSoldier;
	UINT16 cnt;

	// This assertion can be safely removed, assuming the program does what it should. It simply checks
	// whether the "opponent" is on the same team being checked. That should be avoided when calling this
	// function.
	//Assert( pOpponent->bTeam != bTeam );

	// We're checking Merc/Militia visibility
	if (bTeam == OUR_TEAM || bTeam == MILITIA_TEAM )
	{
		for ( cnt = gTacticalStatus.Team[ MILITIA_TEAM ].bFirstID; cnt <= gTacticalStatus.Team[ MILITIA_TEAM ].bLastID; cnt++ )
		{
			pSoldier = MercPtrs[ cnt ];

			if (pSoldier->bActive && pSoldier->bInSector && pSoldier->stats.bLife >= OKLIFE)
			{


				if (pSoldier->aiData.bOppList[ pOpponent->ubID ] == SEEN_CURRENTLY)
					return ( TRUE );
			}
		}
		for ( cnt = gTacticalStatus.Team[ OUR_TEAM ].bFirstID; cnt <= gTacticalStatus.Team[ OUR_TEAM ].bLastID; cnt++ )
		{
			pSoldier = MercPtrs[ cnt ];

			if (pSoldier->bActive && pSoldier->bInSector && pSoldier->stats.bLife >= OKLIFE)
			{
				// This assertion can be safely removed, assuming the program does what it should. It simply checks
				// whether the "opponent" is on the same team being checked. That should be avoided when calling this
				// function.
				//Assert( pOpponent->bSide != bSide );

				if (pSoldier->aiData.bOppList[ pOpponent->ubID ] == SEEN_CURRENTLY)
					return ( TRUE );
			}
		}		
		
		return ( FALSE );
	}
	// Check enemy visibility
	else if (bTeam == ENEMY_TEAM)
	{
		for ( cnt = gTacticalStatus.Team[ ENEMY_TEAM ].bFirstID; cnt <= gTacticalStatus.Team[ ENEMY_TEAM ].bLastID; cnt++ )
		{
			pSoldier = MercPtrs[ cnt ];

			if (pSoldier->bActive && pSoldier->bInSector && pSoldier->stats.bLife >= OKLIFE)
			{
				// This assertion can be safely removed, assuming the program does what it should. It simply checks
				// whether the "opponent" is on the same team being checked. That should be avoided when calling this
				// function.
				//Assert( pOpponent->bSide != bSide );

				if (pSoldier->aiData.bOppList[ pOpponent->ubID ] == SEEN_CURRENTLY)
					return ( TRUE );
			}
		}	
		return ( FALSE );
	}

	else
		return (FALSE);

}

// HEADROCK: Function to assess an enemy's threat value without "me" argument.
INT32 CalcStraightThreatValue( SOLDIERTYPE *pEnemy )
{
	INT32	iThreatValue = 0;

	// If man is inactive, at base, on assignment, dead, unconscious
	if (!pEnemy->bActive || !pEnemy->bInSector || !pEnemy->stats.bLife )
	{
		// he's no threat at all, return a negative number
		iThreatValue = 0;
		return(iThreatValue);
	}

	else
	{
		// ADD twice the man's level (2-20)
		iThreatValue += EffectiveExpLevel(pEnemy); // SANDRO - find precise effective exp level

		// ADD man's total action points (10-35)
		// sevenfm: r7810 fix
		//iThreatValue += pEnemy->CalcActionPoints();
		iThreatValue += 25 * pEnemy->CalcActionPoints() / APBPConstants[AP_MAXIMUM];

		// ADD 1/2 of man's current action points (4-17)
		// sevenfm: r7810 fix
		//iThreatValue += (pEnemy->bActionPoints / 2);
		iThreatValue += 25 * pEnemy->bActionPoints / APBPConstants[AP_MAXIMUM] / 2;

		// ADD 1/10 of man's current health (0-10)
		iThreatValue += (pEnemy->stats.bLife / 10);

		if (pEnemy->bAssignment < ON_DUTY )
		{
			// ADD 1/4 of man's protection percentage (0-25)
			iThreatValue += ArmourPercent( pEnemy ) / 4;

			// ADD 1/5 of man's marksmanship skill (0-20)
			iThreatValue += (pEnemy->stats.bMarksmanship / 5);

			if ( Item[ pEnemy->inv[HANDPOS].usItem ].usItemClass & IC_WEAPON )
			{
				// ADD the deadliness of the item(weapon) he's holding (0-50)
				iThreatValue += Weapon[pEnemy->inv[HANDPOS].usItem].ubDeadliness;
			}
		}

		// SUBTRACT 1/5 of man's bleeding (0-20)
		iThreatValue -= (pEnemy->bBleeding / 5);

		// SUBTRACT 1/10 of man's breath deficiency (0-10)
		iThreatValue -= ((100 - pEnemy->bBreath) / 10);

		// SUBTRACT man's current shock value
		iThreatValue -= pEnemy->aiData.bShock;
	}

	// if this man is conscious
	if (pEnemy->stats.bLife < OKLIFE)
	{
		// if he's still something of a threat
		if (iThreatValue > 0)
		{
			// drastically reduce his threat value (divide by 5 to 18)
			iThreatValue /= (4 + (OKLIFE - pEnemy->stats.bLife));
		}
	}

	// threat value of any opponent can never drop below 1
	if (iThreatValue < 0)
	{
		iThreatValue = 0;
	}

	return(iThreatValue);
}

// Flugente: get the id of the closest soldier with a specific flag that we can currently see
UINT8 GetClosestFlaggedSoldierID( SOLDIERTYPE * pSoldier, INT16 aRange, UINT8 auTeam, UINT32 aFlag, BOOLEAN fCheckSight )
{
	UINT8 id = NOBODY;

	UINT32				uiLoop;
	BOOLEAN				fRangeRestricted = FALSE, fFound = FALSE;
	SOLDIERTYPE *		pFriend;
	INT16				range = aRange;

	// go through each soldier, looking for "friends" (soldiers on same side)
	for (uiLoop = 0; uiLoop < guiNumMercSlots; ++uiLoop)
	{
		pFriend = MercSlots[ uiLoop ];

		// if this merc is inactive, not in sector, or dead
		if (!pFriend)
			continue;
		
		if ( auTeam != pFriend->bTeam )
			continue;

		// check for flag
		if ( !(pFriend->usSoldierFlagMask & aFlag) )
			continue;
				
		// skip ourselves
		if (pFriend->ubID == pSoldier->ubID)
			continue;

		// this is not for tanks
		if (TANK(pFriend))
			continue;

		// skip if this guy is dead
		if (pFriend->stats.bLife < OKLIFE)
			continue;
				
		// if we're not already neighbors
		if (SpacesAway(pSoldier->sGridNo, pFriend->sGridNo) < range)
		{
			// can we see this guy?
			if ( !fCheckSight || SoldierTo3DLocationLineOfSightTest( pSoldier, pFriend->sGridNo, pSoldier->pathing.bLevel, 3, TRUE, CALC_FROM_WANTED_DIR ) )
			{
				range = SpacesAway(pSoldier->sGridNo,pFriend->sGridNo);
				id = pFriend->ubID;
			}
		}
	}
		
	return id;
}

// sevenfm: additional functions used for AI

INT16 MaxNormalVisionDistance( void )
{
	if( NightTime() )
	{
		return gGameExternalOptions.ubStraightSightRange * STRAIGHT_RATIO;
	}
	return gGameExternalOptions.ubStraightSightRange * 2 * STRAIGHT_RATIO;
}

// sevenfm: check friendly soldiers between me and noise gridno
// count only friends that are active and not stationary/onguard/sniper
UINT8 CountFriendsInDirection(SOLDIERTYPE *pSoldier, UINT8 ubDirection, INT16 sDistance, BOOLEAN fCheckSight)
{
	SOLDIERTYPE * pFriend;
	UINT8 ubFriends = 0;

	CHECKF(pSoldier);

	if (ubDirection == DIRECTION_IRRELEVANT)
	{
		return 0;
	}

	// Run through each friendly.
	for (UINT8 iCounter = gTacticalStatus.Team[pSoldier->bTeam].bFirstID; iCounter <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; iCounter++)
	{
		pFriend = MercPtrs[iCounter];

		if (pFriend &&
			pFriend != pSoldier &&
			pFriend->bActive &&
			pFriend->stats.bLife >= OKLIFE &&
			AIDirection(pSoldier->sGridNo, pFriend->sGridNo) == ubDirection &&
			PythSpacesAway(pSoldier->sGridNo, pFriend->sGridNo) <= sDistance &&
			(!fCheckSight || LocationToLocationLineOfSightTest(pSoldier->sGridNo, pSoldier->pathing.bLevel, pFriend->sGridNo, pFriend->pathing.bLevel, TRUE, MAX_VISION_RANGE)))
		{
			ubFriends++;
		}
	}

	return ubFriends;
}

UINT8 CountFriendsInDirectionFromSpot(SOLDIERTYPE *pSoldier, INT32 sSpot, UINT8 ubDirection, INT16 sDistance)
{
	SOLDIERTYPE * pFriend;
	UINT8 ubFriends = 0;

	CHECKF(pSoldier);

	if (ubDirection == DIRECTION_IRRELEVANT)
	{
		return 0;
	}

	// Run through each friendly.
	for (UINT8 iCounter = gTacticalStatus.Team[pSoldier->bTeam].bFirstID; iCounter <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; iCounter++)
	{
		pFriend = MercPtrs[iCounter];

		if (pFriend &&
			pFriend != pSoldier &&
			pFriend->bActive &&
			pFriend->stats.bLife >= OKLIFE &&
			AIDirection(sSpot, pFriend->sGridNo) == ubDirection &&
			PythSpacesAway(sSpot, pFriend->sGridNo) <= sDistance)
		{
			ubFriends++;
		}
	}

	return ubFriends;
}

// sevenfm: check friendly soldiers between me and noise gridno
// count only friends that are active and not stationary/onguard/sniper
UINT8 CountFriendsBetweenMeAndSpotFromSpot(SOLDIERTYPE *pSoldier, INT32 sTargetGridNo)
{
	SOLDIERTYPE * pFriend;
	UINT8 ubFriendDir, ubMyDir;
	UINT8 ubFriends = 0;

	CHECKF(pSoldier);

	if (TileIsOutOfBounds(sTargetGridNo))
	{
		return 0;
	}

	ubMyDir = AIDirection(sTargetGridNo, pSoldier->sGridNo);

	// Run through each friendly.
	for (UINT8 iCounter = gTacticalStatus.Team[pSoldier->bTeam].bFirstID; iCounter <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; iCounter++)
	{
		pFriend = MercPtrs[iCounter];

		if (!pFriend)
		{
			continue;
		}

		ubFriendDir = AIDirection(sTargetGridNo, pFriend->sGridNo);

		if (pFriend != pSoldier &&
			pFriend->bActive &&
			pFriend->stats.bLife >= OKLIFE &&
			pFriend->stats.bLife >= pFriend->stats.bLifeMax / 2 &&
			pFriend->aiData.bOrders > ONGUARD &&
			(ubFriendDir == ubMyDir || ubFriendDir == gOneCDirection[ubMyDir] || ubFriendDir == gOneCCDirection[ubMyDir]) &&
			PythSpacesAway(sTargetGridNo, pFriend->sGridNo) < PythSpacesAway(sTargetGridNo, pSoldier->sGridNo))
		{
			ubFriends++;
		}
	}

	return ubFriends;
}

// check that soldier is flanking
BOOLEAN AICheckIsFlanking( SOLDIERTYPE *pSoldier )
{
	CHECKF(pSoldier);

	if( pSoldier->aiData.bAlertStatus < STATUS_YELLOW ||
		pSoldier->numFlanks == 0 ||
		pSoldier->numFlanks >= MAX_FLANKS_RED )
	{
		return FALSE;
	}

	return TRUE;
}

// count mobile friends that are in BLACK state and not in a dangerous place or have 3/4 APs or hit enemy recently
// this is mostly used to check if we can cross dangerous area (in light at night or fresh corpses)
UINT8 CountFriendsBlack( SOLDIERTYPE *pSoldier, INT32 sClosestOpponent )
{
	CHECKF(pSoldier);

	SOLDIERTYPE * pFriend;
	UINT8 ubFriendCount = 0;
	INT32 sFriendClosestOpponent;

	// by default, use closest known opponent
	if( sClosestOpponent == NOWHERE )
	{
		sClosestOpponent = ClosestKnownOpponent( pSoldier, NULL, NULL );
	}

	if(TileIsOutOfBounds(sClosestOpponent))
	{
		return 0;
	}

	// Run through each friendly.
	for ( UINT8 iCounter = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID ; iCounter <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID ; iCounter ++ )
	{
		pFriend = MercPtrs[ iCounter ];		

		// Make sure that character is alive, not too shocked, and conscious
		if (pFriend != pSoldier && 
			pFriend->bActive && 
			pFriend->stats.bLife >= OKLIFE)
		{
			//sFriendClosestOpponent = ClosestKnownOpponent( pFriend, NULL, NULL );
			sFriendClosestOpponent = ClosestSeenOpponent( pFriend, NULL, NULL );
			if(!TileIsOutOfBounds(sFriendClosestOpponent) &&
				PythSpacesAway( sClosestOpponent, sFriendClosestOpponent ) < DAY_VISION_RANGE / 4 &&
				pFriend->aiData.bAlertStatus == STATUS_BLACK &&
				pFriend->stats.bLife > pFriend->stats.bLifeMax / 2 &&
				( GetNearestRottingCorpseAIWarning( pFriend->sGridNo ) == 0 && !InLightAtNight(pFriend->sGridNo, pFriend->pathing.bLevel) ||
				pFriend->bActionPoints > 3*pFriend->bInitialActionPoints/4 || 
				pFriend->aiData.bLastAttackHit )
				)
			{
				ubFriendCount++;
			}
		}
	}

	return ubFriendCount;
}

// sevenfm: count nearby friend soldiers (on roof)
UINT8 CountNearbyFriendsOnRoof( SOLDIERTYPE *pSoldier, INT32 sGridNo, UINT8 ubDistance )
{
	CHECKF(pSoldier);

	SOLDIERTYPE * pFriend;
	UINT8 ubFriendCount = 0;

	// Run through each friendly.
	for ( UINT8 iCounter = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID ; iCounter <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID ; iCounter ++ )
	{
		pFriend = MercPtrs[ iCounter ];

		if (pFriend != pSoldier && 
			pFriend->bActive && 
			pFriend->stats.bLife >= OKLIFE &&
			PythSpacesAway( sGridNo, pFriend->sGridNo ) <= ubDistance &&
			pFriend->pathing.bLevel > 0)
		{
			ubFriendCount++;
		}
	}

	return ubFriendCount;
}

// sevenfm: count nearby friend soldiers
UINT8 CountNearbyFriends( SOLDIERTYPE *pSoldier, INT32 sGridNo, UINT8 ubDistance )
{
	CHECKF(pSoldier);

	SOLDIERTYPE * pFriend;
	UINT8 ubFriendCount = 0;

	// Run through each friendly.
	for ( UINT8 iCounter = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID ; iCounter <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID ; iCounter ++ )
	{
		pFriend = MercPtrs[ iCounter ];
		// Make sure that character is alive, not too shocked, and conscious, and of higher experience level
		// than the character being suppressed.
		if (pFriend != pSoldier && pFriend->bActive && pFriend->stats.bLife >= OKLIFE &&
			PythSpacesAway( sGridNo, pFriend->sGridNo ) <= ubDistance )
		{
			ubFriendCount++;
		}
	}

	return ubFriendCount;
}

// count neutral civilians
UINT8 CountNearbyNeutrals(SOLDIERTYPE *pSoldier, INT32 sGridNo, INT16 sDistance)
{
	SOLDIERTYPE * pFriend;
	UINT8 ubFriendCount = 0;

	// safety check
	if (!pSoldier)
		return 0;

	for (UINT32 uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pFriend = MercSlots[uiLoop];

		if (pFriend &&
			pFriend != pSoldier &&
			pFriend->bActive &&
			pFriend->stats.bLife >= OKLIFE &&
			CONSIDERED_NEUTRAL(pSoldier, pFriend) &&
			PythSpacesAway(sGridNo, pFriend->sGridNo) <= sDistance)
		{
			ubFriendCount++;
		}
	}

	return ubFriendCount;
}

UINT8 CountFriendsNotAlerted(SOLDIERTYPE *pSoldier)
{
	SOLDIERTYPE * pFriend;
	UINT8 ubFriendCount = 0;

	// Run through each friendly.
	for (UINT8 iCounter = gTacticalStatus.Team[pSoldier->bTeam].bFirstID; iCounter <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; iCounter++)
	{
		pFriend = MercPtrs[iCounter];
		
		if (pFriend && 
			pFriend->bActive &&
			pFriend->stats.bLife >= OKLIFE &&
			(pSoldier->bTeam != CIV_TEAM || pSoldier->ubCivilianGroup != NON_CIV_GROUP && pFriend->ubCivilianGroup == pSoldier->ubCivilianGroup) &&
			pFriend->aiData.bAlertStatus < STATUS_RED)
		{
			ubFriendCount++;
		}
	}

	return ubFriendCount;
}

void AlertFriends(INT8 bTeam, UINT8 ubCivGroup)
{
	SOLDIERTYPE * pFriend;

	// Run through each friendly.
	for (UINT8 iCounter = gTacticalStatus.Team[bTeam].bFirstID; iCounter <= gTacticalStatus.Team[bTeam].bLastID; iCounter++)
	{
		pFriend = MercPtrs[iCounter];

		if (pFriend && 
			pFriend->bActive &&
			pFriend->stats.bLife >= OKLIFE &&
			(bTeam != CIV_TEAM || ubCivGroup != NON_CIV_GROUP && pFriend->ubCivilianGroup == ubCivGroup))
		{
			pFriend->aiData.bAlertStatus = max(pFriend->aiData.bAlertStatus, STATUS_RED);
		}
	}
}

UINT8 CountFriendsFlankSameSpot(SOLDIERTYPE *pSoldier, INT32 sSpot)
{
	CHECKF(pSoldier);

	SOLDIERTYPE * pFriend;
	UINT8 ubFriendCount = 0;
	UINT8 ubMaxDist = TACTICAL_RANGE / 2;
	UINT8 ubFlankLeft = 0;
	UINT8 ubFlankRight = 0;

	if (TileIsOutOfBounds(sSpot))
	{
		sSpot = ClosestKnownOpponent(pSoldier, NULL, NULL);
	}

	if (TileIsOutOfBounds(sSpot))
	{
		return 0;
	}

	// Run through each friendly.
	for (UINT8 iCounter = gTacticalStatus.Team[pSoldier->bTeam].bFirstID; iCounter <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; iCounter++)
	{
		pFriend = MercPtrs[iCounter];

		if (pFriend &&
			pFriend != pSoldier &&
			pFriend->bActive &&
			pFriend->stats.bLife >= OKLIFE &&
			pFriend->aiData.bAlertStatus == STATUS_RED &&
			pFriend->aiData.bOrders > ONGUARD)
		{
			// check if this friend flanks around the same spot
			if (pFriend->numFlanks > 0 && pFriend->numFlanks < MAX_FLANKS_RED &&
				!TileIsOutOfBounds(pFriend->lastFlankSpot) &&
				PythSpacesAway(pFriend->lastFlankSpot, sSpot) < ubMaxDist)
			{
				if (pFriend->flags.lastFlankLeft)
				{
					ubFlankLeft++;
				}
				else
				{
					ubFlankRight++;
				}
			}
		}
	}

	return ubFlankLeft + ubFlankRight;
}

UINT8 CountNearbyFriendsLastAttackHit( SOLDIERTYPE *pSoldier, INT32 sGridNo, UINT8 ubDistance )
{
	CHECKF(pSoldier);

	SOLDIERTYPE * pFriend;
	UINT8 ubFriendCount = 0;

	// Run through each friendly.
	for ( UINT8 iCounter = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID ; iCounter <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID ; iCounter ++ )
	{
		pFriend = MercPtrs[ iCounter ];

		if (pFriend != pSoldier &&
			pFriend->bActive &&
			pFriend->stats.bLife >= OKLIFE &&
			pFriend->aiData.bOrders > ONGUARD &&
			pFriend->aiData.bOrders != SNIPER &&
			PythSpacesAway( sGridNo, pFriend->sGridNo ) <= ubDistance &&
			pFriend->aiData.bLastAttackHit )
		{
			ubFriendCount++;
		}
	}

	return ubFriendCount;
}

// check if gun that AI can use is scoped
BOOLEAN AIGunScoped(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	INT8 bWeaponIn;
	OBJECTTYPE *pObj;

	if ( TANK( pSoldier ) )
	{
		return FALSE;
	}

	bWeaponIn = FindAIUsableObjClass( pSoldier, IC_GUN );

	if (bWeaponIn == NO_SLOT)
	{
		return FALSE;
	}

	pObj = &pSoldier->inv[bWeaponIn];

	if( UsingNewCTHSystem() )
	{
		return NCTHIsScoped(pObj);
	}
	else
	{
		return IsScoped(pObj);
	}
}

BOOLEAN AIGunInHandScoped(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	if( UsingNewCTHSystem() == false && IsScoped(&pSoldier->inv[HANDPOS]) )
	{
		return TRUE;
	}

	if( UsingNewCTHSystem() == true && NCTHIsScoped(&pSoldier->inv[HANDPOS]) )
	{
		return TRUE;
	}
	return FALSE;
}

// return range for the AI gun
UINT16 AIGunRange(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	INT8 bWeaponIn;
	OBJECTTYPE *pObj;

	if ( TANK( pSoldier ) )
	{
		return 0;
	}

	bWeaponIn = FindAIUsableObjClass( pSoldier, IC_GUN );

	if (bWeaponIn == NO_SLOT)
	{
		return 0;
	}

	pObj = &pSoldier->inv[bWeaponIn];

	return GunRange(pObj, pSoldier);
}

UINT8 CountSeenEnemiesLastTurn( SOLDIERTYPE* pSoldier )
{
	CHECKF(pSoldier);

	UINT8	ubTeamLoop;
	UINT8	ubIDLoop;
	UINT8	cnt = 0;

	for( ubTeamLoop = 0; ubTeamLoop < MAXTEAMS; ubTeamLoop++ )
	{
		if( !gTacticalStatus.Team[ubTeamLoop].bTeamActive )
			continue;

		if( gTacticalStatus.Team[ ubTeamLoop ].bSide != pSoldier->bSide )
		{
			// consider guys in this team, which isn't on our side
			for( ubIDLoop = gTacticalStatus.Team[ ubTeamLoop ].bFirstID; ubIDLoop <= gTacticalStatus.Team[ ubTeamLoop ].bLastID; ubIDLoop++ )
			{
				// if this guy SAW an enemy recently...
				if( pSoldier->aiData.bOppList[ ubIDLoop ] >= SEEN_CURRENTLY &&
					pSoldier->aiData.bOppList[ ubIDLoop ] <= SEEN_LAST_TURN )
				{
					cnt++;
				}
			}
		}
	}

	return cnt;
}

INT32 ClosestSeenLastTurnOpponent(SOLDIERTYPE *pSoldier, INT32 * psGridNo, INT8 * pbLevel)
{
	CHECKF(pSoldier);

	INT32 sGridNo, sClosestOpponent = NOWHERE;
	UINT32 uiLoop;
	INT32 iRange, iClosestRange = 1500;
	INT8	*pbPersOL;
	INT8	bLevel, bClosestLevel;
	SOLDIERTYPE * pOpp;

	bClosestLevel = -1;

	// look through this man's personal & public opplists for opponents known
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpp = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpp)
		{
			continue;			// next merc
		}

		// if this merc is neutral/on same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpp ) || (pSoldier->bSide == pOpp->bSide))
		{
			continue;			// next merc
		}

		// silversurfer: ignore empty vehicles
		if ( pOpp->ubWhatKindOfMercAmI == MERC_TYPE__VEHICLE && GetNumberInVehicle( pOpp->bVehicleID ) == 0 )
			continue;

		// Special stuff for Carmen the bounty hunter
		if (pSoldier->aiData.bAttitude == ATTACKSLAYONLY && pOpp->ubProfile != SLAY)
		{
			continue;	// next opponent
		}

		pbPersOL = pSoldier->aiData.bOppList + pOpp->ubID;

		// if this opponent is not seen currently or last turn
		if (*pbPersOL < SEEN_CURRENTLY || *pbPersOL > SEEN_LAST_TURN)
		{
			continue;			// next merc
		}

		// since we're dealing with seen people, use exact gridnos
		sGridNo = pOpp->sGridNo;
		bLevel = pOpp->pathing.bLevel;

		// if we are standing at that gridno(!, obviously our info is old...)
		if (sGridNo == pSoldier->sGridNo)
		{
			continue;			// next merc
		}

		// this function is used only for turning towards closest opponent or changing stance
		// as such, if they AI is in a building,
		// we should ignore people who are on the roof of the same building as the AI
		if ( (bLevel != pSoldier->pathing.bLevel) && SameBuilding( pSoldier->sGridNo, sGridNo ) )
		{
			continue;
		}

		// I hope this will be good enough; otherwise we need a fractional/world-units-based 2D distance function
		//sRange = PythSpacesAway( pSoldier->sGridNo, sGridNo);
		iRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, sGridNo );

		if (iRange < iClosestRange)
		{
			iClosestRange = iRange;
			sClosestOpponent = sGridNo;
			bClosestLevel = bLevel;
		}
	}

	if (psGridNo)
	{
		*psGridNo = sClosestOpponent;
	}
	if (pbLevel)
	{
		*pbLevel = bClosestLevel;
	}
	return( sClosestOpponent );
}

// check if we have a prone sight cover from known enemies at spot
BOOLEAN ProneSightCoverAtSpot(SOLDIERTYPE *pSoldier, INT32 sSpot, BOOLEAN fUnlimited)
{
	CHECKF(pSoldier);

	UINT32		uiLoop;
	SOLDIERTYPE *pOpponent;
	INT32		*pusLastLoc;
	INT8		*pbPersOL;
	INT8		*pbPublOL;
	INT8		*pbLastLevel;

	INT32		sThreatLoc;
	INT8		iThreatLevel;
	UINT16		usSightLimit;
	UINT16		usAdjustedSight;

	// look through all opponents for those we know of
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || pOpponent->stats.bLife < OKLIFE)
		{
			continue;			// next merc
		}

		// if this man is neutral / on the same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;			// next merc
		}

		pbPersOL = pSoldier->aiData.bOppList + pOpponent->ubID;
		pbPublOL = gbPublicOpplist[pSoldier->bTeam] + pOpponent->ubID;

		pusLastLoc = gsLastKnownOppLoc[pSoldier->ubID] + pOpponent->ubID;
		pbLastLevel = gbLastKnownOppLevel[pSoldier->ubID] + pOpponent->ubID;

		// if this opponent is unknown personally and publicly
		if ((*pbPersOL == NOT_HEARD_OR_SEEN) && (*pbPublOL == NOT_HEARD_OR_SEEN))
		{
			continue;			// next merc
		}

		// if personal knowledge is more up to date or at least equal
		if ((gubKnowledgeValue[*pbPublOL - OLDEST_HEARD_VALUE][*pbPersOL - OLDEST_HEARD_VALUE] > 0) ||
			(*pbPersOL == *pbPublOL))
		{
			// using personal knowledge, obtain opponent's "best guess" gridno
			sThreatLoc = *pusLastLoc;
			iThreatLevel = *pbLastLevel;
			//iThreatCertainty = ThreatPercent[*pbPersOL - OLDEST_HEARD_VALUE];
		}
		else
		{
			// using public knowledge, obtain opponent's "best guess" gridno
			sThreatLoc = gsPublicLastKnownOppLoc[pSoldier->bTeam][pOpponent->ubID];
			iThreatLevel = gbPublicLastKnownOppLevel[pSoldier->bTeam][pOpponent->ubID];
			//iThreatCertainty = ThreatPercent[*pbPublOL - OLDEST_HEARD_VALUE];
		}

		gbForceWeaponReady = true;
		usSightLimit = pOpponent->GetMaxDistanceVisible(sSpot, pSoldier->pathing.bLevel, CALC_FROM_ALL_DIRS);
		gbForceWeaponReady = false;

		usAdjustedSight = usSightLimit + usSightLimit * GetSightAdjustment(pOpponent, pSoldier, sSpot, pSoldier->pathing.bLevel, ANIM_PRONE) / 100;

		if (LocationToLocationLineOfSightTest(sThreatLoc, iThreatLevel, sSpot, pSoldier->pathing.bLevel, TRUE, usAdjustedSight, STANDING_LOS_POS, PRONE_LOS_POS) ||
			fUnlimited && LocationToLocationLineOfSightTest(sThreatLoc, iThreatLevel, sSpot, pSoldier->pathing.bLevel, TRUE, NO_DISTANCE_LIMIT, STANDING_LOS_POS, PRONE_LOS_POS))
		{
			return FALSE;
		}
	}

	return TRUE;
}

// check if we have a prone cover from known enemies at spot
BOOLEAN SightCoverAtSpot(SOLDIERTYPE *pSoldier, INT32 sSpot, BOOLEAN fUnlimited)
{
	CHECKF(pSoldier);

	UINT32		uiLoop;
	SOLDIERTYPE *pOpponent;
	INT32		*pusLastLoc;
	INT8		*pbPersOL;
	INT8		*pbPublOL;
	INT8		*pbLastLevel;

	INT32		sThreatLoc;
	INT8		iThreatLevel;
	UINT16		usSightLimit;
	UINT16		usAdjustedSight;

	// look through all opponents for those we know of
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[uiLoop];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || pOpponent->stats.bLife < OKLIFE)
		{
			continue;			// next merc
		}

		// if this man is neutral / on the same side, he's not an opponent
		if (CONSIDERED_NEUTRAL(pSoldier, pOpponent) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;			// next merc
		}

		pbPersOL = pSoldier->aiData.bOppList + pOpponent->ubID;
		pbPublOL = gbPublicOpplist[pSoldier->bTeam] + pOpponent->ubID;

		pusLastLoc = gsLastKnownOppLoc[pSoldier->ubID] + pOpponent->ubID;
		pbLastLevel = gbLastKnownOppLevel[pSoldier->ubID] + pOpponent->ubID;

		// if this opponent is unknown personally and publicly
		if ((*pbPersOL == NOT_HEARD_OR_SEEN) && (*pbPublOL == NOT_HEARD_OR_SEEN))
		{
			continue;			// next merc
		}

		// if personal knowledge is more up to date or at least equal
		if ((gubKnowledgeValue[*pbPublOL - OLDEST_HEARD_VALUE][*pbPersOL - OLDEST_HEARD_VALUE] > 0) ||
			(*pbPersOL == *pbPublOL))
		{
			// using personal knowledge, obtain opponent's "best guess" gridno
			sThreatLoc = *pusLastLoc;
			iThreatLevel = *pbLastLevel;
			//iThreatCertainty = ThreatPercent[*pbPersOL - OLDEST_HEARD_VALUE];
		}
		else
		{
			// using public knowledge, obtain opponent's "best guess" gridno
			sThreatLoc = gsPublicLastKnownOppLoc[pSoldier->bTeam][pOpponent->ubID];
			iThreatLevel = gbPublicLastKnownOppLevel[pSoldier->bTeam][pOpponent->ubID];
			//iThreatCertainty = ThreatPercent[*pbPublOL - OLDEST_HEARD_VALUE];
		}

		gbForceWeaponReady = true;
		usSightLimit = pOpponent->GetMaxDistanceVisible(sSpot, pSoldier->pathing.bLevel, CALC_FROM_ALL_DIRS);
		gbForceWeaponReady = false;

		usAdjustedSight = usSightLimit + usSightLimit * GetSightAdjustment(pOpponent, pSoldier, sSpot, pSoldier->pathing.bLevel, ANIM_STAND) / 100;

		if (LocationToLocationLineOfSightTest(sThreatLoc, iThreatLevel, sSpot, pSoldier->pathing.bLevel, TRUE, usAdjustedSight, STANDING_LOS_POS, STANDING_LOS_POS) ||
			fUnlimited && LocationToLocationLineOfSightTest(sThreatLoc, iThreatLevel, sSpot, pSoldier->pathing.bLevel, TRUE, NO_DISTANCE_LIMIT, STANDING_LOS_POS, STANDING_LOS_POS))
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOLEAN NightLight( void )
{
	if( GetTimeOfDayAmbientLightLevel() < NORMAL_LIGHTLEVEL_DAY + 2 )
	{
		return TRUE;
	}

	return FALSE;
}

UINT8 CountTeamSeeSoldier( INT8 bTeam, SOLDIERTYPE *pSoldier )
{
	SOLDIERTYPE *pFriend;
	UINT16 cnt;
	UINT8 ubFriends = 0;

	CHECKF(pSoldier);

	if( bTeam >= MAXTEAMS )
	{
		return 0;
	}

	for ( cnt = gTacticalStatus.Team[ bTeam ].bFirstID; cnt <= gTacticalStatus.Team[ bTeam ].bLastID; cnt++ )
	{
		pFriend = MercPtrs[ cnt ];

		if( pFriend->bActive &&
			pFriend->bInSector &&
			pFriend->stats.bLife >= OKLIFE &&
			!pFriend->bCollapsed &&
			!pFriend->bBreathCollapsed )
		{
			if (pFriend->aiData.bOppList[ pSoldier->ubID ] == SEEN_CURRENTLY ||
				pFriend->aiData.bOppList[ pSoldier->ubID ] == SEEN_THIS_TURN )
			{
				ubFriends++;
			}
		}
	}	

	return ubFriends;
}

BOOLEAN EnemyCanSeeMe( SOLDIERTYPE *pSoldier )
{
	UINT32		uiLoop;
	SOLDIERTYPE *pOpponent;

	CHECKF( pSoldier );

	//loop through all the enemies and determine the cover
	for (uiLoop = 0; uiLoop<guiNumMercSlots; ++uiLoop)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || !pOpponent->bActive || !pOpponent->bInSector || pOpponent->stats.bLife < OKLIFE)
		{
			continue;			// next merc
		}

		// if this man is neutral / on the same side, he's not an opponent
		if( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;			// next merc
		}

		// if opponent is collapsed/breath collapsed
		if( pOpponent->bCollapsed || pOpponent->bBreathCollapsed )
		{
			continue;
		}

		// check if he is captured
		if(pOpponent->usSoldierFlagMask & SOLDIER_POW)
		{
			continue;
		}

		// check that we see opponent and he can see us
		if( (pSoldier->aiData.bOppList[ pOpponent->ubID ] == SEEN_CURRENTLY ||
			pSoldier->aiData.bOppList[ pOpponent->ubID ] == SEEN_THIS_TURN ||
			gbPublicOpplist[pSoldier->bTeam][ pOpponent->ubID ] == SEEN_CURRENTLY) &&
			pOpponent->aiData.bOppList[ pSoldier->ubID ] == SEEN_CURRENTLY )
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOLEAN EnemyAlerted( SOLDIERTYPE *pSoldier )
{
	UINT32		uiLoop;
	SOLDIERTYPE *pOpponent;

	CHECKF( pSoldier );

	//loop through all the enemies and determine the cover
	for (uiLoop = 0; uiLoop<guiNumMercSlots; ++uiLoop)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || !pOpponent->bActive || !pOpponent->bInSector || pOpponent->stats.bLife < OKLIFE)
		{
			continue;			// next merc
		}

		// if this man is neutral / on the same side, he's not an opponent
		if( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;			// next merc
		}

		// if opponent is collapsed/breath collapsed
		if( pOpponent->bCollapsed || pOpponent->bBreathCollapsed )
		{
			continue;
		}

		// check if he is captured
		if(pOpponent->usSoldierFlagMask & SOLDIER_POW)
		{
			continue;
		}

		if( pOpponent->aiData.bAlertStatus >= STATUS_RED )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// find alerted opponent
BOOLEAN TeamEnemyAlerted(INT8 bTeam)
{
	UINT32		uiLoop;
	SOLDIERTYPE *pOpponent;

	//loop through all the enemies
	for (uiLoop = 0; uiLoop < guiNumMercSlots; ++uiLoop)
	{
		pOpponent = MercSlots[uiLoop];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || !pOpponent->bActive || !pOpponent->bInSector || pOpponent->stats.bLife < OKLIFE)
		{
			continue;			// next merc
		}

		if (!ValidTeamOpponent(bTeam, pOpponent))
		{
			continue;
		}

		// if opponent is collapsed/breath collapsed
		if (pOpponent->IsUnconscious())
		{
			continue;
		}

		if (pOpponent->aiData.bAlertStatus >= STATUS_RED)
		{
			return TRUE;
		}
	}

	return FALSE;
}

UINT32 CountSuspicionValue( SOLDIERTYPE *pSoldier )
{
	UINT32		uiLoop;
	SOLDIERTYPE *pOpponent;
	UINT32		uiValue;
	UINT32		uiTotalValue = 0;

	CHECKF( pSoldier );

	if( !pSoldier->bActive || !pSoldier->bInSector )
	{
		return 0;
	}

	// only new skill system
	if( !gGameOptions.fNewTraitSystem )
	{
		return 0;
	}

	//loop through all the enemies and determine the cover
	for (uiLoop = 0; uiLoop<guiNumMercSlots; ++uiLoop)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || pOpponent->stats.bLife < OKLIFE)
		{
			continue;			// next merc
		}

		// if this man is neutral / on the same side, he's not an opponent
		if( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;			// next merc
		}

		// creatures should not raise suspicion counter even if they are hostile to player
		if( pOpponent->bTeam == CREATURE_TEAM )
		{
			continue;
		}

		// if opponent is collapsed/breath collapsed
		if( pOpponent->bCollapsed || pOpponent->bBreathCollapsed )
		{
			continue;
		}

		// check if he is captured
		if(pOpponent->usSoldierFlagMask & SOLDIER_POW)
		{
			continue;
		}

		// check that this opponent sees us
		if( pOpponent->aiData.bOppList[ pSoldier->ubID ] == SEEN_CURRENTLY || 
			pOpponent->aiData.bAlertStatus >= STATUS_RED && 
			( pOpponent->aiData.bOppList[ pSoldier->ubID ] == SEEN_THIS_TURN || pOpponent->aiData.bOppList[ pSoldier->ubID ] == HEARD_THIS_TURN ) )
		{
			UINT8	ubCovertLevel = NUM_SKILL_TRAITS( pSoldier, COVERT_NT );
			INT16	sDistance = PythSpacesAway(pSoldier->sGridNo, pOpponent->sGridNo);
			INT8	bTownId = GetTownIdForSector( gWorldSectorX, gWorldSectorY );
			UINT8	ubSectorId = SECTOR(gWorldSectorX, gWorldSectorY);
			UINT8	ubSectorData = 0;

			ubSectorData = SectorExternalData[ubSectorId][gbWorldSectorZ].usCurfewValue;

			if ( NightLight() )			// suspicious at night
				ubSectorData = max( ubSectorData, 1 );
			if ( gbWorldSectorZ > 0 )	// underground we are always suspicious				
				ubSectorData = max( ubSectorData, 2 );

			// -----------------------------------------------------------------------------------------------------
			// calculate basic value 

			uiValue = 1 + SoldierDifficultyLevel( pOpponent );
			// add bonus for squad leader
			if (HAS_SKILL_TRAIT( pOpponent, SQUADLEADER_NT ) )
			{
				uiValue += NUM_SKILL_TRAITS( pOpponent, SQUADLEADER_NT );
			}
			// bonus when using flashlight
			if ( pSoldier->GetBestEquippedFlashLightRange() > 0 )
			{
				uiValue++;
			}
			// bonus if bleeding
			if ( pSoldier->bBleeding > 0 )
			{
				uiValue++;
			}
			// bonus for soldier state
			if ( MercUnderTheInfluence( pSoldier ) ||
				GetDrunkLevel( pSoldier ) != SOBER )
			{
				uiValue += 1;
			}
			// bonus if enemy is alerted
			if ( pOpponent->aiData.bAlertStatus >= STATUS_RED )
			{
				uiValue += 2;
			}
			// bonus in combat
			if ( GuySawEnemy( pOpponent ) || pOpponent->aiData.bUnderFire )
			{
				uiValue += 2;
			}
			// bonus in capital			
			if ( bTownId == MEDUNA )	
			{
				uiValue += 2;
			}
			// wearing or carrying backpack is suspicious
			if ( UsingNewInventorySystem() && FindBackpackOnSoldier( pSoldier ) )
			{
				uiValue += 2;
			}
			// bonus if spotting
			if( pSoldier->IsSpotting() )
			{
				uiValue += 2;
			}
			// bonus if soldier is carrying item with sight bonus in main hand
			if( pSoldier->inv[HANDPOS].exists() &&
				Item[ pSoldier->inv[HANDPOS].usItem ].dayvisionrangebonus > 0 || 
				Item[ pSoldier->inv[HANDPOS].usItem ].brightlightvisionrangebonus > 0 || 
				Item[ pSoldier->inv[HANDPOS].usItem ].nightvisionrangebonus > 0 || 
				Item[ pSoldier->inv[HANDPOS].usItem ].cavevisionrangebonus > 0 )
			{
				uiValue += 2;
			}

			// -----------------------------------------------------------------------------------------------------
			// multipliers

			// bonus if observing soldier sees more than one covert soldier
			//uiValue = uiValue * (200 - 100 / max(1, CountSeenCovertOpponents(pOpponent))) / 100;
			uiValue = uiValue * max(1, CountSeenCovertOpponents(pOpponent));

			// bonus depending on number of army men already killed
			if( gTacticalStatus.ubArmyGuysKilled > 0 )
			{
				uiValue = uiValue *(UINT32) sqrt((DOUBLE) gTacticalStatus.ubArmyGuysKilled);
			}			

			// bonus for suspicious movement mode
			if ( pSoldier->bStealthMode || 
				gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_STAND ||
				pSoldier->usAnimState == RUNNING )
			{
				uiValue = uiValue * 2;
			}			

			// soldier spies cause more suspicion (this can be compensated by skill)
			if( pSoldier->usSoldierFlagMask & SOLDIER_COVERT_SOLDIER )
			{
				uiValue = uiValue * 2;
			}

			// increase if spy is civilian and alert is raised
			if( pSoldier->usSoldierFlagMask & SOLDIER_COVERT_CIV && pOpponent->aiData.bAlertStatus >= STATUS_RED )
			{
				uiValue = uiValue * 4;
			}

			// bonus if weapon raised
			if( WeaponReady(pSoldier) )
			{
				uiValue = uiValue * 2;
			}

			// bonus from uniform type (admin - 1, regular - 2, elite - 3)
			uiValue = uiValue * pSoldier->UniformLevel();

			// -----------------------------------------------------------------------------------------------------
			// some modifiers can reduce suspicion level

			// reduce 2-4 times if soldier has covert trait
			if ( HAS_SKILL_TRAIT( pSoldier, COVERT_NT ) )
			{
				uiValue = uiValue / (2 * NUM_SKILL_TRAITS( pSoldier, COVERT_NT ));
			}
			// for special NPCs even without covert trait
			else if( pSoldier->usSoldierFlagMask & SOLDIER_COVERT_CIV && pSoldier->usSoldierFlagMask & SOLDIER_COVERT_NPC_SPECIAL )
			{
				uiValue = uiValue / 2;
			}
			// if observer is drunk, he is less suspicious
			if( GetDrunkLevel(pOpponent) > SOBER && GetDrunkLevel(pOpponent) < HUNGOVER )
			{
				uiValue -= uiValue * GetDrunkLevel(pOpponent) * 25 / 100;
			}

			// finally reduce according to the distance, 4 times at max day vision range
			if( sDistance > DAY_VISION_RANGE / 4 )
			{
				uiValue = uiValue * (DAY_VISION_RANGE / 4) / sDistance;
			}
			// bonus to value is very close and alert is raised
			else if( pOpponent->aiData.bAlertStatus >= STATUS_RED  )
			{
				uiValue = uiValue * (DAY_VISION_RANGE / 4) / (max(sDistance, 1));
			}

			// -----------------------------------------------------------------------------------------------------

			uiTotalValue += uiValue;
		}
	}

	return uiTotalValue;
}

BOOLEAN EnemySeenSoldierRecently( SOLDIERTYPE *pSoldier, UINT8 ubMax, BOOLEAN fOnlyAlerted )
{
	UINT32		uiLoop;
	SOLDIERTYPE *pOpponent;

	// loop through all the enemies
	for (uiLoop = 0; uiLoop<guiNumMercSlots; ++uiLoop)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || pOpponent->stats.bLife < OKLIFE)
		{
			continue;			// next merc
		}

		// if this man is neutral / on the same side, he's not an opponent
		if( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;			// next merc
		}

		// if opponent is collapsed/breath collapsed
		if( pOpponent->bCollapsed || pOpponent->bBreathCollapsed )
		{
			continue;
		}

		// check if he is captured
		if(pOpponent->usSoldierFlagMask & SOLDIER_POW)
		{
			continue;
		}

		if( fOnlyAlerted && pOpponent->aiData.bAlertStatus < STATUS_RED )
		{
			continue;
		}

		// check that this opponent sees us
		if( pOpponent->aiData.bOppList[ pSoldier->ubID ] >= SEEN_CURRENTLY && 
			pOpponent->aiData.bOppList[ pSoldier->ubID ] <= ubMax ||
			gbPublicOpplist[pOpponent->bTeam][pSoldier->ubID] >= SEEN_CURRENTLY &&
			gbPublicOpplist[pOpponent->bTeam][pSoldier->ubID] <= ubMax )
		{
			return( TRUE );
		}
	}

	return FALSE;
}

BOOLEAN EnemyHeardSoldierRecently( SOLDIERTYPE *pSoldier, UINT8 ubMax, BOOLEAN fOnlyAlerted )
{
	UINT32		uiLoop;
	SOLDIERTYPE *pOpponent;

	// loop through all the enemies
	for (uiLoop = 0; uiLoop<guiNumMercSlots; ++uiLoop)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || pOpponent->stats.bLife < OKLIFE)
		{
			continue;			// next merc
		}

		// if this man is neutral / on the same side, he's not an opponent
		if( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;			// next merc
		}

		// if opponent is collapsed/breath collapsed
		if( pOpponent->bCollapsed || pOpponent->bBreathCollapsed )
		{
			continue;
		}

		// check if he is captured
		if(pOpponent->usSoldierFlagMask & SOLDIER_POW)
		{
			continue;
		}

		if( fOnlyAlerted && pOpponent->aiData.bAlertStatus < STATUS_RED )
		{
			continue;
		}

		// check that this opponent sees us
		if( pOpponent->aiData.bOppList[ pSoldier->ubID ] <= HEARD_THIS_TURN && 
			pOpponent->aiData.bOppList[ pSoldier->ubID ] >= ubMax ||
			gbPublicOpplist[pOpponent->bTeam][pSoldier->ubID] <= HEARD_THIS_TURN &&
			gbPublicOpplist[pOpponent->bTeam][pSoldier->ubID] >= ubMax )
		{
			return( TRUE );
		}
	}

	return FALSE;
}

// count friends in black state or under fire
UINT8 CountTeamCombat( SOLDIERTYPE *pSoldier )
{
	CHECKF(pSoldier);

	SOLDIERTYPE * pFriend;
	UINT8 ubFriendCount = 0;

	// Run through each friendly.
	for ( UINT8 iCounter = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID ; iCounter <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID ; iCounter ++ )
	{
		pFriend = MercPtrs[ iCounter ];		

		// Make sure that character is alive, not too shocked, and conscious
		if( pFriend->bActive &&
			pFriend->bInSector &&
			pFriend->stats.bLife >= OKLIFE &&
			!pFriend->bCollapsed &&
			!pFriend->bBreathCollapsed )
		{
			if(	pFriend->aiData.bAlertStatus == STATUS_BLACK ||
				pFriend->aiData.bUnderFire )
			{
				ubFriendCount++;
			}
		}
	}

	return ubFriendCount;
}

UINT8 CountFriendsNeedHelp( SOLDIERTYPE *pSoldier )
{
	CHECKF(pSoldier);

	SOLDIERTYPE * pFriend;
	UINT8 ubFriendCount = 0;

	// Run through each friendly.
	for ( UINT8 iCounter = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID ; iCounter <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID ; iCounter ++ )
	{
		pFriend = MercPtrs[ iCounter ];		

		// Make sure that character is alive, not too shocked, and conscious
		if (pFriend != pSoldier && 
			pFriend->bActive && 
			pFriend->stats.bLife >= OKLIFE)
		{
			//if( pFriend->aiData.bUnderFire || CountSeenEnemiesLastTurn(pFriend) > CountNearbyFriends(pFriend, pFriend->sGridNo, DAY_VISION_RANGE/4) )
			if( CountSeenEnemiesLastTurn(pFriend) > CountNearbyFriends(pFriend, pFriend->sGridNo, DAY_VISION_RANGE/4) )
			{
				ubFriendCount++;
			}
		}
	}

	return ubFriendCount;
}

BOOLEAN GuyKnowsEnemyPosition( SOLDIERTYPE * pSoldier )
{
	UINT8		uiLoop;
	SOLDIERTYPE *pOpponent;

	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpponent || !pOpponent->bActive|| !pOpponent->bInSector)
		{
			continue;
		}

		// if this merc is neutral/on same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide) )
		{
			continue;
		}

		// sevenfm: ignore empty vehicles
		if( pOpponent->ubWhatKindOfMercAmI == MERC_TYPE__VEHICLE && GetNumberInVehicle( pOpponent->bVehicleID ) == 0 )
		{
			continue;
		}

		// if this guy knows something about this enemy
		if ( pSoldier->aiData.bOppList[ pOpponent->ubID ] != NOT_HEARD_OR_SEEN )
		{
			return( TRUE );
		}
		// check also public knowledge
		if ( gbPublicOpplist[pSoldier->bTeam][ pOpponent->ubID ] != NOT_HEARD_OR_SEEN )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}

BOOLEAN AICheckIsSniper(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	if( AIGunType(pSoldier) != GUN_SN_RIFLE )
	{
		return FALSE;
	}

	if( AIGunRange(pSoldier) / CELL_X_SIZE > DAY_VISION_RANGE &&
		AIGunScoped(pSoldier) &&
		pSoldier->stats.bMarksmanship > 90 && HAS_SKILL_TRAIT(pSoldier, SNIPER_NT) )
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN AICheckIsMarksman(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	if( AIGunType(pSoldier) != GUN_SN_RIFLE && 
		AIGunType(pSoldier) != GUN_RIFLE &&
		AIGunType(pSoldier) != GUN_AS_RIFLE )
	{
		return FALSE;
	}

	if( AIGunRange(pSoldier) / CELL_X_SIZE >= DAY_VISION_RANGE &&
		(AIGunScoped(pSoldier) || pSoldier->stats.bMarksmanship > 90 || HAS_SKILL_TRAIT(pSoldier, SNIPER_NT)) )
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN AICheckIsRadioOperator(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	if( pSoldier->CanUseRadio(FALSE) )
		return TRUE;

	return FALSE;
}

BOOLEAN AICheckIsMedic(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	if( !HAS_SKILL_TRAIT( pSoldier, DOCTOR_NT) )
	{
		return FALSE;
	}

	if( FindFirstAidKit( pSoldier ) != NO_SLOT ||
		FindMedKit( pSoldier ) != NO_SLOT )
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN AICheckIsMortarOperator(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	if ( pSoldier->HasMortar() )
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN AICheckIsOfficer(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	if( HAS_SKILL_TRAIT(pSoldier, SQUADLEADER_NT) )
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN AICheckIsCommander(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	if( NUM_SKILL_TRAITS( pSoldier, SQUADLEADER_NT ) > 1 )
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN AICheckIsMachinegunner(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	if( AIGunType(pSoldier) == GUN_LMG )
	{
		return TRUE;
	}
	return FALSE;
}

UINT16 AIGunType(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	INT8 bWeaponIn;

	bWeaponIn = FindAIUsableObjClass( pSoldier, IC_GUN );

	if (bWeaponIn == NO_SLOT)
	{
		return 0;
	}

	if(pSoldier->inv[bWeaponIn].exists())
	{
		return Weapon[pSoldier->inv[bWeaponIn].usItem].ubWeaponType;
	}
	return 0;
}

BOOLEAN CheckDoorAtGridno( UINT32 usGridNo )
{
	STRUCTURE *pStructure;

	pStructure = FindStructure( usGridNo, STRUCTURE_ANYDOOR );
	if ( pStructure != NULL)
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN CheckDoorNearGridno( UINT32 usGridNo )
{
	UINT8	ubMovementCost;
	INT32	sTempGridNo;
	UINT8	ubDirection;

	if( CheckDoorAtGridno(usGridNo) )
	{
		return TRUE;
	}

	for (ubDirection = 0; ubDirection < NUM_WORLD_DIRECTIONS; ubDirection++)
	{
		sTempGridNo = NewGridNo( usGridNo, DirectionInc( ubDirection ) );
		ubMovementCost = gubWorldMovementCosts[ sTempGridNo ][ ubDirection ][ 0 ];
		if ( IS_TRAVELCOST_DOOR( ubMovementCost ) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOLEAN FindBombNearby( SOLDIERTYPE *pSoldier, INT32 sGridNo, UINT8 ubDistance, BOOLEAN fCheckSight )
{
	UINT32	uiBombIndex;
	INT32	sCheckGridno;
	OBJECTTYPE *pObj;

	INT16 sMaxLeft, sMaxRight, sMaxUp, sMaxDown, sXOffset, sYOffset;

	// determine maximum horizontal limits
	sMaxLeft  = min( ubDistance, (sGridNo % MAXCOL));
	sMaxRight = min( ubDistance, MAXCOL - ((sGridNo % MAXCOL) + 1));

	// determine maximum vertical limits
	sMaxUp   = min( ubDistance, (sGridNo / MAXROW));
	sMaxDown = min( ubDistance, MAXROW - ((sGridNo / MAXROW) + 1));

	for (sYOffset = -sMaxUp; sYOffset <= sMaxDown; sYOffset++)
	{
		for (sXOffset = -sMaxLeft; sXOffset <= sMaxRight; sXOffset++)
		{
			sCheckGridno = sGridNo + sXOffset + (MAXCOL * sYOffset);

			if( TileIsOutOfBounds(sCheckGridno) )
			{
				continue;
			}

			// search all bombs that we can see
			for (uiBombIndex = 0; uiBombIndex < guiNumWorldBombs; uiBombIndex++)
			{
				if (gWorldBombs[ uiBombIndex ].fExists &&
					gWorldItems[ gWorldBombs[ uiBombIndex ].iItemIndex ].sGridNo == sCheckGridno &&
					gWorldItems[ gWorldBombs[ uiBombIndex ].iItemIndex ].ubLevel == pSoldier->pathing.bLevel &&
					gWorldItems[ gWorldBombs[ uiBombIndex ].iItemIndex ].bVisible == VISIBLE &&
					gWorldItems[ gWorldBombs[ uiBombIndex ].iItemIndex ].usFlags & WORLD_ITEM_ARMED_BOMB ) //&&
					//(gWorldItems[ gWorldBombs[ uiBombIndex ].iItemIndex ].soldierID == -1 ||
					//MercPtrs[ gWorldItems[ gWorldBombs[ uiBombIndex ].iItemIndex ].soldierID ]->bSide != pSoldier->bSide ))
				{
					pObj = &( gWorldItems[ gWorldBombs[uiBombIndex].iItemIndex ].object );

					if( pObj && pObj->exists() && HasAttachmentOfClass( pObj, AC_REMOTEDET | AC_DETONATOR ) &&
						SoldierTo3DLocationLineOfSightTest( pSoldier, sCheckGridno, pSoldier->pathing.bLevel, 1, FALSE, fCheckSight ? CALC_FROM_WANTED_DIR : CALC_FROM_ALL_DIRS) )
						//(!fCheckSight || SoldierTo3DLocationLineOfSightTest( pSoldier, sCheckGridno, pSoldier->pathing.bLevel, 1, FALSE, CALC_FROM_WANTED_DIR )) )
					{
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

BOOLEAN TeamKnowsSoldier( INT8 bTeam, UINT8 ubID )
{
	SOLDIERTYPE *pFriend;
	UINT16 cnt;

	if( bTeam >= MAXTEAMS || ubID == NOBODY )
	{
		return FALSE;
	}

	if( gbPublicOpplist[bTeam][ubID] != NOT_HEARD_OR_SEEN )
	{
		return TRUE;
	}

	for ( cnt = gTacticalStatus.Team[ bTeam ].bFirstID; cnt <= gTacticalStatus.Team[ bTeam ].bLastID; cnt++ )
	{
		pFriend = MercPtrs[ cnt ];

		if( pFriend->bActive &&
			pFriend->bInSector &&
			pFriend->stats.bLife >= OKLIFE &&
			!pFriend->bCollapsed &&
			!pFriend->bBreathCollapsed &&
			pFriend->aiData.bOppList[ ubID ] != NOT_HEARD_OR_SEEN )
		{
			return TRUE;
		}
	}	

	return FALSE;
}

INT16 DistanceToClosestNotSeekEnemyFriend( SOLDIERTYPE *pSoldier, INT32 sGridNo )
{
	CHECKF(pSoldier);

	INT16 sDistance = 0;
	SOLDIERTYPE * pFriend;

	// Run through each friendly.
	for ( UINT8 iCounter = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID ; iCounter <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID ; iCounter ++ )
	{
		pFriend = MercPtrs[ iCounter ];		

		// Make sure that character is alive, not too shocked, and conscious
		if( pFriend->bActive &&
			pFriend->bInSector &&
			pFriend != pSoldier &&
			pFriend->aiData.bOrders != SEEKENEMY &&
			pFriend->stats.bLife >= OKLIFE &&
			!pFriend->bCollapsed &&
			!pFriend->bBreathCollapsed &&
			!(pFriend->usSoldierFlagMask & SOLDIER_POW) )
		{
			if(	sDistance == 0 || PythSpacesAway(sGridNo, pFriend->sGridNo) < sDistance )
			{
				sDistance = PythSpacesAway(sGridNo, pFriend->sGridNo);
			}
		}
	}

	return sDistance;
}

BOOLEAN LastTargetCollapsed( SOLDIERTYPE *pSoldier )
{
	CHECKF(pSoldier);
	UINT8 ubTarget;

	if(TileIsOutOfBounds(pSoldier->sLastTarget))
	{
		return FALSE;
	}

	// since we don't know the level of target, check both ground and roof levels
	ubTarget = WhoIsThere2( pSoldier->sLastTarget, 0 );
	if( ubTarget == NOBODY )
	{
		ubTarget = WhoIsThere2( pSoldier->sLastTarget, 1 );
	}

	// if still cannot find somebody, return FALSE
	if( ubTarget == NOBODY )
	{
		return FALSE;
	}

	// now check target state
	if( MercPtrs[ubTarget]->stats.bLife < OKLIFE ||
		MercPtrs[ubTarget]->bCollapsed ||
		MercPtrs[ubTarget]->bBreathCollapsed )
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN LastTargetSuppressed( SOLDIERTYPE *pSoldier )
{
	CHECKF(pSoldier);
	UINT8 ubTarget;

	if(TileIsOutOfBounds(pSoldier->sLastTarget))
	{
		return FALSE;
	}

	// since we don't know the level of target, check both ground and roof levels
	ubTarget = WhoIsThere2( pSoldier->sLastTarget, 0 );
	if( ubTarget == NOBODY )
	{
		ubTarget = WhoIsThere2( pSoldier->sLastTarget, 1 );
	}

	// if still cannot find somebody, return FALSE
	if( ubTarget == NOBODY )
	{
		return FALSE;
	}

	// now check target state
	if( CoweringShockLevel(MercPtrs[ubTarget]) )
	{
		return TRUE;
	}

	return FALSE;
}

// use soldier AI - merc bodytype, no robots/tanks/boxers/etc
BOOLEAN SoldierAI( SOLDIERTYPE *pSoldier )
{
	CHECKF(pSoldier);

	BOOLEAN fCivilian = (PTR_CIVILIAN && (pSoldier->ubCivilianGroup == NON_CIV_GROUP ||
		(pSoldier->aiData.bNeutral && gTacticalStatus.fCivGroupHostile[pSoldier->ubCivilianGroup] == CIV_GROUP_NEUTRAL) ||
		(pSoldier->ubBodyType >= FATCIV && pSoldier->ubBodyType <= CRIPPLECIV)));

	if(!IS_MERC_BODY_TYPE( pSoldier ) ||
		pSoldier->aiData.bNeutral ||
		fCivilian ||
		pSoldier->flags.uiStatusFlags & SOLDIER_BOXER ||
		TANK(pSoldier) ||
		pSoldier->flags.uiStatusFlags & SOLDIER_VEHICLE ||
		AM_A_ROBOT(pSoldier))
		return FALSE;

	return TRUE;
}

// danger percent based on distance to closest smoke effect
UINT8 RedSmokeDanger(INT32 sGridNo, INT8 bLevel)
{
	UINT32	uiCnt;
	INT32	sDist;
	INT32	sClosestDist;
	INT32	sMaxDist = min(gSkillTraitValues.usVOMortarRadius, DAY_VISION_RANGE / 2);
	INT32	sClosestSmoke = NOWHERE;
	UINT8	ubDangerPercent = 0;

	if (TileIsOutOfBounds(sGridNo))
	{
		return 0;
	}

	if (!gSkillTraitValues.fROAllowArtillery)
	{
		return 0;
	}

	// no artillery strike danger underground
	if (gbWorldSectorZ > 0)
	{
		return 0;
	}

	// check if artillery strike was ordered by any team
	if (!CheckArtilleryStrike())
	{
		return 0;
	}

	// no danger when in a building
	if (bLevel == 0 && CheckRoof(sGridNo))
	{
		return 0;
	}

	// deep water should be safe
	if (DeepWater(sGridNo, bLevel))
	{
		return 0;
	}

	//loop through all red smoke effects and find closest
	for (uiCnt = 0; uiCnt < guiNumSmokeEffects; uiCnt++)
	{
		if (gSmokeEffectData[uiCnt].fAllocated &&
			gSmokeEffectData[uiCnt].bType == SIGNAL_SMOKE_EFFECT &&
			!TileIsOutOfBounds(gSmokeEffectData[uiCnt].sGridNo))
		{
			sDist = PythSpacesAway(gSmokeEffectData[uiCnt].sGridNo, sGridNo);

			if (sClosestSmoke == NOWHERE || sDist < sClosestDist)
			{
				sClosestDist = sDist;
				sClosestSmoke = gSmokeEffectData[uiCnt].sGridNo;
			}
		}
	}

	// if we found red smoke, calculate danger percent based on distance
	// 0% at DAY_VISION_RANGE/2, 100% at zero range
	if (sClosestSmoke != NOWHERE)
	{
		ubDangerPercent = 100 * (sMaxDist - min(sMaxDist, sClosestDist)) / sMaxDist;
	}

	return ubDangerPercent;
}

BOOLEAN CheckRoof( INT32 sGridNo )
{
	if ( FindStructure( sGridNo, STRUCTURE_ROOF ) != NULL )
	{
		return TRUE;
	}

	return FALSE;
}

// check if artillery strike was ordered by any team
BOOLEAN CheckArtilleryStrike( void )
{
	UINT32	uiBombIndex;
	OBJECTTYPE *pObj;

	// search all bombs
	for (uiBombIndex = 0; uiBombIndex < guiNumWorldBombs; uiBombIndex++)
	{
		if (gWorldBombs[ uiBombIndex ].fExists &&
			gWorldItems[ gWorldBombs[ uiBombIndex ].iItemIndex ].usFlags & WORLD_ITEM_ARMED_BOMB )
		{
			pObj = &( gWorldItems[ gWorldBombs[uiBombIndex].iItemIndex ].object );

			if( pObj && pObj->exists() && (*pObj)[0]->data.ubWireNetworkFlag & ANY_ARTILLERY_FLAG )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOLEAN AICheckHasGun( SOLDIERTYPE *pSoldier )
{
	CHECKF(pSoldier);

	if( FindAIUsableObjClass( pSoldier, IC_GUN ) != NO_SLOT )
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN AICheckShortWeaponRange( SOLDIERTYPE *pSoldier )
{
	CHECKF(pSoldier);

	if( !AICheckHasGun( pSoldier ) )
	{
		return TRUE;
	}

	if( AIGunRange(pSoldier) < DAY_VISION_RANGE / 2 )
	{
		return TRUE;
	}

	return FALSE;
}

// check if we have any sight cover from known enemies at spot
BOOLEAN AnyCoverAtSpot( SOLDIERTYPE *pSoldier, INT32 sSpot )
{
	CHECKF(pSoldier);

	UINT32		uiLoop;
	SOLDIERTYPE *pOpponent;
	INT32		*pusLastLoc;
	INT8		*pbPersOL;
	INT8		*pbPublOL;
	INT8		*pbLastLevel;

	INT32		sThreatLoc;
	//INT32		iThreatCertainty;
	INT8		iThreatLevel;

	// look through all opponents for those we know of
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || pOpponent->stats.bLife < OKLIFE)
		{
			continue;			// next merc
		}

		// if this man is neutral / on the same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;			// next merc
		}

		pbPersOL = pSoldier->aiData.bOppList + pOpponent->ubID;
		pbPublOL = gbPublicOpplist[pSoldier->bTeam] + pOpponent->ubID;

		pusLastLoc = gsLastKnownOppLoc[pSoldier->ubID] + pOpponent->ubID;
		pbLastLevel = gbLastKnownOppLevel[pSoldier->ubID] + pOpponent->ubID;

		// if this opponent is unknown personally and publicly
		if ((*pbPersOL == NOT_HEARD_OR_SEEN) && (*pbPublOL == NOT_HEARD_OR_SEEN))
		{
			continue;			// next merc
		}

		// if personal knowledge is more up to date or at least equal
		// sevenfm: fix for unknown public location
		if ((gubKnowledgeValue[*pbPublOL - OLDEST_HEARD_VALUE][*pbPersOL - OLDEST_HEARD_VALUE] > 0) ||
			(*pbPersOL == *pbPublOL) ||
			*pbPersOL != NOT_HEARD_OR_SEEN && TileIsOutOfBounds(gsPublicLastKnownOppLoc[pSoldier->bTeam][pOpponent->ubID]) && !TileIsOutOfBounds(gsLastKnownOppLoc[pSoldier->ubID][pOpponent->ubID]) )
		{
			// using personal knowledge, obtain opponent's "best guess" gridno
			sThreatLoc = *pusLastLoc;
			iThreatLevel = *pbLastLevel;
			//iThreatCertainty = ThreatPercent[*pbPersOL - OLDEST_HEARD_VALUE];
		}
		else
		{
			// using public knowledge, obtain opponent's "best guess" gridno
			sThreatLoc = gsPublicLastKnownOppLoc[pSoldier->bTeam][pOpponent->ubID];
			iThreatLevel = gbPublicLastKnownOppLevel[pSoldier->bTeam][pOpponent->ubID];
			//iThreatCertainty = ThreatPercent[*pbPublOL - OLDEST_HEARD_VALUE];
		}

		if( !AnyCoverFromSpot( sSpot, pSoldier->pathing.bLevel, sThreatLoc, iThreatLevel ) &&
			LocationToLocationLineOfSightTest( sThreatLoc, iThreatLevel, sSpot, pSoldier->pathing.bLevel, TRUE, NO_DISTANCE_LIMIT, STANDING_LOS_POS, PRONE_LOS_POS) )
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOLEAN AnyCoverFromSpot( INT32 sSpot, INT8 bLevel, INT32 sThreatLoc, INT8 bThreatLevel )
{
	UINT8	ubDirection;
	INT32	sCoverSpot;
	INT8	bCoverHeight;

	if( TileIsOutOfBounds( sSpot ) || TileIsOutOfBounds(sThreatLoc) )
	{
		return FALSE;
	}

	ubDirection = atan8(CenterX(sSpot), CenterY(sSpot), CenterX(sThreatLoc), CenterY(sThreatLoc));
	sCoverSpot = NewGridNo( sSpot, DirectionInc( ubDirection ) );

	if ( TileIsOutOfBounds( sCoverSpot ) )
	{
		return FALSE;
	}

	if ( WhoIsThere2( sCoverSpot, bLevel ) != NOBODY )
	{
		return FALSE;
	}

	if (IsLocationSittableExcludingPeople(sCoverSpot, bLevel))
	{
		return FALSE;
	}

	// explosive structure cannot provide cover!
	if (FindStructFlag(sCoverSpot, bLevel, STRUCTURE_EXPLOSIVE))
	{
		return FALSE;
	}

	// check that structure can provide enough cover
	if (StructureDensity(sCoverSpot, bLevel) < 25)
	{
		return FALSE;
	}

	bCoverHeight = GetTallestStructureHeight( sCoverSpot, bLevel );

	if ( bCoverHeight >= 2 )
	{
		return TRUE;
	}

	return FALSE;
}

// sevenfm: check if suppression is possible (count friends in the fire direction)
BOOLEAN CheckSuppressionDirection( SOLDIERTYPE *pSoldier, INT32 sTargetGridNo, INT8 bTargetLevel )
{
	SOLDIERTYPE * pFriend;
	UINT8 ubShootingDir;
	//UINT8 ubFriendDir;

	CHECKF(pSoldier);

	if(TileIsOutOfBounds(sTargetGridNo))
	{
		return FALSE;
	}

	ubShootingDir = atan8(CenterX(pSoldier->sGridNo),CenterY(pSoldier->sGridNo),CenterX(sTargetGridNo),CenterY(sTargetGridNo));

	UINT32 uiLoop;

	for (uiLoop = 0; uiLoop < guiNumMercSlots; ++uiLoop)
	{
		pFriend = MercSlots[ uiLoop ];

		if( pFriend &&
			pFriend != pSoldier &&
			pFriend->bActive &&
			pFriend->stats.bLife >= OKLIFE &&
			(pFriend->bSide == pSoldier->bSide || CONSIDERED_NEUTRAL(pSoldier, pFriend)) &&
			pFriend->pathing.bLevel == pSoldier->pathing.bLevel &&
			pFriend->pathing.bLevel == bTargetLevel &&
			ubShootingDir == atan8(CenterX(pSoldier->sGridNo),CenterY(pSoldier->sGridNo),CenterX(pFriend->sGridNo),CenterY(pFriend->sGridNo)) &&
			PythSpacesAway( pSoldier->sGridNo, pFriend->sGridNo) < 2 * DAY_VISION_RANGE &&
			AISoldierToSoldierChanceToGetThrough( pSoldier, pFriend ) > 0 &&
			LocationToLocationLineOfSightTest( pSoldier->sGridNo, pSoldier->pathing.bLevel, pFriend->sGridNo, pFriend->pathing.bLevel, TRUE, NO_DISTANCE_LIMIT) &&
			(gAnimControl[ pFriend->usAnimState ].ubHeight == ANIM_STAND || 
			pSoldier->bTeam == MILITIA_TEAM && pFriend->bTeam == CIV_TEAM && pFriend->aiData.bNeutral ||
			pFriend->bTeam == OUR_TEAM && gAnimControl[ pFriend->usAnimState ].ubHeight != ANIM_PRONE )	)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOLEAN AICheckNVG( SOLDIERTYPE *pSoldier )
{
	CHECKF(pSoldier);

	if( Item[pSoldier->inv[HEAD1POS].usItem].nightvisionrangebonus > 0 ||
		Item[pSoldier->inv[HEAD1POS].usItem].cavevisionrangebonus > 0 ||
		Item[pSoldier->inv[HEAD2POS].usItem].nightvisionrangebonus > 0 ||
		Item[pSoldier->inv[HEAD2POS].usItem].cavevisionrangebonus > 0 )
	{
		return TRUE;
	}

	return FALSE;
}

INT8 AIEstimateInterruptLevel( SOLDIERTYPE *pSoldier )
{
	CHECKF(pSoldier);

	INT8 bLevel;

	bLevel = ( 20*EffectiveExpLevel( pSoldier ) + EffectiveAgility( pSoldier, FALSE ) + 15 ) / 30;

	bLevel = __max(0, bLevel - pSoldier->aiData.bShock / 4);

	if ( TANK( pSoldier ) )
	{
		bLevel /= 2;
	}

	return bLevel;
}

INT8 FindMaxEnemyInterruptLevel( SOLDIERTYPE *pSoldier, INT32 sGridNo, INT8 blevel, UINT8 ubDistance )
{
	CHECKF(pSoldier);

	UINT32		uiLoop;
	SOLDIERTYPE *pOpponent;
	INT8		bMaxInterruptLevel = 0;
	INT8		bInterruptLevel;

	INT32		sThreatLoc;
	INT8		iThreatLevel;

	UINT8 ubNum = 0;

	// loop through all the enemies
	for (uiLoop = 0; uiLoop < guiNumMercSlots; ++uiLoop)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || pOpponent->stats.bLife < OKLIFE)
		{
			continue;
		}

		// if this man is neutral / on the same side, he's not an opponent
		if( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;
		}

		// check if he is captured
		if(pOpponent->usSoldierFlagMask & SOLDIER_POW)
		{
			continue;
		}

		// check personal/public knowledge
		if( pSoldier->aiData.bOppList[pOpponent->ubID] == NOT_HEARD_OR_SEEN &&
			gbPublicOpplist[pSoldier->bTeam][pOpponent->ubID] == NOT_HEARD_OR_SEEN )
		{
			continue;
		}

		sThreatLoc = gsPublicLastKnownOppLoc[pSoldier->bTeam][pOpponent->ubID];
		iThreatLevel = gbPublicLastKnownOppLevel[pSoldier->bTeam][pOpponent->ubID];

		// use personal knowledge if possible
		if( pSoldier->aiData.bOppList[pOpponent->ubID] != NOT_HEARD_OR_SEEN )
		{
			sThreatLoc = gsLastKnownOppLoc[pSoldier->ubID][pOpponent->ubID];
			iThreatLevel = gbLastKnownOppLevel[pSoldier->ubID][pOpponent->ubID];
		}

		// if for some reason known location is bad - skip
		if( TileIsOutOfBounds(sThreatLoc) )
		{
			continue;
		}

		// check distance
		if( PythSpacesAway(sThreatLoc, sGridNo ) > ubDistance )
		{
			continue;
		}

		// check level
		if( iThreatLevel != blevel )
		{
			continue;
		}

		bInterruptLevel = AIEstimateInterruptLevel(pOpponent);
		if( bInterruptLevel > bMaxInterruptLevel )
		{
			bMaxInterruptLevel = bInterruptLevel;
		}
	}

	return bMaxInterruptLevel;
}

UINT8 CountPublicKnownEnemies( SOLDIERTYPE *pSoldier, INT32 sGridNo, UINT8 ubDistance )
{
	CHECKF(pSoldier);

	UINT32		uiLoop;
	SOLDIERTYPE *pOpponent;

	INT32		sThreatLoc;
	INT8		iThreatLevel;

	UINT8 ubNum = 0;

	// loop through all the enemies
	for (uiLoop = 0; uiLoop < guiNumMercSlots; ++uiLoop)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || pOpponent->stats.bLife < OKLIFE)
		{
			continue;
		}

		// if this man is neutral / on the same side, he's not an opponent
		if( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;
		}

		// check if he is captured
		if(pOpponent->usSoldierFlagMask & SOLDIER_POW)
		{
			continue;
		}

		sThreatLoc = gsPublicLastKnownOppLoc[pSoldier->bTeam][pOpponent->ubID];
		iThreatLevel = gbPublicLastKnownOppLevel[pSoldier->bTeam][pOpponent->ubID];

		// check distance
		if( PythSpacesAway(sThreatLoc, sGridNo ) > ubDistance )
		{
			continue;
		}

		// check public knowledge
		if( gbPublicOpplist[pSoldier->bTeam][pOpponent->ubID] != NOT_HEARD_OR_SEEN )
		{
			ubNum ++;
		}
	}

	return ubNum;
}

UINT8 CountSeenCovertOpponents( SOLDIERTYPE *pSoldier )
{
	CHECKF(pSoldier);

	UINT8	ubTeamLoop;
	UINT8	ubIDLoop;
	UINT8	cnt = 0;

	for( ubTeamLoop = 0; ubTeamLoop < MAXTEAMS; ubTeamLoop++ )
	{
		if( !gTacticalStatus.Team[ubTeamLoop].bTeamActive )
			continue;

		if( gTacticalStatus.Team[ ubTeamLoop ].bSide != pSoldier->bSide )
		{
			// consider guys in this team, which isn't on our side
			for( ubIDLoop = gTacticalStatus.Team[ ubTeamLoop ].bFirstID; ubIDLoop <= gTacticalStatus.Team[ ubTeamLoop ].bLastID; ubIDLoop++ )
			{
				// check that opponent is covert and we see him currently
				if( pSoldier->aiData.bOppList[ubIDLoop] == SEEN_CURRENTLY &&
					MercPtrs[ubIDLoop]->usSoldierFlagMask & (SOLDIER_COVERT_CIV|SOLDIER_COVERT_SOLDIER) )
				{
					cnt++;
				}
			}
		}
	}

	return cnt;
}

UINT8 AIDirection(INT32 sSpot1, INT32 sSpot2)
{
	if(TileIsOutOfBounds(sSpot1) || TileIsOutOfBounds(sSpot2))
	{
		return DIRECTION_IRRELEVANT;
	}

	return atan8(CenterX(sSpot1),CenterY(sSpot1),CenterX(sSpot2),CenterY(sSpot2));
}

BOOLEAN ValidOpponent(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent)
{
	if (!pSoldier || !pOpponent)
	{
		return FALSE;
	}

	if (!pOpponent->bActive ||
		!pOpponent->bInSector ||
		pOpponent->stats.bLife <= 0 ||
		CONSIDERED_NEUTRAL(pSoldier, pOpponent) ||
		pSoldier->bSide == pOpponent->bSide ||
		pSoldier->aiData.bAttitude == ATTACKSLAYONLY && pOpponent->ubProfile != SLAY ||
		pOpponent->IsEmptyVehicle() ||
		gTacticalStatus.bBoxingState == BOXING && pSoldier->IsBoxer() && !pOpponent->IsBoxer() ||
		pOpponent->ubBodyType == CROW)
	{
		return FALSE;
	}

	return TRUE;
}

BOOLEAN ValidTeamOpponent(INT8 bTeam, SOLDIERTYPE* pOpponent)
{
	if (bTeam >= MAXTEAMS || !pOpponent)
	{
		return FALSE;
	}

	if (!pOpponent->bActive ||
		!pOpponent->bInSector ||
		pOpponent->stats.bLife <= 0 ||
		CONSIDERED_NEUTRAL_TEAM(bTeam, pOpponent) ||
		gTacticalStatus.Team[bTeam].bSide == pOpponent->bSide ||
		pOpponent->IsEmptyVehicle())
	{
		return FALSE;
	}

	return TRUE;
}

INT16 VisionRange(void)
{
	INT16 sDist = MaxNormalDistanceVisible();
	INT8 bLightLevel = GetTimeOfDayAmbientLightLevel();

	sDist = sDist * gGameExternalOptions.ubBrightnessVisionMod[bLightLevel] / 100;

	// Adjust it based on weather...
	if ( guiEnvWeather & ( WEATHER_FORECAST_SHOWERS | WEATHER_FORECAST_THUNDERSHOWERS ) )
	{
		INT16 sWeatherPenalty = 0; // percent vision reduction 0-100%
		sWeatherPenalty = min(gGameExternalOptions.ubVisDistDecreasePerRainIntensity * gbCurrentRainIntensity, 100);

		sDist = (sDist * ( 100 - sWeatherPenalty )) / 100;
	}

	/*if( gfLightningInProgress )
	{
		sDist += sDist * ( ubRealAmbientLightLevel ) / 10;
	}*/

	return sDist;
}

INT16 DayVisionRange(void)
{
	INT16 sDist = MaxNormalDistanceVisible();
	INT8 bLightLevel = NORMAL_LIGHTLEVEL_DAY;

	sDist = sDist * gGameExternalOptions.ubBrightnessVisionMod[bLightLevel] / 100;

	return sDist;
}

INT16 NightVisionRange(void)
{
	INT16 sDist = MaxNormalDistanceVisible();
	INT8 bLightLevel = NORMAL_LIGHTLEVEL_NIGHT;

	sDist = sDist * gGameExternalOptions.ubBrightnessVisionMod[bLightLevel] / 100;

	return sDist;
}

BOOLEAN FindFenceAroundSpot(INT32 sSpot)
{
	if (TileIsOutOfBounds(sSpot))
	{
		return FALSE;
	}

	UINT8 ubDirection;
	INT32 sTempSpot;

	// check adjacent locations
	for (ubDirection = 0; ubDirection < NUM_WORLD_DIRECTIONS; ubDirection++)
	{
		sTempSpot = NewGridNo(sSpot, DirectionInc(ubDirection));

		if (sTempSpot != sSpot && IsCuttableWireFenceAtGridNo(sTempSpot))
		{
			return TRUE;
		}
	}

	return FALSE;
}

UINT8 FlankingDirection(SOLDIERTYPE *pSoldier)
{
	if (!pSoldier)
	{
		return DIRECTION_IRRELEVANT;
	}

	if (!AICheckIsFlanking(pSoldier) || TileIsOutOfBounds(pSoldier->lastFlankSpot))
	{
		return DIRECTION_IRRELEVANT;
	}

	UINT8 ubDir = AIDirection(pSoldier->sGridNo, pSoldier->lastFlankSpot);

	// determine desired direction
	if (pSoldier->flags.lastFlankLeft)
	{
		return gTwoCCDirection[ubDir];
	}
	else
	{
		return gTwoCDirection[ubDir];
	}
}

BOOLEAN WeAttack(INT8 bTeam)
{
	if (bTeam >= MAXTEAMS)
	{
		return FALSE;
	}

	if (bTeam != ENEMY_TEAM)
	{
		return FALSE;
	}

	// check that every soldier has SEEKENEMY order
	SOLDIERTYPE * pFriend;

	// Run through each friendly.
	for (UINT8 iCounter = gTacticalStatus.Team[bTeam].bFirstID; iCounter <= gTacticalStatus.Team[bTeam].bLastID; iCounter++)
	{
		pFriend = MercPtrs[iCounter];

		if (pFriend &&
			pFriend->bActive &&
			pFriend->stats.bLife >= OKLIFE &&
			pFriend->aiData.bOrders != SEEKENEMY)
		{
			return FALSE;
		}
	}

	return TRUE;
}

UINT8 CountKnownEnemiesInDirection(SOLDIERTYPE *pSoldier, UINT8 ubDirection, INT16 sDistance, BOOLEAN fAdjacent)
{
	CHECKF(pSoldier);

	UINT32		uiLoop;
	SOLDIERTYPE *pOpponent;

	INT32		sThreatLoc;
	INT8		iThreatLevel;

	UINT8		ubNum = 0;

	// loop through all the enemies
	for (uiLoop = 0; uiLoop < guiNumMercSlots; ++uiLoop)
	{
		pOpponent = MercSlots[uiLoop];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || pOpponent->stats.bLife < OKLIFE)
		{
			continue;
		}

		if (!ValidOpponent(pSoldier, pOpponent))
		{
			continue;
		}

		// check knowledge
		if (Knowledge(pSoldier, pOpponent->ubID) == NOT_HEARD_OR_SEEN)
		{
			continue;
		}

		sThreatLoc = KnownLocation(pSoldier, pOpponent->ubID);
		iThreatLevel = KnownLevel(pSoldier, pOpponent->ubID);

		if (TileIsOutOfBounds(sThreatLoc))
		{
			continue;
		}

		if (PythSpacesAway(pSoldier->sGridNo, sThreatLoc) > sDistance)
		{
			continue;
		}

		if (AIDirection(pSoldier->sGridNo, sThreatLoc) != ubDirection &&
			(!fAdjacent || AIDirection(pSoldier->sGridNo, sThreatLoc) != gOneCDirection[ubDirection] && AIDirection(pSoldier->sGridNo, sThreatLoc) != gOneCCDirection[ubDirection]))
		{
			continue;
		}

		ubNum++;
	}

	return ubNum;
}

INT8 Knowledge(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID)
{
	if (!pSoldier || ubOpponentID == NOBODY)
	{
		return NOT_HEARD_OR_SEEN;
	}

	if (UsePersonalKnowledge(pSoldier, ubOpponentID))
	{
		return PersonalKnowledge(pSoldier, ubOpponentID);
	}

	return PublicKnowledge(pSoldier->bTeam, ubOpponentID);
}

INT32 KnownLocation(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID)
{
	if (!pSoldier || ubOpponentID == NOBODY)
	{
		return NOWHERE;
	}

	if (UsePersonalKnowledge(pSoldier, ubOpponentID))
	{
		return KnownPersonalLocation(pSoldier, ubOpponentID);
	}

	return KnownPublicLocation(pSoldier->bTeam, ubOpponentID);
}

INT8 KnownLevel(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID)
{
	if (!pSoldier || ubOpponentID == NOBODY)
	{
		return 0;
	}

	if (UsePersonalKnowledge(pSoldier, ubOpponentID))
	{
		return KnownPersonalLevel(pSoldier, ubOpponentID);
	}

	return KnownPublicLevel(pSoldier->bTeam, ubOpponentID);
}

BOOLEAN UsePersonalKnowledge(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID)
{
	INT8		bPersonalKnowledge;
	INT8		bPublicKnowledge;

	if (!pSoldier || ubOpponentID == NOBODY)
	{
		return FALSE;
	}

	bPersonalKnowledge = PersonalKnowledge(pSoldier, ubOpponentID);
	bPublicKnowledge = PublicKnowledge(pSoldier->bTeam, ubOpponentID);

	if (gubKnowledgeValue[bPublicKnowledge - OLDEST_HEARD_VALUE][bPersonalKnowledge - OLDEST_HEARD_VALUE] > 0 ||
		bPersonalKnowledge != NOT_HEARD_OR_SEEN &&
		TileIsOutOfBounds(KnownPublicLocation(pSoldier->bTeam, ubOpponentID)) &&
		!TileIsOutOfBounds(KnownPersonalLocation(pSoldier, ubOpponentID)))
	{
		return TRUE;
	}

	return FALSE;
}

INT8 PersonalKnowledge(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID)
{
	if (!pSoldier || ubOpponentID == NOBODY)
	{
		return NOT_HEARD_OR_SEEN;
	}

	return pSoldier->aiData.bOppList[ubOpponentID];
}

INT32 KnownPersonalLocation(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID)
{
	if (!pSoldier || ubOpponentID == NOBODY)
	{
		return NOWHERE;
	}
	/*if(PersonalKnowledge(pSoldier, ubOpponentID) == NOT_HEARD_OR_SEEN)
	{
	return NOWHERE;
	}*/

	return gsLastKnownOppLoc[pSoldier->ubID][ubOpponentID];
}

INT8 KnownPersonalLevel(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID)
{
	if (!pSoldier || ubOpponentID == NOBODY)
	{
		return 0;
	}

	return gbLastKnownOppLevel[pSoldier->ubID][ubOpponentID];
}

INT8 PublicKnowledge(UINT8 bTeam, UINT8 ubOpponentID)
{
	if (bTeam >= MAXTEAMS || ubOpponentID == NOBODY)
	{
		return NOT_HEARD_OR_SEEN;
	}

	return gbPublicOpplist[bTeam][ubOpponentID];
}

INT32 KnownPublicLocation(UINT8 bTeam, UINT8 ubOpponentID)
{
	if (bTeam >= MAXTEAMS || ubOpponentID == NOBODY)
	{
		return NOWHERE;
	}
	/*if (PublicKnowledge(bTeam, ubOpponentID) == NOT_HEARD_OR_SEEN)
	{
	return NOWHERE;
	}*/

	return gsPublicLastKnownOppLoc[bTeam][ubOpponentID];
}

INT8 KnownPublicLevel(UINT8 bTeam, UINT8 ubOpponentID)
{
	if (bTeam >= MAXTEAMS || ubOpponentID == NOBODY)
	{
		return 0;
	}

	return gbPublicLastKnownOppLevel[bTeam][ubOpponentID];
}

// check that current loaded sector is town
BOOLEAN AICheckTown(void)
{
	// determine sector name
	UINT8 ubTownID = GetTownIdForSector(gWorldSectorX, gWorldSectorY);

	// not underground
	if (gbWorldSectorZ > 0)
	{
		return FALSE;
	}
	// check town sector
	if (ubTownID == BLANK_SECTOR)
	{
		return FALSE;
	}

	return TRUE;
}

UINT8 AISectorType(void)
{
	UINT8	ubSectorID = SECTOR(gWorldSectorX, gWorldSectorY);
	SECTORINFO *pSector = &SectorInfo[ubSectorID];
	UINT8	ubSectorType = PLAINS;
	if (pSector)
	{
		ubSectorType = pSector->ubTraversability[THROUGH_STRATEGIC_MOVE];
	}
	return ubSectorType;
}

// check that current loaded sector is underground
BOOLEAN AICheckUnderground(void)
{
	// not underground
	if (gbWorldSectorZ > 0)
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN NorthSpot(INT32 sSpot, INT8 bLevel)
{
	if (TileIsOutOfBounds(sSpot))
		return FALSE;

	if (bLevel > 0)
		return FALSE;

	INT32 sNextSpot = NewGridNo(sSpot, DirectionInc(NORTHWEST));

	if (gubWorldMovementCosts[sSpot + DirectionInc(NORTHWEST)][NORTHWEST][0] == TRAVELCOST_OFF_MAP ||
		gubWorldMovementCosts[sNextSpot + DirectionInc(NORTHWEST)][NORTHWEST][0] == TRAVELCOST_OFF_MAP)
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN AllowDeepWaterFlanking(SOLDIERTYPE *pSoldier)
{
	if (SoldierAI(pSoldier) &&
		pSoldier->bTeam == ENEMY_TEAM &&
		pSoldier->aiData.bOrders == SEEKENEMY &&
		(pSoldier->aiData.bAttitude == CUNNINGSOLO || gGameOptions.fNewTraitSystem && HAS_SKILL_TRAIT(pSoldier, ATHLETICS_NT)) &&
		pSoldier->aiData.bAlertStatus >= STATUS_RED &&
		!pSoldier->aiData.bUnderFire &&
		!GuySawEnemy(pSoldier))
	{
		return TRUE;
	}

	return FALSE;
}

INT32	RandomizeLocation(INT32 sSpot, INT8 bLevel, UINT8 ubTimes, SOLDIERTYPE *pSightSoldier)
{
	if (TileIsOutOfBounds(sSpot))
	{
		return NOWHERE;
	}

	UINT8 ubDirection;
	UINT8 ubMovementCost;
	INT32 sTempSpot;
	INT32 sSpotArray[NUM_WORLD_DIRECTIONS + 1];
	UINT8 ubSpots;

	for (UINT8 ubCnt = 0; ubCnt < ubTimes; ubCnt++)
	{
		// store original location
		ubSpots = 1;
		sSpotArray[0] = sSpot;

		// find adjacent locations
		for (ubDirection = 0; ubDirection < NUM_WORLD_DIRECTIONS; ubDirection++)
		{
			sTempSpot = NewGridNo(sSpot, DirectionInc(ubDirection));

			if (sTempSpot != sSpot)
			{
				ubMovementCost = gubWorldMovementCosts[sTempSpot][ubDirection][bLevel];

				if (ubMovementCost < TRAVELCOST_BLOCKED &&
					IsLocationSittableExcludingPeople(sTempSpot, bLevel) &&
					(!pSightSoldier || SoldierToVirtualSoldierLineOfSightTest(pSightSoldier, sTempSpot, bLevel, ANIM_STAND, TRUE, NO_DISTANCE_LIMIT)))
				{
					sSpotArray[ubSpots] = sTempSpot;
					ubSpots++;
				}
			}
		}
		// find random location
		sSpot = sSpotArray[Random(ubSpots)];
		// stop if could not find any adjacent spot
		if (ubSpots < 2)
		{
			break;
		}
	}

	return sSpot;
}

INT32	RandomizeOpponentLocation(INT32 sSpot, SOLDIERTYPE *pOpponent, INT16 sMaxDistance)
{
	if (TileIsOutOfBounds(sSpot))
	{
		return NOWHERE;
	}

	INT8 bXOffset, bYOffset;
	INT32 sRandomSpot;

	if (sMaxDistance > 0)
	{
		for (INT cnt = 0; cnt < min(sMaxDistance * 2, 100); cnt++)
		{
			bXOffset = Random(sMaxDistance * 2 + 1) - sMaxDistance;
			bYOffset = Random(sMaxDistance * 2 + 1) - sMaxDistance;

			sRandomSpot = sSpot + bXOffset + (MAXCOL * bYOffset);

			if (!TileIsOutOfBounds(sRandomSpot) && NewOKDestination(pOpponent, sRandomSpot, FALSE, pOpponent->pathing.bLevel))
			{
				return sRandomSpot;
			}
		}
	}

	return sSpot;
}

BOOLEAN InSmoke(INT32 sGridNo, INT8 bLevel)
{
	if (TileIsOutOfBounds(sGridNo))
	{
		return FALSE;
	}

	if (gpWorldLevelData[sGridNo].ubExtFlags[bLevel] & (MAPELEMENT_EXT_SMOKE))
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN AICheckSpecialRole(SOLDIERTYPE *pSoldier)
{
	if (AICheckIsSniper(pSoldier) || AICheckIsMachinegunner(pSoldier) || AICheckIsMortarOperator(pSoldier) || AICheckIsRadioOperator(pSoldier) || AICheckIsCommander(pSoldier))
		return TRUE;

	return FALSE;
}

BOOLEAN SafeSpot(SOLDIERTYPE *pSoldier, INT32 sSpot)
{
	if (!pSoldier)
	{
		return FALSE;
	}

	if (sSpot == NOWHERE)
	{
		sSpot = pSoldier->sGridNo;
	}

	INT8 bLevel = pSoldier->pathing.bLevel;
	BOOLEAN fUnlimitedSightCover = SightCoverAtSpot(pSoldier, sSpot, TRUE);
	BOOLEAN fProneSightCover = ProneSightCoverAtSpot(pSoldier, sSpot, FALSE);
	BOOLEAN fAnyCover = AnyCoverAtSpot(pSoldier, sSpot);

	if ((fUnlimitedSightCover || fProneSightCover && fAnyCover || InARoom(sSpot, NULL) && bLevel == 0 && (fAnyCover || fProneSightCover)) &&
		!InLightAtNight(sSpot, pSoldier->pathing.bLevel) &&
		!pSoldier->aiData.bUnderFire &&
		GetNearestRottingCorpseAIWarning(sSpot) == 0)
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN AbortFinalSpot(SOLDIERTYPE *pSoldier, INT32 sSpot, INT8 bAction, INT32 sClosestDisturbance, INT8 bDisturbanceLevel, INT32& sDangerousSpot)
{
	sDangerousSpot = NOWHERE;

	/*if (!pSoldier || TileIsOutOfBounds(sSpot))
	{
		return FALSE;
	}

	INT8 bLevel = pSoldier->pathing.bLevel;

	INT32	sOpponentGridNo;
	INT8	bOpponentLevel;
	INT32	sClosestOpponent = ClosestKnownOpponent(pSoldier, &sOpponentGridNo, &bOpponentLevel);

	if (TileIsOutOfBounds(sClosestDisturbance))
	{
		sClosestDisturbance = sClosestOpponent;
		bDisturbanceLevel = bOpponentLevel;
	}

	if (TileIsOutOfBounds(sClosestDisturbance))
	{
		return FALSE;
	}

	BOOLEAN fSightCover = SightCoverAtSpot(pSoldier, pSoldier->sGridNo, ANIM_STAND, FALSE);
	BOOLEAN fSightCoverUnlimited = SightCoverAtSpot(pSoldier, pSoldier->sGridNo, ANIM_STAND, TRUE);
	BOOLEAN fProneSightCover = SightCoverAtSpot(pSoldier, pSoldier->sGridNo, ANIM_PRONE, FALSE);
	BOOLEAN fProneSightCoverUnlimited = SightCoverAtSpot(pSoldier, pSoldier->sGridNo, ANIM_PRONE, TRUE);
	BOOLEAN fAnyCover = AnyCoverAtSpot(pSoldier, pSoldier->sGridNo);
	BOOLEAN fDangerousSpot = DangerousSpot(pSoldier);
	BOOLEAN fSafeSpot = SafeSpot(pSoldier);
	BOOLEAN fTerrainCover = TerrainCoverAtSpot(pSoldier, sSpot, pSoldier->pathing.bLevel);
	BOOLEAN fUnderAttack = AICheckSpotUnderAttack(pSoldier, pSoldier->sGridNo, FALSE);
	BOOLEAN fUnderSuccessfulAttack = AICheckSpotUnderAttack(pSoldier, pSoldier->sGridNo, TRUE);
	BOOLEAN fWeOutnumber = AICheckWeOutnumber(pSoldier, sClosestDisturbance);
	BOOLEAN fRushAttack = pSoldier->RushAttackAdvance();
	BOOLEAN fRetreat = pSoldier->RetreatActive();
	BOOLEAN fFlankingFriends = (CountFriendsFlankSameSpot(pSoldier, sClosestDisturbance) > 0);
	BOOLEAN fFriendsBlack = (CountFriendsBlack(pSoldier, sClosestDisturbance) > 0);
	BOOLEAN fSuccessfulAttack = AICheckSuccessfulAttack(pSoldier, TRUE);
	BOOLEAN fTakenLargeHit = pSoldier->TakenLargeHit();
	BOOLEAN fSeekEnemy = (pSoldier->aiData.bOrders == SEEKENEMY);
	BOOLEAN fCanAttackEnemy = CanAttackEnemy(pSoldier, ANIM_STAND, FALSE);
	BOOLEAN fInARoom = (InARoom(pSoldier->sGridNo, NULL) && bLevel == 0);
	BOOLEAN fWeAttack = WeAttack(pSoldier->bTeam);
	INT8	bMorale = pSoldier->aiData.bAIMorale;

	sDangerousSpot = NOWHERE;

	// abort seek if we see bomb
	if (FindBombNearby(pSoldier, sSpot, BOMB_DETECTION_RANGE))
	{
		DebugAI(AI_MSG_INFO, pSoldier, String("found bomb! abort!"));
		return TRUE;
	}

	// abort if moving into red smoke
	if (RedSmokeDanger(sSpot, pSoldier->pathing.bLevel) &&
		!RedSmokeDanger(pSoldier->sGridNo, pSoldier->pathing.bLevel))
	{
		DebugAI(AI_MSG_INFO, pSoldier, String("moving into red smoke! abort!"));
		return TRUE;
	}

	// don't go into light at night (includes smoke check)
	if (!pSoldier->RushAttackAdvance() &&
		InLightAtNight(sSpot, bLevel) &&
		!InLightAtNight(pSoldier->sGridNo, pSoldier->pathing.bLevel) &&
		!InSmoke(sSpot, bLevel, FALSE) &&
		(!fSeekEnemy || !SightCoverAtSpot(pSoldier, sSpot, ANIM_STAND, FALSE) || CountCorpses(pSoldier, sSpot, DAY_VISION_RANGE / 2, FALSE, TRUE) > 0) &&
		(fFlankingFriends || !fSuccessfulAttack || !fSeekEnemy) &&
		!fFriendsBlack)
	{
		DebugAI(AI_MSG_INFO, pSoldier, String("in light at night! abort!"));
		sDangerousSpot = sSpot;
		return TRUE;
	}

	// abort seeking when soldier sees fresh corpse
	if (!pSoldier->RushAttackAdvance() &&
		fSafeSpot &&
		CorpseWarning(pSoldier, sSpot, bLevel) &&
		!InSmoke(sSpot, bLevel, FALSE) &&
		!fFriendsBlack &&
		(fFlankingFriends || !fSuccessfulAttack || !fSeekEnemy || EnemyCanAttackSpot(pSoldier, sSpot, bLevel) || InARoom(sSpot, NULL) && bLevel == 0 || CorpseWarning(pSoldier, sSpot, bLevel, TRUE)))
	{
		DebugAI(AI_MSG_INFO, pSoldier, String("fresh corpse! abort!"));

		if (!SightCoverAtSpot(pSoldier, sSpot, ANIM_STAND, TRUE))
		{
			sDangerousSpot = sSpot;
		}
		return TRUE;
	}

	// check if enemy can attack spot
	if (!pSoldier->RushAttackAdvance() &&
		!InSmoke(sSpot, bLevel, FALSE) &&
		pSoldier->aiData.bAIMorale < MORALE_FEARLESS &&
		(!fSeekEnemy || fFlankingFriends) &&
		!TileIsOutOfBounds(sClosestDisturbance) &&
		PythSpacesAway(sSpot, sClosestDisturbance) < VISION_RANGE &&
		fSafeSpot &&
		!fFriendsBlack &&
		(!fSuccessfulAttack || !fSeekEnemy) &&
		EnemyCanAttackSpot(pSoldier, sSpot, bLevel))
	{
		DebugAI(AI_MSG_INFO, pSoldier, String("enemy can attack spot! abort!"));
		return TRUE;
	}

	// abort if moving out of sight cover and can attack enemy
	if (!pSoldier->RushAttackAdvance() &&
		!pSoldier->aiData.bUnderFire &&
		(bAction == AI_ACTION_SEEK_OPPONENT || bAction == AI_ACTION_TAKE_COVER) &&
		!TileIsOutOfBounds(sClosestDisturbance) &&
		PythSpacesAway(sSpot, sClosestDisturbance) <= (INT16)MAX_VISION_RANGE &&
		fSafeSpot &&
		(AICheckIsSniper(pSoldier) || AICheckIsMachinegunner(pSoldier) || AICheckIsMarksman(pSoldier)) &&
		fCanAttackEnemy &&
		(!SightCoverAtSpot(pSoldier, sSpot, ANIM_STAND, FALSE) || !AnyCoverAtSpot(pSoldier, sSpot)))
	{
		DebugAI(AI_MSG_INFO, pSoldier, String("moving out of sight cover, can attack enemy! abort!"));
		return TRUE;
	}*/

	return FALSE;
}

// needs prepared path before calling this function
BOOLEAN AbortPath(SOLDIERTYPE *pSoldier, INT8 bAction, INT32 sClosestDisturbance, INT8 bDisturbanceLevel, INT32& sDangerousSpot, INT32 &sLastSafeSpot)
{
	sDangerousSpot = NOWHERE;
	sLastSafeSpot = NOWHERE;

	/*if (!pSoldier)
	{
		return FALSE;
	}

	INT32	sOpponentGridNo;
	INT8	bOpponentLevel;
	INT32	sClosestOpponent = ClosestKnownOpponent(pSoldier, &sOpponentGridNo, &bOpponentLevel);

	if (TileIsOutOfBounds(sClosestDisturbance))
	{
		sClosestDisturbance = sClosestOpponent;
		bDisturbanceLevel = bOpponentLevel;
	}

	if (TileIsOutOfBounds(sClosestDisturbance))
	{
		return FALSE;
	}

	INT8 bLevel = pSoldier->pathing.bLevel;
	BOOLEAN fSightCover = SightCoverAtSpot(pSoldier, pSoldier->sGridNo, ANIM_STAND, FALSE);
	BOOLEAN fSightCoverUnlimited = SightCoverAtSpot(pSoldier, pSoldier->sGridNo, ANIM_STAND, TRUE);
	BOOLEAN fProneSightCover = SightCoverAtSpot(pSoldier, pSoldier->sGridNo, ANIM_PRONE, FALSE);
	BOOLEAN fProneSightCoverUnlimited = SightCoverAtSpot(pSoldier, pSoldier->sGridNo, ANIM_PRONE, TRUE);

	BOOLEAN fAnyCover = AnyCoverAtSpot(pSoldier, pSoldier->sGridNo);
	BOOLEAN fDangerousSpot = DangerousSpot(pSoldier);
	BOOLEAN fSafeSpot = SafeSpot(pSoldier);

	BOOLEAN fFlankingFriends = (CountFriendsFlankSameSpot(pSoldier, sClosestDisturbance) > 0);
	BOOLEAN fFriendsBlack = (CountFriendsBlack(pSoldier, sClosestDisturbance) > 0);
	BOOLEAN fSuccessfulAttack = AICheckSuccessfulAttack(pSoldier, TRUE);
	BOOLEAN fSeekEnemy = (pSoldier->aiData.bOrders == SEEKENEMY);
	BOOLEAN fInARoom = (InARoom(pSoldier->sGridNo, NULL) && bLevel == 0);

	BOOLEAN fWeAttack = WeAttack(pSoldier->bTeam);
	INT8	bMorale = pSoldier->aiData.bAIMorale;

	INT16 sLoop;
	INT32 sCheckGridNo = pSoldier->sGridNo;

	sDangerousSpot = NOWHERE;
	sLastSafeSpot = NOWHERE;

	for (sLoop = pSoldier->pathing.usPathIndex; sLoop < pSoldier->pathing.usPathDataSize; sLoop++)
	{
		sCheckGridNo = NewGridNo(sCheckGridNo, DirectionInc((UINT8)(pSoldier->pathing.usPathingData[sLoop])));

		// sevenfm: don't check fences
		if (IsJumpableFencePresentAtGridNo(sCheckGridNo))
		{
			continue;
		}

		// don't go into light at night (includes smoke check)
		if (!pSoldier->RushAttackAdvance() &&
			InLightAtNight(sCheckGridNo, bLevel) &&
			!InLightAtNight(pSoldier->sGridNo, pSoldier->pathing.bLevel) &&
			!InSmoke(sCheckGridNo, bLevel, FALSE) &&
			(!fSeekEnemy || !SightCoverAtSpot(pSoldier, sCheckGridNo, ANIM_STAND, FALSE) || CountCorpses(pSoldier, sCheckGridNo, DAY_VISION_RANGE / 2, FALSE, TRUE) > 0) &&
			(fFlankingFriends || !fSuccessfulAttack || !fSeekEnemy) &&
			!fFriendsBlack)
		{
			DebugAI(AI_MSG_INFO, pSoldier, String("in light at night! abort!"));
			sDangerousSpot = sCheckGridNo;
			return TRUE;
		}

		// check for fresh corpses
		if (!pSoldier->RushAttackAdvance() &&
			fSafeSpot &&
			CorpseWarning(pSoldier, sCheckGridNo, pSoldier->pathing.bLevel) &&
			!InSmoke(sCheckGridNo, pSoldier->pathing.bLevel, FALSE) &&
			!fFriendsBlack &&
			(fFlankingFriends || !fSuccessfulAttack || !fSeekEnemy || EnemyCanAttackSpot(pSoldier, sCheckGridNo, bLevel) || InARoom(sCheckGridNo, NULL) && bLevel == 0 || CorpseWarning(pSoldier, sCheckGridNo, bLevel, TRUE)))
		{
			DebugAI(AI_MSG_INFO, pSoldier, String("fresh corpse! abort!"));

			if (!SightCoverAtSpot(pSoldier, sCheckGridNo, ANIM_STAND, TRUE) || InARoom(sCheckGridNo, NULL) && bLevel == 0 || CorpseWarning(pSoldier, sCheckGridNo, bLevel, TRUE))
			{
				sDangerousSpot = sCheckGridNo;
			}
			return TRUE;
		}

		// check if enemy can attack spot
		if (!pSoldier->RushAttackAdvance() &&
			!InSmoke(sCheckGridNo, pSoldier->pathing.bLevel, FALSE) &&
			pSoldier->aiData.bAIMorale < MORALE_FEARLESS &&
			(!fSeekEnemy || fFlankingFriends) &&
			!TileIsOutOfBounds(sClosestDisturbance) &&
			PythSpacesAway(sCheckGridNo, sClosestDisturbance) < VISION_RANGE &&
			fSafeSpot &&
			!fFriendsBlack &&
			(!fSuccessfulAttack || !fSeekEnemy) &&
			EnemyCanAttackSpot(pSoldier, sCheckGridNo, bLevel))
		{
			DebugAI(AI_MSG_INFO, pSoldier, String("enemy can attack spot! abort!"));
			return TRUE;
		}

		if (SafeSpot(pSoldier, sCheckGridNo))
		{
			sLastSafeSpot = sCheckGridNo;
		}
	}*/

	return FALSE;
}

BOOLEAN CorpseWarning(SOLDIERTYPE *pSoldier, INT32 sGridNo, INT8 bLevel, BOOLEAN fFresh)
{
	CHECKF(pSoldier);

	INT32			cnt;
	ROTTING_CORPSE *pCorpse;
	UINT8			ubDistance = CORPSE_WARNING_DIST;

	for (cnt = 0; cnt < giNumRottingCorpse; ++cnt)
	{
		pCorpse = &(gRottingCorpse[cnt]);

		if (pCorpse &&
			pCorpse->fActivated &&
			pCorpse->def.ubType < ROTTING_STAGE2 &&
			//pCorpse->def.ubType <= FMERC_FALLF &&
			pCorpse->def.ubBodyType <= REGFEMALE &&
			pCorpse->def.ubAIWarningValue >(fFresh ? 1 : 0) &&
			pCorpse->def.bLevel == bLevel &&
			!TileIsOutOfBounds(pCorpse->def.sGridNo) &&
			PythSpacesAway(sGridNo, pCorpse->def.sGridNo) <= ubDistance &&
			(pSoldier->bTeam == ENEMY_TEAM && CorpseEnemyTeam(pCorpse) || pSoldier->bTeam == MILITIA_TEAM && CorpseMilitiaTeam(pCorpse) || pSoldier->bTeam == CIV_TEAM && !pSoldier->aiData.bNeutral))
		{
			return TRUE;
		}
	}

	return FALSE;
}

INT32	CountCorpses(SOLDIERTYPE *pSoldier, INT32 sSpot, INT16 sDistance, BOOLEAN fCheckSight, BOOLEAN fFresh)
{
	CHECKF(pSoldier);

	INT32			cnt;
	ROTTING_CORPSE *pCorpse;
	BOOLEAN			fCorpseOFAlly;

	for (cnt = 0; cnt < giNumRottingCorpse; ++cnt)
	{
		pCorpse = &(gRottingCorpse[cnt]);

		if (pCorpse &&
			pCorpse->fActivated &&
			pCorpse->def.ubType < ROTTING_STAGE2 &&
			pCorpse->def.ubBodyType <= REGFEMALE &&
			pCorpse->def.ubAIWarningValue >(fFresh ? 1 : 0) &&
			pCorpse->def.bLevel == pSoldier->pathing.bLevel &&
			!TileIsOutOfBounds(pCorpse->def.sGridNo) &&
			PythSpacesAway(sSpot, pCorpse->def.sGridNo) <= sDistance &&
			(!fCheckSight || SoldierToVirtualSoldierLineOfSightTest(pSoldier, pCorpse->def.sGridNo, pCorpse->def.bLevel, ANIM_PRONE, TRUE, CALC_FROM_ALL_DIRS)))
		{
			// test ally
			fCorpseOFAlly = FALSE;

			// check whether corpse was one of soldier's allies
			if (pSoldier->bTeam == ENEMY_TEAM)
			{
				for (UINT8 i = UNIFORM_ENEMY_ADMIN; i <= UNIFORM_ENEMY_ELITE; ++i)
				{
					if (COMPARE_PALETTEREP_ID(pCorpse->def.VestPal, gUniformColors[i].vest) && COMPARE_PALETTEREP_ID(pCorpse->def.PantsPal, gUniformColors[i].pants))
					{
						fCorpseOFAlly = TRUE;
						break;
					}
				}
			}
			else if (pSoldier->bTeam == MILITIA_TEAM)
			{
				for (UINT8 i = UNIFORM_MILITIA_ROOKIE; i <= UNIFORM_MILITIA_ELITE; ++i)
				{
					if (COMPARE_PALETTEREP_ID(pCorpse->def.VestPal, gUniformColors[i].vest) && COMPARE_PALETTEREP_ID(pCorpse->def.PantsPal, gUniformColors[i].pants))
					{
						fCorpseOFAlly = TRUE;
						break;
					}
				}
			}
			else
			{
				fCorpseOFAlly = TRUE;
			}

			if (fCorpseOFAlly)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOLEAN CorpseEnemyTeam(ROTTING_CORPSE *pCorpse)
{
	CHECKF(pCorpse);

	// check whether corpse has soldier's uniform
	for (UINT8 i = UNIFORM_ENEMY_ADMIN; i <= UNIFORM_ENEMY_ELITE; ++i)
	{
		if (COMPARE_PALETTEREP_ID(pCorpse->def.VestPal, gUniformColors[i].vest) && COMPARE_PALETTEREP_ID(pCorpse->def.PantsPal, gUniformColors[i].pants))
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOLEAN CorpseMilitiaTeam(ROTTING_CORPSE *pCorpse)
{
	CHECKF(pCorpse);

	// check whether corpse has soldier's uniform
	for (UINT8 i = UNIFORM_MILITIA_ROOKIE; i <= UNIFORM_MILITIA_ELITE; ++i)
	{
		if (COMPARE_PALETTEREP_ID(pCorpse->def.VestPal, gUniformColors[i].vest) && COMPARE_PALETTEREP_ID(pCorpse->def.PantsPal, gUniformColors[i].pants))
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOLEAN AICheckDefense(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	// only for enemy team
	if (pSoldier->bTeam != ENEMY_TEAM)
	{
		return FALSE;
	}

	// SEEKENEMY should always try to attack
	if (pSoldier->bTeam == SEEKENEMY)
	{
		return FALSE;
	}

	// only try to defend in towns and underground
	if (!AICheckTown() && !AICheckUnderground())
	{
		return FALSE;
	}

	return TRUE;
}

// count friends under fire or with shock
UINT8 CountTeamUnderAttack(INT8 bTeam, INT32 sGridNo, INT16 sDistance)
{
	SOLDIERTYPE * pFriend;
	UINT8 ubFriendCount = 0;

	// safety check
	if (bTeam >= MAXTEAMS)
		return 0;

	// Run through each friendly.
	for (UINT8 iCounter = gTacticalStatus.Team[bTeam].bFirstID; iCounter <= gTacticalStatus.Team[bTeam].bLastID; iCounter++)
	{
		pFriend = MercPtrs[iCounter];

		if (pFriend &&
			pFriend->bActive &&
			pFriend->stats.bLife >= OKLIFE &&
			PythSpacesAway(sGridNo, pFriend->sGridNo) <= sDistance &&
			(pFriend->aiData.bUnderFire || pFriend->aiData.bShock > 0))
		{
			ubFriendCount++;
		}
	}

	return ubFriendCount;
}

// check if soldier should advance using sight cover 
BOOLEAN UseSightCoverAdvance(SOLDIERTYPE *pSoldier)
{
	CHECKF(pSoldier);

	if (!SoldierAI(pSoldier))
	{
		return FALSE;
	}

	if (pSoldier->bTeam != ENEMY_TEAM)
	{
		return FALSE;
	}

	if (pSoldier->aiData.bOrders == STATIONARY)
	{
		return FALSE;
	}

	switch (pSoldier->ubSoldierClass)
	{
	case SOLDIER_CLASS_ELITE:
		return TRUE;
		break;
	case SOLDIER_CLASS_ARMY:
		if (pSoldier->aiData.bUnderFire ||
			pSoldier->aiData.bShock > 0 ||
			AICheckDefense(pSoldier) ||
			CorpseWarning(pSoldier, pSoldier->sGridNo, pSoldier->pathing.bLevel) ||
			CountTeamUnderAttack(pSoldier->bTeam, pSoldier->sGridNo, DAY_VISION_RANGE) > 0 ||
			CountCorpses(pSoldier, pSoldier->sGridNo, DAY_VISION_RANGE, TRUE, TRUE) > 0)
		{
			return TRUE;
		}
		break;
	case SOLDIER_CLASS_ADMINISTRATOR:
		if (pSoldier->aiData.bUnderFire ||
			pSoldier->aiData.bShock > 0 ||
			CorpseWarning(pSoldier, pSoldier->sGridNo, pSoldier->pathing.bLevel) ||
			CountTeamUnderAttack(pSoldier->bTeam, pSoldier->sGridNo, DAY_VISION_RANGE) > 0 ||
			CountCorpses(pSoldier, pSoldier->sGridNo, DAY_VISION_RANGE, TRUE, TRUE) > 0)
		{
			return TRUE;
		}
		break;
	}

	if (pSoldier->aiData.bAttitude == DEFENSIVE ||
		pSoldier->aiData.bAttitude == CUNNINGSOLO ||
		pSoldier->aiData.bAttitude == CUNNINGAID)
	{
		return TRUE;
	}

	if (TeamHighPercentKilled(pSoldier->bTeam))
	{
		return TRUE;
	}

	return FALSE;
}

UINT8 ArmyPercentKilled(void)
{
	if (gTacticalStatus.Team[ENEMY_TEAM].bMenInSector + gTacticalStatus.ubArmyGuysKilled == 0)
	{
		return 0;
	}

	return 100 * gTacticalStatus.ubArmyGuysKilled / (gTacticalStatus.Team[ENEMY_TEAM].bMenInSector + gTacticalStatus.ubArmyGuysKilled);
}

UINT8 TeamPercentKilled(INT8 bTeam)
{
	if (bTeam == ENEMY_TEAM)
	{
		return ArmyPercentKilled();
	}
	return 0;
}

BOOLEAN TeamHighPercentKilled(INT8 bTeam)
{
	if (bTeam == ENEMY_TEAM && ArmyPercentKilled() > ArmyPercentKilledTolerance())
	{
		return TRUE;
	}

	return FALSE;
}

// decide how many soldiers can be killed before alarm will be raised
UINT8 ArmyPercentKilledTolerance(void)
{
	// 50% at day, 25% at night, 25-33% for restricted sectors
	return 100 / (2 + SectorCurfew(TRUE));
}

UINT8 SectorCurfew(BOOLEAN fNight)
{
	UINT8	ubSectorId = SECTOR(gWorldSectorX, gWorldSectorY);
	UINT8	ubSectorData = 0;

	ubSectorData = SectorExternalData[ubSectorId][gbWorldSectorZ].usCurfewValue;

	if (fNight && NightLight())			// suspicious at night
		ubSectorData = max(ubSectorData, 1);

	if (gbWorldSectorZ > 0)	// underground we are always suspicious				
		ubSectorData = max(ubSectorData, 2);

	return ubSectorData;
}

BOOLEAN FindObstacleNearSpot(INT32 sSpot, INT8 bLevel)
{
	if (TileIsOutOfBounds(sSpot))
	{
		return FALSE;
	}
	/*if( !IsLocationSittableExcludingPeople(sSpot, bLevel) )
	{
	return FALSE;
	}*/

	UINT8	ubMovementCost;
	INT32	sTempGridNo;
	UINT8	ubDirection;

	// check adjacent reachable tiles
	for (ubDirection = 0; ubDirection < NUM_WORLD_DIRECTIONS; ubDirection++)
	{
		sTempGridNo = NewGridNo(sSpot, DirectionInc(ubDirection));

		if (sTempGridNo != sSpot)
		{
			ubMovementCost = gubWorldMovementCosts[sTempGridNo][ubDirection][bLevel];

			if (ubMovementCost >= TRAVELCOST_BLOCKED || !IsLocationSittableExcludingPeople(sTempGridNo, bLevel))
			{
				return(TRUE);
			}
		}
	}

	return FALSE;
}

// check that enemy can see and attack at spot
BOOLEAN EnemyCanAttackSpot(SOLDIERTYPE *pSoldier, INT32 sSpot, INT8 bLevel)
{
	CHECKF(pSoldier);

	UINT32		uiLoop;
	SOLDIERTYPE *pOpponent;
	INT8		bPersonalKnowledge, bPublicKnowledge;

	INT32		sThreatLoc;
	INT32		iThreatCertainty;
	INT8		iThreatLevel;

	// look through all opponents for those we know of
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[uiLoop];

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (!pOpponent || pOpponent->stats.bLife < OKLIFE)
		{
			continue;			// next merc
		}

		if (!ValidOpponent(pSoldier, pOpponent))
		{
			continue;
		}

		// if this opponent is unknown personally and publicly
		if (Knowledge(pSoldier, pOpponent->ubID) == NOT_HEARD_OR_SEEN)
		{
			continue;
		}

		sThreatLoc = KnownLocation(pSoldier, pOpponent->ubID);
		iThreatLevel = KnownLevel(pSoldier, pOpponent->ubID);
		iThreatCertainty = ThreatPercent[Knowledge(pSoldier, pOpponent->ubID) - OLDEST_HEARD_VALUE];

		bPersonalKnowledge = PersonalKnowledge(pSoldier, pOpponent->ubID);
		bPublicKnowledge = PublicKnowledge(pSoldier->bTeam, pOpponent->ubID);

		// safety check
		if (TileIsOutOfBounds(sThreatLoc))
		{
			continue;
		}

		// standard interrupt conditions
		if ((bPersonalKnowledge == SEEN_CURRENTLY || bPersonalKnowledge == SEEN_THIS_TURN || bPersonalKnowledge == HEARD_THIS_TURN ||
			bPublicKnowledge == SEEN_CURRENTLY || bPublicKnowledge == SEEN_THIS_TURN || bPublicKnowledge == HEARD_THIS_TURN))
		{
			if (!pOpponent->CanInterrupt())
			{
				continue;
			}
		}

		// ignore opponents without weapons
		if (!AICheckHasGun(pOpponent) &&
			PythSpacesAway(sThreatLoc, sSpot) > DAY_VISION_RANGE / 2)
		{
			continue;
		}

		// sevenfm: use real location/level
		//sThreatLoc = pOpponent->sGridNo;
		//iThreatLevel = pOpponent->pathing.bLevel;

		// if we have actual information, and this opponent has enough APs to attack
		//if( AIGunDeadliness(pOpponent) > 0 && 
		//	pOpponent->bActionPoints >= AIGunMinAPsToShoot(pOpponent) )

		// sevenfm: check only sight from enemy location to checked location
		if (LocationToLocationLineOfSightTest(sThreatLoc, iThreatLevel, sSpot, bLevel, TRUE, MAX_VISION_RANGE) &&
			//LOS_Location(sThreatLoc, iThreatLevel, sSpot, bLevel, MAX_VISION_RANGE) &&
			AIGunRange(pOpponent) * 3 / 2 >= PythSpacesAway(sThreatLoc, sSpot))
		{
			return TRUE;
		}
	}

	return FALSE;
}

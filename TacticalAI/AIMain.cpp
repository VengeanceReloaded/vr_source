#ifdef PRECOMPILEDHEADERS
#include "AI All.h"
#include "sound control.h"
#include "Debug Control.h"
#include "cheats.h"
#else
#include "wcheck.h"
#include "sgp.h"
#include "ai.h"
#include "Isometric Utils.h"
#include "overhead.h"
#include "math.h"
#include "Event Pump.h"
#include "Soldier Control.h"
#include "Overhead Types.h"
#include "sys globals.h"
#include "opplist.h"
#include "animation control.h"
#include "font control.h"
#include "interface.h"
#include "screenids.h"
#include "worldman.h"
#include "pathai.h"
#include "points.h"
#include "weapons.h"
#include "items.h"
#include "Handle Items.h"
#include "AIInternals.h"
#include "animation data.h"
#include "los.h"
#include "message.h"
#include "TeamTurns.h"
#include "NPC.h"
#include "Dialogue Control.h"
#include "Soldier Profile.h"
#include "strategicmap.h"
#include "Tactical Save.h"
#include "Soldier Create.h"
#include "Explosion Control.h"
#include "Interactive Tiles.h"
#include "interface dialogue.h"
#include "Vehicles.h"
#include "renderworld.h"
#include "AIList.h"
#include "Soldier macros.h"
#include "Bullets.h"
#include "physics.h"
#include "GameSettings.h"
#include "Debug Control.h"
#include "interface panels.h"
#include "Sound Control.h"
#include "civ quotes.h"
#include "Quests.h"
#include "Campaign Types.h"
#include "Queen Command.h"
#include "cheats.h"
#include "points.h"
#include "Soldier Functions.h" // added by SANDRO
#endif

#include "connect.h"

extern void PauseAITemporarily( void );
extern void UpdateEnemyUIBar( void );
extern void DisplayHiddenTurnbased( SOLDIERTYPE * pActingSoldier );
extern void AdjustNoAPToFinishMove( SOLDIERTYPE *pSoldier, BOOLEAN fSet );

void TurnBasedHandleNPCAI(SOLDIERTYPE *pSoldier);
void HandleAITacticalTraversal( SOLDIERTYPE * pSoldier );

extern UINT8 gubElementsOnExplosionQueue;

extern BOOLEAN gfWaitingForTriggerTimer;

UINT8 gubAICounter;

//
// Commented out/ to fix:
// lots of other stuff, I think
//

//extern UINT8 gubDeadLockDelay;

//#define DEADLOCK_DELAY							10000 //Madd - let's see what this does -- orig: 15000
//#define DEADLOCK_DELAY							( gGameExternalOptions.gubDeadLockDelay * 1000 )

// Very representing if this computer is the host, therefore controlling the ai
extern	BYTE			gfAmIHost;

//#define TESTAI

INT8 GameOption[MAXGAMEOPTIONS] = {0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0};

#define AI_LIMIT_PER_UPDATE		1

BOOLEAN gfTurnBasedAI;

INT8 gbDiff[MAX_DIFF_PARMS][5] =
{
	//       AI DIFFICULTY SETTING
	// WIMPY  EASY  NORMAL  TOUGH  ELITE
	{  -20,  -10,     0,    10,     20  },     // DIFF_ENEMY_EQUIP_MOD
	{  -10,   -5,     0,     5,     10  },     // DIFF_ENEMY_TO_HIT_MOD
	{   -2,   -1,     0,     1,      2  },     // DIFF_ENEMY_INTERRUPT_MOD
	{   50,   65,    80,    90,     95  },     // DIFF_RADIO_RED_ALERT
	{    4,    6,     8,    10,     13  }      // DIFF_MAX_COVER_RANGE
};




void EndAIGuysTurn( SOLDIERTYPE *pSoldier );

void DebugAI( STR szOutput )
{
#ifdef DEBUGDECISIONS
	// Send regular debug msg AND AI debug message
	FILE *		DebugFile;

	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, szOutput );
	if ((DebugFile = fopen( "aidebug.txt", "a+t" )) != NULL)
	{
		fputs( szOutput, DebugFile );
		fputs( "\n", DebugFile );
		fclose( DebugFile );
	}
#endif
}


BOOLEAN InitAI( void )
{
#ifdef JA2TESTVERSION
	FILE *		DebugFile;
#endif

#ifdef _DEBUG
	if (gfDisplayCoverValues)
	{
		//memset( gsCoverValue, 0x7F, sizeof( INT16 ) * WORLD_MAX );
	}
#endif

	//If we are not loading a saved game ( if we are, this has already been called )
	if( !( gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		//init the panic system
		InitPanicSystem();
	}

#ifdef JA2TESTVERSION
	// Clear the AI debug txt file to prevent it from getting huge
	if ((DebugFile = fopen( "aidebug.txt", "w" )) != NULL)
	{
		fputs( "\n", DebugFile );
		fclose( DebugFile );
	}
#endif

	return( TRUE );
}



BOOLEAN AimingGun(SOLDIERTYPE *pSoldier)
{
	return(FALSE);
}

void HandleSoldierAI( SOLDIERTYPE *pSoldier )
{
	UINT32 uiCurrTime = GetJA2Clock();

	// ATE
	// Bail if we are engaged in a NPC conversation/ and/or sequence ... or we have a pause because 
	// we just saw someone... or if there are bombs on the bomb queue
	if ( pSoldier->flags.uiStatusFlags & SOLDIER_ENGAGEDINACTION || gTacticalStatus.fEnemySightingOnTheirTurn || (gubElementsOnExplosionQueue != 0) )
	{
		return;
	}

	if ( gfExplosionQueueActive )
	{
		return;
	}

	if (pSoldier->flags.uiStatusFlags & SOLDIER_PC)
	{
		// if we're in autobandage, or the AI control flag is set and the player has a quote record to perform, or is a boxer,
		// let AI process this merc; otherwise abort
		if ( !(gTacticalStatus.fAutoBandageMode) && !(pSoldier->flags.uiStatusFlags & SOLDIER_PCUNDERAICONTROL && (pSoldier->ubQuoteRecord != 0 || pSoldier->flags.uiStatusFlags & SOLDIER_BOXER) ) )
		{
			// patch...
			if ( pSoldier->aiData.fAIFlags & AI_HANDLE_EVERY_FRAME )
			{
				pSoldier->aiData.fAIFlags &= ~AI_HANDLE_EVERY_FRAME;
			}
			return;
		}

	}
	/*
	else
	{		
	// AI is run on all PCs except the one who is selected
	if (pSoldier->flags.uiStatusFlags & SOLDIER_PC )
	{
	// if this soldier is "selected" then only let user give orders!
	if ((pSoldier->ubID == gusSelectedSoldier) && !(gTacticalStatus.uiFlags & DEMOMODE))
	{
	return;
	}
	}
	}
	*/

	// determine what sort of AI to use
	if ( (gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) )
	{
		gfTurnBasedAI = TRUE;
	}
	else
	{
		gfTurnBasedAI = FALSE;
	}

	// If TURN BASED and NOT NPC's turn, or realtime and not our chance to think, bail...
	if (gfTurnBasedAI)
	{
		if ( (pSoldier->bTeam != OUR_TEAM) && gTacticalStatus.ubCurrentTeam == gbPlayerNum )
		{
			return;
		}
		// why do we let the quote record thing be in here?  we're in turnbased the quote record doesn't matter,
		// we can't act out of turn!
		if ( !(pSoldier->flags.uiStatusFlags & SOLDIER_UNDERAICONTROL) )
			//if ( !(pSoldier->flags.uiStatusFlags & SOLDIER_UNDERAICONTROL) && (pSoldier->ubQuoteRecord == 0))
		{
			return;
		}

		if ( pSoldier->bTeam != gTacticalStatus.ubCurrentTeam )
		{
#ifdef JA2BETAVERSION
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_ERROR, L"Turning off AI flag for %d because trying to act out of turn", pSoldier->ubID );
#endif
			pSoldier->flags.uiStatusFlags &= ~SOLDIER_UNDERAICONTROL;
			return;
		}
		if ( pSoldier->aiData.bMoved )
		{
#ifdef TESTAICONTROL
			if (gfTurnBasedAI)
			{
				DebugAI( String("Ending turn for %d because set to moved", pSoldier->ubID ) );
			}
#endif
			// this guy doesn't get to act!
			EndAIGuysTurn( pSoldier );
			return;
		}

	}
	else if ( !(pSoldier->aiData.fAIFlags & AI_HANDLE_EVERY_FRAME) ) // if set to handle every frame, ignore delay!
	{
		//#ifndef AI_PROFILING
		//Time to handle guys in realtime (either combat or not )
		if ( !TIMECOUNTERDONE( pSoldier->timeCounters.AICounter, pSoldier->uiAIDelay ) )
		{
			// CAMFIELD, LOOK HERE!
			return;
		}
		else
		{
			//Reset counter!
			RESETTIMECOUNTER( pSoldier->timeCounters.AICounter, pSoldier->uiAIDelay );
			//DebugMsg( TOPIC_JA2, DBG_LEVEL_0, String( "%s waiting %d from %d", pSoldier->name, pSoldier->timeCounters.AICounter, uiCurrTime ) );
		}
		//#endif
	}

	if ( pSoldier->aiData.fAIFlags & AI_HANDLE_EVERY_FRAME ) // if set to handle every frame, ignore delay!
	{
		if (pSoldier->ubQuoteActionID != QUOTE_ACTION_ID_TURNTOWARDSPLAYER)
		{
			// turn off flag!
			pSoldier->aiData.fAIFlags &= (~AI_HANDLE_EVERY_FRAME);
		}
	}

	// if this NPC is getting hit, abort
	if (pSoldier->flags.fGettingHit)
	{
		return;
	}

	if ( gTacticalStatus.bBoxingState == PRE_BOXING || gTacticalStatus.bBoxingState == BOXING || gTacticalStatus.bBoxingState == WON_ROUND || gTacticalStatus.bBoxingState == LOST_ROUND )
	{
		if ( ! ( pSoldier->flags.uiStatusFlags & SOLDIER_BOXER ) )
		{
			// do nothing!
#ifdef TESTAICONTROL
			if (gfTurnBasedAI)
			{
				DebugAI( String("Ending turn for %d because not a boxer", pSoldier->ubID ) );
			}
#endif
			EndAIGuysTurn( pSoldier );
			return;
		}
	}

	// if this NPC is dying, bail
	if (pSoldier->stats.bLife < OKLIFE || !pSoldier->bActive )
	{
		if ( pSoldier->bActive && pSoldier->flags.fMuzzleFlash )
		{
			EndMuzzleFlash( pSoldier );
		}
#ifdef TESTAICONTROL
		if (gfTurnBasedAI)
		{
			DebugAI( String("Ending turn for %d because bad life/inactive", pSoldier->ubID ) );
		}
#endif

		EndAIGuysTurn( pSoldier );
		return;
	}

	if ( pSoldier->aiData.fAIFlags & AI_ASLEEP )
	{
		if ( gfTurnBasedAI && pSoldier->bVisible )
		{
			// turn off sleep flag, guy's got to be able to do stuff in turnbased
			// if he's visible
			pSoldier->aiData.fAIFlags &= ~AI_ASLEEP;
		}
		else if ( !(pSoldier->aiData.fAIFlags & AI_CHECK_SCHEDULE) )
		{
			// don't do anything!
#ifdef TESTAICONTROL
			if (gfTurnBasedAI)
			{
				DebugAI( String("Ending turn for %d because asleep and no scheduled action", pSoldier->ubID ) );
			}
#endif

			EndAIGuysTurn( pSoldier );
			return;
		}
	}

	if ( pSoldier->bInSector == FALSE && !(pSoldier->aiData.fAIFlags & AI_CHECK_SCHEDULE) )
	{
		// don't do anything!
#ifdef TESTAICONTROL
		if (gfTurnBasedAI)
		{
			DebugAI( String("Ending turn for %d because out of sector and no scheduled action", pSoldier->ubID ) );
		}
#endif

		EndAIGuysTurn( pSoldier );
		return;
	}

	if ( ( (pSoldier->flags.uiStatusFlags & SOLDIER_VEHICLE) && !TANK( pSoldier ) ) || AM_A_ROBOT( pSoldier ) )
	{
		// bail out!
#ifdef TESTAICONTROL
		if (gfTurnBasedAI)
		{
			DebugAI( String("Ending turn for %d because is vehicle or robot", pSoldier->ubID ) );
		}
#endif

		EndAIGuysTurn( pSoldier );
		return;
	}

	if (pSoldier->bCollapsed)
	{
		// being handled so turn off muzzle flash
		if ( pSoldier->flags.fMuzzleFlash )
		{
			EndMuzzleFlash( pSoldier );
		}

#ifdef TESTAICONTROL
		if (gfTurnBasedAI)
		{
			DebugAI( String("Ending turn for %d because unconscious", pSoldier->ubID ) );
		}
#endif

		// stunned/collapsed!
		CancelAIAction( pSoldier, FORCE );
		EndAIGuysTurn( pSoldier );
		return;
	}

	// in the unlikely situation (Sgt Krott et al) that we have a quote trigger going on
	// during turnbased, don't do any AI
	if ( pSoldier->ubProfile != NO_PROFILE && (pSoldier->ubProfile == SERGEANT || pSoldier->ubProfile == MIKE || pSoldier->ubProfile == JOE) && (gTacticalStatus.uiFlags & INCOMBAT) && (gfInTalkPanel || gfWaitingForTriggerTimer || !DialogueQueueIsEmpty() ) )
	{
		return;
	}

	// ATE: Did some changes here
	// DON'T rethink if we are determined to get somewhere....
	if ( pSoldier->aiData.bNewSituation == IS_NEW_SITUATION )
	{
		BOOLEAN fProcessNewSituation;

		// if this happens during an attack then do nothing... wait for the A.B.C.
		// to be reduced to 0 first -- CJC December 13th
		if ( gTacticalStatus.ubAttackBusyCount > 0 )
		{
			fProcessNewSituation = FALSE;
#if 0
			// HACK!!
			if ( pSoldier->aiData.bAction == AI_ACTION_FIRE_GUN )
			{
				if ( guiNumBullets == 0 )
				{
					// abort attack!
					//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String(">>>>>> Attack busy count lobotomized due to new situation for %d", pSoldier->ubID ) );
					//gTacticalStatus.ubAttackBusyCount = 0;
					fProcessNewSituation = TRUE;
				}
			}
			else if ( pSoldier->aiData.bAction == AI_ACTION_TOSS_PROJECTILE )
			{
				if ( guiNumObjectSlots == 0 )
				{
					// abort attack!
					DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String(">>>>>> Attack busy count lobotomized due to new situation for %d", pSoldier->ubID ) );
					//gTacticalStatus.ubAttackBusyCount = 0;
					fProcessNewSituation = TRUE;
				}
			}
#endif
		}
		else
		{
			fProcessNewSituation = TRUE;
		}

		if ( fProcessNewSituation )
		{
			if ( (pSoldier->flags.uiStatusFlags & SOLDIER_UNDERAICONTROL) && pSoldier->ubQuoteActionID >= QUOTE_ACTION_ID_TRAVERSE_EAST && pSoldier->ubQuoteActionID <= QUOTE_ACTION_ID_TRAVERSE_NORTH && !GridNoOnVisibleWorldTile( pSoldier->sGridNo ) )
			{
				// traversing offmap, ignore new situations
			}
			else if ( pSoldier->ubQuoteRecord == 0 && !gTacticalStatus.fAutoBandageMode  )
			{
				// don't force, don't want escorted mercs reacting to new opponents, etc.
				// now we don't have AI controlled escorted mercs though - CJC
				CancelAIAction( pSoldier, FORCE );
				// zap any next action too
				if ( pSoldier->aiData.bAction != AI_ACTION_END_COWER_AND_MOVE )
				{
					pSoldier->aiData.bNextAction = AI_ACTION_NONE;
				}
				DecideAlertStatus( pSoldier );
			}
			else
			{
				if ( pSoldier->ubQuoteRecord )
				{
					// make sure we're not using combat AI
					pSoldier->aiData.bAlertStatus = STATUS_GREEN;
				}
				pSoldier->aiData.bNewSituation = WAS_NEW_SITUATION;
			}
		}
	}
	else
	{
		// might have been in 'was' state; no longer so...
		pSoldier->aiData.bNewSituation = NOT_NEW_SITUATION;
	}

#ifdef TESTAI
	DebugMsg( TOPIC_JA2AI, DBG_LEVEL_3,String( ".... HANDLING AI FOR %d",pSoldier->ubID));
#endif

	/*********
	Start of new overall AI system
	********/



	if (gfTurnBasedAI)
	{
		if ( ( GetJA2Clock() - gTacticalStatus.uiTimeSinceMercAIStart	) > ( (UINT32)gGameExternalOptions.gubDeadLockDelay * 1000 ) && !gfUIInDeadlock )
		{
			// ATE: Display message that deadlock occured...
			LiveMessage( "Breaking Deadlock" );

#ifdef JA2TESTVERSION
			// display deadlock message
			gfUIInDeadlock = TRUE;
			gUIDeadlockedSoldier = pSoldier->ubID;
			DebugAI(  String("DEADLOCK soldier %d action %s ABC %d", pSoldier->ubID, gzActionStr[pSoldier->aiData.bAction], gTacticalStatus.ubAttackBusyCount ) );
#else

			// If we are in beta version, also report message!
#ifdef JA2BETAVERSION
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_ERROR, L"Aborting AI deadlock for %d. Please sent DEBUG.TXT file and SAVE.", pSoldier->ubID );
#endif
			// just abort
			EndAIDeadlock();
			if ( !(pSoldier->flags.uiStatusFlags & SOLDIER_UNDERAICONTROL) )
			{
				return;
			}
#endif
		}
	}

	// We STILL do not want to issue new orders while an attack busy situation is going on.  This can happen, for example,
	// when a gun is fired in real-time and an enemy sees it.  Delay the AI until that action is done.
	if ( gTacticalStatus.ubAttackBusyCount > 0 )
	{
		return;
	}

	if (pSoldier->aiData.bAction == AI_ACTION_NONE)
	{
		// being handled so turn off muzzle flash
		if ( pSoldier->flags.fMuzzleFlash )
		{
			EndMuzzleFlash( pSoldier );
		}

		gubAICounter++;
		// figure out what to do!
		if (gfTurnBasedAI)
		{
			if (pSoldier->flags.fNoAPToFinishMove)
			{
				// well that move must have been cancelled because we're thinking now!
				//pSoldier->flags.fNoAPToFinishMove = FALSE;
			}
			TurnBasedHandleNPCAI( pSoldier );
		}
		else
		{
			RTHandleAI( pSoldier );
		}

	}
	else
	{

		// an old action was in progress; continue it
		if (pSoldier->aiData.bAction >= FIRST_MOVEMENT_ACTION && pSoldier->aiData.bAction <= LAST_MOVEMENT_ACTION && !pSoldier->flags.fDelayedMovement)
		{
			if (pSoldier->pathing.usPathIndex == pSoldier->pathing.usPathDataSize)
			{				
				if (!TileIsOutOfBounds(pSoldier->sAbsoluteFinalDestination))
				{
					if ( !ACTING_ON_SCHEDULE( pSoldier ) &&  SpacesAway( pSoldier->sGridNo, pSoldier->sAbsoluteFinalDestination ) < 4 )
					{
						// This is close enough... reached final destination for NPC system move
						if ( pSoldier->sAbsoluteFinalDestination != pSoldier->sGridNo )
						{
							// update NPC records to replace our final dest with this location
							ReplaceLocationInNPCDataFromProfileID( pSoldier->ubProfile, pSoldier->sAbsoluteFinalDestination, pSoldier->sGridNo );
						}
						pSoldier->sAbsoluteFinalDestination = pSoldier->sGridNo;
						// change action data so that we consider this our final destination below
						pSoldier->aiData.usActionData = pSoldier->sGridNo;
					}

					if ( pSoldier->sAbsoluteFinalDestination == pSoldier->sGridNo )
					{
						pSoldier->sAbsoluteFinalDestination = NOWHERE;

						if ( !ACTING_ON_SCHEDULE( pSoldier ) && pSoldier->ubQuoteRecord && pSoldier->ubQuoteActionID == QUOTE_ACTION_ID_CHECKFORDEST )
						{
							NPCReachedDestination( pSoldier, FALSE );
							// wait just a little bit so the queue can be processed
							pSoldier->aiData.bNextAction = AI_ACTION_WAIT;
							pSoldier->aiData.usNextActionData = 500;

						}
						else if (pSoldier->ubQuoteActionID >= QUOTE_ACTION_ID_TRAVERSE_EAST && pSoldier->ubQuoteActionID <= QUOTE_ACTION_ID_TRAVERSE_NORTH)
						{
							HandleAITacticalTraversal( pSoldier );
							return;
						}
					}
					else
					{
						// make sure this guy is handled next frame!
						pSoldier->flags.uiStatusFlags |= AI_HANDLE_EVERY_FRAME;
					}
				}
				// for regular guys still have to check for leaving the map
				else if (pSoldier->ubQuoteActionID >= QUOTE_ACTION_ID_TRAVERSE_EAST && pSoldier->ubQuoteActionID <= QUOTE_ACTION_ID_TRAVERSE_NORTH)
				{
					HandleAITacticalTraversal( pSoldier );
					return;
				}

				// reached destination
#ifdef TESTAI
				DebugMsg( TOPIC_JA2AI, DBG_LEVEL_0, String("OPPONENT %d REACHES DEST - ACTION DONE",pSoldier->ubID ) );
#endif

				if ( pSoldier->sGridNo == pSoldier->pathing.sFinalDestination )
				{
					if ( pSoldier->aiData.bAction == AI_ACTION_MOVE_TO_CLIMB )
					{
						// successfully moved to roof!

						// fake setting action to climb roof and see if we can afford this
						pSoldier->aiData.bAction = AI_ACTION_CLIMB_ROOF;
						if (IsActionAffordable(pSoldier))
						{
							// set action to none and next action to climb roof so we do that next
							pSoldier->aiData.bAction = AI_ACTION_NONE;
							pSoldier->aiData.bNextAction = AI_ACTION_CLIMB_ROOF;
						}

					}
				}

				ActionDone(pSoldier);
			}

			//*** TRICK- TAKE INTO ACCOUNT PAUSED FOR NO TIME ( FOR NOW )
			if (pSoldier->flags.fNoAPToFinishMove )
			{
				SoldierTriesToContinueAlongPath(pSoldier);
			}
			// ATE: Let's also test if we are in any stationary animation...
			else if ( ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_STATIONARY ) )
			{
				// ATE: Put some ( MORE ) refinements on here....
				// If we are trying to open door, or jump fence  don't continue until done...
				if ( !pSoldier->flags.fContinueMoveAfterStanceChange && !pSoldier->bEndDoorOpenCode )
				{
					//ATE: just a few more.....
					// If we have ANY pending aninmation that is movement.....
					if ( pSoldier->usPendingAnimation != NO_PENDING_ANIMATION && ( gAnimControl[ pSoldier->usPendingAnimation ].uiFlags & ANIM_MOVING ) )
					{
						// Don't do anything, we're waiting on a pending animation....
					}
					else
					{
						// OK, we have a move to finish...
#ifdef TESTAI
						DebugMsg( TOPIC_JA2AI, DBG_LEVEL_0, String("GONNA TRY TO CONTINUE PATH FOR %d", pSoldier->ubID ) );
#endif

						SoldierTriesToContinueAlongPath(pSoldier);
					}
				}
			}
		}

	}

	/*********
	End of new overall AI system
	********/

}

#define NOSCORE 99

void EndAIGuysTurn( SOLDIERTYPE *pSoldier )
{
	UINT8					ubID;

	if (gfTurnBasedAI)
	{
		if (gTacticalStatus.uiFlags & PLAYER_TEAM_DEAD)
		{
			EndAITurn();
			return;
		}

		// search for any player merc to say close call quote
		for ( ubID = gTacticalStatus.Team[ gbPlayerNum ].bFirstID; ubID <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; ubID++ )
		{
			if ( OK_INSECTOR_MERC( MercPtrs[ ubID ] ) )
			{
				if ( MercPtrs[ ubID ]->flags.fCloseCall )
				{
					if ( !gTacticalStatus.fSomeoneHit && MercPtrs[ ubID ]->bNumHitsThisTurn == 0 && !(MercPtrs[ ubID ]->usQuoteSaidExtFlags & SOLDIER_QUOTE_SAID_EXT_CLOSE_CALL) && Random( 3 ) == 0 )
					{
						// say close call quote!
						TacticalCharacterDialogue( MercPtrs[ ubID ], QUOTE_CLOSE_CALL );
						MercPtrs[ ubID ]->usQuoteSaidExtFlags |= SOLDIER_QUOTE_SAID_EXT_CLOSE_CALL;
					}
					MercPtrs[ ubID ]->flags.fCloseCall = FALSE;
				}
			}
		}
		gTacticalStatus.fSomeoneHit = FALSE;

		// if civ in civ group and hostile, try to change nearby guys to hostile
		if ( pSoldier->ubCivilianGroup != NON_CIV_GROUP && !pSoldier->aiData.bNeutral )
		{

			if ( !(pSoldier->flags.uiStatusFlags & SOLDIER_BOXER)	|| !( gTacticalStatus.bBoxingState == PRE_BOXING || gTacticalStatus.bBoxingState == BOXING ) )
			{
				UINT8 ubFirstProfile;

				ubFirstProfile = CivilianGroupMembersChangeSidesWithinProximity( pSoldier );
				if ( ubFirstProfile != NO_PROFILE )
				{
					TriggerFriendWithHostileQuote( ubFirstProfile );
				}
			}
		}

		if ( gTacticalStatus.uiFlags & SHOW_ALL_ROOFS && ( gTacticalStatus.uiFlags & INCOMBAT ) )
		{
			SetRenderFlags( RENDER_FLAG_FULL );
			gTacticalStatus.uiFlags &= (~SHOW_ALL_ROOFS );
			InvalidateWorldRedundency( );
		}

		// End this NPC's control, move to next dude
		EndRadioLocator( pSoldier->ubID );
		pSoldier->flags.uiStatusFlags &= ( ~SOLDIER_UNDERAICONTROL );
		pSoldier->flags.fTurnInProgress = FALSE;
		pSoldier->aiData.bMoved = TRUE;
		pSoldier->aiData.bBypassToGreen = FALSE;

#ifdef TESTAICONTROL
		if (!(gTacticalStatus.uiFlags & DEMOMODE))
			DebugAI( String("Ending control for %d", pSoldier->ubID ) );
#endif

		// find the next AI guy
		ubID = RemoveFirstAIListEntry();
		if (ubID != NOBODY)
		{
			StartNPCAI( MercPtrs[ ubID ] );
			return;
		}

		// We are at the end, return control to next team
		DebugAI( String("Ending AI turn\n" ) );
		EndAITurn();

	}
	else
	{
		// realtime
	}
}



void EndAIDeadlock(void)
{
	INT32 cnt;
	SOLDIERTYPE *pSoldier;
	INT8 bFound=FALSE;

	// ESCAPE ENEMY'S TURN

	// find enemy with problem and free him up...
	for (cnt=0,pSoldier=Menptr; cnt < MAXMERCS; cnt++,pSoldier++)
	{
		if ( pSoldier->bActive && pSoldier->bInSector )
		{
			if (pSoldier->flags.uiStatusFlags & SOLDIER_UNDERAICONTROL)
			{
				CancelAIAction(pSoldier,FORCE);
#ifdef TESTAICONTROL
				if (gfTurnBasedAI)
				{
					DebugAI( String("Ending turn for %d because breaking deadlock", pSoldier->ubID ) );
				}
#endif

				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Number of bullets in the air is %ld", guiNumBullets ) );

				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Setting attack busy count to 0 from deadlock break" ) );
				gTacticalStatus.ubAttackBusyCount = 0;

				EndAIGuysTurn(pSoldier);
				bFound = TRUE;
				break;
			}
		}
	}


	if (!bFound)
	{
		StartPlayerTeamTurn( TRUE, FALSE );
	}
}


void StartNPCAI(SOLDIERTYPE *pSoldier)
{

	BOOLEAN fInValidSoldier = FALSE;

	// Only the host should do this
#ifdef NETWORKED
	if(!gfAmIHost)
		return;
#endif
	////pSoldier->flags.uiStatusFlags |= SOLDIER_UNDERAICONTROL;
	//if (!(pSoldier->flags.uiStatusFlags & SOLDIER_PC))

	//{
	//SetSoldierAsUnderAiControl( pSoldier );
	//}

	if (!is_networked || is_server)
		pSoldier->SetSoldierAsUnderAiControl( );


	pSoldier->flags.fTurnInProgress = TRUE;

	pSoldier->sLastTwoLocations[0] = NOWHERE;
	pSoldier->sLastTwoLocations[1] = NOWHERE;

	RefreshAI(pSoldier);

#ifdef TESTAICONTROL
	if (!(gTacticalStatus.uiFlags & DEMOMODE))
		DebugAI( String("Giving control to %d", pSoldier->ubID ) );
#endif

	gTacticalStatus.uiTimeSinceMercAIStart = GetJA2Clock();

	DebugMsg( TOPIC_JA2AI, DBG_LEVEL_3 , "Clock set" );

	// important: if "fPausedAnimation" is TRUE, then we have to turn it off else
	// HandleSoldierAI() will not be called!

	if( pSoldier->flags.uiStatusFlags & SOLDIER_VEHICLE )
	{
		if ( GetNumberInVehicle(  pSoldier->bVehicleID ) == 0 )
		{
			fInValidSoldier = TRUE;
		}
	}

	// Locate to soldier
	// If we are not in an interrupt situation!
	if ( (( gTacticalStatus.uiFlags & TURNBASED ) && ( gTacticalStatus.uiFlags & INCOMBAT )) && gubOutOfTurnPersons == 0 )
	{
		if( ( ( pSoldier->bVisible != -1 && pSoldier->stats.bLife) || ( gTacticalStatus.uiFlags & SHOW_ALL_MERCS ) ) && ( fInValidSoldier == FALSE ) )
		{
			// If we are on a roof, set flag for rendering...
			if ( pSoldier->pathing.bLevel != 0 && ( gTacticalStatus.uiFlags & INCOMBAT ) )
			{
				gTacticalStatus.uiFlags |= SHOW_ALL_ROOFS;
				SetRenderFlags( RENDER_FLAG_FULL );
				InvalidateWorldRedundency( );
			}


			//ATE: Changed to show locator

			// Skip locator for green friendly militia
			if ( !(pSoldier->bTeam == MILITIA_TEAM && pSoldier->bSide == 0 && pSoldier->aiData.bAlertStatus == STATUS_GREEN) )
			{
				LocateSoldier( pSoldier->ubID, SETLOCATORFAST );
			}

			// try commenting this out altogether
			/*
			// so long as he's not a neutral civ or a militia friendly to the player
			if ( !(pSoldier->aiData.bNeutral || (pSoldier->bTeam == MILITIA_TEAM && pSoldier->bSide == 0) ) )
			{
			PauseAITemporarily();
			}
			*/
		}

		UpdateEnemyUIBar( );

	}

	// Remove deadlock message
	DebugMsg( TOPIC_JA2AI, DBG_LEVEL_3 , "About to remove deadlock message" );
	EndDeadlockMsg( );
	DebugMsg( TOPIC_JA2AI, DBG_LEVEL_3 , "About to decide alert status" );
	DecideAlertStatus( pSoldier );
	DebugMsg( TOPIC_JA2AI, DBG_LEVEL_3 , "Alert status decided - startnpcai done" );
}



BOOLEAN DestNotSpokenFor(SOLDIERTYPE *pSoldier, INT32 sGridNo)
{
	INT32 cnt;
	SOLDIERTYPE *pOurTeam;

	cnt = gTacticalStatus.Team[pSoldier->bTeam].bFirstID;

	// make a list of all of our team's mercs
	for (pOurTeam = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; cnt++,pOurTeam++)
	{
		if ( pOurTeam->bActive )
		{
			if (pOurTeam->sGridNo == sGridNo || pOurTeam->aiData.usActionData == sGridNo)
				return(FALSE);
		}
	}

	return(TRUE);	// dest is free to go to...
}


INT32 FindAdjacentSpotBeside(SOLDIERTYPE *pSoldier, INT32 sGridNo)
{
	INT32 cnt;
	INT16 mods[4] = {-1,-WORLD_COLS,1,WORLD_COLS};
	INT32 sTempGridNo = NOWHERE, sCheapestDest = MAX_MAP_POS;
	INT16 sCheapestCost=500, sMovementCost;

	for (cnt=0; cnt < 4; cnt++)
	{
		sTempGridNo = sGridNo + mods[cnt];
		if (!OutOfBounds(sGridNo,sTempGridNo))
		{
			if (NewOKDestination(pSoldier,sTempGridNo,PEOPLETOO, pSoldier->pathing.bLevel ) && DestNotSpokenFor(pSoldier,sTempGridNo))
			{
				sMovementCost = PlotPath(pSoldier,sTempGridNo,FALSE,FALSE,FALSE,WALKING,FALSE,FALSE,0);
				if (sMovementCost < sCheapestCost)
				{
					sCheapestCost	= sMovementCost;
					sCheapestDest = sTempGridNo;
				}

			}
		}

	}

	return(sCheapestDest);
}

UINT8 GetMostThreateningOpponent( SOLDIERTYPE *pSoldier )
{
	UINT32				uiLoop;
	INT32				iThreatVal,iMinThreat = 30000;
	SOLDIERTYPE			*pTargetSoldier;
	UINT8					ubTargetSoldier = NOBODY;

	// Loop through all mercs

	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pTargetSoldier = MercSlots[ uiLoop ];

		if (!pTargetSoldier)
		{
			continue;
		}

		// if this soldier is on same team as me, skip him
		if (pTargetSoldier->bTeam == pSoldier->bTeam || pTargetSoldier->bSide == pSoldier->bSide)
		{
			continue;
		}

		// if potential opponent is dead, skip him
		if (!pTargetSoldier->stats.bLife)
		{
			continue;
		}

		if (pSoldier->aiData.bOppList[pTargetSoldier->ubID] != SEEN_CURRENTLY)
			continue;

		// Special stuff for Carmen the bounty hunter
		if (pSoldier->aiData.bAttitude == ATTACKSLAYONLY && pTargetSoldier->ubProfile != 64)
		{
			continue;  // next opponent
		}

		iThreatVal = CalcManThreatValue(pTargetSoldier, pSoldier->sGridNo, TRUE, pSoldier);
		if (iThreatVal < iMinThreat)
		{
			iMinThreat			= iThreatVal;
			ubTargetSoldier	= pTargetSoldier->ubID;
		}

	}

	return( ubTargetSoldier );
}



void FreeUpNPCFromPendingAction( 	SOLDIERTYPE *pSoldier )
{
	if ( pSoldier )
	{
		if ( pSoldier->aiData.bAction == AI_ACTION_PENDING_ACTION
			|| pSoldier->aiData.bAction == AI_ACTION_OPEN_OR_CLOSE_DOOR
			|| pSoldier->aiData.bAction == AI_ACTION_CREATURE_CALL
			|| pSoldier->aiData.bAction == AI_ACTION_YELLOW_ALERT
			|| pSoldier->aiData.bAction == AI_ACTION_RED_ALERT
			|| pSoldier->aiData.bAction == AI_ACTION_UNLOCK_DOOR
			|| pSoldier->aiData.bAction == AI_ACTION_PULL_TRIGGER
			|| pSoldier->aiData.bAction == AI_ACTION_LOCK_DOOR	)
		{
			if ( pSoldier->ubProfile != NO_PROFILE )
			{
				if ( pSoldier->ubQuoteRecord == NPC_ACTION_KYLE_GETS_MONEY )
				{
					// Kyle after getting money
					pSoldier->ubQuoteRecord = 0;
					TriggerNPCRecord( KYLE, 11 );
				}
				else if (pSoldier->usAnimState == END_OPENSTRUCT)
				{
					TriggerNPCWithGivenApproach( pSoldier->ubProfile, APPROACH_DONE_OPEN_STRUCTURE, TRUE );
					//TriggerNPCWithGivenApproach( pSoldier->ubProfile, APPROACH_DONE_OPEN_STRUCTURE, FALSE );
				}
				else if (pSoldier->usAnimState == PICKUP_ITEM || pSoldier->usAnimState == ADJACENT_GET_ITEM )
				{
					TriggerNPCWithGivenApproach( pSoldier->ubProfile, APPROACH_DONE_GET_ITEM, TRUE );
				}
			}
			ActionDone(pSoldier);
		}
	}
}

void FreeUpNPCFromAttacking(UINT8 ubID)
{
	SOLDIERTYPE *pSoldier;

	pSoldier = MercPtrs[ubID];
	ActionDone(pSoldier);
	pSoldier->pathing.bNeedToLook = TRUE;

	/*
	if (pSoldier->aiData.bActionInProgress)
	{
	#ifdef TESTAI
	DebugMsg( TOPIC_JA2AI, DBG_LEVEL_0, String( "FreeUpNPCFromAttacking for %d", pSoldier->ubID ) );
	#endif
	if (pSoldier->aiData.bAction == AI_ACTION_FIRE_GUN)
	{
	if (pSoldier->bDoBurst)
	{
	if (pSoldier->bBulletsLeft == 0)
	{
	// now find the target and have them say "close call" quote if
	// applicable
	pTarget = SimpleFindSoldier( pSoldier->sTargetGridNo, pSoldier->bTargetLevel );
	if (pTarget && pTarget->bTeam == OUR_TEAM && pTarget->flags.fCloseCall && pTarget->bShock == 0)
	{
	// say close call quote!
	TacticalCharacterDialogue( pTarget, QUOTE_CLOSE_CALL );
	pTarget->flags.fCloseCall = FALSE;
	}
	ActionDone(pSoldier);
	pSoldier->bDoBurst = FALSE;
	}
	}
	else
	{
	pTarget = SimpleFindSoldier( pSoldier->sTargetGridNo, pSoldier->bTargetLevel );
	if (pTarget && pTarget->bTeam == OUR_TEAM && pTarget->flags.fCloseCall && pTarget->bShock == 0)
	{
	// say close call quote!
	TacticalCharacterDialogue( pTarget, QUOTE_CLOSE_CALL );
	pTarget->flags.fCloseCall = FALSE;
	}
	ActionDone(pSoldier);
	}
	}
	else if ((pSoldier->aiData.bAction == AI_ACTION_TOSS_PROJECTILE) || (pSoldier->aiData.bAction == AI_ACTION_KNIFE_STAB))
	{
	ActionDone(pSoldier);
	}
	}

	// DO WE NEED THIS???
	//pSoldier->sTarget = NOWHERE;

	// make him look in case he turns to face a new direction
	pSoldier->pathing.bNeedToLook = TRUE;

	// This is here to speed up resolution of interrupts that have already been
	// delayed while AttackingPerson was still set (causing ChangeControl to
	// bail).  Without it, an interrupt would have to wait until next ani frame!
	//if (SwitchTo > -1)
	//  ChangeControl();
	*/
}

void FreeUpNPCFromLoweringGun( SOLDIERTYPE *pSoldier )
{
	if ( pSoldier && pSoldier->aiData.bAction == AI_ACTION_LOWER_GUN )
	{
		ActionDone(pSoldier);
	}
}

void FreeUpNPCFromTurning(SOLDIERTYPE *pSoldier, INT8 bLook)
{

	// if NPC is in the process of changing facing, mark him as being done!
	if ((pSoldier->aiData.bAction == AI_ACTION_CHANGE_FACING) && pSoldier->aiData.bActionInProgress)
	{
#ifdef TESTAI
		DebugMsg( TOPIC_JA2AI, DBG_LEVEL_3,
			String("FREEUPNPCFROMTURNING: our action %d, desdir %d dir %d",pSoldier->aiData.bAction,pSoldier->pathing.bDesiredDirection,pSoldier->ubDirection) );
#endif


		ActionDone(pSoldier);

		if ( bLook )
		{
			//HandleSight(pSoldier,SIGHT_LOOK | SIGHT_RADIO); // no interrupt possible
		}

	}
}


void FreeUpNPCFromStanceChange(SOLDIERTYPE *pSoldier )
{
	// are we/were we doing something?
	if (pSoldier->aiData.bActionInProgress)
	{
		// check and see if we were changing stance
		if (pSoldier->aiData.bAction == AI_ACTION_CHANGE_STANCE || pSoldier->aiData.bAction == AI_ACTION_COWER || pSoldier->aiData.bAction == AI_ACTION_STOP_COWERING)
		{
			// yes we were - are we finished?
			if ( gAnimControl[ pSoldier->usAnimState ].ubHeight == pSoldier->aiData.usActionData )
			{
				// yes! Free us up to do other fun things
				ActionDone(pSoldier);
			}
		}
	}
}

void FreeUpNPCFromRoofClimb(SOLDIERTYPE *pSoldier )
{
	// are we/were we doing something?
	if (pSoldier->aiData.bActionInProgress)
	{
		// check and see if we were climbing
		if (pSoldier->aiData.bAction == AI_ACTION_CLIMB_ROOF)
		{
			// yes! Free us up to do other fun things
			ActionDone(pSoldier);
		}
	}
}




void ActionDone(SOLDIERTYPE *pSoldier)
{
	// if an action is currently selected
	if (pSoldier->aiData.bAction != AI_ACTION_NONE)
	{
		if (pSoldier->flags.uiStatusFlags & SOLDIER_MONSTER)
		{
#ifdef TESTAI
			DebugMsg( TOPIC_JA2AI, DBG_LEVEL_3,
				String("Cancelling actiondone: our action %d, desdir %d dir %d",pSoldier->aiData.bAction,pSoldier->pathing.bDesiredDirection,pSoldier->ubDirection) );
#endif
		}

		// If doing an attack, reset attack busy count and # of bullets
		//if ( gTacticalStatus.ubAttackBusyCount )
		//{
		//	gTacticalStatus.ubAttackBusyCount = 0;
		//	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Setting attack busy count to 0 due to Action Done" ) );
		//	pSoldier->bBulletsLeft = 0;
		//}

		// cancel any turning & movement by making current settings desired ones
		pSoldier->pathing.sFinalDestination	= pSoldier->sGridNo;

		if ( !pSoldier->flags.fNoAPToFinishMove )
		{
			pSoldier->EVENT_StopMerc( pSoldier->sGridNo, pSoldier->ubDirection );
			pSoldier->AdjustNoAPToFinishMove( FALSE );
		}

		//Lalien: moved later in ExecuteAction() case AI_ACTION_RAISE_GUN:
		//AXP 23.03.2007: Sniper deadlock fix
		//if ( pSoldier->aiData.bOrders == SNIPER && pSoldier->aiData.bAction == AI_ACTION_RAISE_GUN && pSoldier->aiData.bLastAction == AI_ACTION_RAISE_GUN)
		//{
		//	pSoldier->aiData.bNextAction = AI_ACTION_END_TURN;
		//}

		// cancel current action
		pSoldier->aiData.bLastAction		= pSoldier->aiData.bAction;
		pSoldier->aiData.bAction			= AI_ACTION_NONE;
		pSoldier->aiData.usActionData		= NOWHERE;
		pSoldier->aiData.bActionInProgress	= FALSE;
		pSoldier->flags.fDelayedMovement	= FALSE;

		/*
		if ( pSoldier->aiData.bLastAction == AI_ACTION_CHANGE_STANCE || pSoldier->aiData.bLastAction == AI_ACTION_COWER || pSoldier->aiData.bLastAction == AI_ACTION_STOP_COWERING )
		{
		pSoldier->SoldierGotoStationaryStance( );
		}
		*/

		if ( pSoldier->aiData.bLastAction == AI_ACTION_RAISE_GUN )
		{
			HandleSight( pSoldier, SIGHT_LOOK );
		}

		// make sure pathStored is not left TRUE by accident.
		// This is possible if we decide on an action that we have no points for
		// (but which set pathStored).  The action is retained until next turn,
		// although NewDest isn't called.  A newSit. could cancel it before then!
		pSoldier->pathing.bPathStored = FALSE;
	}

	if (pSoldier->flags.uiStatusFlags & SOLDIER_DEAD)
	{
		// The last action killed the soldier (stepped on a mine, detonated a LAW too close, etc)
		EndAIGuysTurn( pSoldier);
	}
}


// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////

//	O L D    D G    A I    C O D E

// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////


// GLOBALS:

UINT8 SkipCoverCheck = FALSE;
THREATTYPE Threat[MAXMERCS];


// threat percentage is based on the certainty of opponent knowledge:
// opplist value:        -4  -3  -2  -1 SEEN  1    2   3   4   5
int ThreatPercent[10] = { 20, 40, 60, 80, 25, 100, 90, 75, 60, 45 };



void NPCDoesAct(SOLDIERTYPE *pSoldier)
{
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"NPCDoesAct");

	// if the action is visible and we're in a hidden turnbased mode, go to turnbased
	if (gTacticalStatus.uiFlags & TURNBASED && !(gTacticalStatus.uiFlags & INCOMBAT) && (pSoldier->aiData.bAction == AI_ACTION_FIRE_GUN || pSoldier->aiData.bAction == AI_ACTION_TOSS_PROJECTILE || pSoldier->aiData.bAction == AI_ACTION_KNIFE_MOVE || pSoldier->aiData.bAction == AI_ACTION_KNIFE_STAB || pSoldier->aiData.bAction == AI_ACTION_THROW_KNIFE) )
	{
		DisplayHiddenTurnbased( pSoldier );
	}

	if (gfHiddenInterrupt)
	{
		DisplayHiddenInterrupt( pSoldier );
	}
	//StartInterruptVisually(pSoldier->ubID);
	// *** IAN deleted lots of interrupt related code here to simplify JA2	development

	// CJC Feb 18 99: make sure that soldier is not in the middle of a turn due to visual crap to make enemies
	// face and point their guns at us
	if ( pSoldier->pathing.bDesiredDirection != pSoldier->ubDirection )
	{
		pSoldier->pathing.bDesiredDirection = pSoldier->ubDirection;
	}

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"NPCDoesAct done");
}



void NPCDoesNothing(SOLDIERTYPE *pSoldier)
{
	// NPC, for whatever reason, did/could not start an action, so end his turn
	//pSoldier->moved = TRUE;

#ifdef TESTAICONTROL
	if (gfTurnBasedAI)
	{
		DebugAI( String("Ending turn for %d because doing no-action", pSoldier->ubID ) );
	}
#endif

	EndAIGuysTurn(pSoldier);

	// *** IAN deleted lots of interrupt related code here to simplify JA2	development
}




void CancelAIAction(SOLDIERTYPE *pSoldier, UINT8 ubForce)
{
#ifdef DEBUGDECISIONS
	if (SkipCoverCheck)
	{
		STR tempstr;
		sprintf( tempstr, "CancelAIAction: SkipCoverCheck turned OFF\n",0 );
		DebugAI (tempstr);
	}
#endif

	// re-enable cover checking, something is new or something strange happened
	SkipCoverCheck = FALSE;

	// turn off new situation flag to stop this from repeating all the time!
	if ( pSoldier->aiData.bNewSituation == IS_NEW_SITUATION )
	{
		pSoldier->aiData.bNewSituation = WAS_NEW_SITUATION;
	}

	// NPCs getting escorted do NOT react to new situations, unless forced!
	if (pSoldier->aiData.bUnderEscort && !ubForce)
		return;

	// turn off RED/YELLOW status "bypass to Green", to re-check all actions
	pSoldier->aiData.bBypassToGreen = FALSE;

	ActionDone(pSoldier);
}




/*
void ActionTimeoutExceeded(SOLDIERTYPE *pSoldier, UCHAR alreadyFreedUp)
{
int cnt;
UCHAR attackAction = FALSE;


#ifdef BETAVERSION
if (ConvertedMultiSave)
{
// re-start real-time NPC action timer
EnemyTimedOut = FALSE;
EnemyTimerCnt = ENEMYWAITTOLERANCE;
return;
}
#endif


// check if it's a problem with a offensive combat action
if ((pSoldier->aiData.bAction == AI_ACTION_FIRE_GUN) ||
(pSoldier->aiData.bAction == AI_ACTION_TOSS_PROJECTILE) ||
(pSoldier->aiData.bAction == AI_ACTION_KNIFE_STAB))
{
// THESE ARE LESS SERIOUS, SINCE THEY LIKELY WON'T REPEAT THEMSELVES
attackAction = TRUE;
}
// OTHERS ARE VERY SERIOUS, SINCE THEY ARE LIKELY TO REPEAT THEMSELVES


#ifdef BETAVERSION
sprintf(tempstr,"ActionInProgress - ERROR: %s's timeout limit exceeded.  Action #%d (%d)",
pSoldier->name,pSoldier->aiData.bAction,pSoldier->aiData.usActionData);

#ifdef RECORDNET
fprintf(NetDebugFile,"\n%s\n\n",tempstr);
#endif

PopMessage(tempstr);
SaveGame(ERROR_SAVE);
#endif

#ifdef TESTVERSION
PopMessage("FULL SOLDIER INFORMATION DUMP COMING UP, BRACE THYSELF!");
DumpSoldierInfo(pSoldier);
#endif


// re-start real-time NPC action timer
EnemyTimedOut = FALSE;
EnemyTimerCnt = ENEMYWAITTOLERANCE;

if (attackAction)
{
#ifdef BETAVERSION
NameMessage(pSoldier,"will now be freed up from attacking...",2000);
#endif


// free up ONLY players from whom we haven't received an AI_ACTION_DONE yet
// we can all agree the action is DONE and we can continue...
// (otherwise they'll be calling FreeUp... twice and get REAL screwed up)
NetSend.msgType = NET_FREE_UP_ATTACK;
NetSend.ubID  = pSoldier->ubID;

for (cnt = 0; cnt < MAXPLAYERS; cnt++)
{
if ((cnt != Net.pnum) && Net.player[cnt].playerActive &&
(Net.player[cnt].actionDone != pSoldier->ubID))
SendNetData(cnt);
}

if (!alreadyFreedUp)
FreeUpManFromAttacking(pSoldier->ubID,COMMUNICATE);
}
else if (pSoldier->aiData.bAction == AI_ACTION_CHANGE_FACING)
{
#ifdef BETAVERSION
NameMessage(pSoldier,"will now be freed up from turning...",2000);
#endif

// force him to face in the right direction (as long as it's legal)
if ((pSoldier->pathing.bDesiredDirection >= 1) && (pSoldier->pathing.bDesiredDirection <= 8))
pSoldier->ubDirection = pSoldier->pathing.bDesiredDirection;
else
pSoldier->pathing.bDesiredDirection = pSoldier->ubDirection;

// free up ONLY players from whom we haven't received an AI_ACTION_DONE yet
// we can all agree the action is DONE and we can continue...
// (otherwise they'll be calling FreeUp... twice and get REAL screwed up)
NetSend.msgType    = NET_FREE_UP_TURN;
NetSend.ubID     = pSoldier->ubID;
NetSend.misc_UCHAR = pSoldier->ubDirection;
NetSend.answer     = pSoldier->pathing.bDesiredDirection;

for (cnt = 0; cnt < MAXPLAYERS; cnt++)
{
if ((cnt != Net.pnum) && Net.player[cnt].playerActive &&
(Net.player[cnt].actionDone != pSoldier->ubID))
SendNetData(cnt);
}

if (!alreadyFreedUp)
// this calls FreeUpManFromTurning()
NowFacingRightWay(pSoldier,COMMUNICATE);
}
else
{
#ifdef BETAVERSION
NameMessage(pSoldier,"is having the remainder of his turn canceled...",1000);
#endif

// cancel the remainder of the offender's turn as a penalty!
pSoldier->bActionPoints = 0;
NPCDoesNothing(pSoldier);
}


// cancel whatever the current action is, force this even for escorted NPCs
CancelAIAction(pSoldier,FORCE);


// reset the timeout counter for next time
pSoldier->aiData.bActionTimeout = 0;
}
*/




INT16 ActionInProgress(SOLDIERTYPE *pSoldier)
{
	// if NPC has a desired destination, but isn't currently going there	
	if ((!TileIsOutOfBounds(pSoldier->pathing.sFinalDestination)) && (pSoldier->pathing.sDestination != pSoldier->pathing.sFinalDestination))
	{
		// return success (TRUE) if we successfully resume the movement
		return(TryToResumeMovement(pSoldier,pSoldier->pathing.sFinalDestination));
	}


	// this here should never happen, but it seems to (turns sometimes hang!)
	if ((pSoldier->aiData.bAction == AI_ACTION_CHANGE_FACING) && (pSoldier->pathing.bDesiredDirection != pSoldier->aiData.usActionData))
	{
#ifdef TESTVERSION
		PopMessage("ActionInProgress: WARNING - CONTINUING FACING CHANGE...");
#endif

		// don't try to pay any more APs for this, it was paid for once already!
		pSoldier->pathing.bDesiredDirection = (INT8) pSoldier->aiData.usActionData;   // turn to face direction in actionData
		return(TRUE);
	}


	// needs more time to complete action
	return(TRUE);
}





/*
void RestoreMarkedMines()
{
int gridno;

// all tiles marked with the special NPC mine cost value must be restored
for (gridno = 0; gridno < GRIDSIZE; gridno++)
{
if (GridCost[gridno] == NPCMINECOST)
{
GridCost[gridno] = BackupGridCost[gridno];

#ifdef TESTMINEMARKING
fprintf(NetDebugFile,"\tRestoring marked mine at gridno %d back to gridCost %d\n",gridno,BackupGridCost[gridno]);
#endif
}
}

MarkedNPCMines = FALSE;
}



void MarkDetectableMines(SOLDIERTYPE *pSoldier)
{
int gridno,detectLevel;
GRIDINFO *gpSoldier;


// this should happen, means we missed a clean-up cycle last time!
if (MarkedNPCMines)
{
#ifdef BETAVERSION
sprintf(tempstr,"MarkDetectableMines: ERROR - mines still marked!  Guynum %d",pSoldier->ubID);

#ifdef RECORDNET
fprintf(NetDebugFile,"\n\t%s\n\n",tempstr);
#endif

PopMessage(tempstr);
#endif

RestoreMarkedMines();
}


// make a backup of the current gridcosts
memcpy(BackupGridCost,GridCost,sizeof(GridCost));

// calculate what "level" of mines we are able to detect
detectLevel = CalcMineDetectLevel(pSoldier);


// check every tile, looking for BURIED mines only
for (gridno = 0,gpSoldier = &Grid[0]; gridno < GRIDSIZE; gridno++,gpSoldier++)
{
// if there's a valid object there, and it is still "buried"
if ((gpSoldier->object < 255) &&
(ObjList[gpSoldier->object].visible == BURIED) &&
(ObjList[gpSoldier->object].item == MINE))
{
// are we bright enough to detect it (should we get there) ?
if (detectLevel >= ObjList[gpSoldier->object].trap)
{
// bingo!  Mark it as "unpassable" for the purposes of the path AI
GridCost[gridno] = NPCMINECOST;
MarkedNPCMines = TRUE;

#ifdef TESTMINEMARKING
fprintf(NetDebugFile,"\tNPC %d, dtctLvl %d, marking mine at gridno %d, gridCost was %d\n",pSoldier->ubID,detectLevel,gridno,BackupGridCost[gridno]);
#endif
}
}
}
}

*/




void TurnBasedHandleNPCAI(SOLDIERTYPE *pSoldier)
{


	/*
	if (Status.gamePaused)
	{
	#ifdef DEBUGBUSY
	DebugAI("HandleManAI - Skipping %d, the game is paused\n",pSoldier->ubID);
	#endif

	return;
	}
	//

	// If man is inactive/at base/dead/unconscious
	if (!pSoldier->bActive || !pSoldier->bInSector || (pSoldier->stats.bLife < OKLIFE))
	{
	#ifdef DEBUGDECISIONS
	AINumMessage("HandleManAI - Unavailable man, skipping guy#",pSoldier->ubID);
	#endif

	NPCDoesNothing(pSoldier);
	return;
	}

	if (PTR_CIVILIAN && pSoldier->service &&
	(pSoldier->aiData.bNeutral || MedicsMissionIsEscort(pSoldier)))
	{
	#ifdef DEBUGDECISIONS
	AINumMessage("HandleManAI - Civilian is being serviced, skipping guy#",pSoldier->ubID);
	#endif

	NPCDoesNothing(pSoldier);
	return;
	}
	*/



	/*
	anim = pSoldier->anitype[pSoldier->anim];

	// If man is down on the ground
	if (anim < BREATHING)
	{
	// if he lacks the breath, or APs to get up this turn (life checked above)
	// OR... (new June 13/96 Ian) he's getting first aid...
	if ((pSoldier->bBreath < OKBREATH) || (pSoldier->bActionPoints < (APBPConstants[AP_GET_UP] + APBPConstants[AP_ROLL_OVER]))
	|| pSoldier->service)
	{
	#ifdef RECORDNET
	fprintf(NetDebugFile,"\tAI: %d can't get up (breath %d, AP %d), ending his turn\n",
	pSoldier->ubID,pSoldier->bBreath,pSoldier->bActionPoints);
	#endif
	#ifdef DEBUGDECISIONS
	AINumMessage("HandleManAI - CAN'T GET UP, skipping guy #",pSoldier->ubID);
	#endif

	NPCDoesNothing(pSoldier);
	return;
	}
	else
	{
	// wait until he gets up first, only then worry about deciding his AI

	#ifdef RECORDNET
	fprintf(NetDebugFile,"\tAI: waiting for %d to GET UP (breath %d, AP %d)\n",
	pSoldier->ubID,pSoldier->bBreath,pSoldier->bActionPoints);
	#endif

	#ifdef DEBUGBUSY
	AINumMessage("HandleManAI - About to get up, skipping guy#",pSoldier->ubID);
	#endif

	return;
	}
	}


	// if NPC's has been forced to stop by an opponent's interrupt or similar
	if (pSoldier->flags.forcedToStop)
	{
	#ifdef DEBUGBUSY
	AINumMessage("HandleManAI - Forced to stop, skipping guy #",pSoldier->ubID);
	#endif

	return;
	}

	// if we are still in the midst in an uninterruptable animation
	if (!AnimControl[anim].interruptable)
	{
	#ifdef DEBUGBUSY
	AINumMessage("HandleManAI - uninterruptable animation, skipping guy #",pSoldier->ubID);
	#endif

	return;      // wait a while, let the animation finish first
	}

	*/

	// yikes, this shouldn't occur! we should be trying to finish our move!
	// pSoldier->flags.fNoAPToFinishMove = FALSE;

	// unless in mid-move, get an up-to-date alert status for this guy
	if (pSoldier->pathing.bStopped)
	{
		// if active team is waiting for oppChanceToDecide, that means we have NOT
		// had a chance to go through NewSelectedNPC(), so do the refresh here
		/*
		???
		if (gTacticalStatus.team[Net.turnActive].allowOppChanceToDecide)
		{
		// if mines are still marked (this could happen if we also control the
		// active team that's potentially BEING interrupted), unmark them
		//RestoreMarkedMines();

		RefreshAI(pSoldier);
		}
		else
		{
		DecideAlertStatus(pSoldier);
		}
		*/
	}

	/*
	// move this clause outside of the function...
	if (pSoldier->aiData.bNewSituation)
	// don't force, don't want escorted mercs reacting to new opponents, etc.
	CancelAIAction(pSoldier,DONTFORCE);

	*/


	/*
	if (!pSoldier->stopped)
	{
	#ifdef DEBUGBUSY
	AINumMessage("HandleManAI - Moving, skipping guy#",pSoldier->ubID);
	#endif

	return;
	}
	*/



	if ((pSoldier->aiData.bAction != AI_ACTION_NONE) && pSoldier->aiData.bActionInProgress)
	{
		/*
		if (pSoldier->aiData.bAction == AI_ACTION_RANDOM_PATROL)
		{
		if (pSoldier->pathing.usPathIndex == pSoldier->pathing.usPathDataSize)
		//if (pSoldier->aiData.usActionData == pSoldier->sGridNo )
		//(IC?) if (pSoldier->aiData.bAction == AI_ACTION_RANDOM_PATROL && ( pSoldier->pathing.usPathIndex == pSoldier->pathing.usPathDataSize ) )
		//(old?) if (pSoldier->aiData.bAction == AI_ACTION_RANDOM_PATROL && ( pSoldier->aiData.usActionData == pSoldier->sGridNo ) )
		{
		#ifdef TESTAI
		DebugMsg( TOPIC_JA2AI, DBG_LEVEL_0, String("OPPONENT %d REACHES DEST - ACTION DONE",pSoldier->ubID ) );
		#endif
		ActionDone(pSoldier);
		}

		//*** TRICK- TAKE INTO ACCOUNT PAUSED FOR NO TIME ( FOR NOW )
		if (pSoldier->flags.fNoAPToFinishMove)
		//if (pSoldier->aiData.bAction == AI_ACTION_RANDOM_PATROL && pSoldier->flags.fNoAPToFinishMove)
		{
		// OK, we have a move to finish...

		#ifdef TESTAI
		DebugMsg( TOPIC_JA2AI, DBG_LEVEL_0, String("GONNA TRY TO CONTINUE PATH FOR %d", pSoldier->ubID ) );
		#endif

		SoldierTriesToContinueAlongPath(pSoldier);

		// since we just gave up on our action due to running out of points, better end our turn
		//EndAIGuysTurn(pSoldier);
		}
		}
		*/

		// if action should remain in progress
		if (ActionInProgress(pSoldier))
		{
#ifdef DEBUGBUSY
			AINumMessage("Busy with action, skipping guy#",pSoldier->ubID);
#endif

			// let it continue
			return;
		}
	}


#ifdef DEBUGDECISIONS
	STR tempstr;
	sprintf( tempstr, "HandleManAI - DECIDING for guynum %d(%s) at gridno %d, APs %d\n",
		pSoldier->ubID,pSoldier->name,pSoldier->sGridNo,pSoldier->bActionPoints );
	DebugAI ( tempstr );
#endif


	// if man has nothing to do
	if (pSoldier->aiData.bAction == AI_ACTION_NONE)
	{
		// make sure this flag is turned off (it already should be!)
		pSoldier->aiData.bActionInProgress = FALSE;

		// Since we're NEVER going to "continue" along an old path at this point,
		// then it would be nice place to reinitialize "pathStored" flag for
		// insurance purposes.
		//
		// The "pathStored" variable controls whether it's necessary to call
		// findNewPath() after you've called NewDest(). Since the AI calls
		// findNewPath() itself, a speed gain can be obtained by avoiding
		// redundancy.
		//
		// The "normal" way for pathStored to be reset is inside
		// SetNewCourse() [which gets called after NewDest()].
		//
		// The only reason we would NEED to reinitialize it here is if I've
		// incorrectly set pathStored to TRUE in a process that doesn't end up
		// calling NewDest()
		pSoldier->pathing.bPathStored = FALSE;

		// decide on the next action
		if (pSoldier->aiData.bNextAction != AI_ACTION_NONE)
		{
			// do the next thing we have to do...
			if ( pSoldier->aiData.bNextAction == AI_ACTION_END_COWER_AND_MOVE )
			{
				if ( pSoldier->flags.uiStatusFlags & SOLDIER_COWERING )
				{
					pSoldier->aiData.bAction = AI_ACTION_STOP_COWERING;
					pSoldier->aiData.usActionData = ANIM_STAND;
				}
				else if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight < ANIM_STAND )
				{
					// stand up!
					pSoldier->aiData.bAction = AI_ACTION_CHANGE_STANCE;
					pSoldier->aiData.usActionData = ANIM_STAND;
				}
				else
				{
					pSoldier->aiData.bAction = AI_ACTION_NONE;
				}
				if ( pSoldier->sGridNo == pSoldier->aiData.usNextActionData )
				{
					// no need to walk after this
					pSoldier->aiData.bNextAction = AI_ACTION_NONE;
					pSoldier->aiData.usNextActionData = NOWHERE;
				}
				else
				{
					pSoldier->aiData.bNextAction = AI_ACTION_WALK;
					// leave next-action-data as is since that's where we want to go
				}
			}
			else
			{
				pSoldier->aiData.bAction = pSoldier->aiData.bNextAction;
				pSoldier->aiData.usActionData = pSoldier->aiData.usNextActionData;
				pSoldier->bTargetLevel = pSoldier->aiData.bNextTargetLevel;
				pSoldier->aiData.bNextAction = AI_ACTION_NONE;
				pSoldier->aiData.usNextActionData = 0;
				pSoldier->aiData.bNextTargetLevel = 0;
			}
			if (pSoldier->aiData.bAction == AI_ACTION_PICKUP_ITEM)
			{
				// the item pool index was stored in the special data field
				pSoldier->aiData.uiPendingActionData1 = pSoldier->iNextActionSpecialData;
			}
		}		
		else if (!TileIsOutOfBounds(pSoldier->sAbsoluteFinalDestination))
		{
			if ( ACTING_ON_SCHEDULE( pSoldier ) )
			{
				pSoldier->aiData.bAction = AI_ACTION_SCHEDULE_MOVE;
			}
			else
			{
				pSoldier->aiData.bAction = AI_ACTION_WALK;
			}
			pSoldier->aiData.usActionData = pSoldier->sAbsoluteFinalDestination;
		}
		else
		{
			if (!(gTacticalStatus.uiFlags & ENGAGED_IN_CONV))
			{
				if (CREATURE_OR_BLOODCAT( pSoldier ))
				{
					pSoldier->aiData.bAction = CreatureDecideAction( pSoldier );
				}
				else if (pSoldier->ubBodyType == CROW)
				{
					pSoldier->aiData.bAction = CrowDecideAction( pSoldier );
				}
				else
				{
					pSoldier->aiData.bAction = DecideAction(pSoldier);
				}
			}
		}

		if (pSoldier->aiData.bAction == AI_ACTION_ABSOLUTELY_NONE)
		{
			pSoldier->aiData.bAction = AI_ACTION_NONE;
		}

		// if he chose to continue doing nothing
		if (pSoldier->aiData.bAction == AI_ACTION_NONE)
		{
#ifdef RECORDNET
			fprintf(NetDebugFile,"\tMOVED BECOMING TRUE: Chose to do nothing, guynum %d\n",pSoldier->ubID);
#endif

			DebugMsg (TOPIC_JA2AI,DBG_LEVEL_3,"NPC has no action assigned");
			NPCDoesNothing(pSoldier);  // sets pSoldier->moved to TRUE
			return;
		}



		/*
		// if we somehow just caused an uninterruptable animation to occur
		// This is mainly to finish a weapon_AWAY anim that preceeds a TOSS attack
		if (!AnimControl[ pSoldier->anitype[pSoldier->anim] ].interruptable)
		{
		#ifdef DEBUGBUSY
		DebugAI( String( "Uninterruptable animation %d, skipping guy %d",pSoldier->anitype[pSoldier->anim],pSoldier->ubID ) );
		#endif

		return;      // wait a while, let the animation finish first
		}
		*/

		// to get here, we MUST have an action selected, but not in progress...

		if ( CHEATER_CHEAT_LEVEL( ) )
		{
			//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Soldier %d decides action %d with data %d", pSoldier->ubID, pSoldier->aiData.bAction, pSoldier->aiData.usActionData );
		}

		// see if we can afford to do this action
		if (IsActionAffordable(pSoldier))
		{
			NPCDoesAct(pSoldier);

			// perform the chosen action
			pSoldier->aiData.bActionInProgress = ExecuteAction(pSoldier); // if started, mark us as busy
			
			if ( !pSoldier->aiData.bActionInProgress && !TileIsOutOfBounds(pSoldier->sAbsoluteFinalDestination))
			{
				// turn based... abort this guy's turn
				EndAIGuysTurn( pSoldier );
			}
		}
		else
		{
#ifdef DEBUGDECISIONS
			AINumMessage("HandleManAI - Not enough APs, skipping guy#",pSoldier->ubID);
#endif
			HaltMoveForSoldierOutOfPoints( pSoldier);
			return;
		}
	}
}


void RefreshAI(SOLDIERTYPE *pSoldier)
{
	// produce our own private "mine map" so we can avoid the ones we can detect
	// MarkDetectableMines(pSoldier);

	// whether last attack hit or not doesn't matter once control has been lost
	pSoldier->aiData.bLastAttackHit = FALSE;

	// get an up-to-date alert status for this guy
	DecideAlertStatus(pSoldier);

	if (pSoldier->aiData.bAlertStatus == STATUS_YELLOW)
		SkipCoverCheck = FALSE;

	// if he's in battle or knows opponents are here
	if (gfTurnBasedAI)
	{
		if ((pSoldier->aiData.bAlertStatus == STATUS_BLACK) || (pSoldier->aiData.bAlertStatus == STATUS_RED))
		{
			// always freshly rethink things at start of his turn
			pSoldier->aiData.bNewSituation = IS_NEW_SITUATION;
		}
		else
		{
			// make sure any paths stored during out last AI decision but not reacted
			// to (probably due to lack of APs) get re-tested by the ExecuteAction()
			// function in AI, since the->pathing.sDestination may no longer be legal now!
			pSoldier->pathing.bPathStored = FALSE;

			// if not currently engaged, or even alerted
			// take a quick look around to see if any friends seem to be in trouble
			ManChecksOnFriends(pSoldier);

			// allow stationary GREEN Civilians to turn again at least 1/turn!
		}
		pSoldier->aiData.bLastAction = AI_ACTION_NONE;

	}
}


void AIDecideRadioAnimation( SOLDIERTYPE *pSoldier )
{
	if ( pSoldier->ubBodyType != REGMALE && pSoldier->ubBodyType != BIGMALE )
	{
		// no animation available
		ActionDone( pSoldier );
		return;
	}

	if ( PTR_CIVILIAN && pSoldier->ubCivilianGroup != KINGPIN_CIV_GROUP )
	{
		// don't play anim
		ActionDone( pSoldier );
		return;
	}

	switch( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
	{
	case ANIM_STAND:

		pSoldier->EVENT_InitNewSoldierAnim( AI_RADIO, 0 , FALSE );
		break;

	case ANIM_CROUCH:

		pSoldier->EVENT_InitNewSoldierAnim( AI_CR_RADIO, 0 , FALSE );
		break;

	case ANIM_PRONE:

		ActionDone( pSoldier );
		break;
	}
}

UINT32 GetTankCannonIndex()
{
	UINT32 tankCannonIndex = 0;

	for (UINT32 i = 0; i < MAXITEMS; i++)
	{
		if (Item[i].cannon)
		{
			tankCannonIndex = Item[i].uiIndex;
			break;
		}
	}

	return tankCannonIndex;
}



INT8 ExecuteAction(SOLDIERTYPE *pSoldier)
{
	INT32 iRetCode;
	//NumMessage("ExecuteAction - Guy#",pSoldier->ubID);

	// in most cases, merc will change location, or may cause damage to opponents,
	// so a new cover check will be necessary.  Exceptions handled individually.
	SkipCoverCheck = FALSE;

	// reset this field, too
	pSoldier->aiData.bLastAttackHit = FALSE;

	// WANNE.TANK: Choose cannon or rocket
	UINT16 usHandItem = pSoldier->inv[HANDPOS].usItem;

	if (TANK(pSoldier))
	{
		// No cannon selected to fire
		if (!Item[pSoldier->inv[HANDPOS].usItem].cannon)
		{
			// 50 % chance, that the tank fires with the explosive cannon
			UINT32 fireWithCannon = GetRndNum(2);
			if (fireWithCannon)
			{
				UINT32 tankCannonIndex = GetTankCannonIndex();
				if (tankCannonIndex > 0)
				{
					usHandItem = tankCannonIndex;
				}
			}
		}
	}

	UINT16 usSoldierIndex; // added by SANDRO

#ifdef TESTAICONTROL
	if (gfTurnBasedAI || gTacticalStatus.fAutoBandageMode)
	{
		DebugAI( String( "%d does %s (a.d. %d) in %d with %d APs left", pSoldier->ubID, gzActionStr[pSoldier->aiData.bAction], pSoldier->aiData.usActionData, pSoldier->sGridNo, pSoldier->bActionPoints ) );
	}
#endif

	DebugAI( String( "%d does %s (a.d. %d) at time %lu", pSoldier->ubID, gzActionStr[pSoldier->aiData.bAction], pSoldier->aiData.usActionData, GetJA2Clock() ) );

	// 0verhaul:  The decideaction stage does so many path plots and overrides that
	// relying on a stored path from there is a bad idea.
	pSoldier->pathing.usPathDataSize = pSoldier->pathing.usPathIndex = pSoldier->pathing.bPathStored = 0;

	switch (pSoldier->aiData.bAction)
	{
	case AI_ACTION_NONE:                  // maintain current position & facing
		// do nothing
		break;

	case AI_ACTION_WAIT:									 // hold AI_ACTION_NONE for a specified time
		if (gfTurnBasedAI)
		{
			// probably an action set as a next-action in the realtime prior to combat
			// do nothing
		}
		else
		{
			RESETTIMECOUNTER( pSoldier->timeCounters.AICounter, pSoldier->aiData.usActionData );
			if (pSoldier->ubProfile != NO_PROFILE)
			{
				//DebugMsg( TOPIC_JA2, DBG_LEVEL_0, String( "%s waiting %d from %d", pSoldier->name, pSoldier->timeCounters.AICounter, GetJA2Clock() ) );
			}
		}
		ActionDone( pSoldier );
		break;

	case AI_ACTION_CHANGE_FACING:         // turn this way & that to look
		// as long as we don't see anyone new, cover won't have changed
		// if we see someone new, it will cause a new situation & remove this
		// 0verhaul:  If turning and not moving, set the final destination to the current position
		pSoldier->pathing.sFinalDestination = pSoldier->sGridNo;
		SkipCoverCheck = TRUE;

#ifdef DEBUGDECISIONS
		STR tempstr;
		sprintf( tempstr, "ExecuteAction: SkipCoverCheck ON\n" );
		DebugAI (tempstr);
#endif

		//			pSoldier->pathing.bDesiredDirection = (UINT8) ;   // turn to face direction in actionData
		SendSoldierSetDesiredDirectionEvent( pSoldier, pSoldier->aiData.usActionData );
		// now we'll have to wait for the turning to finish; no need to call TurnSoldier here
		//pSoldier->TurnSoldier( );
		/*
		if (!StartTurn(pSoldier,pSoldier->aiData.usActionData,FASTTURN))
		{
		#ifdef BETAVERSION
		sprintf(tempstr,"ERROR: %s tried TURN to direction %d, StartTurn failed, action %d CANCELED",
		pSoldier->name,pSoldier->aiData.usActionData,pSoldier->aiData.bAction);
		PopMessage(tempstr);
		#endif

		// ZAP NPC's remaining action points so this isn't likely to repeat
		pSoldier->bActionPoints = 0;

		CancelAIAction(pSoldier,FORCE);
		return(FALSE);         // nothing is in progress
		}
		else
		{
		#ifdef RECORDNET
		fprintf(NetDebugFile,"\tAI decides to turn guynum %d to dir %d\n",pSoldier->ubID,pSoldier->aiData.usActionData);
		#endif
		NetLookTowardsDir(pSoldier,pSoldier->aiData.usActionData);
		}
		*/
		break;

	case AI_ACTION_PICKUP_ITEM:					 // grab something!
		SoldierPickupItem( pSoldier, pSoldier->aiData.uiPendingActionData1, pSoldier->aiData.usActionData, 0 );
		break;

	case AI_ACTION_DROP_ITEM:					 // drop item in hand
		//CHRISL: If we have a weapon in both hands, we need to drop the weapon that isn't usable
		if(pSoldier->inv[HANDPOS].fFlags & OBJECT_AI_UNUSABLE)
		{
			SoldierDropItem( pSoldier, &(pSoldier->inv[HANDPOS]) );
			DeleteObj( &(pSoldier->inv[HANDPOS]) );
		}
		if(pSoldier->inv[SECONDHANDPOS].exists() == true && Item[pSoldier->inv[SECONDHANDPOS].usItem].usItemClass == IC_GUN && pSoldier->inv[SECONDHANDPOS].fFlags & OBJECT_AI_UNUSABLE)
		{
			SoldierDropItem( pSoldier, &(pSoldier->inv[SECONDHANDPOS]) );
			DeleteObj( &(pSoldier->inv[SECONDHANDPOS]) );
		}
		// 0verhaul:  Moved into PickDropAnimation because when something is dropped while crouched there's nothing to wait for
		// pSoldier->aiData.bAction = AI_ACTION_PENDING_ACTION;
		break;

		//case AI_ACTION_MOVE_TO_CLIMB:
		//	if ( pSoldier->aiData.usActionData == pSoldier->sGridNo )
		//	{
		//		// change action to climb now and try that.
		//		pSoldier->aiData.bAction = AI_ACTION_CLIMB_ROOF;
		//		if (IsActionAffordable(pSoldier))
		//		{
		//			return( ExecuteAction( pSoldier ) );
		//		}
		//		else
		//		{
		//			// no action started
		//			return( FALSE );
		//		}
		//	}

		// fall through
	case AI_ACTION_RANDOM_PATROL:         // move towards a particular location
	case AI_ACTION_SEEK_FRIEND:           // move towards friend in trouble
	case AI_ACTION_SEEK_OPPONENT:         // move towards a reported opponent
	case AI_ACTION_TAKE_COVER:            // run for nearest cover from threat
	case AI_ACTION_GET_CLOSER:            // move closer to a strategic location

	case AI_ACTION_POINT_PATROL:          // move towards next patrol point
	case AI_ACTION_LEAVE_WATER_GAS:       // seek nearest spot of ungassed land
	case AI_ACTION_SEEK_NOISE:            // seek most important noise heard
	case AI_ACTION_RUN_AWAY:              // run away from nearby opponent(s)
		// SANDRO - ENEMY TAUNTS
		if (gGameSettings.fOptions[TOPTION_ALLOW_TAUNTS] == TRUE && pSoldier->bTeam == ENEMY_TEAM && SOLDIER_CLASS_ENEMY( pSoldier->ubSoldierClass ) && pSoldier->bVisible != -1 )
		{
			if ( Random( 5 ) == 0 )
			{
				if (pSoldier->aiData.bAction == AI_ACTION_SEEK_NOISE )
					StartEnemyTaunt( pSoldier, TAUNT_SEEK_NOISE );
				else if (pSoldier->aiData.bAction == AI_ACTION_RUN_AWAY )
					StartEnemyTaunt( pSoldier, TAUNT_RUN_AWAY );
			}
		}
	case AI_ACTION_APPROACH_MERC:				 // walk up to someone to talk
	case AI_ACTION_TRACK:								 // track by ground scent
	case AI_ACTION_EAT:									 // monster approaching corpse
	case AI_ACTION_SCHEDULE_MOVE:
	case AI_ACTION_WALK:
	case AI_ACTION_WITHDRAW:
	case AI_ACTION_FLANK_LEFT:
	case AI_ACTION_FLANK_RIGHT:
	case AI_ACTION_RUN:
	case AI_ACTION_MOVE_TO_CLIMB:
		if ( pSoldier->aiData.bAction == AI_ACTION_MOVE_TO_CLIMB )
		{
			DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"Executing: AI_ACTION_MOVE_TO_CLIMB");
			DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("Soldier GridNo = %d, action data = %d ", pSoldier->sGridNo , pSoldier->aiData.usActionData));
		}

		if ( gfTurnBasedAI && pSoldier->aiData.bAlertStatus < STATUS_BLACK )
		{			
			if (TileIsOutOfBounds(pSoldier->sLastTwoLocations[0]))
			{
				pSoldier->sLastTwoLocations[0] = pSoldier->sGridNo;
			}			
			else if (TileIsOutOfBounds(pSoldier->sLastTwoLocations[1]))
			{
				pSoldier->sLastTwoLocations[1] = pSoldier->sGridNo;
			}
			// check for loop
			else if ( pSoldier->aiData.usActionData == pSoldier->sLastTwoLocations[1] && pSoldier->sGridNo == pSoldier->sLastTwoLocations[0] )
			{
				DebugAI( String( "%d in movement loop, aborting turn", pSoldier->ubID ) );

				// loop found!
				ActionDone( pSoldier );
				EndAIGuysTurn( pSoldier );
			}
			else
			{
				pSoldier->sLastTwoLocations[0] = pSoldier->sLastTwoLocations[1];
				pSoldier->sLastTwoLocations[1] = pSoldier->sGridNo;
			}
		}

		// Randomly do growl...
		if ( pSoldier->ubBodyType == BLOODCAT )
		{
			if ( ( gTacticalStatus.uiFlags & INCOMBAT ) )
			{
				if ( Random( 2 ) == 0 )
				{
					PlaySoldierJA2Sample( pSoldier->ubID, ( BLOODCAT_GROWL_1 + Random( 4 ) ), RATE_11025, SoundVolume( HIGHVOLUME, pSoldier->sGridNo ), 1, SoundDir( pSoldier->sGridNo ), TRUE );
				}
			}
		}

		// on YELLOW/GREEN status, NPCs keep the actions from turn to turn
		// (newSituation is intentionally NOT set in NewSelectedNPC()), so the
		// possibility exists that NOW the actionData is no longer a valid
		// NPC->pathing.sDestination (path got blocked, someone is now standing at that
		// gridno, etc.)  So we gotta check again that the->pathing.sDestination's legal!

		// optimization - Ian (if up-to-date path is known, do not check again)
		if (!pSoldier->pathing.bPathStored)
		{			
			if ( (!TileIsOutOfBounds(pSoldier->sAbsoluteFinalDestination) || gTacticalStatus.fAutoBandageMode) && !(gTacticalStatus.uiFlags & INCOMBAT) )
			{
				// NPC system move, allow path through
				if (LegalNPCDestination(pSoldier,pSoldier->aiData.usActionData,ENSURE_PATH,WATEROK, PATH_THROUGH_PEOPLE ))
				{
					// optimization - Ian: prevent another path call in SetNewCourse()
					pSoldier->pathing.bPathStored = TRUE;
				}
			}
			else
			{
				if (LegalNPCDestination(pSoldier,pSoldier->aiData.usActionData,ENSURE_PATH,WATEROK, 0))
				{
					// optimization - Ian: prevent another path call in SetNewCourse()
					pSoldier->pathing.bPathStored = TRUE;
				}
			}

			// if we STILL don't have a path
			if ( !pSoldier->pathing.bPathStored )
			{
				// Check if we were told to move by NPC stuff				
				if ( !TileIsOutOfBounds(pSoldier->sAbsoluteFinalDestination) && !(gTacticalStatus.uiFlags & INCOMBAT) )
				{
					//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_ERROR, L"AI %s failed to get path for dialogue-related move!", pSoldier->name );

					// Are we close enough?
					if ( !ACTING_ON_SCHEDULE( pSoldier ) && SpacesAway( pSoldier->sGridNo, pSoldier->sAbsoluteFinalDestination ) < 4 )
					{
						// This is close enough...
						ReplaceLocationInNPCDataFromProfileID( pSoldier->ubProfile, pSoldier->sAbsoluteFinalDestination, pSoldier->sGridNo );
						NPCGotoGridNo( pSoldier->ubProfile, pSoldier->sGridNo, (UINT8) (pSoldier->ubQuoteRecord - 1) );
					}
					else
					{
						// This is important, so try taking a path through people (and bumping them aside)
						if (LegalNPCDestination(pSoldier,pSoldier->aiData.usActionData,ENSURE_PATH,WATEROK, PATH_THROUGH_PEOPLE))
						{
							// optimization - Ian: prevent another path call in SetNewCourse()
							pSoldier->pathing.bPathStored = TRUE;
						}
						else
						{
							// Have buddy wait a while...
							pSoldier->aiData.bNextAction = AI_ACTION_WAIT;
							pSoldier->aiData.usNextActionData = (UINT16)REALTIME_AI_DELAY;
						}
					}

					if (!pSoldier->pathing.bPathStored)
					{
						CancelAIAction(pSoldier,FORCE);
						return(FALSE);         // nothing is in progress
					}
				}
				else
				{
					CancelAIAction(pSoldier,FORCE);
					return(FALSE);         // nothing is in progress
				}
			}
		}

		// add on anything necessary to traverse off map edge
		switch( pSoldier->ubQuoteActionID )
		{
		case QUOTE_ACTION_ID_TRAVERSE_EAST:
			pSoldier->sOffWorldGridNo = pSoldier->aiData.usActionData;
			AdjustSoldierPathToGoOffEdge( pSoldier, pSoldier->aiData.usActionData, EAST );
			break;
		case QUOTE_ACTION_ID_TRAVERSE_SOUTH:
			pSoldier->sOffWorldGridNo = pSoldier->aiData.usActionData;
			AdjustSoldierPathToGoOffEdge( pSoldier, pSoldier->aiData.usActionData, SOUTH );
			break;
		case QUOTE_ACTION_ID_TRAVERSE_WEST:
			pSoldier->sOffWorldGridNo = pSoldier->aiData.usActionData;
			AdjustSoldierPathToGoOffEdge( pSoldier, pSoldier->aiData.usActionData, WEST );
			break;
		case QUOTE_ACTION_ID_TRAVERSE_NORTH:
			pSoldier->sOffWorldGridNo = pSoldier->aiData.usActionData;
			AdjustSoldierPathToGoOffEdge( pSoldier, pSoldier->aiData.usActionData, NORTH );
			break;
		default:
			break;
		}

		NewDest(pSoldier,pSoldier->aiData.usActionData);    // set new->pathing.sDestination to actionData

		// make sure it worked (check that pSoldier->pathing.sDestination == pSoldier->aiData.usActionData)
		if (pSoldier->pathing.sFinalDestination != pSoldier->aiData.usActionData)
		{
#ifdef BETAVERSION
			// this should NEVER happen, indicates AI picked an illegal spot!
			sprintf(tempstr,"ExecuteAction: ERROR - %s tried MOVE to gridno %d, NewDest failed, action %d CANCELED",
				pSoldier->name,pSoldier->aiData.usActionData,pSoldier->aiData.bAction);

#ifdef RECORDNET
			fprintf(NetDebugFile,"\n%s\n\n",tempstr);
#endif

			PopMessage(tempstr);

			sprintf(tempstr,"BLACK-LISTING gridno %d for %s",pSoldier->aiData.usActionData,pSoldier->name);
			PopMessage(tempstr);

			SaveGame(ERROR_SAVE);
#endif
			// temporarily black list this gridno to stop enemy from going there
			pSoldier->pathing.sBlackList = (INT16) pSoldier->aiData.usActionData;

			DebugAI( String( "Setting blacklist for %d to %d", pSoldier->ubID, pSoldier->pathing.sBlackList ) );

			CancelAIAction(pSoldier,FORCE);
			return(FALSE);         // nothing is in progress
		}

		// cancel any old black-listed gridno, got a valid new->pathing.sDestination
		pSoldier->pathing.sBlackList = NOWHERE;
		break;

	case AI_ACTION_ESCORTED_MOVE:         // go where told to by escortPlayer
		// since this is a delayed move, gotta make sure that it hasn't become
		// illegal since escort orders were issued (->pathing.sDestination/route blocked).
		// So treat it like a CONTINUE movement, and handle errors that way
		if (!TryToResumeMovement(pSoldier,pSoldier->aiData.usActionData))
		{
			// don't black-list anything here, and action already got canceled
			return(FALSE);         // nothing is in progress
		}

		// cancel any old black-listed gridno, got a valid new->pathing.sDestination
		pSoldier->pathing.sBlackList = NOWHERE;
		break;

	case AI_ACTION_TOSS_PROJECTILE:       // throw grenade at/near opponent(s)
		LoadWeaponIfNeeded(pSoldier);
		// drop through here...

	case AI_ACTION_KNIFE_MOVE:            // preparing to stab opponent
		if (pSoldier->aiData.bAction == AI_ACTION_KNIFE_MOVE) // if statement because toss falls through
		{
			pSoldier->usUIMovementMode = DetermineMovementMode( pSoldier, AI_ACTION_KNIFE_MOVE );
		}

		// fall through
	case AI_ACTION_FIRE_GUN:              // shoot at nearby opponent
	case AI_ACTION_THROW_KNIFE:						// throw knife at nearby opponent
		// randomly decide whether to say civ quote
		if ( pSoldier->bVisible != -1 && pSoldier->bTeam != MILITIA_TEAM )
		{
			// ATE: Make sure it's a person :)
			if ( IS_MERC_BODY_TYPE( pSoldier ) && pSoldier->ubProfile == NO_PROFILE )
			{
				// SANDRO - SOLDIER TAUNTS
				if (gGameSettings.fOptions[TOPTION_ALLOW_TAUNTS] == TRUE && pSoldier->bTeam == ENEMY_TEAM && SOLDIER_CLASS_ENEMY( pSoldier->ubSoldierClass ) )
				{
					if ( Random( 8 ) == 0 )
					{
						if (Item[pSoldier->inv[HANDPOS].usItem].usItemClass == IC_GUN )
							StartEnemyTaunt( pSoldier, TAUNT_FIRE_GUN );
						else if (Item[pSoldier->inv[HANDPOS].usItem].grenadelauncher || Item[pSoldier->inv[HANDPOS].usItem].mortar || Item[pSoldier->inv[HANDPOS].usItem].rocketlauncher )
							StartEnemyTaunt( pSoldier, TAUNT_FIRE_LAUNCHER );
						else if (pSoldier->aiData.bAction == AI_ACTION_TOSS_PROJECTILE && Item[pSoldier->inv[HANDPOS].usItem].usItemClass == IC_THROWN && !Item[pSoldier->inv[HANDPOS].usItem].flare )
							StartEnemyTaunt( pSoldier, TAUNT_THROW );
						else if (pSoldier->aiData.bAction == AI_ACTION_KNIFE_MOVE )
						{
							if (Item[pSoldier->inv[HANDPOS].usItem].usItemClass == IC_BLADE )
								StartEnemyTaunt( pSoldier, TAUNT_CHARGE_KNIFE );
							//else if (Item[pSoldier->inv[HANDPOS].usItem].usItemClass == IC_PUNCH )
							//	StartEnemyTaunt( pSoldier, TAUNT_CHARGE_HTH );
						}
					}
				}
				// CC, ATE here - I put in some TEMP randomness...
				else if ( Random( 50 ) == 0 )
				{
					StartCivQuote( pSoldier );
				}
			}
		}
#ifdef RECORDNET
		fprintf(NetDebugFile,"\tExecuteAction: %d calling HandleItem(), inHand %d, actionData %d, anitype %d, oldani %d\n",
			pSoldier->ubID,pSoldier->inv[HANDPOS].item,pSoldier->aiData.usActionData,pSoldier->anitype[pSoldier->anim],pSoldier->oldani);
#endif

#ifdef TESTVERSION
		if (pSoldier->aiData.bAction == AI_ACTION_KNIFE_MOVE)
		{
			sprintf(tempstr,"TEST MSG: %s is about to go stab %s. MAKE SURE HE DOES!",
				pSoldier->name,
				ExtMen[WhoIsThere(pSoldier->aiData.usActionData)].name);

			SimulMessage(tempstr,3000,NODECRYPT);
		}
#endif

		if ( pSoldier->aiData.bAction == AI_ACTION_TOSS_PROJECTILE && IsGrenadeLauncherAttached(&pSoldier->inv[HANDPOS]) )
			usHandItem = GetAttachedGrenadeLauncher(&pSoldier->inv[HANDPOS]);

		iRetCode = HandleItem( pSoldier, pSoldier->aiData.usActionData, pSoldier->bTargetLevel, usHandItem, FALSE );
		if ( iRetCode != ITEM_HANDLE_OK)
		{
			if ( iRetCode != ITEM_HANDLE_BROKEN ) // if the item broke, this is 'legal' and doesn't need reporting
			{
				DebugAI( String( "AI %d got error code %ld from HandleItem, doing action %d, has %d APs... aborting deadlock!", pSoldier->ubID, iRetCode, pSoldier->aiData.bAction, pSoldier->bActionPoints ) );
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"AI %d got error code %ld from HandleItem, doing action %d... aborting deadlock!", pSoldier->ubID, iRetCode, pSoldier->aiData.bAction );
			}
			CancelAIAction( pSoldier, FORCE);
#ifdef TESTAICONTROL
			if (gfTurnBasedAI)
			{
				DebugAI( String("Ending turn for %d because of error from HandleItem", pSoldier->ubID ) );
			}
#endif
			EndAIGuysTurn( pSoldier );
		}
		break;

	case AI_ACTION_PULL_TRIGGER:          // activate an adjacent panic trigger

		// turn to face trigger first
		if ( FindStructure( pSoldier->sGridNo + DirectionInc( NORTH ), STRUCTURE_SWITCH ) )
		{
			SendSoldierSetDesiredDirectionEvent( pSoldier, NORTH );
		}
		else
		{
			SendSoldierSetDesiredDirectionEvent( pSoldier, WEST );
		}

		pSoldier->EVENT_InitNewSoldierAnim( AI_PULL_SWITCH, 0 , FALSE );

		DeductPoints( pSoldier, APBPConstants[AP_PULL_TRIGGER], 0 );

		//gTacticalStatus.fPanicFlags					= 0; // turn all flags off
		gTacticalStatus.ubTheChosenOne			= NOBODY;
		break;

	case AI_ACTION_USE_DETONATOR:
		//gTacticalStatus.fPanicFlags					= 0; // turn all flags off
		gTacticalStatus.ubTheChosenOne			= NOBODY;
		//gTacticalStatus.sPanicTriggerGridno	= NOWHERE;

		// grab detonator and set off bomb(s)
		DeductPoints( pSoldier, APBPConstants[AP_USE_REMOTE], APBPConstants[BP_USE_DETONATOR]);// pay for it!
		//SetOffPanicBombs(1000,COMMUNICATE);    // BOOOOOOOOOOOOOOOOOOOOM!!!!!
		SetOffPanicBombs( pSoldier->ubID, 0 );

		// action completed immediately, cancel it right away
		pSoldier->aiData.usActionData = NOWHERE;
		pSoldier->aiData.bLastAction = pSoldier->aiData.bAction;
		pSoldier->aiData.bAction = AI_ACTION_NONE;
		return(FALSE);           // no longer in progress

		break;

	case AI_ACTION_RED_ALERT:             // tell friends opponent(s) seen
		// if a computer merc, and up to now they didn't know you're here
		if (!(pSoldier->flags.uiStatusFlags & SOLDIER_PC) && ( !(gTacticalStatus.Team[pSoldier->bTeam].bAwareOfOpposition) || ( ( gTacticalStatus.fPanicFlags & PANIC_TRIGGERS_HERE ) && gTacticalStatus.ubTheChosenOne == NOBODY ) ) )
		{
			HandleInitialRedAlert(pSoldier->bTeam, TRUE);
		}
		
		// SANDRO - ENEMY TAUNTS
		if (gGameSettings.fOptions[TOPTION_ALLOW_TAUNTS] == TRUE && pSoldier->bTeam == ENEMY_TEAM && SOLDIER_CLASS_ENEMY( pSoldier->ubSoldierClass ) && pSoldier->bVisible != -1 )
		{
			if ( Random( 4 ) == 0 )
			{
				StartEnemyTaunt( pSoldier, TAUNT_ALERT );
			}
		}
		//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Debug: AI radios your position!" );
		// DROP THROUGH HERE!
	case AI_ACTION_YELLOW_ALERT:          // tell friends opponent(s) heard
		//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Debug: AI radios about a noise!" );
		/*
		NetSend.msgType = NET_RADIO_SIGHTINGS;
		NetSend.ubID  = pSoldier->ubID;

		SendNetData(ALL_NODES);
		*/
		DeductPoints(pSoldier,APBPConstants[AP_RADIO],APBPConstants[BP_RADIO]);// pay for it!
		RadioSightings(pSoldier,EVERYBODY,pSoldier->bTeam);      // about everybody
		// action completed immediately, cancel it right away

		// ATE: Change to an animation!
		AIDecideRadioAnimation( pSoldier );
		//return(FALSE);           // no longer in progress
		break;

	case AI_ACTION_CREATURE_CALL:									// creature calling to others
		DeductPoints(pSoldier,APBPConstants[AP_RADIO],APBPConstants[BP_RADIO]);// pay for it!
		CreatureCall( pSoldier );
		//return( FALSE ); // no longer in progress
		break;

	case AI_ACTION_CHANGE_STANCE:                // crouch
		if ( gAnimControl[ pSoldier->usAnimState ].ubHeight == pSoldier->aiData.usActionData )
		{
			// abort!
			ActionDone( pSoldier );
			return( FALSE );
		}

		SkipCoverCheck = TRUE;

#ifdef DEBUGDECISIONS
		sprintf( tempstr, "ExecuteAction: SkipCoverCheck ON\n" );
		DebugAI (tempstr);
#endif
		SendChangeSoldierStanceEvent( pSoldier, (UINT8) pSoldier->aiData.usActionData );
		break;

	case AI_ACTION_COWER:
		// make sure action data is set right
		if ( pSoldier->flags.uiStatusFlags & SOLDIER_COWERING )
		{
			// nothing to do!
			ActionDone( pSoldier );
			return( FALSE );
		}
		else
		{
			pSoldier->aiData.usActionData = ANIM_CROUCH;
			pSoldier->SetSoldierCowerState( TRUE );
		}
		break;

	case AI_ACTION_STOP_COWERING:
		// make sure action data is set right
		if ( pSoldier->flags.uiStatusFlags & SOLDIER_COWERING )
		{
			pSoldier->aiData.usActionData = ANIM_STAND;
			pSoldier->SetSoldierCowerState( FALSE );
		}
		else
		{
			// nothing to do!
			ActionDone( pSoldier );
			return( FALSE );
		}
		break;

	case AI_ACTION_GIVE_AID:              // help injured/dying friend
		//pSoldier->usUIMovementMode = RUNNING;
		iRetCode = HandleItem( pSoldier, pSoldier->aiData.usActionData, 0, pSoldier->inv[HANDPOS].usItem, FALSE );
		if ( iRetCode != ITEM_HANDLE_OK)
		{
#ifdef JA2BETAVERSION
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_ERROR, L"AI %d got error code %ld from HandleItem, doing action %d... aborting deadlock!", pSoldier->ubID, iRetCode, pSoldier->aiData.bAction );
#endif
			CancelAIAction( pSoldier, FORCE);
#ifdef TESTAICONTROL
			if (gfTurnBasedAI)
			{
				DebugAI( String("Ending turn for %d because of error from HandleItem", pSoldier->ubID ) );
			}
#endif
			EndAIGuysTurn( pSoldier );
		}
		break;

	case AI_ACTION_OPEN_OR_CLOSE_DOOR:
	case AI_ACTION_UNLOCK_DOOR:
	case AI_ACTION_LOCK_DOOR:
		{
			STRUCTURE *		pStructure;
			UINT8					ubDirection;
				INT32					sDoorGridNo;

			ubDirection = GetDirectionFromGridNo( pSoldier->aiData.usActionData, pSoldier );
			if (ubDirection == EAST || ubDirection == SOUTH)
			{
				sDoorGridNo = pSoldier->sGridNo;
			}
			else
			{
				sDoorGridNo = pSoldier->sGridNo + DirectionInc( ubDirection );
			}

			pStructure = FindStructure( sDoorGridNo, STRUCTURE_ANYDOOR );
			if (pStructure == NULL)
			{
#ifdef JA2TESTVERSION
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_ERROR, L"AI %d tried to open door it could not then find in %d", pSoldier->ubID, sDoorGridNo );
#endif
				CancelAIAction( pSoldier, FORCE);
#ifdef TESTAICONTROL
				if (gfTurnBasedAI)
				{
					DebugAI( String("Ending turn for %d because of error opening door", pSoldier->ubID ) );
				}
#endif
				EndAIGuysTurn( pSoldier );
			}

			StartInteractiveObject( sDoorGridNo, pStructure->usStructureID, pSoldier, ubDirection );
			InteractWithInteractiveObject( pSoldier, pStructure, ubDirection );
		}
		break;

	case AI_ACTION_LOWER_GUN:
		// for now, just do "action done"
		pSoldier->InternalSoldierReadyWeapon(pSoldier->ubDirection,TRUE);
		HandleSight(pSoldier, SIGHT_LOOK );
		ActionDone( pSoldier );
		break;

	case AI_ACTION_RAISE_GUN: //Madd: action added for snipers to ready weapon and use vision range bonuses
		pSoldier->SoldierReadyWeapon();
		HandleSight(pSoldier, SIGHT_LOOK | SIGHT_RADIO);

		//AXP 23.03.2007: Sniper deadlock fix
		if ( pSoldier->aiData.bOrders == SNIPER && pSoldier->aiData.bLastAction == AI_ACTION_RAISE_GUN)
		{
			pSoldier->aiData.bNextAction = AI_ACTION_END_TURN;
		}

		ActionDone( pSoldier );
		break;

	case AI_ACTION_CLIMB_ROOF:
		DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"Executing: AI_ACTION_CLIMB_ROOF");

		if (pSoldier->pathing.bLevel == 0)
		{
			pSoldier->BeginSoldierClimbUpRoof( );
		}
		else
		{
			pSoldier->BeginSoldierClimbDownRoof( );
		}
		break;

	case AI_ACTION_END_TURN:
		ActionDone( pSoldier );
		if (gfTurnBasedAI)
		{
			EndAIGuysTurn( pSoldier );
		}
		return( FALSE );         // nothing is in progress

	case AI_ACTION_TRAVERSE_DOWN:
		if (gfTurnBasedAI)
		{
			EndAIGuysTurn( pSoldier );
		}
		if ( pSoldier->ubProfile != NO_PROFILE )
		{
			gMercProfiles[ pSoldier->ubProfile ].bSectorZ++;
			gMercProfiles[ pSoldier->ubProfile ].fUseProfileInsertionInfo = FALSE;
		}
		TacticalRemoveSoldier( pSoldier->ubID );
		CheckForEndOfBattle( TRUE );

		return( FALSE );         // nothing is in progress

	case AI_ACTION_OFFER_SURRENDER:
		// start the offer of surrender!
		StartCivQuote( pSoldier );
		break;
	/////////////////////////////////////////////////////////////
	// SANDRO - added a possibility for enemies to try to steal your gun
	case AI_ACTION_STEAL_MOVE:            // preparing to steal opponents weapon
		
		pSoldier->aiData.ubPendingAction		= NO_PENDING_ACTION;
		pSoldier->usUIMovementMode = DetermineMovementMode( pSoldier, AI_ACTION_KNIFE_MOVE );
		usSoldierIndex = WhoIsThere2( pSoldier->aiData.usActionData, pSoldier->bTargetLevel);
		if ( usSoldierIndex != NOBODY )
			MercStealFromMerc( pSoldier, MercPtrs[usSoldierIndex] );

		break;
	/////////////////////////////////////////////////////////////

	default:
#ifdef BETAVERSION
		NumMessage("ExecuteAction - Illegal action type = ",pSoldier->aiData.bAction);
#endif
		return(FALSE);
	}

	// return status indicating execution of action was properly started
	return(TRUE);
}

void CheckForChangingOrders(SOLDIERTYPE *pSoldier)
{
	switch( pSoldier->aiData.bAlertStatus )
	{
	case STATUS_GREEN:
		if ( !CREATURE_OR_BLOODCAT( pSoldier ) )
		{			
			if ( pSoldier->bTeam == CIV_TEAM && pSoldier->ubProfile != NO_PROFILE && pSoldier->aiData.bNeutral && !TileIsOutOfBounds(gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo) && pSoldier->ubCivilianGroup != QUEENS_CIV_GROUP )
			{
				// must make them uncower first, then return to start location
				pSoldier->aiData.bNextAction = AI_ACTION_END_COWER_AND_MOVE;
				pSoldier->aiData.usNextActionData = gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo;
				gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo = NOWHERE;
			}
			else if ( pSoldier->flags.uiStatusFlags & SOLDIER_COWERING )
			{
				pSoldier->aiData.bNextAction = AI_ACTION_STOP_COWERING;
				pSoldier->aiData.usNextActionData = ANIM_STAND;
			}
			else
			{
				pSoldier->aiData.bNextAction = AI_ACTION_CHANGE_STANCE;
				pSoldier->aiData.usNextActionData = ANIM_STAND;
			}
		}
		break;
	case STATUS_YELLOW:
		break;
	default:
		if ((pSoldier->aiData.bOrders == ONGUARD) || (pSoldier->aiData.bOrders == CLOSEPATROL))
		{
			// crank up ONGUARD to CLOSEPATROL, and CLOSEPATROL to FARPATROL
			pSoldier->aiData.bOrders++;       // increase roaming range by 1 category
		}
		else if ( pSoldier->bTeam == MILITIA_TEAM && pSoldier->aiData.bOrders != SNIPER )
		{
			// go on alert!
			pSoldier->aiData.bOrders = SEEKENEMY;
		}
		else if ( CREATURE_OR_BLOODCAT( pSoldier ) )
		{
			if (pSoldier->aiData.bOrders != STATIONARY && pSoldier->aiData.bOrders != ONCALL)
			{
				pSoldier->aiData.bOrders = SEEKENEMY;
			}
		}

		if ( pSoldier->ubProfile == WARDEN )
		{
			// Tixa
			MakeClosestEnemyChosenOne();
		}
		break;
	}
}

void InitAttackType(ATTACKTYPE *pAttack)
{
	// initialize the given bestAttack structure fields to their default values
	pAttack->ubPossible          = FALSE;
	pAttack->ubOpponent          = NOBODY;
	pAttack->ubAimTime           = 0;
	pAttack->ubChanceToReallyHit = 0;
	pAttack->sTarget							= NOWHERE;
	pAttack->iAttackValue				= 0;
	pAttack->ubAPCost            = 0;
}

void HandleInitialRedAlert( INT8 bTeam, UINT8 ubCommunicate)
{
	/*
	if (ubCommunicate)
	{
	NetSend.msgType = NET_RED_ALERT;
	SendNetData(ALL_NODES);
	}*/

	if ( gTacticalStatus.Team[bTeam].bAwareOfOpposition == FALSE )
	{
#ifdef JA2TESTVERSION
		ScreenMsg( FONT_MCOLOR_RED, MSG_ERROR, L"Enemies on team %d prompted to go on RED ALERT!", bTeam );
#endif
	}

	// if there is a stealth mission in progress here, and a panic trigger exists
	if ( bTeam == ENEMY_TEAM && (gTacticalStatus.fPanicFlags & PANIC_TRIGGERS_HERE) )
	{
		// they're going to be aware of us now!
		MakeClosestEnemyChosenOne();
	}

	if ( bTeam == ENEMY_TEAM && gWorldSectorX == 3 && gWorldSectorY == MAP_ROW_P && gbWorldSectorZ == 0 )
	{
		// alert Queen and Joe if they are around
		SOLDIERTYPE *			pSoldier;

		pSoldier = FindSoldierByProfileID( QUEEN, FALSE );
		if ( pSoldier )
		{
			pSoldier->aiData.bAlertStatus = STATUS_RED;
		}

		pSoldier = FindSoldierByProfileID( JOE, FALSE );
		if ( pSoldier )
		{
			pSoldier->aiData.bAlertStatus = STATUS_RED;
		}
	}

	// open and close certain doors when this happens
	//AffectDoors(OPENDOORS, MapExt[Status.cur_sector].opendoors);
	//AffectDoors(CLOSEDOORS,MapExt[Status.cur_sector].closedoors);

	// remember enemies are alerted, prevent another red alert from happening
	gTacticalStatus.Team[ bTeam ].bAwareOfOpposition = TRUE;

}

void ManChecksOnFriends(SOLDIERTYPE *pSoldier)
{
	UINT32 uiLoop;
	SOLDIERTYPE *pFriend;

	// THIS ROUTINE SHOULD ONLY BE CALLED FOR SOLDIERS ON STATUS GREEN or YELLOW

	// go through each soldier, looking for "friends" (soldiers on same side)
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pFriend = MercSlots[ uiLoop ];

		if (!pFriend)
		{
			continue;
		}

		// if this man is neutral / NOT on my side, he's not my friend
		if (pFriend->aiData.bNeutral || (pSoldier->bSide != pFriend->bSide))
			continue;  // next merc

		// if this merc is actually ME
		if (pFriend->ubID == pSoldier->ubID)
			continue;  // next merc

		// if we can see far enough to see this friend
		// and can trace a line of sight to his x,y coordinates
		if (SoldierToSoldierLineOfSightTest(pSoldier, pFriend, TRUE, CALC_FROM_ALL_DIRS))
		{
			// if my friend is in battle or something is clearly happening there
			if ((pFriend->aiData.bAlertStatus >= STATUS_RED) || pFriend->aiData.bUnderFire || (pFriend->stats.bLife < OKLIFE))
			{
#ifdef DEBUGDECISIONS
				STR16 tempstr;
				sprintf(tempstr,"%s sees %s on alert, goes to RED ALERT!",pSoldier->name,pFriend->name );
				AIPopMessage(tempstr);
#endif

				pSoldier->aiData.bAlertStatus = STATUS_RED;
				CheckForChangingOrders(pSoldier);
				SetNewSituation( pSoldier );
				break;         // don't bother checking on any other friends
			}
			else
			{
				// if he seems suspicious or acts like he thought he heard something
				// and I'm still on status GREEN
				if ((pFriend->aiData.bAlertStatus == STATUS_YELLOW) &&
					(pSoldier->aiData.bAlertStatus < STATUS_YELLOW))
				{
#ifdef TESTVERSION
					sprintf(tempstr,"TEST MSG: %s sees %s listening, goes to YELLOW ALERT!",pSoldier->name,ExtMen[pFriend->ubID].name);
					PopMessage(tempstr);
#endif
					pSoldier->aiData.bAlertStatus = STATUS_YELLOW;    // also get suspicious
					SetNewSituation( pSoldier );
					pSoldier->aiData.sNoiseGridno = pFriend->sGridNo;  // pretend FRIEND made noise
					pSoldier->aiData.ubNoiseVolume = 3;                // remember this for 3 turns
					// keep check other friends, too, in case any are already on RED
				}
			}
		}
	}
}


void SetNewSituation( SOLDIERTYPE * pSoldier )
{
	if ( pSoldier->bTeam != gbPlayerNum )
	{
		if ( pSoldier->ubQuoteRecord == 0 && !gTacticalStatus.fAutoBandageMode && !(pSoldier->aiData.bNeutral && gTacticalStatus.uiFlags & ENGAGED_IN_CONV) )
		{
			// allow new situation to be set
			//if (gTacticalStatus.ubAttackBusyCount > 0)
			//{
			//	DebugAttackBusy( "@#!%  NOT setting NewSituation because still busy attacking.\n" );
			//}
			//else
			{
				// 0verhaul:  Let's see if we can do without this.
				pSoldier->aiData.bNewSituation = IS_NEW_SITUATION;
			}

			if ( gTacticalStatus.ubAttackBusyCount != 0 )
			{
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("BBBBBB bNewSituation is set for %d when ABC !=0.", pSoldier->ubID ) );
			}

			if ( !(gTacticalStatus.uiFlags & INCOMBAT) || (gTacticalStatus.uiFlags & REALTIME) )
			{
				// reset delay if necessary!
				RESETTIMECOUNTER( pSoldier->timeCounters.AICounter, Random( 1000 ) );
			}
		}
	}
}


void HandleAITacticalTraversal( SOLDIERTYPE * pSoldier )
{
	UINT8 ubQuoteActionID = pSoldier->ubQuoteActionID;

	HandleNPCChangesForTacticalTraversal( pSoldier );

	if ( pSoldier->ubProfile != NO_PROFILE && NPCHasUnusedRecordWithGivenApproach( pSoldier->ubProfile, APPROACH_DONE_TRAVERSAL ) )
	{
		gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags3 |= PROFILE_MISC_FLAG3_HANDLE_DONE_TRAVERSAL;
	}
	else
	{
		pSoldier->ubQuoteActionID = 0;
	}

#ifdef TESTAICONTROL
	if (gfTurnBasedAI)
	{
		DebugAI( String("Ending turn for %d because traversing out", pSoldier->ubID ) );
	}
#endif

	EndAIGuysTurn( pSoldier );
	RemoveManAsTarget( pSoldier );
	if (pSoldier->bTeam == CIV_TEAM && pSoldier->aiData.fAIFlags & AI_CHECK_SCHEDULE)
	{
		MoveSoldierFromMercToAwaySlot( pSoldier );
		pSoldier->bInSector = FALSE;
	}
	else
	{
		int iMapX = gWorldSectorX;
		int iMapY = gWorldSectorY;

		switch( ubQuoteActionID )
		{
		case QUOTE_ACTION_ID_TRAVERSE_EAST:
			++iMapX;
			break;
		case QUOTE_ACTION_ID_TRAVERSE_WEST:
			--iMapX;
			break;
		case QUOTE_ACTION_ID_TRAVERSE_SOUTH:
			++iMapY;
			break;
		case QUOTE_ACTION_ID_TRAVERSE_NORTH:
			--iMapY;
			break;
		}

		SECTORINFO *pSectorInfo = &( SectorInfo[ SECTOR( iMapX, iMapY ) ] );

		switch( pSoldier->ubSoldierClass )
		{
		case SOLDIER_CLASS_ELITE:
			++pSectorInfo->ubNumElites;
			break;

		case SOLDIER_CLASS_ARMY:
			++pSectorInfo->ubNumTroops;
			break;

		case SOLDIER_CLASS_ADMINISTRATOR:
			++pSectorInfo->ubNumAdmins;
			break;

		}

		ProcessQueenCmdImplicationsOfDeath( pSoldier );
		TacticalRemoveSoldier( pSoldier->ubID );
	}
	CheckForEndOfBattle( TRUE );
}

#ifndef __AI_H
#define __AI_H

#include "types.h"
#include "worlddef.h"
#include "Soldier Control.h"
#include "Isometric Utils.h"
#include "Rotting Corpses.h"

#define TESTAICONTROL

extern INT16	gubAIPathCosts[19][19];
#define AI_PATHCOST_RADIUS 9

extern void BeginMultiPurposeLocator( INT32 sGridNo, INT8 bLevel, BOOLEAN fSlideTo );

extern BOOLEAN gfDisplayCoverValues;
//extern INT16 gsCoverValue[WORLD_MAX];
extern INT16 * gsCoverValue;

// AI actions

enum CreatureCalls
{
	CALL_NONE = 0,
	CALL_1_PREY,
	CALL_MULTIPLE_PREY,
	CALL_ATTACKED,
	CALL_CRIPPLED,
	NUM_CREATURE_CALLS
} ;

#define DONTFORCE		0
#define FORCE			1

// ANY NEW ACTIONS ADDED - UPDATE OVERHEAD.C ARRAY WITH ACTION'S STRING VALUE
#define FIRST_MOVEMENT_ACTION AI_ACTION_RANDOM_PATROL
#define LAST_MOVEMENT_ACTION AI_ACTION_MOVE_TO_CLIMB
typedef enum
{
	AI_ACTION_NONE = 0,						// maintain current position & facing

	// actions that involve a move to another tile
	AI_ACTION_RANDOM_PATROL,			// move towards a random destination
	AI_ACTION_SEEK_FRIEND,				// move towards friend in trouble
	AI_ACTION_SEEK_OPPONENT,			// move towards a reported opponent
	AI_ACTION_TAKE_COVER,					// run for nearest cover from threat
	AI_ACTION_GET_CLOSER,					// move closer to a strategic location

	AI_ACTION_POINT_PATROL,				// move towards next patrol point
	AI_ACTION_LEAVE_WATER_GAS,		// seek nearest spot of ungassed land
	AI_ACTION_SEEK_NOISE,					// seek most important noise heard
	AI_ACTION_ESCORTED_MOVE,			// go where told to by escortPlayer
	AI_ACTION_RUN_AWAY,						// run away from nearby opponent(s)

	AI_ACTION_KNIFE_MOVE,					// preparing to stab an opponent
	AI_ACTION_APPROACH_MERC,			// move up to a merc in order to talk with them; RT
	AI_ACTION_TRACK,							// track a scent
	AI_ACTION_EAT,								// monster eats corpse
	AI_ACTION_PICKUP_ITEM,				// grab things lying on the ground

	AI_ACTION_SCHEDULE_MOVE,			// move according to schedule
	AI_ACTION_WALK,								// walk somewhere (NPC stuff etc)
	AI_ACTION_RUN,								// run somewhere (NPC stuff etc)
	AI_ACTION_WITHDRAW,	// back off
	AI_ACTION_FLANK_LEFT,	// move to the left
	AI_ACTION_FLANK_RIGHT,	// move to the right
	AI_ACTION_MOVE_TO_CLIMB,			// move to edge of roof/building
	// miscellaneous movement actions
	AI_ACTION_CHANGE_FACING,			// turn to face a different direction

	AI_ACTION_CHANGE_STANCE,			// stand, crouch, or go prone
	// actions related to items and attacks
	AI_ACTION_YELLOW_ALERT,				// tell friends opponent(s) heard
	AI_ACTION_RED_ALERT,					// tell friends opponent(s) seen
	AI_ACTION_CREATURE_CALL,			// creature communication
	AI_ACTION_PULL_TRIGGER,				// go off to activate a panic trigger

	AI_ACTION_USE_DETONATOR,			// grab detonator and set off bomb(s)
	AI_ACTION_FIRE_GUN,						// shoot at nearby opponent
	AI_ACTION_TOSS_PROJECTILE,		// throw grenade at/near opponent(s)
	AI_ACTION_KNIFE_STAB,					// during the actual knifing attack
	AI_ACTION_THROW_KNIFE,				// throw a knife

	AI_ACTION_GIVE_AID,						// help injured/dying friend
	AI_ACTION_WAIT,								// RT: don't do anything for a certain length of time
	AI_ACTION_PENDING_ACTION,			// RT: wait for pending action (pickup, door open, etc) to finish
	AI_ACTION_DROP_ITEM,					// duh
	AI_ACTION_COWER,							// for civilians:	cower in fear and stay there!

	AI_ACTION_STOP_COWERING,			// stop cowering
	AI_ACTION_OPEN_OR_CLOSE_DOOR,	// schedule-provoked; open or close door
	AI_ACTION_UNLOCK_DOOR,				// schedule-provoked; unlock door (don't open)
	AI_ACTION_LOCK_DOOR,					// schedule-provoked; lock door (close if necessary)
	AI_ACTION_LOWER_GUN,					// lower gun prior to throwing knife

	AI_ACTION_ABSOLUTELY_NONE,		// like "none" but can't be converted to a wait by realtime
	AI_ACTION_CLIMB_ROOF,					// climb up or down roof
	AI_ACTION_END_TURN,						// end turn (after final stance change)
	AI_ACTION_END_COWER_AND_MOVE,	// sort of dummy value, special for civilians who are to go somewhere at end of battle
	AI_ACTION_TRAVERSE_DOWN,			// move down a level
	AI_ACTION_OFFER_SURRENDER,		// offer surrender to the player
	AI_ACTION_RAISE_GUN,
	AI_ACTION_STEAL_MOVE, // added by SANDRO

	AI_ACTION_RELOAD_GUN,

	AI_ACTION_JUMP_WINDOW,			// added by Flugente: jump through a window
	AI_ACTION_FREE_PRISONER,		// added by Flugente: free a prisoner
	AI_ACTION_USE_SKILL,			// added by Flugente: perform a skill, which one is stored in usAISkillUse
	AI_ACTION_HANDLE_ITEM,			// sevenfm: use item in hand
	AI_ACTION_LAST = AI_ACTION_HANDLE_ITEM
} ActionType;


enum QuoteActionType
{
	QUOTE_ACTION_ID_CHECKFORDEST = 1,
	QUOTE_ACTION_ID_TURNTOWARDSPLAYER,
	QUOTE_ACTION_ID_DRAWGUN,
	QUOTE_ACTION_ID_LOWERGUN,
	QUOTE_ACTION_ID_TRAVERSE_EAST,
	QUOTE_ACTION_ID_TRAVERSE_SOUTH,
	QUOTE_ACTION_ID_TRAVERSE_WEST,
	QUOTE_ACTION_ID_TRAVERSE_NORTH,
} ;

#define RTP_COMBAT_AGGRESSIVE		1
#define RTP_COMBAT_CONSERVE			2
#define RTP_COMBAT_REFRAIN			3

// NB THESE THREE FLAGS SHOULD BE REMOVED FROM CODE
//#define AI_RTP_OPTION_CAN_RETREAT				0x01
//#define AI_RTP_OPTION_CAN_SEEK_COVER		0x02
//#define AI_RTP_OPTION_CAN_HELP					0x04

#define AI_CAUTIOUS											0x08
#define AI_HANDLE_EVERY_FRAME						0x10
#define AI_ASLEEP												0x20
#define AI_LOCK_DOOR_INCLUDES_CLOSE			0x40
#define AI_CHECK_SCHEDULE								0x80

#define NOT_NEW_SITUATION 0
#define WAS_NEW_SITUATION 1
#define IS_NEW_SITUATION 2


#define DIFF_ENEMY_EQUIP_MOD			0
#define DIFF_ENEMY_TO_HIT_MOD			1
#define DIFF_ENEMY_INTERRUPT_MOD		2
#define DIFF_RADIO_RED_ALERT			3
#define DIFF_MAX_COVER_RANGE			4
#define MAX_DIFF_PARMS									5		// how many different difficulty variables?

extern INT8 gbDiff[MAX_DIFF_PARMS][5];

void ActionDone(SOLDIERTYPE *pSoldier);
INT16 ActionInProgress(SOLDIERTYPE *pSoldier);

INT8 CalcMorale(SOLDIERTYPE *pSoldier);
INT32 CalcPercentBetter(INT32 iOldValue, INT32 iNewValue, INT32 iOldScale, INT32 iNewScale);
void CallAvailableEnemiesTo(INT32 sGridNo);
void CallAvailableKingpinMenTo( INT32 sGridNo );
void CallAvailableTeamEnemiesTo( INT32 sGridNo, INT8 bTeam );
void CallEldinTo( INT32 sGridNo );
void CancelAIAction(SOLDIERTYPE *pSoldier, UINT8 ubForce);
void CheckForChangingOrders(SOLDIERTYPE *pSoldier );

INT8 ClosestPanicTrigger( SOLDIERTYPE * pSoldier );

INT32 ClosestKnownOpponent(SOLDIERTYPE *pSoldier, INT32 * psGridNo, INT8 * pbLevel);
INT32 ClosestPC( SOLDIERTYPE *pSoldier, INT32 * psDistance );
INT32 ClosestUnDisguisedPC( SOLDIERTYPE *pSoldier, INT32 * psDistance );	// Flugente: like ClosestPC(...), but does not account for covert or not visible mercs
BOOLEAN CanAutoBandage( BOOLEAN fDoFullCheck );

void DebugAI( STR szOutput );
INT8 DecideAction(SOLDIERTYPE *pSoldier);
INT8 DecideActionBlack(SOLDIERTYPE *pSoldier);
INT8 DecideActionEscort(SOLDIERTYPE *pSoldier);
INT8 DecideActionGreen(SOLDIERTYPE *pSoldier);
INT8 DecideActionRed(SOLDIERTYPE *pSoldier);
INT8 DecideActionYellow(SOLDIERTYPE *pSoldier);

INT16 DistanceToClosestFriend( SOLDIERTYPE * pSoldier );

void EndAIDeadlock(void);
void EndAIGuysTurn( SOLDIERTYPE *pSoldier );

INT8	ExecuteAction(SOLDIERTYPE *pSoldier);

INT32 FindAdjacentSpotBeside(SOLDIERTYPE *pSoldier, INT32 sGridNo);
INT32 FindBestNearbyCover(SOLDIERTYPE *pSoldier, INT32 morale, INT32 *pPercentBetter);
INT32 FindClosestDoor( SOLDIERTYPE * pSoldier );
INT32 FindNearbyPointOnEdgeOfMap( SOLDIERTYPE * pSoldier, INT8 * pbDirection );
INT32 FindNearestEdgePoint( INT32 sGridNo );
INT32 FindNearestPassableSpot( INT32 sGridNo, UINT8 usSearchRadius = 5 );

//Kris:	Added these as I need specific searches on certain sides.
enum
{
	NORTH_EDGEPOINT_SEARCH,
	EAST_EDGEPOINT_SEARCH,
	SOUTH_EDGEPOINT_SEARCH,
	WEST_EDGEPOINT_SEARCH,
};
INT32 FindNearestEdgepointOnSpecifiedEdge( INT32 sGridNo, INT8 bEdgeCode );

INT32 FindNearestUngassedLand(SOLDIERTYPE *pSoldier);
BOOLEAN FindRoofClimbingPoints( SOLDIERTYPE * pSoldier, INT16 sDesiredSpot );
INT32 FindSpotMaxDistFromOpponents(SOLDIERTYPE *pSoldier);
INT32 FindSweetCoverSpot(SOLDIERTYPE *pSoldier);

void FreeUpNPCFromAttacking(UINT8 ubID);
void FreeUpNPCFromPendingAction( SOLDIERTYPE * pSoldier );
void FreeUpNPCFromTurning(SOLDIERTYPE *pSoldier, INT8 bLook);
void FreeUpNPCFromStanceChange(SOLDIERTYPE *pSoldier );
void FreeUpNPCFromLoweringGun( SOLDIERTYPE *pSoldier );
void FreeUpNPCFromRoofClimb(SOLDIERTYPE *pSoldier );

UINT8 GetClosestOpponent( SOLDIERTYPE *pSoldier );
UINT8 GetMostThreateningOpponent( SOLDIERTYPE *pSoldier );

void HandleSoldierAI( SOLDIERTYPE *pSoldier );
void HandleInitialRedAlert( INT8 bTeam, UINT8 ubCivGroup);

void InitPanicSystem();
INT16 InWaterOrGas(SOLDIERTYPE *pSoldier, INT32 sGridno);
BOOLEAN IsActionAffordable(SOLDIERTYPE *pSoldier, INT8 bAction = AI_ACTION_NONE);
BOOLEAN InitAI( void );

void MakeClosestEnemyChosenOne();
void ManChecksOnFriends(SOLDIERTYPE *pSoldier);

void NewDest(SOLDIERTYPE *pSoldier, INT32 sGridNo);
INT32 NextPatrolPoint(SOLDIERTYPE *pSoldier);

INT8 PanicAI(SOLDIERTYPE *pSoldier, UINT8 ubCanMove);
void HaltMoveForSoldierOutOfPoints(SOLDIERTYPE *pSoldier);

INT32 RandDestWithinRange(SOLDIERTYPE *pSoldier);
INT16 RandomFriendWithin(SOLDIERTYPE *pSoldier);

INT16 RoamingRange(SOLDIERTYPE *pSoldier, INT32 *pFromGridNo);

void SetCivilianDestination(UINT8 ubWho, INT32 sGridNo);
void SetNewSituation( SOLDIERTYPE * pSoldier );

UINT8 SoldierDifficultyLevel( SOLDIERTYPE * pSoldier );
void SoldierTriesToContinueAlongPath(SOLDIERTYPE *pSoldier);
void StartNPCAI(SOLDIERTYPE *pSoldier);
void TempHurt(SOLDIERTYPE *pVictim, SOLDIERTYPE *pAttacker);
int TryToResumeMovement(SOLDIERTYPE *pSoldier, INT32 sGridNo);

BOOLEAN ValidCreatureTurn( SOLDIERTYPE * pCreature, INT8 bNewDirection );

BOOLEAN WearGasMaskIfAvailable( SOLDIERTYPE * pSoldier );
INT16 WhatIKnowThatPublicDont(SOLDIERTYPE *pSoldier, UINT8 ubInSightOnly);

INT32 FindClosestClimbPoint (SOLDIERTYPE *pSoldier, BOOLEAN fClimbUp );
INT32 FindFlankingSpot(SOLDIERTYPE *pSoldier, INT32 sPos, INT8 bAction );
BOOLEAN CanClimbFromHere (SOLDIERTYPE * pSoldier, BOOLEAN fUp );

// HEADROCK HAM 3.6: Adding includes for A* cover calculations
INT8 CalcWorstCTGTForPosition( SOLDIERTYPE * pSoldier, UINT8 ubOppID, INT32 sOppGridNo, INT8 bLevel, INT32 iMyAPsLeft );
INT8 CalcBestCTGT( SOLDIERTYPE *pSoldier, UINT8 ubOppID, INT32 sOppGridNo, INT8 bLevel, INT32 iMyAPsLeft );
INT8 CalcAverageCTGTForPosition( SOLDIERTYPE * pSoldier, UINT8 ubOppID, INT32 sOppGridNo, INT8 bLevel, INT32 iMyAPsLeft );
UINT8 NumberOfTeamMatesAdjacent( SOLDIERTYPE * pSoldier, INT32 sGridNo );

// Flugente: get the id of the closest soldier (closer than x tiles) of a specific team with a specific flag that we can currently see
UINT8 GetClosestFlaggedSoldierID( SOLDIERTYPE * pSoldier, INT16 aRange, UINT8 auTeam, UINT32 aFlag, BOOLEAN fCheckSight = TRUE );

// sevenfm:
INT16 MaxNormalVisionDistance( void );
BOOLEAN GuySawEnemy( SOLDIERTYPE * pSoldier, UINT8 ubMax = SEEN_3_TURNS_AGO );
BOOLEAN GuyHeardEnemy(SOLDIERTYPE * pSoldier, UINT8 ubMax = HEARD_2_TURNS_AGO);
INT32 ClosestSeenLastTurnOpponent(SOLDIERTYPE *pSoldier, INT32 * psGridNo, INT8 * pbLevel);
UINT8 CountSeenEnemiesLastTurn( SOLDIERTYPE *pSoldier );
BOOLEAN GuyKnowsEnemyPosition( SOLDIERTYPE * pSoldier );
BOOLEAN EnemySeenSoldierRecently( SOLDIERTYPE *pSoldier, UINT8 ubMax, BOOLEAN fOnlyAlerted );
BOOLEAN EnemyHeardSoldierRecently( SOLDIERTYPE *pSoldier, UINT8 ubMax, BOOLEAN fOnlyAlerted );
BOOLEAN NightLight( void );
BOOLEAN CheckDoorAtGridno( UINT32 usGridNo );
BOOLEAN CheckDoorNearGridno( UINT32 usGridNo );
BOOLEAN FindBombNearby( SOLDIERTYPE *pSoldier, INT32 sGridNo, UINT8 ubDistance);
INT16 DistanceToClosestNotSeekEnemyFriend( SOLDIERTYPE *pSoldier, INT32 sGridNo );
BOOLEAN LastTargetCollapsed( SOLDIERTYPE *pSoldier );
BOOLEAN LastTargetSuppressed( SOLDIERTYPE *pSoldier );
BOOLEAN SoldierAI( SOLDIERTYPE *pSoldier );
BOOLEAN AICheckSuccessfulAttack(SOLDIERTYPE *pSoldier, BOOLEAN fGroup);

UINT8 CountNearbyFriends( SOLDIERTYPE *pSoldier, INT32 sGridNo, UINT8 ubDistance );
UINT8 CountNearbyNeutrals(SOLDIERTYPE *pSoldier, INT32 sGridNo, INT16 sDistance);
UINT8 CountFriendsInDirection(SOLDIERTYPE *pSoldier, UINT8 ubDirection, INT16 sDistance, BOOLEAN fCheckSight);
UINT8 CountFriendsInDirectionFromSpot(SOLDIERTYPE *pSoldier, INT32 sSpot, UINT8 ubDirection, INT16 sDistance);
UINT8 CountFriendsBetweenMeAndSpotFromSpot(SOLDIERTYPE *pSoldier, INT32 sTargetGridNo);
UINT8 CountFriendsBlack( SOLDIERTYPE *pSoldier, INT32 sClosestOpponent = NOWHERE );
UINT8 CountNearbyFriendsOnRoof( SOLDIERTYPE *pSoldier, INT32 sGridNo, UINT8 ubDistance );
UINT8 CountFriendsFlankSameSpot(SOLDIERTYPE *pSoldier, INT32 sSpot = NOWHERE);
UINT8 CountNearbyFriendsLastAttackHit( SOLDIERTYPE *pSoldier, INT32 sGridNo, UINT8 ubDistance );
UINT8 CountFriendsNeedHelp( SOLDIERTYPE *pSoldier );
UINT8 CountPublicKnownEnemies( SOLDIERTYPE *pSoldier, INT32 sGridNo, UINT8 ubDistance );
UINT8 CountSeenCovertOpponents( SOLDIERTYPE *pSoldier );
UINT8 CountFriendsLastAttackHit(SOLDIERTYPE *pSoldier, INT32 sGridNo, INT16 sDistance);

UINT8 CountTeamCombat(SOLDIERTYPE *pSoldier);
UINT8 CountTeamSeeSoldier(INT8 bTeam, SOLDIERTYPE *pSoldier);
BOOLEAN TeamKnowsSoldier(INT8 bTeam, UINT8 ubID);

UINT8 CountFriendsNotAlerted(SOLDIERTYPE *pSoldier);
void AlertFriends(INT8 bTeam, UINT8 ubCivGroup);

BOOLEAN AICheckIsFlanking( SOLDIERTYPE *pSoldier );

BOOLEAN AICheckIsSniper(SOLDIERTYPE *pSoldier);
BOOLEAN AICheckIsMarksman(SOLDIERTYPE *pSoldier);
BOOLEAN AICheckIsMachinegunner(SOLDIERTYPE *pSoldier);
BOOLEAN AICheckIsRadioOperator(SOLDIERTYPE *pSoldier);
BOOLEAN AICheckIsMedic(SOLDIERTYPE *pSoldier);
BOOLEAN AICheckIsMortarOperator(SOLDIERTYPE *pSoldier);
BOOLEAN AICheckIsOfficer(SOLDIERTYPE *pSoldier);
BOOLEAN AICheckIsGLOperator(SOLDIERTYPE *pSoldier);
BOOLEAN AICheckIsCommander(SOLDIERTYPE *pSoldier);

BOOLEAN AICheckSpecialRole(SOLDIERTYPE *pSoldier);

BOOLEAN ProneSightCoverAtSpot( SOLDIERTYPE *pSoldier, INT32 sSpot, BOOLEAN fUnlimited = FALSE );
BOOLEAN SightCoverAtSpot(SOLDIERTYPE *pSoldier, INT32 sSpot, BOOLEAN fUnlimited = FALSE);

BOOLEAN FindFenceAroundSpot(INT32 sSpot);

BOOLEAN AIGunScoped(SOLDIERTYPE *pSoldier);
BOOLEAN AIGunInHandScoped(SOLDIERTYPE *pSoldier);
UINT16 AIGunRange(SOLDIERTYPE *pSoldier);
UINT16 AIGunClass(SOLDIERTYPE *pSoldier);
UINT16 AIGunType(SOLDIERTYPE *pSoldier);
INT16 AIGunMinAPsToShoot(SOLDIERTYPE *pSoldier, BOOLEAN fRaiseCost = TRUE);
FLOAT AIGunScopeMagFactor(SOLDIERTYPE *pSoldier);
UINT16 AIGunAmmo(SOLDIERTYPE *pSoldier);
BOOLEAN AIGunAutofireCapable(SOLDIERTYPE *pSoldier);
UINT8 AIGunDeadliness(SOLDIERTYPE *pSoldier);

#define MAX_SUSPICION 100
UINT32 CountSuspicionValue( SOLDIERTYPE *pSoldier );
BOOLEAN EnemyCanSeeMe( SOLDIERTYPE *pSoldier );
BOOLEAN EnemyAlerted( SOLDIERTYPE *pSoldier );
BOOLEAN TeamEnemyAlerted(INT8 bTeam);

BOOLEAN ValidOpponent(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent);
BOOLEAN ValidTeamOpponent(INT8 bTeam, SOLDIERTYPE* pOpponent);

enum {AI_MSG_START, AI_MSG_DECIDE, AI_MSG_INFO, AI_MSG_TOPIC};
void DebugAI(INT8 bMsgType, SOLDIERTYPE *pSoldier, STR szOutput, INT8 bAction = -1);

INT16 VisionRange(void);
INT16 DayVisionRange(void);
INT16 NightVisionRange(void);

#define MAX_FLANKS_RED 25
#define MAX_FLANKS_YELLOW 25

// vision range defines
#define MAX_VISION_RANGE (gGameExternalOptions.ubStraightSightRange * 4 * STRAIGHT_RATIO)
#define DAY_VISION_RANGE DayVisionRange()
#define NIGHT_VISION_RANGE NightVisionRange()
#define VISION_RANGE VisionRange()

// sevenfm: distance for tactical AI checks, roughly equal to normal day vision range
#define TACTICAL_RANGE (gGameExternalOptions.ubStraightSightRange * STRAIGHT_RATIO * 2)
#define BOMB_DETECTION_RANGE (TACTICAL_RANGE / 4)

// sevenfm: limit min/max flank distance depending on sight range and time of day
#define MIN_FLANK_DIST (DAY_VISION_RANGE / 2)
#define MAX_FLANK_DIST (VISION_RANGE + TACTICAL_RANGE)

UINT8 RedSmokeDanger( INT32 sGridNo, INT8 bLevel );
BOOLEAN CheckArtilleryStrike( void );
BOOLEAN CheckRoof( INT32 sGridNo );

BOOLEAN AICheckHasGun( SOLDIERTYPE *pSoldier );
BOOLEAN AICheckShortWeaponRange( SOLDIERTYPE *pSoldier );
BOOLEAN AnyCoverAtSpot( SOLDIERTYPE *pSoldier, INT32 sSpot );
BOOLEAN AnyCoverFromSpot( INT32 sSpot, INT8 bLevel, INT32 sThreatLoc, INT8 bThreatLevel );
BOOLEAN CheckSuppressionDirection( SOLDIERTYPE *pSoldier, INT32 sTargetGridNo, INT8 bTargetLevel );
BOOLEAN AICheckNVG( SOLDIERTYPE *pSoldier );
INT8 AIEstimateInterruptLevel( SOLDIERTYPE *pSoldier );
INT8 FindMaxEnemyInterruptLevel( SOLDIERTYPE *pSoldier, INT32 sGridNo, INT8 blevel, UINT8 ubDistance );
UINT8 AIDirection(INT32 sSpot1, INT32 sSpot2);
UINT8 FlankingDirection(SOLDIERTYPE *pSoldier);
BOOLEAN WeAttack(INT8 bTeam);
UINT8 CountKnownEnemiesInDirection(SOLDIERTYPE *pSoldier, UINT8 ubDirection, INT16 sDistance, BOOLEAN fAdjacent);

INT8 Knowledge(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID);
INT32 KnownLocation(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID);
INT8 KnownLevel(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID);

BOOLEAN UsePersonalKnowledge(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID);

INT8 PersonalKnowledge(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID);
INT32 KnownPersonalLocation(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID);
INT8 KnownPersonalLevel(SOLDIERTYPE *pSoldier, UINT8 ubOpponentID);

INT8 PublicKnowledge(UINT8 bTeam, UINT8 ubOpponentID);
INT32 KnownPublicLocation(UINT8 bTeam, UINT8 ubOpponentID);
INT8 KnownPublicLevel(UINT8 bTeam, UINT8 ubOpponentID);

enum{
	ADVANCE_SPOT_SIGHT_COVER,
	ADVANCE_SPOT_PRONE_COVER,
	ADVANCE_SPOT_ANY_COVER
};

BOOLEAN AICheckTown(void);
UINT8 AISectorType(void);
BOOLEAN AICheckUnderground(void);
BOOLEAN NorthSpot(INT32 sSpot, INT8 bLevel);
BOOLEAN AllowDeepWaterFlanking(SOLDIERTYPE *pSoldier);
INT32	RandomizeLocation(INT32 sSpot, INT8 bLevel, UINT8 ubTimes, SOLDIERTYPE *pSightSoldier);
INT32	RandomizeOpponentLocation(INT32 sSpot, SOLDIERTYPE *pOpponent, INT16 sMaxDistance);
BOOLEAN InSmoke(INT32 sGridNo, INT8 bLevel);
BOOLEAN SafeSpot(SOLDIERTYPE *pSoldier, INT32 sSpot = NOWHERE);
BOOLEAN AbortFinalSpot(SOLDIERTYPE *pSoldier, INT32 sSpot, INT8 bAction, INT32 sClosestDisturbance, INT8 bDisturbanceLevel, INT32 &sDangerousSpot);
// needs prepared path before calling this function
BOOLEAN AbortPath(SOLDIERTYPE *pSoldier, INT8 bAction, INT32 sClosestDisturbance, INT8 bDisturbanceLevel, INT32 &sDangerousSpot, INT32 &sLastSafeSpot);
BOOLEAN UseSightCoverAdvance(SOLDIERTYPE *pSoldier);
UINT8 TeamPercentKilled(INT8 bTeam);
BOOLEAN TeamHighPercentKilled(INT8 bTeam);
UINT8 ArmyPercentKilled(void);
UINT8 ArmyPercentKilledTolerance(void);
UINT8 SectorCurfew(BOOLEAN fNight);
BOOLEAN AICheckDefense(SOLDIERTYPE *pSoldier);
BOOLEAN AICheckInterrupt(void);
UINT8 CountTeamUnderAttack(INT8 bTeam, INT32 sGridNo, INT16 sDistance);
INT32 FindAdvanceSpot(SOLDIERTYPE *pSoldier, INT32 sTargetSpot, INT8 bAction, UINT8 ubType, BOOLEAN fUnlimited);
BOOLEAN FindObstacleNearSpot(INT32 sSpot, INT8 bLevel);
BOOLEAN EnemyCanAttackSpot(SOLDIERTYPE *pSoldier, INT32 sSpot, INT8 bLevel);

BOOLEAN CorpseWarning(SOLDIERTYPE *pSoldier, INT32 sGridNo, INT8 bLevel, BOOLEAN fFresh = FALSE);
INT32	CountCorpses(SOLDIERTYPE *pSoldier, INT32 sSpot, INT16 sDistance, BOOLEAN fCheckSight, BOOLEAN fFresh);
BOOLEAN CorpseEnemyTeam(ROTTING_CORPSE *pCorpse);
BOOLEAN CorpseMilitiaTeam(ROTTING_CORPSE *pCorpse);

void PrepareMainRedAIWeights(SOLDIERTYPE *pSoldier, INT8 &bSeekPts, INT8 &bHelpPts, INT8 &bHidePts, INT8 &bWatchPts);
INT8 DecideStartFlanking(SOLDIERTYPE *pSoldier, INT32 sClosestDisturbance, BOOLEAN fAbortSeek);
INT8 DecideContinueFlanking(SOLDIERTYPE *pSoldier, INT32 sClosestDisturbance);
INT8 DecideUseWirecutters(SOLDIERTYPE *pSoldier);
INT8 DecideUseGrenadeSpecial(SOLDIERTYPE *pSoldier);

#endif

#ifdef PRECOMPILEDHEADERS
#include "AI All.h"
#else
#include "ai.h"
#include "Weapons.h"
#include "opplist.h"
#include "AIInternals.h"
#include "LOS.h"
#include "Physics.h"
#include "Items.h"
#include "Weapons.h"
#include "Spread Burst.h"
#include "overhead.h"
#include "SkillCheck.h"
#include "Soldier Profile.h"
#include "Isometric Utils.h"
#include "Soldier macros.h"
#include "PATHAI.H"
#include "GameSettings.h"
#include "strategicmap.h"
#include "environment.h"
#include "lighting.h"
#include "Sound Control.h"
#include "message.h"
#include "Vehicles.h"
#include "Soldier Functions.h"//dnl ch69 140913
#include "Reinforcement.h"		// added by Flugente
#include "Town Militia.h"		// added by Flugente
#include "Queen Command.h"		// added by Flugente
#include "Game Clock.h"			// sevenfm
#include "Structure Wrap.h"
#include "Worldman.h"
#include "Rotting Corpses.h"
#endif

// anv: for enemy taunts
#include "Civ Quotes.h"

extern INT16 DirIncrementer[8];

//
// CJC DG->JA2 conversion notes
//
// Still commented out:
//
// EstimateShotDamage - stuff related to legs?
// EstimateStabDamage - stuff related to armour
// EstimateThrowDamage - waiting for grenade, armour definitions
// CheckIfTossPossible - waiting for grenade definitions

// this define should go in soldier control.h


void LoadWeaponIfNeeded(SOLDIERTYPE *pSoldier)
{
	UINT16 usInHand;
	INT8 bPayloadPocket;

	DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("LoadWeaponIfNeeded"));

	usInHand = pSoldier->inv[HANDPOS].usItem;

	if ( IsGrenadeLauncherAttached(&pSoldier->inv[HANDPOS]) )
		usInHand = GetAttachedGrenadeLauncher(&pSoldier->inv[HANDPOS]);

	// if he's got a MORTAR in his hand, make sure he has a MORTARSHELL avail.
	if (Item[usInHand].mortar )
	{
		//		bPayloadPocket = FindObj( pSoldier, MORTAR_SHELL );
		bPayloadPocket = FindLaunchable( pSoldier, usInHand );
		if (bPayloadPocket == NO_SLOT)
		{
#ifdef BETAVERSION
			NumMessage("LoadWeaponIfNeeded: ERROR - no mortar shells found to load MORTAR!	Guynum",pSoldier->ubID);
#endif
			return;	// no shells, can't fire the MORTAR
		}
	}
	// if he's got a GL in his hand, make sure he has some type of GRENADE avail.
	else if (Item[usInHand].grenadelauncher )
	{
		bPayloadPocket = FindGLGrenade( pSoldier );
		if (bPayloadPocket == NO_SLOT || FindNonSmokeLaunchableAttachment( &pSoldier->inv[HANDPOS],usInHand ) != 0 )
		{
#ifdef BETAVERSION
			NumMessage("LoadWeaponIfNeeded: ERROR - no grenades found to load GLAUNCHER!	Guynum",pSoldier->ubID);
#endif
			return;	// no grenades, can't fire the GLAUNCHER... or the launcher has a magsize > 1
		}
	}
	// if he's got a RPG7 in his hand, make sure he has some type of RPG avail.
	else if (Item[usInHand].rocketlauncher && !Item[usInHand].singleshotrocketlauncher )
	{
		bPayloadPocket = FindLaunchable (pSoldier, usInHand );
		if (bPayloadPocket == NO_SLOT)
		{
			return;	// no grenades, can't fire
		}
	}
	else if (Item[usInHand].cannon )
	{
		bPayloadPocket = FindLaunchable( pSoldier, usInHand );
		if (bPayloadPocket == NO_SLOT)
		{
			return;
		}
	}
	else
	{
		// regular hand-thrown grenade in hand, nothing to load!
		return;
	}

	DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("LoadWeaponIfNeeded: remove payload from its pocket, and add it as the hand weapon's first attachment"));
	// remove payload from its pocket, and add it as the hand weapon's first attachment

	if ( TANK( pSoldier ) )
	{
		// don't remove ammo
		gTempObject = pSoldier->inv[bPayloadPocket];
		if (gTempObject.ubNumberOfObjects > 1) {
			gTempObject.RemoveObjectsFromStack(gTempObject.ubNumberOfObjects - 1);
		}
		pSoldier->inv[HANDPOS].AttachObject(pSoldier,&gTempObject,FALSE);
	}
	else if (pSoldier->inv[bPayloadPocket].MoveThisObjectTo(gTempObject, 1) == 0) {
		if(pSoldier->inv[HANDPOS].AttachObject(pSoldier, &gTempObject, FALSE))//dnl ch63 250813 return back rest of object or drop it if not proper attachment
		{
			if(gTempObject.ubNumberOfObjects == 1 && gTempObject[0]->data.objectStatus > 0)
				gTempObject.MoveThisObjectTo(pSoldier->inv[bPayloadPocket], 1);
		}
		else
			AddItemToPool(pSoldier->sGridNo, &gTempObject, 0, pSoldier->pathing.bLevel, WORLD_ITEM_DROPPED_FROM_ENEMY, -1);
	}
}

// FROM SB JA2005
void ResetWeaponMode( SOLDIERTYPE * pSoldier )
{
	// ATE: Don't do this if in a fire amimation.....
	if ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_FIRE )
	{
		return;
	}

	pSoldier->bWeaponMode = WM_NORMAL;

//<DR>
	pSoldier->aiData.bShownAimTime = REFINE_AIM_1;

	//!pSoldier->aiData.ubBurstAP = 0; //SB reset long burst length
	//^^^^no such property, add to soldier control
//	gfDisplayFullCountRing = FALSE;
//	gfDisplayFullCountRingBurst = FALSE;
//</DR>
//	pSoldier->bDoBurst = Weapon[Item[pSoldier->inv[HANDPOS].usItem].ubClassIndex].mode[WM_NORMAL].usROF > 0;

//	DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
//	gfUIForceReExamineCursorData = TRUE;

//	gfShowBurstLength = Weapon[Item[pSoldier->inv[HANDPOS].usItem].ubClassIndex].mode[pSoldier->bWeaponMode].usROF > 0;
//	gfShowBurstLength = Weapon[Item[pSoldier->inv[HANDPOS].usItem].ubClassIndex].mode[pSoldier->bWeaponMode].ubBullets > 1;

}
//</SB>

void CalcBestShot(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestShot)
{
	UINT32 uiLoop;
	INT32 iAttackValue, iThreatValue, iHitRate, iBestHitRate, iPercentBetter, iEstDamage, iTrueLastTarget;
	UINT16 usTrueState, usTurningCost, usRaiseGunCost;
	INT16 sAimTime, ubMinAPcost, ubRawAPCost, sBestAPcost, ubChanceToHit, ubBestAimTime, ubChanceToGetThrough, ubBestChanceToGetThrough, ubFriendlyFireChance, ubBestFriendlyFireChance, ubBestChanceToHit, sStanceAPcost;
	INT16 sAimAPCost;
	BOOLEAN fAddingTurningCost, fAddingRaiseGunCost;
	UINT8 ubMaxPossibleAimTime, ubStance, ubBestStance, ubChanceToReallyHit;
	INT8 bScopeMode;
	SOLDIERTYPE *pOpponent;

	// sevenfm:
	BOOLEAN fSuppression = FALSE;
	INT32 sTarget = NOWHERE;
	INT8 bLevel;
	INT8 bKnowledge;
	INT8 bPersonalKnowledge;
	INT8 bPublicKnowledge;

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"CalcBestShot");

	ubBestChanceToHit = ubBestAimTime = ubChanceToHit = ubBestChanceToGetThrough = ubBestFriendlyFireChance = ubChanceToReallyHit = 0;

	pSoldier->usAttackingWeapon = pSoldier->inv[HANDPOS].usItem;
	pSoldier->bWeaponMode = WM_NORMAL;
	std::map<INT8, OBJECTTYPE*> ObjList;
	GetScopeLists(&pSoldier->inv[HANDPOS], ObjList);
	pSoldier->bScopeMode = USE_BEST_SCOPE;
	pSoldier->bDoBurst = 0;
	pSoldier->bDoAutofire = 0;
	//ubBurstAPs = CalcAPsToBurst( pSoldier->CalcActionPoints( ), &(pSoldier->inv[HANDPOS]), pSoldier );//dnl ch64 270813

	//InitAttackType(pBestShot);		// set all structure fields to defaults//dnl ch69 150913 already initialize from class constructor

	// sevenfm: initialize
	pBestShot->ubPossible = FALSE;
	pBestShot->ubChanceToReallyHit = 0;
	pBestShot->iAttackValue = 0;
	pBestShot->ubOpponent = NOBODY;
	pBestShot->ubFriendlyFireChance = 0;
	pSoldier->bAimShotLocation = AIM_SHOT_RANDOM;

	// hang a pointer into active soldier's personal opponent list
	//pbPersOL = &(pSoldier->aiData.bOppList[0]);

	// determine which attack against which target has the greatest attack value
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[ uiLoop ];
		fSuppression = FALSE;

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpponent || !pOpponent->stats.bLife)
			continue;			// next merc

		// if this man is neutral / on the same side, he's not an opponent
		if (!ValidOpponent(pSoldier, pOpponent))
			continue;

		bKnowledge = Knowledge(pSoldier, pOpponent->ubID);
		bPersonalKnowledge = PersonalKnowledge(pSoldier, pOpponent->ubID);
		bPublicKnowledge = PublicKnowledge(pSoldier->bTeam, pOpponent->ubID);
		// check knowledge
		if (bKnowledge != SEEN_CURRENTLY &&
			bKnowledge != SEEN_THIS_TURN &&
			bKnowledge != SEEN_LAST_TURN &&
			bKnowledge != HEARD_THIS_TURN &&
			bKnowledge != HEARD_LAST_TURN &&
			!((bKnowledge == SEEN_2_TURNS_AGO || bKnowledge == SEEN_3_TURNS_AGO || bKnowledge == HEARD_2_TURNS_AGO) && Weapon[pSoldier->usAttackingWeapon].ubWeaponType == GUN_LMG))
		{
			DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("CalcBestShot: soldier = %d, target = %d, skip guys nobody sees, public opplist = %d",pSoldier->ubID, pOpponent->ubID, gbPublicOpplist[pSoldier->bTeam][pOpponent->ubID]));
			continue;	// next opponent
		}

		// sevenfm: blind soldier can only attack seen/heard personally
		if (pSoldier->bBlindedCounter > 0 &&
			pSoldier->aiData.bOppList[pOpponent->ubID] != SEEN_THIS_TURN &&
			pSoldier->aiData.bOppList[pOpponent->ubID] != HEARD_THIS_TURN)
		{
			continue;	// next opponent
		}

		// sevenfm: determine if we shoot on unseen target for suppression		
		if (bPersonalKnowledge != SEEN_CURRENTLY &&
			bPublicKnowledge != SEEN_CURRENTLY &&
			//!SoldierToSoldierLineOfSightTest(pSoldier, pOpponent, TRUE, CALC_FROM_ALL_DIRS))
			!LOS_Raised(pSoldier, pOpponent, CALC_FROM_ALL_DIRS))
		{
			fSuppression = TRUE;
		}

		// determine enemy location
		if (fSuppression)
		{
			// personal/public knowledge
			sTarget = KnownLocation(pSoldier, pOpponent->ubID);
			bLevel = KnownLevel(pSoldier, pOpponent->ubID);
			// try to randomize location
			sTarget = RandomizeLocation(sTarget, bLevel, 1, pSoldier);
			//DebugShot(pSoldier, String("randomize spot %d", sTarget));
		}
		else
		{
			// we know exact enemy location
			sTarget = pOpponent->sGridNo;
			bLevel = pOpponent->pathing.bLevel;
		}

		// safety check
		if (TileIsOutOfBounds(sTarget))
		{
			//DebugShot(pSoldier, String("enemy location unknown, skip soldier"));
			continue;
		}

		// hack fix
		if (sTarget == 0)
		{
			//DebugShot(pSoldier, String("sTarget = 0, skip soldier"));
			continue;
		}

		// skip if we can see location and location is empty
		if (SoldierToVirtualSoldierLineOfSightTest(pSoldier, sTarget, bLevel, ANIM_PRONE, TRUE, CALC_FROM_ALL_DIRS) &&
			WhoIsThere2(sTarget, bLevel) == NOBODY)
		{
			continue;
		}

		// no fire on unseen opponents with throwing knives
		if ((Item[pSoldier->usAttackingWeapon].usItemClass & IC_THROWING_KNIFE) &&
			bPersonalKnowledge != SEEN_CURRENTLY &&
			!SoldierToSoldierLineOfSightTest(pSoldier, pOpponent, TRUE, CALC_FROM_ALL_DIRS))
		{
			continue;
		}

		// sevenfm: don't allow blind suppression fire for militia
		if (fSuppression &&
			pSoldier->bTeam == MILITIA_TEAM &&
			!pSoldier->aiData.bUnderFire)
		{
			continue;
		}

		// sevenfm: only try to suppress alive and conscious human targets
		if (fSuppression &&
			Weapon[pSoldier->usAttackingWeapon].ubWeaponType != GUN_LMG &&
			(pOpponent->stats.bLife < OKLIFE ||
			pOpponent->bCollapsed && pOpponent->bBreath == 0 ||
			pOpponent->IsCowering() ||
			pOpponent->IsZombie() ||
			!IS_MERC_BODY_TYPE(pOpponent)))
		{
			continue;
		}

#ifdef DEBUGATTACKS
		DebugAI( String( "%s sees %s at gridno %d\n",pSoldier->GetName(),ExtMen[pOpponent->ubID].GetName(),pOpponent->sGridNo ) );
#endif
		ubMinAPcost = MinAPsToAttack(pSoldier, sTarget, DONTADDTURNCOST, 0);// later will be decide if shoot is possible this here is just best guess so ignore turnover

		// if we don't have enough APs left to shoot even a snap-shot at this guy
		if (ubMinAPcost > pSoldier->bActionPoints)
			continue;			// next opponent

		ubChanceToGetThrough = AISoldierToSoldierChanceToGetThrough(pSoldier, pOpponent);

		// if we can't possibly get through all the cover
		if (ubChanceToGetThrough == 0)
			continue;			// next opponent

		if ( (pSoldier->flags.uiStatusFlags & SOLDIER_MONSTER) && (pSoldier->ubBodyType != QUEENMONSTER ) )
		{
			STRUCTURE_FILE_REF *	pStructureFileRef;
			UINT16								usAnimSurface;

			usAnimSurface = DetermineSoldierAnimationSurface( pSoldier, pSoldier->usUIMovementMode );
			pStructureFileRef = GetAnimationStructureRef( pSoldier->ubID, usAnimSurface,pSoldier->usUIMovementMode	);

			if ( pStructureFileRef )
			{
				UINT16		usStructureID;
				INT8			bDir;

				// must make sure that structure data can be added in the direction of the target
				bDir = (INT8) GetDirectionToGridNoFromGridNo( pSoldier->sGridNo, pOpponent->sGridNo );

				// ATE: Only if we have a levelnode...
				if ( pSoldier->pLevelNode != NULL && pSoldier->pLevelNode->pStructureData != NULL )
				{
					usStructureID = pSoldier->pLevelNode->pStructureData->usStructureID;
				}
				else
				{
					usStructureID = INVALID_STRUCTURE_ID;
				}

				if ( ! OkayToAddStructureToWorld( pSoldier->sGridNo, pSoldier->pathing.bLevel, &(pStructureFileRef->pDBStructureRef[ gOneCDirection[ bDir ] ]), usStructureID ) )
				{
					// can't turn in that dir.... next opponent
					continue;
				}
			}
		}

		iBestHitRate = 0;					 // reset best hit rate to minimum
		//dnl ch69 130913 Hoping to optimize
		// consider alternate holding mode and different scopes
		// sevenfm: alt weapon holding scope mode is used only when ubAllowAlternativeWeaponHolding == 3
		for (pSoldier->bScopeMode = (gGameExternalOptions.ubAllowAlternativeWeaponHolding == 3 ? USE_ALT_WEAPON_HOLD : USE_BEST_SCOPE);
			pSoldier->bScopeMode <= (gGameExternalOptions.fScopeModes ? NUM_SCOPE_MODES - 1 : USE_BEST_SCOPE);
			pSoldier->bScopeMode++)
		{
			//dnl ch71 180913 throwing knives cannot be use in fire from hip, also SANDRO said: if the gun is flagged as HeavyGun, then we can only fire it from hip, thus no scopes to use at all, not even iron sights
			if(pSoldier->bScopeMode == USE_ALT_WEAPON_HOLD)
			{
				if(Item[pSoldier->usAttackingWeapon].usItemClass & IC_THROWING_KNIFE)
					continue;

				if (IS_MERC_BODY_TYPE(pSoldier))
					continue;
			}

			if(pSoldier->bScopeMode == USE_ALT_WEAPON_HOLD || (pSoldier->bScopeMode >= USE_BEST_SCOPE && ObjList[pSoldier->bScopeMode] != NULL))
			{
				usTrueState = pSoldier->usAnimState;// because is used in CalculateRaiseGunCost, CalcAimingLevelsAvailableWithAP, CalculateTurningCost
				iTrueLastTarget = pSoldier->sLastTarget;// because is used in MinAPsToShootOrStab

				// --------- Standing ---------
				ubStance = ANIM_STAND;
				// sevenfm: take into account direction when checking stance
				// sevenfm: shoot heavy guns in standing stance only when using hip fire
				if (pSoldier->InternalIsValidStance(AIDirection(pSoldier->sGridNo, sTarget), ubStance) &&
					(pSoldier->bScopeMode == USE_ALT_WEAPON_HOLD || !Weapon[pSoldier->usAttackingWeapon].HeavyGun || !Item[pSoldier->usAttackingWeapon].twohanded || !gGameExternalOptions.ubAllowAlternativeWeaponHolding))
				{
					sStanceAPcost = GetAPsToChangeStance(pSoldier, ubStance);
					if(sStanceAPcost)
					{
						// Going up so first is stance change then turnover, do animation change before APs calculation
						pSoldier->usAnimState = STANDING;
						pSoldier->sLastTarget = NOWHERE;
					}
					GetAPChargeForShootOrStabWRTGunRaises(pSoldier, sTarget, TRUE, &fAddingTurningCost, &fAddingRaiseGunCost, 0);
					usTurningCost = CalculateTurningCost(pSoldier, pSoldier->usAttackingWeapon, fAddingTurningCost);
					usRaiseGunCost = CalculateRaiseGunCost(pSoldier, fAddingRaiseGunCost, sTarget, 0);
					if(fAddingTurningCost && fAddingRaiseGunCost)//dnl ch71 180913
					{
						if(usRaiseGunCost > usTurningCost)
							usTurningCost = 0;
						else
							usRaiseGunCost = 0;
					}
					ubRawAPCost = MinAPsToShootOrStab(pSoldier, sTarget, 0, FALSE, 2);
					ubMinAPcost = ubRawAPCost + usTurningCost + sStanceAPcost + usRaiseGunCost;

					if (pSoldier->bActionPoints - ubMinAPcost >= 0)
					{
						// calc next attack's minimum shooting cost (excludes readying & turning & raise gun)
						ubMaxPossibleAimTime = CalcAimingLevelsAvailableWithAP(pSoldier, sTarget, pSoldier->bActionPoints - ubMinAPcost);

						// sevenfm: check CTGT and friendly fire chance for every stance
						gUnderFire.Clear();
						gUnderFire.Enable();
						ubChanceToGetThrough = AISoldierToSoldierChanceToGetThrough(pSoldier, pOpponent);
						ubFriendlyFireChance = gUnderFire.Chance(pSoldier->bTeam, pSoldier->bSide, TRUE);
						gUnderFire.Disable();

						// sevenfm: only use this stance if we can hit target and cannot hit friends
						if (ubChanceToGetThrough > 0 && ubFriendlyFireChance <= MIN_CHANCE_TO_ACCIDENTALLY_HIT_SOMEONE)
						{
							for (sAimTime = 0; sAimTime <= ubMaxPossibleAimTime; sAimTime++)
							{
								ubChanceToHit = AICalcChanceToHitGun(pSoldier, sTarget, sAimTime, AIM_SHOT_TORSO, bLevel, STANDING);
								sAimAPCost = CalcAPCostForAiming(pSoldier, sTarget, (INT8)sAimTime);
								iHitRate = ubChanceToHit * (pSoldier->bActionPoints - (ubMinAPcost - ubRawAPCost)) / (ubRawAPCost + sAimAPCost);

								// sevenfm: take into account CTGT for every stance
								if (iHitRate * ubChanceToGetThrough > iBestHitRate * ubBestChanceToGetThrough ||
									(Item[pSoldier->usAttackingWeapon].usItemClass & IC_THROWING_KNIFE) && ubChanceToHit > ubBestChanceToHit)// rather take best chance for throwing knives
								{
									iBestHitRate = iHitRate;
									ubBestAimTime = sAimTime;
									ubBestChanceToHit = ubChanceToHit;
									ubBestChanceToGetThrough = ubChanceToGetThrough;
									ubBestFriendlyFireChance = ubFriendlyFireChance;
									bScopeMode = pSoldier->bScopeMode;
									sBestAPcost = ubMinAPcost;
									ubBestStance = ubStance;
								}
							}
						}
					}
					pSoldier->usAnimState = usTrueState;
					pSoldier->sLastTarget = iTrueLastTarget;
				}

				// no crouched/prone if we are tank/using throwing knife/hip firing
				if(pSoldier->bScopeMode == USE_ALT_WEAPON_HOLD || TANK(pSoldier) || (Item[pSoldier->usAttackingWeapon].usItemClass & IC_THROWING_KNIFE))
					continue;

				// --------- Crouched ---------
				ubStance = ANIM_CROUCH;
				// sevenfm: take into account direction
				if (pSoldier->InternalIsValidStance(AIDirection(pSoldier->sGridNo, sTarget), ubStance))
				{
					// change stance then turn
					sStanceAPcost = GetAPsToChangeStance(pSoldier, ubStance);
					if (sStanceAPcost)
					{
						pSoldier->usAnimState = CROUCHING;
						pSoldier->sLastTarget = NOWHERE;
					}
					GetAPChargeForShootOrStabWRTGunRaises(pSoldier, sTarget, TRUE, &fAddingTurningCost, &fAddingRaiseGunCost, 0);
					usTurningCost = CalculateTurningCost(pSoldier, pSoldier->usAttackingWeapon, fAddingTurningCost);
					usRaiseGunCost = CalculateRaiseGunCost(pSoldier, fAddingRaiseGunCost, sTarget, 0);
					if (fAddingTurningCost && fAddingRaiseGunCost)//dnl ch71 180913
					{
						if (usRaiseGunCost > usTurningCost)
							usTurningCost = 0;
						else
							usRaiseGunCost = 0;
					}
					ubRawAPCost = MinAPsToShootOrStab(pSoldier, sTarget, 0, FALSE, 2);
					ubMinAPcost = ubRawAPCost + usTurningCost + sStanceAPcost + usRaiseGunCost;

					if (pSoldier->bActionPoints - ubMinAPcost >= 0)
					{
						ubMaxPossibleAimTime = CalcAimingLevelsAvailableWithAP(pSoldier, sTarget, pSoldier->bActionPoints - ubMinAPcost);

						// sevenfm: check CTGT and friendly fire chance for every stance
						gUnderFire.Clear();
						gUnderFire.Enable();
						ubChanceToGetThrough = AISoldierToSoldierChanceToGetThrough(pSoldier, pOpponent);
						ubFriendlyFireChance = gUnderFire.Chance(pSoldier->bTeam, pSoldier->bSide, TRUE);
						gUnderFire.Disable();

						// sevenfm: only use this stance if we can hit target and cannot hit friends
						if (ubChanceToGetThrough > 0 && ubFriendlyFireChance <= MIN_CHANCE_TO_ACCIDENTALLY_HIT_SOMEONE)
						{
							for (sAimTime = 0; sAimTime <= ubMaxPossibleAimTime; sAimTime++)
							{
								ubChanceToHit = AICalcChanceToHitGun(pSoldier, sTarget, sAimTime, AIM_SHOT_TORSO, bLevel, CROUCHING);
								sAimAPCost = CalcAPCostForAiming(pSoldier, sTarget, (INT8)sAimTime);
								iHitRate = ubChanceToHit * (pSoldier->bActionPoints - (ubMinAPcost - ubRawAPCost)) / (ubRawAPCost + sAimAPCost);

								// sevenfm: take into account CTGT for every stance
								if (iHitRate * ubChanceToGetThrough > iBestHitRate * ubBestChanceToGetThrough)
								{
									iBestHitRate = iHitRate;
									ubBestAimTime = sAimTime;
									ubBestChanceToHit = ubChanceToHit;
									ubBestChanceToGetThrough = ubChanceToGetThrough;
									ubBestFriendlyFireChance = ubFriendlyFireChance;
									bScopeMode = pSoldier->bScopeMode;
									sBestAPcost = ubMinAPcost;
									ubBestStance = ubStance;
								}
							}
						}
					}
					pSoldier->usAnimState = usTrueState;
					pSoldier->sLastTarget = iTrueLastTarget;
				}

				// no prone stance if we have to change direction and stance at the same time
				if (pSoldier->ubDirection != AIDirection(pSoldier->sGridNo, sTarget) &&
					gAnimControl[pSoldier->usAnimState].ubEndHeight > ANIM_PRONE)
				{
					continue;
				}

				// --------- Prone ---------
				ubStance = ANIM_PRONE;
				if (pSoldier->InternalIsValidStance(AIDirection(pSoldier->sGridNo, sTarget), ubStance))
				{
					sStanceAPcost = GetAPsToChangeStance(pSoldier, ubStance);
					if (sStanceAPcost)
					{
						pSoldier->usAnimState = PRONE;
						pSoldier->sLastTarget = NOWHERE;
					}
					GetAPChargeForShootOrStabWRTGunRaises(pSoldier, sTarget, TRUE, &fAddingTurningCost, &fAddingRaiseGunCost, 0);
					usTurningCost = CalculateTurningCost(pSoldier, pSoldier->usAttackingWeapon, fAddingTurningCost);
					usRaiseGunCost = CalculateRaiseGunCost(pSoldier, fAddingRaiseGunCost, sTarget, 0);
					ubRawAPCost = MinAPsToShootOrStab(pSoldier, sTarget, 0, FALSE, 2);
					ubMinAPcost = ubRawAPCost + usTurningCost + sStanceAPcost + usRaiseGunCost;

					if(pSoldier->bActionPoints-ubMinAPcost >= 0)
					{
						ubMaxPossibleAimTime = CalcAimingLevelsAvailableWithAP(pSoldier, sTarget, pSoldier->bActionPoints - ubMinAPcost);

						// sevenfm: check CTGT and friendly fire chance for every stance
						gUnderFire.Clear();
						gUnderFire.Enable();
						ubChanceToGetThrough = AISoldierToSoldierChanceToGetThrough(pSoldier, pOpponent);
						ubFriendlyFireChance = gUnderFire.Chance(pSoldier->bTeam, pSoldier->bSide, TRUE);
						gUnderFire.Disable();

						// sevenfm: only use this stance if we can hit target and cannot hit friends
						if (ubChanceToGetThrough > 0 && ubFriendlyFireChance <= MIN_CHANCE_TO_ACCIDENTALLY_HIT_SOMEONE)
						{
							for (sAimTime = 0; sAimTime <= ubMaxPossibleAimTime; sAimTime++)
							{
								ubChanceToHit = AICalcChanceToHitGun(pSoldier, sTarget, sAimTime, AIM_SHOT_TORSO, bLevel, PRONE);
								sAimAPCost = CalcAPCostForAiming(pSoldier, sTarget, (INT8)sAimTime);
								iHitRate = ubChanceToHit * (pSoldier->bActionPoints - (ubMinAPcost - ubRawAPCost)) / (ubRawAPCost + sAimAPCost);
								// sevenfm: take into account CTGT for every stance
								if (iHitRate * ubChanceToGetThrough > iBestHitRate * ubBestChanceToGetThrough)
								{
									iBestHitRate = iHitRate;
									ubBestAimTime = sAimTime;
									ubBestChanceToHit = ubChanceToHit;
									ubBestChanceToGetThrough = ubChanceToGetThrough;
									ubBestFriendlyFireChance = ubFriendlyFireChance;
									bScopeMode = pSoldier->bScopeMode;
									sBestAPcost = ubMinAPcost;
									ubBestStance = ubStance;
								}
							}
						}
					}
					pSoldier->usAnimState = usTrueState;
					pSoldier->sLastTarget = iTrueLastTarget;
				}
			}
		}

		// if we can't get any kind of hit rate at all
		if (iBestHitRate == 0)
			continue;			// next opponent

		// calculate chance to REALLY hit: shoot accurately AND get past cover
		ubChanceToReallyHit = (UINT8)ceil((ubBestChanceToHit * ubBestChanceToGetThrough) / 100.0f);

		// if we can't REALLY hit at all
		if (ubChanceToReallyHit == 0)
			continue;			// next opponent

		// really limit knife throwing so it doesn't look wrong
		if (Item[pSoldier->usAttackingWeapon].usItemClass == IC_THROWING_KNIFE && 
			(ubChanceToReallyHit < 25 || (PythSpacesAway(pSoldier->sGridNo, sTarget) > CalcMaxTossRange(pSoldier, pSoldier->usAttackingWeapon, FALSE))))// Madd / 2 ) ) ) //dnl ch69 160913 was ubChanceToReallyHit < 30
			continue; // don't bother... next opponent

		// calculate this opponent's threat value (factor in my cover from him)
		iThreatValue = CalcManThreatValue(pOpponent,pSoldier->sGridNo,TRUE,pSoldier);

		// estimate the damage this shot would do to this opponent
		iEstDamage = EstimateShotDamage(pSoldier,pOpponent,ubBestChanceToHit);
		//NumMessage("SHOT EstDamage = ",iEstDamage);

		// calculate the combined "attack value" for this opponent
		// highest possible value before division should be about 1.8 billion...
		// normal value before division should be about 5 million...
		iAttackValue = (iEstDamage * iBestHitRate * ubChanceToReallyHit * iThreatValue) / 1000;
		//NumMessage("SHOT AttackValue = ",iAttackValue / 1000);

		// sevenfm: take into account friendly fire chance
		if (ubBestFriendlyFireChance > 0)
		{
			//iAttackValue = iAttackValue * (100 - ubFriendlyFireChance) / 100;
			iAttackValue = iAttackValue / 2;
		}

		// sevenfm: penalize suppression fire
		if (fSuppression)
		{
			// 25% penalty for shooting at invisible target
			iAttackValue = iAttackValue / 2;
			//DebugShot(pSoldier, String("penalize suppression fire, iAttackValue = %d", iAttackValue));
		}

		// special stuff for assassins to ignore militia more
		if ( pSoldier->IsAssassin() && pOpponent->bTeam == MILITIA_TEAM )
		{
			iAttackValue /= 2;
		}

		// sevenfm: empty vehicles have very low priority
		if ( pOpponent->ubWhatKindOfMercAmI == MERC_TYPE__VEHICLE && GetNumberInVehicle( pOpponent->bVehicleID ) == 0 )
		{
			iAttackValue /= 4;
		}

		// sevenfm: dying, cowering or unconscious soldiers have very low priority
		if (pOpponent->stats.bLife < OKLIFE || pOpponent->bCollapsed && pSoldier->bBreath == 0)
		{
			iAttackValue /= 4;
		}

#ifdef DEBUGATTACKS
		DebugAI( String( "CalcBestShot: best AttackValue vs %d = %d\n",uiLoop,iAttackValue ) );
#endif

		// if we can hurt the guy, OR probably not, but at least it's our best
		// chance to actually hit him and maybe scare him, knock him down, etc.
		if ((iAttackValue > 0) || (ubChanceToReallyHit > pBestShot->ubChanceToReallyHit))
		{
			// if there already was another viable target
			if (pBestShot->ubChanceToReallyHit > 0)
			{
				// OK, how does our chance to hit him compare to the previous best one?
				iPercentBetter = ((ubChanceToReallyHit * 100) / pBestShot->ubChanceToReallyHit) - 100;

				//dnl ch62 180813 ignore firing into breathless targets if there are targets in better condition
				// sevenfm: check that best opponent exists
				if (pBestShot->ubOpponent != NOBODY &&
					(Menptr[pBestShot->ubOpponent].bCollapsed || Menptr[pBestShot->ubOpponent].bBreathCollapsed) &&
					Menptr[pBestShot->ubOpponent].bBreath < OKBREATH
					&& Menptr[pBestShot->ubOpponent].bBreath < pOpponent->bBreath)
				{
					iPercentBetter = PERCENT_TO_IGNORE_THREAT;
				}

				// sevenfm: if best opponent is dying and new opponent is ok, use new opponent
				if (pBestShot->ubOpponent != NOBODY &&
					Menptr[pBestShot->ubOpponent].stats.bLife < OKLIFE &&
					pOpponent->stats.bLife >= OKLIFE)
				{
					iPercentBetter = PERCENT_TO_IGNORE_THREAT;
				}

				// if this chance to really hit is more than 50% worse, and the other
				// guy is conscious at all
				if (iPercentBetter < -PERCENT_TO_IGNORE_THREAT &&
					pBestShot->ubOpponent != NOBODY &&
					Menptr[pBestShot->ubOpponent].stats.bLife >= OKLIFE)
				{
					// then stick with the older guy as the better target
					continue;
				}

				// if this chance to really hit between 50% worse to 50% better
				if (iPercentBetter < PERCENT_TO_IGNORE_THREAT)
				{
					// then the one with the higher ATTACK VALUE is the better target
					if (iAttackValue < pBestShot->iAttackValue)
						// the previous guy is more important since he's more dangerous
						continue;			// next opponent
				}
			}

			// sevenfm: if new opponent is dying and best opponent is ok, ignore new opponent
			if (pBestShot->ubOpponent != NOBODY &&
				Menptr[pBestShot->ubOpponent].stats.bLife >= OKLIFE &&
				pOpponent->stats.bLife < OKLIFE)
			{
				//DebugShot(pSoldier, String("new opponent is dying, best opponent is ok - skip"));
				continue;
			}

			// OOOF!	That was a lot of work!	But we've got a new best target!
			pBestShot->ubPossible			= TRUE;
			pBestShot->ubOpponent			= pOpponent->ubID;
			pBestShot->ubAimTime			= ubBestAimTime;
			pBestShot->ubChanceToReallyHit	= ubChanceToReallyHit;
			pBestShot->sTarget				= sTarget;
			pBestShot->bTargetLevel			= bLevel;
			pBestShot->iAttackValue			= iAttackValue;
			pBestShot->ubAPCost				= sBestAPcost;
			pBestShot->ubStance				= ubBestStance;
			pBestShot->bScopeMode			= bScopeMode;
			pBestShot->ubFriendlyFireChance = (UINT8)ubBestFriendlyFireChance;
		}
	}
//if(pBestShot->ubPossible)SendFmtMsg("CalcBestShot;\r\n  ID=%d Loc=%d APs=%d Ac=%d AcData=%d Al=%d, SM=%d, LAc=%d, NAc=%d AT=%d\r\n  AP?=%d,%d,%d/%d BS=%d", pSoldier->ubID, pSoldier->sGridNo, pSoldier->bActionPoints, pSoldier->aiData.bAction, pSoldier->aiData.usActionData, pSoldier->aiData.bAlertStatus, pBestShot->bScopeMode, pSoldier->aiData.bLastAction, pSoldier->aiData.bNextAction, pBestShot->ubAimTime, pBestShot->ubAPCost, CalcAPCostForAiming(pSoldier, pBestShot->sTarget, (INT8)pBestShot->ubAimTime), CalcTotalAPsToAttack(pSoldier, pBestShot->sTarget, TRUE, pBestShot->ubAimTime), CalcTotalAPsToAttack(pSoldier, pBestShot->sTarget, FALSE, pBestShot->ubAimTime), pBestShot->ubStance);
	pSoldier->bScopeMode = USE_BEST_SCOPE; // better reset this back
}

// JA2Gold: added
BOOLEAN CloseEnoughForGrenadeToss( INT32 sGridNo, INT32 sGridNo2 )
{
	INT32	sTempGridNo;
	UINT8	ubDirection;
	INT16	sXPos, sYPos, sXPos2, sYPos2;
	UINT8	ubMovementCost;

	if (sGridNo == sGridNo2 )
	{
		// checking the same space; if there is a closed door next to location in ANY direction then forget it
		// (could be the player closed a door on us)
		for (ubDirection = 0; ubDirection < NUM_WORLD_DIRECTIONS; ubDirection++)
		{
			sTempGridNo = NewGridNo( sGridNo, DirectionInc( ubDirection ) );
			ubMovementCost = gubWorldMovementCosts[ sTempGridNo ][ ubDirection ][ 0 ];
			if ( IS_TRAVELCOST_DOOR( ubMovementCost ) )
			{
				ubMovementCost = DoorTravelCost( NULL, sTempGridNo, ubMovementCost, FALSE, NULL );
			}
			if ( ubMovementCost >= TRAVELCOST_BLOCKED)
			{
				return( FALSE );
			}
		}
	}
	else
	{
		if ( CardinalSpacesAway( sGridNo, sGridNo2 ) > 2 )
		{
			return( FALSE );
		}

		// we are within 1 space diagonally or at most 2 horizontally or vertically,
		// so we can now do a loop safely

		sTempGridNo = sGridNo;

		sXPos = CenterX( sGridNo );
		sYPos = CenterY( sGridNo );
		sXPos2 = CenterX( sGridNo2 );
		sYPos2 = CenterY( sGridNo2 );
		ubDirection = atan8( sXPos, sYPos, sXPos2, sYPos2 );

		// For each step of the loop, we are checking for door or obstacle movement costs.	If we
		// find we're blocked, then this is no good for grenade tossing!
		do
		{
			sTempGridNo = NewGridNo( sTempGridNo, DirectionInc( ubDirection ) );
			ubMovementCost = gubWorldMovementCosts[ sTempGridNo ][ ubDirection ][ 0 ];
			if ( IS_TRAVELCOST_DOOR( ubMovementCost ) )
			{
				ubMovementCost = DoorTravelCost( NULL, sTempGridNo, ubMovementCost, FALSE, NULL );
			}
			if ( ubMovementCost >= TRAVELCOST_BLOCKED)
			{
				return( FALSE );
			}
		} while( sTempGridNo != sGridNo2 );
	}

	return( TRUE );
}

void CalcBestThrow(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestThrow)
{
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"calcbestthrow");
	// September 9, 1998: added code for LAWs (CJC)
	UINT8	ubLoop, ubLoop2;
	INT32	iAttackValue;
	INT32	iHitRate, iThreatValue, iTotalThreatValue,iOppThreatValue[MAXMERCS];
	INT32	sGridNo, sEndGridNo, sFriendTile[MAXMERCS], sOpponentTile[MAXMERCS];
	INT8	bFriendLevel[MAXMERCS], bOpponentLevel[MAXMERCS];
	INT32	iEstDamage;
	UINT8	ubFriendCnt = 0,ubOpponentCnt = 0, ubOpponentID[MAXMERCS];
	UINT8	ubMaxPossibleAimTime;
	INT16	ubRawAPCost, ubMinAPcost;
	UINT8	ubChanceToHit, ubChanceToGetThrough, ubChanceToReallyHit;
	UINT32	uiPenalty;
	UINT8	ubSearchRange;
	UINT16	usOppDist;
	BOOLEAN fFriendsNearby;
	UINT16	usInHand, usGrenade;
	UINT8	ubOppsInRange, ubOppsAdjacent;
	BOOLEAN fSkipLocation;
	INT8	bPayloadPocket;
	INT8	bMaxLeft,bMaxRight,bMaxUp,bMaxDown,bXOffset,bYOffset;
	INT8	bPersOL, bPublOL;
	SOLDIERTYPE *pOpponent, *pFriend;
	static INT16	sExcludeTile[100]; // This array is for storing tiles that we have
	UINT8	ubNumExcludedTiles = 0;		// already considered, to prevent duplication of effort
	INT32	iTossRange;
	UINT8	ubSafetyMargin = 0;
	UINT8	ubDiff;
	INT8	bEndLevel;
	OBJECTTYPE *pObjGL = NULL;//dnl ch63 240813

	usInHand = pSoldier->inv[HANDPOS].usItem;
	usGrenade = NOTHING;

	// sevenfm: initialize
	pBestThrow->ubPossible = FALSE;
	pBestThrow->ubChanceToReallyHit = 0;
	pBestThrow->iAttackValue = 0;

	if ( IsGrenadeLauncherAttached(&pSoldier->inv[HANDPOS]) )
		usInHand = GetAttachedGrenadeLauncher(&pSoldier->inv[HANDPOS]);

	if ( EXPLOSIVE_GUN( usInHand ) )
	{
		DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"explosive gun");
		iTossRange = GetModifiedGunRange(usInHand) / CELL_X_SIZE;
	}
	else
	{
		DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"about to calcmaxtossrange");
		iTossRange = CalcMaxTossRange( pSoldier, usInHand, TRUE );
	}

	// if he's got a MORTAR in his hand, make sure he has a MORTARSHELL avail.
	if (Item[usInHand].mortar )
	{
		DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"calcbestthrow: buddy's got a mortar");
		bPayloadPocket = FindNonSmokeLaunchable(pSoldier, usInHand);
		if (bPayloadPocket == NO_SLOT)
		{
			bPayloadPocket = FindLaunchable(pSoldier, usInHand);
		}
		if (bPayloadPocket == NO_SLOT)
		{
			return;	// no shells, can't fire the MORTAR
		}
		ubSafetyMargin = (UINT8)Explosive[ Item[ pSoldier->inv[bPayloadPocket].usItem ].ubClassIndex ].ubRadius;
	}
	// if he's got a GL in his hand, make sure he has some type of GRENADE avail.
	else if (Item[usInHand].grenadelauncher )
	{
		// use up pocket 2 first, they get left as drop items
		DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"calcbestthrow: buddy's got a GL");
		//dnl ch63 240813 Check if grenade is already attach or find one in pockets
		bPayloadPocket = HANDPOS;
		pObjGL = FindAttachment_GrenadeLauncher(&pSoldier->inv[bPayloadPocket]);
		OBJECTTYPE *pAttachment = FindLaunchableAttachment(&pSoldier->inv[bPayloadPocket], usInHand);
		if(pAttachment->exists())
		{
			ubSafetyMargin = (UINT8)Explosive[Item[pAttachment->usItem].ubClassIndex].ubRadius;
			usGrenade = pAttachment->usItem;
		}
		else if((bPayloadPocket=FindAmmoToReload(pSoldier, bPayloadPocket, NO_SLOT)) != NO_SLOT)
		{
			ubSafetyMargin = (UINT8)Explosive[Item[pSoldier->inv[bPayloadPocket].usItem].ubClassIndex].ubRadius;
			usGrenade = pSoldier->inv[bPayloadPocket].usItem;
		}
		else
			return;
	}
	else if ( Item[usInHand].rocketlauncher )
	{
		DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"calcbestthrow: buddy's got a rocket launcher");
		// put in hand
		bPayloadPocket = HANDPOS;//dnl ch63 240813
		if (Item[usInHand].singleshotrocketlauncher)
		{
			// sevenfm: for single shot rocket launchers, use buddy item instead
			if (Item[usInHand].usBuddyItem && Item[Item[usInHand].usBuddyItem].usItemClass & IC_EXPLOSV)
			{
				usGrenade = Item[usInHand].usBuddyItem;
				ubSafetyMargin = (UINT8)Explosive[Item[Item[usInHand].usBuddyItem].ubClassIndex].ubRadius;
			}
			else
			{
				// as C1
				usGrenade = C1;
				ubSafetyMargin = (UINT8)Explosive[Item[C1].ubClassIndex].ubRadius;
			}
		}
		else
		{
			bPayloadPocket = FindNonSmokeLaunchable(pSoldier, usInHand);
			if (bPayloadPocket == NO_SLOT)
			{
				bPayloadPocket = FindLaunchable(pSoldier, usInHand);
			}
			if (bPayloadPocket == NO_SLOT)
			{
				return;	// no ammo, can't fire
			}
			ubSafetyMargin = (UINT8)Explosive[ Item[ pSoldier->inv[bPayloadPocket].usItem ].ubClassIndex ].ubRadius;
		}
	}
	else if (Item[usInHand].cannon )
	{
		DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"calcbestthrow: buddy's got a tank");
		bPayloadPocket = FindNonSmokeLaunchable(pSoldier, usInHand);
		if (bPayloadPocket == NO_SLOT)
		{
			bPayloadPocket = FindLaunchable(pSoldier, usInHand);
		}
		if (bPayloadPocket == NO_SLOT)
		{
			return;	// no ammo, can't fire
		}
		ubSafetyMargin = (UINT8)Explosive[ Item[ pSoldier->inv[bPayloadPocket].usItem ].ubClassIndex ].ubRadius;
	}
	else
	{
		// else it's a plain old grenade, now in his hand
		DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"calcbestthrow: buddy's got a grenade");
		bPayloadPocket = HANDPOS;
		ubSafetyMargin = (UINT8)Explosive[ Item[ pSoldier->inv[ bPayloadPocket ].usItem ].ubClassIndex ].ubRadius;
		usGrenade = pSoldier->inv[ bPayloadPocket ].usItem;

		if ( Item[usGrenade].flare )
		{
			// JA2Gold: light isn't as nasty as explosives
			ubSafetyMargin /= 2;
		}
	}

	// sevenfm: limit ubSafetyMargin in case it is set too high in XML
	ubSafetyMargin = min(ubSafetyMargin, TACTICAL_RANGE / 2);

	ubDiff = SoldierDifficultyLevel( pSoldier );

	// make a list of tiles one's friends are positioned in
	for (ubLoop = 0; ubLoop < guiNumMercSlots; ubLoop++)
	{

		pFriend = MercSlots[ubLoop];

		if ( !pFriend )
		{
			continue; // next soldier
		}

		if (pFriend->stats.bLife == 0)
		{
			continue;
		}

		/*
		// if this soldier is inactive, at base, on assignment, or dead
		if (!Menptr[ubLoop].bActive || !Menptr[ubLoop].bInSector || !Menptr[ubLoop].bLife)
		continue;			// next soldier
		*/

		// if this man is neutral / NOT on the same side, he's not a friend
		if (pFriend->aiData.bNeutral || (pSoldier->bSide != pFriend->bSide))
		{
			continue;			// next soldier
		}

		// active friend, remember where he is so that we DON'T blow him up!
		// this includes US, since we don't want to blow OURSELVES up either
		sFriendTile[ubFriendCnt] = pFriend->sGridNo;
		bFriendLevel[ubFriendCnt] = pFriend->pathing.bLevel;
		ubFriendCnt++;
	}

	//NumMessage("ubFriendCnt = ",ubFriendCnt);

	// make a list of tiles one's CURRENTLY SEEN opponents are positioned in
	for (ubLoop = 0; ubLoop < guiNumMercSlots; ubLoop++)
	{
		pOpponent = MercSlots[ubLoop];

		if (!pOpponent)
		{
			// inactive or not in sector
			continue;			// next soldier
		}

		if (!pOpponent->stats.bLife)
		{
			continue;			// next soldier
		}

		/*
		// if this soldier is inactive, at base, on assignment, or dead
		if (!pOpponent->bActive || !pOpponent->bInSector || !pOpponent->stats.bLife)
		continue;			// next soldier
		*/

		bPersOL = pSoldier->aiData.bOppList[pOpponent->ubID];
		bPublOL = gbPublicOpplist[pSoldier->bTeam][pOpponent->ubID];

		// we know nothing about this opponent
		if (bPersOL == NOT_HEARD_OR_SEEN && bPublOL == NOT_HEARD_OR_SEEN)
		{
			continue;
		}

		if (!ValidOpponent(pSoldier, pOpponent))
		{
			continue;
		}

		// sevenfm: additional restrictions

		// blinded soldier can only attack recently seen/heard opponents
		if (pSoldier->bBlindedCounter > 0 &&
			bPersOL != SEEN_CURRENTLY &&
			bPersOL != SEEN_THIS_TURN &&
			bPersOL != HEARD_THIS_TURN)
		{
			continue;
		}

		// don't use gas/flashbang against zombies
		if (usGrenade != NOTHING &&
			!Item[usGrenade].flare &&
			Explosive[Item[usGrenade].ubClassIndex].ubType != EXPLOSV_NORMAL &&
			Explosive[Item[usGrenade].ubClassIndex].ubType != EXPLOSV_CREATUREGAS &&
			Explosive[Item[usGrenade].ubClassIndex].ubType != EXPLOSV_BURNABLEGAS &&
			pOpponent->IsZombie())
		{
			continue;
		}

		// limit smoke grenade use
		if (usGrenade != NOTHING &&
			Explosive[Item[usGrenade].ubClassIndex].ubType == EXPLOSV_SMOKE &&
			(!AICheckHasGun(pOpponent) ||
			(pSoldier->usAnimState == COWERING || pSoldier->usAnimState == COWERING_PRONE) ||
			ShockLevelPercent(pOpponent) > 50 ||
			EffectiveMarksmanship(pOpponent) < 90 && !AICheckIsSniper(pOpponent) && !pOpponent->aiData.bLastAttackHit && !AICheckIsMachinegunner(pOpponent) && !pOpponent->IsSpotting()))
		{
			continue;
		}

		// only normal type explosives against robots, vehicles and tanks
		if (usGrenade != NOTHING &&
			!Item[usGrenade].flare &&
			Explosive[Item[usGrenade].ubClassIndex].ubType != EXPLOSV_NORMAL &&
			Explosive[Item[usGrenade].ubClassIndex].ubType != EXPLOSV_CREATUREGAS &&
			Explosive[Item[usGrenade].ubClassIndex].ubType != EXPLOSV_BURNABLEGAS &&
			(TANK(pOpponent) || pOpponent->flags.uiStatusFlags & SOLDIER_VEHICLE || AM_A_ROBOT(pOpponent)))
		{
			continue;
		}

		// don't use grenades against dying enemies
		if (pOpponent->stats.bLife < OKLIFE && !pOpponent->IsZombie())
		{
			continue;
		}

		// don't use stun/gas grenades against collapsed enemies
		if (usGrenade != NOTHING &&
			!Item[usGrenade].flare &&
			!pOpponent->IsZombie() &&
			//Explosive[Item[usGrenade].ubClassIndex].ubType != EXPLOSV_NORMAL &&
			//Explosive[Item[usGrenade].ubClassIndex].ubType != EXPLOSV_CREATUREGAS &&
			//Explosive[Item[usGrenade].ubClassIndex].ubType != EXPLOSV_BURNABLEGAS &&
			//Explosive[Item[usGrenade].ubClassIndex].ubType != EXPLOSV_MUSTGAS &&
			(pOpponent->bCollapsed || pOpponent->bBreathCollapsed))
		{
			continue;
		}

		// don't use flare if soldier is in light
		if (usGrenade != NOTHING &&
			Item[usGrenade].flare &&
			InLightAtNight(pOpponent->sGridNo, pOpponent->pathing.bLevel))
		{
			continue;
		}

		// don't use flares against opponents on roof
		if (usGrenade != NOTHING &&
			Item[usGrenade].flare &&
			pOpponent->pathing.bLevel > 0)
		{
			continue;
		}

		if ((Item[usInHand].mortar ) || (Item[usInHand].grenadelauncher ) )
		{
			// allow long range firing, where target doesn't PERSONALLY see opponent
			if ((bPersOL != SEEN_CURRENTLY) && (bPublOL != SEEN_CURRENTLY))
			{
				continue;			// next soldier
			}
			// active KNOWN opponent, remember where he is so that we DO blow him up!
			sOpponentTile[ubOpponentCnt] = pOpponent->sGridNo;
			bOpponentLevel[ubOpponentCnt] = pOpponent->pathing.bLevel;
		}
		else
		{
			if (bPersOL == SEEN_CURRENTLY)
			{
				// active KNOWN opponent, remember where he is so that we DO blow him up!
				sOpponentTile[ubOpponentCnt] = pOpponent->sGridNo;
				bOpponentLevel[ubOpponentCnt] = pOpponent->pathing.bLevel;
			}
			else if (bPersOL == SEEN_LAST_TURN)
			{
				if ( !CloseEnoughForGrenadeToss( pOpponent->sGridNo, gsLastKnownOppLoc[ pSoldier->ubID ][ pOpponent->ubID ] ) )
				{
					continue;
				}

				sOpponentTile[ubOpponentCnt] = gsLastKnownOppLoc[ pSoldier->ubID ][ pOpponent->ubID ];
				bOpponentLevel[ubOpponentCnt] = gbLastKnownOppLevel[ pSoldier->ubID ][ pOpponent->ubID ];
			}
			else if (bPersOL == HEARD_LAST_TURN)
			{
				if ( !CloseEnoughForGrenadeToss( pOpponent->sGridNo, gsLastKnownOppLoc[ pSoldier->ubID ][ pOpponent->ubID ] ) )
				{
					continue;
				}
				
				// sevenfm: allow using of non-lethal grenades to attack heard opponents
				BOOLEAN fSkipGrenade = TRUE;
				if( Item[usGrenade].flare )
				{
					fSkipGrenade = FALSE;
				}
				if( usGrenade != NOTHING &&
					pSoldier->aiData.bAlertStatus >= STATUS_RED &&
					( Explosive[Item[usGrenade].ubClassIndex].ubType == EXPLOSV_STUN ||
					Explosive[Item[usGrenade].ubClassIndex].ubType == EXPLOSV_TEARGAS ||
					Explosive[Item[usGrenade].ubClassIndex].ubType == EXPLOSV_FLASHBANG && NightTime() ) )
				{
					fSkipGrenade = FALSE;
				}
				if( fSkipGrenade && !pSoldier->aiData.bUnderFire )
				{
					continue;
				}
				/*if ( !Item[usGrenade].flare && !pSoldier->aiData.bUnderFire && pSoldier->aiData.bShock == 0 )
				{
					continue;
				}*/
				sOpponentTile[ubOpponentCnt] = gsLastKnownOppLoc[ pSoldier->ubID ][ pOpponent->ubID ];
				bOpponentLevel[ubOpponentCnt] = gbLastKnownOppLevel[ pSoldier->ubID ][ pOpponent->ubID ];
			}
			else
			{
				continue;
			}

		}

		// also remember who he is (which soldier #)
		ubOpponentID[ubOpponentCnt] = pOpponent->ubID;

		// remember how relatively dangerous this opponent is (ignore my cover)
		iOppThreatValue[ubOpponentCnt] = CalcManThreatValue(pOpponent,pSoldier->sGridNo,FALSE,pSoldier);

		ubOpponentCnt++;
	}

	//NumMessage("ubOpponentCnt = ",ubOpponentCnt);


	// this is try to minimize enemies wasting their (limited) toss attacks, with the exception of break lights
	// sevenfm: don't spare non-lethal grenades
	BOOLEAN fSpare = TRUE;
	if( Item[usGrenade].flare )
	{
		fSpare = FALSE;
	}
	if( usGrenade != NOTHING &&
		(Explosive[Item[usGrenade].ubClassIndex].ubType == EXPLOSV_STUN ||
		Explosive[Item[usGrenade].ubClassIndex].ubType == EXPLOSV_TEARGAS ||
		Explosive[Item[usGrenade].ubClassIndex].ubType == EXPLOSV_FLASHBANG && NightTime() ) )
	{
		fSpare = FALSE;
	}
	// spare only when soldier is not under attack
	// need 1/2 health for 0 difficulty, 5/6 health for max difficulty
	// need 3 opponents for 0 difficulty, 1 opponent for max difficulty
	if ( fSpare &&				
		ubOpponentCnt < 3 - ubDiff / 2 &&
		!(pSoldier->aiData.bUnderFire &&
			pSoldier->stats.bLife < (ubDiff + 1) * pSoldier->stats.bLifeMax / (ubDiff + 2) ) )
	{
		return;
	}
	/*{
		switch( ubDiff )
		{
		case 0:
			if (ubOpponentCnt < 2 )
			{
				return;
			}
			break;
		case 1:
			// they won't use them until they have 2+ opponents as long as half life left
			//if ((ubOpponentCnt < 2) && (pSoldier->stats.bLife > (pSoldier->stats.bLifeMax / 2)))
			if (ubOpponentCnt < 2)
			{
				return;
			}
			break;
		case 2:
			// they won't use them until they have 2+ opponents as long as 3/4 life left
			//if ((ubOpponentCnt < 2) && (pSoldier->stats.bLife > (pSoldier->stats.bLifeMax / 4) * 3 ))
			if (ubOpponentCnt < 2)
			{
				return;
			}
			break;
		default:
			break;
		}
	}*/

	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"calcbestthrow: about to initattacktype");
	//InitAttackType(pBestThrow);	 // set all structure fields to defaults//dnl ch69 150913

	// look at the squares near each known opponent and try to find the one
	// place where a tossed projectile would do the most harm to the opponents
	// while avoiding one's friends
	for (ubLoop = 0; ubLoop < ubOpponentCnt; ubLoop++)
	{
		//NumMessage("Checking Guy#",ubOpponentID[ubLoop]);

		// search all tiles within 2 squares of this opponent
		ubSearchRange = MAX_TOSS_SEARCH_DIST;
		// sevenfm: increase possible distance from opponent when opponent in a building
		if( gpWorldLevelData[sOpponentTile[ubLoop]].ubTerrainID == FLAT_FLOOR )
		{
			ubSearchRange++;
		}
		// sevenfm: increase possible distance when throwing gas grenades
		if( usGrenade != NOTHING &&
			(Explosive[Item[usGrenade].ubClassIndex].ubType == EXPLOSV_TEARGAS ||
			Explosive[Item[usGrenade].ubClassIndex].ubType == EXPLOSV_MUSTGAS ) )
		{
			ubSearchRange++;
		}

		// increase possible distance when throwing flares		
		if( Item[usGrenade].flare )
		{
			ubSearchRange++;
		}

		// determine maximum horizontal limits
		//bMaxLeft	= min(ubSearchRange,(sOpponentTile[ubLoop] % MAXCOL));
		bMaxLeft = ubSearchRange;
		//NumMessage("bMaxLeft = ",bMaxLeft);
		//bMaxRight = min(ubSearchRange,MAXCOL - ((sOpponentTile[ubLoop] % MAXCOL) + 1));
		bMaxRight = ubSearchRange;
		//NumMessage("bMaxRight = ",bMaxRight);

		// determine maximum vertical limits
		bMaxUp	= ubSearchRange;
		//NumMessage("bMaxUp = ",bMaxUp);
		bMaxDown = ubSearchRange;
		//NumMessage("bMaxDown = ",bMaxDown);

		// evaluate every tile for its opponent-damaging potential
		for (bYOffset = -bMaxUp; bYOffset <= bMaxDown; bYOffset++)
		{
			for (bXOffset = -bMaxLeft; bXOffset <= bMaxRight; bXOffset++)
			{
				//HandleMyMouseCursor(KEYBOARDALSO);

				// calculate the next potential gridno near this opponent
				sGridNo = sOpponentTile[ubLoop] + bXOffset + (MAXCOL * bYOffset);
				//NumMessage("Testing gridno #",sGridNo);

				// this shouldn't ever happen
				if ((sGridNo < 0) || (sGridNo >= GRIDSIZE))
				{
#ifdef BETAVERSION
					NumMessage("CalcBestThrow: ERROR - invalid gridno being tested ",sGridNo);
#endif
					continue;
				}

				// sevenfm: avoid attacking spots outside of visible world to prevent possible bugs
				if (!GridNoOnVisibleWorldTile(sGridNo))
				{
					continue;
				}

				if ( PythSpacesAway( pSoldier->sGridNo, sGridNo ) > iTossRange )
				{
					// can't throw there!
					continue;
				}

				// if considering a gas/smoke grenade, check to see if there is such stuff already there!
				// sevenfm: check that only gas grenades are limited
				if ( usGrenade &&
					( Explosive[Item[usGrenade].ubClassIndex].ubType == EXPLOSV_TEARGAS ||
					Explosive[Item[usGrenade].ubClassIndex].ubType == EXPLOSV_MUSTGAS ||
					Explosive[Item[usGrenade].ubClassIndex].ubType == EXPLOSV_BURNABLEGAS ||
					Explosive[Item[usGrenade].ubClassIndex].ubType == EXPLOSV_SMOKE ))
				{
					if ( gpWorldLevelData[ sGridNo ].ubExtFlags[ bOpponentLevel[ubLoop] ] & MAPELEMENT_EXT_SMOKE ||
						gpWorldLevelData[ sGridNo ].ubExtFlags[ bOpponentLevel[ubLoop] ] & (MAPELEMENT_EXT_TEARGAS | MAPELEMENT_EXT_MUSTARDGAS | MAPELEMENT_EXT_BURNABLEGAS) ||
						gpWorldLevelData[ sGridNo ].ubExtFlags[ bOpponentLevel[ubLoop] ] & MAPELEMENT_EXT_MUSTARDGAS ||
						gpWorldLevelData[ sGridNo ].ubExtFlags[ bOpponentLevel[ubLoop] ] & MAPELEMENT_EXT_BURNABLEGAS)
					{
						continue;
					}
				}

				// sevenfm: skip tile if target is in a room and we throw grenade outside
				if( gpWorldLevelData[sOpponentTile[ubLoop]].ubTerrainID == FLAT_FLOOR &&
					gpWorldLevelData[sGridNo].ubTerrainID != FLAT_FLOOR )
				{
					continue;
				}

				fSkipLocation = FALSE;
				// Check to see if we have considered this tile before:
				for (ubLoop2 = 0; ubLoop2 < ubNumExcludedTiles; ubLoop2++)
				{
					if (sExcludeTile[ubLoop2] == sGridNo)
					{
						// already checked!
						fSkipLocation = TRUE;
						break;
					}
				}
				if (fSkipLocation)
				{
					continue;
				}

				// calculate minimum action points required to throw at this gridno
				ubMinAPcost = MinAPsToAttack(pSoldier,sGridNo,ADDTURNCOST,0);
				DebugMsg(TOPIC_JA2 , DBG_LEVEL_3 , String("MinAPcost to attack = %d",ubMinAPcost));

				// if we don't have enough APs left to throw even without aiming
				DebugMsg(TOPIC_JA2 , DBG_LEVEL_3 , String("Soldier's action points = %d",pSoldier->bActionPoints ));
				if (ubMinAPcost > pSoldier->bActionPoints)
					continue;				// next gridno

				// check whether there are any friends standing near this gridno
				fFriendsNearby = FALSE;

				for (ubLoop2 = 0; ubLoop2 < ubFriendCnt; ubLoop2++)
				{
					if ( (bFriendLevel[ubLoop2] == bOpponentLevel[ubLoop]) && ( PythSpacesAway(sFriendTile[ubLoop2],sGridNo) <= ubSafetyMargin ) )
					{
						//NumMessage("Friend too close: at gridno",sFriendTile[ubLoop2]);
						fFriendsNearby = TRUE;
						break;		// don't bother checking any other friends
					}
				}

				if (fFriendsNearby)
					continue;		// this location is no good, move along now

				// Well this place shows some promise, evaluate its "damage potential"
				iTotalThreatValue = 0;
				ubOppsInRange = 0;
				ubOppsAdjacent = 0;
				// skip this location unless it's right on top of an enemy or
				// adjacent to more than 1
				fSkipLocation = TRUE;

				DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"calcbestthrow: checking opponents");
				for (ubLoop2 = 0; ubLoop2 < ubOpponentCnt; ubLoop2++)
				{
					usOppDist = PythSpacesAway( sOpponentTile[ubLoop2], sGridNo );

					// if this opponent is close enough to the target gridno
					if (usOppDist <= 3)
					{
						// start with this opponents base threat value
						iThreatValue = iOppThreatValue[ubLoop2];

						// estimate how much damage this tossed item would do to him
						iEstDamage = EstimateThrowDamage(pSoldier,bPayloadPocket,MercPtrs[ubOpponentID[ubLoop2]],sGridNo);
						//NumMessage("THROW EstDamage = ",iEstDamage);

						if (usOppDist)
						{
							// reduce the estimated damage for his distance from gridno
							// use 100% at range 0, 80% at range 1, and 60% at range 2, etc.
							iEstDamage = (iEstDamage * (100 - (20 * usOppDist))) / 100;
							//NumMessage("THROW reduced usEstDamage = ",usEstDamage);
						}
						else
						{
							// throwing right on top of someone... always consider this
							fSkipLocation = FALSE;
						}

						// add the product of his threat value & damage caused to total
						iTotalThreatValue += (iThreatValue * iEstDamage);

						// only count opponents still standing worth shooting at (in range)
						if (Menptr[ ubOpponentID[ubLoop2] ].stats.bLife >= OKLIFE)
						{
							ubOppsInRange++;
							if (usOppDist < 2)
							{
								ubOppsAdjacent++;
								if (ubOppsAdjacent > 1 || Item[usGrenade].flare )
								{
									fSkipLocation = FALSE;
									// add to exclusion list so we don't consider it again
								}
							}
						}
					}
				}

				// JA2Gold
				if( gGameOptions.ubDifficultyLevel == DIF_LEVEL_EASY )
				{
					if (fSkipLocation)
					{
						continue;
					}
				}
				else
				{
					if( ubOppsInRange == 0 )
					{
						continue;
					}

					//Only use it if we are in a surface sector ( basement will be hard enough, plus more chances of mercs being clumped together )
					else if( gbWorldSectorZ > 0 && fSkipLocation )
					{
						continue;
					}
				}

				// JA2Gold change to >=
				if (ubOppsAdjacent >= 1 && ubNumExcludedTiles < 100)
				{
					// add to exclusion list so we don't calculate for this location twice
					sExcludeTile[ubNumExcludedTiles] = sGridNo;
					ubNumExcludedTiles++;
				}

				// calculate chance to get through any cover to this gridno
				//ubChanceToGetThrough = ChanceToGetThrough(pSoldier,sGridNo,NOTFAKE,ACTUAL,TESTWALLS,9999,M9PISTOL,NOT_FOR_LOS);

				if ( EXPLOSIVE_GUN( usInHand ) )
				{
					ubChanceToGetThrough = AISoldierToLocationChanceToGetThrough( pSoldier, sGridNo, bOpponentLevel[ubLoop], 0 );
					// anv: tanks shouldn't care about chance to get through - can't hit? At least we'll destroy their cover.
					// sevenfm: elites use rocket launchers to blow up obstacles when shooting at soldiers in buildings
					if( TANK(pSoldier) || 
						( Item[usInHand].rocketlauncher &&
						gpWorldLevelData[sOpponentTile[ubLoop]].ubTerrainID == FLAT_FLOOR &&
						gpWorldLevelData[pSoldier->sGridNo].ubTerrainID != FLAT_FLOOR &&
						pSoldier->ubSoldierClass == SOLDIER_CLASS_ELITE ) )
					{
						ubChanceToGetThrough = 100;
					}
					if ( ubChanceToGetThrough == 0)
					{
						continue; // next gridno
					}
				}
				else
				{
					DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"calcbestthrow: checking chance for launcher to beat cover");
					ubChanceToGetThrough = 100 * CalculateLaunchItemChanceToGetThrough( pSoldier, (pObjGL ? pObjGL : &pSoldier->inv[bPayloadPocket]), sGridNo, bOpponentLevel[ubLoop], 0, &sEndGridNo, TRUE, &bEndLevel, FALSE );//dnl ch63 240813
					//NumMessage("Chance to get through = ",ubChanceToGetThrough);
					// if we can't possibly get through all the cover
					if (ubChanceToGetThrough == 0 )
					{
						if ( bEndLevel == bOpponentLevel[ubLoop] && ubSafetyMargin > 1 )
						{
							// rate "chance of hitting" according to how far away this is from the target
							// but keeping in mind that we don't want to hit far, subtract 1 from the radius here
							// to penalize being far from the target
							uiPenalty = 100 * PythSpacesAway( sGridNo, sEndGridNo ) / (ubSafetyMargin - 1);
							if ( uiPenalty < 100 )
							{
								ubChanceToGetThrough = 100 - (UINT8) uiPenalty;
							}
							else
							{
								continue;
							}
						}
						else
						{
							continue;
						}
					}
				}

				//NumMessage("Total Threat Value = ",iTotalThreatValue);
				//NumMessage("Opps in Range = ",ubOppsInRange);

				// this is try to minimize enemies wasting their (few) mortar shells or LAWs
				// they won't use them on less than 2 targets as long as half life left
				if ((Item[usInHand].mortar || Item[usInHand].rocketlauncher ) && (ubOppsInRange < 2) &&
					(pSoldier->stats.bLife > (pSoldier->stats.bLifeMax / 2)))
				{
					continue;				// next gridno
				}

				// calculate the maximum possible aiming time
				// HEADROCK HAM 4: Required for new Aiming Level Limits function
				ubMaxPossibleAimTime = CalcAimingLevelsAvailableWithAP(pSoldier, sGridNo, pSoldier->bActionPoints-ubMinAPcost);//dnl ch63 250813
				DebugMsg(TOPIC_JA2 , DBG_LEVEL_3 , String("Max Possible Aim Time = %d",ubMaxPossibleAimTime ));

				// calc next attack's minimum AP cost (excludes readying & turning)

				// since grenades & shells are far too valuable to waste, ALWAYS
				// aim for the longest time possible!

				DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"calcbestthrow: checking chance to hit");
				if ( EXPLOSIVE_GUN( usInHand ) )
				{
					ubRawAPCost = MinAPsToShootOrStab( pSoldier, sGridNo,ubMaxPossibleAimTime,FALSE);
					ubChanceToHit = (UINT8) AICalcChanceToHitGun(pSoldier, sGridNo, ubMaxPossibleAimTime, AIM_SHOT_TORSO, pOpponent->pathing.bLevel, STANDING);//dnl ch59 130813
//SendFmtMsg("CalcBestThrow=%d APs=%d mat=%d gno=%d  EXPGUN!!!", ubChanceToHit, ubRawAPCost, ubMaxPossibleAimTime, sGridNo);
				}
				else
				{
					ubMaxPossibleAimTime = (UINT8)APBPConstants[AP_MIN_AIM_ATTACK];//dnl ch63 240813
					// NB grenade launcher is NOT a direct fire weapon!
					ubRawAPCost = (UINT8) MinAPsToThrow( pSoldier, sGridNo, FALSE );
					DebugMsg(TOPIC_JA2 , DBG_LEVEL_3 , String("Raw AP Cost = %d",ubRawAPCost ));
					ubChanceToHit = (UINT8) CalcThrownChanceToHit( pSoldier, sGridNo, ubMaxPossibleAimTime, AIM_SHOT_TORSO );
					DebugMsg(TOPIC_JA2 , DBG_LEVEL_3 , String("Chance to hit = %d",ubChanceToHit ));
//SendFmtMsg("CalcBestThrow=%d APs=%d mat=%d gno=%d", ubChanceToHit, ubRawAPCost, ubMaxPossibleAimTime, sGridNo);
				}

				// mortars are inherently quite inaccurate, don't get proximity bonus
				// rockets can go right past people too so...
				if (!Item[usInHand].mortar && EXPLOSIVE_GUN( usInHand ) )
				{
					// special 50% to Hit bonus: this reflects that even if a tossed item
					// misses by a bit, it's still likely to affect the intended target(s)
					ubChanceToHit += (ubChanceToHit / 2);

					// still can't let it go over 100% chance, though
					if (ubChanceToHit > 100)
					{
						ubChanceToHit = 100;
					}
				}

				if (ubRawAPCost < 1)
					ubRawAPCost = ubMinAPcost;

				DebugMsg (TOPIC_JA2,DBG_LEVEL_3,String("calcbestthrow: checking hit rate: ubRawAPCost %d, ubMaxPossibleAimTime %d", ubRawAPCost, ubMaxPossibleAimTime ));
				iHitRate = (pSoldier->bActionPoints * ubChanceToHit) / (ubRawAPCost + ubMaxPossibleAimTime);
				DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"calcbestthrow: checked hit rate");
				//NumMessage("iHitRate = ",iHitRate);

				// calculate chance to REALLY hit: throw accurately AND get past cover
				ubChanceToReallyHit = (ubChanceToHit * ubChanceToGetThrough) / 100;

				// if we can't REALLY hit at all
				if (ubChanceToReallyHit == 0)
					continue;				// next gridno

				// calculate the combined "attack value" for this opponent
				// maximum possible attack value here should be about 140 million
				// typical attack value here should be about 500 thousand
				DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"calcbestthrow: checking attack value");
				iAttackValue = (iHitRate * ubChanceToReallyHit * iTotalThreatValue) / 1000;
				//NumMessage("THROW AttackValue = ",iAttackValue / 1000);

				// unlike SHOOTing and STABbing, find strictly the highest attackValue
				if (iAttackValue > pBestThrow->iAttackValue)
				{
#ifdef DEBUGATTACKS
					DebugAI( String( "CalcBestThrow: new best attackValue vs %d = %d\n",ubOpponentID[ubLoop],iAttackValue ) );
#endif

					// OOOF!	That was a lot of work!	But we've got a new best target!
					pBestThrow->ubPossible			= TRUE;
					pBestThrow->ubOpponent			= ubOpponentID[ubLoop];
					pBestThrow->ubAimTime			= ubMaxPossibleAimTime;
					pBestThrow->ubChanceToReallyHit = ubChanceToReallyHit;
					pBestThrow->sTarget				= sGridNo;
					pBestThrow->iAttackValue		= iAttackValue;
					pBestThrow->ubAPCost			= ubMinAPcost + CalcAPCostForAiming(pSoldier, sGridNo, ubMaxPossibleAimTime);//dnl ch64 310813
					pBestThrow->bTargetLevel		= bOpponentLevel[ubLoop];

					//sprintf(tempstr,"new best THROW AttackValue = %d at grid #%d",iAttackValue/100000,gridno);
					//PopMessage(tempstr);
				}
			}
		}
	}

	// this is try to minimize enemies wasting their (limited) toss attacks:
	// sevenfm 80-40% depending on soldier difficulty
	UINT8 ubMinChanceToReallyHit = 80 - 10 * ubDiff;
	if( Item[usGrenade].flare )
	{
		ubMinChanceToReallyHit = 30;
	}

	if (pBestThrow->ubChanceToReallyHit < ubMinChanceToReallyHit)
	{
		pBestThrow->ubPossible = FALSE;
	}

	/*switch( ubDiff )
	{
	case 0:
	case 1:
		// don't use unless have a 70% chance to hit
		if (pBestThrow->ubChanceToReallyHit < 70)
		{
			pBestThrow->ubPossible = FALSE;
		}
		break;
	case 2:
	case 3:
	case 4:
		// don't use unless have a 50% chance to hit
		if (pBestThrow->ubChanceToReallyHit < 50)
		{
			pBestThrow->ubPossible = FALSE;
		}
		break;
	default:
		break;
	}*/
//if(pBestThrow->ubPossible)SendFmtMsg("CalcBestThrow;\r\n  ID=%d Loc=%d APs=%d Ac=%d AcData=%d Al=%d, SM=%d, LAc=%d, NAc=%d AT=%d\r\n  AP?=%d,%d,%d/%d BS=%d", pSoldier->ubID, pSoldier->sGridNo, pSoldier->bActionPoints, pSoldier->aiData.bAction, pSoldier->aiData.usActionData, pSoldier->aiData.bAlertStatus, pBestThrow->bScopeMode, pSoldier->aiData.bLastAction, pSoldier->aiData.bNextAction, pBestThrow->ubAimTime, pBestThrow->ubAPCost, CalcAPCostForAiming(pSoldier, pBestThrow->sTarget, (INT8)pBestThrow->ubAimTime), CalcTotalAPsToAttack(pSoldier, pBestThrow->sTarget, TRUE, pBestThrow->ubAimTime), CalcTotalAPsToAttack(pSoldier, pBestThrow->sTarget, FALSE, pBestThrow->ubAimTime), pBestThrow->ubStance);
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"calcbestthrow done");
}

void CalcBestStab(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestStab, BOOLEAN fBladeAttack )
{
	UINT32 uiLoop;
	INT32 iAttackValue;
	INT32 iThreatValue,iHitRate,iBestHitRate,iPercentBetter, iEstDamage;
	BOOLEAN fSurpriseStab;
	INT16 ubRawAPCost,ubMinAPCost,ubMaxPossibleAimTime = 0;
	INT16 ubChanceToReallyHit = 0;
	INT16 ubAimTime,ubChanceToHit,ubBestAimTime;
	SOLDIERTYPE *pOpponent;
	UINT16 usTrueMovementMode;
	INT16 ubBestChanceToHit;
	//InitAttackType(pBestStab);		// set all structure fields to defaults//dnl ch69 150913

	pSoldier->usAttackingWeapon = pSoldier->inv[HANDPOS].usItem;

	// sevenfm: initialize
	pBestStab->ubPossible = FALSE;
	pBestStab->iAttackValue = 0;	
	pBestStab->ubChanceToReallyHit = 0;
	pBestStab->ubOpponent = NOBODY;

	// temporarily make this guy run so we get a proper AP cost value
	// from CalcTotalAPsToAttack
	usTrueMovementMode = pSoldier->usUIMovementMode;
	pSoldier->usUIMovementMode = RUNNING;

	// determine which attack against which target has the greatest attack value

	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpponent || !pOpponent->stats.bLife)
			continue;			// next merc

		if (!ValidOpponent(pSoldier, pOpponent))
		{
			continue;
		}

		// if this opponent is not currently in sight (ignore known but unseen!)
		// sevenfm: allow stabbing recently seen opponents or public known opponents
		if ( pSoldier->aiData.bOppList[pOpponent->ubID] != SEEN_CURRENTLY &&
			pSoldier->aiData.bOppList[pOpponent->ubID] != SEEN_THIS_TURN &&
			gbPublicOpplist[pSoldier->bTeam][pOpponent->ubID] != SEEN_CURRENTLY )
			continue;			// next merc

		// if this opponent is not on the same level
		if (pSoldier->pathing.bLevel != pOpponent->pathing.bLevel)
			continue;			// next merc		

#ifdef DEBUGATTACKS
		DebugAI( String( "%s can see %s\n",pSoldier->name,ExtMen[pOpponent->ubID].name ) );
#endif

		// calculate minimum action points required to stab at this opponent
		ubMinAPCost = CalcTotalAPsToAttack( pSoldier,pOpponent->sGridNo,ADDTURNCOST, 0 );

		//ubMinAPCost = MinAPsToAttack(pSoldier,pOpponent->sGridNo,ADDTURNCOST);
		//NumMessage("MinAPcost to stab this opponent = ",ubMinAPCost);

		// Human: if I don't have enough APs left to get there & stab at this guy, skip 'im.
		// Monster:	I'll do an extra check later on to see if I can reach the guy this turn.

		// if 0 is returned then no path!
		if ( ubMinAPCost > pSoldier->bActionPoints || ubMinAPCost == 0 )
		{
			continue;
		}

		//KeepInterfaceGoing();

		// calc next attack's minimum stabbing cost (excludes movement & turning)
		//ubRawAPCost = MinAPsToShootOrStab(pSoldier,pOpponent->sGridNo, FALSE) - APBPConstants[AP_CHANGE_TARGET];
		ubRawAPCost = MinAPsToAttack(pSoldier,pOpponent->sGridNo, FALSE,0) - APBPConstants[AP_CHANGE_TARGET];
		//NumMessage("ubRawAPCost to stab this opponent = ",ubRawAPCost);

		// sevenfm: minimum possible value for ubRawAPCost
		if (ubRawAPCost < 1)
		{
			//ubRawAPCost = ubMinAPCost;
			ubRawAPCost = 1;
		}

		// determine if this is a surprise stab (must be next to opponent & unseen)
		fSurpriseStab = FALSE;		// assume it is not a surprise stab

		// if opponent doesn't see the attacker
		if (pOpponent->aiData.bOppList[pSoldier->ubID] != SEEN_CURRENTLY)
		{
			// and he's only one space away from attacker
			if (SpacesAway(pSoldier->sGridNo,pOpponent->sGridNo) == 1)
			{
				fSurpriseStab = TRUE;	// we got 'im lined up where we want 'im!
			}
		}


		iBestHitRate = 0;					 // reset best hit rate to minimum

		// calculate the maximum possible aiming time
		// HEADROCK HAM 4: Required for new Aiming Level Limits function
		ubMaxPossibleAimTime = min(AllowedAimingLevels(pSoldier, pOpponent->sGridNo),pSoldier->bActionPoints - ubMinAPCost);
		//NumMessage("Max Possible Aim Time = ",ubMaxPossibleAimTime);

		// consider the various aiming times
		for (ubAimTime = APBPConstants[AP_MIN_AIM_ATTACK]; ubAimTime <= ubMaxPossibleAimTime; ubAimTime++)
		{
			//HandleMyMouseCursor(KEYBOARDALSO);

			//NumMessage("ubAimTime = ",ubAimTime);

			if (!fSurpriseStab)
			{
				if (fBladeAttack)
				{
					ubChanceToHit = (INT16) CalcChanceToStab(pSoldier,pOpponent,ubAimTime);
				}
				else
				{
					ubChanceToHit = (INT16) CalcChanceToPunch(pSoldier,pOpponent,ubAimTime);
				}
			}
			else
				// HEADROCK (HAM): Externalized maximum to JA2_OPTIONS.INI
				// ubChanceToHit = MAXCHANCETOHIT;
				ubChanceToHit = gGameExternalOptions.ubMaximumCTH;
			//NumMessage("chance to Hit = ",ubChanceToHit);

			//sprintf(tempstr,"Vs. %s, at AimTime %d, ubChanceToHit = %d",ExtMen[pOpponent->ubID].name,ubAimTime,ubChanceToHit);
			//PopMessage(tempstr);

			if (ubRawAPCost < 1)
				ubRawAPCost = ubMinAPCost;

			//iHitRate = (pSoldier->bActionPoints * ubChanceToHit) / (ubRawAPCost + ubAimTime);
			// sevenfm: 100AP system
			// sevenfm: take into account pathing and turning
			//iHitRate = (pSoldier->bActionPoints * ubChanceToHit) / (ubRawAPCost + ubAimTime * APBPConstants[AP_CLICK_AIM]);
			iHitRate = ((pSoldier->bActionPoints - ubMinAPCost) * ubChanceToHit) / (ubRawAPCost + ubAimTime * APBPConstants[AP_CLICK_AIM]);
			//NumMessage("hitRate = ",iHitRate);

			// if aiming for this amount of time produces a better hit rate
			if (iHitRate > iBestHitRate)
			{
				iBestHitRate = iHitRate;
				ubBestAimTime = ubAimTime;
				ubBestChanceToHit = ubChanceToHit;
			}
		}


		// if we can't get any kind of hit rate at all
		if (iBestHitRate == 0)
			continue;			// next opponent

		// stabs are not affected by cover, so the chance to REALLY hit is the same
		ubChanceToReallyHit = ubBestChanceToHit;

		// calculate this opponent's threat value
		// NOTE: ignore my cover!	By the time I run beside him I won't have any!
		//iThreatValue = CalcManThreatValue(pOpponent,pSoldier->sGridNo,FALSE,pSoldier);
		iThreatValue = CalcManThreatValue(pOpponent,pSoldier->sGridNo,TRUE,pSoldier);

		// estimate the damage this stab would do to this opponent
		iEstDamage = EstimateStabDamage(pSoldier,pOpponent,ubBestChanceToHit, fBladeAttack );
		//NumMessage("STAB EstDamage = ", iEstDamage);

		// calculate the combined "attack value" for this opponent
		// highest possible value before division should be about 1 billion...
		// normal value before division should be about 5 million...
		iAttackValue = ( iEstDamage * iBestHitRate * ubChanceToReallyHit * iThreatValue) / 1000;
		//NumMessage("STAB AttackValue = ",iAttackValue / 1000);

#ifdef DEBUGATTACKS
		DebugAI( String( "CalcBestStab: best AttackValue vs %d = %d\n",ubLoop,iAttackValue ) );
#endif

		// if we can hurt the guy, OR probably not, but at least it's our best
		// chance to actually hit him and maybe scare him, knock him down, etc.
		if ((iAttackValue > 0) || (ubChanceToReallyHit > pBestStab->ubChanceToReallyHit))
		{
			// if there already was another viable target
			if (pBestStab->ubChanceToReallyHit > 0)
			{
				// OK, how does our chance to hit him compare to the previous best one?
				iPercentBetter = ((ubChanceToReallyHit * 100) / pBestStab->ubChanceToReallyHit) - 100;

				// if this chance to really hit is more than 50% worse, and the other
				// guy is conscious at all
				if (iPercentBetter < -PERCENT_TO_IGNORE_THREAT &&
					pBestStab->ubOpponent != NOBODY &&
					Menptr[pBestStab->ubOpponent].stats.bLife >= OKLIFE)
				{
					// then stick with the older guy as the better target
					continue;
				}

				// if this chance to really hit between 50% worse to 50% better
				if (iPercentBetter < PERCENT_TO_IGNORE_THREAT)
				{
					// then the one with the higher ATTACK VALUE is the better target
					if (iAttackValue < pBestStab->iAttackValue)
					{
						// the previous guy is more important since he's more dangerous
						continue;			// next opponent
					}
				}
			}

			// OOOF!	That was a lot of work!	But we've got a new best target!
			pBestStab->ubPossible			= TRUE;
			pBestStab->ubOpponent			= pOpponent->ubID;
			pBestStab->ubAimTime			= ubBestAimTime;
			pBestStab->ubChanceToReallyHit = ubChanceToReallyHit;
			pBestStab->sTarget			 = pOpponent->sGridNo;
			pBestStab->bTargetLevel		= pOpponent->pathing.bLevel;
			pBestStab->iAttackValue		= iAttackValue;
			pBestStab->ubAPCost			= ubMinAPCost + ubBestAimTime;
		}
	}

	pSoldier->usUIMovementMode = usTrueMovementMode;
}

void CalcTentacleAttack(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestStab )
{
	UINT32 uiLoop;
	INT32 iAttackValue;
	INT32 iThreatValue,iHitRate,iBestHitRate, iEstDamage;
	BOOLEAN fSurpriseStab;
	UINT8 ubMaxPossibleAimTime = 0;
	INT16 ubBestChanceToHit,ubAimTime,ubMinAPCost,ubChanceToHit,ubBestAimTime,ubRawAPCost;
	INT16 ubChanceToReallyHit = 0;
	SOLDIERTYPE *pOpponent;

	//InitAttackType(pBestStab);		// set all structure fields to defaults//dnl ch69 150913

	// sevenfm: initialize
	pBestStab->ubPossible = FALSE;
	pBestStab->iAttackValue = 0;	
	pBestStab->ubChanceToReallyHit = 0;
	pBestStab->ubOpponent = NOBODY;

	pSoldier->usAttackingWeapon = pSoldier->inv[HANDPOS].usItem;

	// determine which attack against which target has the greatest attack value

	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpponent || !pOpponent->stats.bLife)
			continue;			// next merc

		// if this man is neutral / on the same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
			continue;			// next merc

		// if this opponent is not currently in sight (ignore known but unseen!)
		if (pSoldier->aiData.bOppList[pOpponent->ubID] != SEEN_CURRENTLY)
			continue;			// next merc

		// if this opponent is not on the same level
		if (pSoldier->pathing.bLevel != pOpponent->pathing.bLevel)
			continue;			// next merc

		UINT16 usRange = GetModifiedGunRange(CREATURE_QUEEN_TENTACLES);
		// if this opponent is outside the range of our tentacles
		if ( GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, pOpponent->sGridNo ) > usRange )
		{
			continue; // next merc
		}

#ifdef DEBUGATTACKS
		DebugAI( String( "%s can see %s\n",pSoldier->GetName(),ExtMen[pOpponent->ubID].GetName() ) );
#endif

		// calculate minimum action points required to stab at this opponent
		ubMinAPCost = CalcTotalAPsToAttack( pSoldier,pOpponent->sGridNo,ADDTURNCOST, 0 );
		//ubMinAPCost = MinAPsToAttack(pSoldier,pOpponent->sGridNo,ADDTURNCOST);
		//NumMessage("MinAPcost to stab this opponent = ",ubMinAPCost);


		// calc next attack's minimum stabbing cost (excludes movement & turning)
		//ubRawAPCost = MinAPsToShootOrStab(pSoldier,pOpponent->sGridNo, FALSE) - APBPConstants[AP_CHANGE_TARGET];
		ubRawAPCost = MinAPsToAttack(pSoldier,pOpponent->sGridNo, FALSE,0) - APBPConstants[AP_CHANGE_TARGET];
		//NumMessage("ubRawAPCost to stab this opponent = ",ubRawAPCost);

		// sevenfm: set minimum possible AP cost
		if (ubRawAPCost < 1)
		{
			//ubRawAPCost = ubMinAPCost;
			ubRawAPCost = 1;
		}

		// determine if this is a surprise stab (for tentacles, enemy must not see us, no dist limit)
		fSurpriseStab = FALSE;		// assume it is not a surprise stab

		// if opponent doesn't see the attacker
		if (pOpponent->aiData.bOppList[pSoldier->ubID] != SEEN_CURRENTLY)
		{
			fSurpriseStab = TRUE;	// we got 'im lined up where we want 'im!
		}

		iBestHitRate = 0;					 // reset best hit rate to minimum

		// calculate the maximum possible aiming time

		//ubMaxPossibleAimTime = min(AllowedAimingLevels(pSoldier),pSoldier->bActionPoints - ubMinAPCost);
		ubMaxPossibleAimTime = 0;
		//NumMessage("Max Possible Aim Time = ",ubMaxPossibleAimTime);

		// consider the various aiming times
		for (ubAimTime = APBPConstants[AP_MIN_AIM_ATTACK]; ubAimTime <= ubMaxPossibleAimTime; ubAimTime++)
		{
			//HandleMyMouseCursor(KEYBOARDALSO);

			//NumMessage("ubAimTime = ",ubAimTime);

			if (!fSurpriseStab)
			{
				ubChanceToHit = (INT16) CalcChanceToStab(pSoldier,pOpponent,ubAimTime);
			}
			else
				// HEADROCK (HAM): Externalized maximum to JA2_OPTIONS.INI
				// ubChanceToHit = MAXCHANCETOHIT;
				ubChanceToHit = gGameExternalOptions.ubMaximumCTH;
			//NumMessage("chance to Hit = ",ubChanceToHit);

			//sprintf(tempstr,"Vs. %s, at AimTime %d, ubChanceToHit = %d",ExtMen[pOpponent->ubID].name,ubAimTime,ubChanceToHit);
			//PopMessage(tempstr);

			//iHitRate = (pSoldier->bActionPoints * ubChanceToHit) / (ubRawAPCost + ubAimTime);
			// sevenfm: 100AP system
			iHitRate = (pSoldier->bActionPoints * ubChanceToHit) / (ubRawAPCost + ubAimTime * APBPConstants[AP_CLICK_AIM]);
			//NumMessage("hitRate = ",iHitRate);

			// if aiming for this amount of time produces a better hit rate
			if (iHitRate > iBestHitRate)
			{
				iBestHitRate = iHitRate;
				ubBestAimTime = ubAimTime;
				ubBestChanceToHit = ubChanceToHit;
			}
		}

		// if we can't get any kind of hit rate at all
		if (iBestHitRate == 0)
			continue;			// next opponent

		// stabs are not affected by cover, so the chance to REALLY hit is the same
		ubChanceToReallyHit = ubBestChanceToHit;

		// calculate this opponent's threat value
		// NOTE: ignore my cover!	By the time I run beside him I won't have any!
		iThreatValue = CalcManThreatValue(pOpponent,pSoldier->sGridNo,FALSE,pSoldier);

		// estimate the damage this stab would do to this opponent
		iEstDamage = EstimateStabDamage(pSoldier,pOpponent,ubBestChanceToHit, TRUE );
		//NumMessage("STAB EstDamage = ", iEstDamage);

		// calculate the combined "attack value" for this opponent
		// highest possible value before division should be about 1 billion...
		// normal value before division should be about 5 million...
		iAttackValue = ( iEstDamage * iBestHitRate * ubChanceToReallyHit * iThreatValue) / 1000;
		//NumMessage("STAB AttackValue = ",iAttackValue / 1000);

#ifdef DEBUGATTACKS
		DebugAI( String( "CalcBestStab: best AttackValue vs %d = %d\n",ubLoop,iAttackValue ) );
#endif

		// if we can hurt the guy, OR probably not, but at least it's our best
		// chance to actually hit him and maybe scare him, knock him down, etc.
		if (iAttackValue > 0)
		{
			// OOOF!	That was a lot of work!	But we've got a new best target!
			pBestStab->ubPossible			= TRUE;
			pBestStab->ubOpponent			= pOpponent->ubID;
			pBestStab->ubAimTime			= ubBestAimTime;
			pBestStab->ubChanceToReallyHit = ubChanceToReallyHit;
			pBestStab->sTarget			 = pOpponent->sGridNo;
			pBestStab->bTargetLevel		= pOpponent->pathing.bLevel;

			// ADD this target's attack value to our TOTAL...
			pBestStab->iAttackValue				+= iAttackValue;

			pBestStab->ubAPCost			= ubMinAPCost + ubBestAimTime;

		}
	}
}

UINT8 NumMercsCloseTo( INT32 sGridNo, UINT8 ubMaxDist )
{
	INT8						bNumber = 0;
	UINT32					uiLoop;
	SOLDIERTYPE *		pSoldier;

	for ( uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pSoldier = MercSlots[ uiLoop ];

		// sevenfm: count all teams except creatures
		if (pSoldier && pSoldier->bTeam != CREATURE_TEAM && pSoldier->stats.bLife >= OKLIFE)
		{
			if (PythSpacesAway( sGridNo, pSoldier->sGridNo ) <= ubMaxDist)
			{
				bNumber++;
			}
		}
	}

	return( bNumber );
}

INT32 EstimateShotDamage(SOLDIERTYPE *pSoldier, SOLDIERTYPE *pOpponent, INT16 ubChanceToHit)
{
	INT32 iRange,iMaxRange,iPowerLost;
	INT32 iDamage;
	INT32 iDamageBeforeArmour;
	UINT8 ubBonus;
	INT32 iHeadProt = 0, iTorsoProt = 0, iLegProt = 0;
	INT32 iTotalProt;
	//INT8 bPlatePos;
	UINT8	ubAmmoType;
	UINT16 usItem;
	OBJECTTYPE *pObj;

	pObj = &(pSoldier->inv[pSoldier->ubAttackingHand]);
	usItem = pObj->usItem;

	if ( Item[ usItem ].usItemClass & IC_THROWING_KNIFE )
	{
		ubAmmoType = AMMO_KNIFE;
	}
	else
	{
		ubAmmoType = (*pObj)[0]->data.gun.ubGunAmmoType;
	}

	// calculate distance to target, obtain his gun's maximum range rating

	iRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, pOpponent->sGridNo );
	// SANDRO - added specific range calculation
	if ( Item[ usItem ].usItemClass & IC_GUN )
		iMaxRange = GunRange( pObj, pSoldier );
	else
		iMaxRange = GetModifiedGunRange(usItem);

	// bullet loses speed and penetrating power, 50% loss per maximum range
	// SANDRO - you know, 50% is a lot, it often leads to negative damage values in the end
	// while there is no true effect of this in game !!! Made it 25%!
	iPowerLost = ((25 * iRange) / iMaxRange);

	// up to 50% extra impact for making particularly accurate successful shots
	ubBonus = ubChanceToHit / 4;		// /4 is really /2 and /2 again

	iDamage = ((GetDamage(pObj)) * (100 - iPowerLost + ubBonus) / 100) ;

	//NumMessage("Pre-protection damage: ",damage);

	// if opponent is wearing a helmet
	if (pOpponent->inv[HELMETPOS].usItem)
	{
		iHeadProt += (INT32) Armour[Item[pOpponent->inv[HELMETPOS].usItem].ubClassIndex].ubProtection *
			(INT32) pOpponent->inv[HELMETPOS][0]->data.objectStatus / 100;
	}

	// if opponent is wearing a protective vest
	if ( !AmmoTypes[ubAmmoType].ignoreArmour )
	{
		// monster spit and knives ignore kevlar vests
		if (pOpponent->inv[VESTPOS].usItem)
		{
			iTorsoProt += (INT32) Armour[Item[pOpponent->inv[VESTPOS].usItem].ubClassIndex].ubProtection *
				(INT32) pOpponent->inv[VESTPOS][0]->data.objectStatus / 100;
		}
	}

	// check for ceramic plates; these do affect monster spit
	for (attachmentList::iterator iter = pOpponent->inv[VESTPOS][0]->attachments.begin(); iter != pOpponent->inv[VESTPOS][0]->attachments.end(); ++iter) {
		if (Item[iter->usItem].usItemClass == IC_ARMOUR && (*iter)[0]->data.objectStatus > 0 && iter->exists() )
		{
			iTorsoProt += (INT32) Armour[Item[iter->usItem].ubClassIndex].ubProtection *
				(INT32) (*iter)[0]->data.objectStatus / 100;
		}
	}

	// if opponent is wearing armoured leggings (LEGPOS)
	if ( !AmmoTypes[ubAmmoType].ignoreArmour )
	{	// monster spit and knives ignore kevlar leggings
		if (pOpponent->inv[LEGPOS].usItem)
		{
			iLegProt += (INT32) Armour[Item[pOpponent->inv[LEGPOS].usItem].ubClassIndex].ubProtection *
				(INT32) pOpponent->inv[LEGPOS][0]->data.objectStatus / 100;
		}
	}

	// 15% of all shots are to the head, 80% are to the torso.	Calc. avg. prot.
	// NB: make AI guys shoot at head 15% of time, 5% of time at legs

	iTotalProt = ((15 * iHeadProt) + (75 * iTorsoProt) + 5 * iLegProt) / 100;
	iTotalProt = (INT32) (iTotalProt * AmmoTypes[ubAmmoType].armourImpactReductionMultiplier / max(1,AmmoTypes[ubAmmoType].armourImpactReductionDivisor) );

	iDamageBeforeArmour = iDamage;
	//iDamage -= iTotalProt;
	iDamage = __max(0, iDamage - iTotalProt);

	iDamage = (INT32)(iDamage * AmmoTypes[ubAmmoType].afterArmourDamageMultiplier / max(1,AmmoTypes[ubAmmoType].afterArmourDamageDivisor) ) ;

	// sevenfm: take into account BP loss from hit
	if( iDamage < iDamageBeforeArmour )
	{
		iDamage += (iDamageBeforeArmour - iDamage) / 2;
	}

	if (AmmoTypes[ubAmmoType].monsterSpit )
	{
		// cheat and emphasize shots
		//iDamage = (iDamage * 15) / 10;
		switch( usItem )
		{
			// explosive damage is 100-200% that of the rated, so multiply by 3/2s here
		case CREATURE_QUEEN_SPIT: //TODO: Madd - remove the hardcoding here
			iDamage += ( 3 * Explosive[ Item[ LARGE_CREATURE_GAS ].ubClassIndex ].ubDamage * NumMercsCloseTo( pOpponent->sGridNo, (UINT8)Explosive[ Item[ LARGE_CREATURE_GAS ].ubClassIndex ].ubRadius ) ) / 2;
			break;
		case CREATURE_OLD_MALE_SPIT:
			iDamage += ( 3 * Explosive[ Item[ SMALL_CREATURE_GAS ].ubClassIndex ].ubDamage * NumMercsCloseTo( pOpponent->sGridNo, (UINT8)Explosive[ Item[ SMALL_CREATURE_GAS ].ubClassIndex	].ubRadius ) ) / 2;
			break;
		default:
			iDamage += ( 3 * Explosive[ Item[ VERY_SMALL_CREATURE_GAS ].ubClassIndex ].ubDamage * NumMercsCloseTo( pOpponent->sGridNo, (UINT8)Explosive[ Item[ VERY_SMALL_CREATURE_GAS ].ubClassIndex	].ubRadius ) ) / 2;
			break;
		}
	}

	if (iDamage < 1)
		iDamage = 1;	// assume we can do at LEAST 1 pt minimum damage

	return( iDamage );
}

INT32 EstimateThrowDamage( SOLDIERTYPE *pSoldier, UINT8 ubItemPos, SOLDIERTYPE *pOpponent, INT32 sGridNo )
{
	UINT16	ubExplosiveIndex;
	INT32	iExplosDamage, iBreathDamage, iArmourAmount, iDamage = 0;
	INT8	bSlot;


	if( pSoldier == NULL || pOpponent == NULL || ubItemPos > pSoldier->inv.size() || sGridNo > giNumberOfTiles )
		return 0;

	if( pSoldier->inv[ubItemPos].exists() == false )
		return 0;

	if ( Item[pSoldier->inv[ ubItemPos ].usItem].singleshotrocketlauncher )
		ubExplosiveIndex = Item[ C1 ].ubClassIndex;
	else if ( Item[pSoldier->inv[ ubItemPos ].usItem].rocketlauncher || Item[pSoldier->inv[ ubItemPos ].usItem].grenadelauncher || Item[pSoldier->inv[ ubItemPos ].usItem].mortar )
	{
		OBJECTTYPE* pAttachment = FindLaunchableAttachment(&pSoldier->inv[ ubItemPos ],pSoldier->inv[ ubItemPos ].usItem ) ;
		if ( pAttachment->exists() )
			ubExplosiveIndex = Item[pAttachment->usItem].ubClassIndex;
		else
			return 0;
	}
	else if(IsGrenadeLauncherAttached(&pSoldier->inv[ubItemPos]))//dnl ch63 240813 situation when grenade is already in launcher
	{
		OBJECTTYPE *pAttachment = FindLaunchableAttachment(&pSoldier->inv[ubItemPos], GetAttachedGrenadeLauncher(&pSoldier->inv[ubItemPos]));
		if(pAttachment->exists())
			ubExplosiveIndex = Item[pAttachment->usItem].ubClassIndex;
		else
			return(0);
	}
	else if ( Explosive[Item[pSoldier->inv[ ubItemPos ].usItem].ubClassIndex].ubType == EXPLOSV_SMOKE )
		return 5;
	else
		ubExplosiveIndex = Item[ pSoldier->inv[ubItemPos].usItem ].ubClassIndex;

	//		break;
	//}
	// JA2Gold: added
	if ( Item[pSoldier->inv[ubItemPos].usItem].flare )
	{
		return( 5 * ( LightTrueLevel( pOpponent->sGridNo, pOpponent->pathing.bLevel ) - NORMAL_LIGHTLEVEL_DAY ) );
	}


	iExplosDamage = ( ( (INT32) GetModifiedExplosiveDamage( Explosive[ ubExplosiveIndex ].ubDamage, 0 ) ) * 3) / 2;
	iBreathDamage = ( ( (INT32) GetModifiedExplosiveDamage( Explosive[ ubExplosiveIndex ].ubStunDamage, 1 ) ) * 5) / 4;

	// sevenfm: IndoorModifier - increase damage inside buildings
	if (gpWorldLevelData[sGridNo].ubTerrainID == FLAT_FLOOR	)
	{
		iExplosDamage += (INT32) (iExplosDamage * Explosive[ ubExplosiveIndex ].bIndoorModifier);
	}

	// sevenfm: add damage from fragments
	if ( Explosive[ ubExplosiveIndex ].ubType == EXPLOSV_NORMAL &&
		Explosive[ ubExplosiveIndex ].usNumFragments > 0)
	{
		// sevenfm: use NumFragments/10, but no more than 20 fragments
		iExplosDamage += __min( 20, Explosive[ ubExplosiveIndex ].usNumFragments / 10 ) * Explosive[ ubExplosiveIndex ].ubFragDamage;
	}

	if ( Explosive[ ubExplosiveIndex ].ubType == EXPLOSV_TEARGAS || Explosive[ ubExplosiveIndex ].ubType == EXPLOSV_MUSTGAS )
	{
		// if target gridno is outdoors (where tear gas lasts only 1-2 turns)
		if (gpWorldLevelData[sGridNo].ubTerrainID != FLAT_FLOOR)
			iBreathDamage /= 2;		// reduce effective breath damage by 1/2

		bSlot = FindGasMask(pOpponent); //FindObj( pOpponent, GASMASK );
		if ((bSlot == HEAD1POS || bSlot == HEAD2POS || bSlot == HELMETPOS) && pSoldier->inv[bSlot][0]->data.objectStatus >= 70)
		{
			// take condition of the gas mask into account - it could be leaking
			iBreathDamage = (iBreathDamage * (100 - pOpponent->inv[bSlot][0]->data.objectStatus)) / 100;
			//NumMessage("damage after GAS MASK: ",iBreathDamage);
		}

	}
	if ( Explosive[ ubExplosiveIndex ].ubType == EXPLOSV_BURNABLEGAS )
	{
		// if target gridno is outdoors (where tear gas lasts only 1-2 turns)
		if (gpWorldLevelData[sGridNo].ubTerrainID != FLAT_FLOOR)
			iBreathDamage /= 2;		// reduce effective breath damage by 1/2
	}
	else if (iExplosDamage)
	{
		// EXPLOSION DAMAGE is spread amongst locations
		iArmourAmount = ArmourVersusExplosivesPercent( pSoldier );
		iExplosDamage -= iExplosDamage * iArmourAmount / 100;

		if (iExplosDamage < 1)
			iExplosDamage = 1;
	}

	// if this opponent is standing
	if (gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_STAND)
	{
		// 15 pt. flat bonus for knocking him down (for ANY type of explosion)
		iDamage += 15;
	}

	if ( pOpponent->bBreath < OKBREATH || AM_A_ROBOT( pOpponent ) )
	{
		// don't bother to count breath damage against people already down
		iBreathDamage = 0;
	}

	// estimate combined "damage" value considering combined health/breath damage
	iDamage += iExplosDamage + (iBreathDamage / 3);

	// approximate chance of the grenade going off (Ian's formulas are too funky)
	// then use that to reduce the expected damage because thing may not blow!
	iDamage = (iDamage * pSoldier->inv[ubItemPos][0]->data.objectStatus) / 100;

	return( iDamage);
}

INT32 EstimateStabDamage( SOLDIERTYPE *pSoldier, SOLDIERTYPE *pOpponent,
						INT16 ubChanceToHit, BOOLEAN fBladeAttack )
{
	INT32 iImpact, iFluke, iBonus;

	UINT16	usItem;

	usItem = pSoldier->inv[HANDPOS].usItem;

	if (fBladeAttack)
	{
		iImpact = GetDamage(&pSoldier->inv[HANDPOS]);
		iImpact += EffectiveStrength( pSoldier, FALSE ) / 20; // 0 to 5 for strength, adjusted by damage taken
	}
	else
	{
		// this all was a little changed for enhanced close combat system - SANDRO
		if ( gGameExternalOptions.fEnhancedCloseCombatSystem )
		{
			iImpact = EffectiveStrength( pSoldier, FALSE ) / 5; // 0 to 20 for strength, adjusted by damage taken

			if ( pSoldier->usAttackingWeapon )
			{
				iImpact += GetDamage(&pSoldier->inv[HANDPOS]);
			}
			else
			{
				// base HTH damage
				iImpact += 4; // Slightly reduced for we can now attack to head for bigger damage - SANDRO
				// Add melee damage multiplier to bare HtH attacks as well - SANDRO
				// actually I make the influence a little lesser, because to the blades and so,
				// only the item impact is multiplied, not the level and strength bonus, but here it does
				iImpact += iImpact * gGameExternalOptions.iMeleeDamageModifier / 120; 
			}
		}
		else
		{

			// NB martial artists don't get a bonus for using brass knuckles!
			if (pSoldier->usAttackingWeapon && !( HAS_SKILL_TRAIT( pSoldier, MARTIALARTS_OT ) ) )
			{
				iImpact = GetDamage(&pSoldier->inv[HANDPOS]);
			}
			else
			{
				// base HTH damage
				iImpact = 5;
			}
			iImpact += EffectiveStrength( pSoldier, FALSE ) / 5; // 0 to 20 for strength, adjusted by damage taken
		}
	}


	iImpact += (EffectiveExpLevel( pSoldier ) / 2); // 0 to 4 for level // SANDRO - added effective level calc

	iFluke = 0;
	iBonus = ubChanceToHit / 4;				// up to 50% extra impact for accurate attacks

	iImpact = iImpact * (100 + iFluke + iBonus) / 100;

	if (!fBladeAttack)
	{
		// add bonuses for hand-to-hand and martial arts
		// Check for new traits - SANDRO
		if ( gGameOptions.fNewTraitSystem )
		{
			if (!pSoldier->usAttackingWeapon || Item[pSoldier->inv[HANDPOS].usItem].brassknuckles)
			{
				// add bonuses for martial arts
				if ( HAS_SKILL_TRAIT( pSoldier, MARTIAL_ARTS_NT ) )
				{
					// also add breath damage bonus into consideration
					iImpact = (INT32)(((FLOAT)iImpact * (FLOAT)(100 + (gSkillTraitValues.ubMABonusDamageHandToHand + gSkillTraitValues.ubMABonusBreathDamageHandToHand) * NUM_SKILL_TRAITS(pSoldier, MARTIAL_ARTS_NT)) / 100.0f) + 0.5f);
				}
			}
			else
			{
				// +30% damage of blunt weapons for melee character
				if (HAS_SKILL_TRAIT( pSoldier, MELEE_NT ))
				{
					iImpact = (INT32)(((FLOAT)iImpact * (FLOAT)(100 + gSkillTraitValues.ubMEDamageBonusBlunt) / 100.0f) + 0.5f);
				}
			}
		}
		else
		{
			if ( HAS_SKILL_TRAIT( pSoldier, MARTIALARTS_OT ) )
			{
				iImpact = (INT32)(((FLOAT)iImpact * (FLOAT)(100 + gbSkillTraitBonus[MARTIALARTS_OT] * NUM_SKILL_TRAITS(pSoldier, MARTIALARTS_OT)) / 100.0f) + 0.5f);
			}
			if ( HAS_SKILL_TRAIT( pSoldier, HANDTOHAND_OT ) )
			{
				iImpact = (INT32)(((FLOAT)iImpact * (FLOAT)(100 + gbSkillTraitBonus[HANDTOHAND_OT] * NUM_SKILL_TRAITS(pSoldier, HANDTOHAND_OT)) / 100.0f) + 0.5f);
			}
		}
		// SANDRO - Enhanced Close Combat System - aiming at body parts makes difference
		if (gGameExternalOptions.fEnhancedCloseCombatSystem && (gAnimControl[ pOpponent->usAnimState ].ubEndHeight == ANIM_PRONE))
		{
			iImpact = (INT32)(iImpact * 150 / 100);  // 50% incresed damage to lying characters
		}
		// Here, if we're doing a bare-fisted attack,
		// we want to pay attention just to wounds inflicted
		// SANDRO - No, we may consider the breath damage as asort of "real" damage too, so only reduce it by half

		// sevenfm: divide by 4
		iImpact = iImpact / 4;
	}
	// SANDRO - damage bonus to melee trait
	else 
	{
		if ( HAS_SKILL_TRAIT( pSoldier, MELEE_NT ) && (gGameOptions.fNewTraitSystem) )
		{
			iImpact += (iImpact * (100 + gSkillTraitValues.ubMEDamageBonusBlades ) / 100); // +30% damage
		}
		// SANDRO - Enhanced Close Combat System
		if (gGameExternalOptions.fEnhancedCloseCombatSystem)
		{
			if (gAnimControl[ pOpponent->usAnimState ].ubEndHeight == ANIM_PRONE)
			{
				iImpact = (INT32)(iImpact * 140 / 100);  // 40% incresed damage to lying characters
			}
		}
	}

	// Flugente: Add personal damage bonus
	iImpact = (iImpact * (100 + pSoldier->GetMeleeDamageBonus() ) / 100);

	if (iImpact < 1)
	{
		iImpact = 1;
	}

	return( iImpact );
}

INT8 TryToReload( SOLDIERTYPE * pSoldier )
{
	INT8		bSlot;
	WEAPONTYPE *pWeapon;
	OBJECTTYPE *pObj, *pObj2;

	// HEADROCK HAM 3.3: Attempt to reload now takes magazine type into account. Prefernace will be given to magazines of similar type.
	pObj = &(pSoldier->inv[HANDPOS]);
	pWeapon = &(Weapon[pSoldier->inv[HANDPOS].usItem]);
	bSlot = FindAmmo( pSoldier, pWeapon->ubCalibre, pWeapon->ubMagSize, GetAmmoType(pObj), NO_SLOT );

	//if (bSlot != NO_SLOT)
	//{
	//	if (ReloadGun( pSoldier, &(pSoldier->inv[HANDPOS]), &(pSoldier->inv[bSlot]) ))
	//	{
	//		return( TRUE );
	//	}
	//}

	//<SB> manual recharge
	//pObj = &(pSoldier->inv[HANDPOS]);

	if ((*pObj)[0]->data.gun.ubGunShotsLeft && !((*pObj)[0]->data.gun.ubGunState & GS_CARTRIDGE_IN_CHAMBER) )
	{
		(*pObj)[0]->data.gun.ubGunState |= GS_CARTRIDGE_IN_CHAMBER;

		INT16 sModifiedReloadAP = Weapon[Item[(pObj)->usItem].ubClassIndex].APsToReloadManually;

		// modify by ini values
		if ( Item[ pObj->usItem ].usItemClass == IC_GUN )
			sModifiedReloadAP *= gItemSettings.fAPtoReloadManuallyModifierGun[ Weapon[ pObj->usItem ].ubWeaponType ];
		else if ( Item[ pObj->usItem ].usItemClass == IC_LAUNCHER )
			sModifiedReloadAP *= gItemSettings.fAPtoReloadManuallyModifierLauncher;

		////////////////////////////////////////////////////////////////////////////////////////////////////////
		// STOMP traits - SANDRO
		if ( gGameOptions.fNewTraitSystem )
		{
			// Sniper trait makes chambering a round faster
			if (( Weapon[Item[(pObj)->usItem].ubClassIndex].ubWeaponType == GUN_SN_RIFLE || Weapon[Item[(pObj)->usItem].ubClassIndex].ubWeaponType == GUN_RIFLE ) && HAS_SKILL_TRAIT( pSoldier, SNIPER_NT ))
				DeductPoints(pSoldier, (INT16)((((FLOAT)sModifiedReloadAP * (FLOAT)(100 - gSkillTraitValues.ubSNChamberRoundAPsReduction * NUM_SKILL_TRAITS(pSoldier, SNIPER_NT))) / 100.0f) + 0.5f), 0);
			// Ranger trait makes pumping shotguns faster
			else if (( Weapon[Item[(pObj)->usItem].ubClassIndex].ubWeaponType == GUN_SHOTGUN ) && HAS_SKILL_TRAIT( pSoldier, RANGER_NT ))
				DeductPoints(pSoldier, (INT16)((((FLOAT)sModifiedReloadAP * (FLOAT)(100 - gSkillTraitValues.ubRAPumpShotgunsAPsReduction * NUM_SKILL_TRAITS(pSoldier, RANGER_NT))) / 100.0f) + 0.5f), 0);
			else
				DeductPoints(pSoldier, sModifiedReloadAP, 0);
		}
		else
		{
			DeductPoints(pSoldier, sModifiedReloadAP, 0);
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////

		PlayJA2Sample( Weapon[ Item[pObj->usItem].ubClassIndex ].ManualReloadSound, RATE_11025, SoundVolume( HIGHVOLUME, pSoldier->sGridNo ), 1, SoundDir( pSoldier->sGridNo ) );


		if ( pSoldier->IsValidSecondHandShot( ) )
		{
			pObj2 = &(pSoldier->inv[SECONDHANDPOS]);

			if ((*pObj2)[0]->data.gun.ubGunShotsLeft && !((*pObj2)[0]->data.gun.ubGunState & GS_CARTRIDGE_IN_CHAMBER) )
			{
				(*pObj2)[0]->data.gun.ubGunState |= GS_CARTRIDGE_IN_CHAMBER;
				PlayJA2Sample( Weapon[ Item[pObj2->usItem].ubClassIndex ].ManualReloadSound, RATE_11025, SoundVolume( HIGHVOLUME, pSoldier->sGridNo ), 1, SoundDir( pSoldier->sGridNo ) );
			}
		}

		return TRUE;
	}
	else
	{
		if ( pSoldier->IsValidSecondHandShot( ) )
		{
			pObj2 = &(pSoldier->inv[SECONDHANDPOS]);

			if ((*pObj2)[0]->data.gun.ubGunShotsLeft && !((*pObj2)[0]->data.gun.ubGunState & GS_CARTRIDGE_IN_CHAMBER) )
			{
				(*pObj2)[0]->data.gun.ubGunState |= GS_CARTRIDGE_IN_CHAMBER;

				INT16 sModifiedReloadAP = Weapon[Item[(pObj2)->usItem].ubClassIndex].APsToReloadManually;

				// modify by ini values
				if ( Item[ pObj2->usItem ].usItemClass == IC_GUN )
					sModifiedReloadAP *= gItemSettings.fAPtoReloadManuallyModifierGun[ Weapon[ pObj2->usItem ].ubWeaponType ];
				else if ( Item[ pObj2->usItem ].usItemClass == IC_LAUNCHER )
					sModifiedReloadAP *= gItemSettings.fAPtoReloadManuallyModifierLauncher;

				////////////////////////////////////////////////////////////////////////////////////////////////////////
				// STOMP traits - SANDRO (well, I don't know any one-handed sniper rifle, but what the hell...)
				if ( gGameOptions.fNewTraitSystem )
				{
					// Sniper trait makes chambering a round faster
					if (( Weapon[Item[(pObj2)->usItem].ubClassIndex].ubWeaponType == GUN_SN_RIFLE || Weapon[Item[(pObj2)->usItem].ubClassIndex].ubWeaponType == GUN_RIFLE ) && HAS_SKILL_TRAIT( pSoldier, SNIPER_NT ))
						DeductPoints(pSoldier, (INT16)((((FLOAT)sModifiedReloadAP * (FLOAT)(100 - gSkillTraitValues.ubSNChamberRoundAPsReduction * NUM_SKILL_TRAITS(pSoldier, SNIPER_NT))) / 100.0f) + 0.5f), 0);
					// Ranger trait makes pumping shotguns faster
					else if (( Weapon[Item[(pObj2)->usItem].ubClassIndex].ubWeaponType == GUN_SHOTGUN ) && HAS_SKILL_TRAIT( pSoldier, RANGER_NT ))
						DeductPoints(pSoldier, (INT16)((((FLOAT)sModifiedReloadAP * (FLOAT)(100 - gSkillTraitValues.ubRAPumpShotgunsAPsReduction * NUM_SKILL_TRAITS(pSoldier, RANGER_NT))) / 100.0f) + 0.5f), 0);
					else
						DeductPoints(pSoldier, sModifiedReloadAP, 0);
				}
				else
				{
					DeductPoints(pSoldier, sModifiedReloadAP, 0);
				}
				////////////////////////////////////////////////////////////////////////////////////////////////////////

				PlayJA2Sample( Weapon[ Item[pObj2->usItem].ubClassIndex ].ManualReloadSound, RATE_11025, SoundVolume( HIGHVOLUME, pSoldier->sGridNo ), 1, SoundDir( pSoldier->sGridNo ) );

				return TRUE;
			}
		}
	}
	//</SB>

	if (bSlot != NO_SLOT && ReloadGun( pSoldier, &(pSoldier->inv[HANDPOS]), &(pSoldier->inv[bSlot]) ))
	{
		return( TRUE );
	}

	return( NOSHOOT_NOAMMO );
}

/*
INT8 TryToReloadLauncher( SOLDIERTYPE * pSoldier )
{
UINT16	usWeapon;
INT8		bSlot;

usWeapon = pSoldier->inv[HANDPOS].usItem;

if ( usWeapon == TANK_CANNON )
{
bSlot = FindObj( pSoldier, TANK_SHELL );
}
else
{
bSlot = FindLaunchable( pSoldier, usWeapon );
}

if (bSlot != NO_SLOT)
{
}
return( NOSHOOT_NOAMMO );
}
*/

INT8 CanNPCAttack(SOLDIERTYPE *pSoldier)
{
	INT8		bCanAttack;
	INT8		bWeaponIn;

	// NEUTRAL civilians are not allowed to attack, but those that are not
	// neutral (KILLNPC mission guynums, escorted guys) can, if they're armed
	if (PTR_CIVILIAN && pSoldier->aiData.bNeutral)
	{
		return(FALSE);
	}

	// test if if we are able to attack (in general, not at any specific target)
	bCanAttack = OKToAttack(pSoldier,NOWHERE);

	// if soldier can't attack because he doesn't have a weapon or is out of ammo
	// or his weapon isn't loaded
	if ( bCanAttack == NOSHOOT_NOAMMO ) // || NOLOAD
	{
		// try to reload it
		bCanAttack = TryToReload( pSoldier );
		if( bCanAttack == TRUE )
		{
			PossiblyStartEnemyTaunt( pSoldier, TAUNT_RELOAD );
		}
		else
		{
			PossiblyStartEnemyTaunt( pSoldier, TAUNT_OUT_OF_AMMO );
		}
	}
	else if (bCanAttack == NOSHOOT_NOWEAPON)
	{
		// look for another weapon
		bWeaponIn = FindAIUsableObjClass( pSoldier, IC_WEAPON );
		if (bWeaponIn != NO_SLOT)
		{
			DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"cannpcattack: swapping weapon into hand");
			RearrangePocket( pSoldier, HANDPOS, bWeaponIn, FOREVER );
			// look for another weapon if this one is 1-handed
			//			if ( (Item[ pSoldier->inv[ HANDPOS ].usItem ].usItemClass == IC_GUN) && !(Item[ pSoldier->inv[ HANDPOS ].usItem ].fFlags & ITEM_TWO_HANDED ) )
			if ( (Item[ pSoldier->inv[ HANDPOS ].usItem ].usItemClass == IC_GUN) && !(Item[ pSoldier->inv[ HANDPOS ].usItem ].twohanded ) )
			{
				// look for another pistol/SMG if available
				// CHRISL: Change final parameter to use dynamic pocket definition
				bWeaponIn = FindAIUsableObjClassWithin( pSoldier, IC_WEAPON, BIGPOCKSTART, NUM_INV_SLOTS );
				//				if (bWeaponIn != NO_SLOT && (Item[ pSoldier->inv[ bWeaponIn ].usItem ].usItemClass == IC_GUN) && !(Item[ pSoldier->inv[ bWeaponIn ].usItem ].fFlags & ITEM_TWO_HANDED ) )
				if (bWeaponIn != NO_SLOT && (Item[ pSoldier->inv[ bWeaponIn ].usItem ].usItemClass == IC_GUN) && !(Item[ pSoldier->inv[ bWeaponIn ].usItem ].twohanded ) )
				{
					DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"cannpcattack: swapping weapon into holster");
					RearrangePocket( pSoldier, SECONDHANDPOS, bWeaponIn, FOREVER );
				}
			}
			// might need to reload
			bCanAttack = CanNPCAttack( pSoldier );
		}
	}

#ifdef DEBUGDECISIONS
	if (bCanAttack != TRUE) // if for any reason we can't attack right now
	{
		//LocateMember(pSoldier->ubID,SETLOCATOR); // locate to this NPC, don't center
		STR16 tempstr;
		sprintf(tempstr,"%s can't attack! (not OKToAttack, Reason code = %d)",pSoldier->GetName(),bCanAttack);
		AIPopMessage(tempstr);
	}
#endif

	return( bCanAttack );
}

void CheckIfTossPossible(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestThrow)
{
	DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"CheckIfTossPossible");
	INT16 ubMinAPcost;

	pSoldier->bWeaponMode = WM_NORMAL;

	if ( TANK( pSoldier ) )
	{
		pBestThrow->bWeaponIn = FindCannon(pSoldier);//FindObj( pSoldier, TANK_CANNON );
	}
	else
	{
		pBestThrow->bWeaponIn = FindAIUsableObjClass( pSoldier, IC_LAUNCHER );

		if ( pBestThrow->bWeaponIn == NO_SLOT || ( !EnoughAmmo( pSoldier, FALSE, pBestThrow->bWeaponIn) && FindAmmoToReload( pSoldier, pBestThrow->bWeaponIn, NO_SLOT) == NO_SLOT) )
		{
			// Consider rocket launcher/cannon
			pBestThrow->bWeaponIn = FindRocketLauncherOrCannon( pSoldier );
			if ( pBestThrow->bWeaponIn == NO_SLOT || ( !EnoughAmmo( pSoldier, FALSE, pBestThrow->bWeaponIn) && FindAmmoToReload( pSoldier, pBestThrow->bWeaponIn, NO_SLOT) == NO_SLOT) )
			{
				//dnl ch63 240813
				// no rocket launcher (or empty) -- let's look for an underslung/attached GL and a launchable grenade!
				INT8 bGunSlot = FindAIUsableObjClass(pSoldier, IC_GUN);
				pSoldier->bWeaponMode = WM_ATTACHED_GL;// So that EnoughAmmo will check for a grenade not a bullet, also need in calculation during CalcBestThrow
				if(bGunSlot != NO_SLOT && IsGrenadeLauncherAttached(&pSoldier->inv[bGunSlot]) && (EnoughAmmo(pSoldier, FALSE, bGunSlot) || FindAmmoToReload(pSoldier, bGunSlot, NO_SLOT) != NO_SLOT))
					pBestThrow->bWeaponIn = bGunSlot;
				else
				{
					// no rocket launcher or attached GL, consider grenades
					pBestThrow->bWeaponIn = FindThrowableGrenade(pSoldier);
					pSoldier->bWeaponMode = WM_NORMAL;
				}
			}
			else
			{
				// Have rocket launcher... maybe have grenades as well.	which one to use?
				if ( pSoldier->aiData.bAIMorale > MORALE_WORRIED && PreRandom( 2 ) )
				{
					//dnl ch63 240813 use grenade if have one
					INT8 bGrenadeIn = FindThrowableGrenade(pSoldier);
					if(bGrenadeIn != NO_SLOT)
						pBestThrow->bWeaponIn = bGrenadeIn;
				}
			}
		}
	}

	// if the soldier does have a tossable item somewhere
	if (pBestThrow->bWeaponIn != NO_SLOT)
	{
		// if it's in his holster, swap it into his hand temporarily
		if (pBestThrow->bWeaponIn != HANDPOS)
		{
			DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"checkiftosspossible: swapping item into hand");
			RearrangePocket(pSoldier, HANDPOS, pBestThrow->bWeaponIn, TEMPORARILY);
		}


		DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"checkiftosspossible: get minapstoattack");
		// get the minimum cost to attack with this tossable item
		ubMinAPcost = MinAPsToAttack( pSoldier, pSoldier->sLastTarget, DONTADDTURNCOST,0);

		// if we can afford the minimum AP cost to throw this tossable item
		if (pSoldier->bActionPoints >= ubMinAPcost)
		{
			// then look around for a worthy target (which sets bestThrow.ubPossible)
			CalcBestThrow( pSoldier, pBestThrow );
		}

		// if it was in his holster, swap it back into his holster for now
		if (pBestThrow->bWeaponIn != HANDPOS)
		{
			DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"checkiftosspossible: swapping item into holster");
			RearrangePocket( pSoldier, HANDPOS, pBestThrow->bWeaponIn, TEMPORARILY );
		}
	}
	pSoldier->bWeaponMode = WM_NORMAL;//dnl ch63 240813
}

INT8 CountAdjacentSpreadTargets( SOLDIERTYPE * pSoldier, INT16 sFirstTarget, INT8 bTargetLevel )
{
	// return the number of people next to this guy for burst-spread purposes

	INT8	bDirLoop, bDir, bCheckDir, bTargetIndex, bTargets;
	INT16	sTarget;
	SOLDIERTYPE * pTarget, * pTargets[5] = {NULL};

	bTargetIndex = -1;
	bCheckDir = -1;

	pTargets[2] = SimpleFindSoldier( sFirstTarget, bTargetLevel );
	if (pTargets[2] == NULL)
	{
		return( 0 );
	}
	bTargets = 1;

	bDir = (INT8) GetDirectionToGridNoFromGridNo( pSoldier->sGridNo, sFirstTarget );

	for (bDirLoop = 0; bDirLoop < 8; bDirLoop++)
	{
		if (bDir % 2)
		{
			// odd direction = diagonal direction
			switch( bDirLoop )
			{
			case 0:
				bCheckDir = (bDir + 6) % 8;
				bTargetIndex = 0;
				break;
			case 1:
				bCheckDir = (bDir + 5) % 8;
				bTargetIndex = 1;
				break;
			case 2:
				bCheckDir = (bDir + 7) % 8;
				bTargetIndex = 1;
				break;
			case 3:
				bCheckDir = (bDir + 3) % 8;
				bTargetIndex = 3;
				break;
			case 4:
				bCheckDir = (bDir + 1) % 8;
				bTargetIndex = 3;
				break;
			case 5:
				bCheckDir = (bDir + 2) % 8;
				bTargetIndex = 4;
				break;
			case 6:
				// check in front
				bCheckDir = (bDir + 4) % 8;
				bTargetIndex = 1;
				break;
			case 7:
				// check behind
				bCheckDir = (bDir) % 8;
				bTargetIndex = 3;
				break;
			}
		}
		else
		{
			// even = straight
			switch( bDirLoop )
			{
			case 0:
				bCheckDir = (bDir + 5) % 8;
				bTargetIndex = 1;
				break;
			case 1:
				bCheckDir = (bDir + 6) % 8;
				bTargetIndex = 1;
				break;
			case 2:
				bCheckDir = (bDir + 7) % 8;
				bTargetIndex = 1;
				break;
			case 3:
				bCheckDir = (bDir + 3) % 8;
				bTargetIndex = 3;
				break;
			case 4:
				bCheckDir = (bDir + 2) % 8;
				bTargetIndex = 3;
				break;
			case 5:
				bCheckDir = (bDir + 1) % 8;
				bTargetIndex = 3;
				break;
			case 6:
				// check in front
				bCheckDir = (bDir + 4) % 8;
				bTargetIndex = 1;
				break;
			case 7:
				// check behind
				bCheckDir = (bDir) % 8;
				bTargetIndex = 3;
				break;

			}
		}
		if (bDirLoop == 6 && bTargets > 1)
		{
			// we're done!	otherwise we continue and try to find people in front/behind
			break;
		}
		else if (pTargets[bTargetIndex] != NULL)
		{
			continue;
		}
		sTarget = sFirstTarget + DirIncrementer[bCheckDir];
		pTarget = SimpleFindSoldier( sTarget, bTargetLevel );
		if (pTarget)
		{
			// check to see if guy is visible
			if (pSoldier->aiData.bOppList[ pTarget->ubID ] == SEEN_CURRENTLY)
			{
				pTargets[bTargetIndex] = pTarget;
				bTargets++;
			}
		}
	}
	return( bTargets - 1 );
}

INT16 CalcSpreadBurst( SOLDIERTYPE * pSoldier, INT16 sFirstTarget, INT8 bTargetLevel )
{
	INT8	bDirLoop, bDir, bCheckDir, bTargetIndex = 0, bLoop, bTargets;
	INT16	sTarget;
	SOLDIERTYPE * pTarget, * pTargets[5] = {NULL};
	INT8 bAdjacents, bOtherAdjacents;


	bCheckDir = -1;

	pTargets[2] = SimpleFindSoldier( sFirstTarget, bTargetLevel );
	if (pTargets[2] == NULL)
	{
		return( sFirstTarget );
	}
	bTargets = 1;
	bAdjacents = CountAdjacentSpreadTargets( pSoldier, sFirstTarget, bTargetLevel );

	bDir = (INT8) GetDirectionToGridNoFromGridNo( pSoldier->sGridNo, sFirstTarget );

	for (bDirLoop = 0; bDirLoop < 8; bDirLoop++)
	{
		if (bDir % 2)
		{
			// odd direction = diagonal direction
			switch( bDirLoop )
			{
			case 0:
				bCheckDir = (bDir + 6) % 8;
				bTargetIndex = 0;
				break;
			case 1:
				bCheckDir = (bDir + 5) % 8;
				bTargetIndex = 1;
				break;
			case 2:
				bCheckDir = (bDir + 7) % 8;
				bTargetIndex = 1;
				break;
			case 3:
				bCheckDir = (bDir + 3) % 8;
				bTargetIndex = 3;
				break;
			case 4:
				bCheckDir = (bDir + 1) % 8;
				bTargetIndex = 3;
				break;
			case 5:
				bCheckDir = (bDir + 2) % 8;
				bTargetIndex = 4;
				break;
			case 6:
				// check in front
				bCheckDir = (bDir + 4) % 8;
				bTargetIndex = 1;
				break;
			case 7:
				// check behind
				bCheckDir = (bDir) % 8;
				bTargetIndex = 3;
				break;
			}
		}
		else
		{
			// even = straight
			switch( bDirLoop )
			{
			case 0:
				bCheckDir = (bDir + 5) % 8;
				bTargetIndex = 1;
				break;
			case 1:
				bCheckDir = (bDir + 6) % 8;
				bTargetIndex = 1;
				break;
			case 2:
				bCheckDir = (bDir + 7) % 8;
				bTargetIndex = 1;
				break;
			case 3:
				bCheckDir = (bDir + 3) % 8;
				bTargetIndex = 3;
				break;
			case 4:
				bCheckDir = (bDir + 2) % 8;
				bTargetIndex = 3;
				break;
			case 5:
				bCheckDir = (bDir + 1) % 8;
				bTargetIndex = 3;
				break;
			case 6:
				// check in front
				bCheckDir = (bDir + 4) % 8;
				bTargetIndex = 1;
				break;
			case 7:
				// check behind
				bCheckDir = (bDir) % 8;
				bTargetIndex = 3;
				break;

			}
		}
		if (bDirLoop == 6 && bTargets > 1)
		{
			// we're done!	otherwise we continue and try to find people in front/behind
			break;
		}
		else if (pTargets[bTargetIndex] != NULL)
		{
			continue;
		}
		sTarget = sFirstTarget + DirIncrementer[bCheckDir];
		pTarget = SimpleFindSoldier( sTarget, bTargetLevel );
		if (pTarget && pSoldier->aiData.bOppList[ pTarget->ubID ] == SEEN_CURRENTLY)
		{
			bOtherAdjacents = CountAdjacentSpreadTargets( pSoldier, sTarget, bTargetLevel );
			if (bOtherAdjacents > bAdjacents)
			{
				// we should do a spread-burst there instead!
				return( CalcSpreadBurst( pSoldier, sTarget, bTargetLevel ) );
			}
			pTargets[bTargetIndex] = pTarget;
			bTargets++;
		}
	}

	if (bTargets > 1)
	{
		// Move all the locations down in the array if necessary
		// Check the 4th position
		if (pTargets[3] == NULL && pTargets[4] != NULL)
		{
			pTargets[3] = pTargets[4];
			pTargets[4] = NULL;
		}
		// Check the first two positions; we know the 3rd value is set because
		// it's our initial target
		if (pTargets[1] == NULL)
		{
			pTargets[1] = pTargets[2];
			pTargets[2] = pTargets[3];
			pTargets[3] = pTargets[4];
			pTargets[4] = NULL;
		}
		if (pTargets[0] == NULL)
		{
			pTargets[0] = pTargets[1];
			pTargets[1] = pTargets[2];
			pTargets[2] = pTargets[3];
			pTargets[3] = pTargets[4];
			pTargets[4] = NULL;
		}
		// now 50% chance to reorganize to fire in reverse order
		if (Random( 2 ))
		{
			for( bLoop = 0; bLoop < bTargets / 2; bLoop++)
			{
				pTarget = pTargets[bLoop];
				pTargets[bLoop] = pTargets[bTargets - 1 - bLoop];
				pTargets[bTargets - 1 - bLoop] = pTarget;
			}
		}
		AIPickBurstLocations( pSoldier, bTargets, pTargets );
		pSoldier->flags.fDoSpread = TRUE;
	}
	return( sFirstTarget );
}

INT16 AdvanceToFiringRange( SOLDIERTYPE * pSoldier, INT16 sClosestOpponent )
{
	// see how far we can go down a path and still shoot

	INT16		bAttackCost, bTrueActionPoints;
	UINT16	usActionData;

	bAttackCost = MinAPsToAttack(pSoldier, sClosestOpponent, ADDTURNCOST,pSoldier->aiData.bAimTime);

	if (bAttackCost >= pSoldier->bActionPoints)
	{
		// probably want to go as far as possible!
		// return( NOWHERE );
		return( GoAsFarAsPossibleTowards( pSoldier, sClosestOpponent, AI_ACTION_SEEK_OPPONENT ) );
	}

	bTrueActionPoints = pSoldier->bActionPoints;

	pSoldier->bActionPoints -= bAttackCost;

	usActionData = GoAsFarAsPossibleTowards( pSoldier, sClosestOpponent, AI_ACTION_SEEK_OPPONENT );
	//POSSIBLE STRUCTURE PROBLEM HERE.  GOTTHARD 7/15/08
	pSoldier->bActionPoints = bTrueActionPoints;

	return( usActionData );

}

void CheckIfShotPossible(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestShot)
{
	INT16 ubMinAPcost;
	pBestShot->ubPossible = FALSE;
	pBestShot->bWeaponIn = FindAIUsableObjClass(pSoldier, IC_GUN);

	// if the soldier does have a gun
	if (pBestShot->bWeaponIn != NO_SLOT)
	{
		// if it's in his holster, swap it into his hand temporarily
		if (pBestShot->bWeaponIn != HANDPOS)
		{
			RearrangePocket(pSoldier, HANDPOS, pBestShot->bWeaponIn, TEMPORARILY);
		}

		// get the minimum cost to attack with this item
		ubMinAPcost = MinAPsToAttack(pSoldier, pSoldier->sLastTarget, ADDTURNCOST, 0);

		// if we can afford the minimum AP cost
		if (pSoldier->bActionPoints >= ubMinAPcost)
		{
			// then look around for a worthy target (which sets bestThrow.ubPossible)
			CalcBestShot(pSoldier, pBestShot);
		}

		// if it was in his holster, swap it back into his holster for now
		if (pBestShot->bWeaponIn != HANDPOS)
		{
			RearrangePocket(pSoldier, HANDPOS, pBestShot->bWeaponIn, TEMPORARILY);
		}

		// try to use sidearm
		if (pSoldier->bActionPoints < ubMinAPcost && IS_MERC_BODY_TYPE(pSoldier))
		{
			pBestShot->bWeaponIn = FindAIUsableObjClass(pSoldier, IC_GUN, TRUE);

			if (pBestShot->bWeaponIn != NO_SLOT)
			{
				// if it's in his holster, swap it into his hand temporarily
				if (pBestShot->bWeaponIn != HANDPOS)
				{
					RearrangePocket(pSoldier, HANDPOS, pBestShot->bWeaponIn, TEMPORARILY);
				}

				// get the minimum cost to attack with this item
				ubMinAPcost = MinAPsToAttack(pSoldier, pSoldier->sLastTarget, ADDTURNCOST, 0);

				if (pSoldier->bActionPoints >= ubMinAPcost)
				{
					// then look around for a worthy target (which sets bestThrow.ubPossible)
					CalcBestShot(pSoldier, pBestShot);
				}

				// if it was in his holster, swap it back into his holster for now
				if (pBestShot->bWeaponIn != HANDPOS)
				{
					RearrangePocket(pSoldier, HANDPOS, pBestShot->bWeaponIn, TEMPORARILY);
				}
			}
		}
	}
}

// SANDRO - function to determine if we should try to steal the enemy gun
BOOLEAN AIDetermineStealingWeaponAttempt( SOLDIERTYPE * pSoldier, SOLDIERTYPE * pOpponent )
{
	INT16 sChance = 0;
	UINT32 uiSuccessChance = 0;

	if( pOpponent == NULL )
	{
		return( FALSE );
	}

	if( pOpponent->bTeam != gbPlayerNum )
	{
		return( FALSE );
	}

	if( pOpponent->bCollapsed || pOpponent->bBreathCollapsed )
	{
		return( FALSE );
	}

	// sevenfm: don't steal weapons from cowering or prone enemy to prevent possible bug
	if( pOpponent->usAnimState == COWERING_PRONE || 
		pOpponent->usAnimState == COWERING || 
		gAnimControl[ pOpponent->usAnimState ].ubEndHeight == ANIM_PRONE )
	{
		return( FALSE );
	}

	pSoldier->usUIMovementMode = RUNNING;
	if( pSoldier->bActionPoints < GetAPsToStealItem( pSoldier, NULL, pOpponent->sGridNo ) )
	{
		return( FALSE );
	}

	if( (pOpponent->inv[HANDPOS].exists() != true) )	
	{
		return( FALSE );
	}
	if ( !(Item[pOpponent->inv[HANDPOS].usItem].usItemClass & IC_WEAPON) )
	{
		UINT16 dfgvdfv = Item[pOpponent->inv[HANDPOS].usItem].usItemClass;
		return( FALSE );
	}

	uiSuccessChance = CalcChanceToSteal(pSoldier, pOpponent, 0);
	if ( uiSuccessChance >= 100 )
	{
		sChance = 90;
	}
	else if ( uiSuccessChance >= 85 ) 
	{
		sChance = 75;
	}
	else if ( uiSuccessChance >= 70 ) 
	{
		sChance = 60;
	}
	else if ( uiSuccessChance >= 50 ) 
	{
		sChance = 40;
	}
	else if ( uiSuccessChance >= 25 ) 
	{
		sChance = 15;
	}
	else 
	{
		return( FALSE );	
	}

	if( gGameOptions.fNewTraitSystem )
	{
		if( !HAS_SKILL_TRAIT( pSoldier, MARTIAL_ARTS_NT ) )
		{
			return( FALSE );
		}
		else if( NUM_SKILL_TRAITS( pSoldier, MARTIAL_ARTS_NT ) > 1 )
		{
			sChance += 50;
		}
	}
	else 
	{
		if( !HAS_SKILL_TRAIT( pSoldier, MARTIALARTS_OT ) && !HAS_SKILL_TRAIT( pSoldier, HANDTOHAND_OT ) )
		{
			return( FALSE );
		}
		else if(( NUM_SKILL_TRAITS( pSoldier, MARTIALARTS_OT ) > 1 ) || ( NUM_SKILL_TRAITS( pSoldier, HANDTOHAND_OT ) > 1 ))
		{
			sChance += 25;
		}
	}

	if( pSoldier->stats.bLife < pSoldier->stats.bLifeMax )
	{
		sChance -= (pSoldier->stats.bLifeMax - pSoldier->stats.bLife);
	}
	if( pSoldier->bBreath < pSoldier->bBreathMax )
	{
		sChance -= ((pSoldier->bBreathMax - pSoldier->bBreath) / 2);
	}

	if( pOpponent->stats.bLife < pOpponent->stats.bLifeMax )
	{
		sChance += (pOpponent->stats.bLifeMax - pOpponent->stats.bLife);
	}
	if( pOpponent->bBreath < pOpponent->bBreathMax )
	{
		sChance += ((pOpponent->bBreathMax - pOpponent->bBreath) / 2);
	}

	if( pSoldier->bActionPoints > (GetAPsToStealItem( pSoldier, NULL, pOpponent->sGridNo ) +  (2 * ApsToPunch( pSoldier ))) )
	{
		sChance += 35;
	}
	else if ( pSoldier->bActionPoints > (GetAPsToStealItem( pSoldier, NULL, pOpponent->sGridNo ) +  ApsToPunch( pSoldier )) )
	{
		sChance += 20;
	}
	else 
	{
		sChance -= 10;
	}

	if( Chance( sChance ) )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );	
	}
}

// HEADROCK HAM 4: This is required for the AI to be able to assess the length of autofire volleys using the new
// recoil system. 
FLOAT AICalcRecoilForShot( SOLDIERTYPE *pSoldier, OBJECTTYPE *pWeapon, UINT8 ubShotNum)
{
	FLOAT bRecoilX = 0;
	FLOAT bRecoilY = 0;
	GetRecoil( pSoldier, pWeapon, &bRecoilX, &bRecoilY, ubShotNum );
	// Return average shooter's ability to control this gun.
	// HEADROCK HAM 4: TODO: Incorporate items that alter max counter-force.
	FLOAT AverageRecoil = __max(0, ( sqrt( (bRecoilX * bRecoilX) + (bRecoilY * bRecoilY) ) - (gGameCTHConstants.RECOIL_MAX_COUNTER_FORCE * 0.7f) ) );
	return AverageRecoil;
}

//dnl ch61 180813
UnderFire gUnderFire;

void UnderFire::Clear(void)
{
	usUnderFireCnt = 0;
	memset(usUnderFireID, 0, sizeof(usUnderFireID));
	memset(ubUnderFireCTH, 0, sizeof(ubUnderFireCTH));
}

void UnderFire::Add(UINT16 usID, UINT8 ubCTH)
{
	if (!fEnable)
		return;

	if (usUnderFireCnt < MAXUNDERFIRE)
	{
		for (int i = 0; i < usUnderFireCnt; i++)
		{
			if (usUnderFireID[i] == usID)
			{
				if (ubUnderFireCTH[i] < ubCTH)		// sevenfm: use max value
					ubUnderFireCTH[i] = ubCTH;
				return;
			}
		}
		ubUnderFireCTH[usUnderFireCnt] = ubCTH;		// sevenfm: store CTH too!
		usUnderFireID[usUnderFireCnt] = usID;
		usUnderFireCnt++;
	}
}

UINT16 UnderFire::Count(INT8 bTeam)
{
	UINT16 cnt = 0;
	for (UINT16 i = 0; i < usUnderFireCnt; i++)
	{
		if (MercPtrs[usUnderFireID[i]]->bTeam == bTeam)
			++cnt;
	}
	return(cnt);
}

UINT8 UnderFire::Chance(INT8 bTeam, INT8 bSide, BOOLEAN fCheckNeutral)
{
	UINT8 cth = 0;
	for (UINT16 i = 0; i < usUnderFireCnt; i++)
	{
		if ((MercPtrs[usUnderFireID[i]]->bTeam == bTeam || MercPtrs[usUnderFireID[i]]->bSide == bSide || fCheckNeutral && MercPtrs[usUnderFireID[i]]->aiData.bNeutral) &&
			ubUnderFireCTH[i] > cth)
		{
			cth = ubUnderFireCTH[i];
		}
	}
	return(cth);
}

// Flugente AI functions
// determine a gridno that would allow us to hit as many enemies as possible given an effect with radius aRadius tiles
// return true if sufficient gridno is found
// pGridNo will be the GridNo
// aRadius is the area effect radius to use
// uCheckFriends: 0 - do not consider friends at all 1 - consider with negative weight else: ignore any location that might also hit friends
// success only if at a rating of at least aMinRating can be achieved
// any enemy soldiers not fulfilling cond will be excluded from this calculation
// if an enemy soldier fulfills taboo, make sure to not hit him at all!
#define MAX_EXCLUDE_TILES 100
BOOLEAN GetBestAoEGridNo(SOLDIERTYPE *pSoldier, INT32* pGridNo, INT16 aRadius, UINT8 uCheckFriends, UINT8 aMinRating, SOLDIER_CONDITION cond, SOLDIER_CONDITION taboo)
{
	UINT8	ubLoop, ubLoop2;
	INT32	sGridNo, sFriendTile[MAXMERCS], sOpponentTile[MAXMERCS], sTabooTile[MAXMERCS];
	UINT8	ubFriendCnt = 0,ubOpponentCnt = 0, ubTabooCnt = 0, ubOpponentID[MAXMERCS];
	INT32	bMaxLeft,bMaxRight,bMaxUp,bMaxDown, i, j;
	//INT8	bPersOL, bPublOL;
	INT8	bKnowledge;
	SOLDIERTYPE *pFriend;
	static INT16	sExcludeTile[MAX_EXCLUDE_TILES]; // This array is for storing tiles that we have already considered, to prevent duplication of effort
	UINT8	ubNumExcludedTiles = 0;

	INT32 lowestX  = 999999;
	INT32 highestX = 0;
	INT32 lowestY  = 999999;
	INT32 highestY = 0;
	
	// make lists of enemies and friends
	for (ubLoop = 0; ubLoop < guiNumMercSlots; ++ubLoop)
	{
		pFriend = MercSlots[ubLoop];

		if ( !pFriend || !pFriend->bActive || !pFriend->bInSector )
			continue;

		if (pFriend->stats.bLife == 0)
			continue;

		// dying or captured friends are 'helpless' anyway, we are willing to sacrifice them :-)
		if ( uCheckFriends && pSoldier->bSide == pFriend->bSide && pFriend->stats.bLife > OKLIFE && !(pFriend->usSoldierFlagMask & SOLDIER_POW) )
		{
			// active friend, remember where he is so that we DON'T blow him up!
			// this includes US, since we don't want to blow OURSELVES up either
			sFriendTile[ubFriendCnt] = pFriend->sGridNo;
			ubFriendCnt++;
		}
		else
		{
			// if an enemy fulfills taboo, we will remember his tile and be careful not to ever hit it!
			if ( taboo(pFriend) )
			{
				sTabooTile[ubTabooCnt] = pFriend->sGridNo;
				++ubTabooCnt;
				continue;
			}

			// check whether this guy fulfills the target condition
			if (!cond(pFriend))
				continue;

			if (!ValidOpponent(pSoldier, pFriend))
				continue;

			//bPersOL = pSoldier->aiData.bOppList[pFriend->ubID];
			//bPublOL = gbPublicOpplist[pSoldier->bTeam][pFriend->ubID];
			bKnowledge = Knowledge(pSoldier, pFriend->ubID);

			if (bKnowledge == SEEN_CURRENTLY)
			{
				// active KNOWN opponent, remember where he is so that we DO blow him up!
				sOpponentTile[ubOpponentCnt] = pFriend->sGridNo;
			}
			//else if ( bKnowledge == SEEN_LAST_TURN || bKnowledge == HEARD_LAST_TURN || bKnowledge == HEARD_THIS_TURN || bKnowledge == SEEN_THIS_TURN)
			else if (bKnowledge >= HEARD_2_TURNS_AGO && bKnowledge <= SEEN_2_TURNS_AGO)
			{
				sOpponentTile[ubOpponentCnt] = KnownLocation(pSoldier, pFriend->ubID);
				/*sOpponentTile[ubOpponentCnt] = gsLastKnownOppLoc[pSoldier->ubID][pFriend->ubID];
				// cheat; only allow throw if person is REALLY within 2 tiles of where last seen
				if ( SpacesAway( pFriend->sGridNo, gsLastKnownOppLoc[ pSoldier->ubID ][ pFriend->ubID ] ) < 3 )
				{
				sOpponentTile[ubOpponentCnt] = gsLastKnownOppLoc[ pSoldier->ubID ][ pFriend->ubID ];
				}*/
			}
			else
			{
				continue;
			}

			// also remember who he is (which soldier #)
			ubOpponentID[ubOpponentCnt] = pFriend->ubID;

			// update lowest and highest x and y values
			lowestX  = min(lowestX,  sOpponentTile[ubOpponentCnt] % MAXCOL );
			highestX = max(highestX, sOpponentTile[ubOpponentCnt] % MAXCOL );
			lowestY  = min(lowestY,  sOpponentTile[ubOpponentCnt] / MAXCOL );
			highestY = max(highestY, sOpponentTile[ubOpponentCnt] / MAXCOL );

			ubOpponentCnt++;
		}
	}

	// no/not enough enemies found -> no area effect location advisable
	if ( !ubOpponentCnt || ubOpponentCnt < aMinRating )
		return FALSE;

	BOOLEAN fGridNoFound = FALSE;
	INT32 bestGridNo = -1;
	INT8 bestGridNoCnt = aMinRating;

	INT32 currentSoldierGridNo = -1;	

	INT8 enemiesnear = 0;
	INT8 friendsnear = 0;
		
	// look at the squares near each known opponent and try to find the one
	// place where a tossed projectile would do the most harm to the opponents
	// while avoiding one's friends
	for (ubLoop = 0; ubLoop < ubOpponentCnt; ++ubLoop)
	{
		currentSoldierGridNo = sOpponentTile[ubLoop];

		// determine maximum horizontal limits
		bMaxLeft  = max(currentSoldierGridNo % MAXCOL - aRadius, lowestX);
		bMaxRight = min(currentSoldierGridNo % MAXCOL + aRadius, highestX);

		// determine maximum vertical limits
		bMaxDown  = max(currentSoldierGridNo / MAXCOL - aRadius, lowestY);
		bMaxUp	  = min(currentSoldierGridNo / MAXCOL + aRadius, highestY);

		// evaluate every tile for its opponent-damaging potential
		for (i = bMaxLeft; i <= bMaxRight; ++i)
		{
			for (j = bMaxDown; j <= bMaxUp; ++j)
			{
				// calculate the next potential gridno near this opponent
				sGridNo = i + (MAXCOL * j);

				// this shouldn't ever happen
				if ((sGridNo < 0) || (sGridNo >= GRIDSIZE))
					continue;

				// sevenfm: avoid attacking spots outside of visible world to prevent possible bugs
				if (!GridNoOnVisibleWorldTile(sGridNo))
					continue;

				if (PythSpacesAway(currentSoldierGridNo, sGridNo) > aRadius)
					continue;

				// if this tile is taboo, don't even think about targeting it!
				for (ubLoop2 = 0; ubLoop2 < ubTabooCnt; ++ubLoop2)
				{
					if (sTabooTile[ubLoop2] == sGridNo)
						continue;
				}
								
				// Check to see if we have considered this tile before:
				for (ubLoop2 = 0; ubLoop2 < ubNumExcludedTiles; ++ubLoop2)
				{
					if (sExcludeTile[ubLoop2] == sGridNo)
						continue;
				}

				// add this tile to the list of already checked tiles
				if (ubNumExcludedTiles < MAX_EXCLUDE_TILES)
				{
					sExcludeTile[ubNumExcludedTiles] = sGridNo;
					++ubNumExcludedTiles;
				}
				
				// loop over all enemies and friends to determine how many are in range
				enemiesnear = 0;
				friendsnear = 0;

				// check whether there are any friends near this gridno
				for (ubLoop2 = 0; ubLoop2 < ubFriendCnt; ++ubLoop2)
				{
					if ( PythSpacesAway(sFriendTile[ubLoop2], sGridNo) <= aRadius )
						++friendsnear;
				}

				// ignore this location if friends are found and we want to absolutely ignore friendly fire
				if ( friendsnear && uCheckFriends > 1 )
					continue;

				// check whether there are any enemies near this gridno
				for (ubLoop2 = 0; ubLoop2 < ubOpponentCnt; ++ubLoop2)
				{
					if ( PythSpacesAway(sOpponentTile[ubLoop2], sGridNo) <= aRadius )
					{
						++enemiesnear;
					}
				}

				if ( enemiesnear - friendsnear >= bestGridNoCnt )
				{
					bestGridNoCnt = enemiesnear - friendsnear;

					bestGridNo = sGridNo;
					fGridNoFound = TRUE;
				}		
			}
		}
	}

	*pGridNo = bestGridNo;

	return fGridNoFound;
}

// Get the ID of the farthest opponent  we can see, with an optional minimum range
// puID - ID of the farthest opponent pSoldier can see
// sRange - only return an true and give an idea if opponent found is further away than this
BOOLEAN GetFarthestOpponent(SOLDIERTYPE *pSoldier, UINT8* puID, INT16 sRange)
{
	INT32 sGridNo;
	UINT32 uiLoop;
	INT32 iRange = 0;;
	INT8	*pbPersOL;
	SOLDIERTYPE * pOpponent;
	BOOLEAN found = FALSE;
	
	*puID = NOBODY;

	// look through this man's personal & public opplists for opponents known
	for (uiLoop = 0; uiLoop < guiNumMercSlots; ++uiLoop)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpponent)
		{
			continue;			// next merc
		}

		if (!ValidOpponent(pSoldier, pOpponent))
		{
			continue;
		}

		pbPersOL = pSoldier->aiData.bOppList + pOpponent->ubID;

		// if this opponent is not seen personally
		if (*pbPersOL != SEEN_CURRENTLY)
		{
			continue;			// next merc
		}

		// since we're dealing with seen people, use exact gridnos
		sGridNo = pOpponent->sGridNo;

		// if we are standing at that gridno(!, obviously our info is old...)
		if (sGridNo == pSoldier->sGridNo)
		{
			continue;			// next merc
		}

		// I hope this will be good enough; otherwise we need a fractional/world-units-based 2D distance function
		//sRange = PythSpacesAway( pSoldier->sGridNo, sGridNo);
		iRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, sGridNo );

		if (iRange > sRange)
		{
			sRange = iRange;
			*puID = uiLoop;
			found = TRUE;
		}
	}

	return( found );
}

// are there more allies than friends in adjacent sectors?
BOOLEAN MoreFriendsThanEnemiesinNearbysectors(UINT8 ausTeam, INT16 aX, INT16 aY, INT8 aZ)
{
	UINT16 enemyteam = NumEnemiesInFiveSectors(aX, aY) - NumEnemiesInAnySector(aX, aY, aZ);
	UINT16 militiateam = CountAllMilitiaInFiveSectors(aX, aY) - CountAllMilitiaInSector(aX, aY);

	if ( ausTeam == ENEMY_TEAM )
		return (enemyteam > militiateam);

	return (militiateam > enemyteam);
}

// check if we can toss grenade at spot, and prepare attack data
// grenade should be in hand
void CheckTossAt(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestThrow, INT32 sTargetSpot, INT8 bTargetLevel, UINT8 ubOpponentID)
{
	UINT16	usInHand, usGrenade;
	INT32	iTossRange;

	INT32	sEndSpot = NOWHERE;
	INT8	bEndLevel = 0;

	UINT8	ubAPCost;
	UINT8	ubChanceToHit;
	UINT8	ubChanceToReallyHit;
	UINT8	ubChanceToGetThrough;
	INT32	iHitRate;
	INT32	iAttackValue;
	INT32	iTotalThreatValue = 100;
	UINT8	ubMaxPossibleAimTime = 0;
	UINT16	usTrueState = pSoldier->usAnimState;
	UINT8	ubStance = gAnimControl[pSoldier->usAnimState].ubEndHeight;

	usInHand = pSoldier->inv[HANDPOS].usItem;

	// initialize
	pBestThrow->ubPossible = FALSE;
	pBestThrow->ubChanceToReallyHit = 0;
	pBestThrow->iAttackValue = 0;

	//DebugShot( pSoldier, String("check toss at %d %d", sTargetSpot, bTargetLevel));

	// sevenfm: safety check
	if (TileIsOutOfBounds(sTargetSpot) || !GridNoOnVisibleWorldTile(sTargetSpot))
	{
		return;
	}

	iTossRange = CalcMaxTossRange(pSoldier, usInHand, TRUE);
	//DebugShot( pSoldier, String("toss range %d", iTossRange));

	usGrenade = pSoldier->inv[HANDPOS].usItem;
	//DebugShot( pSoldier, String("grenade %d", usGrenade));

	ubChanceToGetThrough = 100 * CalculateLaunchItemChanceToGetThrough(pSoldier, &pSoldier->inv[HANDPOS], sTargetSpot, bTargetLevel, 0, &sEndSpot, TRUE, &bEndLevel, FALSE);
	//DebugShot( pSoldier, String("CTGT %d end spot %d end level %d", ubChanceToGetThrough, sEndSpot, bEndLevel));

	ubAPCost = (UINT8)MinAPsToThrow(pSoldier, sTargetSpot, TRUE) + CalcAPCostForAiming(pSoldier, sTargetSpot, ubMaxPossibleAimTime);
	//DebugShot( pSoldier, String("AP cost %d", ubAPCost));

	ubChanceToHit = (UINT8)CalcThrownChanceToHit(pSoldier, sTargetSpot, 0, AIM_SHOT_TORSO);
	//DebugShot( pSoldier, String("CTH %d", ubChanceToHit));

	ubChanceToReallyHit = (ubChanceToHit * ubChanceToGetThrough) / 100;
	//DebugShot( pSoldier, String("chance to really hit %d", ubChanceToReallyHit));

	iHitRate = (pSoldier->bActionPoints * ubChanceToHit) / ubAPCost;
	//DebugShot( pSoldier, String("hit rate %d", iHitRate));

	iAttackValue = (iHitRate * ubChanceToReallyHit * iTotalThreatValue) / 1000;
	//DebugShot( pSoldier, String("attack value %d", iAttackValue));

	// maybe try to stand up for better range
	if (ubChanceToReallyHit == 0 &&
		gAnimControl[pSoldier->usAnimState].ubEndHeight < ANIM_STAND &&
		pSoldier->InternalIsValidStance(AIDirection(pSoldier->sGridNo, sTargetSpot), ANIM_STAND) &&
		pSoldier->bActionPoints >= ubAPCost + GetAPsToChangeStance(pSoldier, ANIM_STAND))
	{
		pSoldier->usAnimState = STANDING;

		iTossRange = CalcMaxTossRange(pSoldier, usInHand, TRUE);
		//DebugShot( pSoldier, String("toss range %d", iTossRange));

		usGrenade = pSoldier->inv[HANDPOS].usItem;
		//DebugShot( pSoldier, String("grenade %d", usGrenade));

		ubChanceToGetThrough = 100 * CalculateLaunchItemChanceToGetThrough(pSoldier, &pSoldier->inv[HANDPOS], sTargetSpot, bTargetLevel, 0, &sEndSpot, TRUE, &bEndLevel, FALSE);
		//DebugShot( pSoldier, String("CTGT %d end spot %d end level %d", ubChanceToGetThrough, sEndSpot, bEndLevel));

		ubAPCost = (UINT8)MinAPsToThrow(pSoldier, sTargetSpot, TRUE) + CalcAPCostForAiming(pSoldier, sTargetSpot, ubMaxPossibleAimTime);
		//DebugShot( pSoldier, String("AP cost %d", ubAPCost));

		ubChanceToHit = (UINT8)CalcThrownChanceToHit(pSoldier, sTargetSpot, 0, AIM_SHOT_TORSO);
		//DebugShot( pSoldier, String("CTH %d", ubChanceToHit));

		ubChanceToReallyHit = (ubChanceToHit * ubChanceToGetThrough) / 100;
		//DebugShot( pSoldier, String("chance to really hit %d", ubChanceToReallyHit));

		iHitRate = (pSoldier->bActionPoints * ubChanceToHit) / ubAPCost;
		//DebugShot( pSoldier, String("hit rate %d", iHitRate));

		iAttackValue = (iHitRate * ubChanceToReallyHit * iTotalThreatValue) / 1000;
		//DebugShot( pSoldier, String("attack value %d", iAttackValue));

		pSoldier->usAnimState = usTrueState;

		ubStance = ANIM_STAND;
		ubAPCost += GetAPsToChangeStance(pSoldier, ANIM_STAND);
	}

	if (ubChanceToReallyHit > 0)
	{
		// OOOF!	That was a lot of work!	But we've got a new best target!
		pBestThrow->ubPossible = TRUE;
		pBestThrow->ubOpponent = ubOpponentID;
		pBestThrow->ubAimTime = ubMaxPossibleAimTime;
		pBestThrow->ubChanceToReallyHit = ubChanceToReallyHit;
		pBestThrow->sTarget = sTargetSpot;
		pBestThrow->iAttackValue = iAttackValue;
		pBestThrow->ubAPCost = ubAPCost;
		pBestThrow->bTargetLevel = bTargetLevel;
		pBestThrow->ubStance = ubStance;

		//DebugShot( pSoldier, String("throw possible: opponent %d, aim time %d, CTH %d, Target %d, Level %d, AP cost %d", pBestThrow->ubOpponent, pBestThrow->ubAimTime, pBestThrow->ubChanceToReallyHit, pBestThrow->sTarget, pBestThrow->bTargetLevel, pBestThrow->ubAPCost));
	}
	else
	{
		//DebugShot(pSoldier, String("throw not possible"));
	}
}

void CheckTossFlankFence(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestThrow)
{
	//ScreenMsg(FONT_KHAKI, MSG_INTERFACE, L"[%d] Check Toss Flank Fence", pSoldier->ubID);
	//DebugShot(pSoldier, String("\nCheckTossFlankFence"));

	INT16 ubMinAPcost;
	INT8 bGrenadeIn = NO_SLOT;

	// initialize
	pBestThrow->ubPossible = FALSE;
	pBestThrow->ubChanceToReallyHit = 0;
	pBestThrow->iAttackValue = 0;
	pBestThrow->ubOpponent = NOBODY;

	if (!IS_MERC_BODY_TYPE(pSoldier))
	{
		//DebugShot(pSoldier, String("not merc body type"));
		return;
	}

	pSoldier->bWeaponMode = WM_NORMAL;

	bGrenadeIn = FindThrowableGrenade(pSoldier, EXPLOSV_NORMAL, 10);

	if (bGrenadeIn != NO_SLOT)
	{
		pBestThrow->bWeaponIn = bGrenadeIn;
		//DebugShot(pSoldier, String("found grenade in slot %d", pBestThrow->bWeaponIn));

		// if it's in his holster, swap it into his hand temporarily
		if (pBestThrow->bWeaponIn != HANDPOS)
		{
			//DebugShot(pSoldier, String("rearrange pocket"));
			RearrangePocket(pSoldier, HANDPOS, pBestThrow->bWeaponIn, TEMPORARILY);
		}

		// get the minimum cost to attack with this tossable item
		ubMinAPcost = MinAPsToAttack(pSoldier, pSoldier->sGridNo, DONTADDTURNCOST, 0);
		//DebugShot(pSoldier, String("min APs to attack %d", ubMinAPcost));

		// if we can afford the minimum AP cost to throw this tossable item
		if (pSoldier->bActionPoints >= ubMinAPcost)
		{
			// get direction of position to flank from soldier's position
			UINT8 ubDesiredDir = FlankingDirection(pSoldier);

			INT32 sFenceSpot = NewGridNo(pSoldier->sGridNo, DirectionInc(ubDesiredDir));

			if (sFenceSpot != pSoldier->sGridNo &&
				(FindConcertina(sFenceSpot) || IsCuttableWireFenceAtGridNo(sFenceSpot) || FindStruct(sFenceSpot, pSoldier->pathing.bLevel, BLUEFLAG_GRAPHIC)))
			{
				CheckTossAt(pSoldier, pBestThrow, sFenceSpot, pSoldier->pathing.bLevel, NOBODY);

				// check adjacent tiles
				if (!pBestThrow->ubPossible)
				{
					INT32 sTempSpot;

					for (UINT8 ubDirection = 0; ubDirection < NUM_WORLD_DIRECTIONS; ubDirection++)
					{
						sTempSpot = NewGridNo(pSoldier->sGridNo, DirectionInc(ubDirection));

						if (sTempSpot != pSoldier->sGridNo)
						{
							CheckTossAt(pSoldier, pBestThrow, sTempSpot, pSoldier->pathing.bLevel, NOBODY);

							if (pBestThrow->ubPossible)
							{
								break;
							}
						}
					}
				}
			}

			// if it was in his holster, swap it back into his holster for now
			if (pBestThrow->bWeaponIn != HANDPOS)
			{
				//DebugShot(pSoldier, String("rearrange pocket"));
				RearrangePocket(pSoldier, HANDPOS, pBestThrow->bWeaponIn, TEMPORARILY);
			}
		}
		else
		{
			//DebugShot(pSoldier, String("not enough AP to throw grenade"));
		}
	}
	else
	{
		//DebugShot(pSoldier, String("not found grenade"));
	}

	pSoldier->bWeaponMode = WM_NORMAL;
}

void CheckTossOpponentFence(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestThrow)
{
	//ScreenMsg(FONT_KHAKI, MSG_INTERFACE, L"[%d] Check Toss Flank Fence", pSoldier->ubID);
	//DebugShot(pSoldier, String("\nCheckTossFlankFence"));

	INT16 ubMinAPcost;
	INT8 bGrenadeIn = NO_SLOT;
	INT32 sPathCost, sNewPathCost;
	INT32 sOriginalGridNo;

	// initialize
	pBestThrow->ubPossible = FALSE;
	pBestThrow->ubChanceToReallyHit = 0;
	pBestThrow->iAttackValue = 0;
	pBestThrow->ubOpponent = NOBODY;

	if (!IS_MERC_BODY_TYPE(pSoldier))
	{
		//DebugShot(pSoldier, String("not merc body type"));
		return;
	}

	INT32 sOpponentSpot;
	INT8 bOppponentLevel;
	INT32 sClosestOpponent = ClosestKnownOpponent(pSoldier, &sOpponentSpot, &bOppponentLevel);

	if (TileIsOutOfBounds(sClosestOpponent))
	{
		return;
	}

	pSoldier->bWeaponMode = WM_NORMAL;

	bGrenadeIn = FindThrowableGrenade(pSoldier, EXPLOSV_NORMAL, 10);

	/*if(bGrenadeIn == NO_SLOT)
	{
		// try to create item
		UINT16 usItem = MINI_GRENADE;
		if (usItem > 0)
		{
			OBJECTTYPE newobj;
			CreateItem(usItem, 80 + Random(20), &newobj);
			newobj.fFlags |= OBJECT_UNDROPPABLE;

			// try to place item in inventory
			if (AutoPlaceObject(pSoldier, &newobj, FALSE))
			{
				bGrenadeIn = FindThrowableGrenade(pSoldier, EXPLOSV_NORMAL, 10);
			}
		}
	}*/

	if (bGrenadeIn != NO_SLOT)
	{
		pBestThrow->bWeaponIn = bGrenadeIn;
		//DebugShot(pSoldier, String("found grenade in slot %d", pBestThrow->bWeaponIn));

		// if it's in his holster, swap it into his hand temporarily
		if (pBestThrow->bWeaponIn != HANDPOS)
		{
			//DebugShot(pSoldier, String("rearrange pocket"));
			RearrangePocket(pSoldier, HANDPOS, pBestThrow->bWeaponIn, TEMPORARILY);
		}

		// get the minimum cost to attack with this tossable item
		ubMinAPcost = MinAPsToAttack(pSoldier, pSoldier->sGridNo, DONTADDTURNCOST, 0);
		//DebugShot(pSoldier, String("min APs to attack %d", ubMinAPcost));

		// if we can afford the minimum AP cost to throw this tossable item
		if (pSoldier->bActionPoints >= ubMinAPcost)
		{
			// get direction to closest opponent
			UINT8 ubDesiredDir = AIDirection(pSoldier->sGridNo, sClosestOpponent);

			INT32 sFenceSpot = NewGridNo(pSoldier->sGridNo, DirectionInc(ubDesiredDir));
			INT32 sNextSpot = NewGridNo(sFenceSpot, DirectionInc(ubDesiredDir));

			if (sFenceSpot != pSoldier->sGridNo &&
				sNextSpot != sFenceSpot &&
				(FindConcertina(sFenceSpot) || IsCuttableWireFenceAtGridNo(sFenceSpot) || FindStruct(sFenceSpot, pSoldier->pathing.bLevel, BLUEFLAG_GRAPHIC)) &&
				IsLocationSittable(sNextSpot, pSoldier->pathing.bLevel))
			{
				sPathCost = EstimatePlotPath(pSoldier, sClosestOpponent, FALSE, FALSE, FALSE, RUNNING, pSoldier->bStealthMode, FALSE, 0);
				sOriginalGridNo = pSoldier->sGridNo;
				pSoldier->sGridNo = sNextSpot;
				sNewPathCost = EstimatePlotPath(pSoldier, sClosestOpponent, FALSE, FALSE, FALSE, RUNNING, pSoldier->bStealthMode, FALSE, 0);
				pSoldier->sGridNo = sOriginalGridNo;

				if (sNewPathCost > 0 && (sPathCost == 0 || sPathCost > sNewPathCost && sPathCost - sNewPathCost > APBPConstants[AP_MAXIMUM]))
				{
					CheckTossAt(pSoldier, pBestThrow, sFenceSpot, pSoldier->pathing.bLevel, NOBODY);
				}				
			}

			// if it was in his holster, swap it back into his holster for now
			if (pBestThrow->bWeaponIn != HANDPOS)
			{
				//DebugShot(pSoldier, String("rearrange pocket"));
				RearrangePocket(pSoldier, HANDPOS, pBestThrow->bWeaponIn, TEMPORARILY);
			}
		}
		else
		{
			//DebugShot(pSoldier, String("not enough AP to throw grenade"));
		}
	}
	else
	{
		//DebugShot(pSoldier, String("not found grenade"));
	}

	pSoldier->bWeaponMode = WM_NORMAL;
}

void CheckTossGrenadeSpecial(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestThrow)
{
	//ScreenMsg(FONT_MCOLOR_LTGREEN, MSG_INTERFACE, L"[%d] Check Toss Grenade Special", pSoldier->ubID);
	//DebugShot(pSoldier, String("\nCheckTossGrenadeSpecial"));

	INT16 ubMinAPcost;
	INT8 bGrenadeIn = NO_SLOT;
	UINT16 usGrenade;
	UINT8 ubType;
	UINT8 ubRadius;
	UINT8 ubMinDamage;

	// initialize
	pBestThrow->ubPossible = FALSE;
	pBestThrow->ubChanceToReallyHit = 0;
	pBestThrow->iAttackValue = 0;
	pBestThrow->ubOpponent = NOBODY;

	if (!IS_MERC_BODY_TYPE(pSoldier))
	{
		//DebugShot(pSoldier, String("not merc body type"));
		return;
	}

	pSoldier->bWeaponMode = WM_NORMAL;

	for (ubType = EXPLOSV_NORMAL; ubType < EXPLOSV_ANY_TYPE; ubType++)
	{
		switch (ubType)
		{
		case EXPLOSV_NORMAL:
		case EXPLOSV_STUN:
			ubMinDamage = 10; break;
		case EXPLOSV_MUSTGAS:
		case EXPLOSV_CREATUREGAS:
		case EXPLOSV_BURNABLEGAS:
			ubMinDamage = 5; break;
		case EXPLOSV_FLASHBANG:
			ubMinDamage = 1; break;
		case EXPLOSV_FLARE:
		case EXPLOSV_NOISE:
		case EXPLOSV_SMOKE:
		case EXPLOSV_SIGNAL_SMOKE:
		case EXPLOSV_TEARGAS:
			ubMinDamage = 0; break;
		default:
			ubMinDamage = 10;
		}

		bGrenadeIn = FindThrowableGrenade(pSoldier, ubType, ubMinDamage);

		if (bGrenadeIn != NO_SLOT)
		{
			pBestThrow->bWeaponIn = bGrenadeIn;
			usGrenade = pSoldier->inv[bGrenadeIn].usItem;
			//ubType = Explosive[Item[usGrenade].ubClassIndex].ubType;
			ubRadius = max(Explosive[Item[usGrenade].ubClassIndex].ubRadius, min(TACTICAL_RANGE / 4, Explosive[Item[usGrenade].ubClassIndex].ubFragRange / CELL_X_SIZE));
			//DebugShot(pSoldier, String("found grenade in slot %d", pBestThrow->bWeaponIn));

			// if it's in his holster, swap it into his hand temporarily
			if (pBestThrow->bWeaponIn != HANDPOS)
			{
				//DebugShot(pSoldier, String("rearrange pocket"));
				RearrangePocket(pSoldier, HANDPOS, pBestThrow->bWeaponIn, TEMPORARILY);
			}

			// get the minimum cost to attack with this tossable item
			ubMinAPcost = MinAPsToAttack(pSoldier, pSoldier->sGridNo, DONTADDTURNCOST, 0);
			//DebugShot(pSoldier, String("min APs to attack %d", ubMinAPcost));

			// if we can afford the minimum AP cost to throw this tossable item
			if (pSoldier->bActionPoints >= ubMinAPcost)
			{
				//DebugShot(pSoldier, String("Enough AP, find best spot for grenade"));

				INT16 sMaxLeft, sMaxRight, sMaxUp, sMaxDown, sXOffset, sYOffset;
				INT32 iSearchRange = TACTICAL_RANGE / 2;
				INT32 sSpot;
				INT32 sBestSpot = NOWHERE;
				INT8 bSoldierLevel = pSoldier->pathing.bLevel;
				INT8 bLevel = 0;	// always throw at ground level
				UINT8 ubMinDistance = TACTICAL_RANGE / 8;
				INT16 sMaxEnemyDistance = TACTICAL_RANGE * 2;
				INT32 iValue;
				INT32 iBestValue = 0;

				INT32	sClosestOpponent;
				INT32	sClosestOpponentPathCost = 0;
				INT32	sClosestOpponentStraightPathCost = 0;
				INT32	sEnemySpot = NOWHERE;
				INT8	bEnemyLevel = 0;
				INT16	sClosestOpponentDistance = 0;
				UINT8	ubSpotDir;
				INT16	sObstaclePercent = 0;
				//UINT8	ubStructureExplosionRadius = Explosive[Item[STRUCTURE_IGNITE].ubClassIndex].ubRadius;
				UINT8	ubStructureExplosionRadius = (UINT8)Explosive[Item[C1].ubClassIndex].ubRadius;
				//UINT16	usRoom;
				INT32 iRCD = RangeChangeDesire(pSoldier);

				sClosestOpponent = ClosestKnownOpponent(pSoldier, &sEnemySpot, &bEnemyLevel);

				if (!TileIsOutOfBounds(sEnemySpot) && PythSpacesAway(pSoldier->sGridNo, sEnemySpot) < DAY_VISION_RANGE * 2)
				{
					sClosestOpponentPathCost = PlotPath(pSoldier, sEnemySpot, FALSE, FALSE, FALSE, RUNNING, 0, FALSE, 0);
					sClosestOpponentDistance = PythSpacesAway(pSoldier->sGridNo, sEnemySpot);
					sClosestOpponentStraightPathCost = sClosestOpponentDistance * (APBPConstants[AP_MOVEMENT_FLAT] + APBPConstants[AP_MODIFIER_RUN]);
					sObstaclePercent = min(100, 100 * abs(sClosestOpponentPathCost - sClosestOpponentStraightPathCost) / sClosestOpponentStraightPathCost);
				}
				//ScreenMsg(FONT_ORANGE, MSG_INTERFACE, L"[%d] enemy %d level %d path cost %d distance %d straight cost %d", pSoldier->ubID, sEnemySpot, bEnemyLevel, sClosestOpponentPathCost, sClosestOpponentDistance, sClosestOpponentStraightPathCost);

				// determine maximum horizontal limits
				sMaxLeft = min(iSearchRange, (pSoldier->sGridNo % MAXCOL));
				sMaxRight = min(iSearchRange, MAXCOL - ((pSoldier->sGridNo % MAXCOL) + 1));

				// determine maximum vertical limits
				sMaxUp = min(iSearchRange, (pSoldier->sGridNo / MAXROW));
				sMaxDown = min(iSearchRange, MAXROW - ((pSoldier->sGridNo / MAXROW) + 1));

				//DebugShot(pSoldier, String("search range %d max left %d max right %d max up %d max down %d", iSearchRange, sMaxLeft, sMaxRight, sMaxUp, sMaxDown));

				// check all tiles in range
				for (sYOffset = -sMaxUp; sYOffset <= sMaxDown; sYOffset++)
				{
					for (sXOffset = -sMaxLeft; sXOffset <= sMaxRight; sXOffset++)
					{
						sSpot = pSoldier->sGridNo + sXOffset + (MAXCOL * sYOffset);

						if (TileIsOutOfBounds(sSpot))
						{
							continue;
						}

						// basic checks
						if (Water(sSpot, bLevel))
						{
							continue;
						}

						// too close to soldier or any friend
						if (PythSpacesAway(pSoldier->sGridNo, sSpot) < ubMinDistance || CountNearbyFriends(pSoldier, sSpot, ubMinDistance) > 0)
						{
							//continue;
						}

						// check for neutrals
						if (CountNearbyNeutrals(pSoldier, sSpot, ubMinDistance))
						{
							//continue;
						}

						// if there's already active grenade there
						if (FindBombNearby(pSoldier, sSpot, 1))
						{
							continue;
						}

						iValue = 0;

						////DebugShot(pSoldier, String("check spot %d for zombie corpse", sSpot));
						// check for zombie corpse
						if (gGameSettings.fOptions[TOPTION_ZOMBIES])
						{
							INT32			cnt;
							ROTTING_CORPSE *pCorpse;
							UINT16 recanimstate;

							for (cnt = 0; cnt < giNumRottingCorpse; ++cnt)
							{
								pCorpse = &(gRottingCorpse[cnt]);

								if (pCorpse &&
									pCorpse->fActivated &&
									pCorpse->def.bLevel == bLevel &&
									pCorpse->def.sGridNo == sSpot &&
									!(pCorpse->def.usFlags & (ROTTING_CORPSE_HEAD_TAKEN | ROTTING_CORPSE_NEVER_RISE_AGAIN)) &&
									CorpseOkToSpawnZombie(pCorpse, &recanimstate) &&
									(ubType == EXPLOSV_NORMAL || ubType == EXPLOSV_BURNABLEGAS))
								{
									iValue = max(iValue, 5 * gGameExternalOptions.sZombieDifficultyLevel);
									//DebugShot(pSoldier, String("spot %d, found zombie corpse!", sSpot));
									break;
								}
							}
						}

						////DebugShot(pSoldier, String("check spot %d for blue flag", sSpot));
						// check for obstacles between soldier and closest enemy
						if (sObstaclePercent > 25 &&
							bSoldierLevel == 0 &&
							pSoldier->aiData.bOrders == SEEKENEMY &&
							!TileIsOutOfBounds(sEnemySpot) &&
							ubType == EXPLOSV_NORMAL &&
							(FindStruct(sSpot, bLevel, BLUEFLAG_GRAPHIC) || FindConcertina(sSpot) || IsCuttableWireFenceAtGridNo(sSpot)))
						{
							INT32	sTempSpot;
							UINT8	ubDirection;
							UINT8	ubCount = 0;

							// check adjacent tiles
							for (ubDirection = 0; ubDirection < NUM_WORLD_DIRECTIONS; ubDirection++)
							{
								sTempSpot = NewGridNo(sSpot, DirectionInc(ubDirection));

								if (sTempSpot != sSpot && (FindStruct(sTempSpot, bLevel, BLUEFLAG_GRAPHIC) || FindConcertina(sTempSpot) || IsCuttableWireFenceAtGridNo(sTempSpot)))
								{
									ubCount++;
								}
							}

							if(ubCount >= 2)
							{
								ubSpotDir = AIDirection(pSoldier->sGridNo, sSpot);
								iValue = max(iValue, sObstaclePercent * min(100, 10 * CountKnownEnemiesInDirection(pSoldier, ubSpotDir, sMaxEnemyDistance, FALSE) +
									5 * CountKnownEnemiesInDirection(pSoldier, gOneCDirection[ubSpotDir], sMaxEnemyDistance, FALSE) +
									5 * CountKnownEnemiesInDirection(pSoldier, gOneCCDirection[ubSpotDir], sMaxEnemyDistance, FALSE)) / 100);
							}							

							//DebugShot(pSoldier, String("spot %d, found obstacle and %d nearby! value %d opponents in dir %d", sSpot, ubCount, iValue, CountKnownEnemiesInDirection(pSoldier, ubSpotDir, sMaxEnemyDistance, TRUE)));
						}

						// too close to soldier or any friend
						if (PythSpacesAway(pSoldier->sGridNo, sSpot) < ubMinDistance || CountNearbyFriends(pSoldier, sSpot, ubMinDistance) > 0 || CountNearbyNeutrals(pSoldier, sSpot, ubMinDistance))
						{
							iValue = iValue / 2;
						}

						// for regular explosives, prefer closest spot
						/*if (ubType == EXPLOSV_NORMAL)
						{
						iValue = iValue * iSearchRange / (iSearchRange + PythSpacesAway(pSoldier->sGridNo, sSpot));
						}*/

						// found possible throw spot
						if (iValue > iBestValue)
						{
							CheckTossAt(pSoldier, pBestThrow, sSpot, bLevel, NOBODY);

							/*if (ubType == EXPLOSV_NORMAL || ubType == EXPLOSV_BURNABLEGAS)
							{
								// check adjacent tiles
								if (!pBestThrow->ubPossible)
								{
									INT32 sTempSpot;
									for (UINT8 ubDirection = 0; ubDirection < NUM_WORLD_DIRECTIONS; ubDirection++)
									{
										sTempSpot = NewGridNo(sSpot, DirectionInc(ubDirection));
										if (sTempSpot != sSpot)
										{
											CheckTossAt(pSoldier, pBestThrow, sTempSpot, bLevel, NOBODY);
											if (pBestThrow->ubPossible)
											{
												break;
											}
										}
									}
								}
							}*/

							// can throw, set best spot
							if (pBestThrow->ubPossible)
							{
								// take into account chance to really hit for all spots
								iValue = iValue * pBestThrow->ubChanceToReallyHit;
								// if still best spot to throw
								if (iValue > iBestValue)
								{
									sBestSpot = sSpot;
									iBestValue = iValue;
								}

								//DebugShot(pSoldier, String("found new best value %d spot %d", iBestValue, sBestSpot));
							}
						}
					}
				}

				// finally, prepare data for toss
				pBestThrow->ubPossible = FALSE;
				pBestThrow->ubChanceToReallyHit = 0;
				pBestThrow->iAttackValue = 0;
				pBestThrow->ubOpponent = NOBODY;

				if (!TileIsOutOfBounds(sBestSpot))
				{
					CheckTossAt(pSoldier, pBestThrow, sBestSpot, bLevel, NOBODY);
					pBestThrow->iAttackValue = iBestValue;
					//DebugShot(pSoldier, String("CheckTossAt result: possible %d cth %d attack value %d", pBestThrow->ubPossible, pBestThrow->ubChanceToReallyHit, pBestThrow->iAttackValue));
				}
				else
				{
					//DebugShot(pSoldier, String("could not find good throwing spot"));
				}
			}

			// if it was in his holster, swap it back into his holster for now
			if (pBestThrow->bWeaponIn != HANDPOS)
			{
				//DebugShot(pSoldier, String("rearrange pocket"));
				RearrangePocket(pSoldier, HANDPOS, pBestThrow->bWeaponIn, TEMPORARILY);
			}
		}
		else
		{
			//DebugShot(pSoldier, String("not found grenade"));
		}
	}

	pSoldier->bWeaponMode = WM_NORMAL;
	//ScreenMsg(FONT_MCOLOR_LTGREEN, MSG_INTERFACE, L"Check Toss Grenade Special end");
}

void CheckTossGrenadeAt(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestThrow, INT32 sTargetSpot, INT8 bTargetLevel, UINT8 ubGrenadeType)
{
	//DebugShot( pSoldier, String("\nCheckTossGrenadeAt"));

	INT16 ubMinAPcost;
	INT8 bGrenadeIn = NO_SLOT;

	// initialize
	pBestThrow->ubPossible = FALSE;
	pBestThrow->ubChanceToReallyHit = 0;
	pBestThrow->iAttackValue = 0;

	if (!IS_MERC_BODY_TYPE(pSoldier))
	{
		//DebugShot( pSoldier, String("not merc body type"));
		return;
	}

	if (TileIsOutOfBounds(sTargetSpot) || !GridNoOnVisibleWorldTile(sTargetSpot))
	{
		//DebugShot( pSoldier, String("bad sTargetSpot %d", sTargetSpot));
		return;
	}

	pSoldier->bWeaponMode = WM_NORMAL;

	bGrenadeIn = FindThrowableGrenade(pSoldier, ubGrenadeType);

	if (bGrenadeIn != NO_SLOT)
	{
		pBestThrow->bWeaponIn = bGrenadeIn;
		//DebugShot(pSoldier, String("found grenade in slot %d", pBestThrow->bWeaponIn));

		// if it's in his holster, swap it into his hand temporarily
		if (pBestThrow->bWeaponIn != HANDPOS)
		{
			//DebugShot(pSoldier, String("rearrange pocket"));
			RearrangePocket(pSoldier, HANDPOS, pBestThrow->bWeaponIn, TEMPORARILY);
		}

		// get the minimum cost to attack with this tossable item
		ubMinAPcost = MinAPsToAttack(pSoldier, pSoldier->sGridNo, DONTADDTURNCOST, 0);
		//DebugShot(pSoldier, String("min APs to attack %d", ubMinAPcost));

		// if we can afford the minimum AP cost to throw this tossable item
		if (pSoldier->bActionPoints >= ubMinAPcost)
		{
			//DebugShot(pSoldier, String("check if can throw grenade"));
			CheckTossAt(pSoldier, pBestThrow, sTargetSpot, bTargetLevel, NOBODY);
		}

		// if it was in his holster, swap it back into his holster for now
		if (pBestThrow->bWeaponIn != HANDPOS)
		{
			//DebugShot(pSoldier, String("rearrange pocket"));
			RearrangePocket(pSoldier, HANDPOS, pBestThrow->bWeaponIn, TEMPORARILY);
		}
	}
	else
	{
		//DebugShot( pSoldier, String("not found grenade"));
	}

	pSoldier->bWeaponMode = WM_NORMAL;
}
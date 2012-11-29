#ifndef ITEMS_H
#define ITEMS_H
#include "Soldier Control.h"
#include "Overhead Types.h"
#include "Weapons.h"


//forward declarations of common classes to eliminate includes
class OBJECTTYPE;
class SOLDIERTYPE;

// Snap: Moved here from opplist.cpp
// These defined are no longer used to calculate item bonuses,
// but for lack of anything better, they are still used in other places
#define NIGHTSIGHTGOGGLES_BONUS 2 * STRAIGHT_RATIO
#define UVGOGGLES_BONUS 4 * STRAIGHT_RATIO

extern UINT8 SlotToPocket[7];

extern BOOLEAN WeaponInHand( SOLDIERTYPE * pSoldier );

INT8 FindAmmo( SOLDIERTYPE * pSoldier, UINT8 ubCalibre, UINT16 ubMagSize, UINT8 ubAmmoType, INT8 bExcludeSlot );

INT8 FindBestWeaponIfCurrentIsOutOfRange(SOLDIERTYPE * pSoldier, INT8 bCurrentWeaponIndex, UINT16 bWantedRange);

INT16 FindAttachmentSlot( OBJECTTYPE* pObj, UINT16 usItem, UINT8 subObject = 0);
OBJECTTYPE* FindAttachment( OBJECTTYPE * pObj, UINT16 usItem, UINT8 subObject = 0 );
extern INT8 FindObjClass( SOLDIERTYPE * pSoldier, 	UINT32 usItemClass );
extern INT8 FindAIUsableObjClass( SOLDIERTYPE * pSoldier, 	UINT32 usItemClass );
extern INT8 FindAIUsableObjClassWithin( SOLDIERTYPE * pSoldier, 	UINT32 usItemClass, INT8 bLower, INT8 bUpper );
extern INT8 FindEmptySlotWithin( SOLDIERTYPE * pSoldier, INT8 bLower, INT8 bUpper );
extern INT8 FindObjInObjRange( SOLDIERTYPE * pSoldier, UINT16 usItem1, UINT16 usItem2 );
extern INT8 FindLaunchable( SOLDIERTYPE * pSoldier, UINT16 usWeapon );
extern INT8 FindGLGrenade( SOLDIERTYPE * pSoldier );
extern INT8 FindThrowableGrenade( SOLDIERTYPE * pSoldier );
extern INT8 FindUsableObj( SOLDIERTYPE * pSoldier, UINT16 usItem );

void DeleteObj(OBJECTTYPE * pObj );
void SwapObjs( OBJECTTYPE * pObj1, OBJECTTYPE * pObj2 );
void SwapObjs(SOLDIERTYPE* pSoldier, int leftSlot, int rightSlot, BOOLEAN fPermanent);
void SwapObjs(SOLDIERTYPE* pSoldier, int slot, OBJECTTYPE* pObject, BOOLEAN fPermanent);
void CleanUpItemStats( OBJECTTYPE * pObj );

//Returns true if swapped, false if added to end of stack
extern BOOLEAN PlaceObjectAtObjectIndex( OBJECTTYPE * pSourceObj, OBJECTTYPE * pTargetObj, UINT8 ubIndex, UINT32 ubCap );

UINT16 CalculateAmmoWeight( UINT16 usGunAmmoItem, UINT16 ubShotsLeft );
UINT16	CalculateObjectWeight( OBJECTTYPE *pObject );
UINT32 CalculateCarriedWeight( SOLDIERTYPE * pSoldier );
// CHRISL:
UINT16 CalculateItemSize( OBJECTTYPE *pObject );


extern UINT16 TotalPoints( OBJECTTYPE * pObj );
extern UINT16 UseKitPoints( OBJECTTYPE * pObj, UINT16 usPoints, SOLDIERTYPE *pSoldier );

extern BOOLEAN EmptyWeaponMagazine( OBJECTTYPE * pWeapon, OBJECTTYPE *pAmmo, UINT32 subObject = 0 );
BOOLEAN CreateItem( UINT16 usItem, INT16 bStatus, OBJECTTYPE * pObj );
BOOLEAN CreateAmmo( UINT16 usItem, OBJECTTYPE * pObj, UINT16 ubShotsLeft = 0);
BOOLEAN CreateItems( UINT16 usItem, INT8 bStatus, UINT8 ubNumber, OBJECTTYPE * pObj );
BOOLEAN CreateMoney( UINT32 uiMoney, OBJECTTYPE * pObj );
extern UINT16 DefaultMagazine( UINT16 usItem );
UINT16 RandomMagazine( UINT16 usItem, UINT8 ubPercentStandard, UINT8 maxCoolness );
UINT16 RandomMagazine( OBJECTTYPE * pGun, UINT8 ubPercentStandard, UINT8 maxCoolness );
extern BOOLEAN ReloadGun( SOLDIERTYPE * pSoldier, OBJECTTYPE * pGun, OBJECTTYPE * pAmmo, UINT32 subObject = 0 );

bool IsBackpackSlot(INT8 bSlot);
// CHRISL: Add new inventory version of ItemSlotLimit function
UINT8 ItemSlotLimit( OBJECTTYPE * pObject, INT16 bSlot, SOLDIERTYPE *pSoldier = NULL, BOOLEAN cntAttach = TRUE);

// Function to put an item in a soldier profile
// It's very primitive, just finds an empty place!
BOOLEAN PlaceObjectInSoldierProfile( UINT8 ubProfile, OBJECTTYPE *pObject );
BOOLEAN RemoveObjectFromSoldierProfile( UINT8 ubProfile, UINT16 usItem );
INT8 FindObjectInSoldierProfile( UINT8 ubProfile, UINT16 usItem );

BOOLEAN ObjectExistsInSoldierProfile( UINT8 ubProfile, UINT16 usItem );
void SetMoneyInSoldierProfile( UINT8 ubProfile, UINT32 uiMoney );

void CheckEquipmentForDamage( SOLDIERTYPE *pSoldier, INT32 iDamage );
BOOLEAN ArmBomb( OBJECTTYPE * pObj, INT8 bSetting );

// NOTE TO ANDREW: 
// 
// The following functions expect that pObj points to the object
// "in the cursor", which should have memory allocated for it already
BOOLEAN PlaceObject( SOLDIERTYPE * pSoldier, INT8 bPos, OBJECTTYPE * pObj );

BOOLEAN AutoPlaceObjectAnywhere(SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN fNewItem, INT8 bExcludeSlot = NO_SLOT);
BOOLEAN AutoPlaceObjectToWorld(SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, INT8 bVisible = TRUE);

// Send fNewItem to true to set off new item glow in inv panel
BOOLEAN AutoPlaceObject( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN fNewItem, INT8 bExcludeSlot = NO_SLOT );

//ADB
bool FitsInSmallPocket(OBJECTTYPE* pObj);

bool PlaceInAnySlot(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, bool fNewItem, int bExcludeSlot = NO_SLOT);
bool PlaceInAnyPocket(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, bool fNewItem, int bExcludeSlot = NO_SLOT);
bool PlaceInAnySmallPocket(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, bool fNewItem, int bExcludeSlot = NO_SLOT);
bool PlaceInAnyBigOrMediumPocket(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, bool fNewItem, int bExcludeSlot = NO_SLOT);
bool TryToStackInSlot(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, int bSlot);
bool TryToPlaceInSlot(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, bool fNewItem, int& startSlot, int endSlot);


void RemoveInvObject( SOLDIERTYPE *pSoldier, UINT16 usItem );
void InitItemAttachments(OBJECTTYPE* pObj);
std::vector<UINT16> GetItemSlots(OBJECTTYPE* pObj, UINT8 subObject = 0, BOOLEAN fAttachment = FALSE);
void RemoveProhibitedAttachments(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, UINT16 usItem, BOOLEAN fOnlyRemoveWhenSlotsChange = 1);
void ReInitMergedItem(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, UINT16 usOldItem);
void EjectAmmoAndPlace(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, UINT8 subObject = 0);

BOOLEAN CanItemFitInVehicle( SOLDIERTYPE *pSoldier, OBJECTTYPE *pObj, INT8 bPos, BOOLEAN fDoingPlacement );
BOOLEAN CanItemFitInPosition( SOLDIERTYPE *pSoldier, OBJECTTYPE *pObj, INT8 bPos, BOOLEAN fDoingPlacement );

void DistributeStatus(OBJECTTYPE* pSourceObject, OBJECTTYPE* pTargetObject, INT16 bMaxPoints);

UINT8 SwapKeysToSlot( SOLDIERTYPE * pSoldier, INT8 bKeyRingPosition, OBJECTTYPE * pObj );

// create a keyobject
BOOLEAN CreateKeyObject( OBJECTTYPE * pObj , UINT8 ubNumberOfKeys, UINT8 ubKeyIdValue );
BOOLEAN DeleteKeyObject( OBJECTTYPE * pObj );
BOOLEAN AllocateObject( OBJECTTYPE **pObj );

// removes a key from a *KEYRING* slot
BOOLEAN RemoveKeyFromSlot( SOLDIERTYPE * pSoldier, INT8 bKeyRingPosition, OBJECTTYPE * pObj );

// take several
BOOLEAN RemoveKeysFromSlot( SOLDIERTYPE * pSoldier, INT8 bKeyRingPosition, UINT8 ubNumberOfKeys ,OBJECTTYPE * pObj );

// add the keys to an inventory slot
UINT8 AddKeysToSlot( SOLDIERTYPE * pSoldier, INT8 bKeyRingPosition, OBJECTTYPE * pObj );



//Kris:	December 9, 1997
//I need a bunch of validation functions for ammo, attachments, etc., so I'll be adding them here.
//Chris, maybe you might find these useful, or add your own.	I don't really know what I'm doing yet,
//so feel free to correct me...

//Simple check to see if the item has any attachments
BOOLEAN ItemHasAttachments( OBJECTTYPE *pItem, SOLDIERTYPE * pSoldier = NULL, UINT8 iter = 0 );

//Determine if this item can receive this attachment.	This is different, in that it may
//be possible to have this attachment on this item, but may already have an attachment on
//it which doesn't work simultaneously with the new attachment (like a silencer and duckbill).
BOOLEAN ValidItemAttachment( OBJECTTYPE * pObj, UINT16 usAttachment, BOOLEAN fAttemptingAttachment, BOOLEAN fDisplayMessage = TRUE, UINT8 subObject = NULL, std::vector<UINT16> usAttachmentSlotIndexVector = std::vector<UINT16>());
//Return true if usAttachment would fit on an item with usItem, without considering possible other attachments on this gun. This may be inaccurate for NAS, because slots can change.
BOOLEAN ValidAttachment( UINT16 usAttachment, UINT16 usItem, UINT8* ubAPCost = NULL );
//This function does the same as the above, but is more accurate.
BOOLEAN ValidAttachment( UINT16 usAttachment, OBJECTTYPE * pObj, UINT8* ubAPCost = NULL, UINT8 subObject = 0, std::vector<UINT16> usAttachmentSlotIndexVector = std::vector<UINT16>() );
BOOLEAN ValidItemAttachmentSlot( OBJECTTYPE * pObj, UINT16 usAttachment, BOOLEAN fAttemptingAttachment, BOOLEAN fDisplayMessage = TRUE, UINT8 subObject = 0, INT16 slotCount = -1, BOOLEAN fIgnoreAttachmentInSlot = FALSE, OBJECTTYPE ** ppAttachInSlot = NULL, std::vector<UINT16> usAttachmentSlotIndexVector = std::vector<UINT16>());

// Determines if it is possible to merge an item with any item whose class 
// is the same as the indicated item
BOOLEAN ValidAttachmentClass( UINT16 usAttachment, UINT16 usItem );

//Determines if it is possible to equip this weapon with this ammo.
BOOLEAN ValidAmmoType( UINT16 usItem, UINT16 usAmmoType );

//Determines if this item is a two handed item.
BOOLEAN TwoHandedItem( UINT16 usItem );

//Existing functions without header def's, added them here, just incase I'll need to call
//them from the editor.

UINT8 AttachmentAPCost( UINT16 usAttachment, UINT16 usItem, SOLDIERTYPE * pSoldier ); // SANDRO - added argument
UINT8 AttachmentAPCost( UINT16 usAttachment, OBJECTTYPE * pObj, SOLDIERTYPE * pSoldier, UINT8 subObject = 0, std::vector<UINT16> usAttachmentSlotIndexVector = std::vector<UINT16>() );


BOOLEAN ValidLaunchable( UINT16 usLaunchable, UINT16 usItem );
UINT16 GetLauncherFromLaunchable( UINT16 usLaunchable );

BOOLEAN ValidMerge( UINT16 usMerge, UINT16 usItem );
BOOLEAN EvaluateValidMerge( UINT16 usMerge, UINT16 usItem, UINT16 * pusResult, UINT16 * pusResult2, UINT8 * pubType, UINT8 * pubAPCost );


// is the item passed a medical/ first aid kit? and what type?
INT8 IsMedicalKitItem( OBJECTTYPE *pObject );

BOOLEAN AutoReload( SOLDIERTYPE * pSoldier );
INT8 FindAmmoToReload( SOLDIERTYPE * pSoldier, INT8 bWeaponIn, INT8 bExcludeSlot );

void SwapOutHandItem( SOLDIERTYPE * pSoldier );
void SwapHandItems( SOLDIERTYPE * pSoldier );

OBJECTTYPE* FindAttachmentByClass( OBJECTTYPE * pObj, UINT32 uiItemClass, UINT8 subObject = 0 );
void WaterDamage( SOLDIERTYPE *pSoldier );

INT8 FindObj( SOLDIERTYPE * pSoldier, UINT16 usItem, INT8 bLower = 0, INT8 bUpper = NUM_INV_SLOTS );

BOOLEAN ApplyCammo( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN *pfGoodAPs );

UINT32 ConvertProfileMoneyValueToObjectTypeMoneyValue( UINT8 ubStatus );
UINT8 ConvertObjectTypeMoneyValueToProfileMoneyValue( UINT32 uiMoneyAmount );

BOOLEAN CheckForChainReaction( UINT16 usItem, INT16 bStatus, INT16 bDamage, BOOLEAN fOnGround );

BOOLEAN ItemIsLegal( UINT16 usItemIndex, BOOLEAN fIgnoreCoolness = FALSE );
BOOLEAN ExtendedGunListGun( UINT16 usGun );
UINT16 StandardGunListReplacement( UINT16 usGun );
UINT16 FindReplacementMagazine( UINT8 ubCalibre, UINT16 ubMagSize, UINT8 ubAmmoType);
UINT16 FindReplacementMagazineIfNecessary( UINT16 usOldGun, UINT16 usOldAmmo, UINT16 usNewGun );

BOOLEAN DamageItemOnGround( OBJECTTYPE * pObject, INT32 sGridNo, INT8 bLevel, INT32 iDamage, UINT8 ubOwner );

BOOLEAN ApplyCanteen( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN *pfGoodAPs );
BOOLEAN ApplyElixir( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN *pfGoodAPs );

BOOLEAN CompatibleFaceItem( UINT16 usItem1, UINT16 usItem2 );

UINT32 MoneySlotLimit( INT8 bSlot );

void CheckEquipmentForFragileItemDamage( SOLDIERTYPE *pSoldier, INT32 iDamage );

// Range of Xray device
#define XRAY_RANGE 40
// Seconds that Xray lasts
#define XRAY_TIME 5

extern void ActivateXRayDevice( SOLDIERTYPE * pSoldier );
extern void TurnOffXRayEffects( SOLDIERTYPE * pSoldier );
OBJECTTYPE* FindLaunchableAttachment( OBJECTTYPE * pObj, UINT16 usWeapon );
UINT16 FindLegalGrenade(UINT16 usItem);

void DamageObj( OBJECTTYPE * pObj, INT8 bAmount, UINT8 subObject = 0 );

UINT16 MagazineClassIndexToItemType(UINT16 usMagIndex);

// Item property-related stuff added by Madd Mugsy

BOOLEAN IsScoped( OBJECTTYPE * pObj );
BOOLEAN NCTHIsScoped( OBJECTTYPE * pObj );
INT16 GetReliability( OBJECTTYPE * pObj );
INT16 GetAimBonus( OBJECTTYPE * pObj, INT32 iRange, INT16 ubAimTime );
//WarmSteel - Used to determine the base aim bonus of the most powerful scope.
INT16 GetBaseScopeAimBonus( OBJECTTYPE * pObj, INT32 iRange );
// HEADROCK: Function to get the natural aimbonus of the weapon and its attachments
INT16 GetFlatAimBonus( OBJECTTYPE * pObj );
// HEADROCK: Function to get the final loudness value of a weapon, after reduction from its own characteristics, ammo and attachments
INT16 GetFinalLoudness( OBJECTTYPE * pObj );
//INT16 GetMinAimBonusRange( OBJECTTYPE * pObj );
INT16 GetToHitBonus( OBJECTTYPE * pObj, INT32 iRange, UINT8 bLightLevel, BOOLEAN fProneStance = FALSE );
// HEADROCK: Added alternate function that only returns the natural to-hit-bonii of a weapon, ammo and attachments
INT16 GetFlatToHitBonus( OBJECTTYPE * pObj );
// HEADROCK: Function to get average of best laser ranges from weapon and attachments
INT16 GetAverageBestLaserRange( OBJECTTYPE * pObj );
// HEADROCK: Function to get bipod bonus from weapon and its attachments
INT16 GetBipodBonus( OBJECTTYPE * pObj );
INT16 GetBurstToHitBonus( OBJECTTYPE * pObj, BOOLEAN fProneStance = FALSE );
INT16 GetAutoToHitBonus( OBJECTTYPE * pObj, BOOLEAN fProneStance = FALSE	);
INT16 GetGearAimBonus( SOLDIERTYPE * pSoldier, INT32 iRange, INT16 ubAimTime);
INT16 GetGearToHitBonus( SOLDIERTYPE * pSoldier );
INT16 GetMinRangeForAimBonus( OBJECTTYPE * pObj );

// HEADROCK: Function to get the final loudness value of a weapon, after reduction from its own characteristics, ammo and attachments
INT16 GetFinalLoudness( OBJECTTYPE * pObj );
UINT32 FindRangeBonusAttachment( OBJECTTYPE * pObj );
INT16 GetRangeBonus( OBJECTTYPE * pObj );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HEADROCK HAM 4: The following functions return the value of new NCTH-related modifiers from an item and all its
// attachments. They are stance-based, meaning that the soldier's stance determines which modifier is referenced.
// For a "default" value, feed the function a value of ubStance=ANIM_STAND.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT32 GetFlatBaseModifier( OBJECTTYPE *pObj, UINT8 ubStance );
INT32 GetPercentBaseModifier( OBJECTTYPE *pObj, UINT8 ubStance );
INT32 GetFlatAimModifier( OBJECTTYPE *pObj, UINT8 ubStance );
INT32 GetPercentAimModifier( OBJECTTYPE *pObj, UINT8 ubStance );
INT32 GetPercentCapModifier( OBJECTTYPE *pObj, UINT8 ubStance );
INT32 GetPercentHandlingModifier( OBJECTTYPE *pObj, UINT8 ubStance );
INT32 GetDropCompensationModifier( OBJECTTYPE *pObj, UINT8 ubStance );
INT32 GetCounterForceMaxModifier( OBJECTTYPE *pObj, UINT8 ubStance );
INT32 GetCounterForceAccuracyModifier( OBJECTTYPE *pObj, UINT8 ubStance );
INT32 GetCounterForceFrequencyModifier( OBJECTTYPE *pObj, UINT8 ubStance );
INT32 GetTargetTrackingModifier( OBJECTTYPE *pObj, UINT8 ubStance );
INT32 GetAimLevelsModifier( OBJECTTYPE *pObj, UINT8 ubStance );
INT32 GetAimLevelsTraitModifier( SOLDIERTYPE *pSoldier, OBJECTTYPE *pObj );

// Returns the character's stance as 0/1/2.
INT8 GetStanceModifierRef( INT8 ubStance );

INT16 GetDamageBonus( OBJECTTYPE * pObj );
INT16 GetMeleeDamageBonus( OBJECTTYPE * pObj );

INT16 GetPercentAPReduction( OBJECTTYPE * pObj );
INT16 GetPercentAutofireAPReduction( OBJECTTYPE * pObj );
INT16 GetPercentBurstFireAPReduction( OBJECTTYPE * pObj );
INT16 GetPercentReadyTimeAPReduction( OBJECTTYPE * pObj );
INT16 GetPercentReloadTimeAPReduction( OBJECTTYPE * pObj );
INT16 GetRateOfFireBonus( OBJECTTYPE * pObj );
INT16 GetBurstSizeBonus( OBJECTTYPE * pObj );
BOOLEAN HasFastBurst( OBJECTTYPE * pObj );
INT16 GetMagSizeBonus( OBJECTTYPE * pObj, UINT8 subObject = 0 );

// HEADROCK HAM 4: This function now calculates and returns the weapon's recoil as X/Y offsets.
void GetRecoil( SOLDIERTYPE *pSoldier, OBJECTTYPE *pObj, INT8 *bRecoilX, INT8 *bRecoilY, UINT8 ubNumBullet );
void GetFlatRecoilModifier( OBJECTTYPE *pObj, INT8 *bRecoilModifierX, INT8 *bRecoilModifierY );
INT16 GetPercentRecoilModifier( OBJECTTYPE *pObj );
// HEADROCK HAM 4: This function returns whether the last bullet in a burst/autofire volley was a tracer.
BOOLEAN WasPrevBulletATracer( SOLDIERTYPE *pSoldier, OBJECTTYPE *pWeapon );

// Snap: GetTotalVisionRangeBonus checks for light levels
// and returns cumulative bonus from the following functions:
//
// GetVisionRangeBonus applies to all light levels
// GetNightVisionRangeBonus: bLightLevel > NORMAL_LIGHTLEVEL_DAY, above ground only
// GetCaveVisionRangeBonus: bLightLevel > NORMAL_LIGHTLEVEL_DAY, underground only
// GetDayVisionRangeBonus: NORMAL_LIGHTLEVEL_DAY <= bLightLevel < NORMAL_LIGHTLEVEL_NIGHT
// GetBrightLightVisionRangeBonus: bLightLevel < NORMAL_LIGHTLEVEL_DAY
// (remember: darker is higher!)
INT16 GetTotalVisionRangeBonus( SOLDIERTYPE * pSoldier, UINT8 bLightLevel );
// HEADROCK: Following function used to get the vision range bonus of an item and its attachments. Second argument controls specific
// vision type.
INT16 GetItemVisionRangeBonus( OBJECTTYPE * pObj, INT16 VisionType );
// HEADROCK: This function calculates percent tunnel vision from an object and its attachments.
UINT8 GetItemPercentTunnelVision( OBJECTTYPE * pObj );

// Snap: Scale night vision bonus with light level
INT16 NightBonusScale( INT16 bonus, UINT8 bLightLevel );

UINT8 GetPercentTunnelVision( SOLDIERTYPE * pSoldier );
BOOLEAN HasThermalOptics( SOLDIERTYPE * pSoldier );

INT8 FindHearingAid( SOLDIERTYPE * pSoldier );
INT16 GetHearingRangeBonus( SOLDIERTYPE * pSoldier );
// HEADROCK: This is the same function as above, but calculates hearing range bonus without SOLDIERTYPE, from an item and its attachments
INT16 GetItemHearingRangeBonus( OBJECTTYPE * pObj );

INT8 IsGrenadeLauncher( OBJECTTYPE * pObj );
INT16 GetGrenadeLauncherStatus( OBJECTTYPE * pObj );
BOOLEAN IsGrenadeLauncherAttached( OBJECTTYPE * pObj, UINT8 subObject = 0 );
OBJECTTYPE* FindAttachment_GrenadeLauncher( OBJECTTYPE * pObj );
UINT16 GetAttachedGrenadeLauncher( OBJECTTYPE * pObj );
INT8 FindRocketLauncher( SOLDIERTYPE * pSoldier );
INT8 FindRocketLauncherOrCannon( SOLDIERTYPE * pSoldier );
INT8 FindNonSmokeLaunchable( SOLDIERTYPE * pSoldier, UINT16 usWeapon );
OBJECTTYPE* FindNonSmokeLaunchableAttachment( OBJECTTYPE * pObj, UINT16 usWeapon );
UINT16 PickARandomLaunchable(UINT16 itemIndex);
bool BadGoggles(SOLDIERTYPE *pTeamSoldier);

BOOLEAN IsFlashSuppressor( OBJECTTYPE * pObj, SOLDIERTYPE * pSoldier );
// HEADROCK: Declaration of new function to detect flash suppression without SOLDIERTYPE argument
BOOLEAN IsFlashSuppressorAlt( OBJECTTYPE * pObj );
INT16 GetFlashSuppressorStatus( OBJECTTYPE * pObj );
BOOLEAN IsRemoteDetonator( OBJECTTYPE * pObj );
BOOLEAN IsDetonator( OBJECTTYPE * pObj );
BOOLEAN IsDuckbill( OBJECTTYPE * pObj );

// Snap: Returns CUMULATIVE noise volume modifier, with component status factored in.
// Noise volume is then calculated as volume * GetPercentNoiseVolume / 100
UINT16 GetPercentNoiseVolume( OBJECTTYPE * pObj );

INT16 GetAttachedArmourBonus( OBJECTTYPE * pObj );
INT16 GetBulletSpeedBonus( OBJECTTYPE * pObj );
INT8 FindGasMask( SOLDIERTYPE * pSoldier );
INT8 FindLockBomb( SOLDIERTYPE * pSoldier );
INT8 FindMetalDetector( SOLDIERTYPE * pSoldier );
INT8 FindCannon( SOLDIERTYPE * pSoldier );
INT8 FindUsableCrowbar( SOLDIERTYPE * pSoldier );
INT8 FindToolkit( SOLDIERTYPE * pSoldier );
BOOLEAN IsDetonatorAttached( OBJECTTYPE * pObj );
BOOLEAN IsRemoteDetonatorAttached( OBJECTTYPE * pObj );
OBJECTTYPE* FindAttachedBatteries( OBJECTTYPE * pObj );
INT8 FindMedKit( SOLDIERTYPE * pSoldier );
INT8 FindFirstAidKit( SOLDIERTYPE * pSoldier );
INT8 FindLocksmithKit( SOLDIERTYPE * pSoldier );
INT8 FindCamoKit( SOLDIERTYPE * pSoldier );
INT8 FindWalkman( SOLDIERTYPE * pSoldier );
INT8 FindTrigger( SOLDIERTYPE * pSoldier );
INT8 FindRemoteControl( SOLDIERTYPE * pSoldier );
INT16 GetWornCamo( SOLDIERTYPE * pSoldier );
INT16 GetCamoBonus( OBJECTTYPE * pObj );
INT16 GetWornStealth( SOLDIERTYPE * pSoldier );
INT16 GetStealthBonus( OBJECTTYPE * pObj );
// HEADROCK: Non-status-dependent version of the above
INT16 GetBasicStealthBonus( OBJECTTYPE * pObj );

void ApplyEquipmentBonuses(SOLDIERTYPE * pSoldier);

INT16 GetGearAimBonus( SOLDIERTYPE * pSoldier, INT32 iRange, INT16 ubAimTime);
INT16 GetGearToHitBonus( SOLDIERTYPE * pSoldier );
INT16 GetGearAPBonus( SOLDIERTYPE * pSoldier );
// HEADROCK: This function gets total AP bonus for an item and its attachments
INT16 GetAPBonus( OBJECTTYPE * pObj );

UINT16 GetFirstExplosiveOfType(UINT16 expType);

OBJECTTYPE* FindSunGogglesInInv( SOLDIERTYPE * pSoldier, INT8 * bSlot, BOOLEAN * isAttach, BOOLEAN searchAllInventory = FALSE );
OBJECTTYPE* FindNightGogglesInInv( SOLDIERTYPE * pSoldier, INT8 * bSlot, BOOLEAN * isAttach, BOOLEAN searchAllInventory = FALSE  );

UINT8 AllowedAimingLevels(SOLDIERTYPE * pSoldier, INT32 sGridNo);
// HEADROCK HAM 4: New functions to determine Scope and Laser factors on a weapon.
FLOAT GetHighestScopeMagnificationFactor( OBJECTTYPE *pObj );
FLOAT GetScopeMagnificationFactor( OBJECTTYPE *pObj, FLOAT uiRange );
FLOAT GetBestScopeMagnificationFactor( SOLDIERTYPE *pSoldier, OBJECTTYPE *pObj, FLOAT uiRange );
FLOAT GetProjectionFactor( OBJECTTYPE *pObj );
FLOAT GetScopeRangeMultiplier( SOLDIERTYPE *pSoldier, OBJECTTYPE *pObj, FLOAT d2DDistance );
INT32 GetGunAccuracy( OBJECTTYPE *pObj );
INT32 GetAccuracyModifier( OBJECTTYPE *pObj );
// HEADROCK HAM 4: Added range factor
UINT8 AllowedAimingLevelsNCTH( SOLDIERTYPE *pSoldier, INT32 sGridNo);
UINT8 GetAllowedAimingLevelsForItem( SOLDIERTYPE *pSoldier, OBJECTTYPE *pObj, UINT8 ubStance );

INT16 GetWornUrbanCamo( SOLDIERTYPE * pSoldier );
INT16 GetUrbanCamoBonus( OBJECTTYPE * pObj );
INT16 GetWornDesertCamo( SOLDIERTYPE * pSoldier );
INT16 GetDesertCamoBonus( OBJECTTYPE * pObj );
INT16 GetWornSnowCamo( SOLDIERTYPE * pSoldier );
INT16 GetSnowCamoBonus( OBJECTTYPE * pObj );

// HEADROCK HAM 3.6: Looks for a backpack anywhere on this character.
INT8 FindBackpackOnSoldier( SOLDIERTYPE * pSoldier );

// HEADROCK HAM 3.6: Handler functions to get modified damage with INI parameter.
UINT8 GetModifiedExplosiveDamage( UINT16 ubDamage );
UINT8 GetModifiedMeleeDamage( UINT16 ubDamage );
UINT8 GetModifiedGunDamage( UINT16 ubDamage );

UINT16 GetModifiedGunRange( UINT16 usWeaponIndex );


// SANDRO - added some procedures
INT16 ReduceCamoFromSoldier( SOLDIERTYPE * pSoldier, INT16 iCamoToRemove, INT16 iCamoToSkip );
BOOLEAN HasExtendedEarOn( SOLDIERTYPE * pSoldier );
BOOLEAN UseTotalMedicalKitPoints( SOLDIERTYPE * pSoldier, UINT16 usPointsToConsume );

#endif




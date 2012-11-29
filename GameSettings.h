#ifndef _GAME_SETTINGS__H_
#define _GAME_SETTINGS__H_

#include "Types.h"


//If you add any options, MAKE sure you add the corresponding string to the Options Screen string array.
//	 look up : zOptionsScreenHelpText , zOptionsToggleText
//Also, define its initialization and add its load/save to INI lines in : InitGameSettings() , SaveGameSettings() , LoadGameSettings()
//Also, to establish non typical display rules (options screen list) : Establish_Options_Screen_Rules()
enum
{
	TOPTION_SPEECH,
	TOPTION_MUTE_CONFIRMATIONS,
	TOPTION_SUBTITLES,
	TOPTION_KEY_ADVANCE_SPEECH,
	TOPTION_ANIMATE_SMOKE,
//	TOPTION_HIDE_BULLETS,
//	TOPTION_CONFIRM_MOVE,
	TOPTION_BLOOD_N_GORE,
	TOPTION_DONT_MOVE_MOUSE,
	TOPTION_OLD_SELECTION_METHOD,
	TOPTION_ALWAYS_SHOW_MOVEMENT_PATH,
//	TOPTION_TIME_LIMIT_TURNS,			//moved to the game init screen
	TOPTION_SHOW_MISSES,
	TOPTION_RTCONFIRM,
//	TOPTION_DISPLAY_ENEMY_INDICATOR,	//Displays the number of enemies seen by the merc, ontop of their portrait
	TOPTION_SLEEPWAKE_NOTIFICATION,
	TOPTION_USE_METRIC_SYSTEM,			//If set, uses the metric system
	TOPTION_MERC_ALWAYS_LIGHT_UP,
	TOPTION_SMART_CURSOR,
	TOPTION_SNAP_CURSOR_TO_DOOR,
	TOPTION_GLOW_ITEMS,
	TOPTION_TOGGLE_TREE_TOPS,
	TOPTION_TOGGLE_WIREFRAME,
	TOPTION_3D_CURSOR,
	TOPTION_CTH_CURSOR,

	//Madd:
	TOPTION_GL_BURST_CURSOR,
	TOPTION_ALLOW_TAUNTS, // changed from drop all - SANDRO
	TOPTION_GL_HIGH_ANGLE,
	TOPTION_ALLOW_REAL_TIME_SNEAK, // changed from aim levels restriction - SANDRO

	//lalien
	TOPTION_SPACE_SELECTS_NEXT_SQUAD,
	TOPTION_SHOW_ITEM_SHADOW,
	TOPTION_SHOW_WEAPON_RANGE_IN_TILES,
	TOPTION_TRACERS_FOR_SINGLE_FIRE,
	TOPTION_RAIN_SOUND,
	TOPTION_ALLOW_CROWS,
	TOPTION_ALLOW_SOLDIER_TOOLTIPS,
	TOPTION_USE_AUTO_SAVE,
	TOPTION_SILENT_SKYRIDER,
	//TOPTION_LOW_CPU_USAGE,
	TOPTION_ENHANCED_DESC_BOX,

	// arynn
	TOPTION_TOGGLE_TURN_MODE,

	// HEADROCK HAM 3.6:
	TOPTION_STAT_PROGRESS_BARS,

	

	// HEADROCK HAM 4:
	TOPTION_ALT_MAP_COLOR,

	// WANNE: Moved alternate bullets graphics (tracers) to options
	TOPTION_ALTERNATE_BULLET_GRAPHICS,

	// CHRISL: HAM 4: Activate/Deactivate NCTH mode
	TOPTION_USE_NCTH,

	// WANNE:	
	TOPTION_SHOW_TACTICAL_FACE_GEAR,
	TOPTION_SHOW_TACTICAL_FACE_ICONS,

	TOPTION_DISABLE_CURSOR_SWAP,				// Disable cursor swapping every second between talk and quick exchange

	// arynn: Debug/Cheat
	TOPTION_CHEAT_MODE_OPTIONS_HEADER,
	TOPTION_FORCE_BOBBY_RAY_SHIPMENTS,			// force all pending Bobby Ray shipments
	TOPTION_CHEAT_MODE_OPTIONS_END,
	TOPTION_DEBUG_MODE_OPTIONS_HEADER,			// an example options screen options header (pure text)                                                         
	
	// HEADROCK HAM 4:
	TOPTION_REPORT_MISS_MARGIN,
	
	TOPTION_SHOW_RESET_ALL_OPTIONS,				// failsafe show/hide option to reset all options 
	TOPTION_RESET_ALL_OPTIONS,					// a do once and reset self option (button like effect)                                                         
	TOPTION_RETAIN_DEBUG_OPTIONS_IN_RELEASE,	// allow debug options that were set in debug.exe to continue in a rel.exe (debugging release can be beneficial)
	TOPTION_DEBUG_MODE_RENDER_OPTIONS_GROUP,	// an example option that will show/hide other options                                                          
	TOPTION_RENDER_MOUSE_REGIONS,				// an example of a DEBUG build option                                                                           
	TOPTION_DEBUG_MODE_OPTIONS_END,				// an example options screen options divider (pure text)   

	// this is THE LAST option that exists (intended for debugging the options screen, doesnt do anything, except exist)
	TOPTION_LAST_OPTION, 
	NUM_GAME_OPTIONS,							// Toggles prior to this will be able to be toggled by the player

	//These options will NOT be toggable by the Player

	// JA2Gold
	TOPTION_MERC_CASTS_LIGHT,

	TOPTION_HIDE_BULLETS,
	TOPTION_TRACKING_MODE,

	NUM_ALL_GAME_OPTIONS,
};	


typedef struct
{
	INT8				bLastSavedGameSlot;					// The last saved game number goes in here	

	UINT8				ubMusicVolumeSetting;               // Volume Setting
	UINT8				ubSoundEffectsVolume;               // Volume Setting
	UINT8				ubSpeechVolume;                     // Volume Setting

	//The following are set from the status of the toggle boxes in the Options Screen
	BOOLEAN				fOptions[ NUM_ALL_GAME_OPTIONS + 1 ];	// Toggle Options (Speech, Subtitles, Show Tree Tops, etc.. )

	UINT32				uiMeanwhileScenesSeenFlags;         // Bit Vector describing seen 'mean whiles..' (not sure why this is in Game Settings )

	BOOLEAN				fHideHelpInAllScreens;              // Controls Help "do not show help again" checkbox

	UINT8				ubSizeOfDisplayCover;               // The number of grids the player designates thru "Delete + '=' or '-'"
	UINT8				ubSizeOfLOS;                        // The number of grids the player designates thru "End    + '=' or '-'"

}	GAME_SETTINGS;

// CHRISL: New Enums to track inventory system
enum
{
	INVENTORY_OLD = 0,
	INVENTORY_NEW = 1,
};

enum
{
	ATTACHMENT_OLD = 0,
	ATTACHMENT_NEW = 1,
};


//Enums for the difficulty levels
enum
{
	DIF_LEVEL_ZERO,
	DIF_LEVEL_EASY,
	DIF_LEVEL_MEDIUM,
	DIF_LEVEL_HARD,
	DIF_LEVEL_INSANE,
	DIF_LEVEL_FIVE,
};

//Enums for the difficulty levels
enum
{
	STYLE_REALISTIC = 0, //the specific indices are there to keep compartibility wiht old saves (back when there were only 2 styles)
	STYLE_SCIFI		= 1,
	STYLE_PLATINUM	= 2,
};

enum
{
	BR_GOOD = 1, 
	BR_GREAT = 2,
	BR_EXCELLENT = 4,
	BR_AWESOME = 10,
};

// Added by SANDRO
enum
{
	ITEM_PROGRESS_VERY_SLOW = 0,
	ITEM_PROGRESS_SLOW,
	ITEM_PROGRESS_NORMAL,
	ITEM_PROGRESS_FAST,
	ITEM_PROGRESS_VERY_FAST,
};

typedef struct
{
	BOOLEAN fGunNut;
	BOOLEAN fAirStrikes;//Madd
	UINT8	ubGameStyle;
	UINT8	ubDifficultyLevel;
	BOOLEAN	fTurnTimeLimit;
	BOOLEAN	fIronManMode;
	UINT8	ubBobbyRay;
	UINT8	ubInventorySystem;
	UINT8	ubAttachmentSystem;
	// SANDRO - added variables
	UINT8	ubMaxIMPCharacters;
	BOOLEAN	fNewTraitSystem;
	BOOLEAN fEnableAllTerrorists;
	BOOLEAN	fEnemiesDropAllItems;
	UINT8   ubProgressSpeedOfItemsChoices;
	BOOLEAN fEnableAllWeaponCaches;
	//UINT8	ubFiller[];

} GAME_OPTIONS;

bool UsingNewInventorySystem();
bool UsingNewAttachmentSystem();
bool UsingNewCTHSystem();
BOOLEAN IsNIVModeValid(bool checkRes = true);

// Snap: Options read from an INI file in the default of custom Data directory
typedef struct
{
	INT32	iStartingCashNovice;
	INT32	iStartingCashExperienced;
	INT32	iStartingCashExpert;
	INT32	iStartingCashInsane;

	UINT32 iGameStartingTime; //Lalien: game starting time	
	UINT32 iFirstArrivalDelay; 

	BOOLEAN fSellAll;
	INT16 iPriceModifier;

	// WDS: Allow flexible numbers of IMPs of each sex
	INT32 iIMPMaleCharacterCount;	// Count of how many there are
	INT32 iIMPFemaleCharacterCount;
	//INT32 iMaxIMPCharacters;		// Limit of how many to allow
	//
	// iaIMPSlots is an array of the slots (in prof.dat) to use for IMPs.
	//
	INT32 *iaIMPSlots;

	// SANDRO was here - some changes done
	INT32 iIMPProfileCost;
	BOOLEAN fDynamicIMPProfileCost;
	INT32 iMinAttribute;
	INT32 iMaxAttribute;
	INT32 iImpAttributePoints;
	INT32 iMaxZeroBonus;

	INT32 iIMPStartingLevelCostMultiplier;
	INT32 iBonusPointsForDisability;
	INT32 iBonusPointsPerSkillNotTaken;
	INT32 iMaxMilitiaPerSector;
	INT32 iTrainingSquadSize;
	INT32 iMilitiaTrainingCost;
	INT32 iMilitiaCostModifier;
	INT32 iRegularCostModifier;
	INT32 iVeteranCostModifier;
	INT32 iMinLoyaltyToTrain;

	INT32 iMaxEnemyGroupSize;

	INT32 iReinforcementPoolNovice;
	INT32 iReinforcementPoolExperienced;
	INT32 iReinforcementPoolExpert;
	INT32 iReinforcementPoolInsane;

	INT32 iForcePercentageNovice;
	INT32 iForcePercentageExperienced;
	INT32 iForcePercentageExpert;
	INT32 iForcePercentageInsane;

	INT32 iPercentElitesBonusExpert;
	INT32 iPercentElitesBonusInsane;
	INT32 iPercentElitesBonusExperienced;

	UINT8 ubMinEnemyGroupSizeNovice;
	UINT8 ubMinEnemyGroupSizeExperienced;
	UINT8 ubMinEnemyGroupSizeExpert;
	UINT8 ubMinEnemyGroupSizeInsane;

	BOOLEAN fMercDayOne;
	BOOLEAN fAllMercsAvailable;
	
	// tais: Any AIM merc on assignment?
	UINT8 fMercsOnAssignment;
	// tais: Soldiers wear any armour
	UINT8 fSoldiersWearAnyArmour;

	INT8 iEasyAPBonus;
	INT8 iExperiencedAPBonus;
	INT8 iExpertAPBonus;
	INT8 iInsaneAPBonus;
	INT8 iPlayerAPBonus;

	////////////////////////////////////
	// added by SANDRO
	INT16 sEnemyAdminCtHBonusPercent;
	INT16 sEnemyRegularCtHBonusPercent;
	INT16 sEnemyEliteCtHBonusPercent;

	INT8 sEnemyAdminDamageResistance;	
	INT8 sEnemyRegularDamageResistance;
	INT8 sEnemyEliteDamageResistance;

	BOOLEAN fAssignTraitsToEnemy;
	BOOLEAN fAssignTraitsToMilitia;
	INT8 bAssignedTraitsRarity;

	BOOLEAN fCamoRemoving;
	BOOLEAN fEnhancedCloseCombatSystem;

	UINT16 usAwardSpecialExpForQuests;
	////////////////////////////////////

	// Kaiden: Vehicle Inventory change - Added for INI Option
	BOOLEAN fVehicleInventory;

	///////////////////////////////////////
	// changed/added these - SANDRO
	UINT8 sMercsAutoresolveOffenseBonus;
	UINT8 sMercsAutoresolveDeffenseBonus;
	BOOLEAN fEnableChanceOfEnemyAmbushes; 
	INT8 bChanceModifierEnemyAmbushes;
	UINT8 usSpecialNPCStronger;
	///////////////////////////////////////

	// System settings
	UINT8 gubDeadLockDelay;
	BOOLEAN gfEnableEmergencyButton_SkipStrategicEvents;


	//Video settings	
	BOOLEAN gfVSync;

	//Animation settings
	FLOAT giPlayerTurnSpeedUpFactor;
	FLOAT giEnemyTurnSpeedUpFactor;
	FLOAT giCreatureTurnSpeedUpFactor;
	FLOAT giMilitiaTurnSpeedUpFactor;
	FLOAT giCivilianTurnSpeedUpFactor;

	//Sound settings
	UINT32 guiWeaponSoundEffectsVolume;

	// WDS - Option to turn off stealing
	BOOLEAN fStealingDisabled;

	// WANNE: Externalize chance of shipment package lost
	UINT32	gubChanceOfShipmentLost;

	// Militia Settings	
	BOOLEAN fAllowTacticalMilitiaCommand;
	BOOLEAN gfTrainVeteranMilitia;
	BOOLEAN gfAllowMilitiaGroups;
	BOOLEAN gfmusttrainroaming;
	BOOLEAN gflimitedRoaming;
	BOOLEAN gfAllowReinforcements;
	BOOLEAN gfAllowReinforcementsOnlyInCity;
	UINT32	guiBaseQueenPoolIncrement;
	
	UINT32	guiAllowMilitiaGroupsDelay;
	UINT32	guiTrainVeteranMilitiaDelay;
	UINT32	guiCreateEachNHours;
	UINT32	guiNumMobileMilitiaTrained;
	// HEADROCK HAM 3.6: These settings are REDUNDANT.
	//UINT32	guiMinMilitiaSquadSize;
	//UINT32	guiMaxMilitiaSquadSize;
	//UINT32	guiMaxMilitiaSquadSizeBattle;

	// SANDRO - added some variables
	INT16	sGreenMilitiaAutoresolveStrength;
	INT16	sRegularMilitiaAutoresolveStrength;
	INT16	sVeteranMilitiaAutoresolveStrength;
	INT8	bGreenMilitiaAPsBonus;
	INT8	bRegularMilitiaAPsBonus;
	INT8	bVeteranMilitiaAPsBonus;
	INT16	sGreenMilitiaCtHBonusPercent;	
	INT16	sRegularMilitiaCtHBonusPercent;
	INT16	sVeteranMilitiaCtHBonusPercent;
	INT8	bGreenMilitiaDamageResistance;
	INT8	bRegularMilitiaDamageResistance;
	INT8	bVeteranMilitiaDamageResistance;
	INT8	bGreenMilitiaEquipmentQualityModifier;
	INT8	bRegularMilitiaEquipmentQualityModifier;
	INT8	bVeteranMilitiaEquipmentQualityModifier;

	// WDS - Improve Tony's and Devin's inventory like BR's
	BOOLEAN tonyUsesBRSetting;
	BOOLEAN devinUsesBRSetting;

	// WDS - Smart goggle switching
	BOOLEAN smartGoggleSwitch;
	
	// WDS - Automatically flag mines
	BOOLEAN automaticallyFlagMines;

	// WDS - Automatically try to save when an assertion failure occurs
	BOOLEAN autoSaveOnAssertionFailure;
	UINT32  autoSaveTime;

	//Sight range
	UINT32	ubStraightSightRange;
	UINT32 ubVisDistDecreasePerRainIntensity;
	BOOLEAN gfAllowLimitedVision;

	BOOLEAN gfShiftFUnloadWeapons;
	BOOLEAN gfShiftFRemoveAttachments;

	// Rain settings
	BOOLEAN gfAllowRain;
	UINT32 gusRainChancePerDay;
	UINT32 gusRainMinLength;
	UINT32 gusRainMaxLength;
	UINT32 guiMaxRainDrops;
	UINT32 ubWeaponReliabilityReductionPerRainIntensity;
	double ubBreathGainReductionPerRainIntensity;

	// Thunder settings
	BOOLEAN gfAllowLightning;
	UINT32 guiMinLightningInterval;
	UINT32 guiMaxLightningInterval;
	UINT32 guiMinDLInterval;
	UINT32 guiMaxDLInterval;
	UINT32 guiProlongLightningIfSeenSomeone;
	UINT32 guiChanceToDoLightningBetweenTurns;

	// WDS - Progress settings
	UINT32 ubGameProgressPortionKills;
	UINT32 ubGameProgressPortionControl;
	UINT32 ubGameProgressPortionIncome;
	UINT32 ubGameProgressPortionVisited;
	UINT32 ubGameProgressMinimum;
	INT32 bGameProgressModifier;

	// Event settings
	UINT32 ubGameProgressStartMadlabQuest;
	UINT32 ubGameProgressMikeAvailable;
	UINT32 ubGameProgressIggyAvaliable;
	BOOLEAN ubSendTroopsToDrassen;

	// WDS - make number of mercenaries, etc. be configurable
	// group sizes
	UINT32 ubGameMaximumNumberOfPlayerMercs;
	UINT32 ubGameMaximumNumberOfPlayerVehicles;
	UINT32 ubGameMaximumNumberOfEnemies;
	UINT32 ubGameMaximumNumberOfCreatures;
	UINT32 ubGameMaximumNumberOfRebels;
	UINT32 ubGameMaximumNumberOfCivilians;

	//Gameplay settings

	INT32 iExplosivesDamageModifier;
	INT32 iGunDamageModifier;
	INT32 iGunRangeModifier;
	INT32 iMeleeDamageModifier;

	UINT32 ubEasyEnemyStartingAlertLevel;
	UINT32 ubNormalEnemyStartingAlertLevel;
	UINT32 ubHardEnemyStartingAlertLevel;
	UINT32 ubInsaneEnemyStartingAlertLevel;

	UINT32 ubEasyEnemyStartingAlertDecay;
	UINT32 ubNormalEnemyStartingAlertDecay;
	UINT32 ubHardEnemyStartingAlertDecay;
	UINT32 ubInsaneEnemyStartingAlertDecay;

	UINT32 ubEasyTimeEvaluateInMinutes;
	UINT32 ubNormalTimeEvaluateInMinutes;
	UINT32 ubHardTimeEvaluateInMinutes;
	UINT32 ubInsaneTimeEvaluateInMinutes;

	UINT32 ubEasyTimeEvaluateVariance;
	UINT32 ubNormalTimeEvaluateVariance;
	UINT32 ubHardTimeEvaluateVariance;
	UINT32 ubInsaneTimeEvaluateVariance;

	UINT32 ubEasyGracePeriodInHours;
	UINT32 ubNormalGracePeriodInHours;
	UINT32 ubHardGracePeriodInHours;
	UINT32 ubInsaneGracePeriodInHours;

	UINT32 ubEasyPatrolGracePeriodInDays;
	UINT32 ubNormalPatrolGracePeriodInDays;
	UINT32 ubHardPatrolGracePeriodInDays;
	UINT32 ubInsanePatrolGracePeriodInDays;

	UINT32 ubEasyNumAwareBattles;
	UINT32 ubNormalNumAwareBattles;
	UINT32 ubHardNumAwareBattles;
	UINT32 ubInsaneNumAwareBattles;

	// WDS - New AI
	BOOLEAN useNewAI;

	BOOLEAN gfEasyUnlimitedTroops;
	BOOLEAN gfNormalUnlimitedTroops;
	BOOLEAN gfHardUnlimitedTroops;
	BOOLEAN gfInsaneUnlimitedTroops;

	BOOLEAN gfEasyAggressiveQueen;
	BOOLEAN gfNormalAggressiveQueen;
	BOOLEAN gfHardAggressiveQueen;
	BOOLEAN gfInsaneAggressiveQueen;

	BOOLEAN gfInvestigateSector;
	BOOLEAN gfReassignPendingReinforcements;

	INT32 ubEnemiesItemDrop;

	BOOLEAN	gfUseExternalLoadscreens;

	//tais: nsgi
	BOOLEAN gfUseNewStartingGearInterface;
	// DBrot: Seperate Starting items for Experts
	BOOLEAN fExpertsGetDifferentChoices;
	BOOLEAN gfUseAutoSave;

	//Merc Assignment settings
	INT32 ubAssignmentUnitsPerDay;
	INT32 ubMinutesForAssignmentToCount;

	INT32 ubTrainingSkillMin;
	INT32 ubTrainingSkillMax;
	INT32 ubSelfTrainingDivisor;
	INT32 ubInstructedTrainingDivisor;
	// HEADROCK HAM 3.5: No longer necessary.
	//INT32 ubGunRangeTrainingBonus;
	INT32 ubTownMilitiaTrainingRate;
	// HEADROCK HAM 3.5: No longer necessary.
	//INT32 ubMaxMilitiaTrainersPerSector;
	INT32 ubTeachBonusToTrain;
	INT32 ubRpcBonusToTrainMilitia;
	INT32 ubMinSkillToTeach;

	INT32 ubLowActivityLevel;
	INT32 ubMediumActivityLevel;
	INT32 ubHighActivityLevel;
	INT32 ubDoctoringRateDivisor;
	INT32 ubHospitalHealingRate;
	INT32 ubBaseMedicalSkillToDealWithEmergency;
	INT32 ubMultiplierForDifferenceInLifeValueForEmergency;
	INT32 ubPointCostPerHealthBelowOkLife;//OKLIFE = 15

	INT32 ubRepairCostPerJam;
	INT32 ubRepairRateDivisor;

	//Misc settings
	BOOLEAN fAmmoDynamicWeight; //Pulmu
	BOOLEAN fEnableCrepitus;
	// SANDRO was here - removed these two
	//BOOLEAN fEnableAllWeaponCaches;
	//BOOLEAN fEnableAllTerrorists;
	BOOLEAN gfRevealItems;
	BOOLEAN fEnableArmorCoverage; // ShadoWarrior for Captain J's armor coverage

	// ShadoWarrior: Tooltip changes (start)
	UINT8 ubSoldierTooltipDetailLevel;

	BOOLEAN fEnableDynamicSoldierTooltips;
	BOOLEAN fEnableSoldierTooltipLocation;
	BOOLEAN fEnableSoldierTooltipBrightness;
	BOOLEAN fEnableSoldierTooltipRangeToTarget;
	BOOLEAN fEnableSoldierTooltipID;
	BOOLEAN fEnableSoldierTooltipOrders;
	BOOLEAN fEnableSoldierTooltipAttitude;
	BOOLEAN fEnableSoldierTooltipActionPoints;
	BOOLEAN fEnableSoldierTooltipHealth;
	BOOLEAN fEnableSoldierTooltipTraits; // added by SANDRO
	BOOLEAN fEnableSoldierTooltipHelmet;
	BOOLEAN fEnableSoldierTooltipVest;
	BOOLEAN fEnableSoldierTooltipLeggings;
	BOOLEAN fEnableSoldierTooltipHeadItem1;
	BOOLEAN fEnableSoldierTooltipHeadItem2;
	BOOLEAN fEnableSoldierTooltipWeapon;
	BOOLEAN fEnableSoldierTooltipSecondHand;
	BOOLEAN fEnableSoldierTooltipBigSlot1;
	BOOLEAN fEnableSoldierTooltipBigSlot2;
	BOOLEAN fEnableSoldierTooltipBigSlot3;
	BOOLEAN fEnableSoldierTooltipBigSlot4;
	BOOLEAN fEnableSoldierTooltipBigSlot5;
	BOOLEAN fEnableSoldierTooltipBigSlot6;
	BOOLEAN fEnableSoldierTooltipBigSlot7;
	// ShadoWarrior: Tooltip changes (end)
	//SCORE: UDT related
	BOOLEAN gfAllowUDTRange;
	BOOLEAN gfAllowUDTDetail;
	INT8	ubUDTModifier;

	//Kaiden MERC Deaths Externalized:
	BOOLEAN gfMercsDieOnAssignment;
	INT32 giEasyMercDeaths;
	INT32 giExperiencedMercDeaths;
	INT32 giExpertMercDeaths;
	INT32 giInsaneMercDeaths;

	// Lesh: slow enemy items choice progress
	BOOLEAN fSlowProgressForEnemyItemsChoice;

	//afp - use bullet tracers
	BOOLEAN gbBulletTracer;

	// CHRISL: option to allow Slay to remain as a hired PC
	BOOLEAN fEnableSlayForever;

	// CHRISL: Setting to turn off the description and stack popup options from the sector inventory panel
	BOOLEAN fSectorDesc;

	// WANNE: Restrict female enemies from beeing in the queens army (except Female Elite enemies)?
	BOOLEAN fRestrictFemaleEnemiesExceptElite;

	// HEADROCK: Enhanced Item Description Box ON/OFF
	// WANNE: Changed from BOOLEAN to INT32!
	INT32 iEnhancedDescriptionBox;
	
	//WarmSteel - New Attachment System?	
	BOOLEAN fUseDefaultSlots;
	UINT32	usAttachmentDropRate;
	INT16   iMaxEnemyAttachments;

	//** ddd
	//enable ext mouse key
	BOOLEAN fExtMouseKeyEnabled;
	// for small progress bar
	BOOLEAN fSmallSizeProgressbar;
	// stamina multiplier (for all weapons eg knife fist gun
	INT32 uStaminaHit;
	//enable ext mouse key
	BOOLEAN bAltAimEnabled;	
	BOOLEAN bAimedBurstEnabled;	
	INT16 uAimedBurstPenalty;
	BOOLEAN	bWeSeeWhatMilitiaSeesAndViceVersa;
	BOOLEAN	bAllowWearSuppressor;
	BOOLEAN	bLazyCivilians;
	INT16 iChanceSayAnnoyingPhrase;
	BOOLEAN	bNewTacticalAIBehavior;
	FLOAT uShotHeadPenalty;
	FLOAT fShotHeadDivisor;
	INT16 iPenaltyShootUnSeen;

	FLOAT fOutOfGunRangeOrSight;
	// WANNE: Always use "prof.dat".
	BOOLEAN fUseDifficultyBasedProfDat;

	// 2Points: Use new repair algorithm
	BOOLEAN fAdditionalRepairMode;

	// CHRISL: Skyrider and enemy occupied sectors
	UINT8 ubSkyriderHotLZ;

	// WANNE: Fast loading settings
	BOOLEAN fDisableLaptopTransition;
	BOOLEAN fFastWWWSitesLoading;
	BOOLEAN fDisableStrategicTransition;

	// CPT: Cover System Settings
	UINT8 ubStealthTraitCoverValue;
	UINT8 ubStealthEffectiveness;
	UINT8 ubTreeCoverEffectiveness;
	UINT8 ubCamouflageEffectiveness;
	UINT8 ubStanceEffectiveness;
	UINT8 ubLBEEffectiveness;
	UINT8 ubMovementEffectiveness;
	UINT8 ubCoverDisplayUpdateWait;

	// HEADROCK HAM B1: Global modifier for mine income (0 is automatically turned to 1 to avoid divide-by-zero.)
	INT16 usMineIncomePercentage;

	// HEADROCK HAM B1: Allows changing the maximum CTH possible, for any attack 0-100
	UINT8 ubMaximumCTH;
	
	// HEADROCK HAM B1: Allows changing the minimum CTH possible, for any attack 0-100
	UINT8 ubMinimumCTH;

	// HEADROCK HAM B1: Allows reducing the chance of scoring a hit fractionally, if CTH = Minimum CTH
	UINT16 usMinimumCTHDivisor;

	// HEADROCK HAM B1: Allows Restricted Roaming Militia to move through previously visited sectors, even if restricted.
	BOOLEAN fUnrestrictVisited;

	// HEADROCK HAM B1: Allows the capture of towns to dynamically alter roaming restrictions
	BOOLEAN fDynamicRestrictRoaming;

	// HEADROCK HAM B2.1: This controls how effective suppression is, by increasing the number of ubSuppressionPoints accumulated by combatants (percentage);
	INT16 sSuppressionEffectiveness;

	UINT8 ubSuppressionToleranceMax;
	UINT8 ubSuppressionToleranceMin;

	// HEADROCK HAM B2: Suppression Shock effectiveness (percentage, 100 = "normal", 0 = deactivated. Range 0-65535)
	UINT16 usSuppressionShockEffect;

	// HEADROCK HAM B2.1: CTH penalty given by a "Cowering" target to any enemy shooter.
	UINT8 ubAimPenaltyPerTargetShock;

	// HEADROCK HAM B2.3: It is now possible for "cowering" to increase a target's suppression.
	UINT8 ubCowerEffectOnSuppression;

	// HEADROCK HAM B2.5: Turn on Realistic Tracers. 0 = off (regular tracers). 1 = Fully realistic tracers. 2 = Tracer Bump + 1.13 Autofire Penalty Reduction
	UINT8 ubRealisticTracers;

	// HEADROCK HAM B2.5: Realistic tracers - one of every X bullets in a tracer magazines will be a tracer bullet. 0 = off (JA2 normal)
	UINT8 ubNumBulletsPerTracer;

	// HEADROCK HAM B2.5: Realistic tracers - CTH increased by this amount whenever a tracer is fired. 0 = off.
	UINT8 ubCTHBumpPerTracer;

	// CHRISL: Exeternalize the minimum range at which tracers can improve autofire hit chance
	UINT16 ubMinRangeTracerEffect;

	// HEADROCK HAM B2.6: Increased aiming costs?
	BOOLEAN fIncreasedAimingCost;

	// added by SANDRO
	BOOLEAN fAimLevelRestriction;

	// HEADROCK HAM B2.6: Dynamically determine Max-Aiming based also on weapon, bipod, etc?
	BOOLEAN fDynamicAimingTime;

	// allow old behaviour
	BOOLEAN fAimLevelsDependOnDistance; // feel free to dump this option
	INT32 iAimLevelsCompatibilityOption; // but not this one

	//WarmSteel - These determine in which group each scope belongs. Needed for dynamic aiming limits.
	INT16 sVeryHighPowerScope;
	INT16 sHighPowerScope;
	INT16 sMediumPowerScope;

	//CHRISL: AI Sniper fields
	BOOLEAN fAISniperElite;
	UINT16 fAISniperRange;
	UINT16 fAISniperChance;
	UINT16 fAISniperChanceWithSR;

	// HEADROCK HAM B2.6: Controls how much effect target movement has on aiming
	FLOAT iMovementEffectOnAiming;

	// HEADROCK HAM B2.6: Autofire Bullets/5AP modifier
	INT8 bAutofireBulletsPer5APModifier;

	// HEADROCK HAM B2.6/B1: Adjustable "luck" factor in Auto-Resolve
	FLOAT iAutoResolveLuckFactor;

	// HEADROCK HAM B2.6: Adjustable maximum for Suppression Shock effect. This has the added effect of reducing overall susceptibility to shock and may render some enemies unshockable.
	UINT8 ubMaxSuppressionShock;

	// HEADROCK HAM B2.6/2/1: Toggle new Burst/Auto CTH bars: 0=neither, 1=both, 2=Burst, 3=Auto
	UINT8 ubNewCTHBars;

	// CHRISL: Changed from a simple flag to two externalized values for more modder control over AI suppression
	UINT16 ubAISuppressionMinimumMagSize;
	UINT16 ubAISuppressionMinimumAmmo;

	// HEADROCK HAM B2.7: Change the speed of skill progression. (defaults set to JA2 normal)
	UINT16 usHealthSubpointsToImprove;
	UINT16 usStrengthSubpointsToImprove;
	UINT16 usDexteritySubpointsToImprove;
	UINT16 usAgilitySubpointsToImprove;
	UINT16 usWisdomSubpointsToImprove;
	UINT16 usMarksmanshipSubpointsToImprove;
	UINT16 usMedicalSubpointsToImprove;
	UINT16 usMechanicalSubpointsToImprove;
	UINT16 usExplosivesSubpointsToImprove;
	UINT16 usLeadershipSubpointsToImprove;
	UINT16 usLevelSubpointsToImprove;

	// HEADROCK HAM B2.7: When turned on, this will give a CTH approximation instead of an exact value, on CTH Bars and "F" key feedback.
	BOOLEAN fApproximateCTH;

	// HEADROCK HAM B2.7: Augmented Roaming Militia code - turn this to TRUE to allow militia free travel through San Mona, Tixa, Orta, Omerta, and Estoni.
	BOOLEAN fAllowMilitiaMoveThroughMinorCities;

	// HEADROCK HAM B2.8: These are new cowering penalty divisors that help us determine how effective cowering is in different stances and when the shooter is targetting different bodyparts
	UINT8 ubCoweringPenaltyDivisorProne;
	UINT8 ubCoweringPenaltyDivisorCrouchedHead;
	UINT8 ubCoweringPenaltyDivisorCrouchedTorso;
	UINT8 ubCoweringPenaltyDivisorCrouchedLegs;

	// HEADROCK HAM B2.8: This is the maximum range at which a target gives out the full CTH penalty for cowering. At lower range, it'll give proportionally less penalty.
	UINT16 usMinRangeForFullCoweringPenalty;

	// HEADROCK HAM B2.8: Absolute maximum CTH penalty from target/shooter cowering
	UINT16 usMaxShooterCoweringPenalty;
	UINT16 usMaxTargetCoweringPenalty;

	// HEADROCK HAM B2.8: If this is turned on, Militia will drop their equipment similar to enemies, IF killed by non-player character.
	UINT8 ubMilitiaDropEquipment;

	// HEADROCK HAM B2.8: New Trainer Relations: 2 = Trainees will go to sleep when their trainer goes to sleep. 3 = Trainer will go to sleep if all trainees are asleep. 1 = Both. 0 = Neither.
	UINT8 ubSmartTrainingSleep;

	// HEADROCK HAM B2.8: New Trainer Relations: 2 = Trainees will wake up when their trainer wakes up. 3 = Trainer will wake up if all trainees wake up. 1 = Both. 0 = Neither.
	UINT8 ubSmartTrainingWake;

	// HEADROCK HAM B2.8: New Trainer Relations: 2 = Trainers will rest if no trainees available. 3 = Trainees will rest if no trainers available (not recommended). 1 = Both. 0 = Neither.
	UINT8 ubSmartTrainingRest;

	// HEADROCK HAM 3: Externalized ratio between Weight and Strength. This value determines how many strength points we need to lift 0.5kg with no encumberance.
	FLOAT iStrengthToLiftHalfKilo;

	// HEADROCK HAM 3: Define minimum leadership required for training militia. Set to 0 for "no limit".
	UINT8 ubMinimumLeadershipToTrainMilitia;

	// HEADROCK HAM 3: Define effect of "TEACHER" trait in increasing effective leadership, for purposes of eligibility for training militia
	UINT16 usTeacherTraitEffectOnLeadership;

	// HEADROCK HAM 3: If enabled, the trainer's "effective" leadership skill determines HOW MANY militia he/she creates per session (only works in town militia training).
	BOOLEAN fLeadershipAffectsMilitiaQuantity;

	// HEADROCK HAM 3: If "LEADERSHIP_AFFECTS_MILITIA_QUANTITY" is true, this value determines the lowest leadership required to train a full (default size 10) squad of town militia in one training session.
	UINT8 ubReqLeadershipForFullTraining;

	// HEADROCK HAM 3: Four INI settings to control the number of kills you must accumulate to win one progress point, one setting per difficulty level
	UINT16 usNumKillsPerProgressPointNovice;
	UINT16 usNumKillsPerProgressPointExperienced;
	UINT16 usNumKillsPerProgressPointExpert;
	UINT16 usNumKillsPerProgressPointInsane;

	// HEADROCK HAM 3: If enabled, changes the way the game calculates progress. It will look at each progress control separately (Kills/Income/Control/Visited) and set the progress to the HIGHEST scoring control.
	BOOLEAN fAlternateProgressCalculation;

	// HEADROCK HAM 3: If enabled, tooltipping over Bobby Ray's weapons will show a list of possible attachments to those weapons.
	BOOLEAN fBobbyRayTooltipsShowAttachments;

	// CHRISL: Converts the AutoFireToHitBonus value to a percentage for CTH calculations
	UINT8 ubFlatAFTHBtoPrecentMultiplier;

	// HEADROCK HAM 3.1: Divisor for the AP-to-Ready cost charge on first aiming click, when extra aiming costs are enabled. 0 = No ready-time-based charge.
	UINT8 ubFirstAimReadyCostDivisor;

	// HEADROCK HAM 3.1: This is a suppression tool that not everyone will like. It gives an on-screen message when any character has been suppressed so much he's lost his next turn completely!
	BOOLEAN fShowSuppressionShutdown;

	// HEADROCK HAM 3.1: Select which mine will run out. -1 = Random selection. 0 = no mine. 1 = San Mona (unused), 2 = Drassen, 3 = Alma, 4 = Cambria, 5 = Chitzena, 6 = Grumm.
	INT8 bWhichMineRunsOut;

	// HEADROCK HAM 3.1: Can the Humvee go off-road?
	BOOLEAN fHumveeOffroad;

	// HEADROCK HAM 3.2: If activated, reinforcements (militia/enemy) arrive in the battle with 0 APs. This makes them less of a diablo-ex-machina.
	UINT8 ubReinforcementsFirstTurnFreeze;

	// HEADROCK HAM 3.2: This feature allows the status, leadership and experience of nearby friendlies help/hinder a character's tolerance, based on their distance from him.
	BOOLEAN fFriendliesAffectTolerance;

	// HEADROCK HAM 3.2: Set a divisor for the CtH of Mortar weapons.
	UINT8 ubMortarCTHDivisor;

	// HEADROCK HAM 3.2: This enabled reduced sight for cowering characters. 0 = disabled. 2 = Reduced Sightrange. 3 = Tunnel-vision. 1 = Both.
	UINT8 ubCoweringReducesSightRange;

	// HEADROCK HAM 3.2: Critical Headshots may cause blindness. Rolls 1 to X change of being blinded. 0 = disabled.
	UINT8 ubChanceBlindedByHeadshot;

	// HEADROCK HAM 3.2: Critical Legshots cause additional AP loss
	BOOLEAN fCriticalLegshotCausesAPLoss;

	// HEADROCK HAM 3.2: When enabled, this setting removes the player's omniscience inside his own sectors. The player will no longer see the movement of enemy groups unless someone is nearby to scout them.
	BOOLEAN fNoEnemyDetectionWithoutRecon;

	// HEADROCK HAM 3.2: Determines the training bonus for the Medical skill, when training in "hospital" sectors
	UINT8 ubHospitalTrainingBonus;

	// HEADROCK HAM 3.2: Determines the repaid bonus when training in factory ("industrial") sectors
	UINT8 ubFactoryRepairBonus;

	// HEADROCK HAM 3.2: How much MECHANICAL skill do we need to gain a repair bonus from a factory?
	UINT8 ubMinMechanicalForFactoryRepairBonus;

	// HEADROCK HAM 3.3: Externalized maximum possible penalty for hitting a moving target. JA2 Default = 30.
	UINT16 usMaxCTHPenaltyForMovingTarget;

	// HEADROCK HAM 3.3: Increases tolerance while moving.
	UINT8 ubTilesMovedPerBonusTolerancePoint;

	// HEADROCK HAM 3.3: New militia feature, Minimum Leadership required to train Roaming Militia
	UINT8 ubMinimumLeadershipToTrainMobileMilitia;

	// HEADROCK HAM 3.3: If enabled, the trainer's "effective" leadership skill determines HOW MANY militia he/she creates per session.
	BOOLEAN fLeadershipAffectsMobileMilitiaQuantity;

	// HEADROCK HAM 3.3: If "LEADERSHIP_AFFECTS_MILITIA_QUANTITY" is true, this value determines the lowest leadership required to train a full (default size 10) squad of town militia in one training session.
	UINT8 ubReqLeadershipForFullMobileTraining;

	// HEADROCK HAM 3.3: Minimum distance (in METERS) at which character suffer from friendly suppression.
	UINT16 usMinDistanceFriendlySuppression;

	// HEADROCK HAM 3.3: If enabled, Roaming Militia can automatically reinforce city garrisons. 
	BOOLEAN fAllowMobileReinforceCities;

	// HEADROCK HAM 3.3: If enabled, Roaming Militia can automatically reinforce city garrisons. 
	BOOLEAN fAllowMobileReinforceSAM;

	// HEADROCK HAM 3.4: This controls the intensity of Hiding the Bullet Count during combat. The higher it is, the more intense the effect. Negative values reduce the effect.
	UINT16 usBulletHideIntensity;

	// HEADROCK HAM 3.4: What percentage of a new Mobile Militia group will be made of Elites? If >0, then at least one of every group will be an Elite. 100 = All Elites.
	UINT8 ubPercentRoamingMilitiaElites;

	// HEADROCK HAM 3.4: What percentage of a new Mobile Militia group will be made of Elites? If >0, then at least one of every group will be an Elite. 100 = All Elites.
	UINT8 ubPercentRoamingMilitiaRegulars;

	// HEADROCK HAM 3.5: Does leadership affect the Quality of new Mobile Militia groups?
	BOOLEAN fLeadershipAffectsMobileMilitiaQuality;

	// HEADROCK HAM 3.5: Explosive Suppression Effectiveness alters the amount of Suppression Points you get from nearby blasts.
	UINT16 usExplosionSuppressionEffect;

	// HEADROCK HAM 3.5: When enabled, goggle-switching affects every merc in the sector, not just those belonging to the current squad.
	BOOLEAN fGoggleSwapAffectsAllMercsInSector;

	// HEADROCK HAM 3.5: Helicopter - Base cost per tile in "GREEN" airspace
	UINT16 usHelicopterBaseCostPerGreenTile;

	// HEADROCK HAM 3.5: Helicopter - Base cost per tile in "RED" airspace
	UINT16 usHelicopterBaseCostPerRedTile;

	// HEADROCK HAM 3.5: Experimental, arrival sectorX/Y.
	UINT8 ubDefaultArrivalSectorX;
	UINT8 ubDefaultArrivalSectorY;

	// HEADROCK HAM 3.5: Limit bonus from tracers based on range to target. This is a divisor for the effect.
	UINT8 ubRangeDifficultyAimingWithTracers;

	// HEADROCK HAM 3.6: Militia can now place flags on discovered mines.
	BOOLEAN fMilitiaPlaceBlueFlags;

	// HEADROCK PROFEX: Activate this to read Profile data from Profiles.XML and Opinions.XML
	BOOLEAN fReadProfileDataFromXML;

	// HEADROCK PROFEX: Activate this to read Profile data from Profiles.XML and Opinions.XML
	BOOLEAN fWriteProfileDataToXML;

	// HEADROCK HAM 3.6: Progress bars for each stat, displayed behind the stat value on the merc panel.
	UINT8 ubStatProgressBarsRed;
	UINT8 ubStatProgressBarsGreen;
	UINT8 ubStatProgressBarsBlue;

	// HEADROCK HAM 3.6: Determines how likely a character is, every hour, to trigger a facility event. The actual chance is X in every Y attempts, where X is facility-specific, and Y is the value adjusted here.
	UINT16 usFacilityEventRarity;
	// HEADROCK HAM 3.6: Controls how important the character's stats are, in insuring he/she gets better results (or smaller damage) from using facilities. The higher this is, the more dangerous all facilities are.
	UINT8 ubFacilityDangerRate;

	// HEADROCK HAM 3.6: Determines whether the extrapolated daily costs of mercs are figured into the "Daily Expenses" display. 0 = No, just facility costs. 1 = Only mercs with a fixed daily rate. 2 = All mercs, including AIM contracts.
	UINT8 ubIncludeContractsInExpenses;

	// HEADROCK HAM 3.6: Maximum number of messages displayed in Tactical view
	UINT8 ubMaxMessagesTactical;

	// HEADROCK HAM 3.6: Daily upkeep costs for militia
	UINT16 usDailyCostTownGreen;
	UINT16 usDailyCostTownRegular;
	UINT16 usDailyCostTownElite;
	UINT16 usDailyCostMobileGreen;
	UINT16 usDailyCostMobileRegular;
	UINT16 usDailyCostMobileElite;

	// HEADROCK HAM 3.6: Non-Combat Bodytypes can't become hostile
	BOOLEAN fCanTrueCiviliansBecomeHostile;

	// HEADROCK HAM 3.6: Militia become hostile when attacked. 0 = No. 1 = If killed. 2 = If attacked (JA2 Default)
	UINT8 ubCanMilitiaBecomeHostile;

	// HEADROCK HAM 3.6: If activated, the game does not switch focus to a merc who spots an enemy in real-time mode. This fixes issues with Real-Time Sneak.
	BOOLEAN fNoAutoFocusChangeInRealtimeSneak;

	UINT8 ubEarlyRebelsRecruitment[4];	// early recruitment of Miguel and Carlos
	// silversurfer: don't play quote when merc spots a mine? TRUE = shut up! FALSE = tell me that you found a mine!
	BOOLEAN fMineSpottedNoTalk;

	// WANNE: Don't stop and talk when spotting a new item in turn based mode
	BOOLEAN fItemSpottedNoTalk;

	// The_Bob - real time sneaking code 01/06/09
	// Suport disabling/silencing real time sneaking via external .ini file
	//BOOLEAN fAllowRealTimeSneak; // SANDRO - moved to preferences
	BOOLEAN fQuietRealTimeSneak;

	//legion by Jazz
	BOOLEAN fCanJumpThroughWindows;
	BOOLEAN fCanClimbOnWalls;

	//legion by Jazz
	BOOLEAN fIndividualHiddenPersonNames;
	
	//SAveGame Slot by Jazz
	BOOLEAN fSaveGameSlot;

	//dnl ch51 081009 JA2 Debug Settings
	BOOLEAN fEnableInventoryPoolQ;		
	
	//legion by Jazz
	//BOOLEAN fShowTacticalFaceGear; //legion 2
	//BOOLEAN fShowTacticalFaceIcons; //legion 2
	INT8 bTacticalFaceIconStyle;
	
	//Enemy Names Group Legion 2 by Jazz
	BOOLEAN fEnemyNames;
	
	//Civ Names Group Legion 2 by Jazz
	BOOLEAN fCivGroupName;
	
	//Enemy Rank Legion 2 by Jazz
	BOOLEAN fEnemyRank;
	
	BOOLEAN fShowCamouflageFaces;

	FLOAT gMercLevelUpSalaryIncreasePercentage;

	UINT8 ubChanceTonyAvailable; // silversurfer/SANDRO
	
	BOOLEAN fStandUpAfterBattle;
	
	INT32 iInitialMercArrivalLocation;
	
} GAME_EXTERNAL_OPTIONS;

typedef struct
{
	UINT8 ubMaxNumberOfTraits;
	UINT8 ubNumberOfMajorTraitsAllowed;

	// GENERIC SETTINGS
	INT8 bCtHModifierAssaultRifles;
	INT8 bCtHModifierSniperRifles;
	INT8 bCtHModifierRifles;
	INT8 bCtHModifierSMGs;
	INT8 bCtHModifierLMGs;
	INT8 bCtHModifierShotguns;
	INT8 bCtHModifierPistols;
	INT8 bCtHModifierMachinePistols;
	INT8 bCtHModifierRocketLaunchers;
	INT8 bCtHModifierGrenadeLaunchers;
	INT16 sCtHModifierMortar;
	INT8 bCtHModifierThrowingKnives;
	INT8 bCtHModifierThrowingGrenades;
	INT8 bCtHModifierHtHAttack;
	INT8 bModifierDodgeHtHChance;
	INT8 bCtHModifierKnifeAttack;
	INT8 bModifierDodgeKnifeChance;
	INT8 bCtHModifierRobot;
	UINT8 ubCtHPenaltyDualShot;
	INT8 bPercentModifierHtHBreathLoss;
	INT8 bPercentModifierBladesBreathLoss;
	INT8 bPercentModifierBluntBreathLoss;
	UINT8 ubModifierForAPsAddedOnAimedPunches;
	UINT8 ubModifierForAPsAddedOnAimedBladedAttackes;
	INT8 bSpeedModifierDoctoring;
	INT8 bSpeedModifierBandaging;
	INT8 bSpeedModifierRepairing;
	INT8 bSpeedModifierTrainingMilitia;
	INT8 bSpeedModifierTeachingOthers;
	INT8 bCheckModifierLockpicking;
	INT8 bCheckModifierAttachDetonators;
	INT8 bCheckModifierSetExplosives;
	INT8 bCheckModifierDisarmExplosiveTraps;
	INT8 bCheckModifierDisarmElectronicTraps;
	INT8 bCheckModifierAttachSpecialItems;
	INT8 bTanksDamageResistanceModifier;
	UINT8 ubDamageNeededToLoseStats;

	// AUTO WEAPONS
	UINT8 ubAWBonusCtHAssaultRifles;
	UINT8 ubAWBonusCtHSMGs;
	UINT8 ubAWBonusCtHLMGs;
	UINT8 ubAWAutoFirePenaltyReduction;
	UINT8 ubAWUnwantedBulletsReduction;
	UINT8 ubAWFiringSpeedBonusLMGs;
	UINT8 ubAWPercentReadyLMGReduction;

	// HEAVY WEAPONS
	UINT8 ubHWGrenadeLaunchersAPsReduction;
	UINT8 ubHWRocketLaunchersAPsReduction;
	UINT8 ubHWBonusCtHGrenadeLaunchers;
	UINT8 ubHWBonusCtHRocketLaunchers;
	UINT8 ubHWMortarAPsReduction;
	UINT8 ubHWMortarCtHPenaltyReduction;
	UINT8 ubHWDamageTanksBonusPercent;
	UINT8 ubHWDamageBonusPercentForHW;

	// SNIPER
	UINT8 ubSNBonusCtHRifles;
	UINT8 ubSNBonusCtHSniperRifles;
	UINT8 ubSNEffRangeToTargetReduction;
	UINT8 ubSNAimingBonusPerClick;
	UINT8 ubSNDamageBonusPerClick;
	UINT8 ubSNDamageBonusFromNumClicks;
	UINT8 ubSNChamberRoundAPsReduction;
	UINT8 ubSNAimClicksAdded;
	
	// RANGER
	UINT8 ubRABonusCtHRifles;
	UINT8 ubRABonusCtHShotguns;
	UINT8 ubRAPumpShotgunsAPsReduction;
	UINT8 ubRAGroupTimeSpentForTravellingFoot;
	UINT8 ubRAGroupTimeSpentForTravellingVehicle;
	UINT8 ubRAMaxBonusesToTravelSpeed;
	UINT8 ubRABreathForTravellingReduction;
	UINT8 ubRAWeatherPenaltiesReduction;
	//UINT8 ubRACamoEffectivenessBonus;
	UINT8 ubRACamoWornountSpeedReduction;

	// GUNSLINGER
	UINT8 ubGSFiringSpeedBonusPistols;
	UINT8 ubGSEffectiveRangeBonusPistols;
	UINT8 ubGSBonusCtHPistols;
	UINT8 ubGSBonusCtHMachinePistols;
	UINT8 ubGSCtHMPExcludeAuto;
	UINT8 ubGSAimingBonusPerClick;
	UINT8 ubGSPercentReadyPistolsReduction;
	UINT8 ubGSRealoadSpeedHandgunsBonus;
	UINT8 ubGSAimClicksAdded;

	// MARTIAL ARTS
	UINT8 ubMABonusCtHBareHands;
	UINT8 ubMABonusCtHBrassKnuckles;
	UINT8 ubMAPunchAPsReduction;
	UINT8 ubMABonusDamageHandToHand;
	UINT8 ubMABonusBreathDamageHandToHand;
	UINT16 usMALostBreathRegainPenalty;
	UINT16 usMAAimedPunchDamageBonus;
	INT8 bMAAimedPunchCtHModifier;
	UINT8 ubMAChanceToDodgeHtH;
	UINT8 ubMAOnTopCTDHtHBareHanded;
	UINT8 ubMAOnTopCTDHtHBrassKnuckles;
	UINT8 ubMAChanceToDodgeMelee;
	UINT8 ubMAReducedAPsToSteal;
	UINT8 ubMAAPsChangeStanceReduction;
	UINT8 ubMAApsTurnAroundReduction;
	UINT8 ubMAAPsClimbOrJumpReduction;
	UINT8 ubMAChanceToCkickDoors;
	BOOLEAN fPermitExtraAnimationsOnlyToMA;

	// SQUADLEADER
	UINT16 usSLRadiusNormal;
	UINT16 usSLRadiusExtendedEar;
	UINT8 ubSLMaxBonuses;
	UINT8 ubSLBonusAPsPercent;
	UINT8 ubSLEffectiveLevelInRadius;
	UINT8 ubSLEffectiveLevelAsStandby;
	UINT8 ubSLOverallSuppresionBonusPercent;
	UINT8 ubSLMoraleGainBonus;
	UINT8 ubSLMoraleLossReduction;
	UINT8 ubSLFearResistance;
	UINT8 ubSLDeathMoralelossMultiplier;

	// TECHNICIAN
	UINT16 usTERepairSpeedBonus;
	UINT16 usTELockpickingBonus;
	UINT16 usTEDisarmElTrapBonus;
	UINT16 usTEAttachingItemsBonus;
	UINT8 ubTEUnjamGunBonus;
	UINT8 ubTERepairElectronicsPenaltyReduction;
	UINT8 ubTEChanceToDetectTrapsBonus;
	UINT8 ubTECtHControlledRobotBonus;
	UINT8 ubTERepairRobotPenaltyReduction;
	UINT8 ubTETraitsNumToRepairRobot;

	// DOCTOR
	UINT8 ubDONumberTraitsNeededForSurgery;
	UINT8 ubDOSurgeryHealPercentBase;
	UINT8 ubDOSurgeryHealPercentOnTop;
	UINT16 usDOSurgeryMedBagConsumption;
	UINT16 usDOSurgeryMaxBreathLoss;
	UINT16 usDORepairStatsRateBasic;
	UINT16 usDORepairStatsRateOnTop;
	UINT8 ubDORepStPenaltyIfAlsoHealing;
	UINT8 ubDOHealingPenaltyIfAlsoStatRepair;
	BOOLEAN fDORepStShouldThrowMessage;
	UINT8 ubDOBandagingSpeedPercent;
	UINT16 usDODoctorAssignmentBonus;
	UINT8 ubDONaturalRegenBonus;
	UINT8 ubDOMaxRegenBonuses;

	// AMBIDEXTROUS
	UINT8 ubAMPenaltyDoubleReduction;
	UINT8 ubAMReloadSpeedMagazines;
	UINT8 ubAMReloadSpeedLoose;
	UINT8 ubAMPickItemsAPsReduction;
	UINT8 ubAMWorkBackpackAPsReduction;
	UINT8 ubAMHandleDoorsAPsReduction;
	UINT8 ubAMHandleBombsAPsReduction;
	UINT8 ubAMAttachingItemsAPsReduction;

	// MELEE
	UINT8 ubMEBladesAPsReduction;
	UINT8 ubMECtHBladesBonus;
	UINT8 ubMECtHBluntBonus;
	UINT8 ubMEDamageBonusBlades;
	UINT8 ubMEDamageBonusBlunt;
	UINT16 usMEAimedMeleeAttackDamageBonus;
	UINT8 ubMEDodgeBladesBonus;
	UINT8 ubMECtDBladesOnTopWithBladeInHands;
	UINT8 ubMEDodgeBluntBonus;
	UINT8 ubMECtDBluntOnTopWithBladeInHands;

	// THROWING
	UINT8 ubTHBladesAPsReduction;
	UINT8 ubTHBladesMaxRange;
	UINT8 ubTHBladesCtHBonus;
	UINT8 ubTHBladesCtHBonusPerClick;
	UINT8 ubTHBladesDamageBonus;
	UINT8 ubTHBladesDamageBonusPerClick;
	UINT8 ubTHBladesSilentCriticalHitChance;
	UINT8 ubTHBladesCriticalHitMultiplierBonus;
	UINT8 ubTHBladesAimClicksAdded;

	// NIGHT OPS
	UINT8 ubNOeSightRangeBonusInDark;
	UINT8 ubNOHearingRangeBonus;
	UINT8 ubNOHearingRangeBonusInDark;
	UINT8 ubNOIterruptsBonusInDark;
	UINT8 ubNONeedForSleepReduction;

	// STEALTHY
	UINT8 ubSTStealthModeSpeedBonus;
	UINT8 ubSTBonusToMoveQuietly;
	UINT8 ubSTStealthBonus;
	UINT8 ubSTStealthPenaltyForMovingReduction;

	// ATHLETICS
	UINT8 ubATAPsMovementReduction;
	UINT8 ubATBPsMovementReduction;

	// BODYBUILDING
	UINT8 ubBBDamageResistance;
	UINT8 ubBBCarryWeightBonus;
	UINT8 ubBBBreathLossForHtHImpactReduction;
	UINT16 usBBIncreasedNeededDamageToFallDown;

	// DEMOLITIONS
	UINT8 ubDEAPsNeededToThrowGrenadesReduction;
	UINT8 ubDEMaxRangeToThrowGrenades;
	UINT8 ubDECtHWhenThrowingGrenades;
	UINT8 ubDEDamageOfBombsAndMines;
	UINT8 ubDEAttachDetonatorCheckBonus;
	UINT8 ubDEPlantAndRemoveBombCheckBonus;
	UINT8 ubDEPlacedBombLevelBonus;
	UINT8 ubDEShapedChargeDamageMultiplier;

	// TEACHING
	UINT8 ubTGBonusToTrainMilitia;
	UINT8 ubTGEffectiveLDRToTrainMilitia;
	UINT8 ubTGBonusToTeachOtherMercs;
	UINT8 ubTGEffectiveSkillValueForTeaching;
	UINT8 ubTGBonusOnPractising;

	// SCOUTING
	UINT8 ubSCSightRangebonusWithScopes;
	UINT16 usSCSightRangebonusWithBinoculars;
	UINT8 ubSCTunnelVisionReducedWithBinoculars;
	BOOLEAN fSCCanDetectEnemyPresenseAround;
	BOOLEAN fSCCanDetermineEnemyNumbersAround;
	BOOLEAN fSCDetectionInDiagonalSectors;
	BOOLEAN fSCPreventsTheEnemyToAmbushMercs;
	BOOLEAN fSCPreventsBloodcatsAmbushes;
	BOOLEAN fSCThrowMessageIfAmbushPrevented;

} SKILL_TRAIT_VALUES;

// HEADROCK HAM 4: Constants used as coefficients by the various parts of the new CTH system.
typedef struct
{
	UINT32 NORMAL_SHOOTING_DISTANCE;		// Distance at which 1x magnification is 100% effective. This is a major component of the entire shooting mechanism.
	FLOAT DEGREES_MAXIMUM_APERTURE;			// Maximum possible aperture for a 100% muzzle sway shot. Decrease to make all shots more accurate.
	FLOAT RANGE_COEFFICIENT;				// Determines maximum range which decides when gravity forces bullets to drop
	FLOAT GRAVITY_COEFFICIENT;				// Changes the way gravity works in the game. Higher values mean bullets don't drop as quickly after reaching max range.
	FLOAT VERTICAL_BIAS;					// This float can be used to reduce the chance of missing too far upwards or downwards (compared to left/right).
	FLOAT SCOPE_RANGE_MULTIPLIER;			// Adjusts the minimum effective range of scopes
	FLOAT SIDE_FACING_DIVISOR;				// Deals with a visual error in NCTH relating to shooting at a target who is facing directly perpendicular to the shooters facing.

	FLOAT BASE_EXP;				// Importance of Experience for BASE CTH
	FLOAT BASE_MARKS;				// Importance of Marksmanship for BASE CTH
	FLOAT BASE_WIS;				// Importance of Wisdom for BASE CTH
	FLOAT BASE_DEX;				// Importance of Dexterity for BASE CTH

	FLOAT BASE_LOW_MORALE;		// Applied gradually when morale is below 50.
	FLOAT BASE_HIGH_MORALE;		// Applied gradually when morale is above 50.
	FLOAT BASE_PSYCHO;			// Applied for each level of PSYCHO trait
	FLOAT BASE_SHOOTING_UPWARDS;	// Applied gradually for shooting at a higher target. Decreases with distance.
	FLOAT BASE_INJURY;			// Applied gradually for injuries
	FLOAT BASE_DRUNK[4];			// Applied for drunkness levels
	FLOAT BASE_FATIGUE;			// Applied gradually for fatigue
	FLOAT BASE_SAME_TARGET;		// Applied for shooting at the same target again
	FLOAT BASE_GASSED;			// Applied for shooting while breathing gas
	FLOAT BASE_BEING_BANDAGED;	// Applied for shooting while being bandaged
	FLOAT BASE_SHOCK;				// Applied gradually for shock points
	FLOAT BASE_AGILE_TARGET;		// Applied gradually for agile or experienced target
	FLOAT BASE_TARGET_INVISIBLE;	// Applied for shooting at a target you can't see
	FLOAT BASE_DRAW_COST;			// Applied per 1 AP of the weapon's Ready Cost, under 100AP.
	FLOAT BASE_TWO_GUNS;			// Gun Difficulty Multiplier for shooting two guns
	FLOAT BASE_ONE_HANDED;		// Gun Difficulty Multiplier for shooting a pistol with one hand.
	FLOAT BASE_STANDING_STANCE;	// Gun Difficulty Multiplier for shooting from a standing stance
	FLOAT BASE_CROUCHING_STANCE;	// Gun Difficulty Multiplier for shooting from a crouched stance
	FLOAT BASE_PRONE_STANCE;		// Gun Difficulty Multiplier for shooting from a prone stance
	FLOAT BASE_HEAVY_WEAPON;		// Gun Difficulty Multiplier for shooting a launcher
	FLOAT BASE_DIFFICULTY[6];		// Applied for game difficulty

	FLOAT AIM_EXP;				// Importance of Experience for AIMING CTH
	FLOAT AIM_MARKS;				// Importance of Marksmanship for AIMING CTH
	FLOAT AIM_WIS;				// Importance of Wisdom for AIMING CTH
	FLOAT AIM_DEX;				// Importance of Dexterity for AIMING CTH

	FLOAT AIM_TOO_CLOSE_SCOPE;	// Applied per tile closer than the scope's minimum range.
	FLOAT AIM_GUN_CONDITION;		// Applied per point of condition below 50.
	FLOAT AIM_LOW_MORALE;			// Applied gradually when morale is below 50.
	FLOAT AIM_HIGH_MORALE;		// Applied gradually when morale is above 50.
	FLOAT AIM_PSYCHO;				// Applied for each level of PSYCHO trait
	FLOAT AIM_VISIBILITY;			// Applied for bad visibility
	FLOAT AIM_SHOOTING_UPWARDS;	// Applied gradually for shooting at a higher target. Decreases with distance.
	FLOAT AIM_INJURY;				// Applied gradually for injuries
	FLOAT AIM_DRUNK[4];			// Applied for drunkness levels
	FLOAT AIM_FATIGUE;			// Applied gradually for fatigue
	FLOAT AIM_GASSED;				// Applied for shooting while breathing gas
	FLOAT AIM_BEING_BANDAGED;		// Applied for shooting while being bandaged
	FLOAT AIM_SHOCK;				// Applied gradually for shock points
	FLOAT AIM_TARGET_INVISIBLE;	// Applied for shooting at a target you can't see
	FLOAT AIM_SNIPER_SKILL;		// Applied as percentage bonus to CTH cap, once per each level.
	FLOAT AIM_DRAW_COST;			// Applied per 1 AP of the weapon's Ready Cost, under 100AP.
	FLOAT AIM_STANDING_STANCE;	// Gun Difficulty multiplier for shooting a heavy gun when standing
	FLOAT AIM_CROUCHING_STANCE;	// Gun Difficulty multiplier for shooting a heavy gun from a crouched stance
	FLOAT AIM_PRONE_STANCE;		// Gun Difficulty multiplier for shooting a heavy gun when prone, before bipod bonuses
	FLOAT AIM_TWO_GUNS;			// Gun Difficulty multiplier for shooting two guns (halved for each Ambidextrous level)
	FLOAT AIM_ONE_HANDED;			// Gun Difficulty multiplier for firing a pistol with one hand.
	FLOAT AIM_HEAVY_WEAPON;		// Gun Difficulty multiplier for shooting a launcher (halved for each Heavy Weapons level)
	FLOAT AIM_DIFFICULTY[6];		// Applied for game difficulty

	FLOAT MOVEMENT_MRK;
	FLOAT MOVEMENT_WIS;
	FLOAT MOVEMENT_DEX;
	FLOAT MOVEMENT_EXP_LEVEL;
	UINT32 MOVEMENT_TRACKING_DIFFICULTY;
	FLOAT MOVEMENT_PENALTY_PER_TILE;

	FLOAT PRE_RECOIL_WIS;
	FLOAT PRE_RECOIL_EXP_LEVEL;
	FLOAT PRE_RECOIL_AUTO_WEAPONS_SKILL;

	FLOAT RECOIL_MAX_COUNTER_STR;
	FLOAT RECOIL_MAX_COUNTER_AGI;
	FLOAT RECOIL_MAX_COUNTER_EXP_LEVEL;
	FLOAT RECOIL_MAX_COUNTER_FORCE;
	FLOAT RECOIL_MAX_COUNTER_CROUCH;
	FLOAT RECOIL_MAX_COUNTER_PRONE;
	FLOAT RECOIL_COUNTER_ACCURACY_MIN_ERROR;
	FLOAT RECOIL_COUNTER_ACCURACY_DEX;
	FLOAT RECOIL_COUNTER_ACCURACY_WIS;
	FLOAT RECOIL_COUNTER_ACCURACY_AGI;
	FLOAT RECOIL_COUNTER_ACCURACY_EXP_LEVEL;
	FLOAT RECOIL_COUNTER_ACCURACY_AUTO_WEAPONS_DIVISOR;
	FLOAT RECOIL_COUNTER_ACCURACY_TRACER_BONUS;
	FLOAT RECOIL_COUNTER_ACCURACY_ANTICIPATION;
	FLOAT RECOIL_COUNTER_ACCURACY_COMPENSATION;
	FLOAT RECOIL_COUNTER_FREQUENCY_AGI;
	FLOAT RECOIL_COUNTER_FREQUENCY_EXP_LEVEL;
	FLOAT RECOIL_COUNTER_FREQUENCY_AUTO_WEAPONS_DIVISOR;

	UINT8 RECOIL_COUNTER_INCREMENT;
	UINT8 RECOIL_COUNTER_INCREMENT_TRACER;
	UINT32 NORMAL_RECOIL_DISTANCE;
	FLOAT MAX_BULLET_DEV;
	BOOLEAN RANGE_EFFECTS_DEV;
	FLOAT MAX_EFFECTIVE_RANGE_MULTIPLIER;
	FLOAT MAX_EFFECTIVE_RANGE_REDUCTION;
	BOOLEAN MAX_EFFECTIVE_USE_GRADIENT;

} CTH_CONSTANTS;

//This structure will contain general Ja2 settings	NOT individual game settings.
extern GAME_SETTINGS		gGameSettings;

//This structure will contain the Game options set at the beginning of the game.
extern GAME_OPTIONS		gGameOptions;

// Snap: Options read from an INI file in the default of custom Data directory
extern GAME_EXTERNAL_OPTIONS gGameExternalOptions; 

extern SKILL_TRAIT_VALUES gSkillTraitValues;  // SANDRO - added this one

// HEADROCK HAM 4: CTH constants read from a separate INI file
extern CTH_CONSTANTS gGameCTHConstants;

// WDS - Automatically try to save when an assertion failure occurs
extern bool alreadySaving;

BOOLEAN	SaveGameSettings();
BOOLEAN LoadGameSettings();
// Snap: Read options from an INI file in the default of custom Data directory
void LoadGameExternalOptions();
void LoadSkillTraitsExternalSettings(); // SANDRO - added this one
void LoadGameAPBPConstants();
// HEADROCK HAM 4: Read CTH/Shooting coefficients from file
void LoadCTHConstants();
void FreeGameExternalOptions();

void InitGameOptions();

void InitGameSettings();

BOOLEAN GetCDLocation( );

void DisplayGameSettings( );

BOOLEAN MeanwhileSceneSeen( UINT8 ubMeanwhile );

BOOLEAN SetMeanwhileSceneSeen( UINT8 ubMeanwhile );

BOOLEAN	CanGameBeSaved();

INT16 DynamicAdjustAPConstants(INT16 iniReadValue, INT16 iniDefaultValue, BOOLEAN reverse = FALSE);

#endif

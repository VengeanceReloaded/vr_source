#ifndef __INTERFACE_PANELS
#define __INTERFACE_PANELS

struct EXTENDED_PANEL_STRING
{
	UINT8 uiFontColorLeft;
	UINT8 uiFontColorRight;
	CHAR16 sPanelStringLeft[32];
	CHAR16 sPanelStringRight[32];
	UINT8 uiFontR;
	UINT8 uiFontL;
};

typedef enum
{
	STANCEUP_BUTTON = 0,
	UPDOWN_BUTTON,
	CLIMB_BUTTON,
	STANCEDOWN_BUTTON,
	HANDCURSOR_BUTTON,
	PREVMERC_BUTTON,
	NEXTMERC_BUTTON,
	OPTIONS_BUTTON,
	BURSTMODE_BUTTON,
	LOOK_BUTTON,
	TALK_BUTTON,
	MUTE_BUTTON,
	SM_DONE_BUTTON,
	SM_MAP_SCREEN_BUTTON,
	//HOP_BUTTON, //Legion by Jazz 
	NUM_SM_BUTTONS
};


typedef enum
{
	TEAM_DONE_BUTTON = 0,
	TEAM_MAP_SCREEN_BUTTON,
	CHANGE_SQUAD_BUTTON,
	NUM_TEAM_BUTTONS
};


#define NEW_ITEM_CYCLE_COUNT				19
#define NEW_ITEM_CYCLES						4
#define	NUM_TEAM_SLOTS						10


#define	PASSING_ITEM_DISTANCE_OKLIFE		3
#define	PASSING_ITEM_DISTANCE_NOTOKLIFE		2

#define	SHOW_LOCATOR_NORMAL					1
#define SHOW_LOCATOR_FAST					2
#define	SHOW_LOCATOR_NOISE					3

//extern INT16 INTERFACE_CLOCK_X;
//extern INT16 INTERFACE_CLOCK_Y;
extern int	LOCATION_NAME_X;
extern int	LOCATION_NAME_Y;


BOOLEAN CreateSMPanelButtons( );
void	RemoveSMPanelButtons( );
BOOLEAN InitializeSMPanel(	);
BOOLEAN ShutdownSMPanel( );
void RenderSMPanel( BOOLEAN *pfDirty );
void EnableSMPanelButtons( BOOLEAN fEnable, BOOLEAN fFromItemPickup );


/* In this function i will initialize all coords for SingleMercPanel
 * If i will success i will return true, i think that could be usefull when
 * we will have our panes more customizable for ex. we can read coords from file
 * and when we will read all values we return TRUE if not we will retunf FALSE
 * any questions? joker
 */
//BOOLEAN InitializeSMPanelCoords( ); 
BOOLEAN InitializeSMPanelCoordsNew();
BOOLEAN InitializeSMPanelCoordsOld(); 


BOOLEAN CreateTEAMPanelButtons( );
void	RemoveTEAMPanelButtons( );
BOOLEAN InitializeTEAMPanel(	);
BOOLEAN ShutdownTEAMPanel( );
void RenderTEAMPanel( BOOLEAN fDirty );
void UpdateTEAMPanel( );

/*
 * Just like b4 we will initialize all coords but now for TEAMPanel
 */
BOOLEAN InitializeTEAMPanelCoords( ); 

void SetSMPanelCurrentMerc( UINT8 ubNewID );
void SetTEAMPanelCurrentMerc( UINT8 ubNewID );
UINT16 GetSMPanelCurrentMerc(	);
void UpdateSMPanel( );

BOOLEAN InitTEAMSlots( );
void AddPlayerToInterfaceTeamSlot( UINT8 ubID );
BOOLEAN RemovePlayerFromInterfaceTeamSlot( UINT8 ubID );
BOOLEAN GetPlayerIDFromInterfaceTeamSlot( UINT8 ubPanelSlot, UINT8 *pubID );
void RemoveAllPlayersFromSlot( );
BOOLEAN PlayerExistsInSlot( UINT8 ubID );
BOOLEAN RemovePlayerFromTeamSlotGivenMercID( UINT8 ubMercID );
void CheckForAndAddMercToTeamPanel( SOLDIERTYPE *pSoldier );
void CleanUpStack( OBJECTTYPE * pObj, OBJECTTYPE * pCursorObj );

void DisableTacticalTeamPanelButtons( BOOLEAN fDisable );
// CHRISL: Add X,Y coords as parameters so we can control the placement of TownID string on the team and inventory panels seperately
void RenderTownIDString( INT16 sX, INT16 sY );
void KeyRingSlotInvClickCallback( MOUSE_REGION * pRegion, INT32 iReason );
// CHRISL: New functions to handle initialization of inventory systems
void InitNewInventorySystem( );
void InitOldInventorySystem( );



// ATE TO BE MOVED TO INTERFACE_ITEMS.C
extern INT8		gbNewItem[ NUM_INV_SLOTS ];
extern INT8		gbNewItemCycle[ NUM_INV_SLOTS ];
extern UINT8	gubNewItemMerc;

void ShowRadioLocator( UINT8 ubID, UINT8 ubLocatorSpeed );
void EndRadioLocator( UINT8 ubID );


extern MOUSE_REGION	gSMPanelRegion;
extern UINT32		guiSecItemHiddenVO;
extern BOOLEAN		gfDisableTacticalPanelButtons;


typedef struct
{
	UINT8	ubID;
	BOOLEAN	fOccupied;

} TEAM_PANEL_SLOTS_TYPE;

extern TEAM_PANEL_SLOTS_TYPE	gTeamPanel[ NUM_TEAM_SLOTS ];



//Used when the shop keeper interface is active
void DisableSMPpanelButtonsWhenInShopKeeperInterface( BOOLEAN fDontDrawButtons );
//void DisableSMPpanelButtonsWhenInShopKeeperInterface( );

//
void ReEvaluateDisabledINVPanelButtons( );
void CheckForReEvaluateDisabledINVPanelButtons( );

void CheckForDisabledForGiveItem( );
void ReevaluateItemHatches( SOLDIERTYPE *pSoldier, BOOLEAN fEnable );
//CHRISL: New function to deal with rendering backpack buttons
void RenderBackpackButtons(int bpAction);

void HandlePanelFaceAnimations( SOLDIERTYPE *pSoldier );

void GoToMapScreenFromTactical( void );

// Moved to weapons.h by ADB, rev 1513
//void HandleTacticalEffectsOfEquipmentChange( SOLDIERTYPE *pSoldier, UINT32 uiInvPos, UINT16 usOldItem, UINT16 usNewItem );

void FinishAnySkullPanelAnimations( );

UINT8 FindNextMercInTeamPanel( SOLDIERTYPE *pSoldier, BOOLEAN fGoodForLessOKLife, BOOLEAN fOnlyRegularMercs );

// CHRISL: New callback functions for backpack buttons
BOOLEAN ChangeDropPackStatus(SOLDIERTYPE *pSoldier, BOOLEAN newStatus);

// HEADROCK HAM 4: This variable tells the program what "page" of the description box we are currently looking at.
// By saving this in memory, we tell the DescBox which background and values to draw, and make sure that we end up
// on the same page every time we open the description box. UDB buttons allow switching between pages.
extern UINT8 gubDescBoxPage;
// silversurfer: Now we also have a secondary page on the general tab for weapons so we can look at their secondary attributes.
extern UINT8 gubDescGenPage;
// Record which line we're looking at. This only applied to the Advanced tab in description boxes.
extern UINT8 gubDescBoxLine;
// Record how many lines there are for the current item in the Advanced tab.
extern UINT8 gubDescBoxTotalAdvLines;

// Jenilee: determine the cost of moving this item around in our inventory
UINT16 GetInvMovementCost(OBJECTTYPE* pObj, INT16 old_pos, INT16 new_pos);

// anv - extended panel functions
typedef enum
{
	EXTENDED_PANEL_AGILITY,
	EXTENDED_PANEL_DEXTERITY,
	EXTENDED_PANEL_EXP_LVL,
	EXTENDED_PANEL_STRENGTH,
	EXTENDED_PANEL_EXPLOSIVES,
	EXTENDED_PANEL_MARKSMANSHIP,
	EXTENDED_PANEL_LEADERSHIP,
	EXTENDED_PANEL_WISDOM,
	EXTENDED_PANEL_MECHANICAL,
	EXTENDED_PANEL_MEDICAL
};

BOOLEAN DrawExtendedPanel(SOLDIERTYPE *pSoldier, UINT16 screenWidth );
EXTENDED_PANEL_STRING GetNameString(SOLDIERTYPE *pSoldier);
EXTENDED_PANEL_STRING GetKillsString(SOLDIERTYPE *pSoldier);
EXTENDED_PANEL_STRING GetAssistsString(SOLDIERTYPE *pSoldier);
EXTENDED_PANEL_STRING GetBattlesString(SOLDIERTYPE *pSoldier);
EXTENDED_PANEL_STRING GetShotsFiredString(SOLDIERTYPE* pSoldier);
EXTENDED_PANEL_STRING GetShotsHitString(SOLDIERTYPE* pSoldier);
EXTENDED_PANEL_STRING GetHitPercentageString(SOLDIERTYPE *pSoldier);
EXTENDED_PANEL_STRING GetTimesWoundedString(SOLDIERTYPE *pSoldier);
EXTENDED_PANEL_STRING GetAchievementsString(SOLDIERTYPE* pSoldier);
EXTENDED_PANEL_STRING *GetSkillsStrings(SOLDIERTYPE *pSoldier, UINT16 uiSkills);
EXTENDED_PANEL_STRING GetDisabilityString(SOLDIERTYPE *pSoldier);
EXTENDED_PANEL_STRING GetCharacterString(SOLDIERTYPE *pSoldier);
EXTENDED_PANEL_STRING GetAttitudeOldString(SOLDIERTYPE* pSoldier);
EXTENDED_PANEL_STRING GetCurrentContractString(SOLDIERTYPE *pSoldier);
EXTENDED_PANEL_STRING GetTotalServiceString(SOLDIERTYPE *pSoldier);
EXTENDED_PANEL_STRING GetTotalCostString(SOLDIERTYPE *pSoldier);
EXTENDED_PANEL_STRING GetMedicalDepositString(SOLDIERTYPE *pSoldier);
EXTENDED_PANEL_STRING GetDailyCostString(SOLDIERTYPE *pSoldier);
EXTENDED_PANEL_STRING GetSquadString(SOLDIERTYPE* pSoldier);
EXTENDED_PANEL_STRING GetStatString(SOLDIERTYPE* pSoldier, UINT16 uiStat);
INT32 ReturnFont(STR8 FontName);
EXTENDED_PANEL_STRING GetTextString(STR16 sStringL, STR16 sStringR);
EXTENDED_PANEL_STRING GetEmptyString();
BOOLEAN WriteStringOnPanel(EXTENDED_PANEL_STRING expsString, UINT16 uiLine, UINT16 uiPanelX, UINT16 uiPanelY, UINT16 uiPanelWidth, UINT16 uiRowDist);

#endif
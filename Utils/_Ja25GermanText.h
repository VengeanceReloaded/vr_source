#ifndef _JA25GERMANTEXT__H_
#define _JA25GERMANTEXT__H_


enum
{
	//TCTL_MSG__RANGE_TO_TARGET,
	TCTL_MSG__ATTACH_TRANSMITTER_TO_LAPTOP,
	TACT_MSG__CANNOT_AFFORD_MERC,
	TACT_MSG__AIMMEMBER_FEE_TEXT,
	TACT_MSG__AIMMEMBER_ONE_TIME_FEE,
	TACT_MSG__FEE,
	TACT_MSG__SOMEONE_ELSE_IN_SECTOR,
	//TCTL_MSG__GUN_RANGE_AND_CTH,
	//TCTL_MSG__RANGE_TO_TARGET_AND_GUN_RANGE,	// WANNE: Not used
	TCTL_MSG__DISPLAY_COVER,
	TCTL_MSG__LOS,
	TCTL_MSG__INVALID_DROPOFF_SECTOR,
	TCTL_MSG__PLAYER_LOST_SHOULD_RESTART,
	TCTL_MSG__JERRY_BREAKIN_LAPTOP_ANTENA,
	TCTL_MSG__END_GAME_POPUP_TXT_1,
	TCTL_MSG__END_GAME_POPUP_TXT_2,
	TCTL_MSG__IRON_MAN_CANT_SAVE_NOW,
	TCTL_MSG__CANNOT_SAVE_DURING_COMBAT,
	TCTL_MSG__CAMPAIGN_NAME_TOO_LARGE,
	TCTL_MSG__CAMPAIGN_DOESN_T_EXIST,
	TCTL_MSG__DEFAULT_CAMPAIGN_LABEL,
	TCTL_MSG__CAMPAIGN_LABEL,
	TCTL_MSG__NEW_CAMPAIGN_CONFIRM,
	TCTL_MSG__CANT_EDIT_DEFAULT,

};
extern	STR16	zNewTacticalMessages[];
extern	STR16 gzIMPSkillTraitsText[];

////////////////////////////////////////////////////////
// added by SANDRO
extern	STR16 gzIMPSkillTraitsTextNewMajor[];
extern	STR16 gzIMPSkillTraitsTextNewMinor[];

extern  STR16 gzIMPMajorTraitsHelpTextsAutoWeapons[];
extern  STR16 gzIMPMajorTraitsHelpTextsHeavyWeapons[];
extern  STR16 gzIMPMajorTraitsHelpTextsSniper[];
extern  STR16 gzIMPMajorTraitsHelpTextsRanger[];
extern  STR16 gzIMPMajorTraitsHelpTextsGunslinger[];
extern  STR16 gzIMPMajorTraitsHelpTextsMartialArts[];
extern  STR16 gzIMPMajorTraitsHelpTextsSquadleader[];
extern  STR16 gzIMPMajorTraitsHelpTextsTechnician[];
extern  STR16 gzIMPMajorTraitsHelpTextsDoctor[];
extern  STR16 gzIMPMajorTraitsHelpTextsNone[];

extern	STR16 gzIMPMinorTraitsHelpTextsAmbidextrous[];
extern	STR16 gzIMPMinorTraitsHelpTextsMelee[];
extern	STR16 gzIMPMinorTraitsHelpTextsThrowing[];
extern	STR16 gzIMPMinorTraitsHelpTextsStealthy[];
extern	STR16 gzIMPMinorTraitsHelpTextsNightOps[];
extern	STR16 gzIMPMinorTraitsHelpTextsAthletics[];
extern	STR16 gzIMPMinorTraitsHelpTextsBodybuilding[];
extern	STR16 gzIMPMinorTraitsHelpTextsDemolitions[];
extern	STR16 gzIMPMinorTraitsHelpTextsTeaching[];
extern	STR16 gzIMPMinorTraitsHelpTextsScouting[];
extern	STR16 gzIMPMinorTraitsHelpTextsNone[];

extern STR16 gzIMPOldSkillTraitsHelpTexts[];

extern STR16 gzIMPNewCharacterTraitsHelpTexts[];

extern STR16 gzIMPDisabilitiesHelpTexts[];

extern STR16 gzIMPProfileCostText[];

extern STR16 zGioNewTraitsImpossibleText[];
///////////////////////////////////////////////////////

enum
{
	IMM__IRON_MAN_MODE_WARNING_TEXT,
};
extern	STR16	gzIronManModeWarningText[];

// display cover message (for tactical usually, seperated)
// display cover terrain type info (used in cover information)
enum
{
	DC_MSG__COVER_INFORMATION,
	DC_MSG__GUN_RANGE_INFORMATION,
	DC_MSG__COVER_DRAW_OFF,
	DC_MSG__COVER_DRAW_MERC_VIEW,
	DC_MSG__COVER_DRAW_ENEMY_VIEW,
	DC_TTI__WOOD,
	DC_TTI__URBAN,
	DC_TTI__DESERT,
	DC_TTI__SNOW,
	DC_TTI__WOOD_AND_DESERT,
	DC_TTI__UNKNOWN
};
extern STR16 gzDisplayCoverText[];

#endif



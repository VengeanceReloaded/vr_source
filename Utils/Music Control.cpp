#ifdef PRECOMPILEDHEADERS
	#include "Utils All.h"
#else
	#include "types.h"
	#include "Music Control.h"
	#include "soundman.h"
	#include "Random.h"
	#include "jascreens.h"
	#include "overhead.h"
	#include "timer control.h"
	#include "strategicmap.h"
#endif

#include "Overhead Types.h"
//extern int iScreenMode;

static UINT32 uiMusicHandle = NO_SAMPLE;
static BOOLEAN fMusicPlaying = FALSE;

static BOOLEAN fMusicFadingOut = FALSE;
static BOOLEAN fMusicFadingIn = FALSE;
static UINT32 uiMusicVolume = 50;

static BOOLEAN gfMusicEnded = FALSE;

static UINT8 gubMusicMode = 0;

static UINT8 gubOldMusicMode = 0;

static INT8 gbVictorySongCount = 0;
static INT8 gbDeathSongCount = 0;

#ifdef NEWMUSIC
static INT32 bMainMenuModeSong = MENUMIX_MUSIC;
static INT32 bLaptopModeSong = MARIMBAD2_MUSIC;
static INT32 bCreditsModeSong = CREDITS_MUSIC;
static INT32 bNothingModeSong = NOTHING_A_MUSIC;
static INT32 bEnemyModeSong = TENSOR_A_MUSIC;
static INT32 bBattleModeSong = BATTLE_A_MUSIC;
static INT32 bCreepyModeSong = CREEPY_MUSIC;
static INT32 bCreepyBattleModeSong = CREATURE_BATTLE_MUSIC;

static INT32 NewSoundID = -1;
static BOOLEAN SetSoundID = FALSE;

static INT32 gubOldMusicMode2 = 0;
#else
static INT8 bMainMenuModeSong;
static INT8 bLaptopModeSong;
static INT8 bCreditsModeSong;
static INT8 bNothingModeSong;
static INT8 bEnemyModeSong;
static INT8 bBattleModeSong;
static INT8 bCreepyModeSong;
static INT8 bCreepyBattleModeSong;
#endif

static BOOLEAN gfUseCreatureMusic = FALSE;

static INT8 gbFadeSpeed = 1;

static BOOLEAN gfDontRestartSong = FALSE;
// unused
//BOOLEAN	gfForceMusicToTense = FALSE;


CHAR8 *szMusicList[NUM_MUSIC]=
{
	"MUSIC\\marimbad 2",
	"MUSIC\\marimbad 2 B",
	"MUSIC\\marimbad 2 C",
	"MUSIC\\marimbad 2 D",
	"MUSIC\\marimbad 2 E",
	"MUSIC\\marimbad 2 F",
	"MUSIC\\marimbad 2 G",
	"MUSIC\\marimbad 2 H",
	"MUSIC\\menumix1",
	"MUSIC\\menumix1 B",
	"MUSIC\\menumix1 C",
	"MUSIC\\menumix1 D",
	"MUSIC\\nothing A",
	"MUSIC\\nothing B",
	"MUSIC\\nothing C",
	"MUSIC\\nothing D",
	"MUSIC\\nothing E",
	"MUSIC\\nothing F",
	"MUSIC\\nothing G",
	"MUSIC\\nothing H",
	"MUSIC\\nothing I",
	"MUSIC\\nothing J",
	"MUSIC\\nothing K",
	"MUSIC\\nothing L",
	"MUSIC\\nothing M",
	"MUSIC\\nothing N",
	"MUSIC\\nothing O",
	"MUSIC\\nothing P",
	"MUSIC\\nothing Q",
	"MUSIC\\nothing R",
	"MUSIC\\nothing S",
	"MUSIC\\nothing T",
	"MUSIC\\nothing U",
	"MUSIC\\nothing V",
	"MUSIC\\nothing W",
	"MUSIC\\nothing X",
	"MUSIC\\nothing Y",
	"MUSIC\\nothing Z",
	"MUSIC\\nothing Z1",
	"MUSIC\\nothing Z2",
	"MUSIC\\nothing Z3",
	"MUSIC\\nothing Z4",
	"MUSIC\\nothing Z5",
	"MUSIC\\nothing Z6",
	"MUSIC\\nothing Z7",
	"MUSIC\\nothing Z8",
	"MUSIC\\nothing Z9",
	"MUSIC\\nothing Z10",
	"MUSIC\\nothing Z11",
	"MUSIC\\nothing Z12",
	"MUSIC\\nothing Z13",
	"MUSIC\\nothing Z14",
	"MUSIC\\nothing Z15",
	"MUSIC\\nothing Z16",
	"MUSIC\\nothing Z17",
	"MUSIC\\nothing Z18",
	"MUSIC\\nothing Z19",
	"MUSIC\\nothing Z20",
	"MUSIC\\tensor A",
	"MUSIC\\tensor B",
	"MUSIC\\tensor C",
	"MUSIC\\tensor D",
	"MUSIC\\tensor E",
	"MUSIC\\tensor F",
	"MUSIC\\tensor G",
	"MUSIC\\tensor H",
	"MUSIC\\tensor I",
	"MUSIC\\tensor J",
	"MUSIC\\tensor K",
	"MUSIC\\tensor L",
	"MUSIC\\tensor M",
	"MUSIC\\tensor N",
	"MUSIC\\tensor O",
	"MUSIC\\tensor P",
	"MUSIC\\tensor Q",
	"MUSIC\\tensor R",
	"MUSIC\\tensor S",
	"MUSIC\\tensor T",
	"MUSIC\\tensor U",
	"MUSIC\\tensor V",
	"MUSIC\\tensor W",
	"MUSIC\\tensor X",
	"MUSIC\\tensor Y",
	"MUSIC\\tensor Z",
	"MUSIC\\triumph",
	"MUSIC\\death",
	"MUSIC\\battle A",
	"MUSIC\\battle B",
	"MUSIC\\battle C",
	"MUSIC\\battle D",
	"MUSIC\\battle E",
	"MUSIC\\battle F",
	"MUSIC\\battle G",
	"MUSIC\\battle H",
	"MUSIC\\battle I",
	"MUSIC\\battle J",
	"MUSIC\\battle K",
	"MUSIC\\battle L",
	"MUSIC\\battle M",
	"MUSIC\\battle N",
	"MUSIC\\battle O",
	"MUSIC\\battle P",
	"MUSIC\\battle Q",
	"MUSIC\\battle R",
	"MUSIC\\battle S",
	"MUSIC\\battle T",
	"MUSIC\\battle U",
	"MUSIC\\battle V",
	"MUSIC\\battle W",
	"MUSIC\\battle X",
	"MUSIC\\battle Y",
	"MUSIC\\battle Z",
	"MUSIC\\creepy",
	"MUSIC\\creepy B",
	"MUSIC\\creepy C",
	"MUSIC\\creepy D",
	"MUSIC\\creature battle",
	"MUSIC\\creature battle B",
	"MUSIC\\creature battle C",
	"MUSIC\\creature battle D",
	"MUSIC\\credits",
	"MUSIC\\credits B",
	"MUSIC\\credits C",
	"MUSIC\\credits D",
	"MUSIC",
};

BOOLEAN StartMusicBasedOnMode(void);
UINT8 GetAmountOfTracksBetween(UINT8 ubStartIndex, UINT8 ubEndIndex);
void DoneFadeOutDueToEndMusic(void);
void MusicStopCallback(void *pData);
BOOLEAN MusicStop(void);
BOOLEAN MusicFadeOut(void);
BOOLEAN MusicFadeIn(void);

#ifdef NEWMUSIC
MUSIC_SOUND_VALUES MusicSoundValues[256];
INT32 GlobalSoundID = -1;
#endif
//extern void HandleEndDemoInCreatureLevel( );

//BOOLEAN NoEnemiesInSight( )
//{
//	SOLDIERTYPE			 *pSoldier;
//	INT32										cnt;
//
//	// Loop through our guys
//	// End the turn of player charactors
//	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;
//
//	// look for all mercs on the same team,
//	for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++, pSoldier++ )
//	{
//		if ( pSoldier->bActive && pSoldier->stats.bLife >= OKLIFE )
//		{
//			if ( pSoldier->aiData.bOppCnt != 0 )
//			{
//				return( FALSE );
//			}
//		}
//	}
//
//	return( TRUE );
//}



//********************************************************************************
// MusicPlay
//
//		Starts up one of the tunes in the music list.
//
//	Returns:	TRUE if the music was started, FALSE if an error occurred
//
//********************************************************************************
#ifdef NEWMUSIC
BOOLEAN MusicPlay(UINT32 uiNum, UINT32 MusicMode, BOOLEAN NewSound )
#else
BOOLEAN MusicPlay(UINT32 uiNum)
#endif
{
	// WANNE: We want music in windowed mode
	//if( 1==iScreenMode ) /* on Windowed mode, skip the music? was coded for WINDOWED_MODE that way...*/
	//return FALSE;

	static CHAR8 zFileName[164];
	SOUNDPARMS spParms;

	if(fMusicPlaying)
		MusicStop();

	memset(&spParms, 0xff, sizeof(SOUNDPARMS));
	spParms.uiPriority = PRIORITY_MAX;
	spParms.uiVolume = 0;
	spParms.uiLoop = 1;	// Lesh: only 1 line added

	spParms.EOSCallback = MusicStopCallback;

	//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "About to call SoundPlayStreamedFile" );

	// Lesh: patch to allow playback ogg files
#ifdef NEWMUSIC
	if ( NewSound == FALSE && MusicMode == MUSIC_OLD_TYPE )
	{
#endif
	sprintf( zFileName, "%s.ogg", szMusicList[uiNum] );
	if ( !FileExists( zFileName ) )
		sprintf( zFileName, "%s.wav", szMusicList[uiNum] );
#ifdef NEWMUSIC
	}
	else if ( NewSound == TRUE )
	{
		//sprintf( zFileName, szMusicList[15] );
		if ( MusicMode == MUSIC_TACTICAL_NOTHING ) 
		{
			sprintf( zFileName, "%s\\NOTHING_%d.ogg", szMusicList[15], uiNum);
			if ( !FileExists( zFileName ) )
				sprintf( zFileName, "%s\\NOTHING_%d.wav", szMusicList[15], uiNum );
		}
		else if ( MusicMode == MUSIC_TACTICAL_ENEMYPRESENT ) 
		{
			sprintf( zFileName, "%s\\TENSOR_%d.ogg", szMusicList[15], uiNum);
			if ( !FileExists( zFileName ) )
				sprintf( zFileName, "%s\\TENSOR_%d.wav", szMusicList[15], uiNum );
		}
		else if ( MusicMode == MUSIC_TACTICAL_BATTLE ) 
		{
			sprintf( zFileName, "%s\\BATTLE_%d.ogg", szMusicList[15], uiNum);
			if ( !FileExists( zFileName ) )
				sprintf( zFileName, "%s\\BATTLE_%d.wav", szMusicList[15], uiNum );
		}
		else if ( MusicMode == MUSIC_TACTICAL_VICTORY ) 
		{
			sprintf( zFileName, "%s\\TRIUMPH_%d.ogg", szMusicList[15], uiNum);
		if ( !FileExists( zFileName ) )
				sprintf( zFileName, "%s\\TRIUMPH_%d.wav", szMusicList[15], uiNum );
		}
		else if ( MusicMode == MUSIC_TACTICAL_BATTLE_MUSIC ) 
		{
			sprintf( zFileName, "%s\\CREATURE_BATTLE_%d.ogg", szMusicList[15], uiNum);
			if ( !FileExists( zFileName ) )
				sprintf( zFileName, "%s\\CREATURE_BATTLE_%d.wav", szMusicList[15], uiNum );
		}
		else if ( MusicMode == MUSIC_TACTICAL_CREEPY_MUSIC ) 
		{
			sprintf( zFileName, "%s\\CREEPY_%d.ogg", szMusicList[15], uiNum);
			if ( !FileExists( zFileName ) )
				sprintf( zFileName, "%s\\CREEPY_%d.wav", szMusicList[15], uiNum );
		}
		else if ( MusicMode == OTHER_MUSIC_TACTICAL ) 
		{
			sprintf( zFileName, "%s\\%d.ogg", szMusicList[15], uiNum);
			if ( !FileExists( zFileName ) )
				sprintf( zFileName, "%s\\%d.wav", szMusicList[15], uiNum );
		}
		
	}
#endif
	uiMusicHandle = SoundPlayStreamedFile(zFileName, &spParms);

	if(uiMusicHandle != SOUND_ERROR)
	{
		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music PLay %d %d", uiMusicHandle, gubMusicMode	) );

		gfMusicEnded = FALSE;
		fMusicPlaying = TRUE;
		MusicFadeIn();
		return TRUE;
	}

	//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music PLay %d %d", uiMusicHandle, gubMusicMode	) );
	return FALSE;
}

//********************************************************************************
// MusicSetVolume
//
//		Sets the volume on the currently playing music.
//
//	Returns:	TRUE if the volume was set, FALSE if an error occurred
//
//********************************************************************************
BOOLEAN MusicSetVolume(UINT32 uiVolume)
{
	INT32 uiOldMusicVolume = uiMusicVolume;

	// WANNE: We want music in windowed mode
	//if( 1==iScreenMode ) /* on Windowed mode, skip the music? was coded for WINDOWED_MODE that way...*/
	//return FALSE;

	uiMusicVolume = __min(uiVolume, 127);

	if(uiMusicHandle != NO_SAMPLE)
	{
		// get volume and if 0 stop music!
		if (uiMusicVolume == 0)
		{
			gfDontRestartSong = TRUE;
			MusicStop();
			return TRUE;
		}

		SoundSetVolume(uiMusicHandle, uiMusicVolume);

		return TRUE;
	}

	// If here, check if we need to re-start music
	// Have we re-started?
	if (uiMusicVolume > 0 && uiOldMusicVolume == 0)
	{
		StartMusicBasedOnMode();
	}

	return FALSE;
}

//********************************************************************************
// MusicGetVolume
//
//		Gets the volume on the currently playing music.
//
//	Returns:	TRUE if the volume was set, FALSE if an error occurred
//
//********************************************************************************
UINT32 MusicGetVolume(void)
{
	return uiMusicVolume;
}

//********************************************************************************
// MusicStop
//
//		Stops the currently playing music.
//
//	Returns:	TRUE if the music was stopped, FALSE if an error occurred
//
//********************************************************************************
static BOOLEAN MusicStop(void)
{
	// WANNE: We want music in windowed mode
	//if( 1==iScreenMode ) /* on Windowed mode, skip the music? was coded for WINDOWED_MODE that way...*/
	//	return(FALSE);

	if(uiMusicHandle != NO_SAMPLE)
	{
		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music Stop %d %d", uiMusicHandle, gubMusicMode ) );

		SoundStop(uiMusicHandle);
		fMusicPlaying = FALSE;
		uiMusicHandle = NO_SAMPLE;
		return TRUE;
	}

	//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music Stop %d %d", uiMusicHandle, gubMusicMode ) );
	return FALSE;
}

//********************************************************************************
// MusicFadeOut
//
//		Fades out the current song.
//
//	Returns:	TRUE if the music has begun fading, FALSE if an error occurred
//
//********************************************************************************
static BOOLEAN MusicFadeOut(void)
{
	if(uiMusicHandle != NO_SAMPLE)
	{
		fMusicFadingOut = TRUE;
		return TRUE;
	}
	return FALSE;
}

//********************************************************************************
// MusicFadeIn
//
//		Fades in the current song.
//
//	Returns:	TRUE if the music has begun fading in, FALSE if an error occurred
//
//********************************************************************************
static BOOLEAN MusicFadeIn(void)
{
	if(uiMusicHandle != NO_SAMPLE)
	{
		fMusicFadingIn = TRUE;
		return TRUE;
	}
	return FALSE;
}

//********************************************************************************
// MusicPoll
//
//		Handles any maintenance the music system needs done. Should be polled from
//	the main loop, or somewhere with a high frequency of calls.
//
//	Returns:	TRUE always
//
//********************************************************************************
BOOLEAN MusicPoll(BOOLEAN /*fForce*/)
{
	//DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"MusicPoll");

	// WANNE: We want music in windowed mode
	//if( 1==iScreenMode ) /* on Windowed mode, skip the music? was coded for WINDOWED_MODE that way...*/
	//	return(TRUE);

	INT32 iVol;

	//DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"MusicPoll: SoundServiceStreams ");
	SoundServiceStreams();
	//DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"MusicPoll: SoundServiceRandom ");
	SoundServiceRandom();

	//DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"MusicPoll: Handle Sound every sound overhead time");
	// Handle Sound every sound overhead time....
	if (COUNTERDONE(MUSICOVERHEAD))
	{
		//DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"MusicPoll: Reset counter");
		// Reset counter
		RESETCOUNTER(MUSICOVERHEAD);

		if (fMusicFadingIn)
		{
			//DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"MusicPoll: music fading in");
			if(uiMusicHandle != NO_SAMPLE)
			{
				iVol = SoundGetVolume(uiMusicHandle);
				iVol = __min( (INT32)uiMusicVolume, iVol+gbFadeSpeed );
				SoundSetVolume(uiMusicHandle, iVol);
				if(iVol == (INT32)uiMusicVolume)
				{
					fMusicFadingIn = FALSE;
					gbFadeSpeed = 1;
				}
			}
		}
		else if (fMusicFadingOut)
		{
			//DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"MusicPoll: music fading out");
			if(uiMusicHandle != NO_SAMPLE)
			{
				iVol = SoundGetVolume(uiMusicHandle);
				iVol = (iVol >=1)? iVol-gbFadeSpeed : 0;

				iVol = __max( (INT32)iVol, 0 );

				SoundSetVolume(uiMusicHandle, iVol);
				if(iVol == 0)
				{
					MusicStop();
					fMusicFadingOut = FALSE;
					gbFadeSpeed = 1;
				}
			}
		}

		if (gfMusicEnded)
		{
			//DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"MusicPoll: music ended");
			// OK, based on our music mode, play another!
			//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music End Loop %d %d", uiMusicHandle, gubMusicMode ) );

			// If we were in victory mode, change!
			if (gbVictorySongCount == 1 || gbDeathSongCount == 1)
			{
				if (gbDeathSongCount == 1 && guiCurrentScreen == GAME_SCREEN)
				{
					CheckAndHandleUnloadingOfCurrentWorld();
				}

				if (gbVictorySongCount == 1)
				{
					SetMusicMode(MUSIC_TACTICAL_NOTHING);
				}
			}
			else
			{
				if (!gfDontRestartSong)
				{
					//DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"MusicPoll: don't restart song, StartMusicBasedOnMode");
					StartMusicBasedOnMode();
				}
			}

			gfMusicEnded = FALSE;
			gfDontRestartSong = FALSE;
		}
	}

	//DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"MusicPoll done");
	return TRUE;
}
#ifdef NEWMUSIC
static BOOLEAN SetMusicModeID(UINT8 ubMusicMode, INT32 SoundID, BOOLEAN fForce)
{
	static INT8 bPreviousMode = 0;
	
	static INT32 bPreviousMusic = 0;
	
	// OK, check if we want to restore
	if (ubMusicMode == MUSIC_RESTORE)
	{
		if (bPreviousMode == MUSIC_TACTICAL_VICTORY || bPreviousMode == MUSIC_TACTICAL_DEATH)
		{
			bPreviousMode = MUSIC_TACTICAL_NOTHING;
		}	
		
		ubMusicMode = bPreviousMode;
		SoundID = bPreviousMusic;
		GlobalSoundID  = SoundID;
	}
	else
	{
		// Save previous mode...
		bPreviousMode = gubOldMusicMode;
		bPreviousMusic = SoundID;
		GlobalSoundID  = SoundID;
	}

	// if different, start a new music song
	//if ( ( fForce || gubOldMusicMode != ubMusicMode ) && SoundID != -1)
	if ( fForce || SoundID != -1 )
	{
		// Set mode....
		gubMusicMode = ubMusicMode;
		NewSoundID = SoundID;
		GlobalSoundID  = SoundID;

		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music New Mode %d %d", uiMusicHandle, gubMusicMode	) );

		gbVictorySongCount = 0;
		gbDeathSongCount = 0;

		if(uiMusicHandle != NO_SAMPLE)
		{
			// Fade out old music
			MusicFadeOut();
		}
		else
		{		
			// Change music!
			StartMusicBasedOnMode();
		}
	}

	gubOldMusicMode = gubMusicMode;
	gubOldMusicMode2 = SoundID;

	return TRUE;
}
#endif

static BOOLEAN SetMusicMode(UINT8 ubMusicMode, BOOLEAN fForce)
{
	static INT8 bPreviousMode = 0;

	#ifdef NEWMUSIC
	SetSoundID = FALSE;
	//GlobalSoundID  = -1;
	
	if ( SetSoundID == FALSE )
		NewSoundID = -1;	
	#endif

	// OK, check if we want to restore
	if (ubMusicMode == MUSIC_RESTORE)
	{
		if (bPreviousMode == MUSIC_TACTICAL_VICTORY || bPreviousMode == MUSIC_TACTICAL_DEATH)
		{
			bPreviousMode = MUSIC_TACTICAL_NOTHING;
		}
		
		#ifdef NEWMUSIC
		if ( GlobalSoundID == -1 )
		{
			SetSoundID = FALSE;
			GlobalSoundID  = -1;
			NewSoundID = -1;	
		}
		else if ( GlobalSoundID != -1)
		{
			SetSoundID = TRUE;
			NewSoundID = gubOldMusicMode2;	
			GlobalSoundID  = NewSoundID;
		}
		#endif
		ubMusicMode = bPreviousMode;
			
		
	}
	else
	{
		// Save previous mode...
		bPreviousMode = gubOldMusicMode;
	}

	// if different, start a new music song
	#ifdef NEWMUSIC
	if (fForce || gubOldMusicMode != ubMusicMode || GlobalSoundID == -1 || SetSoundID == TRUE )
	#else
	if (fForce || gubOldMusicMode != ubMusicMode)
	#endif
	{
		// Set mode....
		gubMusicMode = ubMusicMode;

		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music New Mode %d %d", uiMusicHandle, gubMusicMode	) );

		gbVictorySongCount = 0;
		gbDeathSongCount = 0;

		if(uiMusicHandle != NO_SAMPLE)
		{
			// Fade out old music
			MusicFadeOut();
		}
		else
		{
			// Change music!
			StartMusicBasedOnMode();
		}
	}

	gubOldMusicMode = gubMusicMode;

	return TRUE;
}

#ifdef NEWMUSIC
static BOOLEAN StartMusicBasedOnMode(void)
{
	static BOOLEAN fFirstTime = TRUE;


	if (fFirstTime)
	{
		fFirstTime = FALSE;

		// anv: check for available tracks
		ubMainMenuTracks = GetAmountOfTracksBetween(MENUMIX_MUSIC, MENUMIX_D_MUSIC);
		ubLaptopTracks = GetAmountOfTracksBetween(MARIMBAD2_MUSIC, MARIMBAD2_H_MUSIC);
		ubCreditsTracks = GetAmountOfTracksBetween(CREDITS_MUSIC, CREDITS_MUSIC_D);
		ubNothingTracks = GetAmountOfTracksBetween(NOTHING_A_MUSIC, NOTHING_Z_MUSIC);
		ubEnemyTracks = GetAmountOfTracksBetween(TENSOR_A_MUSIC, TENSOR_Z_MUSIC);
		ubBattleTracks = GetAmountOfTracksBetween(BATTLE_A_MUSIC, BATTLE_Z_MUSIC);
		ubCreepyEnemyTracks = GetAmountOfTracksBetween(CREEPY_MUSIC, CREEPY_MUSIC_D);
		ubCreepyBattleTracks = GetAmountOfTracksBetween(CREATURE_BATTLE_MUSIC, CREATURE_BATTLE_MUSIC_D);

		bMainMenuModeSong = (INT8)(MENUMIX_MUSIC + Random(ubMainMenuTracks));
		bLaptopModeSong = (INT8)(MARIMBAD2_MUSIC + Random(ubLaptopTracks));
		bCreditsModeSong = (INT8)(CREDITS_MUSIC + Random(ubCreditsTracks));
		bNothingModeSong = (INT8) (NOTHING_A_MUSIC + Random(ubNothingTracks));
		bEnemyModeSong = (INT8) (TENSOR_A_MUSIC + Random(ubEnemyTracks));
		bBattleModeSong = (INT8) (BATTLE_A_MUSIC + Random(ubBattleTracks));
		bCreepyModeSong = (INT8) (CREEPY_MUSIC + Random(ubCreepyEnemyTracks));
		bCreepyBattleModeSong = (INT8) (CREATURE_BATTLE_MUSIC + Random(ubCreepyBattleTracks));
	}


	//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "StartMusicBasedOnMode() %d %d", uiMusicHandle, gubMusicMode ) );

	// Setup a song based on mode we're in!
	switch(gubMusicMode)
	{
		case MUSIC_MAIN_MENU:
			// ATE: Don't fade in
			gbFadeSpeed = (INT8)uiMusicVolume;
			MusicPlay(bMainMenuModeSong,MUSIC_OLD_TYPE,FALSE);
			bMainMenuModeSong = (INT8)(MENUMIX_MUSIC + Random(ubMainMenuTracks));
			break;

		case MUSIC_LAPTOP:
			gbFadeSpeed = (INT8)uiMusicVolume;
			MusicPlay(bLaptopModeSong,MUSIC_OLD_TYPE,FALSE);
			bLaptopModeSong = (INT8)(MARIMBAD2_MUSIC + Random(ubLaptopTracks));
			break;

		case MUSIC_TACTICAL_NOTHING:
			// ATE: Don't fade in
			gbFadeSpeed = (INT8)uiMusicVolume;
			if(gfUseCreatureMusic)
			{
				if ( NewSoundID == -1)
				{
					SetSoundID = FALSE;
					MusicPlay(bCreepyModeSong,MUSIC_OLD_TYPE,FALSE);
				}
				else
				{
					SetSoundID = TRUE;
					MusicPlay(NewSoundID,MUSIC_TACTICAL_CREEPY_MUSIC,TRUE);
				}
				bCreepyEnemySong = (INT8) (CREEPY_MUSIC + Random(ubCreepyEnemyTracks));
			}
			else
			{
				if ( NewSoundID == -1)
				{
					SetSoundID = FALSE;
					MusicPlay(bNothingModeSong,MUSIC_OLD_TYPE,FALSE);
				}
				else
				{
					SetSoundID = TRUE;
					MusicPlay(NewSoundID,MUSIC_TACTICAL_NOTHING,TRUE);
				}							
				bNothingModeSong = (INT8) (NOTHING_A_MUSIC + Random(ubNothingTracks));			
			}
			break;

		case MUSIC_TACTICAL_ENEMYPRESENT:
			// ATE: Don't fade in EnemyPresent...
			gbFadeSpeed = (INT8)uiMusicVolume;
			if(gfUseCreatureMusic)
			{
				if ( NewSoundID == -1)
				{
					SetSoundID = FALSE;
					MusicPlay(bCreepyModeSong,MUSIC_OLD_TYPE,FALSE);
				}
				else
				{
					SetSoundID = TRUE;
					MusicPlay(NewSoundID,MUSIC_TACTICAL_ENEMYPRESENT,TRUE);
				}
				bCreepyEnemySong = (INT8) (CREEPY_MUSIC + Random(ubCreepyEnemyTracks));
			}
			else
			{
				if ( NewSoundID == -1)
				{
					SetSoundID = FALSE;
					MusicPlay(bEnemyModeSong,MUSIC_OLD_TYPE,FALSE);
				}
				else
				{
					SetSoundID = TRUE;
					MusicPlay(NewSoundID,MUSIC_TACTICAL_ENEMYPRESENT,TRUE);
				}
				bEnemyModeSong = (INT8) (TENSOR_A_MUSIC + Random(ubEnemyTracks));			
			}
			break;

		case MUSIC_TACTICAL_BATTLE:
			// ATE: Don't fade in
			gbFadeSpeed = (INT8)uiMusicVolume;
			if(gfUseCreatureMusic)
			{
				if ( NewSoundID == -1)
				{
					SetSoundID = FALSE;
					MusicPlay(bCreepyBattleModeSong,MUSIC_OLD_TYPE,FALSE);
				}
				else
				{
					SetSoundID = TRUE;
					MusicPlay(NewSoundID,MUSIC_TACTICAL_BATTLE_MUSIC,TRUE);
				}			
				bCreepyBattleModeSong = (INT8) (CREATURE_BATTLE_MUSIC + Random(ubCreepyBattleTracks));
			}
			else
			{
				if ( NewSoundID == -1)
				{
					SetSoundID = FALSE;
					MusicPlay(bBattleModeSong,MUSIC_OLD_TYPE,FALSE);
				}
				else
				{
					SetSoundID = TRUE;
					MusicPlay(NewSoundID,MUSIC_TACTICAL_BATTLE,TRUE);
				}
				bBattleModeSong = (INT8) (BATTLE_A_MUSIC + Random(ubBattleTracks));
			}
			break;

		case MUSIC_TACTICAL_VICTORY:

			// ATE: Don't fade in EnemyPresent...
			gbFadeSpeed = (INT8)uiMusicVolume;
			if ( NewSoundID == -1)
			{
				SetSoundID = FALSE;
				MusicPlay(TRIUMPH_MUSIC,MUSIC_OLD_TYPE,FALSE);
			}
			else
			{
				SetSoundID = TRUE;
				MusicPlay(NewSoundID,MUSIC_TACTICAL_VICTORY,TRUE);
			}
			gbVictorySongCount++;

			if(gfUseCreatureMusic && !gbWorldSectorZ)
			{
				//We just killed all the creatures that just attacked the town.
				gfUseCreatureMusic = FALSE;
			}
			break;

		case MUSIC_TACTICAL_DEATH:

			// ATE: Don't fade in EnemyPresent...
			gbFadeSpeed = (INT8)uiMusicVolume;
			if ( NewSoundID == -1)
			{
				SetSoundID = FALSE;
				MusicPlay(DEATH_MUSIC,MUSIC_OLD_TYPE,FALSE);
			}
			else
			{
				SetSoundID = TRUE;
				MusicPlay(NewSoundID,MUSIC_TACTICAL_DEATH,TRUE);
			}
			gbDeathSongCount++;
			break;

		case MUSIC_CREDITS:
			gbFadeSpeed = (INT8)uiMusicVolume;
			MusicPlay(bCreditsModeSong);
			bCreditsModeSong = (INT8)(CREDITS_MUSIC + Random(ubCreditsTracks));
			break;

		default:
			MusicFadeOut();
			break;
	}

	return TRUE;
}
#else
static BOOLEAN StartMusicBasedOnMode(void)
{
	static BOOLEAN fFirstTime = TRUE;
	static UINT8 ubMainMenuTracks = 0;
	static UINT8 ubLaptopTracks = 0;
	static UINT8 ubCreditsTracks = 0;
	static UINT8 ubNothingTracks = 0;
	static UINT8 ubEnemyTracks = 0;
	static UINT8 ubBattleTracks = 0;
	static UINT8 ubCreepyEnemyTracks = 0;
	static UINT8 ubCreepyBattleTracks = 0;

	if (fFirstTime)
	{
		fFirstTime = FALSE;

		// anv: check for available tracks
		ubMainMenuTracks = GetAmountOfTracksBetween(MENUMIX_MUSIC, MENUMIX_D_MUSIC);
		ubLaptopTracks = GetAmountOfTracksBetween(MARIMBAD2_MUSIC, MARIMBAD2_H_MUSIC);
		ubCreditsTracks = GetAmountOfTracksBetween(CREDITS_MUSIC, CREDITS_MUSIC_D);
		ubNothingTracks = GetAmountOfTracksBetween(NOTHING_A_MUSIC, NOTHING_Z20_MUSIC);
		ubEnemyTracks = GetAmountOfTracksBetween(TENSOR_A_MUSIC, TENSOR_Z_MUSIC);
		ubBattleTracks = GetAmountOfTracksBetween(BATTLE_A_MUSIC, BATTLE_Z_MUSIC);
		ubCreepyEnemyTracks = GetAmountOfTracksBetween(CREEPY_MUSIC, CREEPY_MUSIC_D);
		ubCreepyBattleTracks = GetAmountOfTracksBetween(CREATURE_BATTLE_MUSIC, CREATURE_BATTLE_MUSIC_D);

		bMainMenuModeSong = (INT8)(MENUMIX_MUSIC + Random(ubMainMenuTracks));
		bLaptopModeSong = (INT8)(MARIMBAD2_MUSIC + Random(ubLaptopTracks));
		bCreditsModeSong = (INT8)(CREDITS_MUSIC + Random(ubCreditsTracks));
		bNothingModeSong = (INT8) (NOTHING_A_MUSIC + Random(ubNothingTracks));
		bEnemyModeSong = (INT8) (TENSOR_A_MUSIC + Random(ubEnemyTracks));
		bBattleModeSong = (INT8) (BATTLE_A_MUSIC + Random(ubBattleTracks));
		bCreepyModeSong = (INT8) (CREEPY_MUSIC + Random(ubCreepyEnemyTracks));
		bCreepyBattleModeSong = (INT8) (CREATURE_BATTLE_MUSIC + Random(ubCreepyBattleTracks));
	}


	//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "StartMusicBasedOnMode() %d %d", uiMusicHandle, gubMusicMode ) );

	// Setup a song based on mode we're in!
	switch(gubMusicMode)
	{
		case MUSIC_MAIN_MENU:
			// ATE: Don't fade in
			gbFadeSpeed = (INT8)uiMusicVolume;
			MusicPlay(bMainMenuModeSong);
			bMainMenuModeSong = (INT8)(MENUMIX_MUSIC + Random(ubMainMenuTracks));
			break;

		case MUSIC_LAPTOP:
			gbFadeSpeed = (INT8)uiMusicVolume;
			MusicPlay(bLaptopModeSong);
			bLaptopModeSong = (INT8)(MARIMBAD2_MUSIC + Random(ubLaptopTracks));
			break;

		case MUSIC_TACTICAL_NOTHING:
			// ATE: Don't fade in
			gbFadeSpeed = (INT8)uiMusicVolume;
			if(gfUseCreatureMusic)
			{
				MusicPlay(bCreepyModeSong);
				bCreepyModeSong = (INT8) (CREEPY_MUSIC + Random(ubCreepyEnemyTracks));
			}
			else
			{
				MusicPlay(bNothingModeSong);
				bNothingModeSong = (INT8) (NOTHING_A_MUSIC + Random(ubNothingTracks));		
			}
			break;

		case MUSIC_TACTICAL_ENEMYPRESENT:
			// ATE: Don't fade in EnemyPresent...
			gbFadeSpeed = (INT8)uiMusicVolume;
			if(gfUseCreatureMusic)
			{
				MusicPlay(bCreepyModeSong);
				bCreepyModeSong = (INT8) (CREEPY_MUSIC + Random(ubCreepyEnemyTracks));
			}
			else
			{
				MusicPlay(bEnemyModeSong);
				bEnemyModeSong = (INT8) (TENSOR_A_MUSIC + Random(ubEnemyTracks));
			}
			break;

		case MUSIC_TACTICAL_BATTLE:
			// ATE: Don't fade in
			gbFadeSpeed = (INT8)uiMusicVolume;
			if(gfUseCreatureMusic)
			{
				MusicPlay(bCreepyBattleModeSong);
				bCreepyBattleModeSong = (INT8) (CREATURE_BATTLE_MUSIC + Random(ubCreepyBattleTracks));
			}
			else
			{
				MusicPlay(bBattleModeSong);
				bBattleModeSong = (INT8) (BATTLE_A_MUSIC + Random(ubBattleTracks));
			}
			break;

		case MUSIC_TACTICAL_VICTORY:

			// ATE: Don't fade in EnemyPresent...
			gbFadeSpeed = (INT8)uiMusicVolume;
			MusicPlay(TRIUMPH_MUSIC);
			gbVictorySongCount++;

			if(gfUseCreatureMusic && !gbWorldSectorZ)
			{
				//We just killed all the creatures that just attacked the town.
				gfUseCreatureMusic = FALSE;
			}
			break;

		case MUSIC_TACTICAL_DEATH:

			// ATE: Don't fade in EnemyPresent...
			gbFadeSpeed = (INT8)uiMusicVolume;
			MusicPlay(DEATH_MUSIC);
			gbDeathSongCount++;
			break;

		case MUSIC_CREDITS:
			gbFadeSpeed = (INT8)uiMusicVolume;
			MusicPlay(bCreditsModeSong);
			bCreditsModeSong = (INT8)(CREDITS_MUSIC + Random(ubCreditsTracks));
			break;

		default:
			MusicFadeOut();
			break;
	}

	return TRUE;
}
#endif

UINT8 GetAmountOfTracksBetween(UINT8 ubStartIndex, UINT8 ubEndIndex)
{
	UINT8 ExistingSndsFiles = 0;
	SGPFILENAME		zFileName;
	for( UINT8 i = ubStartIndex; i < ubEndIndex + 1; i++)
	{
		sprintf( zFileName, "%s.ogg", szMusicList[i] );
		if( !FileExists( zFileName ) )
		{
			sprintf( zFileName, "%s.wav", szMusicList[i] );
			if( FileExists( zFileName ) )
			{
				ExistingSndsFiles++;
			}
			else
				break;
		}
		else
		{
			ExistingSndsFiles++;
		}
	}
	return ExistingSndsFiles;
}

BOOLEAN SetMusicMode(UINT8 ubMusicMode)
{
	return SetMusicMode(ubMusicMode, FALSE);
}

#ifdef NEWMUSIC
BOOLEAN SetMusicModeID(UINT8 ubMusicMode, INT32 SoundID)
{
	return SetMusicModeID(ubMusicMode, SoundID, FALSE);
}
#endif

static void MusicStopCallback(void *pData)
{
	//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music EndCallback %d %d", uiMusicHandle, gubMusicMode	) );

	gfMusicEnded = TRUE;
	uiMusicHandle = NO_SAMPLE;

	//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Music EndCallback completed" );
}


void SetMusicFadeSpeed(INT8 bFadeSpeed)
{
	gbFadeSpeed = bFadeSpeed;
}

UINT8 GetMusicMode(void)
{
	return gubMusicMode;
}

BOOLEAN UsingCreatureMusic(void)
{
	return gfUseCreatureMusic;
}

void UseCreatureMusic(BOOLEAN fUseCreatureMusic)
{
	if (gfUseCreatureMusic != fUseCreatureMusic)
	{
		// this means a change
		gfUseCreatureMusic = fUseCreatureMusic;
		SetMusicMode(gubMusicMode, TRUE);	// same as before
	}
}

BOOLEAN IsMusicPlaying(void)
{
	return fMusicPlaying;
}

UINT32 GetMusicHandle(void)
{
	return uiMusicHandle;
}

// unused
//void FadeMusicForXSeconds( UINT32 uiDelay )
//{
//	INT16 sNumTimeSteps, sNumVolumeSteps;
//
//	// get # time steps in delay....
//	sNumTimeSteps = (INT16)( uiDelay / 10 );
//
//	// Devide this by music volume...
//	sNumVolumeSteps = (INT16)( uiMusicVolume / sNumTimeSteps );
//
//	// Set fade delay...
//	SetMusicFadeSpeed( (INT8)sNumVolumeSteps );
//}

// unused
//void	DoneFadeOutDueToEndMusic( void )
//{
//	// Quit game....
//	InternalLeaveTacticalScreen( MAINMENU_SCREEN );
//	//SetPendingNewScreen( MAINMENU_SCREEN );
//}



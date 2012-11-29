#ifdef PRECOMPILEDHEADERS
	#include "JA2 All.h"
#else
	#include "Types.h"
	#include "GameVersion.h"
#endif
 
	
//
//	Keeps track of the game version
//
 
#ifdef JA2EDITOR

//MAP EDITOR BUILD VERSION
CHAR16 zVersionLabel[256]		= { L"Map Editor v1.13.4870 (Official 4870 Update)" }; 
#elif defined JA2BETAVERSION

//BETA/TEST BUILD VERSION
CHAR16 zVersionLabel[256]		= { L"Debug v1.13.4870 (Official 4870 Update)" }; 
#elif defined CRIPPLED_VERSION

//RELEASE BUILD VERSION s
CHAR16 zVersionLabel[256]		= { L"Beta v. 0.98" }; 

#else

	//RELEASE BUILD VERSION	
	CHAR16		zVersionLabel[256]		= { L"Vengeance: Reloaded 1.13.4870+ (no asserts)" };
#endif
										
CHAR8		czVersionNumber[16]	= { "Build 12.09.21" };		//YY.MM.DD
CHAR16		zTrackingNumber[16]	= { L"Z" };
	

// SAVE_GAME_VERSION is defined in header, change it there

#ifdef PRECOMPILEDHEADERS
	#include "JA2 All.h"
#else
	#include "Types.h"
	#include "GameVersion.h"
#endif
 
//
//	Keeps track of the game version
//
 
// ------------------------------
// MAP EDITOR (Release and Debug) BUILD VERSION
// ------------------------------
#ifdef JA2EDITOR
	
	#ifdef JA2UB
		CHAR16 zVersionLabel[256]		= { L"Unfinished Business - Map Editor v1.13.7609 (2014 Official Release)" }; 
	#else
		CHAR16 zVersionLabel[256]		= { L"Map Editor Vengeance: Reloaded 1.13.7609+" }; 
	#endif

// ------------------------------
// DEBUG BUILD VERSIONS
// ------------------------------
#elif defined JA2BETAVERSION

	//DEBUG BUILD VERSION	
	#ifdef JA2UB
		CHAR16 zVersionLabel[256]		= { L"Debug: Unfinished Business - v1.13.7609 (2014 Official Release)" }; 
	#elif defined (JA113DEMO)
		CHAR16 zVersionLabel[256]		= { L"Debug: Jagged Alliance 2 Demo - v1.13.7609 (2014 Official Release)" }; 
	#else
		CHAR16 zVersionLabel[256]		= { L"Debug: JA2 Vengeance: Reloaded 1.13.7609+" }; 
	#endif

#elif defined CRIPPLED_VERSION

	//RELEASE BUILD VERSION s
	CHAR16 zVersionLabel[256]		= { L"Beta v. 0.98" }; 

// ------------------------------
// RELEASE BUILD VERSIONS
// ------------------------------
#else

	//RELEASE BUILD VERSION	
	#ifdef JA2UB
		CHAR16		zVersionLabel[256]		= { L"Unfinished Business - v1.13.7609 (2014 Official Release)" };
	#elif defined (JA113DEMO)
		CHAR16		zVersionLabel[256]		= { L"Jagged Alliance 2 Demo - v1.13.7609 (2014 Official Release)" };
	#else
		CHAR16		zVersionLabel[256]		= { L"JA2 Vengeance: Reloaded 1.13.7609+" };
	#endif

#endif
										
CHAR8		czVersionNumber[16]	= { "Build 18.07.16" };		//YY.MM.DD
CHAR16		zTrackingNumber[16]	= { L"Z" };
	
// SAVE_GAME_VERSION is defined in header, change it there

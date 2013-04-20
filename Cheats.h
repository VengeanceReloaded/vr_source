#ifndef _CHEATS__H_
#define _CHEATS__H_

#include "Language Defines.h"


extern	UINT8			gubCheatLevel;




// GLOBALS FOR CHEAT MODE......
#ifdef JA2TESTVERSION
	#define						STARTING_CHEAT_LEVEL						8
#elif defined JA2BETAVERSION
	#define						STARTING_CHEAT_LEVEL						3
#else
	#define						STARTING_CHEAT_LEVEL						0
#endif



	// ATE: remove cheats unless we're doing a debug build
//#ifdef JA2TESTVERSION
	#define						INFORMATION_CHEAT_LEVEL( )			( gubCheatLevel >= 3 )
	#define						CHEATER_CHEAT_LEVEL( )					( gubCheatLevel >= 7 )
	#define						DEBUG_CHEAT_LEVEL( )					( gubCheatLevel >= 8 )
//#else
//	#define						INFORMATION_CHEAT_LEVEL( )			( FALSE )
//	#define						CHEATER_CHEAT_LEVEL( )					( FALSE )
//	#define						DEBUG_CHEAT_LEVEL( )					( FALSE )
//#endif

	#define						RESET_CHEAT_LEVEL( )						( gubCheatLevel = 0 )


#endif
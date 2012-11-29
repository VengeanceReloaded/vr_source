#ifdef JA2_PRECOMPILEDHEADERS
	#include "Tactical All.h"
#else
	#include "sgp.h"
	#include "overhead types.h"
	#include "Sound Control.h"
	#include "overhead.h"
	#include "Event Pump.h"
	#include "weapons.h"
	#include "Animation Control.h"
	#include "sys globals.h"
	#include "Handle UI.h"
	#include "Isometric Utils.h"
	#include "worldman.h"
	#include "math.h"
	#include "points.h"
	#include "ai.h"
	#include "los.h"
	#include "renderworld.h"
	#include "opplist.h"
	#include "interface.h"
	#include "message.h"
	#include "campaign.h"
	#include "items.h"
	#include "text.h"
	#include "tile animation.h"
	#include "Dialogue Control.h"
	#include "SkillCheck.h"
	#include "explosion control.h"
	#include "Quests.h"
	#include "Physics.h"
	#include "Random.h"
	#include "Vehicles.h"
	#include "bullets.h"
	#include "morale.h"
	#include "meanwhile.h"
	#include "SkillCheck.h"
	#include "gamesettings.h"
	#include "SaveLoadMap.h"
	#include "Debug Control.h"
	#include "expat.h"
	#include "XML.h"
#endif

struct
{
	PARSE_STAGE	curElement;

	CHAR8		szCharData[MAX_CHAR_DATA_LENGTH+1];

	UINT32			maxArraySize;
	UINT32			curIndex;
	UINT32			currentDepth;
	UINT32			maxReadDepth;
}
typedef stringParseData;

static void XMLCALL
stringStartElementHandle(void *userData, const XML_Char *name, const XML_Char **atts)
{
	stringParseData * pData = (stringParseData *)userData;

	if(pData->currentDepth <= pData->maxReadDepth) //are we reading this element?
	{
		if(strcmp(name, "STRINGLIST") == 0 && pData->curElement == ELEMENT_NONE)
		{
			pData->curElement = ELEMENT_LIST;

			pData->maxReadDepth++; //we are not skipping this element
		}
		else if(strcmp(name, "STRING") == 0 && pData->curElement == ELEMENT_LIST)
		{
			pData->curElement = ELEMENT_PROPERTY;

			pData->maxReadDepth++; //we are not skipping this element
			pData->curIndex++;
		}

		pData->szCharData[0] = '\0';
	}

	pData->currentDepth++;

}

static void XMLCALL
stringCharacterDataHandle(void *userData, const XML_Char *str, int len)
{
	stringParseData * pData = (stringParseData *)userData;

	if( (pData->currentDepth <= pData->maxReadDepth) &&
		(strlen(pData->szCharData) < MAX_CHAR_DATA_LENGTH)
	){
		strncat(pData->szCharData,str,__min((unsigned int)len,MAX_CHAR_DATA_LENGTH-strlen(pData->szCharData)));
	}
}


static void XMLCALL
stringEndElementHandle(void *userData, const XML_Char *name)
{
	stringParseData * pData = (stringParseData *)userData;

	if(pData->currentDepth <= pData->maxReadDepth) //we're at the end of an element that we've been reading
	{
		if(strcmp(name, "STRINGLIST") == 0)
		{
			pData->curElement = ELEMENT_NONE;
		}
		else if(strcmp(name, "STRING") == 0)
		{
			pData->curElement = ELEMENT;

			if(pData->curIndex < pData->maxArraySize)
			{
				swprintf(AmmoCaliber[pData->curIndex], L"%S", pData->szCharData);
			}
		}

		pData->maxReadDepth--;
	}

	pData->currentDepth--;
}




BOOLEAN ReadInStringArray()
{
	HWFILE		hFile;
	UINT32		uiBytesRead;
	UINT32		uiFSize;
	CHAR8 *		lpcBuffer;
	XML_Parser	parser = XML_ParserCreate(NULL);

	stringParseData pData;

	DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Loading %s",AMMOCALIBERSTRINGSFILENAME ) );

	// Open strings file
	hFile = FileOpen( AMMOCALIBERSTRINGSFILENAME, FILE_ACCESS_READ, FALSE );
	if ( !hFile )
		return( FALSE );

	uiFSize = FileGetSize(hFile);
	lpcBuffer = (CHAR8 *) MemAlloc(uiFSize+1);

	//Read in block
	if ( !FileRead( hFile, lpcBuffer, uiFSize, &uiBytesRead ) )
	{
		MemFree(lpcBuffer);
		return( FALSE );
	}

	lpcBuffer[uiFSize] = 0; //add a null terminator

	FileClose( hFile );


	XML_SetElementHandler(parser, stringStartElementHandle, stringEndElementHandle);
	XML_SetCharacterDataHandler(parser, stringCharacterDataHandle);


	memset(&pData,0,sizeof(pData));
	pData.maxArraySize = MAXITEMS;
	pData.curIndex = 0xffffffff;

	XML_SetUserData(parser, &pData);


	if(!XML_Parse(parser, lpcBuffer, uiFSize, TRUE))
	{
		CHAR8 errorBuf[511];

		sprintf(errorBuf, "XML Parser Error in %s.xml: %s at line %d", AMMOCALIBERSTRINGSFILENAME, XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser));
		LiveMessage(errorBuf);

		MemFree(lpcBuffer);
		return FALSE;
	}

	MemFree(lpcBuffer);


	XML_ParserFree(parser);

	return( TRUE );
}
BOOLEAN WriteStringArray()
{
	HWFILE		hFile;
	DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("WriteStringArray"));
	//Debug code; make sure that what we got from the file is the same as what's there
	// Open a new file
	hFile = FileOpen( "TABLEDATA\\AmmoCaliberStrings out.xml", FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS, FALSE );
	if ( !hFile )
		return( FALSE );

	{
		UINT32 cnt;

		FilePrintf(hFile,"<STRINGLIST>\r\n");
		for(cnt = 0;cnt < 29;cnt++)
		{
			FilePrintf(hFile,"\t<STRING>");

			STR16 szRemainder = AmmoCaliber[cnt]; //the remaining string to be output (for making valid XML)

			while(szRemainder[0] != '\0')
			{
				UINT32 uiCharLoc = wcscspn(szRemainder,L"&<>\'\"\0");
				CHAR16 invChar = szRemainder[uiCharLoc];

				if(uiCharLoc)
				{
					szRemainder[uiCharLoc] = '\0';
					FilePrintf(hFile,"%S",szRemainder);
					szRemainder[uiCharLoc] = invChar;
				}

				szRemainder += uiCharLoc;

				switch(invChar)
				{
					case '&':
						FilePrintf(hFile,"&amp;");
						szRemainder++;
					break;

					case '<':
						FilePrintf(hFile,"&lt;");
						szRemainder++;
					break;

					case '>':
						FilePrintf(hFile,"&gt;");
						szRemainder++;
					break;

					case '\'':
						FilePrintf(hFile,"&apos;");
						szRemainder++;
					break;

					case '\"':
						FilePrintf(hFile,"&quot;");
						szRemainder++;
					break;
				}
			}

//			FilePrintf(hFile,"\t\t<STRING>%s</STRING>\r\n",						AmmoCaliber[cnt]);

			FilePrintf(hFile,"</STRING>\r\n");
		}
		FilePrintf(hFile,"</STRINGLIST>\r\n");
	}
	FileClose( hFile );

	return( TRUE );
}


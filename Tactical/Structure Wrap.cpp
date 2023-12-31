#ifdef PRECOMPILEDHEADERS
	#include "Tactical All.h"
#else
	#include <stdio.h>
	#include "debug.h"
	#include "worlddef.h"
	#include "worldman.h"
	#include "structure wrap.h"
	#include "isometric utils.h"
	#include "worldman.h"
	#include "overhead.h"
	#include "renderworld.h"
	#include "strategicmap.h"
	#include "rotting corpses.h"
#endif

extern BOOLEAN DoesSAMExistHere( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ, INT32 sGridNo );



//----------------legion by Jazz

BOOLEAN	IsJumpableWindowPresentAtGridNo( INT32 sGridNo, INT8 direction2, BOOLEAN fIntactWindowsAlso )
{
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_WALLNWINDOW );

	if ( pStructure )
	{
		// anv: VR - quick fix in place of additional tile properties for now
		LEVELNODE *pNode = FindLevelNodeBasedOnStructure(sGridNo, pStructure);
		if (pNode != NULL)
		{
			if ((giCurrentTilesetID == 54 && pNode->usIndex >= 682 && pNode->usIndex <= 693) ||
				((giCurrentTilesetID == 12 || giCurrentTilesetID == 22 || giCurrentTilesetID == 41 || giCurrentTilesetID == 44) && pNode->usIndex >= 877 && pNode->usIndex <= 888))
				// special tag disables jumping through specific windows (e.g. barred windows in Tixa)
				return(FALSE);
		}

		if ( ( direction2 == SOUTH || direction2 == NORTH ) && (pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT || pStructure->ubWallOrientation == INSIDE_TOP_LEFT ) && pStructure->fFlags & STRUCTURE_WALLNWINDOW && !(pStructure->fFlags & STRUCTURE_SPECIAL) )
	    {
			if ( fIntactWindowsAlso || ( pStructure->fFlags & STRUCTURE_OPEN ) )
         		return( TRUE );
	    }
	                            	
        if ( ( direction2 == EAST || direction2 == WEST ) && ( pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT || pStructure->ubWallOrientation == INSIDE_TOP_RIGHT ) && pStructure->fFlags & STRUCTURE_WALLNWINDOW && !(pStructure->fFlags & STRUCTURE_SPECIAL) )
		{
			if ( fIntactWindowsAlso || ( pStructure->fFlags & STRUCTURE_OPEN ) )
				return( TRUE );
	    }
	}

	return( FALSE );
}


BOOLEAN	IsOknoFencePresentAtGridno( INT32 sGridNo )
{

	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_WALLNWINDOW );

	if ( pStructure )
	{
		if ( pStructure->fFlags & STRUCTURE_WALLNWINDOW && !(pStructure->fFlags & STRUCTURE_SPECIAL) && ( pStructure->fFlags & STRUCTURE_OPEN ) )
		{
			return( TRUE );
		}
	}
	
/*	STRUCTURE * pStructure;
	STRUCTURE * pStructure2;

	pStructure = FindStructure( sGridNo, STRUCTURE_WALLNWINDOW );

	if ( pStructure )
	{
	//	pStructure2 = FindStructure( sGridNo, STRUCTURE_WALL );
              // 	if ( !pStructure2 )
              //	{
        	if ( pStructure->fFlags & STRUCTURE_WALLNWINDOW && !(pStructure->fFlags & STRUCTURE_SPECIAL)  && ( pStructure->fFlags & STRUCTURE_OPEN ))
		{
			return( TRUE );
		}
		
	//	}
		
	}
	
	*/
	return( FALSE );
}

BOOLEAN	IsLegionWallPresentAtGridno( INT32 sGridNo )
{
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_FENCE );

	if ( pStructure )
	{
		
		if ( pStructure->fFlags & STRUCTURE_FENCE && pStructure->fFlags & STRUCTURE_SPECIAL && pStructure->fFlags & STRUCTURE_WALL ) 
		{
			return( TRUE );
		}
	}
	return( FALSE );
}
//-------------------------------------------------------------------------------------

BOOLEAN	IsFencePresentAtGridNo( INT32 sGridNo )
{
	if ( FindStructure( sGridNo, STRUCTURE_ANYFENCE ) != NULL )
	{
		return( TRUE );
	}

	return( FALSE );
}

BOOLEAN	IsRoofPresentAtGridNo( INT32 sGridNo )
{
	if ( FindStructure( sGridNo, STRUCTURE_ROOF ) != NULL )
	{
		return( TRUE );
	}

	return( FALSE );
}


BOOLEAN	IsJumpableFencePresentAtGridNo( INT32 sGridNo )
{
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_OBSTACLE );

	if ( pStructure )
	{
		if ( pStructure->fFlags & STRUCTURE_FENCE && !(pStructure->fFlags & STRUCTURE_SPECIAL) )
		{
			return( TRUE );
		}
		if ( pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_SANDBAG && StructureHeight( pStructure ) < 2 )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


BOOLEAN	IsDoorPresentAtGridNo( INT32 sGridNo )
{
	if ( FindStructure( sGridNo, STRUCTURE_ANYDOOR ) != NULL )
	{
		return( TRUE );
	}

	return( FALSE );
}


BOOLEAN	IsTreePresentAtGridNo( INT32 sGridNo )
{
	if ( FindStructure( sGridNo, STRUCTURE_TREE ) != NULL )
	{
		return( TRUE );
	}

	return( FALSE );
}


LEVELNODE *IsWallPresentAtGridNo( INT32 sGridNo )
{
	LEVELNODE *pNode = NULL;
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_WALLSTUFF );

	if ( pStructure != NULL )
	{
		pNode = FindLevelNodeBasedOnStructure( sGridNo, pStructure );
	}

	return( pNode );
}

LEVELNODE	*GetWallLevelNodeOfSameOrientationAtGridNo( INT32 sGridNo, INT8 ubOrientation )
{
	LEVELNODE *pNode = NULL;
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_WALLSTUFF );

	while ( pStructure != NULL )
	{
		// Check orientation
		if ( pStructure->ubWallOrientation == ubOrientation )
		{
			pNode = FindLevelNodeBasedOnStructure( sGridNo, pStructure );
			return( pNode );
		}
		pStructure = FindNextStructure( pStructure, STRUCTURE_WALLSTUFF );
	}

	return( NULL );
}


LEVELNODE	*GetWallLevelNodeAndStructOfSameOrientationAtGridNo( INT32 sGridNo, INT8 ubOrientation, STRUCTURE **ppStructure )
{
	LEVELNODE *pNode = NULL;
	STRUCTURE * pStructure, * pBaseStructure;

	(*ppStructure) = NULL;

	pStructure = FindStructure( sGridNo, STRUCTURE_WALLSTUFF );

	while ( pStructure != NULL )
	{
		// Check orientation
		if ( pStructure->ubWallOrientation == ubOrientation )
		{
			pBaseStructure = FindBaseStructure( pStructure );
			if (pBaseStructure)
			{
				pNode = FindLevelNodeBasedOnStructure( pBaseStructure->sGridNo, pBaseStructure );
				(*ppStructure) = pBaseStructure;
				return( pNode );
			}
		}
		pStructure = FindNextStructure( pStructure, STRUCTURE_WALLSTUFF );
	}

	return( NULL );
}


BOOLEAN IsDoorVisibleAtGridNo( INT32 sGridNo )
{
	STRUCTURE * pStructure;
	INT32 sNewGridNo;

	pStructure = FindStructure( sGridNo, STRUCTURE_ANYDOOR );

	if ( pStructure != NULL )
	{
		// Check around based on orientation
		switch( pStructure->ubWallOrientation )
		{
			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:

				// Here, check north direction
				sNewGridNo = NewGridNo( sGridNo, DirectionInc( NORTH ) );

				if ( IsRoofVisible2( sNewGridNo ) )
				{
					// OK, now check south, if true, she's not visible
					sNewGridNo = NewGridNo( sGridNo, DirectionInc( SOUTH ) );

					if ( IsRoofVisible2( sNewGridNo ) )
					{
						return( FALSE );
					}
				}
				break;

			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:

				// Here, check west direction
				sNewGridNo = NewGridNo( sGridNo, DirectionInc( WEST ) );

				if ( IsRoofVisible2( sNewGridNo ) )
				{
					// OK, now check south, if true, she's not visible
					sNewGridNo = NewGridNo( sGridNo, DirectionInc( EAST ) );

					if ( IsRoofVisible2( sNewGridNo ) )
					{
						return( FALSE );
					}
				}
				break;

		}

	}

	// Return true here, even if she does not exist
	return( TRUE );
}


BOOLEAN DoesGridNoContainHiddenStruct( INT32 sGridNo, BOOLEAN *pfVisible )
{
	// ATE: These are ignored now - always return false

	//STRUCTURE *pStructure;

	//pStructure = FindStructure( sGridNo, STRUCTURE_HIDDEN );

	//if ( pStructure != NULL )
	//{
	//	if ( !(gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) && !(gTacticalStatus.uiFlags&SHOW_ALL_MERCS)	)
	//	{
	//		*pfVisible = FALSE;
	//	}
	//	else
	//	{
	//		*pfVisible = TRUE;
	//	}//
//
	//	return( TRUE );
	//}

	return( FALSE );
}


BOOLEAN IsHiddenStructureVisible( INT32 sGridNo, UINT16 usIndex )
{
	// Check if it's a hidden struct and we have not revealed anything!
	if ( gTileDatabase[ usIndex ].uiFlags & HIDDEN_TILE )
	{
		if ( !(gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) && !(gTacticalStatus.uiFlags&SHOW_ALL_MERCS) )
		{
			// Return false
			return( FALSE );
		}
	}

	return( TRUE );
}


BOOLEAN	WallExistsOfTopLeftOrientation( INT32 sGridNo )
{
	// CJC: changing to search only for normal walls, July 16, 1998
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_WALL );

	while ( pStructure != NULL )
	{
		// Check orientation
		if ( pStructure->ubWallOrientation == INSIDE_TOP_LEFT || pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT )
		{
			return( TRUE );
		}

		pStructure = FindNextStructure( pStructure, STRUCTURE_WALL );

	}

	return( FALSE );
}

BOOLEAN	WallExistsOfTopRightOrientation( INT32 sGridNo )
{
	// CJC: changing to search only for normal walls, July 16, 1998
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_WALL );

	while ( pStructure != NULL )
	{
		// Check orientation
		if ( pStructure->ubWallOrientation == INSIDE_TOP_RIGHT || pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT )
		{
			return( TRUE );
		}

		pStructure = FindNextStructure( pStructure, STRUCTURE_WALL );

	}

	return( FALSE );
}

BOOLEAN WallOrClosedDoorExistsOfTopLeftOrientation( INT32 sGridNo )
{
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_WALLSTUFF );

	while ( pStructure != NULL )
	{
		// skip it if it's an open door
		if ( ! ( ( pStructure->fFlags & STRUCTURE_ANYDOOR ) && ( pStructure->fFlags & STRUCTURE_OPEN ) ) )
		{
			// Check orientation
			if ( pStructure->ubWallOrientation == INSIDE_TOP_LEFT || pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT )
			{
				return( TRUE );
			}
		}

		pStructure = FindNextStructure( pStructure, STRUCTURE_WALLSTUFF );

	}

	return( FALSE );
}

BOOLEAN WallOrClosedDoorExistsOfTopRightOrientation( INT32 sGridNo )
{
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_WALLSTUFF );

	while ( pStructure != NULL )
	{
		// skip it if it's an open door
		if ( ! ( ( pStructure->fFlags & STRUCTURE_ANYDOOR ) && ( pStructure->fFlags & STRUCTURE_OPEN ) ) )
		{
			// Check orientation
			if ( pStructure->ubWallOrientation == INSIDE_TOP_RIGHT || pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT )
			{
				return( TRUE );
			}
		}

		pStructure = FindNextStructure( pStructure, STRUCTURE_WALLSTUFF );

	}

	return( FALSE );
}

BOOLEAN OpenRightOrientedDoorWithDoorOnRightOfEdgeExists( INT32 sGridNo )
{
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_ANYDOOR );

	while ( pStructure != NULL && (pStructure->fFlags & STRUCTURE_OPEN) )
	{
		// Check orientation
		if ( pStructure->ubWallOrientation == INSIDE_TOP_RIGHT || pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT )
		{
			if ( (pStructure->fFlags & STRUCTURE_DOOR) || (pStructure->fFlags & STRUCTURE_DDOOR_RIGHT) )
			{
				return( TRUE );
			}
		}

		pStructure = FindNextStructure( pStructure, STRUCTURE_ANYDOOR );

	}

	return( FALSE );
}

BOOLEAN OpenLeftOrientedDoorWithDoorOnLeftOfEdgeExists( INT32 sGridNo )
{
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_ANYDOOR );

	while ( pStructure != NULL && (pStructure->fFlags & STRUCTURE_OPEN) )
	{
		// Check orientation
		if ( pStructure->ubWallOrientation == INSIDE_TOP_LEFT || pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT )
		{
			if ( (pStructure->fFlags & STRUCTURE_DOOR) || (pStructure->fFlags & STRUCTURE_DDOOR_LEFT) )
			{
				return( TRUE );
			}
		}

		pStructure = FindNextStructure( pStructure, STRUCTURE_ANYDOOR );

	}

	return( FALSE );
}

STRUCTURE	* FindCuttableWireFenceAtGridNo( INT32 sGridNo )
{
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_WIREFENCE );
	if (pStructure != NULL && pStructure->ubWallOrientation != NO_ORIENTATION && !(pStructure->fFlags & STRUCTURE_OPEN) )
	{
		return( pStructure );
	}
	return( NULL );
}

BOOLEAN CutWireFence( INT32 sGridNo )
{
	STRUCTURE * pStructure;

	pStructure = FindCuttableWireFenceAtGridNo( sGridNo );
	if (pStructure)
	{
		pStructure = SwapStructureForPartnerAndStoreChangeInMap( sGridNo, pStructure );
		if (pStructure)
		{
			RecompileLocalMovementCosts( sGridNo );
			SetRenderFlags( RENDER_FLAG_FULL );
			return( TRUE );
		}
	}
	return( FALSE );
}

BOOLEAN IsCuttableWireFenceAtGridNo( INT32 sGridNo )
{
	return( FindCuttableWireFenceAtGridNo( sGridNo ) != NULL );
}


BOOLEAN IsRepairableStructAtGridNo( INT32 sGridNo, UINT8 *pubID )
{
	UINT8 ubMerc;

	// OK, first look for a vehicle....
	ubMerc = WhoIsThere2( sGridNo, 0 );

	if ( pubID != NULL )
	{
		(*pubID) = ubMerc;
	}

	if ( ubMerc != NOBODY )
	{
		if ( MercPtrs[ ubMerc ]->flags.uiStatusFlags & SOLDIER_VEHICLE )
		{
			return( 2 );
		}
	}
	// Then for over a robot....

	// then for SAM site....
	if ( DoesSAMExistHere( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, sGridNo ) )
	{
		return( 3 );
	}


	return( FALSE );
}


BOOLEAN IsRefuelableStructAtGridNo( INT32 sGridNo, UINT8 *pubID )
{
	UINT8 ubMerc;

	// OK, first look for a vehicle....
	ubMerc = WhoIsThere2( sGridNo, 0 );

	if ( pubID != NULL )
	{
		(*pubID) = ubMerc;
	}

	if ( ubMerc != NOBODY )
	{
		if ( MercPtrs[ ubMerc ]->flags.uiStatusFlags & SOLDIER_VEHICLE )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}


// Flugente: determine wether a fortification can be built on this position
BOOLEAN IsFortificationPossibleAtGridNo( INT32 sGridNo )
{
	INT8 bOverTerrainType = GetTerrainType( sGridNo );
	if( bOverTerrainType == MED_WATER || bOverTerrainType == DEEP_WATER || bOverTerrainType == LOW_WATER )
		return FALSE;

	STRUCTURE * pStructure = NULL;

	pStructure = FindStructure( sGridNo, (STRUCTURE_OBSTACLE|STRUCTURE_PERSON) );

	return( pStructure == NULL );
}


BOOLEAN IsCutWireFenceAtGridNo( INT32 sGridNo )
{
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_WIREFENCE );
	if (pStructure != NULL && (pStructure->ubWallOrientation != NO_ORIENTATION) && (pStructure->fFlags & STRUCTURE_OPEN) )
	{
		return( TRUE );
	}
	return( FALSE );
}

BOOLEAN FindConcertina(INT32 sSpot)
{
	if (TileIsOutOfBounds(sSpot))
		return FALSE;

	STRUCTURE * pStruct = FindStructure(sSpot, (STRUCTURE_GENERIC));
	if (pStruct)
	{
		// if this is a multi-tile structure, be sure to use the base gridno
		if (!(pStruct->fFlags & STRUCTURE_BASE_TILE))
		{
			pStruct = FindBaseStructure(pStruct);
		}

		if (pStruct)
		{
			LEVELNODE* pNode = FindLevelNodeBasedOnStructure(pStruct->sGridNo, pStruct);

			if (pNode)
			{
				UINT32 uiTileType = 0;
				if (GetTileType(pNode->usIndex, &uiTileType))
				{
					std::string tilesestr = GetNameToTileSet(uiTileType);

					if (strcmp(tilesestr.c_str(), "spot_1.sti") == 0 && pStruct->pDBStructureRef->pDBStructure->usStructureNumber > 1)
					{
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

INT32 FindDoorAtGridNoOrAdjacent( INT32 sGridNo )
{
	STRUCTURE * pStructure;
	STRUCTURE * pBaseStructure;
	INT32				sTestGridNo;

	sTestGridNo = sGridNo;
	pStructure = FindStructure( sTestGridNo, STRUCTURE_ANYDOOR );
	if (pStructure)
	{
		pBaseStructure = FindBaseStructure( pStructure );
		return( pBaseStructure->sGridNo );
	}

	sTestGridNo = sGridNo + DirectionInc( NORTH );
	pStructure = FindStructure( sTestGridNo, STRUCTURE_ANYDOOR );
	if (pStructure)
	{
		pBaseStructure = FindBaseStructure( pStructure );
		return( pBaseStructure->sGridNo );
	}

	sTestGridNo = sGridNo + DirectionInc( WEST );
	pStructure = FindStructure( sTestGridNo, STRUCTURE_ANYDOOR );
	if (pStructure)
	{
		pBaseStructure = FindBaseStructure( pStructure );
		return( pBaseStructure->sGridNo );
	}

	return( NOWHERE );
}



BOOLEAN IsCorpseAtGridNo( INT32 sGridNo, UINT8 ubLevel )
{
	if ( GetCorpseAtGridNo( sGridNo , ubLevel ) != NULL )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


BOOLEAN SetOpenableStructureToClosed( INT32 sGridNo, UINT8 ubLevel )
{
	STRUCTURE *		pStructure;
	STRUCTURE *		pNewStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_OPENABLE );
	if ( !pStructure )
	{
		return( FALSE );
	}

	if ( pStructure->fFlags & STRUCTURE_OPEN )
	{
		pNewStructure = SwapStructureForPartner( sGridNo, pStructure );
		if ( pNewStructure != NULL)
		{
			RecompileLocalMovementCosts( sGridNo );
			SetRenderFlags( RENDER_FLAG_FULL );
		}
	}
	// else leave it as is!
	return( TRUE );
}

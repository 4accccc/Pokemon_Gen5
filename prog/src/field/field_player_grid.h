#ifndef __FIELD_PLAYER_GRID_H__
#define __FIELD_PLAYER_GRID_H__

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"

typedef enum
{
	PLAYER_ANIME_FLAG_STOP = 0,
	PLAYER_ANIME_FLAG_WALK,
	PLAYER_ANIME_FLAG_RUN,
	PLAYER_ANIME_FLAG_MAX,
}PLAYER_ANIME_FLAG;

//extern PC_ACTCONT * CreatePlayerActGrid( FIELD_SETUP*	gs, HEAPID heapID );
extern PC_ACTCONT * CreatePlayerActGrid( FIELD_MAIN_WORK *, HEAPID heapID );
extern void DeletePlayerActGrid( PC_ACTCONT* pcActCont );
extern void PlayerActGrid_Update(
	PC_ACTCONT *pcActCont, u16 dir, const VecFx32 *pos );
extern void PlayerActGrid_AnimeSet(
	PC_ACTCONT *pcActCont, int dir , PLAYER_ANIME_FLAG flag );
extern FLDMAPPER_GRIDINFODATA * PlayerActGrid_GridInfoGet(
		PC_ACTCONT *pcActCont );
extern void PlayerActGrid_ScaleSizeSet(
	PC_ACTCONT *pcActCont, fx16 sizeX, fx16 sizeY );

#endif //__FIELD_PLAYER_GRID_H__

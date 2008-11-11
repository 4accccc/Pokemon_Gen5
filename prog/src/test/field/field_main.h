#ifndef __FIELD_MAIN_H__
#define __FIELD_MAIN_H__

#include "gamesystem/gamesystem.h"
#include "field_g3d_mapper.h"
#include "field_player.h"			//PC_ACT_FUNC

#define FLD_COMM_PLAYER_MAX (4)
#define FLD_COMM_ACTOR_MAX (FLD_COMM_PLAYER_MAX-1)

enum
{
	DIR_UP = 0,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
	DIR_NOT,
	DIR_MAX4 = DIR_NOT,
};

typedef struct _FIELD_MAIN_WORK FIELD_MAIN_WORK;

extern void DEBUG_FldGridProc_Camera( FIELD_MAIN_WORK *fieldWork );

//field_sub_grid.c
extern void DEBUG_FldGridProc_Camera( FIELD_MAIN_WORK *fieldWork );
extern void DEBUG_FldGridProc_ScaleChange( FIELD_MAIN_WORK *fieldWork );
extern void DEBUG_FldGridProc_ScaleControl( FIELD_MAIN_WORK *fieldWork );

extern void	FieldBoot(GAMESYS_WORK * gsys, HEAPID heapID );
extern void	FieldEnd( void );
extern BOOL	FieldMain( GAMESYS_WORK * gsys );

extern void FieldMain_AddCommActor(
	FIELD_MAIN_WORK *fieldWork, const PLAYER_WORK *player );

#endif //__FIELD_MAIN_H__

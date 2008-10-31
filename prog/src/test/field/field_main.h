#ifndef __FIELD_MAIN_H__
#define __FIELD_MAIN_H__

#include "gamesystem/gamesystem.h"
#include "field_g3d_mapper.h"
#include "field_player.h"			//PC_ACT_FUNC

typedef struct _FIELD_MAIN_WORK FIELD_MAIN_WORK;

extern void DEBUG_FldGridProc_Camera( FIELD_MAIN_WORK *fieldWork );

//field_sub_grid.c
extern void DEBUG_FldGridProc_Camera( FIELD_MAIN_WORK *fieldWork );
extern void DEBUG_FldGridProc_ScaleChange( FIELD_MAIN_WORK *fieldWork );
extern void DEBUG_FldGridProc_ScaleControl( FIELD_MAIN_WORK *fieldWork );

extern void	FieldBoot(GAMESYS_WORK * gsys, HEAPID heapID );
extern void	FieldEnd( void );
extern BOOL	FieldMain( GAMESYS_WORK * gsys );

#endif //__FIELD_MAIN_H__

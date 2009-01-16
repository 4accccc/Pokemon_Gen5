#ifndef __FIELD_MAIN_H__
#define __FIELD_MAIN_H__

#include "gamesystem/gamesystem.h"
#include "field_g3d_mapper.h"
#include "field_player.h"			//PC_ACT_FUNC

#include "field/fieldmap.h"
#include "field/field_msgbg.h"

#include "field_camera.h"

#define FLD_COMM_PLAYER_MAX (4)
#define FLD_COMM_ACTOR_MAX (FLD_COMM_PLAYER_MAX-1)

typedef enum
{
	DIR_UP = 0,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
	DIR_LEFTUP,
	DIR_RIGHTUP,
	DIR_LEFTDOWN,
	DIR_RIGHTDOWN,
	DIR_MAXALL,
	DIR_NOT,
	DIR_MAX4 = DIR_LEFTUP,
}FLDDIR;

extern void DEBUG_FldGridProc_Camera( FIELD_MAIN_WORK *fieldWork );
extern FLDMSGBG * FIELDMAP_GetFLDMSGBG( FIELD_MAIN_WORK *fieldWork );
extern FIELD_CAMERA * FIELDMAP_GetFieldCamera( FIELD_MAIN_WORK *fieldWork );

//field_sub_grid.c
extern void DEBUG_FldGridProc_Camera( FIELD_MAIN_WORK *fieldWork );
extern void DEBUG_FldGridProc_ScaleChange( FIELD_MAIN_WORK *fieldWork );
extern void DEBUG_FldGridProc_ScaleControl( FIELD_MAIN_WORK *fieldWork );

extern void FieldMain_AddCommActor(
	FIELD_MAIN_WORK *fieldWork, const PLAYER_WORK *player );
extern void* FieldMain_GetCommSys( const FIELD_MAIN_WORK *fieldWork);
extern const BOOL FieldMain_IsFieldUpdate( const FIELD_MAIN_WORK *fieldWork );
extern void FieldMain_UpdateFieldFunc( FIELD_MAIN_WORK *fieldWork );

extern u16 FieldMainGrid_GetPlayerDir( const FIELD_MAIN_WORK *fieldWork );

#endif //__FIELD_MAIN_H__

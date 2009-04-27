//======================================================================
/**
 * @file	fieldmap.h
 * @brief フィールドマップ
 * @author
 * @data
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "field/field_msgbg.h"

#include "field_g3d_mapper.h"
#include "map_matrix.h"

#include "field/fieldmap_proc.h"

#include "field_player.h"
#include "field_camera.h"
#include "field_light.h"
#include "field_fog.h"
#include "weather.h"

#include "field_debug.h"

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
///	フィールドマップ　コントロールタイプ
//--------------------------------------------------------------
typedef enum
{
	FLDMAP_CTRLTYPE_GRID = 0, ///<グリッドタイプ
	FLDMAP_CTRLTYPE_NOGRID, ///<非グリッドタイプ
	FLDMAP_CTRLTYPE_MAX,
}FLDMAP_CTRLTYPE;

//消します
#define FLD_COMM_PLAYER_MAX (4)
#define FLD_COMM_ACTOR_MAX (FLD_COMM_PLAYER_MAX-1)

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	DEPEND_FUNCTIONS
//--------------------------------------------------------------
typedef struct
{
	FLDMAP_CTRLTYPE type;
	void (*create_func)(FIELDMAP_WORK*, VecFx32*, u16);
	void (*main_func)(FIELDMAP_WORK*, VecFx32*);
	void (*delete_func)(FIELDMAP_WORK*);
}DEPEND_FUNCTIONS;

//======================================================================
//	extern
//======================================================================
//FIELDMAP_WORK 設定、参照
extern void * FieldMain_GetCommSys( const FIELDMAP_WORK *fieldWork );
extern FLDMSGBG * FIELDMAP_GetFldMsgBG( FIELDMAP_WORK *fieldWork );
extern FIELD_CAMERA * FIELDMAP_GetFieldCamera( FIELDMAP_WORK *fieldWork );
extern FIELD_LIGHT * FIELDMAP_GetFieldLight( FIELDMAP_WORK *fieldWork );
extern FIELD_FOG_WORK * FIELDMAP_GetFieldFog( FIELDMAP_WORK *fieldWork );
extern FIELD_WEATHER * FIELDMAP_GetFieldWeather( FIELDMAP_WORK *fieldWork );
extern FLDMMDLSYS * FIELDMAP_GetFldMMdlSys( FIELDMAP_WORK *fieldWork );
extern FIELD_DEBUG_WORK * FIELDMAP_GetDebugWork( FIELDMAP_WORK *fieldWork );
extern GAMESYS_WORK * FIELDMAP_GetGameSysWork( FIELDMAP_WORK *fieldWork );
extern MAP_MATRIX * FIELDMAP_GetMapMatrix( FIELDMAP_WORK *fieldWork );
extern HEAPID FIELDMAP_GetHeapID( FIELDMAP_WORK *fieldWork );
extern GFL_BBDACT_SYS * FIELDMAP_GetBbdActSys( FIELDMAP_WORK *fieldWork );
extern FLDMAPPER * FIELDMAP_GetFieldG3Dmapper( FIELDMAP_WORK *fieldWork );
extern void * FIELDMAP_GetMapCtrlWork( FIELDMAP_WORK *fieldWork );
extern void FIELDMAP_SetMapCtrlWork(FIELDMAP_WORK *fieldWork,void *ctrlWork);
extern int FIELDMAP_GetKeyCont( const FIELDMAP_WORK *fieldWork );
extern int FIELDMAP_GetKeyTrg( const FIELDMAP_WORK *fieldWork );
extern FIELD_PLAYER * FIELDMAP_GetFieldPlayer( FIELDMAP_WORK *fieldWork );

//---------------------------------------------------------------
//	いずれ消します
//---------------------------------------------------------------
extern void DEBUG_FldGridProc_Camera( FIELDMAP_WORK *fieldWork );
//field_sub_grid.c
extern void DEBUG_FldGridProc_Camera( FIELDMAP_WORK *fieldWork );
extern void DEBUG_FldGridProc_ScaleChange( FIELDMAP_WORK *fieldWork );
extern void DEBUG_FldGridProc_ScaleControl( FIELDMAP_WORK *fieldWork );
extern void FieldMain_AddCommActor(
	FIELDMAP_WORK *fieldWork, const PLAYER_WORK *player );
extern void* FieldMain_GetCommSys( const FIELDMAP_WORK *fieldWork);
extern const BOOL FieldMain_IsFieldUpdate( const FIELDMAP_WORK *fieldWork );
extern void FieldMain_UpdateFieldFunc( FIELDMAP_WORK *fieldWork );
extern u16 FieldMainGrid_GetPlayerDir( const FIELDMAP_WORK *fieldWork );
extern BOOL FIELDMAP_CheckGridControl( FIELDMAP_WORK *fieldWork );

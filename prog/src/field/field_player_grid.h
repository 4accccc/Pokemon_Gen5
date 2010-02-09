//======================================================================
/**
 * @file	field_player_grid.c
 * @brief グリッド専用 フィールドプレイヤー制御
 * @author kagaya
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "field_player.h"
#include "field_player_core.h"

#include "field_effect.h"

//======================================================================
//	define
//======================================================================
#if 0 //FIELD_PLAYERへ移動
//--------------------------------------------------------------
/// FIELD_PLAYER_GRID_REQBIT
//--------------------------------------------------------------
typedef enum
{
  ///自機を２速歩行に
  FIELD_PLAYER_GRID_REQBIT_NORMAL = (1<<0),
  ///自機を自転車に
  FIELD_PLAYER_GRID_REQBIT_CYCLE  = (1<<1),
  ///自機を波乗り状態に
  FIELD_PLAYER_GRID_REQBIT_SWIM = (1<<2),
}FIELD_PLAYER_GRID_REQBIT;

#define FIELD_PLAYER_GRID_REQBIT_MAX (3)
#endif

//======================================================================
//	struct
//======================================================================
///FIELD_PLAYER_GRID
typedef struct _TAG_FIELD_PLAYER_GRID FIELD_PLAYER_GRID;

//======================================================================
//	extern
//======================================================================

//======================================================================
//	FIELD_PLAYERからFIELD_PLAYER_GRIDを取得
//======================================================================
extern FIELD_PLAYER_GRID* FIELD_PLAYER_GetGridWk( FIELD_PLAYER* fld_player );


//======================================================================
//	FIELD_PLAYER内部で使用する関数
//======================================================================
// ----FIELD_PLAYER_GRIDの生成・破棄・更新
extern FIELD_PLAYER_GRID * FIELD_PLAYER_GRID_Init(
		FIELD_PLAYER_CORE *player_core, HEAPID heapID );
extern void FIELD_PLAYER_GRID_Delete( FIELD_PLAYER_GRID *g_jiki );

extern void FIELD_PLAYER_GRID_Move(
		FIELD_PLAYER_GRID *g_jiki, int key_trg, int key_cont );

extern PLAYER_MOVE_VALUE FIELD_PLAYER_GRID_GetMoveValue(
    FIELD_PLAYER_GRID *gjiki, u16 dir );

/* FIELD_PLAYER_COREへ
extern void FIELD_PLAYER_GRID_SetRequest(
  FIELD_PLAYER_GRID *gjiki, u32 req_bit );
extern void FIELD_PLAYER_GRID_UpdateRequest( FIELD_PLAYER_GRID *gjiki );
*/

extern BOOL FIELD_PLAYER_GRID_CheckStartMove(
    FIELD_PLAYER_GRID *gjiki, u16 dir );

extern void FIELD_PLAYER_GRID_ForceStop( FIELD_PLAYER_GRID *gjiki );
extern BOOL FIELD_PLAYER_GRID_SetMoveStop( FIELD_PLAYER_GRID *gjiki );

/* FIELD_PLAYER_COREへ
extern void FIELD_PLAYER_GRID_SetEffectTaskWork(
    FIELD_PLAYER_GRID *gjiki, FLDEFF_TASK *task );
extern FLDEFF_TASK * FIELD_PLAYER_GRID_GetEffectTaskWork(
    FIELD_PLAYER_GRID *gjiki );
*/
/* FIELD_PLAYERへ
extern void FIELD_PLAYER_SetNaminori( FIELD_PLAYER_GRID *gjiki );
extern void FIELD_PLAYER_SetNaminoriEnd( FIELD_PLAYER_GRID *gjiki );
*/
/* FIELD_PLAYER_COREへ
extern u16 FIELD_PLAYER_GRID_GetKeyDir( FIELD_PLAYER_GRID *gjiki, int key );
*/

extern BOOL FIELD_PLAYER_GRID_CheckUnderForceMove( FIELD_PLAYER_GRID *gjiki );
extern BOOL FIELD_PLAYER_GRID_CheckOzeFallOut( FIELD_PLAYER_GRID *gjiki );
extern FIELD_PLAYER_CORE * FIELD_PLAYER_GRID_GetFieldPlayerCore(
    FIELD_PLAYER_GRID *gjiki );
extern MMDL * FIELD_PLAYER_GRID_GetMMdl( FIELD_PLAYER_GRID *gjiki );

extern void FIELD_PLAYER_GRID_CheckSpecialDrawForm( FIELD_PLAYER_GRID *gjiki, BOOL menu_open_flag );



//======================================================================
//	Gridでのみ動作する関数
//======================================================================
extern GMEVENT * FIELD_PLAYER_GRID_CheckMoveEvent( FIELD_PLAYER_GRID *fld_player,
    int key_trg, int key_cont, PLAYER_EVENTBIT evbit );

extern GFL_TCB * FIELD_PLAYER_GRID_SetEventNaminoriStart(
    FIELD_PLAYER *fld_player, u16 dir, MAPATTR attr, HEAPID heapID );
extern BOOL FIELD_PLAYER_GRID_CheckEventNaminoriStart( GFL_TCB *tcb );
extern void FIELD_PLAYER_GRID_DeleteEventNaminoriStart( GFL_TCB *tcb );
extern GFL_TCB * FIELD_PLAYER_GRID_SetEventTakinobori(
    FIELD_PLAYER *fld_player, u16 dir, u16 no, HEAPID heapID );
extern BOOL FIELD_PLAYER_GRID_CheckEventTakinobori( GFL_TCB *tcb );
extern void FIELD_PLAYER_GRID_DeleteEventTakinobori( GFL_TCB *tcb );

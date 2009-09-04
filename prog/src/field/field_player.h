//======================================================================
/**
 * @file	field_player.h
 * @date	2008.9.29
 * @brief	フィールドプレイヤー制御
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap_proc.h"
#include "fldmmdl.h"
#include "field_g3d_mapper.h"

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
/// PLAYER_MOVE_VALUE
//--------------------------------------------------------------
typedef enum
{
  PLAYER_MOVE_VALUE_STOP, ///<停止状態
  PLAYER_MOVE_VALUE_WALK, ///<移動中
  PLAYER_MOVE_VALUE_TURN, ///<振り向き中
}PLAYER_MOVE_VALUE;

//--------------------------------------------------------------
/// PLAYER_MOVE_STATE
//--------------------------------------------------------------
typedef enum
{
  PLAYER_MOVE_STATE_OFF,    ///<動作なし
  PLAYER_MOVE_STATE_START,  ///<動作開始
  PLAYER_MOVE_STATE_ON,     ///<動作中
  PLAYER_MOVE_STATE_END,    ///<動作終了
}PLAYER_MOVE_STATE;

#if 0 //PLAYER_WORKへ移動
//--------------------------------------------------------------
/// PLAYER_MOVE_FORM
//--------------------------------------------------------------
typedef enum
{
  PLAYER_MOVE_FORM_NORMAL = 0,   ///<自機二足歩行
  PLAYER_MOVE_FORM_CYCLE,  ///<自機自転車移動
  PLAYER_MOVE_FORM_SWIM, ///<自機波乗り移動
  PLAYER_MOVE_FORM_MAX,
}PLAYER_MOVE_FORM;
#endif

//--------------------------------------------------------------
/// PLAYER_DRAW_FORM
//--------------------------------------------------------------
typedef enum
{
  PLAYER_DRAW_FORM_NORMAL = 0, ///<自機二足歩行
  PLAYER_DRAW_FORM_CYCLE, ///<自機自転車移動
  PLAYER_DRAW_FORM_SWIM, ///<自機波乗り移動
  PLAYER_DRAW_FORM_MAX,
}PLAYER_DRAW_FORM;

//======================================================================
//	struct
//======================================================================
///FIELD_PLAYER
typedef struct _FIELD_PLAYER FIELD_PLAYER;

//======================================================================
//	extern
//======================================================================
//作成、削除、更新
extern FIELD_PLAYER * FIELD_PLAYER_Create(
    PLAYER_WORK *playerWork, FIELDMAP_WORK *fieldWork,
		const VecFx32 *pos, int sex, HEAPID heapID );
extern void FIELD_PLAYER_Delete( FIELD_PLAYER *fld_player );
extern void FIELD_PLAYER_Update( FIELD_PLAYER *fld_player );
extern void FIELD_PLAYER_UpdateMoveStatus( FIELD_PLAYER *fld_player );

//参照、設定
extern void FIELD_PLAYER_GetPos(
		const FIELD_PLAYER *fld_player, VecFx32 *pos );
extern void FIELD_PLAYER_SetPos(
		FIELD_PLAYER *fld_player, const VecFx32 *pos );
extern u16 FIELD_PLAYER_GetDir( const FIELD_PLAYER *fld_player );
extern void FIELD_PLAYER_SetDir( FIELD_PLAYER *fld_player, u16 dir );
extern FIELDMAP_WORK * FIELD_PLAYER_GetFieldMapWork(
		FIELD_PLAYER *fld_player );
extern MMDL * FIELD_PLAYER_GetMMdl( FIELD_PLAYER *fld_player );
extern FLDMAPPER_GRIDINFODATA * FIELD_PLAYER_GetGridInfoData(
		FIELD_PLAYER *fld_player );
extern void FIELD_PLAYER_SetMoveValue(
    FIELD_PLAYER *fld_player, PLAYER_MOVE_VALUE val );
extern PLAYER_MOVE_VALUE FIELD_PLAYER_GetMoveValue(
    const FIELD_PLAYER *fld_player );
extern PLAYER_MOVE_STATE FIELD_PLAYER_GetMoveState(
    const FIELD_PLAYER *fld_player );
extern PLAYER_MOVE_FORM FIELD_PLAYER_GetMoveForm(
    const FIELD_PLAYER *fld_player );
extern void FIELD_PLAYER_SetMoveForm(
    FIELD_PLAYER *fld_player, PLAYER_MOVE_FORM form );
extern int FIELD_PLAYER_GetSex( const FIELD_PLAYER *fld_player );

//性別、OBJコード、各フォーム
extern u16 FIELD_PLAYER_GetDrawFormToOBJCode(
    int sex, PLAYER_DRAW_FORM form );
extern PLAYER_MOVE_FORM FIELD_PLAYER_GetOBJCodeToMoveForm(
    int sex, u16 code );
extern u16 FIELD_PLAYER_GetMoveFormToOBJCode(
    int sex, PLAYER_MOVE_FORM form );
extern PLAYER_DRAW_FORM FIELD_PLAYER_GetOBJCodeToDrawForm(
    int sex, u16 code );

//ツール
extern void FIELD_PLAYER_GetDirGridPos(
		FIELD_PLAYER *fld_player, u16 dir, s16 *gx, s16 *gy, s16 *gz );
extern void FIELD_PLAYER_GetDirPos(
		FIELD_PLAYER *fld_player, u16 dir, VecFx32 *pos );
extern void FIELD_PLAYER_GetFrontGridPos(
		FIELD_PLAYER *fld_player, s16 *gx, s16 *gy, s16 *gz );
extern BOOL FIELD_PLAYER_CheckLiveMMdl( FIELD_PLAYER *fld_player );
extern void FIELD_PLAYER_ChangeMoveForm(
    FIELD_PLAYER *fld_player, PLAYER_MOVE_FORM form );

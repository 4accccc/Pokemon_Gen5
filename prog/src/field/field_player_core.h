//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_player_core.h
 *	@brief  フィールドプレイヤー制御　コア情報
 *	@author	gamefreak
 *	@date		2010.02.05
 *
 *	モジュール名：FIELD_PLAYER_CORE
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "field_player.h"
#include "field_effect.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	FIELD_PLAYER_CORE
//=====================================
typedef struct _FIELD_PLAYER_CORE FIELD_PLAYER_CORE;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// Playerからcoreを取得
extern FIELD_PLAYER_CORE * FIELD_PLAYER_GetCoreWk( FIELD_PLAYER* p_player );

// FIELD_PLAYER_CORE 生成・破棄・更新
extern FIELD_PLAYER_CORE * FIELD_PLAYER_CORE_Create(
    PLAYER_WORK *playerWork, FIELDMAP_WORK *fieldWork,
		const VecFx32 *pos, int sex, HEAPID heapID );
extern void FIELD_PLAYER_CORE_Delete( FIELD_PLAYER_CORE *player_core );
extern void FIELD_PLAYER_CORE_Update( FIELD_PLAYER_CORE *player_core );
extern void FIELD_PLAYER_CORE_UpdateMoveStatus( FIELD_PLAYER_CORE *player_core );


// MMDL座標・方向・見た目操作
extern void FIELD_PLAYER_CORE_GetPos(
		const FIELD_PLAYER_CORE *player_core, VecFx32 *pos );
extern void FIELD_PLAYER_CORE_SetPos(
		FIELD_PLAYER_CORE *player_core, const VecFx32 *pos );
extern u16 FIELD_PLAYER_CORE_GetDir( const FIELD_PLAYER_CORE *player_core );
extern void FIELD_PLAYER_CORE_SetDir( FIELD_PLAYER_CORE *player_core, u16 dir );
extern void FIELD_PLAYER_CORE_ChangeOBJCode( FIELD_PLAYER_CORE *player_core, u16 code );
extern void FIELD_PLAYER_CORE_ClearOBJCodeFix( FIELD_PLAYER_CORE *player_core );

// Req関係
extern void FIELD_PLAYER_CORE_SetRequest(
  FIELD_PLAYER_CORE * player_core, FIELD_PLAYER_REQBIT req_bit );
extern void FIELD_PLAYER_CORE_UpdateRequest( FIELD_PLAYER_CORE * player_core );


// 情報の取得・設定
extern FIELDMAP_WORK * FIELD_PLAYER_CORE_GetFieldMapWork(
		FIELD_PLAYER_CORE *player_core );
extern MMDL * FIELD_PLAYER_CORE_GetMMdl( const FIELD_PLAYER_CORE *player_core );
extern void FIELD_PLAYER_CORE_SetMoveValue(
    FIELD_PLAYER_CORE *player_core, PLAYER_MOVE_VALUE val );
extern PLAYER_MOVE_VALUE FIELD_PLAYER_CORE_GetMoveValue(
    const FIELD_PLAYER_CORE *player_core );
extern void FIELD_PLAYER_CORE_SetMoveState(
    FIELD_PLAYER_CORE *player_core, PLAYER_MOVE_STATE val );
extern PLAYER_MOVE_STATE FIELD_PLAYER_CORE_GetMoveState(
    const FIELD_PLAYER_CORE *player_core );
extern PLAYER_MOVE_FORM FIELD_PLAYER_CORE_GetMoveForm(
    const FIELD_PLAYER_CORE *player_core );
extern void FIELD_PLAYER_CORE_SetMoveForm(
    FIELD_PLAYER_CORE *player_core, PLAYER_MOVE_FORM form );
extern int FIELD_PLAYER_CORE_GetSex( const FIELD_PLAYER_CORE *player_core );
extern GAMESYS_WORK * FIELD_PLAYER_CORE_GetGameSysWork( FIELD_PLAYER_CORE *player_core );
extern PLAYER_WORK * FIELD_PLAYER_CORE_GetPlayerWork( const FIELD_PLAYER_CORE * player_core );


// 情報のいろんな関係の関数
extern BOOL FIELD_PLAYER_CORE_CheckLiveMMdl( const FIELD_PLAYER_CORE *player_core );

extern void FIELD_PLAYER_CORE_ChangeMoveForm(
    FIELD_PLAYER_CORE *player_core, PLAYER_MOVE_FORM form );
extern void FIELD_PLAYER_CORE_ResetMoveForm( FIELD_PLAYER_CORE *player_core );
extern void FIELD_PLAYER_CORE_ChangeDrawForm(
    FIELD_PLAYER_CORE *player_core, PLAYER_DRAW_FORM form );
extern PLAYER_DRAW_FORM FIELD_PLAYER_CORE_GetDrawForm( FIELD_PLAYER_CORE *player_core );
extern void FIELD_PLAYER_CORE_ChangeFormRequest( FIELD_PLAYER_CORE *player_core, PLAYER_DRAW_FORM form );
extern BOOL FIELD_PLAYER_CORE_ChangeFormWait( FIELD_PLAYER_CORE *player_core );
extern BOOL FIELD_PLAYER_CORE_CheckIllegalOBJCode( FIELD_PLAYER_CORE *player_core );


// 自機オーダーチェック
extern BOOL FIELD_PLAYER_CORE_CheckStartMove(
    const FIELD_PLAYER_CORE * player_core, u16 dir );
// キー入力方向
extern u16 FIELD_PLAYER_CORE_GetKeyDir( const FIELD_PLAYER_CORE* player_core, int key );
extern void FIELD_PLAYER_CORE_SetMoveStartKeyDir( FIELD_PLAYER_CORE* player_core, int key ); // 動作開始時のキー方向を保存　InitMoveStartCommonで呼びます。


// EffectTask
extern void FIELD_PLAYER_CORE_SetEffectTaskWork(
    FIELD_PLAYER_CORE *player_core, FLDEFF_TASK *task );
extern FLDEFF_TASK * FIELD_PLAYER_CORE_GetEffectTaskWork(
    FIELD_PLAYER_CORE *player_core );

#ifdef _cplusplus
}	// extern "C"{
#endif




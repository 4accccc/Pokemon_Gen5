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
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap_proc.h"
#include "fldmmdl.h"
#include "field_g3d_mapper.h"
#include "map_attr.h"

#include "field_player_code.h"
#include "field_effect.h"

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
  PLAYER_DRAW_FORM_ITEMGET, ///<自機アイテムゲット
  PLAYER_DRAW_FORM_SAVEHERO, ///<自機セーブ
  PLAYER_DRAW_FORM_PCHERO, ///<自機PC預け
  PLAYER_DRAW_FORM_YURE, ///<自機ゆれ
  PLAYER_DRAW_FORM_FISHING, ///<自機釣り
  PLAYER_DRAW_FORM_CUTIN, ///<自機カットイン
  PLAYER_DRAW_FORM_MAX,
}PLAYER_DRAW_FORM;

//--------------------------------------------------------------
/// PLAYER_EVENTBIT
//--------------------------------------------------------------
typedef enum
{
  PLAYER_EVENTBIT_NON = 0, ///<イベント許可　なし
  PLAYER_EVENTBIT_KAIRIKI = (1<<0), ///<イベント許可　かいりき
  PLAYER_EVENTBIT_TAKINOBORI = (1<<1), ///<イベント許可 たきのぼり
}PLAYER_EVENTBIT;

//======================================================================
//	struct
//======================================================================
///FIELD_PLAYER
typedef struct _FIELD_PLAYER FIELD_PLAYER;
///FIELD_PLAYER_REQBIT
typedef u32 FIELD_PLAYER_REQBIT;

//======================================================================
//	extern
//======================================================================
//======================================================================
//	Grid Rail Hybrid共通処理
//======================================================================
//作成、削除、更新
extern FIELD_PLAYER * FIELD_PLAYER_Create(
    PLAYER_WORK *playerWork, FIELDMAP_WORK *fieldWork,
		const VecFx32 *pos, int sex, HEAPID heapID );
extern void FIELD_PLAYER_Delete( FIELD_PLAYER *fld_player );
extern void FIELD_PLAYER_Update( FIELD_PLAYER *fld_player );
extern void FIELD_PLAYER_UpdateMoveStatus( FIELD_PLAYER *fld_player );
extern GMEVENT * FIELD_PLAYER_CheckMoveEvent( FIELD_PLAYER *fld_player,
    int key_trg, int key_cont, PLAYER_EVENTBIT evbit );

// プレイヤー動作変更 REQBIT 
extern void FIELD_PLAYER_SetRequest(
  FIELD_PLAYER * fld_player, FIELD_PLAYER_REQBIT req_bit );
extern void FIELD_PLAYER_UpdateRequest( FIELD_PLAYER * fld_player );

// 自機オーダーチェック
extern BOOL FIELD_PLAYER_CheckStartMove(
    FIELD_PLAYER * fld_player, u16 dir );

// 強制停止
extern void FIELD_PLAYER_ForceStop( FIELD_PLAYER * fld_player );
extern void FIELD_PLAYER_SetMoveStop( FIELD_PLAYER * fld_player );

// EffectTask 波乗りのポケモンなど
extern void FIELD_PLAYER_SetEffectTaskWork(
    FIELD_PLAYER * fld_player, FLDEFF_TASK *task );
extern FLDEFF_TASK * FIELD_PLAYER_GetEffectTaskWork(
    FIELD_PLAYER * fld_player );

// 指定方向を自機に渡すとどうなるか
extern PLAYER_MOVE_VALUE FIELD_PLAYER_GetDirMoveValue(
    FIELD_PLAYER * fld_player, u16 dir );

// キー入力方向
extern u16 FIELD_PLAYER_GetKeyDir( const FIELD_PLAYER* fld_player, int key );

// 波乗り
extern void FIELD_PLAYER_SetNaminori( FIELD_PLAYER * fld_player );
extern void FIELD_PLAYER_SetNaminoriEnd( FIELD_PLAYER * fld_player );

//参照、設定
extern void FIELD_PLAYER_GetPos(
		const FIELD_PLAYER *fld_player, VecFx32 *pos );
extern void FIELD_PLAYER_SetPos(
		FIELD_PLAYER *fld_player, const VecFx32 *pos );
extern u16 FIELD_PLAYER_GetDir( const FIELD_PLAYER *fld_player );
extern void FIELD_PLAYER_SetDir( FIELD_PLAYER *fld_player, u16 dir );

extern FIELDMAP_WORK * FIELD_PLAYER_GetFieldMapWork(
		FIELD_PLAYER *fld_player );
extern MMDL * FIELD_PLAYER_GetMMdl( const FIELD_PLAYER *fld_player );
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
extern GAMESYS_WORK * FIELD_PLAYER_GetGameSysWork( FIELD_PLAYER *fld_player );

extern MAPATTR FIELD_PLAYER_GetMapAttr( const FIELD_PLAYER *fld_player );
extern MAPATTR FIELD_PLAYER_GetDirMapAttr( const FIELD_PLAYER *fld_player, u16 dir );

//性別、OBJコード、各フォーム
extern u16 FIELD_PLAYER_GetDrawFormToOBJCode(
    int sex, PLAYER_DRAW_FORM form );
extern PLAYER_MOVE_FORM FIELD_PLAYER_GetOBJCodeToMoveForm(
    int sex, u16 code );
extern u16 FIELD_PLAYER_GetMoveFormToOBJCode(
    int sex, PLAYER_MOVE_FORM form );
extern PLAYER_DRAW_FORM FIELD_PLAYER_GetOBJCodeToDrawForm(
    int sex, u16 code );
extern PLAYER_DRAW_FORM FIELD_PLAYER_CheckOBJCodeToDrawForm(
    u16 code );
extern BOOL FIELD_PLAYER_CheckChangeEventDrawForm( FIELD_PLAYER *fld_player );

//ツール
extern void FIELD_PLAYER_GetDirPos(
		const FIELD_PLAYER *fld_player, u16 dir, VecFx32 *pos );
extern void FIELD_PLAYER_GetDirWay( 
    const FIELD_PLAYER *fld_player, u16 dir, VecFx32* way );
extern BOOL FIELD_PLAYER_CheckLiveMMdl( const FIELD_PLAYER *fld_player );
extern void FIELD_PLAYER_ChangeMoveForm(
    FIELD_PLAYER *fld_player, PLAYER_MOVE_FORM form );
extern void FIELD_PLAYER_ResetMoveForm( FIELD_PLAYER *fld_player );
extern void FIELD_PLAYER_ChangeDrawForm(
    FIELD_PLAYER *fld_player, PLAYER_DRAW_FORM form );
extern PLAYER_DRAW_FORM FIELD_PLAYER_GetDrawForm( FIELD_PLAYER *fld_player );
extern void FIELD_PLAYER_ChangeFormRequest( FIELD_PLAYER *fld_player, PLAYER_DRAW_FORM form );
extern BOOL FIELD_PLAYER_ChangeFormWait( FIELD_PLAYER *fld_player );
extern BOOL FIELD_PLAYER_CheckAttrNaminori(
    FIELD_PLAYER *fld_player, MAPATTR nattr, MAPATTR fattr );
extern void FIELD_PLAYER_CheckSpecialDrawForm(
    FIELD_PLAYER *fld_player, BOOL menu_open_flag );
extern void FIELD_PLAYER_ChangeOBJCode( FIELD_PLAYER *fld_player, u16 code );
extern void FIELD_PLAYER_ClearOBJCodeFix( FIELD_PLAYER *fld_player );
extern BOOL FIELD_PLAYER_CheckIllegalOBJCode( FIELD_PLAYER *fld_player );

// モデル検索
extern MMDL * FIELD_PLAYER_GetFrontMMdl( const FIELD_PLAYER *fld_player );


//======================================================================
//	Grid 専用処理
//======================================================================
// GRIDワークのセットアップ
extern void FIELD_PLAYER_SetUpGrid( FIELD_PLAYER *fld_player, HEAPID heapID );
// GRIDワークの動作処理
extern void FIELD_PLAYER_MoveGrid( FIELD_PLAYER *fld_player, int key_trg, int key_cont );

//TOOL
extern void FIELD_PLAYER_GetDirGridPos(
		const FIELD_PLAYER *fld_player, u16 dir, s16 *gx, s16 *gy, s16 *gz );
extern void FIELD_PLAYER_GetFrontGridPos(
		const FIELD_PLAYER *fld_player, s16 *gx, s16 *gy, s16 *gz );




//======================================================================
//	NoGrid 専用処理
//======================================================================
// NOGRIDワークのセットアップ
extern void FIELD_PLAYER_SetUpNoGrid( FIELD_PLAYER *fld_player, HEAPID heapID );
// NOGRIDワークの動作処理
extern void FIELD_PLAYER_MoveNoGrid( FIELD_PLAYER *fld_player, int key_trg, int key_cont );

// NOGRID リスタート
extern void FIELD_PLAYER_RestartNoGrid( FIELD_PLAYER *fld_player, const RAIL_LOCATION* cp_pos );
// NOGRID　停止
extern void FIELD_PLAYER_StopNoGrid( FIELD_PLAYER *fld_player );

// RAILWKの取得
extern FIELD_RAIL_WORK* FIELD_PLAYER_GetNoGridRailWork( const FIELD_PLAYER *fld_player );

// Location
extern void FIELD_PLAYER_SetNoGridLocation( FIELD_PLAYER* p_player, const RAIL_LOCATION* cp_location );
extern void FIELD_PLAYER_GetNoGridLocation( const FIELD_PLAYER* cp_player, RAIL_LOCATION* p_location );

// RailPos
extern void FIELD_PLAYER_GetNoGridPos( const FIELD_PLAYER* cp_player, VecFx32* p_pos );


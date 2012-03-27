//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		infinity_dungeon_tool.h
 *	@brief  無限ダンジョン用ツール
 *	@author	Masayuki Onoue
 *	@date		2011.10.11
 *  @note   vsプロジェクトでもソースを流用し、テストを行うことを考慮し設計します
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

// VSでコンパイルする時に設定する
#ifdef _WIN32
// VSでコンパイルする時に設定する
#define VS_PROJECT
#endif

#ifndef VS_PROJECT
#include <gflib.h>
#else
#include "C:/home\pokemon_tool/inf_dungeon_test/inf_dungeon_test/inf_debug_gflib_override.h"
#endif

#include "../../../resource/infinity_dungeon/data/infinity_dungeon_tr_area_data.h" // for INF_TR_AREA_DATA_MAX
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_room_pos_type.h" // for INF_TR_ROOM_POS_TYPE_NONE
#include "../../../resource/infinity_dungeon/data/inf_floor_num.h" // for INF_AREA_ROOM_SET_MAX
#include "arc/fieldmap/fldmmdl_objcode.h"   // for OBJCODE
#include "infinity_dungeon_warp_dir.h"      // for WARP_DIR_*
#include "fieldmap/zone_id.h"               // for ZONE_ID_*
#include "infinity_dungeon_def.h"           // for INFINITY_DUNGEON_TRID_MAX
#include "pm_version.h"

//=============================================================================
//
//					定数宣言
//
//=============================================================================

// 部屋に出現するトレーナーの最大値
#define ROOM_TR_MAX (4)

// トレーナー出現タイプ
#define INF_MULTI_DUNGEON (0)   ///< 両方
#define INF_B2_ONLY_DUNGEON (1) ///< 黒のみ
#define INF_W2_ONLY_DUNGEON (2) ///< 白のみ

// 無限ダンジョンに登場するトレーナータイプ
#define INFINITY_DUNGEON_TALK_PATTERN_MAX (30)

// 部屋サイズ定義
#define ROOM_MAX_X            (4)                       ///< 部屋X最大数
#define ROOM_MAX_Y            (4)                       ///< 部屋Y最大数
#define ROOM_MAX              (ROOM_MAX_X*ROOM_MAX_Y)   ///< 部屋最大数

//-------------------------------------
//	ヒントタイプ
//=====================================
enum
{
  INFTR_HINT_TYPE_GATE_TR_DIR,          ///< A-1
  INFTR_HINT_TYPE_GATE_TR_UP_DOWN,      ///< A-2
  INFTR_HINT_TYPE_GATE_TR_ROOM_TR_NUM,  ///< A-3
  INFTR_HINT_TYPE_GATE_SEX,             ///< B-1
  INFTR_HINT_TYPE_GATE_MOVE_TYPE,       ///< B-2
  INFTR_HINT_TYPE_MEDIC_DIR,            ///< C
  INFTR_HINT_TYPE_BOSS_MONSTER,         ///< D
  INFTR_HINT_TYPE_BOSS_INFO,            ///< E1,E2
  INFTR_HINT_TYPE_NONE,
  INFTR_HINT_TYPE_MAX,
};

//-------------------------------------
//	会話タイプ
//=====================================
enum
{
  INFTR_TALK_TYPE_BOY,      ///< 少年
  INFTR_TALK_TYPE_GIRL,     ///< 少女
  INFTR_TALK_TYPE_YOUNG_M,  ///< 成人男性
  INFTR_TALK_TYPE_YOUNG_W,  ///< 成人女性
  INFTR_TALK_TYPE_MIDDLE_M, ///< 中年男性
  INFTR_TALK_TYPE_MIDDLE_W, ///< 中年女性
  INFTR_TALK_TYPE_OLD_M,    ///< 老人
  INFTR_TALK_TYPE_OLD_W,    ///< 老女
  INFTR_TALK_TYPE_MAX,
};

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

//-------------------------------------
///	MMDL用パラメータ
//=====================================
typedef struct {
  u16 move_code;  ///< 動作移動モデルコード
  u16 ev_type;    ///< イベントタイプ
  u16 direction;  ///< 方向
  u16 param0;     ///< パラメータ0
  u16 param1;     ///< パラメータ1
  s32 lim_x;      ///< x限界値
  s32 lim_z;      ///< y限界値
  u16 hint_msg_id;///< ヒントメッセージID
}INF_MMDL_PARAM;

//-------------------------------------
///	部屋データ
//=====================================
typedef struct {
  u16 move_type;  ///< 移動タイプ
  u32 g_x;        ///< 位置X
  u32 g_y;        ///< 位置Y
}INF_ROOM_TR_DATA;

//-------------------------------------
///	配置パラメータ
//=====================================
typedef struct {
  u8  version;      ///< バージョン
  u8  member_num;   ///< メンバー数
  u32 room_type;    ///< 部屋タイプ
  INF_ROOM_TR_DATA tr_move_data[ROOM_TR_MAX]; ///< 部屋トレーナーデータ
}INF_ROOM_SET_TR_PARAM;

//-------------------------------------
///	部屋データ
//=====================================
typedef struct {
  u32 tr_pos_type;    ///< 位置タイプ
//  u32 probability;
}INF_ROOM_DATA;

//-------------------------------------
///	トレーナーデータ
//=====================================
typedef struct {
  u8 start_area;  ///< 出現開始エリア
  u8 end_area;    ///< 出現終了エリア
  u8 version;     ///< バージョン制限
  u8 tr_type;     ///< トレーナータイプ
}INF_TR_AREA_DATA;

//-------------------------------------
// グリッド座標データ
//=====================================
typedef struct {
  s32 x;
  s32 y;
  s32 z;
}GRID_DATA;

//-------------------------------------
///	レベルデータ
//=====================================
typedef struct {
  u16 level_min;  ///< 最小レベル
  u16 level_max;  ///< 最大レベル
}INF_LEVEL_TABLE_PARAM;

//-------------------------------------
///	ルームデータ
//=====================================
typedef struct{
  u8  tr_pos_type;  ///< トレーナー位置タイプ
  u8 room_type;     ///< 部屋タイプ
  BOOL can_move[4]; ///< 左右上下に移動できるか
  u8  room_id;      ///< 部屋ID
}ROOM_DATA;

//-------------------------------------
///	ダンジョン内トレーナーデータ
//=====================================
typedef struct{
  u16 tr_id[INFINITY_DUNGEON_TRID_MAX]; ///<  全トレーナーランダム配列
}DUNGEON_TRAINER;

//-------------------------------------
///	無限ダンジョン用シード
//=====================================
typedef struct 
{
  s32 master_seed;        ///< シード
  s32 tr_data_seed;       ///< トレーナーデータシード
  s32 dungeon_data_seed;  ///< ダンジョンのシード
  s32 reward_item_seed;   ///< 報酬のシード
  u16 tr_seed[INFINITYDUNGEON_FLOOR_MAX][INFINITY_DUNGEON_TRID_MAX]; ///< トレーナーシード
}INF_DUNGEON_SEED;

//-------------------------------------
///	無限ダンジョンデータ
//=====================================
typedef struct {
  INF_DUNGEON_SEED  seed; ///< ダンジョンのシード
  ROOM_DATA         room_data[INFINITYDUNGEON_FLOOR_MAX][ROOM_MAX]; ///< 部屋データ
  DUNGEON_TRAINER   tr_data[INFINITYDUNGEON_FLOOR_MAX];             ///< トレーナーデータ
  u16               recover_data[INFINITY_DUNGEON_RECOVERTR_MAX];   ///< 回復トレーナーデータ
  u16               recover_room_id[INFINITYDUNGEON_FLOOR_MAX];     ///< 回復トレーナー部屋IDデータ
  u8                gate_trainer_floor;     ///< ゲートトレーナーフロア
  u8                gate_trainer_room_id;   ///< ゲートトレーナー部屋ID
  u8                gate_trainer_id_pos;    ///< ゲートトレーナー位置
  u16               reward_item_id;         ///< 報酬アイテムID
  u8                reward_item_num;        ///< 報酬アイテム数
  u8                version;                ///< バージョン
}INF_DUNGEON_DATA;

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern const INF_MMDL_PARAM* INF_TOOL_GetMMDLParam( u16 move_type );
extern const INF_ROOM_SET_TR_PARAM* INF_TOOL_GetRoomTrParam( u16 tr_pos_type );
extern u8 INF_TOOL_GetMemberNum( u8 pos_type );
extern u8 INF_TOOL_GetRoomType( u8 pos_type );
extern u8 INF_TOOL_GetRoomVersion( u8 pos_type );
extern const INF_TR_AREA_DATA* INF_TOOL_GetTrainerParam( u16 tr_index );
extern void INF_TOOL_GetFloorSize( u8 area_no ,s32 *fs_x ,s32 *fs_y );
extern const u16* INF_TOOL_GetZoneIdTable( u8 version );
extern const GRID_DATA* INF_TOOL_GetElevatorPos( u8 version );
extern const GRID_DATA* INF_TOOL_GetReceptionElevatorPos( int elevator_type );
extern const GRID_DATA* INF_TOOL_GetWarpPosTable( u8 version ,int room_type ,int warp_dir  );
extern const GRID_DATA* INF_TOOL_GetBmodelPos( u8 version ,int room_type ,int warp_dir  );
extern const INF_LEVEL_TABLE_PARAM* INF_TOOL_GetLevelData( u16 area_no );
extern u8 INF_TOOL_GetPowRnd( u16 area_no );
extern u8 INF_TOOL_GetTrainerHintType( const INF_DUNGEON_DATA *p_dungeon_data,u16 obj_code ,u8 floor_no, u8 id_pos );
extern u8 INF_TOOL_GetTrainerTalkType( u16 obj_code);
extern void INF_TOOL_RndArray( u32 *p_array , int size ,GFL_STD_RandContext *rand_context );
extern void INF_TOOL_RndArrayU8( u8 *p_array , int size ,GFL_STD_RandContext *rand_context );
extern void INF_TOOL_SetDungeonSeed( INF_DUNGEON_SEED *p_seed ,u32 master_seed );
extern const u8* INF_TOOL_GetFloorData( u16 area_no ,u16 floor_no ,u8 version );
extern const u8* INF_TOOL_GetTrTypeCntTable( u16 area_no ,u8 version);
extern u8 INF_TOOL_GetTrainerType( u16 tr_index );
extern BOOL INF_TOOL_DUNGEON_GetIsGateTrainer( const INF_DUNGEON_DATA *p_wk ,u8 floor_no, u8 id_pos );
extern u8 INF_TOOL_DUNGEON_GetDungeonColor( const INF_DUNGEON_DATA *p_wk );
extern int INF_TOOL_DUNGEON_GetRoomTrainerIDPos( const INF_DUNGEON_DATA *p_dungeon_data ,u32 floor_id ,u32 room_id,u8 room_tr_no);
extern u16 INF_TOOL_DUNGEON_GetTrIDByIDPos( const INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_id ,u8 id_pos );
extern void INF_TOOL_RndArrayU16( u16 *p_array , int size ,GFL_STD_RandContext *rand_context );
extern int INF_TOOL_DUNGEON_GetTrainerRoomID( const INF_DUNGEON_DATA *p_dungeon_data, int floor_id, int id_pos );


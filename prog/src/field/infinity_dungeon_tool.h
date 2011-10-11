//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file	  infinity_dungeon_tool.h
//	 @brief   無限ダンジョン用ツール
//   @author	Masayuki Onoue
//   @date		2011.10.11
//	 @note   vsプロジェクトでもソースを流用し、テストを行うことを考慮し設計します
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "../../../resource/infinity_dungeon/data/infinity_dungeon_room_setup_data.h" // for INF_AREA_ROOM_SET_MAX
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_tr_area_data.h" // for INF_TR_AREA_DATA_MAX
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_room_pos_type.h" // for INF_TR_ROOM_POS_TYPE_NONE
#include "arc/fieldmap/fldmmdl_objcode.h"   // for OBJCODE
#include "infinity_dungeon_warp_dir.h"      // for WARP_DIR_*
#include "fieldmap/zone_id.h"               // for ZONE_ID_*

//=============================================================================
//
//					定数宣言
//
//=============================================================================
#define ROOM_TR_MAX (4)
#define INF_MULTI_DUNGEON (0)
#define INF_B2_ONLY_DUNGEON (1)
#define INF_W2_ONLY_DUNGEON (2)
#define INFINITY_DUNGEON_TALK_PATTERN_MAX (30)    // 無限ダンジョンに登場するトレーナータイプ

//-------------------------------------
//	ヒントタイプ
//=====================================
enum
{
  INFTR_HINT_TYPE_UP,
  INFTR_HINT_TYPE_DOWN,
  INFTR_HINT_TYPE_BOSS_FLOOR,
  INFTR_HINT_TYPE_BOSS_DIR,
  INFTR_HINT_TYPE_BOSS_MONSTER,
  INFTR_HINT_TYPE_MEDIC_DIR,
  INFTR_HINT_TYPE_NONE,
  INFTR_HINT_TYPE_MAX,
};

//-------------------------------------
//	会話タイプ
//=====================================
enum
{
  INFTR_TALK_TYPE_BOY,      // 少年
  INFTR_TALK_TYPE_GIRL,     // 少女
  INFTR_TALK_TYPE_YOUNG_M,  // 成人男性
  INFTR_TALK_TYPE_YOUNG_W,  // 成人女性
  INFTR_TALK_TYPE_MIDDLE_M, // 中年男性
  INFTR_TALK_TYPE_MIDDLE_W, // 中年女性
  INFTR_TALK_TYPE_OLD_M,    // 老人
  INFTR_TALK_TYPE_OLD_W,    // 老女
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
  u16 move_code;
  u16 ev_type;
  u16 direction;
  u16 param0;
  u16 param1;
  s32 lim_x;
  s32 lim_z;
}INF_MMDL_PARAM;

//-------------------------------------
///	部屋データ
//=====================================
typedef struct {
  u16 move_type;
  u32 g_x;
  u32 g_y;
}INF_ROOM_TR_DATA;

//-------------------------------------
///	配置パラメータ
//=====================================
typedef struct {
  u8  version;
  u8  member_num;
  u32 room_type;
  INF_ROOM_TR_DATA tr_move_data[ROOM_TR_MAX];
}INF_ROOM_SET_TR_PARAM;

//-------------------------------------
///	部屋データ
//=====================================
typedef struct {
  u32 tr_pos_type;
  u32 probability;
}INF_ROOM_DATA;

//-------------------------------------
///	部屋設置用データ
//=====================================
typedef struct {
  u8  member_num;
  INF_ROOM_DATA room_setup_data[INF_AREA_ROOM_SET_MAX];
}INF_ROOM_SETUP_PARAM;

//-------------------------------------
///	トレーナーデータ
//=====================================
typedef struct {
  u8 start_area;
  u8 end_area;
  u8 version;
  u8 set_lv;
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
  u16 level_max;
  u16 level_min;
}INF_LEVEL_TABLE_PARAM;

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
extern const INF_ROOM_SETUP_PARAM* INF_TOOL_GetRoomSetupParam( u16 area_no );
extern const INF_TR_AREA_DATA* INF_TOOL_GetTrainerParam( u16 tr_index );
extern void INF_TOOL_GetFloorSize( u8 area_no ,s32 *fs_x ,s32 *fs_y );
extern const u16* INF_TOOL_GetZoneIdTable( u8 version );
extern const GRID_DATA* INF_TOOL_GetElevatorPos( u8 version );
extern const GRID_DATA* INF_TOOL_GetReceptionElevatorPos( int elevator_type );
extern const GRID_DATA* INF_TOOL_GetWarpPosTable( u8 version ,int room_type ,int warp_dir  );
extern const GRID_DATA* INF_TOOL_GetBmodelPos( u8 version ,int room_type ,int warp_dir  );
extern const INF_LEVEL_TABLE_PARAM* INF_TOOL_GetLevelData( u16 area_no );
extern u8 INF_TOOL_GetPowRnd( u16 area_no );
extern u8 INF_TOOL_GetTrainerHintType( u16 obj_code);
extern u8 INF_TOOL_GetTrainerTalkType( u16 obj_code);


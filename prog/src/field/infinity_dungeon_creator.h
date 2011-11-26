//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//  
//   @file		infinity_dungeon_creator.h
// 	 @brief   無限ダンジョン：ダンジョン生成用
//   @author	Masayuki Onoue
//   @date		2011.10.11
//	 @note   vsプロジェクトでもソースを流用し、テストを行うことを考慮し設計します
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "infinity_dungeon_tool.h" // for INF_DUNGEON_DATA

//=============================================================================
//
//					定数宣言
//
//=============================================================================

// 乱数生成時最高思考数
#define INF_MAX_SEARC_CNT             (100)          
// トレーナーが配置されている部屋数の際少数
#define INFINITY_DUNGEON_TR_ROOM_MIN  (5)

//-------------------------------------
//	部屋タイプ
//=====================================
enum{
  ROOM_TYPE_ELEVATOR,
  ROOM_TYPE_BOSS,
  ROOM_TYPE_SPECIAL1,
  ROOM_TYPE_SPECIAL2,
  ROOM_TYPE_DEFAULT,
  ROOM_TYPE_RECEPTION,
  ROOM_TYPE_MAX,
};

//-------------------------------------
//	ダンジョン移動用定数
//=====================================
enum{
  INF_MOVE_UP,
  INF_MOVE_DOWN,
  INF_MOVE_RIGHT,
  INF_MOVE_LEFT,
  INF_MOVE_MAX,
};

//-------------------------------------
//	部屋移動チェックモード
//=====================================
enum{
  INF_CHECKMODE_FULL_CLOSE, // 全ての扉がしまっているときのみ
  INF_CHECKMODE_MOVE,       // ボス部屋とエレベーター部屋のみをチェック
};

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

//-------------------------------------
///	部屋位置データ
//=====================================
typedef struct {
  int x;
  int y;
}DIG_POS;

//-------------------------------------
///	迷路生成用ワーク
//=====================================
typedef struct {
  int data_index;
  DIG_POS now_pos;
  DIG_POS dig_pos_list[INF_MAX_SEARC_CNT];
}INF_DIG_DATA;

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern void INF_DCREATOR_SetDungeonData( INF_DUNGEON_DATA *p_dungeon_data ,int area_no, u32 seed ,int fs_x ,int fs_y);
extern void INF_DCREATOR_SetTrainerData( INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no , u32 seed,u8 top_floor );


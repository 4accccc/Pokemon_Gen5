//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		infinity_dungeon_local_def.h
//   @brief   無限ダンジョンローカル定義
//   @author	Masayuki Onoue
//   @date		2011.04.11
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
//
//					定数宣言
//
//=============================================================================
#define INFINITY_DUNGEON_TRDATA_MAX (320)    // 無限ダンジョン内トレーナーデータMAX
#define INFINITY_DUNGEON_TALK_PATTERN_MAX (28)    // 無限ダンジョンに登場するトレーナータイプ
#define INFINITY_DUNGEON_TR_ROOM_MIN      (5)     // トレーナーが配置されている部屋数の際少数

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

//-------------------------------------
//	マップ移動タイプ
//=====================================
enum{
  MAP_CHAGE_TYPE_ROOM,
  MAP_CHAGE_TYPE_FOOR,
  MAP_CHAGE_TYPE_MAX,
};

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
//	受付のエレベータID
//=====================================
enum{
  RECEPTION_ELEVATOR_NORMAL,
  RECEPTION_ELEVATOR_EX,
};


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
#define INFINITY_DUNGEON_TALK_PATTERN_MAX (30)    // 無限ダンジョンに登場するトレーナータイプ
#define INFINITY_DUNGEON_TR_ROOM_MIN      (5)     // トレーナーが配置されている部屋数の際少数
#define INFINITY_DUNGEON_BLOCK_OBJ_ID     (0xB0)     // 封鎖に使用する動作モデルID

#define INF_TRAINER_SETLV_1 (100)       // 配置レベル1のトレーナーの出現率
#define INF_TRAINER_SETLV_2 (30)        // 配置レベル2のトレーナーの出現率
#define INF_TRAINER_SETLV_3 (10)        // 配置レベル3のトレーナーの出現率
#define INF_LEFT_ELEVATOR_AREA (4)  // 左エレベーターを使用するエリア番号最大(ゼロオリジン)


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
  RECEPTION_ELEVATOR_L,
  RECEPTION_ELEVATOR_R,
  RECEPTION_ELEVATOR_EX,
};

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

//-------------------------------------
//	８方向（トレーナーの位置を指し示すのに使用）
//=====================================
enum
{
  INFTR_DIR_U,  // 上
  INFTR_DIR_D,  // 下
  INFTR_DIR_L,  // 左
  INFTR_DIR_R,  // 右
  INFTR_DIR_LU,  // 上左
  INFTR_DIR_RU,  // 上右
  INFTR_DIR_LD,  // 下左
  INFTR_DIR_RD,  // 下右
  INFTR_DIR_MAX,
};


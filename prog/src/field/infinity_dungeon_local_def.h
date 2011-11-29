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

#define INF_TRAINER_SETLV_1 (100)       // 配置レベル1のトレーナーの出現率
#define INF_TRAINER_SETLV_2 (30)        // 配置レベル2のトレーナーの出現率
#define INF_TRAINER_SETLV_3 (10)        // 配置レベル3のトレーナーの出現率

//-------------------------------------
//	マップ移動タイプ
//=====================================
enum{
  MAP_CHAGE_TYPE_ROOM,
  MAP_CHAGE_TYPE_FOOR,
  MAP_CHAGE_TYPE_MAX,
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


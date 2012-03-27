//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_zone_arc_data_def.h
 *	@brief  生息地情報アーカイブデータ定義
 *	@author	Masayuki Onoue
 *	@date		2011.09.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "gamesystem/pm_season.h"
#include "savedata/habitat_list_save.h"

//=============================================================================
//
//					定数宣言
//
//=============================================================================
enum{
  HABITAT_ZONE_POKE_LIST_MAX  = 30 ,  // ポケモンデータ最大数
};

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

// 生息地リストポケモンデータ
typedef struct {
  u16   monsno; ///< モンスターナンバー
  u8    is_appear[ PMSEASON_TOTAL ][ HABITAT_GROUND_MAX ]; ///< 出現条件 
  u8    form[ PMSEASON_TOTAL ][ HABITAT_GROUND_MAX ]; ///< 優先フォルム
  u8    padding[2];
}ARC_HABITAT_MONS_DATA;

// 生息地リストアーカイブデータ
typedef struct {
  u8    season_enable[HABITAT_GROUND_MAX];  ///< 季節の有効状態
  u8    land_enable[HABITAT_GROUND_MAX];    ///< 地形の有効状態
  u16   ground_type;                        ///< 地形タイプ
  u16   data_num;                           ///< データ数
  ARC_HABITAT_MONS_DATA mons_data[HABITAT_ZONE_POKE_LIST_MAX];  ///< モンスターデータ
}ARC_HABITAT_ZONE_DATA;


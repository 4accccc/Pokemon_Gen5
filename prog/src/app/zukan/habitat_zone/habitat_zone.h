//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_zone.h
 *	@brief  生息地リスト：ゾーン詳細画面
 *	@author	Masayuki Onoue
 *	@date		2011.08.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "savedata/zukan_savedata.h"
#include "savedata/habitat_list_save.h"
#include "gamesystem/game_data.h"
#include "gamesystem/pm_season.h"
#include "../../../../resource/habitat_list/habitat_list_data.h" // for HABITAT_LIST_DATA_MAX

#include "habitat_zone_arc_data_def.h"

//===================================================================================
//	定数
//===================================================================================
enum{
  HABITAT_ZONE_POKE_DATA_MAX  = 20,   // リストに収まるデータ数
};

// 戻り値
typedef enum{
  HABITAT_ZONE_RET_EXIT,
  HABITAT_ZONE_RET_RETURN,
  HABITAT_ZONE_RET_INFO,
}HABITAT_ZONE_RET_MODE;

// ポケモンの見た、捕獲状態
typedef enum{
  HABITAT_POKE_STATUS_NONEXISTENT,
  HABITAT_POKE_STATUS_UNKNOWN,
  HABITAT_POKE_STATUS_LOOK,
  HABITAT_POKE_STATUS_GET,
}HABITAT_POKE_STATUS;

// 地形の詳細
typedef enum{
  HABITAT_GROUND_INFO_GRASS,
  HABITAT_GROUND_INFO_SAND,
  HABITAT_GROUND_INFO_SWAMP,
  HABITAT_GROUND_INFO_CAVE,
  HABITAT_GROUND_INFO_BRIDGE,
  HABITAT_GROUND_INFO_MULTI,
  HABITAT_GROUND_INFO_NONE,
}HABITAT_GROUND_INFO_TYPE;

// ポケモンデータ
typedef struct {
  u16   monsno; ///< モンスターナンバー
  u8  is_appear[ PMSEASON_TOTAL ][ HABITAT_GROUND_MAX ]; ///< 出現条件 
  u8  form[ PMSEASON_TOTAL ][ HABITAT_GROUND_MAX ]; ///< 季節ごとのフォルム
  u8 status; ///< 見た、捕獲、知らない
}HABITAT_ZONE_POKE_DATA;

// ゾーンデータ
typedef struct {
  u16 zone_group;              ///< ゾーングループ
  u16 season_enable[HABITAT_GROUND_MAX];           ///< 季節の存在状態
  HABITAT_GROUND_INFO_TYPE info_ground_type;  ///< 地形の詳細情報
  HABITAT_ZONE_POKE_DATA poke_data[ HABITAT_ZONE_POKE_LIST_MAX ]; ///< データリスト
  HABITATLIST_STATUS  is_complete[ HABITAT_GROUND_MAX ];  ///< コンプリートしているか
  BOOL                is_zone_complete; //< ゾーンをコンプリートしているか
}HABITAT_ZONE_DATA;

//===================================================================================
//	構造体
//===================================================================================
// 外部設定データ
typedef struct {
	// [in]
  GAMEDATA *p_gdata;				///< ゲームデータ
  u32 zone_cnt;             ///< データ数
  HABITAT_ZONE_DATA zone_list_data[HABITAT_LIST_DATA_MAX];      ///< ゾーンごとのデータ格納先

	// [out]
	u32	ret_mode;								///< 終了モード
}ZUKANHABITAT_ZONE_DATA;

// メインワーク
typedef struct _HABITAT_ZONE_WK HABITAT_ZONE_WK;

//===================================================================================
//	グローバル
//===================================================================================

// 図鑑PROCデータ
extern const GFL_PROC_DATA ZUKANHABITAT_ZONE_ProcData;


//===================================================================================
//	プロトタイプ宣言
//===================================================================================

FS_EXTERN_OVERLAY( zukan_habitat_zone );


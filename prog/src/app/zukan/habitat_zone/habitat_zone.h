//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		habitat_zone.h
//   @brief  生息地リスト：ゾーン詳細画面
//   @author	Masayuki Onoue
//   @date		2011.08.11
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "savedata/zukan_savedata.h"
#include "gamesystem/game_data.h"
#include "gamesystem/pm_season.h"

//===================================================================================
//	定数
//===================================================================================
enum{
  HABITAT_ZONE_POKE_DATA_MAX  = 20,   // リストに収まるデータ数
  HABITAT_ZONE_POKE_LIST_MAX  = 30 ,  // ポケモンデータ最大数
};

// 戻り値
typedef enum{
  HABITAT_ZONE_RET_EXIT,
  HABITAT_ZONE_RET_RETURN,
  HABITAT_ZONE_RET_INFO,
}HABITAT_ZONE_RET_MODE;

// コンプリートタイプ
typedef enum{
  ZONE_COMTYPE_NONE,  // 未コンプリート
  ZONE_COMTYPE_LOOK,  // 見たコンプリート
  ZONE_COMTYPE_GET,   // 捕まえたコンプリート
}ZONE_COMTYPE;

// ポケモンの見た、捕獲状態
typedef enum{
  HABITAT_ZONE_STATUS_NONEXISTENT,
  HABITAT_ZONE_STATUS_UNKNOWN,
  HABITAT_ZONE_STATUS_LOOK,
  HABITAT_ZONE_STATUS_GET,
}HABITAT_ZONE_STATUS;

// 地形タイプ
typedef enum{
  HABITAT_ZONE_APPEAR_LAND,
  HABITAT_ZONE_APPEAR_WATERSIDE,
  HABITAT_ZONE_APPEAR_FISHING,
  HABITAT_ZONE_APPEAR_MAX,
}HABITAT_ZONE_APPEAR;

// 生息地リストアーカイブデータ
typedef struct {
  u16   monsno; // モンスターナンバー
  u8    is_appear[ PMSEASON_TOTAL ][ HABITAT_ZONE_APPEAR_MAX ]; // 出現条件 
  u8    padding[2];
}ARC_HABITAT_MONS_DATA;

// 生息地リストアーカイブデータ
typedef struct {
  u8    season_enable;
  u8    land_enable[HABITAT_ZONE_APPEAR_MAX];
  u16   ground_type;
  u16   data_num;
  ARC_HABITAT_MONS_DATA mons_data[HABITAT_ZONE_POKE_LIST_MAX];
}ARC_HABITAT_ZONE_DATA;

// ポケモンデータ
typedef struct {
  u16   monsno; // モンスターナンバー
  u8  is_appear[ PMSEASON_TOTAL ][ HABITAT_ZONE_APPEAR_MAX ]; // 出現条件 
  u8 status; // 見た、捕獲、知らない
}HABITAT_ZONE_POKE_DATA;

// ゾーンデータ
typedef struct {
  u16 zone_group;              // ゾーングループ
  HABITAT_ZONE_POKE_DATA poke_data[ HABITAT_ZONE_POKE_LIST_MAX ]; // データリスト
}HABITAT_ZONE_DATA;

//===================================================================================
//	構造体
//===================================================================================
// 外部設定データ
typedef struct {
	// [in]
  GAMEDATA *p_gdata;				// ゲームデータ
  u32 zone_cnt;             // データ数
  u32 start_zone_index;     // 開始データインデックス
  u32 start_season;         // 開始季節
  HABITAT_ZONE_APPEAR start_land_type;      // 開始場所タイプ
  HABITAT_ZONE_DATA *p_zone_list_data;      // ゾーンごとのデータ格納先

	// [out]
	u32	ret_mode;								// 終了モード
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


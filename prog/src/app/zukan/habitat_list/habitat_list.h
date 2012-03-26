//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_list.h
 *	@brief  生息地リスト：リスト画面
 *	@author	Masayuki Onoue
 *	@date		2011.08.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "savedata/zukan_savedata.h"
#include "gamesystem/game_data.h"
#include "gamesystem/pm_season.h"
#include "../habitat_zone/habitat_zone.h" // for HABITAT_GROUND_MAX
#include "../../../../resource/habitat_list/habitat_list_data.h" // for HABITAT_LIST_DATA_MAX

// 戻り値
typedef enum{
  HABITAT_LIST_RET_EXIT,
  HABITAT_LIST_RET_RETURN,
  HABITAT_LIST_RET_CALL_ZONE_INFO,
  HABITAT_LIST_RET_ZENKOKU,
  HABITAT_LIST_RET_LOCAL,
}HABITAT_LIST_RET_MODE;

// ゾーンの状態
typedef enum{
  HABITAT_LIST_STATUS_NEW,        // 新着(高優先度)
  HABITAT_LIST_STATUS_NOTICE,     // お知らせあり
  HABITAT_LIST_STATUS_GET_CMP,    // 捕獲コンプリート
  HABITAT_LIST_STATUS_LOOK_CMP,   // 見たコンプリート
  HABITAT_LIST_STATUS_NONE,       // アイコンなし(低優先度)
}HABITAT_LIST_STATUS;

//===================================================================================
//	定数
//===================================================================================
// リストデータ
typedef struct {
  u16           zone_group; // ゾーングループ
  u8            is_exist_landtype[ HABITAT_GROUND_MAX ];
  HABITATLIST_STATUS  is_complete[ HABITAT_GROUND_MAX ];  // コンプリートしているか
  HABITAT_LIST_STATUS list_status[ HABITAT_GROUND_MAX ];  // リストの状態
  BOOL                is_vanish[ HABITAT_GROUND_MAX ];    // リストの表示状態
}ZUKANHABITAT_LIST_LISTDATA;

//===================================================================================
//	構造体
//===================================================================================
// 外部設定データ
typedef struct {
	// [in]
  GAMEDATA  *p_gdata;				                // ゲームデータ
  u8        list_size;
  ZUKANHABITAT_LIST_LISTDATA list_data[HABITAT_LIST_DATA_MAX];  // リストデータ

	// [out]
	u32	ret_mode;								// 終了モード
}ZUKANHABITAT_LIST_DATA;

// メインワーク
typedef struct _HABITAT_LIST_WK HABITAT_LIST_WK;

//===================================================================================
//	グローバル
//===================================================================================

// 図鑑PROCデータ
extern const GFL_PROC_DATA ZUKANHABITAT_LIST_ProcData;


//===================================================================================
//	プロトタイプ宣言
//===================================================================================

FS_EXTERN_OVERLAY( zukan_habitat_list );



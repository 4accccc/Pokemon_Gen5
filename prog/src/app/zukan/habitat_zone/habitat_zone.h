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
#include "habitat_zone_gra.h"
#include "gamesystem/pm_season.h"

//=============================================================================
//
//					定数宣言
//
//=============================================================================

// 出現場所定義
typedef enum{
  HABITAT_ZONE_APPEAR_LAND,
  HABITAT_ZONE_APPEAR_WATERSIDE,
  HABITAT_ZONE_APPEAR_FISHING,
  HABITAT_ZONE_APPEAR_MAX,
}HABITAT_ZONE_APPEAR;

// 出現場所定義
typedef enum{
  HABITAT_ZONE_STATUS_NONEXISTENT,
  HABITAT_ZONE_STATUS_UNKNOWN,
  HABITAT_ZONE_STATUS_LOOK,
  HABITAT_ZONE_STATUS_GET,
}HABITAT_ZONE_STATUS;

//-------------------------------------
///	シーケンス切替パラメータ
//=====================================
typedef enum {
  SEQ_CHANGE_PARAM_L,
  SEQ_CHANGE_PARAM_R,
  SEQ_CHANGE_PARAM_UP,
  SEQ_CHANGE_PARAM_DOWN,
  SEQ_CHANGE_PARAM_A,
  SEQ_CHANGE_PARAM_TOUCH,
  SEQ_CHANGE_PARAM_NONE,
}SEQ_CHANGE_PARAM;

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

// リストデータ
typedef struct {
  s32   monsno; // モンスターナンバー
  BOOL  is_appear[ PMSEASON_TOTAL ][ HABITAT_ZONE_APPEAR_MAX ]; // 出現条件 
  HABITAT_ZONE_STATUS status; // 見た、捕獲、知らない
}HABITAT_ZONE_POKE_DATA;


typedef struct {
  u16 zone_id;              // ゾーンID
  HABITAT_ZONE_POKE_DATA poke_data[ HABITAT_ZONE_POKE_LIST_MAX ]; // データリスト
}HABITAT_ZONE_DATA;

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

// ゾーン詳細画面ワーク
typedef struct {
  u32 main_seq;
  u32 sub_seq;
  ZUKANHABITAT_ZONE_DATA *p_init_param;
  HABITAT_ZONE_GRA_WK gra_sys;
  GFL_MSGDATA         *p_app_msg;
  GFL_TCB             *p_vtask;

  BOOL  is_cursor_active;           // カーソルが選択状態かどうか
  s32   select_index;               // 選択中のポケモン
  s32   zone_data_cnt;              // データ数
  s32   list_size;                  // ポケモンリストに表示されている数
  s32   zone_data_index;            // リストのインデックス
  s32   season;                     // 表示中のの季節
  HABITAT_ZONE_APPEAR land_type;  // 表示中のの場所タイプ
  WORDSET *p_word_set;

  SEQ_CHANGE_PARAM seq_param; // シーケンス切替パラメータ

}HABITAT_ZONE_WK;

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================


//===================================================================================
//	グローバル
//===================================================================================

// 図鑑PROCデータ
extern const GFL_PROC_DATA ZUKANHABITAT_ZONE_ProcData;


//===================================================================================
//	プロトタイプ宣言
//===================================================================================

FS_EXTERN_OVERLAY( zukan_habitat_zone );


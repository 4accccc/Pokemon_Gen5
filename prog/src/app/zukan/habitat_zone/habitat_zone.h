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
  SEQ_CHANGE_PARAM_U,
  SEQ_CHANGE_PARAM_D,
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
  BOOL  is_appear_land[ HABITAT_ZONE_APPEAR_MAX ]; // 場所出現するかどうか
  BOOL  is_appear_season[ PMSEASON_TOTAL ]; // 四季出現するかどうか
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
  u16 zone_cnt;             // データ数
  u16 start_zone_index;     // 開始データインデックス
  u16 start_season;         // 開始季節
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

  u16 zone_data_cnt;             // データ数
  u8  list_size;            // ポケモンリストに表示されている数
  u8  zone_data_index;           // リストのインデックス
  u8  season;
  HABITAT_ZONE_APPEAR land_type;
  WORDSET *p_word_set;

  SEQ_CHANGE_PARAM seq_param; // シーケンス切替パラメータ

}HABITAT_ZONE_WK;

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================


//============================================================================================
//	グローバル
//============================================================================================

// 図鑑PROCデータ
extern const GFL_PROC_DATA ZUKANHABITAT_ZONE_ProcData;


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

FS_EXTERN_OVERLAY( zukan_habitat_zone );


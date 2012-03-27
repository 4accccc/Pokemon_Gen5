//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_zone_main_def.h
 *	@brief  生息地リスト：ゾーン詳細画面用データ定義
 *	@author	Masayuki Onoue
 *	@date		2011.08.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "habitat_zone_gra.h" // for HABITAT_ZONE_GRA_WK
#include "habitat_zone.h"

//=============================================================================
//
//					定数宣言
//
//=============================================================================

// 定義ヘッダー
enum
{
  // メッセージ表示場所
  LAND_MSG_POS_X = 12,
  LAND_MSG_POS_Y = 4,
  LAND_MSG_W = 240,
  LAND_MSG_H = 32,
  SEASON_MSG_POS_X = 40,
  SEASON_MSG_POS_Y = 52,
  SEASON_MSG_W = 32,
  SEASON_MSG_H = 16,
  EXPLANATION_MSG_POS_X = 124,
  EXPLANATION_MSG_POS_Y = 172,
  EXPLANATION_MSG_W = 96,
  EXPLANATION_MSG_H = 16,
  MAPNAME_MSG_POS_X = 8,
  MAPNAME_MSG_POS_Y = 4,
  MAPNAME_MSG_W = 256,
  MAPNAME_MSG_H = 16,
  NOHABITATDATA_MSG_POS_X = 56,
  NOHABITATDATA_MSG_POS_Y = 120,
  NOHABITATDATA_MSG_W = 200,
  NOHABITATDATA_MSG_H = 16,
  STUMP_SE_FRAME = 19,
};


//-------------------------------------
///	シーケンス切替パラメータ
//=====================================
typedef enum {
  SEQ_CHANGE_PARAM_L,
  SEQ_CHANGE_PARAM_R,
  SEQ_CHANGE_PARAM_UP,
  SEQ_CHANGE_PARAM_DOWN,
  SEQ_CHANGE_PARAM_A,
  SEQ_CHANGE_PARAM_B,
  SEQ_CHANGE_PARAM_X,
  SEQ_CHANGE_PARAM_TOUCH,
  SEQ_CHANGE_PARAM_TOUCH_INIT,
  SEQ_CHANGE_PARAM_POKE_SELECT,
  SEQ_CHANGE_PARAM_NONE,
  SEQ_CHANGE_PARAM_SET_LAND_TYPE,
}SEQ_CHANGE_PARAM;

// メインシーケンス状態定義
typedef enum{
  HABITAT_ZONE_MAIN_CONTINUE,
  HABITAT_ZONE_MAIN_EXIT,
}HABITAT_ZONE_MAIN_STATUS;

//-------------------------------------
///	SE
//=====================================
enum {
  SE_CALL_INFO          = SEQ_SE_DECIDE1,
  SE_ZONE_CHANGE        = SEQ_SE_SELECT1,
  SE_SEASON_CHANGE      = SEQ_SE_SELECT3,
  SE_GROUND_TYPE_CHANGE = SEQ_SE_SELECT3,
  SE_TOP_TO_POKESELECT  = SEQ_SE_DECIDE1,
  SE_RETURN             = SEQ_SE_CANCEL1,
  SE_EXIT               = SEQ_SE_CLOSE1,
  SE_Y_REG              = SEQ_SE_SYS_07,
  SE_CURSOR_MOVE        = SEQ_SE_SELECT1,
  SE_POKE_SELECT        = SEQ_SE_DECIDE1,
};

//=============================================================================
//
//					構造体宣言
//
//=============================================================================


// ゾーン詳細画面ワーク
struct _HABITAT_ZONE_WK{
  HABITAT_ZONE_MAIN_STATUS main_status;
  u32 main_seq;
  u32 sub_seq;
  ZUKANHABITAT_ZONE_DATA *p_init_param;
  HABITAT_ZONE_GRA_WK gra_sys;
  GFL_MSGDATA         *p_app_msg;
  GFL_TCB             *p_vtask;

  BOOL  is_cursor_active;           ///<  カーソルが選択状態かどうか
  s32   select_index;               ///<  選択中のポケモン
  s32   tp_select_index;            ///<  タッチで選択したインデックス
  s32   zone_data_cnt;              ///<  データ数
  s32   list_size;                  ///<  ポケモンリストに表示されている数
  s32   zone_data_index;            ///<  リストのインデックス
  s32   season;                     ///<  表示中のの季節
  HABITAT_GROUND_TYPE land_type;    ///<  表示中のの場所タイプ
  WORDSET *p_word_set;
  HABITATLIST_STATUS cmp_event_id;  ///<  コンプリートイベントのID

  // ゾーンごとに更新されるデータ -----------------
  BOOL  land_appear[ HABITAT_GROUND_MAX ];           ///<  地形を表示するか
  BOOL  land_complete_status[ HABITAT_GROUND_MAX ];  ///<  地形ごとのコンプリート
  u32   monsno_list[HABITAT_ZONE_POKE_LIST_MAX];  ///<  表示中のモンスター番号リスト

  SEQ_CHANGE_PARAM seq_param; ///<  シーケンス切替パラメータ
  
  // 図鑑セーブデータ
  ZUKAN_SAVEDATA    *p_zkn_sv;
  HABITATLIST_SAVE  *p_habitat_sv;
  
  // スクロール
  s32 main_scroll_cnt;
  s32 sub_scroll_cnt;

  // 地形位置
  HABITAT_GROUND_TYPE default_land_type;    ///<  表示中のの場所タイプ
  
  // はんこ演出
  u32 stump_se_frame;

  // 生息していないデータかどうか
  BOOL is_no_habitat; ///< 生息していないデータかどうか

};


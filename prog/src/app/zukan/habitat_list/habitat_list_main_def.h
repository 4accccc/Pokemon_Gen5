//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_list_main_def.h
 *	@brief  生息地リスト：リスト画面用定義
 *	@author	Masayuki Onoue
 *	@date		2011.08.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "habitat_list_gra.h" // for HABITAT_LIST_GRA_WK
#include "savedata/zukan_savedata.h"

//=============================================================================
//
//					定数宣言
//
//=============================================================================

//-------------------------------------
/// フォント色
//=====================================

#define MBG_BUTTON_FONT_COL PRINTSYS_LSB_Make(15,2,1)
#define SBG_TITLE_FONT_COL PRINTSYS_LSB_Make(0xE,0xF,0)


//-------------------------------------
/// メインシーケンス状態定義
//=====================================
typedef enum{
  HABITAT_LIST_MAIN_CONTINUE,
  HABITAT_LIST_MAIN_EXIT,
}HABITAT_LIST_MAIN_STATUS;

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
  SEQ_CHANGE_PARAM_NONE,
  SEQ_CHANGE_PARAM_NOANM_WAIT,
}SEQ_CHANGE_PARAM;

//-------------------------------------
///	文字表示位置
//=====================================
enum{
  TITLE_MSG_POS_X = 8,
  TITLE_MSG_POS_Y = 4,
  TITLE_MSG_W = 256,
  TITLE_MSG_H = 16,
  BUTTON_01_POS_X = 0,
  BUTTON_01_POS_Y = 110 - 12,
  BUTTON_01_MSG_W = 40,
  BUTTON_01_MSG_H = 16,
  BUTTON_02_POS_X = 0,
  BUTTON_02_POS_Y = 133 - 12,
  BUTTON_02_MSG_W = 40,
  BUTTON_02_MSG_H = 16,
  BUTTON_03_POS_X = 0,
  BUTTON_03_POS_Y = 156 - 12,
  BUTTON_03_MSG_W = 40,
  BUTTON_03_MSG_H = 16,
};

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

// リスト画面ワーク
struct _HABITAT_LIST_WK{
  HABITAT_LIST_MAIN_STATUS main_status;
  u32 main_seq;
  u32 sub_seq;
  ZUKANHABITAT_LIST_DATA *p_init_param;
  HABITAT_LIST_GRA_WK gra_sys;
  GFL_MSGDATA         *p_app_msg;
  GFL_TCB             *p_vtask;
  GFL_TCB             *p_htask;
  BOOL  is_zenkoku;

  WORDSET         *p_word_set;
  ZUKAN_SAVEDATA  *p_zkn_sv;
  HABITATLIST_SAVE *p_habitat_sv;

  SEQ_CHANGE_PARAM seq_param; // シーケンス切替パラメータ
  u32 list_key_state;         // リストキー状態

  // アニメーション待ち用
  u8  anm_seq_no;             // アニメーションシーケンス番号
  u8  anm_clwk_id;            // アニメーション中のセルアクターワークID
  u32 next_seq_id;
  SEQ_CHANGE_PARAM next_seq_param; // シーケンス切替パラメータ

  // ボタン点滅用
  u8	button_cnt;					    // ボタンアニメ用カウンタ

  // スクロール
  s32 main_scroll_cnt;
  s32 sub_scroll_cnt;

};


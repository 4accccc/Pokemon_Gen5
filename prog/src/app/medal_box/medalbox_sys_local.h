//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		medalboxsys_local.h
//   @brief
//   @author	Masayuki Onoue
//   @date		2011.01.13
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once



//=============================================================================
//
//					定数宣言
//
//=============================================================================

// ==================
// シーケンス番号
// ==================
typedef enum{
  SEQ_MDLBOX_INIT,
  SEQ_MDLBOX_MAIN,
  SEQ_MDLBOX_SCROLLPAGE,
  SEQ_MDLBOX_SCROLL_BUTTON,
  SEQ_MDLBOX_EXIT,
  SEQ_MDLBOX_MAX,
} MEDALBOXSYS_SEQ;

// ==================
// スクロール方向
// ==================
typedef enum{
  SCROLL_DIR_RIGHT = 1,
  SCROLL_DIR_LEFT  = -1,
} SCROLL_DIR;

// ==================
// メダルボックスモード
// ==================
typedef enum{
  BOXMODE_NORMAL,
  BOXMODE_LIST,
} MEDALBOXSYS_MODE;

// ==================
// リソースID
// ==================
enum{
  OBJRESID_MEDAL_CELLANM,
  OBJRESID_MEDAL_PLTT,
  OBJRESID_MEDAL_CGR,
  OBJRESID_UI_CELLANM,
  OBJRESID_UI_PLTT,
  OBJRESID_UI_CGR,
  OBJRESID_COMMONUI_CELLANM,
  OBJRESID_COMMONUI_PLTT,
  OBJRESID_COMMONUI_CGR,
  OBJRESID_COMMONUI_SEARCH_CGR,

  OBJRESID_MAX,
};

// ==================
// キーモード
// ==================
typedef enum{
  KEYMODE_KEY,
  KEYMODE_TP,
} KEY_MODE;

// ==================
// カーソルの状態
// ==================
typedef enum{
  CURSOR_STATE_NOT_SELECT,
  CURSOR_STATE_SCROLL,
  CURSOR_STATE_SELECT,
} CURSOR_STATE;

// ==================
// UI用当たり判定テーブル
// ==================
typedef enum{
  UI_HIT_TABLE_TRG_SEARCH,
  UI_HIT_TABLE_TRG_CURSOR_LEFT,
  UI_HIT_TABLE_TRG_CURSOR_RIGHT,
  UI_HIT_TABLE_TRG_RETURN,
  UI_HIT_TABLE_TRG_EXIT,
  UI_HIT_TABLE_TRG_END, // テーブル終了定義
  UI_HIT_TABLE_TRG_MAX,
} UI_HIT_TABLE_TRG_TYPE;

// ==================
// UI用当たり判定テーブル
// ==================
typedef enum{
  UI_HIT_TABLE_CONT_SCROLL_BUTTON,
  UI_HIT_TABLE_CONT_END, // テーブル終了定義
  UI_HIT_TABLE_CONT_MAX,
} UI_HIT_TABLE_CONT_TYPE;

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================


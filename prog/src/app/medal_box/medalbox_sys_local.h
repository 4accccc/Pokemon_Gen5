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
  SEQ_MDLBOX_MENU,
  SEQ_MDLBOX_SORT,
  SEQ_MDLBOX_MODE_CHANGE,
  SEQ_MDLBOX_RETURN,
  SEQ_MDLBOX_EXIT,
  SEQ_PUSUMEDALBUTTON,
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
  OBJRESID_COMMONUI_SEARCH_CELLANM,

  OBJRESID_MAX,
};

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

// ==================
// メニュー用メッセージID
// ==================
typedef enum
{
  MENU_MSG_MODE_CHANGE  = MSG_MEDALBOX_08,
  MENU_MSG_ONLY_TITLE   = MSG_MEDALBOX_09,
  MENU_MSG_ONLY_HINT    = MSG_MEDALBOX_10,
  MENU_MSG_TYPE         = MSG_MEDALBOX_11,
  MENU_MSG_NAME         = MSG_MEDALBOX_12,
  MENU_MSG_NEW          = MSG_MEDALBOX_13,
  MENU_MSG_DEFAULT      = MSG_MEDALBOX_14,
  MENU_MSG_CANCEL       = MSG_MEDALBOX_15,
}MENU_MSG_TAG;

// ==================
// メニュー用メッセージID
// ==================
typedef enum
{
  INFOTYPE_TARGET,
  INFOTYPE_GET,
}INFOTYPE;

// ==================
// メニュー用メッセージID
// ==================
typedef enum
{
  PUSHMEDAL_TYPE_SET,
  PUSHMEDAL_TYPE_REMOVE,
  PUSHMEDAL_TYPE_MAX,
}PUSHMEDAL_TYPE;

// ==================
// セルアニメーションシーケンス
// ==================
typedef enum
{
  ANMSEQ_NEWEFFECT = 4,
}CELL_ANM_SEQ;

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


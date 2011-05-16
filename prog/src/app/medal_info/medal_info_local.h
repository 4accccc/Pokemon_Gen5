//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medal_info_local.h
 *	@brief  メダル詳細画面ローカル
 *	@author	Masayuki Onoue
 *	@date		2011.05.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//-------------------------------------
///	BGフレーム
//=====================================
typedef enum
{
	MEDALINFO_BG_FRAME_BACK_M,	    // 背景
  MEDALINFO_BG_FRAME_FONTBORD_M,  // 文字板
	MEDALINFO_BG_FRAME_BAR_M,       // ページユニット
	MEDALINFO_BG_FRAME_FONT_M,		  // 文字用

	MEDALINFO_BG_FRAME_BACK_S ,	    // 背景

	MEDALINFO_BG_FRAME_MAX,			//ソース内で使用
}MEDALINFO_GRAPHIC_BG_FRAME;

//-------------------------------------
///	BGプライオリティー
//=====================================
typedef enum
{
  MEDALINFO_BG_PRIORITY_BAR_M       = 0,
  MEDALINFO_BG_PRIORITY_FONT_M,
  MEDALINFO_BG_PRIORITY_FONTBORD_M,
  MEDALINFO_BG_PRIORITY_BACK_M,

  MEDALINFO_BG_PRIORITY_BACK_S     = 0,
}MEDALINFO_BG_PRIORITY;

//-------------------------------------
///	OBJプライオリティー(BG)
//=====================================
typedef enum
{
  MEDALINFO_OBJ_PRIORITY_UI         = 0,
  MEDALINFO_OBJ_PRIORITY_MEDAL      = 1,
}MEDALINFO_OBJ_PRIORITY;

//-------------------------------------
///	CKWK
//=====================================
typedef enum
{
	OBJ_CLWK_MEDAL,
	OBJ_CLWK_EXIT,
	OBJ_CLWK_RETURN,
	OBJ_CLWK_MAX,
}MEDALINFO_GRAPHIC_OBJ_CLWK ;

//-------------------------------------
// パレット番号
//=====================================
enum
{
  MDLINFO_BACK_PAL_ID = 0,
  MDLINFO_BAR_PAL_ID  = 1,
  MDLINFO_FONT_PAL_ID = 2,
}MDLINFO_PAL_ID;

//-------------------------------------
// 文字で使用するパレット番号
//=====================================
enum
{
  FLONT_COLLAR_L = 1,
  FLONT_COLLAR_S = 2,
  FLONT_COLLAR_B = 0,
}STR_COLOR;

//-------------------------------------
// 文字の表示位置
//=====================================
enum
{
 STR_POS_TITLE_X = (8),
 STR_POS_TITLE_Y = (8),
 STR_POS_TITLE_KAKUTOKU_X = (128),
 STR_POS_TITLE_KAKUTOKU_Y = (32),
 STR_POS_KAKUTOKU_X = (216),
 STR_POS_KAKUTOKU_Y = (28),
 STR_POS_TITLE_ICHIOSI_X = (48),
 STR_POS_TITLE_ICHIOSI_Y = (52),
 STR_POS_MEDAL_NAME_X = (48),
 STR_POS_MEDAL_NAME_Y = (64),
 STR_POS_TITLE_GET_DAY_X = (48),
 STR_POS_TITLE_GET_DAY_Y = (88),
 STR_POS_GET_DAY_X = (152),
 STR_POS_GET_DAY_Y = (84),
 STR_POS_INFO_X = (16),
 STR_POS_INFO_Y = (108),

 // タイトル画面が登録されていない場合に使用
 STR_POS_TITLE_X_NO_PUSH = (8),
 STR_POS_TITLE_Y_NO_PUSH = (8),
 STR_POS_TITLE_KAKUTOKU_X_NO_PUSH = (128),
 STR_POS_TITLE_KAKUTOKU_Y_NO_PUSH = (32+28),
 STR_POS_KAKUTOKU_X_NO_PUSH = (216),
 STR_POS_KAKUTOKU_Y_NO_PUSH = (28+28),
 STR_POS_TITLE_ICHIOSI_X_NO_PUSH = (8),
 STR_POS_TITLE_ICHIOSI_Y_NO_PUSH = (52+28),
 STR_POS_MEDAL_NAME_X_NO_PUSH = (8),
 STR_POS_MEDAL_NAME_Y_NO_PUSH = (64+28),

}STR_POS_DATA;

//-------------------------------------
// OBJ表示位置
//=====================================
enum
{
  OBJ_POS_MEDAL_X = (24),
  OBJ_POS_MEDAL_Y = (64),
  OBJ_POS_RETURN_X = (32-4)*8,
  OBJ_POS_RETURN_Y = (24-3)*8,
  OBJ_POS_EXIT_X = (32-9)*8,
  OBJ_POS_EXIT_Y = (24-3)*8,
};

//-------------------------------------
///	アーカイブタイプ
//=====================================
enum{
  ARC_TYPE_MEDAL_ICON,
  ARC_TYPE_COMMON_BUTTON_GRA,
  ARC_TYPE_MAX,
};

// ==================
// リソースID
// ==================
enum{
  OBJRESID_MEDAL_CELLANM,
  OBJRESID_MEDAL_PLTT,
  OBJRESID_MEDAL_CGR,
  OBJRESID_COMMONUI_CELLANM,
  OBJRESID_COMMONUI_PLTT,
  OBJRESID_COMMONUI_CGR,

  OBJRESID_MAX,
};

// ==================
// UI用当たり判定テーブル
// ==================
typedef enum{
  UI_HIT_TABLE_TRG_R, // 右側ボタン
  UI_HIT_TABLE_TRG_L, // 左側ボタン
  UI_HIT_TABLE_TRG_END, // テーブル終了定義
  UI_HIT_TABLE_TRG_MAX,
} UI_HIT_TABLE_TRG_TYPE;

// ==================
// メインシーケンス
// ==================
enum{
  SEQ_FADE_IN_START,
  SEQ_FADE_IN_WAIT,
  SEQ_MAIN,
  SEQ_FADE_OUT_START,
  SEQ_FADE_OUT_WAIT,
  SEQ_EXIT,
};

#define MEDALOBJ_PALETTENUM_MEDAL       (6)       // メダルOBJのパレット使用本数
#define MEDALOBJ_PALETTENUM_COMMON_UI   (4)       // COMMONUIOBJのパレット使用本数

#define BACK_SCROLL_SPEED (0x800) // 背景のスクロールスピード


// ==================
// SE
// ==================
#define SE_EXIT   (SEQ_SE_CLOSE1)
#define SE_CANCEL (SEQ_SE_CANCEL1)


//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		habitat_zone_gra.h
//   @brief   生息地リスト：ゾーン詳細画面グラフィックモジュール
//   @author	Masayuki Onoue
//   @date		2011.08.11
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
//
//					定数宣言
//
//=============================================================================
// パレットアニメ
#include "system/palanm.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

// プリントシステム
#include "app/app_printsys_common.h"

// OBJ初期位置
enum
{
  OBJ_POS_L_X = 24,
  OBJ_POS_L_Y = 56,
  OBJ_POS_R_X = 80,
  OBJ_POS_R_Y = 56,
  OBJ_POS_STAMP_BIG_X = 224,
  OBJ_POS_STAMP_BIG_Y = 20,
  OBJ_POS_STAMP_LAND_X = 128,
  OBJ_POS_STAMP_LAND_Y = 48,
  OBJ_POS_STAMP_WATERSIDE_X = 176,
  OBJ_POS_STAMP_WATERSIDE_Y = 48,
  OBJ_POS_STAMP_FISHING_X   = 224,
  OBJ_POS_STAMP_FISHING_Y   = 48,
};

// アニメーションシーケンス定義
enum
{
  OBJ_ANM_SEQ_R     = 22,
  OBJ_ANM_SEQ_R_ANM = 23,
  OBJ_ANM_SEQ_L     = 24,
  OBJ_ANM_SEQ_L_ANM = 25,
  OBJ_ANM_SEQ_STAMP_BIG_LOOK    = 27,
  OBJ_ANM_SEQ_STAMP_BIG_GET     = 26,
  OBJ_ANM_SEQ_STAMP_MIDDLE_LOOK = 9,
  OBJ_ANM_SEQ_STAMP_MIDDLE_GET  = 7,
};

// OBJWKID
enum
{
  OBJ_WK_SEASON_L,
  OBJ_WK_SEASON_R,
  OBJ_WK_STAMP_BIG,
  OBJ_WK_STAMP_LAND,
  OBJ_WK_STAMP_WATERSIDE,
  OBJ_WK_STAMP_FISHING,
  OBJ_WK_STAMP_SMALL_01,
  OBJ_WK_STAMP_SMALL_02,
  OBJ_WK_STAMP_SMALL_03,
  OBJ_WK_STAMP_SMALL_04,
  OBJ_WK_STAMP_SMALL_05,
  OBJ_WK_STAMP_SMALL_06,
  OBJ_WK_STAMP_SMALL_07,
  OBJ_WK_STAMP_SMALL_08,
  OBJ_WK_STAMP_SMALL_09,
  OBJ_WK_STAMP_SMALL_10,
  OBJ_WK_STAMP_SMALL_11,
  OBJ_WK_STAMP_SMALL_12,
  OBJ_WK_STAMP_SMALL_13,
  OBJ_WK_STAMP_SMALL_14,
  OBJ_WK_STAMP_SMALL_15,
  COMMON_OBJ_WK_MAX,

  // ポケモンごとのスタンプアイコン開始位置
  OBJ_WK_STAMP_SMALL_START = OBJ_WK_STAMP_SMALL_01,
};

// 定数定義
enum{
  HABITAT_ZONE_POKE_LIST_MAX = 15,  // ゾーン詳細画面のポケモンデータ最大数
  HABITAT_ZONE_UNIT_CLWK_MAX = HABITAT_ZONE_POKE_LIST_MAX + COMMON_OBJ_WK_MAX,
};

// OBJ表示優先度
enum
{
  HABITAT_ZONE_OBJPRI_STAMP,
  HABITAT_ZONE_OBJPRI_MINI_STAMP,
  HABITAT_ZONE_OBJPRI_POKEICON,
};

//-------------------------------------
///	BG面定義
//=====================================
typedef enum _BG_FRAME{
  BGM_FRAME_FONT        = GFL_BG_FRAME0_M,
  BGM_FRAME_MENUBAR     = GFL_BG_FRAME1_M,
  BGM_FRAME_BACK        = GFL_BG_FRAME2_M,
  BGM_FRAME_SCROLL_WIN  = GFL_BG_FRAME3_M,
  BGS_FRAME_FONT        = GFL_BG_FRAME0_S,
  BGS_FRAME_BAR         = GFL_BG_FRAME1_S,
  BGS_FRAME_MAP         = GFL_BG_FRAME2_S,
  BGS_FRAME_NO_USE      = GFL_BG_FRAME3_S,
}BG_FRAME;

// ==================================
// プリントユーティリティー準備
// ==================================
typedef enum
{
  PRINT_UTIL_INFO_WIN,
  PRINT_UTIL_MAX,
}PRINT_UTIL_WIN;

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

// ゾーン詳細画面ワーク:グラフィック
typedef struct {
  // パレットフェード　-------------------------------
  PALETTE_FADE_PTR  p_pfd;
	
  // セルアクターユニット
  GFL_CLUNIT			*p_clunit;
  
  // ポケアイコン ------------------------------------
  UI_EASY_CLWK_RES  clres_poke_icon[HABITAT_ZONE_POKE_LIST_MAX];
  GFL_CLWK          *p_clwk_poke_icon[HABITAT_ZONE_POKE_LIST_MAX];
  
  // 共通OBJリソース
  UI_EASY_CLWK_RES  clres_common_obj;
  GFL_CLWK          *p_clwk_common_obj[COMMON_OBJ_WK_MAX];
  
  // 文字ウィンドウ---------------------
  BOOL              is_print_util_wati;         // 転送待ち
	GFL_FONT			    *p_font;
  PRINT_UTIL        print_util[PRINT_UTIL_MAX]; // プリントユーティリティー
  PRINT_QUE         *p_print_que;
  GFL_BMPWIN        *p_font_win;
  
  // 背景スクロール---------------------
  u32 back_bg_scroll_wait_count;

}HABITAT_ZONE_GRA_WK;

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern void HABITAT_ZONE_GRA_Init( HABITAT_ZONE_GRA_WK *p_wk );
extern void HABITAT_ZONE_GRA_CallOpeningFade( HABITAT_ZONE_GRA_WK *p_wk );
extern void HABITAT_ZONE_GRA_Exit( HABITAT_ZONE_GRA_WK *p_wk );
extern void HABITAT_ZONE_GRA_VblankMain( HABITAT_ZONE_GRA_WK *p_wk );
extern void HABITAT_ZONE_GRA_Main( HABITAT_ZONE_GRA_WK *p_wk );
extern GFL_CLWK* HABITAT_ZONE_GRA_LoadPokeIcon( HABITAT_ZONE_GRA_WK *p_wk ,u32 mons_no ,u8 index );
extern BOOL HABITAT_ZONE_GRA_CheckPrintUtilStatus( HABITAT_ZONE_GRA_WK *p_wk );
extern void HABITAT_ZONE_GRA_DrawStr( PRINT_UTIL_WIN win_id, const STRBUF* buf,
    HABITAT_ZONE_GRA_WK *p_wk ,s16 x, s16 y ,PRINTSYS_LSB font_colro );
//extern void HABITAT_ZONE_GRA_FillBmpWin( PRINT_UTIL_WIN win_id ,u8 x ,u8 y, u8 w ,u8 h ,u8 col_code ,HABITAT_ZONE_GRA_WK *p_wk );
extern void HABITAT_ZONE_GRA_BmpFil( PRINT_UTIL_WIN win_id ,u8 col_code ,HABITAT_ZONE_GRA_WK *p_wk );
extern GFL_CLWK* HABITAT_ZONE_GRA_GetComClWk( HABITAT_ZONE_GRA_WK *p_wk ,s32 obj_id );


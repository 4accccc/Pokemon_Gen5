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

// 点滅アニメーション
#include "system/blink_palanm.h"

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
  OBJ_POS_BUTTON_LAND_X         = 80,
  OBJ_POS_BUTTON_LAND_Y         = 184,
  OBJ_POS_BUTTON_WATERSIDE_X    = 120,
  OBJ_POS_BUTTON_WATERSIDE_Y    = 184,
  OBJ_POS_BUTTON_FISHING_X      = 160,
  OBJ_POS_BUTTON_FISHING_Y      = 184,
  OBJ_POS_BUTTON_EXPLANATION_X  = 160,
  OBJ_POS_BUTTON_EXPLANATION_Y  = 180,
  OBJ_POS_BUTTON_UP_ARROW_X     = 0,
  OBJ_POS_BUTTON_UP_ARROW_Y     = 168,
  OBJ_POS_BUTTON_DOWN_ARROW_X   = 24,
  OBJ_POS_BUTTON_DOWN_ARROW_Y   = 168,
  OBJ_POS_BUTTON_CHECK_X        = 184,
  OBJ_POS_BUTTON_CHECK_Y        = 172,
  OBJ_POS_BUTTON_CANCEL_X       = 208,
  OBJ_POS_BUTTON_CANCEL_Y       = 168,
  OBJ_POS_BUTTON_RETURN_X       = 232,
  OBJ_POS_BUTTON_RETURN_Y       = 168,
};

// アニメーションシーケンス定義
enum
{
  OBJ_ANM_SEQ_R     = 22,
  OBJ_ANM_SEQ_R_ANM = 23,
  OBJ_ANM_SEQ_L     = 24,
  OBJ_ANM_SEQ_L_ANM = 25,
  OBJ_ANM_SEQ_STAMP_BIG_LOOK      = 27,
  OBJ_ANM_SEQ_STAMP_BIG_GET       = 26,
  OBJ_ANM_SEQ_STAMP_MIDDLE_LOOK   = 9,
  OBJ_ANM_SEQ_STAMP_MIDDLE_GET    = 7,
  OBJ_ANM_SEQ_BUTTON_LAND         = 16,
  OBJ_ANM_SEQ_BUTTON_WATERSIDE    = 17,
  OBJ_ANM_SEQ_BUTTON_FISHING      = 18,
  OBJ_ANM_SEQ_BUTTON_EXPLANATION  = 28,
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
  OBJ_WK_BUTTON_LAND,
  OBJ_WK_BUTTON_WATERSIDE,
  OBJ_WK_BUTTON_FISHING,
  OBJ_WK_BUTTON_EXPLANATION,
  OBJ_WK_UP_ARROW,
  OBJ_WK_DOWN_ARROW,
  OBJ_WK_CHECK_BOX,
  OBJ_WK_CANCEL,
  OBJ_WK_RETURN,
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

// 共通素材用OBJWKID
enum
{
  APP_MENU_OBJ_WK_MAX,
};

// 定数定義
enum{
  HABITAT_ZONE_POKE_LIST_MAX  = 15,  // ゾーン詳細画面のポケモンデータ最大数
  HABITAT_ZONE_UNIT_CLWK_MAX  = HABITAT_ZONE_POKE_LIST_MAX + COMMON_OBJ_WK_MAX,
  LAND_TYPE_ICON_PAL_ON       = 0xA,  // 地形タイプがONの時のパレット
  LAND_TYPE_ICON_PAL_OFF      = 0x9,  // 地形タイプがOFFの時のパレット
  LAND_TYPE_ICON_PAL_NONE     = 0xB,  // 地形タイプがないの時のパレット
  LAND_TYPE_ICON_W            = 4,   // 地形タイプアイコンの幅
  LAND_TYPE_ICON_H            = 3,   // 地形タイプアイコンの高さ
  LAND_TYPE_ICON_LAND_POS_X   = 13,   // 草地アイコンの開始座標
  LAND_TYPE_ICON_LAND_POS_Y   = 6,   // 草地アイコンの開始座標
  LAND_TYPE_ICON_WATERSIDE_POS_X  = 19,   // 草地アイコンの開始座標
  LAND_TYPE_ICON_WATERSIDE_POS_Y  = 6,    // 草地アイコンの開始座標
  LAND_TYPE_ICON_FISHING_POS_X    = 25,   // 草地アイコンの開始座標
  LAND_TYPE_ICON_FISHING_POS_Y    = 6,    // 草地アイコンの開始座標
  BLINK_PALID_POKE_CURSOR_CHANGE  = 0xA,  // 滅用パレット番号(書き換え対象)
  BLINK_PALID_POKE_CURSOR_START   = 0x9,  // ポケモン選択点滅用パレット番号(基準)
  BLINK_PALID_POKE_CURSOR_END     = 0xA,  // ポケモン選択点滅用パレット番号(終了)
  BLINK_PAL_SIZE_POKE_CURSOR      = 5,    // 点滅させるパレットの数
  POKEICON_ROW      = 3,  // リストの行数
  POKEICON_LINE     = 5,  // リストの列数
  POKEICON_START_X  = 48, // リストの開始X座標
  POKEICON_START_Y  = 88, // リストの開始Y座標
  POKEICON_MARGIN_W = 40, // 横のアイコンまでの距離
  POKEICON_MARGIN_Y = 32, // 下のアイコンまでの距離
  POKEICON_CURSOR_POS_X   = 4,    // ポケモンアイコンカーソルの開始位置
  POKEICON_CURSOR_POS_Y   = 10,   // ポケモンアイコンカーソルの開始位置
  POKEICON_CURSOR_POS_W   = POKEICON_MARGIN_W/8,   // カーソルの開始位置
  POKEICON_CURSOR_POS_H   = POKEICON_MARGIN_Y/8,   // カーソルの開始位置
  POKEICON_CURSOR_ON_PALID    = BLINK_PALID_POKE_CURSOR_CHANGE,
  POKEICON_CURSOR_OFF_PALID   = BLINK_PALID_POKE_CURSOR_START,
  POKEICON_CURSOR_NONE_PALID  = 0xB,

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

// 地形アイコンID
typedef enum{
  GRA_LANDICON_TYPE_LAND,
  GRA_LANDICON_TYPE_WATERSIDE,
  GRA_LANDICON_TYPE_FISHING,
}GRA_LANDICON_TYPE;

// 地形アイコンステータス
typedef enum{
  GRA_LANDICON_ON,
  GRA_LANDICON_OFF,
  GRA_LANDICON_NONE,
}GRA_LANDICON_STATUS;

// カーソルの選択状態
typedef enum{
  GRA_POKEICON_CURSOR_ON,
  GRA_POKEICON_CURSOR_OFF,
  GRA_POKEICON_CURSOR_NONE,
}GRA_POKEICON_CURSOR_STATUS;

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

// ゾーン詳細画面ワーク:グラフィック
typedef struct {
  // パレットフェード　-------------------------------
  PALETTE_FADE_PTR  p_pfd;
  BLINKPALANM_WORK  *p_blinkanm_wk; // 点滅用
	
  // セルアクターユニット
  GFL_CLUNIT			*p_clunit;
  
  // ポケアイコン ------------------------------------
  UI_EASY_CLWK_RES  clres_poke_icon[HABITAT_ZONE_POKE_LIST_MAX];
  GFL_CLWK          *p_clwk_poke_icon[HABITAT_ZONE_POKE_LIST_MAX];
  
  // 共通OBJリソース
  UI_EASY_CLWK_RES  clres_common_obj;
  GFL_CLWK          *p_clwk_common_obj[COMMON_OBJ_WK_MAX];
  
  // 共通素材
  UI_EASY_CLWK_RES  clres_app_menu_obj;
  
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
extern BOOL HABITAT_ZONE_GRA_CheckPrintUtilStatus( HABITAT_ZONE_GRA_WK *p_wk );
extern void HABITAT_ZONE_GRA_DrawStr( PRINT_UTIL_WIN win_id, const STRBUF* str,
    HABITAT_ZONE_GRA_WK *p_wk,s16 x,s16 y,s16 w, s16 h ,PRINTSYS_LSB font_colro );
extern void HABITAT_ZONE_GRA_BmpFil( PRINT_UTIL_WIN win_id ,u8 col_code ,HABITAT_ZONE_GRA_WK *p_wk );
extern GFL_CLWK* HABITAT_ZONE_GRA_GetComClWk(
                                        HABITAT_ZONE_GRA_WK *p_wk ,s32 obj_id );
extern void HABITAT_ZONE_GRA_SetLnadTypeIcon( HABITAT_ZONE_GRA_WK *p_wk ,
                      GRA_LANDICON_TYPE icon_type, GRA_LANDICON_STATUS status );
extern void HABITAT_ZONE_GRA_SetPokeIcon( HABITAT_ZONE_GRA_WK *p_wk ,
                                                          u16 monsno ,u8 index );
extern void HABITAT_ZONE_GRA_DeletePokeIcon( HABITAT_ZONE_GRA_WK *p_wk ,u8 index );
extern void HABITAT_ZONE_GRA_SetCursorStatus( HABITAT_ZONE_GRA_WK *p_wk ,
                                    u8 index ,GRA_POKEICON_CURSOR_STATUS status );
extern void HABITAT_ZONE_GRA_SetPokeAnmFlag( HABITAT_ZONE_GRA_WK *p_wk ,
                                                          u8 index ,BOOL is_anm );
extern void HABITAT_ZONE_GRA_ClearWin( PRINT_UTIL_WIN win_id ,HABITAT_ZONE_GRA_WK *p_wk ,s16 x, s16 y ,s16 w, s16 h );


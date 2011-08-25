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

#include "habitat_zone.h"

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

// アプリ共通リソース
#include "app/app_menu_common.h"

// タウンマップデータ
#include "app/townmap_data_sys.h"

// ゲームデータ
#include "gamesystem/game_data.h"

// OBJ初期位置
enum
{
  OBJ_POS_L_X = 24,
  OBJ_POS_L_Y = 60,
  OBJ_POS_R_X = 80,
  OBJ_POS_R_Y = 60,
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
  OBJ_ANM_SEQ_STAMP_MIDDLE_GET    = 7,
  OBJ_ANM_SEQ_STAMP_MIDDLE_LOOK   = 8,
  OBJ_ANM_SEQ_STAMP_SMALL_GET     = 9,
  OBJ_ANM_SEQ_STAMP_SMALL_LOOK    = 10,
  OBJ_ANM_SEQ_BUTTON_LAND         = 16,
  OBJ_ANM_SEQ_BUTTON_WATERSIDE    = 17,
  OBJ_ANM_SEQ_BUTTON_FISHING      = 18,
  OBJ_ANM_SEQ_BUTTON_LAND_ANM     = 19,
  OBJ_ANM_SEQ_BUTTON_WATERSIDE_ANM= 20,
  OBJ_ANM_SEQ_BUTTON_FISHING_ANM  = 21,
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
  OBJ_WK_MAP_BLINK,
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
  SMALL_ICON_OFFSET_X   = 16,    // ポケモンアイコン用見た、捕獲アイコンオフセット
  SMALL_ICON_OFFSET_Y   = 0,   // ポケモンアイコン用見た、捕獲アイコンオフセット

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
  BGS_FRAME_ROOT        = GFL_BG_FRAME3_S,
}BG_FRAME;

// ==================================
// プリントユーティリティー準備
// ==================================
typedef enum
{
  PRINT_UTIL_INFO_WIN,
  PRINT_UTIL_MAP_NAME,
  PRINT_UTIL_MAX,
}PRINT_UTIL_WIN;

// 地形アイコンID(HABITAT_ZONE_APPEARの並びと同一)
typedef enum{
  GRA_LANDICON_TYPE_LAND,
  GRA_LANDICON_TYPE_WATERSIDE,
  GRA_LANDICON_TYPE_FISHING,
  GRA_LANDICON_TYPE_MAX,
}GRA_LANDICON_TYPE;

// 地形アイコンステータス
typedef enum{
  GRA_LANDICON_ON,
  GRA_LANDICON_OFF,
  GRA_LANDICON_NONE,
}GRA_LANDICON_STATUS;

// 見た捕獲アイコン小の状態
typedef enum{
  GRA_SMALLICON_GET,
  GRA_SMALLICON_LOOK,
  GRA_SMALLICON_NONE,
}GRA_SMALLICON_STATUS;

// 見た捕獲アイコン小の状態
typedef enum{
  GRA_MIDDLEICON_GET,
  GRA_MIDDLEICON_LOOK,
  GRA_MIDDLEICON_NONE,
}GRA_MIDDLEICON_STATUS;

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
  // ゲームデータ
  GAMEDATA *p_gdata;

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
  
  // 上画面ルートオブジェクト
  UI_EASY_CLWK_RES  clres_root_obj;
  
  // 文字ウィンドウ---------------------
  BOOL              is_print_util_wati;         // 転送待ち
	GFL_FONT			    *p_font;
  PRINT_UTIL        print_util[PRINT_UTIL_MAX]; // プリントユーティリティー
  PRINT_QUE         *p_print_que;
  GFL_BMPWIN        *p_font_win;
  GFL_BMPWIN        *p_font_win_sub;
  
  // 背景スクロール---------------------
  u32 back_bg_scroll_wait_count;

  // タウンマップデータ ----------------
  TOWNMAP_DATA*     p_townmap_data;
  
  // アルファアニメーション -----------
  s32               obj_exist_ev1;
  u16               obj_exist_alpha_anime_count;

}HABITAT_ZONE_GRA_WK;

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern void HABITAT_ZONE_GRA_Init( HABITAT_ZONE_GRA_WK *p_wk ,GAMEDATA *p_gdata );
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
extern void HABITAT_ZONE_GRA_SetMapBlink( HABITAT_ZONE_GRA_WK *p_wk ,u16 zone_id );
extern void HABITAT_ZONE_GRA_SetSmallIconStatus( HABITAT_ZONE_GRA_WK* p_wk,
                                          u16 index ,GRA_SMALLICON_STATUS status);
extern void HABITAT_ZONE_GRA_SetMiddleIconStatus( HABITAT_ZONE_GRA_WK* p_wk,
                      GRA_LANDICON_TYPE icon_type ,GRA_MIDDLEICON_STATUS status);


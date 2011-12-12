//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		habitat_list_gra.h
//   @brief   生息地リスト：リスト画面グラフィックモジュール
//   @author	Masayuki Onoue
//   @date		2011.08.22
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include <gflib.h>

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

// ゲームデータ
#include "gamesystem/game_data.h"

// フレームリスト
#include "ui/frame_list.h"

// フレームリスト
#include "habitat_list.h" // for ZUKANHABITAT_LIST_LISTDATA

//=============================================================================
//
//					定数宣言
//
//=============================================================================
// 定数
enum
{
  PLACE_DATA_MAX       = 100,    // リスト登録数がこの数を超えない必要がある
  LIST_ICON_NUM        = 3,      // リスト1本につくアイコン数
  LIST_BAR_NUM         = 9,      // バーの総数
  LIST_ICON_MAX        = (LIST_BAR_NUM * LIST_ICON_NUM),
  LIST_ICON_X_OFFSET   = 12,       // リストアイコンのXオフセット値
  LIST_ICON_Y_OFFSET   = 12,      // リストアイコンのYオフセット値
  LIST_ICON_SET_W      = 16,       // アイコンの幅
  BLINK_PAL_NO_01      = 5,       // 点滅用パレット(選択時)
  BLINK_PAL_NO_02      = 7,       // 点滅用パレット(選択時)
  BLINK_PAL_NO_01_CMP  = 6,       // 点滅用パレット(選択時)
  BLINK_PAL_NO_02_CMP  = 7,       // 点滅用パレット(選択時)
  BLINK_PAL_NO_SELECT  = 7,       // 点滅用パレット(選択時)
  LIST_BAR_H           = 24,      // リストカーソルの高さ
  LIST_BAR_POS_X       = 116,     // リストカーソルX位置

  BLINK_PALID_POKE_CURSOR_CHANGE  = 0x1,  // 点滅(書き換え対象)
  BLINK_PALID_POKE_CURSOR_START   = 0x1,  // 点滅用パレット番号(基準)
  BLINK_PALID_POKE_CURSOR_END     = 0x2,  // 滅用パレット番号(終了)
  BLINK_PAL_SIZE_POKE_CURSOR      = 0x5,    // 点滅させるパレットの数

};

// OBJWK定義
enum
{
  LIST_ICON_CLWKID_START,
  LIST_ICON_CLWKID_END = (LIST_ICON_CLWKID_START + LIST_ICON_MAX),
  LIST_ICON_CLWKID_START_S,
  LIST_ICON_CLWKID_END_S = (LIST_ICON_CLWKID_START_S + LIST_ICON_MAX),
  LIST_ICON_CLWKID_START_SW,
  LIST_ICON_CLWKID_END_SW = (LIST_ICON_CLWKID_START_SW + LIST_ICON_MAX),
  MINIMAP_CLWKID,
  MODE_BUTTON_01_CLWKID,
  MODE_BUTTON_02_CLWKID,
  MODE_BUTTON_03_CLWKID,
  SCROLL_BUTTON_CLWKID,
  SCROLL_BAR_CLWKID,
  LIST_CURSOR_CLWKID,

  // ここから下はスクロール
  SELECT_BUTTON_CLWKID,
  L_BUTTON_CLWKID,
  R_BUTTON_CLWKID,
  CHECKBOX_CLWKID,
  RETURN_BUTTON_CLWKID,
  EXIT_BUTTON_CLWKID,
  COMMON_OBJ_WK_MAX,

  M_SCROLL_OBJ_START = SELECT_BUTTON_CLWKID,
  M_SCROLL_OBJ_END = EXIT_BUTTON_CLWKID,
};

// アニメーションシーケンス
enum
{
  OBJ_ANM_SEQ_SCROLL_BUTTON       = 2,    // スクロールボタン
  OBJ_ANM_SEQ_SCROLL_BAR          = 3,    // スクロールバー
  OBJ_ANM_SEQ_SELECT_BUTTON       = 1,    // セレクトボタン
  OBJ_ANM_SEQ_SELECT_BUTTON_ANM   = 5,    // セレクトボタン(アニメ)
  OBJ_ANM_SEQ_MODE_BUTTON     = 13,   // モード切替ボタン
  OBJ_ANM_SEQ_MODE_BUTTON_ANM = 14,   // モード切替ボタン（アニメ）
  OBJ_ANM_SEQ_MODE_BUTTON_SELECT_ANM = 42,   // モード切替ボタン（選択アニメ）
  OBJ_ANM_SEQ_MINIMAP     = 15, // ミニマップ
  OBJ_ANM_SEQ_MINIMAP_ANM = 40, // ミニマップ(アニメーション)
  OBJ_ANM_SEQ_NEW_ICON    = 12, // 新着アイコン
  OBJ_ANM_SEQ_EX_ICON     = 11, // びっくりマーク
  OBJ_ANM_SEQ_GET_ICON    = 7,  // 捕獲アイコン
  OBJ_ANM_SEQ_LOOK_ICON   = 8,  // 見たアイコン
  
  OBJ_ANM_SEQ_LIST_CURSOR01         = 32,    // リストカーソル
  OBJ_ANM_SEQ_LIST_CURSOR02         = 34,    // リストカーソル
  OBJ_ANM_SEQ_LIST_CURSOR03         = 36,    // リストカーソル
};

// 定数定義
enum{
  HABITAT_LIST_POKE_LIST_MAX  = 15,  // ゾーン詳細画面のポケモンデータ最大数
  HABITAT_LIST_UNIT_CLWK_MAX  = HABITAT_LIST_POKE_LIST_MAX + COMMON_OBJ_WK_MAX,
  MINI_MAPX_POS_X         = 32,  // ミニマップのの表示位置X
  MINI_MAPX_POS_Y         = 64,  // ミニマップのの表示位置Y
  MODE_BUTTON_01_POS_X    = 32,  // モード切替ボタン位置X
  MODE_BUTTON_01_POS_Y    = 110, // モード切替ボタン位置Y
  MODE_BUTTON_02_POS_X    = 32,  // モード切替ボタン位置X
  MODE_BUTTON_02_POS_Y    = 133, // モード切替ボタン位置Y
  MODE_BUTTON_03_POS_X    = 32,  // モード切替ボタン位置X
  MODE_BUTTON_03_POS_Y    = 156, // モード切替ボタン位置Y
  SELECT_BUTTON_POS_X     = 32,  // セレクトボタン位置X
  SELECT_BUTTON_POS_Y     = 180, // セレクトボタン位置Y
  BUTTON_LEFT_ARROW_X     = 132, // 左矢印
  BUTTON_LEFT_ARROW_Y     = 168, // 左矢印
  BUTTON_RIGHT_ARROW_X    = 156, // 右矢印
  BUTTON_RIGHT_ARROW_Y    = 168, // 右矢印
  BUTTON_CHECK_X          = 184, // チェックボックス
  BUTTON_CHECK_Y          = 172, // チェックボックス
  BUTTON_CANCEL_X         = 208, // キャンセル
  BUTTON_CANCEL_Y         = 168, // キャンセル
  BUTTON_RETURN_X         = 232, // 戻る
  BUTTON_RETURN_Y         = 168, // 戻る
  SCROLL_BAR_POS_X        = 252,  // スクロールバーX(SCROLL_BAR_PXを参考にした)
  SCROLL_BAR_POS_Y        = 84,  // スクロールバーY

};

//-------------------------------------
///	BG面定義
//=====================================
typedef enum _BG_FRAME{
  BGM_FRAME_FONT        = GFL_BG_FRAME0_M,
  BGM_FRAME_MENUBAR     = GFL_BG_FRAME1_M,
  BGM_FRAME_LIST        = GFL_BG_FRAME2_M,
  BGM_FRAME_SCROLL_WIN  = GFL_BG_FRAME3_M,
  BGS_FRAME_FONT        = GFL_BG_FRAME0_S,
  BGS_FRAME_BAR         = GFL_BG_FRAME1_S,
  BGS_FRAME_LIST        = GFL_BG_FRAME2_S,
  BGS_FRAME_SCROLL_WIN  = GFL_BG_FRAME3_S,
}BG_FRAME;

// ==================================
// プリントユーティリティー準備
// ==================================
typedef enum
{
  PRINT_UTIL_INFO_WIN,
  PRINT_UTIL_TITLE_WIN,
  PRINT_UTIL_MAX,
}PRINT_UTIL_WIN;

// ==================================
// リスト背景定義ID
// ==================================
typedef enum
{
  FLIST_BACK_L,     // 草地だけ
  FLIST_BACK_W,     // 水辺だけ
  FLIST_BACK_LW,    // 草地、水辺
  FLIST_BACK_WF,    // 水辺、釣り
  FLIST_BACK_LWF,   // 草地、水辺、釣り
  FLIST_BACK_L_CMP,     // 草地だけ(コンプリート)
  FLIST_BACK_W_CMP,     // 水辺だけ(コンプリート)
  FLIST_BACK_LW_CMP,    // 草原、釣り(コンプリート)
  FLIST_BACK_WF_CMP,    // 水辺、釣り(コンプリート)
  FLIST_BACK_LWF_CMP,   // 草地、水辺、釣り(コンプリート)
  FLIST_BACK_TYPE_MAX,
  FLIST_BACK_NORMAL_MAX   = FLIST_BACK_LWF+1,
  FLIST_BACK_COMP_OFFSET  = FLIST_BACK_NORMAL_MAX, // コンプリート時のOOFFSET
}FLIST_BACK_TYPE;

//=============================================================================
//
//					構造体宣言
//
//=============================================================================
// アイコン状態データ
typedef struct {
  BOOL draw_flag;
  BOOL is_refresh;
  u16 anm_seq;
  GFL_CLACTPOS pos;
}GRA_ICON_DATA;

// ゾーン詳細画面ワーク:グラフィック
typedef struct {
  // ゲームデータ
  GAMEDATA *p_gdata;

  // パレットフェード　-------------------------------
  PALETTE_FADE_PTR  p_pfd;
  BLINKPALANM_WORK  *p_blinkanm_wk; // 点滅用
  BOOL              is_stop_blink;
	
  // セルアクターユニット
  GFL_CLUNIT			*p_clunit;
  
  // 共通OBJリソース
  UI_EASY_CLWK_RES  clres_common_obj;
  UI_EASY_CLWK_RES  clres_common_obj_s;
  
  // 共通素材
  UI_EASY_CLWK_RES  clres_app_menu_obj;
  
  // セルアクターワーク
  GFL_CLWK          *p_clwk_common_obj[COMMON_OBJ_WK_MAX];
  
  // 文字ウィンドウ---------------------
  BOOL              is_print_util_wati;         // 転送待ち
	GFL_FONT			    *p_font;
  PRINT_UTIL        print_util[PRINT_UTIL_MAX]; // プリントユーティリティー
  PRINT_QUE         *p_print_que;
  GFL_BMPWIN        *p_font_win;
  GFL_BMPWIN        *p_font_win_sub;
  
  // 背景スクロール---------------------
  u32 back_bg_scroll_wait_count;

  // リスト -----------
  u8              list_size;
  u32             list_pos_max;
  FRAMELIST_WORK  *p_lwk;		// リストワーク
  STRBUF          *p_place_name[PLACE_DATA_MAX];
  ZUKANHABITAT_LIST_LISTDATA *p_list_data;
  GRA_ICON_DATA icon_data_main[ LIST_ICON_MAX ];
  GRA_ICON_DATA icon_data_sub[ LIST_ICON_MAX ];
  BOOL list_update;

}HABITAT_LIST_GRA_WK;

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern void HABITAT_LIST_GRA_Init( HABITAT_LIST_GRA_WK *p_wk ,GAMEDATA *p_gdata );
extern void HABITAT_LIST_GRA_CallFade( HABITAT_LIST_GRA_WK *p_wk, u8 start, u8 end );
extern void HABITAT_LIST_GRA_Exit( HABITAT_LIST_GRA_WK *p_wk );
extern void HABITAT_LIST_GRA_VblankMain( HABITAT_LIST_GRA_WK *p_wk );
extern void HABITAT_LIST_GRA_HblankMain( HABITAT_LIST_GRA_WK *p_wk );
extern void HABITAT_LIST_GRA_Main( HABITAT_LIST_GRA_WK *p_wk );
extern BOOL HABITAT_LIST_GRA_CheckPrintUtilStatus( HABITAT_LIST_GRA_WK *p_wk );
extern void HABITAT_LIST_GRA_DrawStr( PRINT_UTIL_WIN win_id, const STRBUF* str,
    HABITAT_LIST_GRA_WK *p_wk,s16 x,s16 y,s16 w, s16 h ,PRINTSYS_LSB font_colro );
extern void HABITAT_LIST_GRA_BmpFil( PRINT_UTIL_WIN win_id ,u8 col_code ,HABITAT_LIST_GRA_WK *p_wk );
extern GFL_CLWK* HABITAT_LIST_GRA_GetComClWk(
                                        HABITAT_LIST_GRA_WK *p_wk ,s32 obj_id );
extern void HABITAT_LIST_GRA_ClearWin( PRINT_UTIL_WIN win_id ,HABITAT_LIST_GRA_WK *p_wk ,s16 x, s16 y ,s16 w, s16 h );
extern BOOL HABITAT_LIST_GRA_FrameListInit( HABITAT_LIST_GRA_WK *p_wk );
extern void HABITAT_LIST_GRA_MakeList( HABITAT_LIST_GRA_WK* p_wk, u8 list_size,
    ZUKANHABITAT_LIST_LISTDATA *p_list_data ,u16 default_zone_group,u16 initPos);
extern void HABITAT_LIST_GRA_DeleteList( HABITAT_LIST_GRA_WK* p_wk );
extern u32 HABITAT_LIST_GRA_ListMain( HABITAT_LIST_GRA_WK *p_wk );
extern void HABITAT_LIST_GRA_GetScrollBarYPos( HABITAT_LIST_GRA_WK *p_wk );
extern void HABITAT_LIST_GRA_RecoverDrawFlag( HABITAT_LIST_GRA_WK *p_wk );
extern u32 HABITAT_LIST_GRA_GetListPos( HABITAT_LIST_GRA_WK *p_wk );
extern void HABITAT_LIST_GRA_ChageSelectItemPalette( HABITAT_LIST_GRA_WK *p_wk ,u8 pal );
extern u32 HABITAT_LIST_GRA_GetScrollCount( HABITAT_LIST_GRA_WK *p_wk );
extern void HABITAT_LIST_GRA_SetCursorPos( HABITAT_LIST_GRA_WK *p_wk ,u32 cursor_pos );
extern ZUKANHABITAT_LIST_LISTDATA* HABITAT_LIST_GRA_GetListData(
                                  HABITAT_LIST_GRA_WK *p_wk ,u32 list_index );
extern void HABITAT_LIST_GRA_SetAnmSeq( HABITAT_LIST_GRA_WK *p_wk ,s32 clwkid ,u32 anm_seq );
extern u32 HABITAT_LIST_GRA_GetCursorPos( HABITAT_LIST_GRA_WK *p_wk );
extern BOOL HABITAT_LIST_GRA_GetCanListDown( HABITAT_LIST_GRA_WK *p_wk );
extern BOOL HABITAT_LIST_GRA_CheckAnmActive( HABITAT_LIST_GRA_WK *p_wk ,s32 clwkid );
extern u16 HABITAT_LIST_GRA_GetAnmSeq( HABITAT_LIST_GRA_WK *p_wk ,s32 clwkid );
extern u16 HABITAT_LIST_GRA_CheckFade( HABITAT_LIST_GRA_WK *p_wk );
extern void HABITAT_LIST_GRA_AddScrollFrame( HABITAT_LIST_GRA_WK *p_wk ,s32 ypos,BOOL is_main  );
extern void HABITAT_LIST_GRA_StartMiniMapIconAnm( HABITAT_LIST_GRA_WK *p_wk );
extern void HABITAT_LIST_GRA_SetBlinkStop( HABITAT_LIST_GRA_WK *p_wk ,BOOL is_stop );


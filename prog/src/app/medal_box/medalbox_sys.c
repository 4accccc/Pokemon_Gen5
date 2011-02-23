
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medalbox_sys.c
 *	@brief  メダルボックスメイン
 *	@author	Masayuki Onoue
 *	@data		2011.01.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "system/gfl_use.h"       // GFUser_VIntr_CreateTCB
#include "system/main.h"          // HEAPID
#include "arc_def.h"              // ARC_ID
#include "message.naix"           // MESSAGE_ARC_ID

#include "sound/pm_sndsys.h"      // サウンド

#include "print/gf_font.h"        // フォント
#include "font/font.naix"         // フォント
#include "print/wordset.h"        // ワードセット

#include "medalbox_gra.naix"      // グラフィックアーカイブ

#include "app_menu_common.naix"   // 共通素材
#include "app/app_menu_common.h"  // 共通素材

#include "msg/msg_medalname.h"    // for MDL_NAME_HINT_000
#include "msg/msg_medalbox.h"     // for MSG_MEDALBOX_～

#include "app/medalbox_sys.h"
#include "medalbox_data_sys.h"
#include "medalbox_sys_local.h"

#include "gamesystem/msgspeed.h"      // MSGSPEED_GetWait
#include "app/app_keycursor.h"        // プリントカーソル
#include "app/app_printsys_common.h"  // プリントシステム

#include "app/app_taskmenu.h"         // アプリケーションタスクメニュー
#include "system/bmp_winframe.h"      // BMPウィンドウフレーム

#include "system/blink_palanm.h"      // カーソルアニメーション


FS_EXTERN_OVERLAY(ui_common);         // オーバーレイ

//============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//-------------------------------------
///	ヒープサイズ
//=====================================
#define MEDALBOXSYS_HEAP_SIZE (0x60000)

//-------------------------------------
///	リソース初期位置
//=====================================

#define UI_CAP_POS_X     (256/2)
#define UI_CAP_POS_Y     (192/2)

#define UI_RETURN_POS_X     (256-24)
#define UI_RETURN_POS_Y     (192-24)

#define UI_EXIT_POS_X   (256-24*2)
#define UI_EXIT_POS_Y   (192-24)

#define UI_SCROLBER_POS_X (122-6)
#define UI_SCROLBER_POS_Y (180)

#define UI_SCROLBTN_POS_X (UI_SCROLBER_POS_X - 7*8)
#define UI_SCROLBTN_POS_Y (180)

//#define UI_CURSOR_RIGHT_POS_X (256-24*3-12+4)
#define UI_CURSOR_RIGHT_POS_X (176)
#define UI_CURSOR_RIGHT_POS_Y (192-24)

#define UI_CURSOR_LEFT_POS_X  (32)
//#define UI_CURSOR_LEFT_POS_X  (24 + 12 -4)
#define UI_CURSOR_LEFT_POS_Y  (192-24)

#define UI_SEARCH_POS_X   (0)
#define UI_SEARCH_POS_Y   (192-24)

//-------------------------------------
///	パレット定義
//=====================================
#define MEDALOBJ_PALETTENUM_MEDAL       (6)       // メダルOBJのパレット使用本数
#define MEDALOBJ_PALETTENUM_UI          (4)       // UIOBJのパレット使用本数
#define MEDALOBJ_PALETTENUM_COMMON_UI   (4)       // COMMONUIOBJのパレット使用本数
#define PAGEUNIT_PAL                    (0)       // ページユニットのデフォルトパレット
#define PUSSHMEDAL_NUMBER_PAL           (8)       // イチオシボタン時の文字色変更用パレット

#define FONTBORD_COLOR_OFFSET           (1)       // 文字盤カラーが入っている、パレットの開始位置

// MAIN
#define SYSTEM_FONT_PAL                 (11)      // フォント用パレット
#define APP_WINFRAME_USE_PALETE         (12)      // ウィンドウフレーム用パレット
#define APP_TASKMENU_USE_PALETE         (13)      // アプリケーションタスクメニューが使用するパレット(2本)
#define MEDAL_NAME_BMPWIN_PAL_NUM       (15)      // メダル名BMPWIN用パレットナンバー

// SUB
#define MEDAL_INFO_BMPWIN_PAL_NUM       (15)      // メダル情報BMPWIN用パレットナンバー


//-------------------------------------
///	アプリ設定
//=====================================
#define MEDALBOX_DRAW_PAGE_NUM_LIST     (5)       // ボックスモードが一覧の時に表示するページ数
#define MEDALBOX_DRAW_PAGE_NUM_NORMAL   (3)       // ボックスモードがノーマルに表示するページ数

#define FONTBORD_WIDTH                  (184)     // 文字盤の幅
#define CAP_WIDTH                       (80)      // フタの幅
#define PAGE_WIDTH                      (80)      // ページの幅
#define MONITOR_HEIGHT                  (192)     // モニター全体の高さ
#define MONITOR_TOP_HEIGHT              (32)      // モニターの飛び出している部分の高さ

#define NAME_BMPWIN_MAX                 (5)       // メダル名表示用BMPWINの数
#define MEDAL_NAME_BMPWIN_SIZE_W        (15)      // メダル名表示領域の幅（キャラクタ単位)
#define MEDAL_NAME_BMPWIN_SIZE_H        (3)       // メダル名表示領域の高さ（キャラクタ単位)
#define MEDAL_NAME_BMPWIN_STR_COL_L     (2)       // 文字(Letter)
#define MEDAL_NAME_BMPWIN_STR_COL_S     (3)       // 影(Shadow)
#define MEDAL_NAME_BMPWIN_STR_COL_B     (1)       // 背景色(Background)

#define MEDAL_INFO_TITLE_DRAW_X         (8)       // タイトル表示位置X
#define MEDAL_INFO_TITLE_DRAW_Y         (8)       // タイトル表示位置Y

#define MEDAL_INFO_INFO_DRAW_X          (20)      // 説明文表示位置X
#define MEDAL_INFO_INFO_DRAW_Y          (128)     // 説明文表示位置Y

#define MEDAL_INFO_GETTYPE_DRAW_X       (232)     // 取得タイプ表示位置
#define MEDAL_INFO_GETTYPE_DRAW_Y       (80)      // 取得タイプ表示位置

#define MEDAL_INFO_DAYTE_DRAW_X         (232)     // 日付
#define MEDAL_INFO_DAYTE_DRAW_Y         (96)      // 日付

#define MEDAL_INFO_NUMERATOR_DRAW_X     (232-32)  // 分子
#define MEDAL_INFO_NUMERATOR_DRAW_Y     (8)      // 分子

#define MEDAL_INFO_SLASH_DRAW_X         (232-24)  // スラッシュ
#define MEDAL_INFO_SLASH_DRAW_Y         (8)      // スラッシュ

#define MEDAL_INFO_DENOMINATOR_DRAW_X   (232)     // 分母
#define MEDAL_INFO_DENOMINATOR_DRAW_Y   (8)      // 分母

#define MEDAL_INFO_NAME_DRAW_X          (232)     // メダル名表示位置X
#define MEDAL_INFO_NAME_DRAW_Y          (48)      // メダル名表示位置Y

#define MEDAL_INFO_BMPWIN_STR_COL_L     (1)       // 文字(Letter)
#define MEDAL_INFO_BMPWIN_STR_COL_S     (2)       // 影(Shadow)、
#define MEDAL_INFO_BMPWIN_STR_COL_B     (0)       // 背景色(Background)

#define MEDAL_NAME_DRAW_PAGE            (1)       // メダル名を表示するページ

#define UP_MEDAL_POS_X                  (44)      // 上画面に表示するメダルの位置
#define UP_MEDAL_POS_Y                  (70)      // 上画面に表示するメダルの位置

#define POLISH_EFFECT_SCROLL_SPEED      (10)      // 光沢エフェクトのスクロールスピード
#define POLISH_EFFECT_SCROLL_SIZE       (512)     // 光沢エフェクトがスクロールするサイズ

#define DIR_KEY_SCROLL_SPEED    (0x7000)    // ページスクロールスピード
#define DIR_KEY_SCROLL_COUNT    (1)         // 方向キーでスクロールするページ数
#define LR_KEY_SCROLL_SPEED     (0x14000)   // ページスクロールスピード
#define LR_KEY_SCROLL_COUNT     (3)         // LRキーでスクロールするページ数
#define MEDAL_HIT_RADIUS        (16)        // メダルの半径（当たり判定に使用)
#define AUTO_SCROLL_SPEED_MAX   (0x20000)   // 自動スクロール最高速度
#define AUTO_SCROLL_SPEED_MIN   (0x9000)    // 自動スクロール最低速度
#define AUTO_SCROLL_SPEED_UP    (0x2000)    // 自動スクロール加速値
#define AUTO_SCROLL_SPEED_DOWN  (0x1500)    // 自動スクロール減速値

#define AUTO_SCROLL_SPEED       (0x20000)   // ページスクロールスピード
#define SCROLLBAR_LENGTH        (112)       // スクロールバーの長さ

#define MENU_NO_USE             (msg_medalbox_max)       // 使用しないメニュー用ID
#define SEARCH_MENU_MAX         (8)         // メニューの最大数(ソートのタイプ＋やめる、モードチェンジ)

#define PUSHMEDAL_BUTTON_MAX    (5)         // 一押し登録ボタンの数
#define PUSHMEDAL_BUTTON_H      (32)        // 一押しボタンの高さ
#define PUSHMEDAL_BUTTON_W      (32)        // 一押しボタンの幅
#define PUSUMEDAL_INFO_FRAME    (60*3)      // イチオシ登録確認メッセージの表示時間
#define NEW_EFFECT_RANGE        (26)        // 新品エフェクトの出る範囲
#define NEW_EFFECT_FREQUENTLY   (10)         // 新エフェクトの出る頻度(大きいほど出にくい)
#define CURSOR_BRINK_SPEED      (4)         // カーソルが点滅している時の速度

#define FONTBORD_SCROLL_SPEED   (0x2333*4)
#define CAP_SCROLL_SPEED        (0x1000*4)
#define MONITORE_SCROLL_SPEED   (0x5000)
#define SCROLL_CALIBRATION      (3)         // スクロールバーでの位置誤差を無視する範囲

#define SLIDE_MOVE_START_LENGTH   (20)      // この値だけタッチスライドを行うと、ページスクロール
#define SPEED_GAIN_PAGE           (5)       // スライドバータッチで加速を始めるページ枚数
#define NORMAL_MOVE_UNITPOS_RIGHT (1)       // ノーマルモード時に、右ページ移動を行うユニット位置
#define LIST_MOVE_UNITPOS_RIGHT   (3)       // リストモード時に、右ページ移動を行うユニット位置

#define HIT_STOP_FRAME            (6)       // フタがぶつかった時演出
#define POLIS_RESTART_FRAME       (100)      // 光沢が表示された後に待機する時間(最低値)
#define POLIS_RESTART_FRAME_RAND  (40)      // 光沢を再開するまでの時間にランダムで追加される時間(0~POLIS_RESTART_FRAME)

//-------------------------------------
///	VRAM設定
//=====================================
static const GFL_DISP_VRAM sc_vramSetTable =
{
	GX_VRAM_BG_128_A,					  // メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,				// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_128_B,					// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_NONE,						// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_NONE,				// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_128K,		
	GX_OBJVRAMMODE_CHAR_1D_128K,		
};

//-------------------------------------
///	BGフレーム
//=====================================
typedef enum
{
	MEDALBOXSYS_BG_FRAME_FONTBORD_M,	// 文字盤+バー
	MEDALBOXSYS_BG_FRAME_ETC_M,		    // 光沢 or メッセージウィンドウ
	MEDALBOXSYS_BG_FRAME_PAGEUNIT_M,  // ページユニット
	MEDALBOXSYS_BG_FRAME_FONT_M,		  // 文字用

	MEDALBOXSYS_BG_FRAME_MONITOR_S,		// モニター
	MEDALBOXSYS_BG_FRAME_EFFECTS_S,	  // モニター演出
	MEDALBOXSYS_BG_FRAME_BACK_S ,	    // 背景
	MEDALBOXSYS_BG_FRAME_FONT_S,	    // 説明文表示用

	MEDALBOXSYS_BG_FRAME_MAX,			//ソース内で使用
}MEDALBOXSYS_GRAPHIC_BG_FRAME;

//-------------------------------------
///	BGプライオリティー
//=====================================
typedef enum
{
  MEDALBOXSYS_BG_PRIORITY_ETC_M         = 0,
  MEDALBOXSYS_BG_PRIORITY_FONT_M,
  MEDALBOXSYS_BG_PRIORITY_FONTBORD_M,
  MEDALBOXSYS_BG_PRIORITY_PAGEUNIT_M,
  MEDALBOXSYS_BG_PRIORITY_ETC_M_EFFETC  = 2,  // エフェクト面として動作している時のプライオリティー

  MEDALBOXSYS_BG_PRIORITY_EFFECTS_S     = 0,
  MEDALBOXSYS_BG_PRIORITY_FONT_S,
  MEDALBOXSYS_BG_PRIORITY_MONITOR_S,
  MEDALBOXSYS_BG_PRIORITY_BACK_S,
}MEDALBOXSYS_BG_PRIORITY;

//-------------------------------------
///	OBJプライオリティー(BG)
//=====================================
typedef enum
{
  MEDALBOXSYS_OBJ_PRIORITY_UI         = 2,
  MEDALBOXSYS_OBJ_PRIORITY_EFFECT     = 3,
  MEDALBOXSYS_OBJ_PRIORITY_MEDAL      = 3,
  MEDALBOXSYS_OBJ_PRIORITY_NEWEFFETC  = 3,
  MEDALBOXSYS_OBJ_PRIORITY_CURSOR     = 3,
  MEDALBOXSYS_OBJ_PRIORITY_UP_MEDAL   = 1,
}MEDALBOXSYS_OBJ_PRIORITY;


//-------------------------------------
///	OBJプライオリティー(ソフト)
//=====================================
typedef enum
{
  MEDALBOXSYS_OBJ_SOFT_PRIORITY_TOP,
  MEDALBOXSYS_OBJ_SOFT_PRIORITY_MEDIUM_TOP,
  MEDALBOXSYS_OBJ_SOFT_PRIORITY_MEDIUM,
  MEDALBOXSYS_OBJ_SOFT_PRIORITY_BOTTOM,
}MEDALBOXSYS_OBJ_SOFT_PRIORITY;

//-------------------------------------
///	CKWK
//=====================================
typedef enum
{
	MEDALBOXSYS_OBJ_CLWK_MEDAL1,
	MEDALBOXSYS_OBJ_CLWK_MEDAL2,
	MEDALBOXSYS_OBJ_CLWK_MEDAL3,
	MEDALBOXSYS_OBJ_CLWK_MEDAL4,
	MEDALBOXSYS_OBJ_CLWK_MEDAL5,
	MEDALBOXSYS_OBJ_CLWK_MEDAL6,
	MEDALBOXSYS_OBJ_CLWK_MEDAL7,
	MEDALBOXSYS_OBJ_CLWK_MEDAL8,
	MEDALBOXSYS_OBJ_CLWK_MEDAL9,
	MEDALBOXSYS_OBJ_CLWK_MEDAL10,
	MEDALBOXSYS_OBJ_CLWK_MEDAL11,
	MEDALBOXSYS_OBJ_CLWK_MEDAL12,
	MEDALBOXSYS_OBJ_CLWK_MEDAL13,
	MEDALBOXSYS_OBJ_CLWK_MEDAL14,
	MEDALBOXSYS_OBJ_CLWK_MEDAL15,
	MEDALBOXSYS_OBJ_CLWK_MEDAL16,
	MEDALBOXSYS_OBJ_CLWK_MEDAL17,
	MEDALBOXSYS_OBJ_CLWK_MEDAL18,
	MEDALBOXSYS_OBJ_CLWK_MEDAL19,
	MEDALBOXSYS_OBJ_CLWK_MEDAL20,
	MEDALBOXSYS_OBJ_CLWK_MEDAL21,
	MEDALBOXSYS_OBJ_CLWK_MEDAL22,
	MEDALBOXSYS_OBJ_CLWK_MEDAL23,
	MEDALBOXSYS_OBJ_CLWK_MEDAL24,
	MEDALBOXSYS_OBJ_CLWK_MEDAL25,
	MEDALBOXSYS_OBJ_CLWK_SCROLBTN,
	MEDALBOXSYS_OBJ_CLWK_SCROLBER,
	MEDALBOXSYS_OBJ_CLWK_CURSOR,
	MEDALBOXSYS_OBJ_CLWK_ICHIOSHI,
	MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0,
	MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_1,
	MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_2,
	MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_3,
	MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_4,
	MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_5,
	MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_6,
	MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_7,
	MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_8,
	MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_9,
	MEDALBOXSYS_OBJ_CLWK_CAP,
	MEDALBOXSYS_OBJ_CLWK_RETURN,
  MEDALBOXSYS_OBJ_CLWK_CURSOR_LEFT,
  MEDALBOXSYS_OBJ_CLWK_CURSOR_RIGHT,
	MEDALBOXSYS_OBJ_CLWK_EXIT,
	MEDALBOXSYS_OBJ_CLWK_SEARCH,

  MEDALBOXSYS_OBJ_CLWK_MEDAL1_OBJWIN,
  MEDALBOXSYS_OBJ_CLWK_MEDAL2_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL3_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL4_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL5_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL6_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL7_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL8_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL9_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL10_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL11_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL12_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL13_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL14_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL15_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL16_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL17_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL18_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL19_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL20_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL21_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL22_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL23_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL24_OBJWIN,
	MEDALBOXSYS_OBJ_CLWK_MEDAL25_OBJWIN,

	MEDALBOXSYS_OBJ_CLWK_UPMEDAL,         // 上画面用メダルOBJ
	MEDALBOXSYS_OBJ_CLWK_MAX,
	MEDALBOXSYS_OBJ_MEDAL_CLWK_MAX      = MEDALBOXSYS_OBJ_CLWK_MEDAL25 + 1 ,// メダルのセルID最大数
	MEDALBOXSYS_OBJ_MEDAL_OBJWIN_MAX    = MEDALBOXSYS_OBJ_CLWK_MEDAL25_OBJWIN - MEDALBOXSYS_OBJ_CLWK_MEDAL1_OBJWIN + 1 ,// OBJウィンドウの最大値
	MEDALBOXSYS_OBJ_UI_CLWK_MAX         = MEDALBOXSYS_OBJ_CLWK_CAP + 1,    // UIセルワークの管理ID
	MEDALBOXSYS_OBJ_COMMON_CLWK_MAX     = MEDALBOXSYS_OBJ_CLWK_EXIT + 1,    // コモンUIセルワークの管理ID
}MEDALBOXSYS_GRAPHIC_OBJ_CLWK ;

#define NEW_EFFETC_MAX      (MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_9-MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+1) // 新品エフェクトの最大数
#define MEDAL_HIT_TABLE_MAX (MEDALBOXSYS_OBJ_MEDAL_CLWK_MAX+1-PAGE_MEDAL_MAX-PAGE_MEDAL_MAX)  // 当たり判定テーブルの数(最初の5枚と最後の5枚には当たり判定がない)

//-------------------------------------
///	アーカイブタイプ
//=====================================
enum{
  ARC_TYPE_MEDAL_GRA,
  ARC_TYPE_COMMON_BUTTON_GRA,
  ARC_TYPE_MAX,
};

//-------------------------------------
///	BG面設定
//=====================================
static const struct 
{
	u32									frame;
	GFL_BG_BGCNT_HEADER	bgcnt_header;
	u32									mode;
}	sc_bgsetup[MEDALBOXSYS_BG_FRAME_MAX]	=
{	
	// MEDALBOXSYS_BG_FRAME_FONTBORD_M
	{	
		GFL_BG_FRAME0_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_FONTBORD_M, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_ETC_M
	{	
		GFL_BG_FRAME1_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_ETC_M, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_PAGEUNIT_M
	{	
		GFL_BG_FRAME2_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_PAGEUNIT_M, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_FONT_M
	{	
		GFL_BG_FRAME3_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x14000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_FONT_M, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},


	// MEDALBOXSYS_BG_FRAME_MONITOR_S
	{	
		GFL_BG_FRAME0_S,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_MONITOR_S, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_EFFECTS_S
	{	
		GFL_BG_FRAME1_S,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_EFFECTS_S, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_BACK_S
	{	
		GFL_BG_FRAME2_S,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_BACK_S, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_FONT_S
	{	
		GFL_BG_FRAME3_S,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_FONT_S, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
};

// ========================
// メダル座標
// ========================
static const struct
{
  s32 x;
  s32 y;
}	sc_medal_pos[PAGE_MEDAL_MAX]	=
{
  { 24      , 20        },
  { 24 + 24 , 20 + 32   },
  { 24      , 20 + 64   },
  { 24 + 24 , 20 + 96   },
  { 24      , 20 + 128  },
};

// ========================
// イチオシボタン座標
// ========================
static const struct
{
  s32 x;
  s32 y;
}	sc_ichioshi_pos[PAGE_MEDAL_MAX]	=
{
  { 224      , 24       },
  { 224 + 16 , 24 + 32  },
  { 224      , 24 + 64  },
  { 224 + 16 , 24 + 96  },
  { 224      , 24 + 128 },
};

// ========================
// メダル名表示ウィンドウ座標
// ========================
static const struct
{
  s32 x; // ドット単位
  s32 y; // ドット単位
}	sc_medalname_win_pos[NAME_BMPWIN_MAX]	=
{
  { 12*8 , 1*8+4 },
  { 14*8 , 5*8+4 },
  { 12*8 , 9*8+4 },
  { 14*8 , 13*8+4 },
  { 12*8 , 17*8+4 },
};

// ========================
// UI用当たり判定テーブル(トリガーボタン)
// ========================
static const GFL_UI_TP_HITTBL sc_ui_hittbl_trg[UI_HIT_TABLE_TRG_MAX] =
{
  //  UI_HIT_TABLE_TRG_SEARCH
  { 
    UI_SEARCH_POS_Y ,
    UI_SEARCH_POS_Y + 24,
    UI_SEARCH_POS_X ,
    UI_SEARCH_POS_X + 24,
  },
  //  UI_HIT_TABLE_TRG_CURSOR_LEFT
  { 
    UI_CURSOR_LEFT_POS_Y ,
    UI_CURSOR_LEFT_POS_Y + 24 ,
    UI_CURSOR_LEFT_POS_X ,
    UI_CURSOR_LEFT_POS_X + 24,
  }, 
  //  UI_HIT_TABLE_TRG_CURSOR_RIGHT
  { 
    UI_CURSOR_RIGHT_POS_Y ,
    UI_CURSOR_RIGHT_POS_Y + 24 ,
    UI_CURSOR_RIGHT_POS_X ,
    UI_CURSOR_RIGHT_POS_X + 24,
  }, 
  //  UI_HIT_TABLE_TRG_RETURN
  { 
    UI_RETURN_POS_Y ,
    UI_RETURN_POS_Y + 24 ,
    UI_RETURN_POS_X ,
    UI_RETURN_POS_X + 24 ,
  }, 
  //  UI_HIT_TABLE_TRG_EXIT
  { UI_EXIT_POS_Y,
    UI_EXIT_POS_Y + 24 ,
    UI_EXIT_POS_X,
    UI_EXIT_POS_X + 24 ,
  },
  // 終了コード
  { GFL_UI_TP_HIT_END , 0 , 0 , 0 },
};

// ========================
// ユニット上の数字位地（一桁目・キャラ単位)
// ========================
static const struct
{
  s32 x;
  s32 y;
}	sc_unit_number_pos[PAGE_MEDAL_MAX]	=
{
  { 8 , 2 },
  { 2 , 6 },
  { 8 , 10 },
  { 2 , 14 },
  { 8 , 18 },
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	メダルボックスメインワーク
//=====================================
typedef struct {

  // システム関連
	HEAPID heapID;
  MEDALBOXSYS_PARAM *p_param;
	GFL_TCB						*p_vblank_task;
	GFL_CLUNIT			  *p_clunit;
	GFL_CLWK				  *p_clwk[MEDALBOXSYS_OBJ_CLWK_MAX];
  u32							  obj_res[OBJRESID_MAX];
  GFL_BMPWIN        *p_name_win;
  GFL_BMPWIN        *p_info_win;
  GFL_BMPWIN        *p_menu_win;

  // メッセージ関連
	GFL_FONT			    *p_font;
	GFL_MSGDATA		    *p_app_msg;    // 共通
	GFL_MSGDATA		    *p_mdlname_msg;   // メダル
	GFL_MSGDATA		    *p_mdlinfo_msg;
	GFL_MSGDATA		    *p_mdlhelp_msg;

  // アプリ関連
  u8                sub_seq;
  MEDALBOXSYS_MODE  box_mode;       // メダルボックスモード
  MEDALBOXSYS_SEQ   now_seq;        // 現在のシーケンス
  MY_MEDALDATA      medal_data;     // メダルデータ
  u16               top_page_num;             // ページユニットの一番左端に表示するページ番号
  u16               modechange_top_page_num;  // モードチェンジ後に表示されるトップページ
  fx32              page_offset;    // ページオフセット
  SCROLL_DIR        scroll_dir;     // スクロール方向
  s32               scroll_count;   // スクロール回数
  u32               scroll_speed;   // スクロールスピード
  PAGEDATA          page[ MEDALBOX_DRAW_PAGE_NUM_LIST ];   // 表示しているページデータ
  u8                cursor_pos;         // カーソル位置
  u8                cursor_pos_tmp;     // 保存カーソル位置
  CURSOR_STATE      cursor_state;       // カーソル状態

  fx32              scroll_button_pos;  // スクロールバーの位置(初期座標からのオフセット)
  APP_TASKMENU_RES  *p_taskmenu_res;        // タスクメニュー用リソース
  APP_TASKMENU_ITEMWORK menulist[SEARCH_MENU_MAX];
  APP_TASKMENU_WORK *p_menu_wk;
  s32               menu_cnt;                 // メニューの数

  fx32              cap_offset;           // フォントボードの表示位置
  fx32              monitor_offset;       // フォントボードの表示位置
  fx32              fontbord_offset;      // フォントボードの表示位置

  u32               tmp_frame;            // サブシーケンス用フレーム数テンポラリ

  PUSHMEDAL_TYPE    pushmedal_set_type;   // イチオシボタンの設定方法

  s32               before_x;             // 1フレーム前のｘ座標
  s32               slide_length;         // スライド量
  BOOL              is_slide_touch;       // 画面をタッチしてのスライド中
  BLINKPALANM_WORK  *p_blinkanm_wk;       // カーソル点滅アニメーション           

  GFL_TCBLSYS               *p_tcb_sys;               // プリントシステム用タスク
  APP_PRINTSYS_COMMON_WORK  print_sys_wk;             // プリントシステムワーク
  PRINT_QUE                 *p_print_que;             // プリントキュー
  PRINT_STREAM              *p_print_stream;          // プリントストリーム
  APP_KEYCURSOR_WORK        *p_cursor;                // カーソル
  SCROLLEND_CURSOR_POS      scrollend_cursor_pos;     // スクロール終了時点でのカーソル位置

  BOOL        is_print_util_wati;                     // プリントユーティリティー終了待ち
  BOOL        is_polis_active;                        // 光沢がスクロール中かどうか
  s32         polis_frame;                            // 光沢用フレーム
  PRINT_UTIL  print_util[PRINT_UTIL_MAX];           // 文字列描画フレーム用ユーティリティー
#ifdef PM_DEBUG
  // デバッグ
  u8              debgu_seq;
#endif

}MEDALBOXSYS_WORK;

//-------------------------------------
///	BG面設定
//=====================================
static const struct 
{
  u32 msgid[SEARCH_MENU_MAX];
}	sc_menu_data[SORT_MODE_MAX]	=
{
  // SORT_MODE_DEFAULT
  {
    MENU_MSG_MODE_NORMAL,
    MENU_MSG_ONLY_TITLE,
    MENU_MSG_ONLY_HINT,
    MENU_MSG_TYPE,
    MENU_MSG_NAME,
    MENU_MSG_NEW,
    MENU_MSG_CANCEL,
    MENU_NO_USE,
  },
  // SORT_MODE_NEW
  {
    MENU_MSG_MODE_NORMAL,
    MENU_MSG_ONLY_TITLE,
    MENU_MSG_ONLY_HINT,
    MENU_MSG_TYPE,
    MENU_MSG_NAME,
    MENU_MSG_DEFAULT,
    MENU_MSG_CANCEL,
    MENU_NO_USE,
  },
  // SORT_MODE_TITLE_ONLY
  {
    MENU_MSG_MODE_NORMAL,
    MENU_MSG_ONLY_HINT,
    MENU_MSG_TYPE,
    MENU_MSG_NAME,
    MENU_MSG_NEW,
    MENU_MSG_DEFAULT,
    MENU_MSG_CANCEL,
    MENU_NO_USE,
  },  
  // SORT_MODE_HINT_ONLY
  {
    MENU_MSG_MODE_NORMAL,
    MENU_MSG_ONLY_TITLE,
    MENU_MSG_TYPE,
    MENU_MSG_NAME,
    MENU_MSG_NEW,
    MENU_MSG_DEFAULT,
    MENU_MSG_CANCEL,
    MENU_NO_USE,
  },  
  // SORT_MODE_NAME
  {
    MENU_MSG_MODE_NORMAL,
    MENU_MSG_ONLY_TITLE,
    MENU_MSG_ONLY_HINT,
    MENU_MSG_TYPE,
    MENU_MSG_NEW,
    MENU_MSG_DEFAULT,
    MENU_MSG_CANCEL,
    MENU_NO_USE,
  },  
  // SORT_MODE_TYPE
  {
    MENU_MSG_MODE_NORMAL,
    MENU_MSG_ONLY_TITLE,
    MENU_MSG_ONLY_HINT,
    MENU_MSG_NAME,
    MENU_MSG_NEW,
    MENU_MSG_DEFAULT,
    MENU_MSG_CANCEL,
    MENU_NO_USE,
  },  
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static GFL_PROC_RESULT MEDALBOXSYS_ProcInit( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT MEDALBOXSYS_ProcMain( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT MEDALBOXSYS_ProcTerm( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs );

//=============================================================================
// システム
//=============================================================================
static void MDLSYS_MakeCommonData( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_DeleteCommonData( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_GraphicInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_GraphicDelete( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_BGInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_BGDelete( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_EFFETC_ReLoad( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_OBJInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_OBJDelete( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_UI_Init( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_MedalInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_BMPWinInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_BMPWinDelete( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_wk_adrs );
static void MDLSYS_SetNextSeq( MEDALBOXSYS_WORK *p_wk ,MEDALBOXSYS_SEQ next_seq );
static void MDLSYS_PrintInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_PrintMain( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_PrintDelete( MEDALBOXSYS_WORK *p_wk );
static u32 MDLSYS_BG_GetFrame( int frame_id );

//=============================================================================
// 初期化シーケンス
//=============================================================================
static GFL_PROC_RESULT MDLSYS_INIT_Main( MEDALBOXSYS_WORK *p_wk );

//=============================================================================
// メインシーケンスメイン
//=============================================================================
static GFL_PROC_RESULT MDLSYS_MAIN_Main( MEDALBOXSYS_WORK *p_wk );
static void MainKeyStateCheck( MEDALBOXSYS_WORK *p_wk );
static void MakeMedalHitTbl( MEDALBOXSYS_WORK *p_wk ,GFL_UI_TP_HITTBL medal_hittbl[] );
static void MakeMedalHitFontBordTbl( MEDALBOXSYS_WORK *p_wk ,GFL_UI_TP_HITTBL medal_hittbl[] );
static void MakePushMedalButtonHitTbl( MEDALBOXSYS_WORK *p_wk ,GFL_UI_TP_HITTBL button_hittbl[] );
static void GetCursorPos( MEDALBOXSYS_WORK *p_wk , u8 cursor_pos , GFL_CLACTPOS *p_pos );
static void SetCursor( MEDALBOXSYS_WORK *p_wk , u8 cursor_pos , BOOL se_play );
static BOOL CursorUpdate( MEDALBOXSYS_WORK *p_wk );
static void SetRightKeyScrollSeq( MEDALBOXSYS_WORK *p_wk );
static void SetLeftKeyScrollSeq( MEDALBOXSYS_WORK *p_wk );
static void SetCursorState( MEDALBOXSYS_WORK *p_wk , CURSOR_STATE state , u8 medal_id );
static void NewEffectMain( MEDALBOXSYS_WORK *p_wk );
static void SlideTouchCheck( MEDALBOXSYS_WORK *p_wk );
static BOOL SlideTouchMain( MEDALBOXSYS_WORK *p_wk );

//=============================================================================
// ページ切替シーケンスメイン
//=============================================================================
static GFL_PROC_RESULT MDLSYS_SCROLLPAGE_Main( MEDALBOXSYS_WORK *p_wk );
static BOOL ScrollPage( MEDALBOXSYS_WORK *p_wk , fx32 move_speed );
static void PageRefresh( MEDALBOXSYS_WORK *p_wk , BOOL is_draw_medal_name , BOOL is_scrollbutton_refresh );
static void SetScrollDirection( MEDALBOXSYS_WORK *p_wk , SCROLL_DIR dir );
static void DrawMedalName( MEDALBOXSYS_WORK *p_wk , MedalID medal_id , u8 DrawPos );
static void ClearMedalName( MEDALBOXSYS_WORK *p_wk );
static void TransMedalName( MEDALBOXSYS_WORK *p_wk );
static void DrawMedalInfo( MEDALBOXSYS_WORK *p_wk , MY_MEDAL_ST *p_medal_st , INFOTYPE info_type );
static PRINTSYS_LSB GetMedalNameFontColor( void );
static PRINTSYS_LSB GetMedalInfoFontColor( void );
static u8 GetCursolSelectUnitPos( MEDALBOXSYS_WORK *p_wk );
static u8 GetCursolSelectMedalPos( MEDALBOXSYS_WORK *p_wk );
static void SetScrollCount( MEDALBOXSYS_WORK *p_wk , s32 count );
static void SetScrollSpeed( MEDALBOXSYS_WORK *p_wk , u32 speed );
static void SetScrollEndCursorPos( MEDALBOXSYS_WORK *p_wk , SCROLLEND_CURSOR_POS pos );
static void RefreshMedalScrollPos( MEDALBOXSYS_WORK *p_wk );
static void MoveNewEffectPos( MEDALBOXSYS_WORK *p_wk , s16 scroll_size );

//=============================================================================
// スクロールボタン
//=============================================================================
static GFL_PROC_RESULT MDLSYS_SCROLLBUTTON_Main( MEDALBOXSYS_WORK *p_wk );
static SCROLL_RESULT move_speed_scroll( MEDALBOXSYS_WORK *p_wk );

//=============================================================================
// 一押しボタン登録メイン
//=============================================================================
static GFL_PROC_RESULT MDLSYS_PUSHMEDALBUTTON_Main( MEDALBOXSYS_WORK *p_wk );
static void SetPushMedalData( MEDALBOXSYS_WORK *p_wk ,MedalID medal_id , PUSHMEDAL_TYPE set_type );

//=============================================================================
// モードチェンジ
//=============================================================================
static GFL_PROC_RESULT MDLSYS_MODECHANGE_Main( MEDALBOXSYS_WORK *p_wk );

//=============================================================================
// 戻るボタンシーケンス
//=============================================================================
static GFL_PROC_RESULT MDLSYS_RETURNBUTTON_Main( MEDALBOXSYS_WORK *p_wk );

//=============================================================================
// 終了ボタンシーケンス
//=============================================================================
static GFL_PROC_RESULT MDLSYS_EXITBUTTON_Main( MEDALBOXSYS_WORK *p_wk );

//=============================================================================
// メニューシーーケンス
//=============================================================================
static GFL_PROC_RESULT MDLSYS_MENU_Main( MEDALBOXSYS_WORK *p_wk );
static void OpenSearchMenu( MEDALBOXSYS_WORK *p_wk );

//=============================================================================
// メニューシーーケンス
//=============================================================================
static GFL_PROC_RESULT MDLSYS_SORT_Main( MEDALBOXSYS_WORK *p_wk );

//=============================================================================
// メダルシステム
//=============================================================================
static void MDLSYS_PageMedalDraw( MEDALBOXSYS_WORK *p_wk , u8 draw_page , u8 unit_pos , BOOL is_draw_medal_name);
static void MDLSYS_GetDrawMedalPos( int num , GFL_CLACTPOS* p_pos );
static void MDLSYS_GetDrawPushMedalButtonPos( int num , GFL_CLACTPOS* p_pos );
static s32  MDLSYS_GetMedalObjWindID( int num );
static void MDLSYS_PolishEffectScrollMain( MEDALBOXSYS_WORK *p_wk );

//=============================================================================
// デバッグ関数
//=============================================================================
#ifdef PM_DEBUG
static void DBEG_MDLSYS_DebugMain( MEDALBOXSYS_WORK *p_wk );
static void DBEG_MDLSYS_DebugInit( MEDALBOXSYS_WORK *p_wk );
#endif



// 外部参照用プロセスデータ定義
GFL_PROC_DATA medalBox_ProcData =
{
  MEDALBOXSYS_ProcInit,
  MEDALBOXSYS_ProcMain,
  MEDALBOXSYS_ProcTerm
};


//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスシステムプロセス初期化処理
 *
 *	@param	* p_proc
 *	@param	* seq 
 *	@param	*p_param_adrs
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MEDALBOXSYS_ProcInit( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs )
{
  // ワーク定義
  MEDALBOXSYS_WORK *p_wk;

  // プロセス用ヒープID
  HEAPID heapID = HEAPID_MEDALBOXSYS;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

  // 0個判定
  {
    MEDALBOXSYS_PARAM *p_tmp_param = p_param_adrs;
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_tmp_param->p_gamedata );
    MEDALBOX* p_medal_box = SaveData_GetMedalBox( p_sv );

    // メダルの数が0個のときは終了する
    if( MEDALBOX_GetMedalCount( p_medal_box ,TRUE) <= 0 )
    {
      return GFL_PROC_RES_FINISH;
    }
  }

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, heapID , MEDALBOXSYS_HEAP_SIZE );

	//ワーク作成
	p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(MEDALBOXSYS_WORK), heapID );
	GFL_STD_MemClear( p_wk, sizeof(MEDALBOXSYS_WORK) );

  // パラメータ
	p_wk->p_param	= p_param_adrs;
  p_wk->heapID  = heapID;

  // 共通データ作成
  MDLSYS_MakeCommonData( p_wk );

  // グラフィック初期化
  MDLSYS_GraphicInit( p_wk );

  // プリントシステムの初期化
  MDLSYS_PrintInit( p_wk );

  // UIの初期化
  MDLSYS_UI_Init( p_wk );
  
  // ビットマップウィンドウの初期化
  MDLSYS_BMPWinInit( p_wk );

  // メダルの初期化
  MDLSYS_MedalInit( p_wk );

  // カーソル点滅アニメ
  {
    ARCHANDLE	*p_handle;

    p_wk->p_blinkanm_wk = BLINKPALANM_Create( MEDALOBJ_PALETTENUM_MEDAL*16  , 16 , BLINKPALANM_MODE_MAIN_OBJ, p_wk->heapID );

    // グラフィックへのハンドル
		p_handle	= GFL_ARC_OpenDataHandle( ARCID_MEDALGRA , p_wk->heapID );

    // パレットアニメデータの作成
    BLINKPALANM_SetPalBufferArcHDL(
        p_wk->p_blinkanm_wk, p_handle, NARC_medalbox_gra_medal_obj2_NCLR, 0*16, 1*16 );
		
    // アーカイブハンドルを閉じる
    GFL_ARC_CloseDataHandle( p_handle );
  }

  //通信アイコン
  GFL_NET_ReloadIconTopOrBottom( TRUE , p_wk->heapID );

  // メダル情報を表示する
  DrawMedalInfo( p_wk , NULL , INFOTYPE_TARGET );

#ifdef PM_DEBUG
  DBEG_MDLSYS_DebugInit( p_wk );
#endif
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスシステムメイン
 *
 *	@param	* p_proc
 *	@param	* seq 
 *	@param	*p_param_adrs
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MEDALBOXSYS_ProcMain( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs )
{
  MEDALBOXSYS_WORK *p_wk = p_wk_adrs;
  GFL_PROC_RESULT  proc_ret = GFL_PROC_RES_FINISH;

  // 0個判定
  {
    MEDALBOXSYS_PARAM *p_tmp_param = p_param_adrs;
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_tmp_param->p_gamedata );
    MEDALBOX* p_medal_box = SaveData_GetMedalBox( p_sv );

    // メダルの数が0個のときは終了する
    if( MEDALBOX_GetMedalCount( p_medal_box ,TRUE) <= 0 )
    {
      return GFL_PROC_RES_FINISH;
    }
  }

#ifdef PM_DEBUG
  {
    const int trg = GFL_UI_KEY_GetTrg();

    // デバッグ関数
    DBEG_MDLSYS_DebugMain( p_wk );

    //  if( trg & PAD_BUTTON_START )
    //  {
    //    return GFL_PROC_RES_FINISH;
    //  }
  }
#endif

  switch( p_wk->now_seq )
  {
    // 起動演出シーケンス
  case SEQ_MDLBOX_INIT:
    proc_ret = MDLSYS_INIT_Main( p_wk );
    break;

    // メインシーケンス
  case SEQ_MDLBOX_MAIN:
    proc_ret = MDLSYS_MAIN_Main( p_wk );
    break;

    // ページスクロール
  case SEQ_MDLBOX_SCROLLPAGE:
    proc_ret = MDLSYS_SCROLLPAGE_Main( p_wk );
    break;

    // スクロールボタン
  case SEQ_MDLBOX_SCROLL_BUTTON:
    proc_ret = MDLSYS_SCROLLBUTTON_Main( p_wk );
    break;

    // モードチェンジシーケンス
  case SEQ_MDLBOX_MODE_CHANGE:
    proc_ret = MDLSYS_MODECHANGE_Main( p_wk );
    break;

    // 戻るボタン
  case SEQ_MDLBOX_RETURN:
    proc_ret = MDLSYS_RETURNBUTTON_Main( p_wk );
    break;

    // 終了シーケンス
  case SEQ_MDLBOX_EXIT:
    proc_ret = MDLSYS_EXITBUTTON_Main( p_wk );
    break;

    // メニュー用シーケンス
  case SEQ_MDLBOX_MENU:
    proc_ret = MDLSYS_MENU_Main( p_wk );
    break;

    // ソートシーケンス
  case SEQ_MDLBOX_SORT:
    proc_ret = MDLSYS_SORT_Main( p_wk );
    break;

    // 一押しメダル登録シーケンス
  case SEQ_PUSUMEDALBUTTON:
    proc_ret = MDLSYS_PUSHMEDALBUTTON_Main( p_wk );
    break;

  default:
    {
      GF_ASSERT( 0 );
    }
    proc_ret = GFL_PROC_RES_FINISH;
    break;
  }


    // 光沢エフェクトスクロールメイン
    MDLSYS_PolishEffectScrollMain( p_wk );

    // プリントメイン
    MDLSYS_PrintMain( p_wk );

  // ユーティリティーの更新
  {
    int i;
    BOOL trans_end;

    // 描画待ちフラグを初期化
    p_wk->is_print_util_wati = FALSE;

    // プリントキューを更新
    PRINTSYS_QUE_Main( p_wk->p_print_que );

    // プリントユーティリティーを更新
    for( i = 0; i < PRINT_UTIL_MAX ; i++ )
    {
      trans_end = PRINT_UTIL_Trans( &p_wk->print_util[i], p_wk->p_print_que );

      if( trans_end == FALSE )
      {
        p_wk->is_print_util_wati = TRUE;
      }
    }
  }

  // セルの更新
  GFL_CLACT_SYS_Main();

  return proc_ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスシステム終了処理
 *
 *	@param	* p_proc
 *	@param	* seq 
 *	@param	*p_param_adrs
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MEDALBOXSYS_ProcTerm( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs )
{
  MEDALBOXSYS_WORK *p_wk = p_wk_adrs;

  // 0個判定
  {
    MEDALBOXSYS_PARAM *p_tmp_param = p_param_adrs;
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_tmp_param->p_gamedata );
    MEDALBOX* p_medal_box = SaveData_GetMedalBox( p_sv );

    // メダルの数が0個のときは終了する
    if( MEDALBOX_GetMedalCount( p_medal_box ,TRUE) <= 0 )
    {
      //オーバーレイ破棄
      GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));
      return GFL_PROC_RES_FINISH;
    }
  }

  MEDALDATASYS_SaveMedalData( &p_wk->medal_data , p_wk->p_param->p_gamedata , p_wk->heapID );

  // 点滅アニメ終了
  BLINKPALANM_Exit( p_wk->p_blinkanm_wk );
  
  // VBLANKTask消去
	GFL_TCB_DeleteTask( p_wk->p_vblank_task );

  // BMPWinの開放
  MDLSYS_BMPWinDelete( p_wk );
  
  // OBJデータ開放
  MDLSYS_OBJDelete( p_wk );

  // BGデータ開放
  MDLSYS_BGDelete( p_wk );

  // グラフィックを開放
  MDLSYS_GraphicDelete( p_wk );

  // プリントシステムの開放
  MDLSYS_PrintDelete( p_wk );

  // 共通データ開放
  MDLSYS_DeleteCommonData( p_wk );

  // ワーク開放
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_MEDALBOXSYS );

	//オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルシステム共通データ作成
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_MakeCommonData( MEDALBOXSYS_WORK *p_wk )
{
  HEAPID heapID = p_wk->heapID;

	// フォント
	p_wk->p_font	= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

  // アプリケーション用メッセージ
  p_wk->p_app_msg =GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_medalbox_dat, heapID );

	// メダル名メッセージデータ
	p_wk->p_mdlname_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_medalname_dat, heapID );

	// メダル説明文メッセージデータ
	p_wk->p_mdlinfo_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_medalexplanation_dat, heapID );

	// メダルヒントメッセージデータ
	p_wk->p_mdlhelp_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_medalhelp_dat, heapID );

  // メダルデータの生成
  MEDALDATASYS_InitMedalData( &p_wk->medal_data , p_wk->p_param->p_gamedata , p_wk->heapID );
  
  // デフォルトの並び順に設定
  MEDALDATASYS_SortMedal( &p_wk->medal_data , SORT_MODE_DEFAULT );

  // スクロールデータの初期化
  p_wk->monitor_offset = FX32_CONST((-MONITOR_HEIGHT) + MONITOR_TOP_HEIGHT);

  // スクロール後のカーソル位置を初期化
  p_wk->scrollend_cursor_pos = SCROLLEND_CURSOR_STAY;

  // 文字描画待ちフラグ初期化
  p_wk->is_print_util_wati  = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルシステム共通データ開放
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_DeleteCommonData( MEDALBOXSYS_WORK *p_wk )
{

  // メダルデータ
  MEDALDATASYS_DeleteMedalData( &p_wk->medal_data );

  // メッセージ
  GFL_MSG_Delete( p_wk->p_app_msg );
	GFL_MSG_Delete( p_wk->p_mdlhelp_msg );
	GFL_MSG_Delete( p_wk->p_mdlinfo_msg );
	GFL_MSG_Delete( p_wk->p_mdlname_msg );

  // フォント
	GFL_FONT_Delete( p_wk->p_font );
}

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィック初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_GraphicInit( MEDALBOXSYS_WORK *p_wk )
{
  //レジスタ初期化
  G2_BlendNone();
  G2S_BlendNone();	
  
  // ウィンドウマスク設定ON
  {
    G2_SetWndOBJInsidePlane( GX_WND_PLANEMASK_BG0|GX_WND_PLANEMASK_BG1|GX_WND_PLANEMASK_BG2|GX_WND_PLANEMASK_BG3|GX_WND_PLANEMASK_OBJ, TRUE );
    G2_SetWndOutsidePlane( GX_WND_PLANEMASK_BG0|GX_WND_PLANEMASK_BG2|GX_WND_PLANEMASK_BG3|GX_WND_PLANEMASK_OBJ, TRUE );
    GX_SetVisibleWnd( GX_WNDMASK_OW );
    GXS_SetVisibleWnd( GX_WNDMASK_NONE );
  }


  //VRAMクリアー
  GFL_DISP_ClearVRAM( 0 );

  //VRAMバンク設定
  GFL_DISP_SetBank( &sc_vramSetTable );

  // ディスプレイON
  GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
  GFL_DISP_SetDispOn();

  //	表示
  GFL_DISP_GX_InitVisibleControl();

  //	フォント初期化
  GFL_FONTSYS_Init();

  // BGの初期化
  MDLSYS_BGInit( p_wk );

  // OBJの初期化
  MDLSYS_OBJInit( p_wk );

  // プリントシステムの初期化
  APP_PRINTSYS_COMMON_PrintStreamInit( &p_wk->print_sys_wk , APP_PRINTSYS_COMMON_TYPE_BOTH );
  
  // 描画用Vブランクタスクを登録
	p_wk->p_vblank_task	= GFUser_VIntr_CreateTCB( MDLSYS_Graphic_VBlankTask , p_wk , 0 );
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィックを開放する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_GraphicDelete( MEDALBOXSYS_WORK *p_wk )
{
  // OBJウィンドウを停止
  {
    GX_SetVisibleWnd( GX_WNDMASK_NONE );
    GXS_SetVisibleWnd( GX_WNDMASK_NONE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_BGInit( MEDALBOXSYS_WORK *p_wk )
{
  int i;
  HEAPID heapID = p_wk->heapID;

	//初期化
	GFL_BG_Init( heapID );

	//グラフィックモード設定
	{	
		static const GFL_BG_SYS_HEADER sc_bgsys_header	=
		{	
			GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_2D
		};
		GFL_BG_SetBGMode( &sc_bgsys_header );
	}

	//BG面設定
	{	
		int i;
		for( i = 0; i < MEDALBOXSYS_BG_FRAME_MAX; i++ )
		{	
			GFL_BG_SetBGControl( sc_bgsetup[i].frame, &sc_bgsetup[i].bgcnt_header, sc_bgsetup[i].mode );
			GFL_BG_SetVisible( sc_bgsetup[i].frame, VISIBLE_ON );
    }
  }

	//読み込み
	{	
		ARCHANDLE	*p_handle;
		p_handle	= GFL_ARC_OpenDataHandle( ARCID_MEDALGRA , heapID );

    // ====================================
		// 下画面パレット(メイン)
    // ====================================
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_medalbox_gra_medal_bgd_NCLR,
				PALTYPE_MAIN_BG, 0, 0 , heapID );

    // キャラクターデータ（下画面共通)
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_medalbox_gra_medal_bgd_NCGR,
        MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M), 0, 0, FALSE, heapID );

    // ページユニット
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalbox_gra_medal_bgd1_NSCR,
				MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_PAGEUNIT_M), 0, 0, FALSE, heapID );

    // メダルユニットはページ1枚左にはみ出している
    GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_PAGEUNIT_M) ,GFL_BG_SCROLL_X_SET , PAGE_WIDTH );

    // 文字盤
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalbox_gra_medal_bgd2_NSCR,
				MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M), 0, 0, FALSE, heapID );

    // キャラクターデータ（光沢演出用)
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_medalbox_gra_medal_bgd_NCGR,
        MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M), 0, 0, FALSE, heapID );

    // 光沢
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalbox_gra_medal_bgd3_NSCR,
        MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M), 0, 0, FALSE, heapID );
 
    // ====================================
		// 上画面(サブ)
    // ====================================
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_medalbox_gra_medal_bgu_NCLR,
				PALTYPE_SUB_BG, 0, 0 , heapID );

    // キャラクターデータ（上画面共通)
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_medalbox_gra_medal_bgu_NCGR,
				MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_MONITOR_S), 0, 0, FALSE, heapID );

    // モニター
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalbox_gra_medal_bgu2_NSCR,
				MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_MONITOR_S), 0, 0, FALSE, heapID );

    // モニター演出
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalbox_gra_medal_bgu3_NSCR,
				MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_EFFECTS_S), 0, 0, FALSE, heapID );

    // 背景
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalbox_gra_medal_bgu1_NSCR,
				MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_BACK_S), 0, 0, FALSE, heapID );

    // ボックスランクで色を変える
    {
      u8      box_rank;
      SAVE_CONTROL_WORK *p_sv;
      MEDALBOX* p_medal_box;
      u32     color_offset;

      // セーブデータの取得
      p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata );
      p_medal_box = SaveData_GetMedalBox( p_sv );
      box_rank = MEDALBOX_GetMedalBoxRank( p_medal_box );
      color_offset = box_rank;

      // B2とW2で色を変化させる
      if( box_rank == MEDALBOX_RANK_BW )
#if PM_VERSION == VERSION_W2
      {
        color_offset = box_rank;
      }
#else
      {
        color_offset = box_rank + 1;
      }
#endif

      GFL_ARCHDL_UTIL_TransVramPaletteEx( p_handle, NARC_medalbox_gra_medal_bgd_NCLR,
          PALTYPE_MAIN_BG, 0x20*(color_offset+FONTBORD_COLOR_OFFSET) , 0x20*FONTBORD_COLOR_OFFSET , 0x20 , heapID );
    }

		GFL_ARC_CloseDataHandle( p_handle );
	}

  // フォント用パレットの読み込み
  {
    GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
        0x20*SYSTEM_FONT_PAL, 0x20, p_wk->heapID);
  }


  // ブレンド設定
  {
    G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1 ,
        (GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ) ,
        15, 15 );
  }

  {
    // エフェクト面としてのプライオリティーを設定
    GFL_BG_SetPriority( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) , MEDALBOXSYS_BG_PRIORITY_ETC_M_EFFETC );
  }

  // 位置設定
  {
    GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_MONITOR_S) ,GFL_BG_SCROLL_Y_SET , FX_Whole(p_wk->monitor_offset) );
    GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_EFFECTS_S) ,GFL_BG_SCROLL_Y_SET , FX_Whole(p_wk->monitor_offset) );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  エフェクトリソースをロードする
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_EFFETC_ReLoad( MEDALBOXSYS_WORK *p_wk )
{
  ARCHANDLE	*p_handle;
  p_handle	= GFL_ARC_OpenDataHandle( ARCID_MEDALGRA , p_wk->heapID );

  // キャラクターデータ（光沢演出用)
  GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_medalbox_gra_medal_bgd_NCGR,
      MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M), 0, 0, FALSE, p_wk->heapID );

  // 光沢
  GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalbox_gra_medal_bgd3_NSCR,
      MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M), 0, 0, FALSE, p_wk->heapID );

  GFL_ARC_CloseDataHandle( p_handle );

}

//----------------------------------------------------------------------------
/**
 *	@brief  BGリソースの削除
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_BGDelete( MEDALBOXSYS_WORK *p_wk )
{	
	//破棄
	{	
		//GFL_BG_FillCharacterRelease( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M), 1,0 );
	}
	
	//BG面破棄
	{	
		int i;
		for( i = 0; i < MEDALBOXSYS_BG_FRAME_MAX; i++ )
		{	
			GFL_BG_ClearFrame( sc_bgsetup[i].frame );
			GFL_BG_FreeBGControl( sc_bgsetup[i].frame );
		}
	}

  // ブレンド設定
  {
    G2S_SetBlendAlpha(GX_BLEND_PLANEMASK_NONE,GX_BLEND_PLANEMASK_NONE,31,0);
  }

	//終了
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();

}

//----------------------------------------------------------------------------
/**
 *	@brief  OBJの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_OBJInit( MEDALBOXSYS_WORK *p_wk )
{
  
  HEAPID heapID = p_wk->heapID;

	//システム作成
  {
    const GFL_CLSYS_INIT sc_clsys_init = {
      0, 0,
      0, 512,
      4, 124,
      4, 124,
      0,
      32,32,32,32,
      16, 16,
    };
    GFL_CLACT_SYS_Create( &sc_clsys_init, &sc_vramSetTable , heapID );
  }

	// ユニット
	{	
		p_wk->p_clunit	= GFL_CLACT_UNIT_Create( 128, 1, heapID );
		GFL_CLACT_UNIT_SetDefaultRend( p_wk->p_clunit );
	}

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );


	//リソース読み込み
	{	
    int i;
		ARCHANDLE	*	p_handle[ARC_TYPE_MAX];
    u32* obj_res = p_wk->obj_res;
   
    // =======================================
    // リソースの読み込み
    // =======================================
    p_handle[ARC_TYPE_MEDAL_GRA] = GFL_ARC_OpenDataHandle( ARCID_MEDALGRA , heapID );
    p_handle[ARC_TYPE_COMMON_BUTTON_GRA] = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON , heapID );

    // ======================================
    // メダルOBJ
    // ======================================
    obj_res[ OBJRESID_MEDAL_CELLANM ] = GFL_CLGRP_CELLANIM_Register( p_handle[ARC_TYPE_MEDAL_GRA],
        NARC_medalbox_gra_medal_obj1_NCER, NARC_medalbox_gra_medal_obj1_NANR , heapID );

    obj_res[ OBJRESID_MEDAL_PLTT ]    = GFL_CLGRP_PLTT_RegisterEx( p_handle[ARC_TYPE_MEDAL_GRA],
        NARC_medalbox_gra_medal_obj1_NCLR , CLSYS_DRAW_MAX ,  0 , 0 , MEDALOBJ_PALETTENUM_MEDAL , heapID );
  
    obj_res[ OBJRESID_MEDAL_CGR ]     = GFL_CLGRP_CGR_Register( p_handle[ARC_TYPE_MEDAL_GRA],
        NARC_medalbox_gra_medal_obj1_NCGR , FALSE, CLSYS_DRAW_MAX, heapID );

    // ======================================
    // UIOBJ
    // ======================================
    obj_res[ OBJRESID_UI_CELLANM ] = GFL_CLGRP_CELLANIM_Register( p_handle[ARC_TYPE_MEDAL_GRA],
        NARC_medalbox_gra_medal_obj2_NCER, NARC_medalbox_gra_medal_obj2_NANR , heapID );

    obj_res[ OBJRESID_UI_PLTT ]    = GFL_CLGRP_PLTT_RegisterEx( p_handle[ARC_TYPE_MEDAL_GRA],
        NARC_medalbox_gra_medal_obj2_NCLR ,
        CLSYS_DRAW_MAIN ,  32*MEDALOBJ_PALETTENUM_MEDAL , 0 , MEDALOBJ_PALETTENUM_UI , heapID );	
    
    // ボックスランクで色を変える
    {
      u8      box_rank;
      SAVE_CONTROL_WORK *p_sv;
      MEDALBOX* p_medal_box;
      u32     color_offset;

      // セーブデータの取得
      p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata );
      p_medal_box = SaveData_GetMedalBox( p_sv );
      box_rank = MEDALBOX_GetMedalBoxRank( p_medal_box );
      color_offset = box_rank;

      // B2とW2で色を変化させる
      if( box_rank == MEDALBOX_RANK_BW )
#if PM_VERSION == VERSION_W2
      {
        color_offset = box_rank;
      }
#else
      {
        color_offset = box_rank+1;
      }
#endif

      GFL_CLGRP_PLTT_RegisterEx( p_handle[ARC_TYPE_MEDAL_GRA],
          NARC_medalbox_gra_medal_obj2_NCLR ,
          CLSYS_DRAW_MAIN ,  32*(MEDALOBJ_PALETTENUM_MEDAL+3) , color_offset+3 , 1 , heapID );	
    }

    obj_res[ OBJRESID_UI_CGR ]     = GFL_CLGRP_CGR_Register( p_handle[ARC_TYPE_MEDAL_GRA],
        NARC_medalbox_gra_medal_obj2_NCGR , FALSE, CLSYS_DRAW_MAIN, heapID );

    // ======================================
    // COMMONUIOBJ
    // ======================================
    obj_res[ OBJRESID_COMMONUI_CELLANM ] = GFL_CLGRP_CELLANIM_Register( p_handle[ARC_TYPE_COMMON_BUTTON_GRA],
        NARC_app_menu_common_bar_button_128k_NCER, NARC_app_menu_common_bar_button_128k_NANR , heapID );

    obj_res[ OBJRESID_COMMONUI_PLTT ]    = GFL_CLGRP_PLTT_RegisterEx( p_handle[ARC_TYPE_COMMON_BUTTON_GRA],
        NARC_app_menu_common_bar_button_NCLR ,
        CLSYS_DRAW_MAIN ,  32 * (MEDALOBJ_PALETTENUM_UI+MEDALOBJ_PALETTENUM_MEDAL), 0 ,
        MEDALOBJ_PALETTENUM_COMMON_UI , heapID );	
  
    obj_res[ OBJRESID_COMMONUI_CGR ]     = GFL_CLGRP_CGR_Register( p_handle[ARC_TYPE_COMMON_BUTTON_GRA],
        NARC_app_menu_common_bar_button_128k_NCGR , FALSE, CLSYS_DRAW_MAIN, heapID );

    // ======================================
    // COMMONUIOBJ(検索ボタン)
    // ================================== 
    obj_res[ OBJRESID_COMMONUI_SEARCH_CGR ]     = GFL_CLGRP_CGR_Register( p_handle[ARC_TYPE_COMMON_BUTTON_GRA],
        NARC_app_menu_common_search_button_128k_NCGR , FALSE, CLSYS_DRAW_MAIN, heapID );
    
    obj_res[ OBJRESID_COMMONUI_SEARCH_CELLANM ] = GFL_CLGRP_CELLANIM_Register( p_handle[ARC_TYPE_COMMON_BUTTON_GRA],
        NARC_app_menu_common_search_button_128k_NCER, NARC_app_menu_common_search_button_128k_NANR , heapID );

    // ======================================
    // アーカイブハンドルの開放
    // ======================================
    for( i = 0 ; i < ARC_TYPE_MAX ; i++ )
    {
      GFL_ARC_CloseDataHandle( p_handle[i] );
    }
  }

  // ======================================
  //CLWK作成
  // ======================================
  {
		int i;
    GFL_CLACTPOS pos;
    GFL_CLWK_DATA cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    // ======================================
    // 下画面
    // ======================================
    // メダルCLWK作成
    {
      for( i = 0 ; i < MEDALBOXSYS_OBJ_MEDAL_CLWK_MAX ; i++)
      {
        // 位置データの書き込み
        MDLSYS_GetDrawMedalPos( i , &pos );
        cldata.pos_x = pos.x;
        cldata.pos_y = pos.y;

        // プライオリティー
        cldata.bgpri = MEDALBOXSYS_OBJ_PRIORITY_MEDAL;
        cldata.softpri = MEDALBOXSYS_OBJ_SOFT_PRIORITY_BOTTOM;

        // ワークの作成
        p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
            p_wk->obj_res[OBJRESID_MEDAL_CGR],
            p_wk->obj_res[OBJRESID_MEDAL_PLTT],
            p_wk->obj_res[OBJRESID_MEDAL_CELLANM],
            &cldata, CLSYS_DEFREND_MAIN, heapID );
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i] , 0 );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
      }
    }

    // UIの作成
    for( i = MEDALBOXSYS_OBJ_MEDAL_CLWK_MAX; i < MEDALBOXSYS_OBJ_UI_CLWK_MAX; i++ )
    {	
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.bgpri   = MEDALBOXSYS_OBJ_PRIORITY_UI;
      cldata.softpri = MEDALBOXSYS_OBJ_SOFT_PRIORITY_MEDIUM;

      // ワークの作成
      p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->obj_res[OBJRESID_UI_CGR],
          p_wk->obj_res[OBJRESID_UI_PLTT],
          p_wk->obj_res[OBJRESID_UI_CELLANM],
          &cldata, CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i] , 0 );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
    }

    // COMMONUIの作成
    for( i = MEDALBOXSYS_OBJ_UI_CLWK_MAX; i < MEDALBOXSYS_OBJ_COMMON_CLWK_MAX; i++ )
    {	
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.bgpri   = MEDALBOXSYS_OBJ_PRIORITY_UI;
      cldata.softpri = MEDALBOXSYS_OBJ_SOFT_PRIORITY_MEDIUM;

      // ワークの作成
      p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->obj_res[OBJRESID_COMMONUI_CGR],
          p_wk->obj_res[OBJRESID_COMMONUI_PLTT],
          p_wk->obj_res[OBJRESID_COMMONUI_CELLANM],
          &cldata, CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i] , 0 );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
    }

    // 検索ボタンの作成
    {
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.bgpri   = MEDALBOXSYS_OBJ_PRIORITY_UI;
      cldata.softpri = MEDALBOXSYS_OBJ_SOFT_PRIORITY_MEDIUM;

      // ワークの作成
      p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_SEARCH]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->obj_res[OBJRESID_COMMONUI_SEARCH_CGR],
          p_wk->obj_res[OBJRESID_COMMONUI_PLTT],
          p_wk->obj_res[OBJRESID_COMMONUI_SEARCH_CELLANM],
          &cldata, CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_SEARCH] , APP_COMMON_BARICON_SEARCH);
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_SEARCH], TRUE );
    }

    // OBJWinの作成
    {
      int i;
      GFL_CLACTPOS pos;
      GFL_CLWK_DATA cldata;
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      for( i = MEDALBOXSYS_OBJ_CLWK_MEDAL1_OBJWIN; i <= MEDALBOXSYS_OBJ_CLWK_MEDAL25_OBJWIN; i++ )
      {	
        // 位置データの書き込み
        MDLSYS_GetDrawMedalPos( i - MEDALBOXSYS_OBJ_CLWK_MEDAL1_OBJWIN , &pos );
        cldata.pos_x = pos.x;
        cldata.pos_y = pos.y;

        // プライオリティー
        cldata.bgpri = MEDALBOXSYS_OBJ_PRIORITY_MEDAL;
        cldata.softpri = MEDALBOXSYS_OBJ_SOFT_PRIORITY_BOTTOM;

        // ワークの作成
        p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
            p_wk->obj_res[OBJRESID_MEDAL_CGR],
            p_wk->obj_res[OBJRESID_MEDAL_PLTT],
            p_wk->obj_res[OBJRESID_MEDAL_CELLANM],
            &cldata, CLSYS_DEFREND_MAIN, heapID );
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i] , 0 );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
        GFL_CLACT_WK_SetObjMode( p_wk->p_clwk[i], GX_OAM_MODE_OBJWND );
      }
    }

    // ======================================
    // 上画面
    // ======================================
    // メダルCLWK作成
    {
      // 位置データの書き込み
      cldata.pos_x = UP_MEDAL_POS_X;
      cldata.pos_y = UP_MEDAL_POS_Y;
      cldata.bgpri = MEDALBOXSYS_OBJ_PRIORITY_UP_MEDAL;

      // ワークの作成
      p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_UPMEDAL]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->obj_res[OBJRESID_MEDAL_CGR],
          p_wk->obj_res[OBJRESID_MEDAL_PLTT],
          p_wk->obj_res[OBJRESID_MEDAL_CELLANM],
          &cldata, CLSYS_DEFREND_SUB, heapID );
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_UPMEDAL] , 0 );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_UPMEDAL], FALSE );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  OBJデータの開放
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_OBJDelete( MEDALBOXSYS_WORK *p_wk )
{
  int i;

	//CLWK取得
	{
		for( i = 0; i < MEDALBOXSYS_OBJ_CLWK_MAX; i++ )
		{	
			if( p_wk->p_clwk[i] )
			{	
				GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
			}
		}
	}

  //リソース破棄
  {	
    // メダルOBJ
    {
      GFL_CLGRP_CELLANIM_Release( p_wk->obj_res[OBJRESID_MEDAL_CELLANM] );
      GFL_CLGRP_CGR_Release( p_wk->obj_res[OBJRESID_MEDAL_CGR] );
      GFL_CLGRP_PLTT_Release( p_wk->obj_res[OBJRESID_MEDAL_PLTT] );	
    }
    // UIOBJ
    {
      GFL_CLGRP_CELLANIM_Release( p_wk->obj_res[OBJRESID_UI_CELLANM] );
      GFL_CLGRP_CGR_Release( p_wk->obj_res[OBJRESID_UI_CGR] );
      GFL_CLGRP_PLTT_Release( p_wk->obj_res[OBJRESID_UI_PLTT] );	
    }
    // COMMONUIOBJ
    {
      GFL_CLGRP_CELLANIM_Release( p_wk->obj_res[OBJRESID_COMMONUI_CELLANM] );
      GFL_CLGRP_CGR_Release( p_wk->obj_res[OBJRESID_COMMONUI_CGR] );
      GFL_CLGRP_PLTT_Release( p_wk->obj_res[OBJRESID_COMMONUI_PLTT] );	
    }
    // 検索ボタン
    {
      GFL_CLGRP_CGR_Release( p_wk->obj_res[OBJRESID_COMMONUI_SEARCH_CGR] );
      GFL_CLGRP_CELLANIM_Release( p_wk->obj_res[OBJRESID_COMMONUI_SEARCH_CELLANM] );
    }
  }


	//システム破棄
	{
    if( p_wk->p_clunit )
    {	
      GFL_CLACT_UNIT_Delete( p_wk->p_clunit );
    }
  }

	GFL_CLACT_SYS_Delete();
}

//----------------------------------------------------------------------------
/**
 *	@brief  UIの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_UI_Init( MEDALBOXSYS_WORK *p_wk )
{
  int i;
  GFL_CLACTPOS pos;
  GFL_CLWK **p_clwk = p_wk->p_clwk;

  // スクロールボタン
  pos.x = UI_SCROLBTN_POS_X;
  pos.y = UI_SCROLBTN_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_SCROLBTN] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_SCROLBTN] , 0 );
  
  // スクロールボタン
  pos.x = UI_SCROLBER_POS_X;
  pos.y = UI_SCROLBER_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_SCROLBER] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_SCROLBER] , 1 );

  // カーソル
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR] , 2 );
  GFL_CLACT_WK_SetBgPri( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR], MEDALBOXSYS_OBJ_PRIORITY_CURSOR );

  // イチオシ
  MDLSYS_GetDrawPushMedalButtonPos( 0 , &pos);
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI] , 3 );
  
  // キラキラ
  pos.x = 0;
  pos.y = 0;
  // 新品エフェクト
  for( i=0 ; i<NEW_EFFETC_MAX; i++ )
  {
    GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i] , &pos , CLSYS_DEFREND_MAIN );
    GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i] , 4 );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i] , FALSE );
    GFL_CLACT_WK_SetBgPri( p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i], MEDALBOXSYS_OBJ_PRIORITY_NEWEFFETC );
    GFL_CLACT_WK_SetAutoAnmFlag( p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i] , TRUE);
    GFL_CLACT_WK_SetSoftPri( p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i] , MEDALBOXSYS_OBJ_SOFT_PRIORITY_MEDIUM_TOP );
  }

  // フタ
  pos.x = UI_CAP_POS_X;
  pos.y = UI_CAP_POS_Y;
  GFL_CLACT_WK_SetSoftPri( p_clwk[MEDALBOXSYS_OBJ_CLWK_CAP] , MEDALBOXSYS_OBJ_SOFT_PRIORITY_TOP );
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_CAP] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_CAP] , 5 );

   
  // 検索
  pos.x = UI_SEARCH_POS_X;
  pos.y = UI_SEARCH_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_SEARCH] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_SEARCH] , 0 );
   
  // キャンセル
  pos.x = UI_EXIT_POS_X;
  pos.y = UI_EXIT_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_EXIT] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_EXIT] , APP_COMMON_BARICON_EXIT );
   
  // 戻る
  pos.x = UI_RETURN_POS_X;
  pos.y = UI_RETURN_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_RETURN] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_RETURN] , APP_COMMON_BARICON_RETURN );

  // 右カーソル
  pos.x = UI_CURSOR_RIGHT_POS_X;
  pos.y = UI_CURSOR_RIGHT_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_RIGHT] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_RIGHT] , APP_COMMON_BARICON_CURSOR_RIGHT );

  // 左カーソル
  pos.x = UI_CURSOR_LEFT_POS_X;
  pos.y = UI_CURSOR_LEFT_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_LEFT] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_LEFT] , APP_COMMON_BARICON_CURSOR_LEFT );
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビットマップウィンドウの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_BMPWinInit( MEDALBOXSYS_WORK *p_wk )
{
  int i;
	
  // BMPWINの初期化
  GFL_BMPWIN_Init( p_wk->heapID );
  
  // 名前用BMPWIN作成
  p_wk->p_name_win = GFL_BMPWIN_Create( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M) ,
      0   , 0   ,
      32  , 24  ,
      MEDAL_NAME_BMPWIN_PAL_NUM , GFL_BMP_CHRAREA_GET_B );

  GF_ASSERT( p_wk->p_name_win );
  GFL_BMPWIN_MakeScreen( p_wk->p_name_win );

  // 説明文表示用BMPWIN
  p_wk->p_info_win = GFL_BMPWIN_Create( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_S) ,
      0   , 0   ,
      32  , 24  ,
      MEDAL_INFO_BMPWIN_PAL_NUM , GFL_BMP_CHRAREA_GET_B );
  GFL_BMPWIN_MakeScreen( p_wk->p_info_win );

  // 転送リクエスト
  GFL_BG_LoadScreenV_Req(MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M));
  GFL_BG_LoadScreenV_Req(MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_S));
  
  // プリントユーティリティーの準備
  PRINT_UTIL_Setup( &p_wk->print_util[ PRINT_UTIL_NAME_WIN ] , p_wk->p_name_win  );
  PRINT_UTIL_Setup( &p_wk->print_util[ PRINT_UTIL_INFO_WIN ] , p_wk->p_info_win  );
 
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビットマップウィンドウの開放
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_BMPWinDelete( MEDALBOXSYS_WORK *p_wk )
{
  int i;

  // 名前用BMPWIN
  GFL_BMPWIN_Delete( p_wk->p_name_win );

  // 情報ウィンドウ
  GFL_BMPWIN_Delete( p_wk->p_info_win );
}

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィックVブランクタスク
 *
 *	@param	*p_tcb
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_wk_adrs )
{
  // セルの更新
  GFL_CLACT_SYS_VBlankFunc();
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_MedalInit( MEDALBOXSYS_WORK *p_wk )
{
  u8 medal_id   = GAMEDATA_GetMedalBoxTopMedalID( p_wk->p_param->p_gamedata );
  u8 medal_pos  = MEDALDATASYS_GetMedalSortPos( &p_wk->medal_data , medal_id );
  
  // メダル位置が存在する場合はトップをずらしておく
  if( medal_id != MDLID_MAX )
  {
    p_wk->top_page_num = medal_pos/PAGE_MEDAL_MAX;
  }
  else
  {
    p_wk->top_page_num = 0;
  }

  // ページのリフレッシュ
  PageRefresh( p_wk ,TRUE , TRUE);
}

//----------------------------------------------------------------------------
/**
 *	@brief  ページのメダルを描画する
 *
 *	@param	*p_wk 
 *	@param	draw_page  表示するページ番号 
 *	@param	unit_pos  ページユニットの位置（0~4）
 *	@param	is_draw_medal_name  メダル名を描画するか
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_PageMedalDraw( MEDALBOXSYS_WORK *p_wk , u8 draw_page , u8 unit_pos , BOOL is_draw_medal_name)
{
  BOOL is_push_medal_draw = FALSE;
  int i;
  PAGEDATA *p_page = &p_wk->page[ unit_pos ];
  MY_MEDALDATA *p_medal_data = &p_wk->medal_data;

  GF_ASSERT( unit_pos < MEDALBOX_DRAW_PAGE_NUM_LIST );

  MEDALDATASYS_GetPageMedal( p_medal_data , p_page , draw_page );

  for( i = 0; i < PAGE_MEDAL_MAX ; i ++ )
  {
    u8 draw_pos = unit_pos * PAGE_MEDAL_MAX + i;

    if( i < p_page->medal_max )
    {
      // ヒントメダルの場合は最後のシーケンスを設定する
      switch( p_page->p_medal_st[i]->medal_state )
      {
      case MEDAL_STATE_TITLE_GET:
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[draw_pos] , p_page->p_medal_st[i]->graphic_type );
        break;
      case MEDAL_STATE_HINT_GET:
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[draw_pos] , GFL_CLACT_WK_GetAnmSeqNum(p_wk->p_clwk[draw_pos]) -1 );
        break;
      default:
        GF_ASSERT(0);
        break;
      }

      // 左端のみデータを表示する
      if( unit_pos == MEDAL_NAME_DRAW_PAGE )
      {
        if( p_wk->box_mode == BOXMODE_NORMAL )
        {
          if( is_draw_medal_name == TRUE )
          {

            // メダル名の描画
            if( p_page->p_medal_st[i]->medal_state == MEDAL_STATE_TITLE_GET )
            {
              DrawMedalName( p_wk , p_page->p_medal_st[i]->medal_id , draw_pos % PAGE_MEDAL_MAX ) ;
            }
            else
            {
              DrawMedalName( p_wk , MDL_NAME_HINT_000 , draw_pos % PAGE_MEDAL_MAX ) ;
            }

          }
          // 一押しボタンの表示
          if( p_page->p_medal_st[i]->medal_id == p_wk->medal_data.push_medalid && p_wk->box_mode == BOXMODE_NORMAL )
          {
            GFL_CLACTPOS pos;
            MDLSYS_GetDrawPushMedalButtonPos( draw_pos % PAGE_MEDAL_MAX , &pos);
            GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI] , &pos , CLSYS_DEFREND_MAIN );
            GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI], TRUE );
            is_push_medal_draw = TRUE;
          }
        }
      }

      // ページユニットに番号を書き込みを書き込み
      {
        int draw_num = draw_page*PAGE_MEDAL_MAX + draw_pos % PAGE_MEDAL_MAX + 1; // 1からスタート
        int n;
        int dim = 100;
        for( n=0 ; n < 3 ; n++ ) 
        {
          int number = draw_num / dim;
          s32 x = sc_unit_number_pos[i].x + (PAGE_WIDTH/8)*unit_pos - (2-n); // ページ幅をキャラ単位に直して、桁ごとに移動
          s32 y = sc_unit_number_pos[i].y;
          u8  pal_number;

          if( p_page->p_medal_st[i]->medal_id == p_wk->medal_data.push_medalid )
          {
            pal_number = PUSSHMEDAL_NUMBER_PAL;
          }
          else
          {
            pal_number = PAGEUNIT_PAL;
          }
          if( number == 0)
          {
            // 0は10番目にに配置されている
            GFL_BG_FillScreen( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_PAGEUNIT_M), 10 , x , y , 1, 1, pal_number );
          }
          else
          {
            GFL_BG_FillScreen( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_PAGEUNIT_M), number , x , y , 1, 1, pal_number );
          }

          // 桁数を下げる
          draw_num -= (number * dim);
          dim /= 10;
        }
      }

      // メダル画像を表示する
      {
        s32 obj_win_id;
        obj_win_id = MDLSYS_GetMedalObjWindID( draw_pos );
        // メダルOBJの表示
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[draw_pos], TRUE );
        // ウィンドウマスクを表示
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[obj_win_id], TRUE );
      }
    }
    else
    {
      // メダル画像を表示しない
      {
        s32 obj_win_id;
        obj_win_id = MDLSYS_GetMedalObjWindID( draw_pos );
        // メダルOBJの表示
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[draw_pos], FALSE );
        // ウィンドウマスクを表示
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[obj_win_id], FALSE );
      }

      // ページユニットに0番を書き込み
      {
        int n;
        for( n=0 ; n < 3 ; n++ ) 
        {
          s32 x = sc_unit_number_pos[i].x + (PAGE_WIDTH/8)*unit_pos - (2-n); // ページ幅をキャラ単位に直して、桁ごとに移動
          s32 y = sc_unit_number_pos[i].y;
          
          GFL_BG_FillScreen( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_PAGEUNIT_M), 10 , x , y , 1, 1, PAGEUNIT_PAL );
        }
      }
    }
  }
   
  // イチオシボタンを消す
  if( (is_push_medal_draw == FALSE) && (p_wk->box_mode == BOXMODE_NORMAL) && (unit_pos == MEDAL_NAME_DRAW_PAGE) )
  {
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI], FALSE );
  }

  GFL_BG_LoadScreenV_Req( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_PAGEUNIT_M) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルの表示先取得
 *
 *	@param	num   メダル穴番号
 *	@param	p_pos データ取得先
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_GetDrawMedalPos( int num , GFL_CLACTPOS* p_pos )
{
  p_pos->x = sc_medal_pos[num%PAGE_MEDAL_MAX].x + ( num/PAGE_MEDAL_MAX - 1 )*PAGE_WIDTH;
  p_pos->y = sc_medal_pos[num%PAGE_MEDAL_MAX].y;
}

//----------------------------------------------------------------------------
/**
 *	@brief  イチオシボタンの描画位置を取得
 *
 *	@param	num 
 *	@param	p_pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_GetDrawPushMedalButtonPos( int num , GFL_CLACTPOS* p_pos )
{
  p_pos->x = sc_ichioshi_pos[num].x;
  p_pos->y = sc_ichioshi_pos[num].y;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルに対応するOBJWINのセルIDを取得する
 *
 *	@param	num メダルのOBJID
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static s32  MDLSYS_GetMedalObjWindID( int num )
{
  return num+MEDALBOXSYS_OBJ_CLWK_MEDAL1_OBJWIN;
}

//----------------------------------------------------------------------------
/**
 *	@brief  光沢エフェクトメイン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_PolishEffectScrollMain( MEDALBOXSYS_WORK *p_wk )
{
  // シーケンスごとにスクロールするかどうかを決定
  switch( p_wk->now_seq )
  {
  case SEQ_MDLBOX_INIT:
  case SEQ_MDLBOX_MAIN:
  case SEQ_MDLBOX_SCROLLPAGE:
  case SEQ_MDLBOX_SCROLL_BUTTON:
  case SEQ_MDLBOX_MODE_CHANGE:
  case SEQ_MDLBOX_RETURN:
  case SEQ_MDLBOX_EXIT:
  case SEQ_MDLBOX_SORT:
    {
      // エフェクト面をスクロールさせる
      if( p_wk->is_polis_active == TRUE )
      {
        if( GFL_BG_GetScrollX(MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M)) <= -POLISH_EFFECT_SCROLL_SIZE  )
        {
          p_wk->polis_frame = POLIS_RESTART_FRAME + GFL_STD_MtRand( POLIS_RESTART_FRAME_RAND );
          p_wk->is_polis_active = FALSE;
          GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) ,GFL_BG_SCROLL_X_SET , 0 );
        }
        else
        {
          // スクロール
          GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) ,GFL_BG_SCROLL_X_DEC , POLISH_EFFECT_SCROLL_SPEED );
        }
      }
      else
      {
        p_wk->polis_frame--;
        if( p_wk->polis_frame <= 0 )
        {
          p_wk->is_polis_active = TRUE;
        }
      }
    }
    return;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  シーケンスの切替
 *
 *	@param	*p_wk 
 *	@param	next_seq 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_SetNextSeq( MEDALBOXSYS_WORK *p_wk ,MEDALBOXSYS_SEQ next_seq )
{
  // シーケンスを切り替える
  p_wk->now_seq = next_seq;
  p_wk->sub_seq = 0;

  // タッチスライドを初期化
  p_wk->is_slide_touch = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  プリントシステム初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_PrintInit( MEDALBOXSYS_WORK *p_wk )
{
  // ストリームの初期化
  p_wk->p_print_stream = NULL;

  // プリントシステム用タスクの生成
  p_wk->p_tcb_sys = GFL_TCBL_Init( p_wk->heapID , p_wk->heapID , 1 , 0 );

  // プリントキューの作成
  p_wk->p_print_que = PRINTSYS_QUE_Create( p_wk->heapID );

  // カーソルの作成
  p_wk->p_cursor = APP_KEYCURSOR_Create( 0xf , TRUE , TRUE , p_wk->heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  プリントシステムメイン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_PrintMain( MEDALBOXSYS_WORK *p_wk )
{
  // タスクの更新
  GFL_TCBL_Main( p_wk->p_tcb_sys );

  // プリントストリームの更新
  if( p_wk->p_print_stream != NULL )
  {
    BOOL ret;
    
    // キーカーソルの更新
    APP_KEYCURSOR_Main( p_wk->p_cursor , p_wk->p_print_stream , p_wk->p_menu_win );

    // ストリームの更新
    ret = APP_PRINTSYS_COMMON_PrintStreamFunc( &p_wk->print_sys_wk , p_wk->p_print_stream );
    if( ret == TRUE )
    {
      if( GFL_UI_KEY_GetTrg() & APP_PRINTSYS_COMMON_TRG_KEY || GFL_UI_TP_GetTrg() == TRUE )
      {
        PMSND_PlaySystemSE( APP_PRINTSYS_COMMON_SE_TRG );
        PRINTSYS_PrintStreamDelete( p_wk->p_print_stream );
        p_wk->p_print_stream = NULL;
      }
      else
      {
        APP_KEYCURSOR_Write( p_wk->p_cursor , GFL_BMPWIN_GetBmp( p_wk->p_menu_win ) , 0xF );
        GFL_BMPWIN_TransVramCharacter( p_wk->p_menu_win );
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  プリントシステムの開放
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_PrintDelete( MEDALBOXSYS_WORK *p_wk )
{
  // カーソルの開放
  APP_KEYCURSOR_Delete( p_wk->p_cursor );

  // プリントキューの開放
  PRINTSYS_QUE_Clear( p_wk->p_print_que );
  PRINTSYS_QUE_Delete( p_wk->p_print_que );
  p_wk->p_print_que = NULL;

  // タスクの終了
  GFL_TCBL_Exit( p_wk->p_tcb_sys );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGデータからフレームIDを取得する
 *
 *	@param	frame_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 MDLSYS_BG_GetFrame( int frame_id )
{
  return sc_bgsetup[ frame_id ].frame;
}


// ============================================================
// 
// シーケンスメイン関数郡
//
// ============================================================

//----------------------------------------------------------------------------
/**
 *	@brief  ボックス初期化シーケンス
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_INIT_Main( MEDALBOXSYS_WORK *p_wk )
{
  switch( p_wk->sub_seq )
  {
  case 0:
    {
      // ブラックアウトの開始
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );

      {
        GFL_CLACTPOS pos;
        pos.x = UP_MEDAL_POS_X;
        pos.y = UP_MEDAL_POS_Y - FX_Whole(p_wk->monitor_offset);

        // 上画面のスクロール
        GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_MONITOR_S) ,GFL_BG_SCROLL_Y_SET , FX_Whole(p_wk->monitor_offset) );
        GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_EFFECTS_S) ,GFL_BG_SCROLL_Y_SET , FX_Whole(p_wk->monitor_offset) );
        GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_S) ,GFL_BG_SCROLL_Y_SET , FX_Whole(p_wk->monitor_offset) );

        GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_UPMEDAL] , &pos , CLSYS_DEFREND_SUB);
      }

      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      // フェードの終了待ち
      if( !GFL_FADE_CheckFade() )
      {
        PMSND_PlaySE( SEQ_SE_MD_01 );
        p_wk->sub_seq++;
      }
    }
    break;
  case 2:
    {
      BOOL          cap_scroll_end        = FALSE;
      BOOL          monitor_scroll_end    = FALSE;
      GFL_CLACTPOS  cap_pos;

      // フタ部分のスクロール
      p_wk->cap_offset -= CAP_SCROLL_SPEED;

      // フタの位置を設定
      cap_pos.x = UI_CAP_POS_X + FX_Whole(p_wk->cap_offset);
      cap_pos.y = UI_CAP_POS_Y;

      // 移動範囲制限
      if( cap_pos.x < -CAP_WIDTH)
      {
        p_wk->cap_offset = FX32_CONST(-CAP_WIDTH);
        
        // フタの位置を設定
        cap_pos.x = UI_CAP_POS_X + FX_Whole(p_wk->cap_offset);
        cap_pos.y = UI_CAP_POS_Y;

        cap_scroll_end = TRUE;
      }

      p_wk->monitor_offset += MONITORE_SCROLL_SPEED;
      if( p_wk->monitor_offset > 0 )
      {
        p_wk->monitor_offset = 0;
        monitor_scroll_end = TRUE;
      }

      // ページユニットのスクロール
      GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CAP] , &cap_pos , CLSYS_DEFREND_MAIN );

      // 上画面のスクロール
      GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_MONITOR_S) ,GFL_BG_SCROLL_Y_SET , FX_Whole(p_wk->monitor_offset) );
      GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_EFFECTS_S) ,GFL_BG_SCROLL_Y_SET , FX_Whole(p_wk->monitor_offset) );
      GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_S) ,GFL_BG_SCROLL_Y_SET , FX_Whole(p_wk->monitor_offset) );

      {
        GFL_CLACTPOS pos;
        pos.x = UP_MEDAL_POS_X;
        pos.y = UP_MEDAL_POS_Y - FX_Whole(p_wk->monitor_offset);

        GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_UPMEDAL] , &pos , CLSYS_DEFREND_SUB );
        
      }
 
      // 次のシーケンスへ移動
      if( cap_scroll_end && monitor_scroll_end )
      {
        // 終了方法を調べてカーソルを表示する
        if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
        {
          u8 medal_id   = GAMEDATA_GetMedalBoxTopMedalID( p_wk->p_param->p_gamedata );
          u8 medal_pos  = MEDALDATASYS_GetMedalSortPos( &p_wk->medal_data , medal_id );
          if( medal_id != MDLID_MAX )
          {
            SetCursor( p_wk , medal_pos%PAGE_MEDAL_MAX , TRUE );
          }
          else
          {
            SetCursor( p_wk , 0  , TRUE );
          }
        }
        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_MAIN );
      }
    }
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メインシーケンス(SEQ_MDLBOX_MAIN)
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_MAIN_Main( MEDALBOXSYS_WORK *p_wk )
{
  // メインキー入力処理
  if( p_wk->is_print_util_wati == FALSE )
  {
    MainKeyStateCheck( p_wk );
  }

  // 新品エフェクト生成メイン
  NewEffectMain( p_wk );
 
  // カーソル点滅アニメ
  BLINKPALANM_Main( p_wk->p_blinkanm_wk );

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メインシーケンスキー入力
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MainKeyStateCheck( MEDALBOXSYS_WORK *p_wk )
{
  const int trg = GFL_UI_KEY_GetTrg();
  const int rep = GFL_UI_KEY_GetRepeat();
  const int cont = GFL_UI_KEY_GetCont();
  int tp_hit;

  // ==========================
  // タッチスライドでのページ送り判定
  // ==========================
  SlideTouchCheck( p_wk );

  // スライド移動量計算
  SlideTouchMain( p_wk );

  // スクロール移動量でスクロール処理を行う
  if( p_wk->slide_length < -SLIDE_MOVE_START_LENGTH )
  {
    SetScrollSpeed( p_wk , DIR_KEY_SCROLL_SPEED );
    SetScrollCount( p_wk , DIR_KEY_SCROLL_COUNT );
    SetScrollDirection( p_wk , SCROLL_DIR_LEFT );
    MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
    p_wk->slide_length = 0;
    GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);
  }

  // スクロール移動量でスクロール処理を行う
  if( p_wk->slide_length > SLIDE_MOVE_START_LENGTH )
  {
    SetScrollSpeed( p_wk , DIR_KEY_SCROLL_SPEED );
    SetScrollCount( p_wk , DIR_KEY_SCROLL_COUNT );
    SetScrollDirection( p_wk , SCROLL_DIR_RIGHT );
    MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
    p_wk->slide_length = 0;
    GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);
  }

  // ==========================
  // イチオシボタン登録
  // ==========================
  {
    if( p_wk->box_mode == BOXMODE_NORMAL )
    {
      GFL_UI_TP_HITTBL button_hittbl[PUSHMEDAL_BUTTON_MAX+1];

      // 一押しボタン登録テーブル
      MakePushMedalButtonHitTbl( p_wk , button_hittbl );

      // ヒットデータの取得
      tp_hit = GFL_UI_TP_HitTrg( button_hittbl );

      // 当たっていない時
      if( (tp_hit != GFL_UI_TP_HIT_NONE) )
      {
        u8 unit_pos   = GetCursolSelectUnitPos( p_wk ); // カーソルがどのユニット位置にいるか
        // メダルがあるかどうかの判定
        if( p_wk->page[unit_pos].medal_max > tp_hit )
        {
          MY_MEDAL_ST *p_medal_st = p_wk->page[unit_pos].p_medal_st[tp_hit];
          if( p_medal_st->medal_state == MEDAL_STATE_TITLE_GET )
          {
            MedalID medal_id = p_medal_st->medal_id;
            if( medal_id != p_wk->medal_data.push_medalid )
            {
              SetPushMedalData( p_wk ,medal_id , PUSHMEDAL_TYPE_SET );
            }
            else
            {
              SetPushMedalData( p_wk ,medal_id , PUSHMEDAL_TYPE_REMOVE );
            }
            
            // カーソル位置をせっていする
            SetCursor( p_wk , tp_hit  , FALSE );

            // シーケンスを切り替える
            MDLSYS_SetNextSeq( p_wk , SEQ_PUSUMEDALBUTTON );
            GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);
            return;
          }
          else
          {
            // カーソル位置をせっていする
            SetCursor( p_wk , tp_hit  , FALSE );

            // シーケンスを切り替える
            SetPushMedalData( p_wk , 0 , PUSHMEDAL_TYPE_HINT );
            MDLSYS_SetNextSeq( p_wk , SEQ_PUSUMEDALBUTTON );
            GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);
            return;
          }
        }
      }
    }

  }

  // キャンセル
  if( trg & PAD_BUTTON_X )
  {
    MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_EXIT );
    GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
    return;
  }

  // 戻る
  if( trg & PAD_BUTTON_B )
  {
    MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_RETURN );
    GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
    return;
  }

  // ==========================
  // タッチでのUI操作(トリガー)
  // ==========================
  {
    tp_hit = GFL_UI_TP_HitTrg( sc_ui_hittbl_trg );
    switch( tp_hit )
    {
    case UI_HIT_TABLE_TRG_SEARCH:
      {
        ONOUE_Printf("UI_HIT_TABLE_TRG_SEARCH\n");
        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_MENU );
        GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);
      }
      return;
    case UI_HIT_TABLE_TRG_CURSOR_LEFT:
      {
        ONOUE_Printf("UI_HIT_TABLE_TRG_CURSOR_LEFT\n");
        SetScrollSpeed( p_wk , DIR_KEY_SCROLL_SPEED );
        SetScrollCount( p_wk , DIR_KEY_SCROLL_COUNT );
        SetScrollDirection( p_wk , SCROLL_DIR_LEFT );
        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
        GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);
      }
      return;
    case UI_HIT_TABLE_TRG_CURSOR_RIGHT:
      {
        ONOUE_Printf("UI_HIT_TABLE_TRG_CURSOR_RIGHT\n");
        SetScrollSpeed( p_wk , DIR_KEY_SCROLL_SPEED );
        SetScrollCount( p_wk , DIR_KEY_SCROLL_COUNT );
        SetScrollDirection( p_wk , SCROLL_DIR_RIGHT );
        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
        GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);
      }
      return;
    case UI_HIT_TABLE_TRG_RETURN:
      {
        ONOUE_Printf("UI_HIT_TABLE_TRG_RETURN\n");
        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_RETURN );
        GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
      }
      return;
    case UI_HIT_TABLE_TRG_EXIT:
      {
        ONOUE_Printf("UI_HIT_TABLE_TRG_EXIT\n");
        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_EXIT );
        GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
      }
      return;
    }
  }
  
  // ==========================
  // タッチでのUI操作(べた入力)
  // ==========================
  {
    // ========================
    // UI用当たり判定テーブル(トリガーボタン)
    GFL_UI_TP_HITTBL ui_hittbl_cont[UI_HIT_TABLE_CONT_MAX] =
    {
      // UI_HIT_TABLE_CONT_SCROLL_BUTTON
      { 
        UI_SCROLBER_POS_Y -12,
        UI_SCROLBER_POS_Y +21,
        UI_SCROLBER_POS_X -SCROLLBAR_LENGTH/2,
        UI_SCROLBER_POS_X +SCROLLBAR_LENGTH/2, 
      }, 
      // 終了コード
      { GFL_UI_TP_HIT_END , 0 , 0 , 0 },
    };

    tp_hit = GFL_UI_TP_HitCont( ui_hittbl_cont );

    switch( tp_hit )
    {
    case UI_HIT_TABLE_CONT_SCROLL_BUTTON:
      {
        u32 tp_x,tp_y;
        if( GFL_UI_TP_GetPointCont( &tp_x ,&tp_y )  )
        {
          // タッチフレームのスクロールボタン位置を設定
          if( tp_x < UI_SCROLBTN_POS_X )
          {
            p_wk->scroll_button_pos = 0;
          }
          else if( (tp_x - UI_SCROLBTN_POS_X) > SCROLLBAR_LENGTH )
          {
            p_wk->scroll_button_pos = SCROLLBAR_LENGTH;
          }
          else
          {
            p_wk->scroll_button_pos = tp_x - UI_SCROLBTN_POS_X;
          }

          ONOUE_Printf("UI_HIT_TABLE_CONT_SCROLL_BUTTON\n");
          MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLL_BUTTON );
          GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);
        }
      }
      return;
    }
  }

  // ==========================
  // メニューを開く
  // ==========================
  if( trg & PAD_BUTTON_START )
  {
    MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_MENU );
    GFL_UI_SetTouchOrKey(GFL_APP_KTST_KEY);
    return;
  }


  // ==========================
  // キーでのカーソル移動
  // ==========================
  if( CursorUpdate( p_wk ) )
  {
    // 入力処理があった場合は以降の処理を行わない
    return;
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  スライドタッチでのページ移動開始判定
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SlideTouchCheck( MEDALBOXSYS_WORK *p_wk )
{
  u32 x,y;
  if( GFL_UI_TP_GetPointTrg( &x , &y) )
  {
    // スライドチェック中は開始判定を行わない
    if( p_wk->is_slide_touch == FALSE)
    {
      if( p_wk->box_mode == BOXMODE_NORMAL )
      {

        // キャップ幅以下
        if( x < CAP_WIDTH )
        {
          p_wk->is_slide_touch = TRUE;
          p_wk->before_x = x;
        }
      }
      else
      {
        p_wk->is_slide_touch = TRUE;
        p_wk->before_x = x;
      }
    }
  }
  if( GFL_UI_TP_GetCont() == FALSE )
  {
    p_wk->is_slide_touch = FALSE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  スライドタッチメイン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SlideTouchMain( MEDALBOXSYS_WORK *p_wk )
{
  if( p_wk->is_slide_touch )
  {
    u32 x,y;
    if( GFL_UI_TP_GetPointCont( &x , &y) )
    {
      p_wk->slide_length += p_wk->before_x - (s32)x;
      p_wk->before_x = (s32)x;

      if( p_wk->box_mode == BOXMODE_NORMAL )
      {
        // キャップ幅以下
        if( x > CAP_WIDTH )
        {
          p_wk->is_slide_touch = FALSE;
        }
      }
    }
  }
  
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ページスクロールシーケンス（SEQ_MDLBOX_SCROLLPAGE）
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_SCROLLPAGE_Main( MEDALBOXSYS_WORK *p_wk )
{
  BOOL  scroll_end = FALSE;
  u8    sub_seq = p_wk->sub_seq;

  switch( sub_seq )
  {
  case 0:
    {
      // メダル名を非表示に
      ClearMedalName( p_wk );

      SetCursorState( p_wk , CURSOR_STATE_SCROLL , MDLID_MAX );

      // 左右カーソルのアニメーション
      if( p_wk->scroll_dir == SCROLL_DIR_RIGHT )
      {
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_RIGHT] , APP_COMMON_BARICON_CURSOR_RIGHT_ON );
        GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_RIGHT] , TRUE);
      }
      else
      {
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_LEFT] , APP_COMMON_BARICON_CURSOR_LEFT_ON );
        GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_LEFT] , TRUE);
      }

      // スライド音
      PMSND_PlaySE( SEQ_SE_MD_02 );

      // サブシーケンスを進める
      p_wk->sub_seq++;
    }
    // 同一シーケンスで行うのでスルー
  case 1:
    {
      // ページのスクロール
      scroll_end = ScrollPage( p_wk , p_wk->scroll_speed * p_wk->scroll_dir );

      if( scroll_end )
      {
        // カウンタデクリメント
        p_wk->scroll_count--;
   
        // メインシーケンスへの移動
        if( p_wk->scroll_count <= 0 )
        {
          // サブシーケンスを進める
          p_wk->sub_seq++;
        }
        else
        {
          PMSND_PlaySE( SEQ_SE_MD_02 ); // ページ送り音
        }
      }
    }
    break;
  case 2:
    {
      u8  top_pos;

      // カーソルの存在するユニットの一番上のカーソル位置を計算
      top_pos   = (p_wk->cursor_pos/PAGE_MEDAL_MAX) * PAGE_MEDAL_MAX;
 
      // カーソル位置をせっていする
      if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
      {
        // カーソル位置をセットしなおす
        switch( p_wk->scrollend_cursor_pos )
        {
        case SCROLLEND_CURSOR_STAY:
          {
            SetCursor( p_wk , p_wk->cursor_pos , TRUE );
          }
          break;
        case SCROLLEND_CURSOR_TOP:
          {
            SetCursor( p_wk , top_pos  , TRUE);
          }
          break;
        case SCROLLEND_CURSOR_BOTTOM:
          {
            SetCursor( p_wk , top_pos+(PAGE_MEDAL_MAX-1)  , TRUE);
          }
          break;
        }
      }
      else
      {
        // 戻り先を現在のカーソル位置に設定
        p_wk->cursor_pos_tmp = p_wk->cursor_pos;

        // スクロール状態を未選択
        SetCursorState( p_wk , CURSOR_STATE_NOT_SELECT , MDLID_MAX );
      }

      // 次のカーソル終了位置を元に戻しておく
      p_wk->scrollend_cursor_pos = SCROLLEND_CURSOR_STAY;


      // スクロールカウントを初期化
      p_wk->scroll_count = 1;

      // ボタン位置を設定
      {
        GFL_CLACTPOS pos;
        fx32 page_ratio;
        fx32 fx_page_pos;

        // ページ位置を取得
        page_ratio = FX_Div( p_wk->top_page_num << FX32_SHIFT ,((p_wk->medal_data.page_max-1) << FX32_SHIFT) );
        fx_page_pos = FX_Mul( (SCROLLBAR_LENGTH<< FX32_SHIFT) ,  page_ratio);
        p_wk->scroll_button_pos = FX_Whole(fx_page_pos);

        // スクロールボタンの位置を設定
        pos.x = UI_SCROLBTN_POS_X + p_wk->scroll_button_pos;
        pos.y = UI_SCROLBTN_POS_Y;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_SCROLBTN] , &pos , CLSYS_DEFREND_MAIN );
      }

      // メダル名の転送
      TransMedalName( p_wk );

      // メインシーケンスに返す
      MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_MAIN );
    }
    break;
  }
  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロール方向の設定
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetScrollDirection( MEDALBOXSYS_WORK *p_wk , SCROLL_DIR dir )
{
  p_wk->scroll_dir = dir;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ページのスクロール
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL ScrollPage( MEDALBOXSYS_WORK *p_wk , fx32 move_speed )
{
  int i;
  BOOL scroll_end = FALSE;
  s32 offset;

  // ページのスクロール
  p_wk->page_offset += move_speed;
  offset = FX_Whole(p_wk->page_offset);

  // ページ幅以上に移動している時は、オフセットを初期化しページを進める
  if( MATH_ABS( offset ) >= PAGE_WIDTH )
  {
    // スクロール方向にページを進める
    if( (p_wk->top_page_num == 0) && (p_wk->scroll_dir < 0))
    {
      p_wk->top_page_num = p_wk->medal_data.page_max-1;
    }
    else
    {
      p_wk->top_page_num += p_wk->scroll_dir;
      p_wk->top_page_num %= p_wk->medal_data.page_max;
    }

    // オフセットの初期化
    p_wk->page_offset = 0;
    offset = 0;

    // ページのリフレッシュ
    PageRefresh( p_wk , TRUE , FALSE);

    scroll_end = TRUE;
  }

  // スクロールするオブジェクトの位置を反映する
  RefreshMedalScrollPos( p_wk );
  MoveNewEffectPos( p_wk , FX_Whole( move_speed ) );

  return scroll_end;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロールオフセットに合わせて、メダルの位置を移動させる
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void RefreshMedalScrollPos( MEDALBOXSYS_WORK *p_wk )
{
  GFL_CLACTPOS pos;
  s32 offset;
  int i;

  // ページのスクロール
  offset = FX_Whole(p_wk->page_offset);
 
  // 位置データの書き込み
  for( i = 0 ; i < MEDALBOXSYS_OBJ_MEDAL_CLWK_MAX ; i++ )
  {
    s32 obj_win_id;
    // 移動後の位置を計算
    MDLSYS_GetDrawMedalPos( i , &pos );
    pos.x -= offset;

    // メダルユニットはページ1枚左にはみ出している
    GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_PAGEUNIT_M) ,GFL_BG_SCROLL_X_SET , offset + PAGE_WIDTH );

    // メダルOBJの移動
    GFL_CLACT_WK_SetPos( p_wk->p_clwk[i] , &pos , CLSYS_DEFREND_MAIN );

    // OBJウィンドウも同時に移動
    obj_win_id = MDLSYS_GetMedalObjWindID( i );
    GFL_CLACT_WK_SetPos( p_wk->p_clwk[obj_win_id] , &pos , CLSYS_DEFREND_MAIN );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  新着エフェクトをスクロールさせる
 *
 *	@param	*p_wk 
 *	@param	scroll_size スクロールサイズ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MoveNewEffectPos( MEDALBOXSYS_WORK *p_wk , s16 scroll_size )
{
  GFL_CLACTPOS pos;

  int n;
  // 新品エフェクトのスクロール
  for( n=0 ; n<NEW_EFFETC_MAX; n++ )
  {
    if( GFL_CLACT_WK_CheckAnmActive(p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+n]) == TRUE )
    {
      GFL_CLACT_WK_GetPos( p_wk->p_clwk[ MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+n ] ,&pos , CLSYS_DEFREND_MAIN);
      pos.x -= scroll_size;
      GFL_CLACT_WK_SetPos( p_wk->p_clwk[ MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+n ] ,&pos , CLSYS_DEFREND_MAIN);
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  top_page_numに合わせてページ状態を更新する
 *
 *	@param	*p_wk 
 *	@param	refresh_num リフレッシュするページ数
 *	@param	is_scrollbutton_refresh スクロールボタンのリフレッシュを行う
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PageRefresh( MEDALBOXSYS_WORK *p_wk , BOOL is_draw_medal_name , BOOL is_scrollbutton_refresh )
{
  int i;
  u8 refresh_num;

  // メダルを全表示
  if( p_wk->box_mode == BOXMODE_NORMAL )
  {
    refresh_num = MEDALBOX_DRAW_PAGE_NUM_NORMAL;
  }
  else if( p_wk->box_mode == BOXMODE_LIST )
  {
    refresh_num = MEDALBOX_DRAW_PAGE_NUM_LIST;
  }
 
  // メダル名を非表示に
  if( is_draw_medal_name )
  {
    ClearMedalName( p_wk );
  }

  // イチオシボタンを消す
  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI], FALSE );

  for( i =0 ; i < refresh_num ; i++ )
  {
    int draw_page = p_wk->top_page_num + i -1;
    // 最初のページにループ
    if( draw_page < 0)
    {
      MDLSYS_PageMedalDraw( p_wk , p_wk->medal_data.page_max-1 , i , is_draw_medal_name);
    }
    // ページの最大数を超えている場合は最初のページへ
    else if( draw_page >= p_wk->medal_data.page_max )
    {
      MDLSYS_PageMedalDraw( p_wk , draw_page%p_wk->medal_data.page_max , i , is_draw_medal_name);
    }
    else
    {
      // 左端は-1ページ前を表示
      MDLSYS_PageMedalDraw( p_wk , p_wk->top_page_num + i -1 , i ,is_draw_medal_name);
    }
  }

  // メダル名の転送
  if( is_draw_medal_name )
  {
    TransMedalName( p_wk );
  }

  if( is_scrollbutton_refresh == TRUE )
  {
    // ボタン位置を設定
    {
      GFL_CLACTPOS pos;
      fx32 page_ratio;
      fx32 fx_page_pos;

      // ページ位置を取得
      page_ratio = FX_Div( p_wk->top_page_num << FX32_SHIFT ,((p_wk->medal_data.page_max-1) << FX32_SHIFT) );
      fx_page_pos = FX_Mul( (SCROLLBAR_LENGTH<< FX32_SHIFT) ,  page_ratio);
      p_wk->scroll_button_pos = FX_Whole(fx_page_pos);

      // スクロールボタンの位置を設定
      pos.x = UI_SCROLBTN_POS_X + p_wk->scroll_button_pos;
      pos.y = UI_SCROLBTN_POS_Y;
      GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_SCROLBTN] , &pos , CLSYS_DEFREND_MAIN );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル名表示
 *
 *	@param	*p_wk     
 *	@param	medal_id  描画するメダル名
 *	@param	DrawPos   描画位置（0~4)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawMedalName( MEDALBOXSYS_WORK *p_wk , MedalID medal_id , u8 DrawPos )
{
  STRBUF * str;
  PRINTSYS_LSB font_colro = GetMedalNameFontColor();

  // ノーマルモードのときのみ描画を行う
  if( p_wk->box_mode != BOXMODE_NORMAL )
  {
    return;
  }

  // メダル名の取得
  str = GFL_MSG_CreateString( p_wk->p_mdlname_msg , medal_id );

  // 文字列を直接描画
//  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(p_wk->p_name_win) ,
//      sc_medalname_win_pos[DrawPos].x , sc_medalname_win_pos[DrawPos].y ,
//      str , p_wk->p_font ,
//      font_colro );

  // 文字列を描画
  PRINT_UTIL_PrintColor( &p_wk->print_util[PRINT_UTIL_NAME_WIN] ,
      p_wk->p_print_que , sc_medalname_win_pos[DrawPos].x , sc_medalname_win_pos[DrawPos].y ,
      str ,p_wk->p_font , font_colro);

  // バッファーの消去 
  GFL_STR_DeleteBuffer( str );

}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル名の転送
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void TransMedalName( MEDALBOXSYS_WORK *p_wk )
{
  // 転送
  GFL_BMPWIN_TransVramCharacter( p_wk->p_name_win );
  
  // 転送リクエスト
  GFL_BG_LoadScreenV_Req(MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M));
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル情報表示
 *
 *	@param	*p_wk 
 *	@param	*p_medal_st メダルデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawMedalInfo( MEDALBOXSYS_WORK *p_wk , MY_MEDAL_ST *p_medal_st , INFOTYPE info_type )
{
  STRBUF  *str;
  u8      box_rank;
  PRINTSYS_LSB font_colro = GetMedalInfoFontColor();
  WORDSET *p_word_set = WORDSET_Create( p_wk->heapID ); 
  SAVE_CONTROL_WORK *p_sv;
  MEDALBOX* p_medal_box;

  GF_ASSERT( p_wk );

  // セーブデータの取得
  p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata );
  p_medal_box = SaveData_GetMedalBox( p_sv );


  // ボックスランクの取得
  box_rank = MEDALBOX_GetMedalBoxRank( p_medal_box );

  // =======================================
  // メダル画像変更
  // =======================================
  {
    GFL_CLWK  *p_clwk =  p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_UPMEDAL];
    if( info_type == INFOTYPE_GET )
    {
      if( p_medal_st->medal_state == MEDAL_STATE_TITLE_GET )
      {
        GFL_CLACT_WK_SetAnmSeq( p_clwk , p_medal_st->graphic_type );
        GFL_CLACT_WK_SetDrawEnable( p_clwk, TRUE );
      }
      else
      {
        GFL_CLACT_WK_SetAnmSeq( p_clwk , GFL_CLACT_WK_GetAnmSeqNum(p_clwk) -1 );
        GFL_CLACT_WK_SetDrawEnable( p_clwk, TRUE );
      }
    }
    else
    {
      GFL_CLACT_WK_SetDrawEnable( p_clwk, FALSE );
    }
  }

  // ウィンドウの塗りつぶし
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_info_win) , PRINTSYS_LSB_GetB(font_colro) );

  // =======================================
  // タイトルの表示
  // =======================================
  {
    if( (info_type == INFOTYPE_TARGET) && (box_rank != MEDALBOX_RANK_BW))
    {
      str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALBOX_01 );
      // 文字列を描画
      PRINT_UTIL_PrintColor( &p_wk->print_util[PRINT_UTIL_INFO_WIN] , p_wk->p_print_que , 
          MEDAL_INFO_TITLE_DRAW_X , MEDAL_INFO_TITLE_DRAW_Y ,
          str ,p_wk->p_font , font_colro);

      // バッファーの消去 
      GFL_STR_DeleteBuffer( str );
    }

    if(info_type == INFOTYPE_GET)
    {
      if( p_medal_st->medal_state == MEDAL_STATE_TITLE_GET )
      {
        str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALBOX_02 );
      }
      else
      {
        str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALBOX_03 );
      }
      // 文字列を描画
      PRINT_UTIL_PrintColor( &p_wk->print_util[PRINT_UTIL_INFO_WIN] , p_wk->p_print_que , 
          MEDAL_INFO_TITLE_DRAW_X , MEDAL_INFO_TITLE_DRAW_Y ,
          str ,p_wk->p_font , font_colro);

      // バッファーの消去 
      GFL_STR_DeleteBuffer( str );
    }
  }

  // =======================================
  // 数値の表示
  // =======================================
  {
      STRBUF * str_num;
      u16 title_cnt;  // タイトル取得数
      u16 get_cnt;    // 総取得数
      u8  rank_target_medal;
      title_cnt = MEDALBOX_GetMedalCount( p_medal_box , FALSE);
      get_cnt = MEDALBOX_GetMedalCount( p_medal_box , TRUE);

      // 目標メダル数を取得
      rank_target_medal = MEDALBOX_GetMedalBoxMax( p_medal_box ) + 1;

      if( info_type == INFOTYPE_GET )
      {
        // スラッシュ表示
        {
          str_num = GFL_STR_CreateBuffer( 20, p_wk->heapID );
          str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALBOX_04_01 );

          if(p_medal_st->medal_state == MEDAL_STATE_TITLE_GET)
          {
            WORDSET_RegisterNumber( p_word_set , 0 , title_cnt , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_ZENKAKU);
            WORDSET_RegisterNumber( p_word_set , 1 , get_cnt , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_ZENKAKU);
            WORDSET_ExpandStr( p_word_set, str_num , str  );
          }
          else
          {
            WORDSET_RegisterNumber( p_word_set , 0 , get_cnt - title_cnt , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_ZENKAKU);
            WORDSET_RegisterNumber( p_word_set , 1 , get_cnt , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_ZENKAKU);
            WORDSET_ExpandStr( p_word_set, str_num , str  );
          }

          // 文字列を描画
          PRINT_UTIL_PrintColor( &p_wk->print_util[PRINT_UTIL_INFO_WIN] , p_wk->p_print_que , 
              MEDAL_INFO_NUMERATOR_DRAW_X , MEDAL_INFO_NUMERATOR_DRAW_Y ,
              str_num ,p_wk->p_font , font_colro);

          // バッファーの消去 
          GFL_STR_DeleteBuffer( str );
          GFL_STR_DeleteBuffer( str_num );
        }
      }
      
      if( (info_type == INFOTYPE_TARGET) && (box_rank != MEDALBOX_RANK_BW))
      {
        str_num = GFL_STR_CreateBuffer( 20 , p_wk->heapID );
        str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALBOX_04 );

        WORDSET_RegisterNumber( p_word_set , 0 , rank_target_medal , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_ZENKAKU);
        WORDSET_ExpandStr( p_word_set, str_num , str  );

        // 文字列を描画
        PRINT_UTIL_PrintColor( &p_wk->print_util[PRINT_UTIL_INFO_WIN] , p_wk->p_print_que , 
            MEDAL_INFO_NUMERATOR_DRAW_X , MEDAL_INFO_NUMERATOR_DRAW_Y ,
              str_num ,p_wk->p_font , font_colro);

        // バッファーの消去 
        GFL_STR_DeleteBuffer( str );
        GFL_STR_DeleteBuffer( str_num );
      }
  }

  // =======================================
  // メダル名表示
  // =======================================
  {
    if( info_type == INFOTYPE_GET )
    {
      u32 width;
      if(p_medal_st->medal_state == MEDAL_STATE_TITLE_GET)
      {
        str = GFL_MSG_CreateString( p_wk->p_mdlname_msg , p_medal_st->medal_id );
      }
      else
      {
        str = GFL_MSG_CreateString( p_wk->p_mdlname_msg , MDL_NAME_HINT_000 );
      }

      // 幅の計算
      width = PRINTSYS_GetStrWidth( str , p_wk->p_font , 0);

        // 文字列を描画
        PRINT_UTIL_PrintColor( &p_wk->print_util[PRINT_UTIL_INFO_WIN] , p_wk->p_print_que , 
            MEDAL_INFO_NAME_DRAW_X - width, MEDAL_INFO_NAME_DRAW_Y,
              str ,p_wk->p_font , font_colro);

      // バッファーの消去 
      GFL_STR_DeleteBuffer( str );

    }
  }

  // =======================================
  // 取得日
  // =======================================
  {
    if( info_type == INFOTYPE_GET )
    {
      STRBUF *str_ymd;
      
      str_ymd  = GFL_STR_CreateBuffer( 30, p_wk->heapID ); 

      WORDSET_RegisterNumber( p_word_set , 0 , p_medal_st->year , 2 , STR_NUM_DISP_ZERO , STR_NUM_CODE_ZENKAKU);
      WORDSET_RegisterNumber( p_word_set , 1 , p_medal_st->month , 3 , STR_NUM_DISP_LEFT , STR_NUM_CODE_ZENKAKU);
      WORDSET_RegisterNumber( p_word_set , 2 , p_medal_st->day , 3 , STR_NUM_DISP_LEFT , STR_NUM_CODE_ZENKAKU);

      if(p_medal_st->medal_state == MEDAL_STATE_TITLE_GET)
      {
        str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALBOX_16 ); // 獲得した日付
      }
      else
      {
        str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALBOX_17 );
      }

      // 文字列を描画
      PRINT_UTIL_PrintColor( &p_wk->print_util[PRINT_UTIL_INFO_WIN] , p_wk->p_print_que , 
          MEDAL_INFO_GETTYPE_DRAW_X , MEDAL_INFO_GETTYPE_DRAW_Y,
          str ,p_wk->p_font , font_colro);

      // バッファーの消去 
      GFL_STR_DeleteBuffer( str );

      // 日付
      str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALBOX_18 );

      WORDSET_ExpandStr( p_word_set, str_ymd , str  );
      
      // 文字列を描画
      PRINT_UTIL_PrintColor( &p_wk->print_util[PRINT_UTIL_INFO_WIN] , p_wk->p_print_que , 
          MEDAL_INFO_DAYTE_DRAW_X , MEDAL_INFO_DAYTE_DRAW_Y,
          str_ymd ,p_wk->p_font , font_colro);

      // バッファーの消去 
      GFL_STR_DeleteBuffer( str );
      GFL_STR_DeleteBuffer( str_ymd );
    }
  }

  // =======================================
  // メダルの説明文表示
  // =======================================
  {
    if( info_type == INFOTYPE_GET )
    {
      if(p_medal_st->medal_state == MEDAL_STATE_TITLE_GET)
      {
        // メダルの説明文取得
        str = GFL_MSG_CreateString( p_wk->p_mdlinfo_msg , p_medal_st->medal_id );
      }
      else
      {
        str = GFL_MSG_CreateString( p_wk->p_mdlhelp_msg , p_medal_st->medal_id );
      }

     // 文字列を描画
      PRINT_UTIL_PrintColor( &p_wk->print_util[PRINT_UTIL_INFO_WIN] , p_wk->p_print_que , 
          MEDAL_INFO_INFO_DRAW_X , MEDAL_INFO_INFO_DRAW_Y,
          str ,p_wk->p_font , font_colro);

      // バッファーの消去 
      GFL_STR_DeleteBuffer( str );
    }
  }

  // 転送リクエスト
  if( (info_type == INFOTYPE_TARGET) && (box_rank == MEDALBOX_RANK_BW))
  {
    // 転送
    GFL_BMPWIN_TransVramCharacter( p_wk->p_info_win );
    GFL_BG_LoadScreenV_Req(MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_S));
  }

  // ワードセットの開放
  WORDSET_Delete( p_word_set );

  // 情報を表示した時に新着フラグを落とす
  {
    if( info_type == INFOTYPE_GET )
    {
      MEDALBOX_SetNewMedalFlgOff( p_medal_box , p_medal_st->medal_id  );
    }
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル名のクリアー
 *
 *	@param	*p_wk 
 *	@param	ClearPos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void ClearMedalName( MEDALBOXSYS_WORK *p_wk )
{
  PRINTSYS_LSB font_colro = GetMedalNameFontColor();

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_name_win) , 0 );

  // 転送
  GFL_BMPWIN_TransVramCharacter( p_wk->p_name_win );

  // 転送リクエスト
  GFL_BG_LoadScreenV_Req(MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M));
}

//----------------------------------------------------------------------------
/**
 *	@brief  下画面メダルタイトル部分のフォントカラーを取得する
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static PRINTSYS_LSB GetMedalNameFontColor( void )
{
  return PRINTSYS_LSB_Make(MEDAL_NAME_BMPWIN_STR_COL_L,MEDAL_NAME_BMPWIN_STR_COL_S,MEDAL_NAME_BMPWIN_STR_COL_B);
}

//----------------------------------------------------------------------------
/**
 *	@brief  上画面メダル情報部分のフォントカラーを取得する
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static PRINTSYS_LSB GetMedalInfoFontColor( void )
{
  return PRINTSYS_LSB_Make(MEDAL_INFO_BMPWIN_STR_COL_L,MEDAL_INFO_BMPWIN_STR_COL_S,MEDAL_INFO_BMPWIN_STR_COL_B);
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル用タッチ当たり判定テーブル作成
 *
 *	@param	*p_wk 
 *	@param	medal_hittbl[] 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MakeMedalHitTbl( MEDALBOXSYS_WORK *p_wk ,GFL_UI_TP_HITTBL medal_hittbl[] )
{
  u8 i;
  GFL_CLACTPOS pos;
  for( i=0; i < MEDAL_HIT_TABLE_MAX-1 ; i++ )
  {
    // メダルの描画位置を取得
    MDLSYS_GetDrawMedalPos( i + PAGE_MEDAL_MAX , &pos );
    
    medal_hittbl[i].rect.top   = pos.y - 2*8;
    medal_hittbl[i].rect.bottom= pos.y + 2*8;
    medal_hittbl[i].rect.left  = pos.x - 3*8;
    medal_hittbl[i].rect.right = pos.x + 3*8;
  }

  medal_hittbl[i].rect.top   = GFL_UI_TP_HIT_END;
  medal_hittbl[i].rect.bottom= 0;
  medal_hittbl[i].rect.left  = 0;
  medal_hittbl[i].rect.right = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  文字盤用当たり判定テーブル
 *
 *	@param	*p_wk 
 *	@param	medal_hittbl[] 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MakeMedalHitFontBordTbl( MEDALBOXSYS_WORK *p_wk ,GFL_UI_TP_HITTBL medal_hittbl[] )
{
  int i;
  u8 x,y;
  for( i=0; i < NAME_BMPWIN_MAX ; i++ )
  {
    // カーソルの位置を取得
    x = sc_medalname_win_pos[i].x;
    y = sc_medalname_win_pos[i].y;

    medal_hittbl[i].rect.top   = y;
    medal_hittbl[i].rect.bottom= y + 24;
    medal_hittbl[i].rect.left  = x;
    medal_hittbl[i].rect.right = x + 15*8;
  }

  medal_hittbl[i].rect.top   = GFL_UI_TP_HIT_END;
  medal_hittbl[i].rect.bottom= 0;
  medal_hittbl[i].rect.left  = 0;
  medal_hittbl[i].rect.right = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  イチオシボタン用当たり判定テーブル
 *
 *	@param	*p_wk 
 *	@param	button_hittbl[] 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MakePushMedalButtonHitTbl( MEDALBOXSYS_WORK *p_wk ,GFL_UI_TP_HITTBL button_hittbl[] )
{
  u8 i;
  GFL_CLACTPOS pos;
  for( i=0; i < PUSHMEDAL_BUTTON_MAX ; i++ )
  {
    // カーソルの位置を取得
    MDLSYS_GetDrawPushMedalButtonPos( i , &pos );
    button_hittbl[i].rect.top   = pos.y - PUSHMEDAL_BUTTON_H/2;
    button_hittbl[i].rect.bottom= pos.y + PUSHMEDAL_BUTTON_H/2;
    button_hittbl[i].rect.left  = pos.x - PUSHMEDAL_BUTTON_W/2;
    button_hittbl[i].rect.right = pos.x + PUSHMEDAL_BUTTON_W/2;
  }

  button_hittbl[i].circle.code = GFL_UI_TP_HIT_END;
  button_hittbl[i].circle.x    = 0;
  button_hittbl[i].circle.y    = 0;
  button_hittbl[i].circle.r    = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソルの位置を取得する
 *
 *	@param	cursor_pos  カーソルの場所
 *	@param	*p_pos      データの取得先
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GetCursorPos( MEDALBOXSYS_WORK *p_wk , u8 cursor_pos , GFL_CLACTPOS *p_pos )
{
  // カーソルが番はユニット１の先頭をさしていることに注意
  MDLSYS_GetDrawMedalPos( cursor_pos + PAGE_MEDAL_MAX , p_pos );
  p_pos->y -= 4;  // メダル座標に比べて-4の位置が中心になる
}

//----------------------------------------------------------------------------
/**
 *	@brief  入力処理でのカーソル位置更新
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CursorUpdate( MEDALBOXSYS_WORK *p_wk )
{
  const int trg = GFL_UI_KEY_GetTrg();
  const int rep = GFL_UI_KEY_GetRepeat();
  const int cont = GFL_UI_KEY_GetCont();
  int tp_hit;

  u8 cursor_tmp = p_wk->cursor_pos;
  u8 unit_pos   = GetCursolSelectUnitPos( p_wk ); // カーソルがどのユニット位置にいるか
  u8 top_pos    = (cursor_tmp/PAGE_MEDAL_MAX) * PAGE_MEDAL_MAX;      // そのユニット内で一番上のcursorposを取得
  u8 curpos_in_page = GetCursolSelectMedalPos( p_wk );

  
  // ==========================
  // メダルタッチでのカーソル移動
  // ==========================

  {
    if( p_wk->box_mode == BOXMODE_NORMAL )
    {
      // 文字盤のサイズ＋終了コード用のテーブルを作成する
      GFL_UI_TP_HITTBL fontbord_hittbl[NAME_BMPWIN_MAX+1];

      // 文字盤用当たり判定テーブル作成
      MakeMedalHitFontBordTbl( p_wk , fontbord_hittbl );

      // ヒットデータの取得
      tp_hit = GFL_UI_TP_HitTrg( fontbord_hittbl );

      // 当たっていない時
      if( (tp_hit != GFL_UI_TP_HIT_NONE) )
      {
        GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);
        SetCursor( p_wk , tp_hit , TRUE );
        return TRUE;
      }
    }
  }

  {
    // メダル
    GFL_UI_TP_HITTBL medal_hittbl[MEDAL_HIT_TABLE_MAX];

    // メダル辺り判定テーブル作成
    MakeMedalHitTbl( p_wk , medal_hittbl );

    // ヒットデータの取得
    tp_hit = GFL_UI_TP_HitTrg( medal_hittbl );

    // 当たっていない時
    if( (tp_hit != GFL_UI_TP_HIT_NONE) )
    {
      GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);
      SetCursor( p_wk , tp_hit , TRUE );
      return TRUE;
    }
  }
 
  // ==========================
  // キー入力でのカーソル移動
  // ==========================
  
  // カーソル位置をコピー
  cursor_tmp = p_wk->cursor_pos;

  // カーソルが選択されていない状態
  if( p_wk->cursor_state == CURSOR_STATE_NOT_SELECT )
  {
    if( (trg & PAD_KEY_UP) || (trg & PAD_KEY_DOWN) || (trg & PAD_KEY_LEFT) || (trg & PAD_KEY_RIGHT) || (trg & PAD_BUTTON_L) || (trg & PAD_BUTTON_R) || (trg & PAD_BUTTON_A) )
    {
      GFL_UI_SetTouchOrKey(GFL_APP_KTST_KEY);
      SetCursor( p_wk , p_wk->cursor_pos_tmp  , TRUE);
      p_wk->cursor_pos_tmp = 0;
      return TRUE;
    }
  }
  else
  {
    // 上へ移動
    if( rep & PAD_KEY_UP )
    {
      // 一番上を選択しているかを判定
      
      if( cursor_tmp == 0 )
      {
        ONOUE_Printf("前ページへ\n");
        SetScrollEndCursorPos( p_wk , SCROLLEND_CURSOR_BOTTOM);
        SetLeftKeyScrollSeq( p_wk );
        return TRUE;
      }
      else
      {
        cursor_tmp--;
      }

      GFL_UI_SetTouchOrKey(GFL_APP_KTST_KEY);
      SetCursor( p_wk , cursor_tmp  , TRUE);
      return TRUE;
    }

    // 下へ移動
    if( rep & PAD_KEY_DOWN )
    {
      if( (p_wk->box_mode == BOXMODE_NORMAL )&&(cursor_tmp == (PAGE_MEDAL_MAX*NORMAL_MOVE_UNITPOS_RIGHT-1)))
      {
        ONOUE_Printf("次ページへ\n");
        SetScrollEndCursorPos( p_wk , SCROLLEND_CURSOR_TOP);
        SetRightKeyScrollSeq( p_wk );
        return TRUE;
      }
      else if( (p_wk->box_mode == BOXMODE_LIST )&&(cursor_tmp == (PAGE_MEDAL_MAX*LIST_MOVE_UNITPOS_RIGHT-1))  )
      {
        ONOUE_Printf("次ページへ\n");
        SetScrollEndCursorPos( p_wk , SCROLLEND_CURSOR_TOP);
        SetRightKeyScrollSeq( p_wk );
        return TRUE;
      }
      else
      {
        cursor_tmp++;
      }

      GFL_UI_SetTouchOrKey(GFL_APP_KTST_KEY);
      SetCursor( p_wk , cursor_tmp  , TRUE);
      return TRUE;
    }

    // ページ送りorカーソル移動
    if( rep & PAD_KEY_RIGHT )
    {
      u8 unit_pos   = GetCursolSelectUnitPos( p_wk ); 
      // 右端の時のみページ送り
      if( ( (p_wk->box_mode == BOXMODE_NORMAL )&&(unit_pos == NORMAL_MOVE_UNITPOS_RIGHT) ) ||
          ( (p_wk->box_mode == BOXMODE_LIST )&&(unit_pos == LIST_MOVE_UNITPOS_RIGHT )))
      {
        SetRightKeyScrollSeq( p_wk );
        return TRUE;
      }
      else
      {
        cursor_tmp+=PAGE_MEDAL_MAX;
        SetCursor( p_wk , cursor_tmp  , TRUE);
      }
    }

    // ページ送りorカーソル移動
    if( rep & PAD_KEY_LEFT )
    {
      u8 unit_pos   = GetCursolSelectUnitPos( p_wk ); 
      // 左端の時のみページ送り
      if( unit_pos == 1 )
      {
        SetLeftKeyScrollSeq( p_wk );
        return TRUE;
      }
      else
      {
        cursor_tmp-=PAGE_MEDAL_MAX;
        SetCursor( p_wk , cursor_tmp  , TRUE);
      }
    }

    // 3ページ送り
    if( rep & PAD_BUTTON_R )
    {
      SetScrollSpeed( p_wk , LR_KEY_SCROLL_SPEED );
      SetScrollCount( p_wk , LR_KEY_SCROLL_COUNT );
      SetScrollDirection( p_wk , SCROLL_DIR_RIGHT );
      MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
      GFL_UI_SetTouchOrKey(GFL_APP_KTST_KEY);
      return TRUE;
    }

    // 3ページ送り
    if( rep & PAD_BUTTON_L )
    {
      SetScrollSpeed( p_wk , LR_KEY_SCROLL_SPEED );
      SetScrollCount( p_wk , LR_KEY_SCROLL_COUNT );
      SetScrollDirection( p_wk , SCROLL_DIR_LEFT );
      MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
      GFL_UI_SetTouchOrKey(GFL_APP_KTST_KEY);
      return TRUE;
    }
    
    // イチオシ登録
    if( trg & PAD_BUTTON_A )
    {
      u8 unit_pos   = GetCursolSelectUnitPos( p_wk ); // カーソルがどのユニット位置にいるか
      u8 curpos_in_page = GetCursolSelectMedalPos( p_wk );
      // メダルがあるかどうかの判定
      if( p_wk->page[unit_pos].medal_max > curpos_in_page )
      {
        MY_MEDAL_ST *p_medal_st = p_wk->page[unit_pos].p_medal_st[curpos_in_page];
        if( p_medal_st->medal_state == MEDAL_STATE_TITLE_GET )
        {
          MedalID medal_id = p_medal_st->medal_id;
          if( medal_id != p_wk->medal_data.push_medalid )
          {
            SetPushMedalData( p_wk ,medal_id , PUSHMEDAL_TYPE_SET );
          }
          else
          {
            SetPushMedalData( p_wk ,medal_id , PUSHMEDAL_TYPE_REMOVE );
          }
          MDLSYS_SetNextSeq( p_wk , SEQ_PUSUMEDALBUTTON );
          GFL_UI_SetTouchOrKey(GFL_APP_KTST_KEY);
          return TRUE;
        }
        else
        {
          // シーケンスを切り替える
          SetPushMedalData( p_wk , 0 , PUSHMEDAL_TYPE_HINT );
          MDLSYS_SetNextSeq( p_wk , SEQ_PUSUMEDALBUTTON );
          GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  右キーでのスクロールをセットする
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetRightKeyScrollSeq( MEDALBOXSYS_WORK *p_wk )
{
  SetScrollSpeed( p_wk , DIR_KEY_SCROLL_SPEED );
  SetScrollCount( p_wk , DIR_KEY_SCROLL_COUNT );
  SetScrollDirection( p_wk , SCROLL_DIR_RIGHT );
  MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
  GFL_UI_SetTouchOrKey(GFL_APP_KTST_KEY);
}

//----------------------------------------------------------------------------
/**
 *	@brief  左キーでのスクロールをセットする
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetLeftKeyScrollSeq( MEDALBOXSYS_WORK *p_wk )
{
  SetScrollSpeed( p_wk , DIR_KEY_SCROLL_SPEED );
  SetScrollCount( p_wk , DIR_KEY_SCROLL_COUNT );
  SetScrollDirection( p_wk , SCROLL_DIR_LEFT );
  MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
  GFL_UI_SetTouchOrKey(GFL_APP_KTST_KEY);
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソルを設定する
 *
 *	@param	*p_wk 
 *	@param	cursor_pos  カーソルの場所
 *	@param	se_play     効果音を鳴らす
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetCursor( MEDALBOXSYS_WORK *p_wk , u8 cursor_pos , BOOL se_play )
{
  GFL_CLACTPOS  pos;
  MY_MEDAL_ST*  p_st;
  u8            unit_pos;
  u8            medal_pos;
  u8            cursor_tmp;

  // 同じ場所は選択できない
  if( ( p_wk->cursor_pos == cursor_pos ) && ( p_wk->cursor_state == CURSOR_STATE_SELECT ))
  {
    return;
  }

  // カーソル位置を保存
  cursor_tmp = p_wk->cursor_pos;
  p_wk->cursor_pos = cursor_pos;

  // ユニット上の位置
  unit_pos = GetCursolSelectUnitPos( p_wk );

  // ページ上のメダル位置
  medal_pos = GetCursolSelectMedalPos( p_wk );

  // ノーマルモード時は1ページ目（画面一番左端のページ）しか選択できない
  if( (p_wk->box_mode == BOXMODE_NORMAL) && (unit_pos != 1) )
  {
    p_wk->cursor_pos = cursor_tmp;
    return;
  }

  // メダル情報の取得
  p_st = p_wk->page[unit_pos].p_medal_st[medal_pos];

  if( p_st )
  {
    // カーソルの位置を変更する
    GetCursorPos( p_wk , cursor_pos , &pos );
    GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR] , &pos , CLSYS_DEFREND_MAIN );

    // メダル情報を表示する
    DrawMedalInfo( p_wk , p_st , INFOTYPE_GET );

    // カーソルを選択状態にする
    SetCursorState( p_wk , CURSOR_STATE_SELECT , p_st->medal_id );
  }
  else
  {
    // カーソルの位置を変更する
    GetCursorPos( p_wk , cursor_pos , &pos );
    GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR] , &pos , CLSYS_DEFREND_MAIN );

    // カーソルを選択状態にする
    SetCursorState( p_wk , CURSOR_STATE_SELECT , MDLID_MAX );

    // メダル情報を表示する
    DrawMedalInfo( p_wk , p_st , INFOTYPE_TARGET );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソルの状態
 *
 *	@param	*p_wk 
 *	@param	on_off 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetCursorState( MEDALBOXSYS_WORK *p_wk , CURSOR_STATE state , u8 medal_id )
{
  p_wk->cursor_state = state;

  // 最初に表示すべきメダルを変更
  GAMEDATA_SetMedalBoxTopMedalID( p_wk->p_param->p_gamedata , medal_id );

  switch( p_wk->cursor_state )
  {
  case CURSOR_STATE_NOT_SELECT:
    {
      DrawMedalInfo( p_wk , NULL , INFOTYPE_TARGET );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR] , FALSE );
    }
    break;
  case CURSOR_STATE_SCROLL:
    {
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR] , FALSE );
    }
    break;
  case CURSOR_STATE_SELECT:
    {
      // カーソルの選択音を鳴らす
      PMSND_PlaySE( SEQ_SE_MD_03 );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR] , TRUE );
    }
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  新品エフェクト生成メイン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void NewEffectMain( MEDALBOXSYS_WORK *p_wk )
{
  int i;
  GFL_CLACTPOS pos;
  GFL_CLWK **p_clwk = p_wk->p_clwk;

  // 新品エフェクト
  for( i=0 ; i<NEW_EFFETC_MAX; i++ )
  {
    if( GFL_STD_MtRand( NEW_EFFECT_FREQUENTLY ) == 0 )
    {
      if( GFL_CLACT_WK_CheckAnmActive(p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i]) == FALSE )
      {
        u8        page_num = GFL_STD_MtRand( MEDALBOX_DRAW_PAGE_NUM_LIST-1 ) +1; // 0ページ目には出さない
        PAGEDATA  *p_page = &p_wk->page[ page_num ];
        u8        medal_pos = GFL_STD_MtRand( PAGE_MEDAL_MAX );

        // 範囲外には出さない
        if( p_page->medal_max > medal_pos )
        {
          // 新着のみに出す
          if( p_page->p_medal_st[ medal_pos ]->new_state != MDL_NO_NEW )
          {
            MDLSYS_GetDrawMedalPos( page_num*PAGE_MEDAL_MAX + medal_pos , &pos );
            pos.x -=p_wk->page_offset + (GFL_STD_MtRand( NEW_EFFECT_RANGE )-NEW_EFFECT_RANGE/2);
            pos.y -=p_wk->page_offset + (GFL_STD_MtRand( NEW_EFFECT_RANGE )-NEW_EFFECT_RANGE/2);
            GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i] , ANMSEQ_NEWEFFECT );
            GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i] , &pos , CLSYS_DEFREND_MAIN );
            GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i] , TRUE );
            GFL_CLACT_WK_SetAutoAnmFlag( p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i] , TRUE);
          }
        }
        else
        {
          // 表示しない場合もアニメーションは進行させ、メダル数が少ない場合に頻度が上がらないようにする
          GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i] , ANMSEQ_NEWEFFECT );
          GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i] , &pos , CLSYS_DEFREND_MAIN );
          GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i] , FALSE );
          GFL_CLACT_WK_SetAutoAnmFlag( p_clwk[MEDALBOXSYS_OBJ_CLWK_NEWEFFECT_0+i] , TRUE);
        }
      }
    }
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソルがどのユニット上にいるかを返す(カーソルが0番目のユニットをさすことはない)
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetCursolSelectUnitPos( MEDALBOXSYS_WORK *p_wk )
{
  u8 cursor_pos = p_wk->cursor_pos;
  return cursor_pos/PAGE_MEDAL_MAX + 1;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソルがページ上のどのメダルをさしているかを返す
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetCursolSelectMedalPos( MEDALBOXSYS_WORK *p_wk )
{
  u8 cursor_pos = p_wk->cursor_pos;
  return cursor_pos%PAGE_MEDAL_MAX; // ユニット内のページ位置
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロールを繰り返す回数を設定する
 *
 *	@param	*p_wk 
 *	@param	count 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetScrollCount( MEDALBOXSYS_WORK *p_wk , s32 count )
{
  p_wk->scroll_count = count;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロール後のカーソル位置
 *
 *	@param	*p_wk 
 *	@param	pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetScrollEndCursorPos( MEDALBOXSYS_WORK *p_wk , SCROLLEND_CURSOR_POS pos )
{
  p_wk->scrollend_cursor_pos = pos;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロールするスピードを変更する
 *
 *	@param	*p_wk 
 *	@param	speed: 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetScrollSpeed( MEDALBOXSYS_WORK *p_wk , u32 speed )
{
  p_wk->scroll_speed = speed;
}

//----------------------------------------------------------------------------
/**
 *	@brief  終了ボタンシーケンス(SEQ_MDLBOX_EXIT)
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_EXITBUTTON_Main( MEDALBOXSYS_WORK *p_wk )
{
  GFL_CLWK *p_button_clwk = p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_EXIT];

  // 終了ボタンシーケンス
  switch( p_wk->sub_seq )
  {
  case 0:
    {
      // 終了ボタンアニメーション
      GFL_CLACT_WK_SetAnmSeq( p_button_clwk , APP_COMMON_BARICON_EXIT_ON );
      GFL_CLACT_WK_SetAutoAnmFlag( p_button_clwk , TRUE);
      PMSND_PlaySystemSE(SEQ_SE_CLOSE1);
      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      if( GFL_CLACT_WK_CheckAnmActive(p_button_clwk) == FALSE )
      {
        p_wk->sub_seq++;
      }
    }
    break;
  case 2:
    {
      // ブラックアウトの開始
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
      p_wk->sub_seq++;
    }
    break;
  case 3:
    if( !GFL_FADE_CheckFade() ){
      p_wk->p_param->result = MB_RESULT_EXIT;
      return GFL_PROC_RES_FINISH;
    }
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  戻るボタンシーケンス(SEQ_MDLBOX_RETURN)
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_RETURNBUTTON_Main( MEDALBOXSYS_WORK *p_wk )
{
  GFL_CLWK *p_button_clwk = p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_RETURN];

  // 終了ボタンシーケンス
  switch( p_wk->sub_seq )
  {
  case 0:
    {
      // 終了ボタンアニメーション
      GFL_CLACT_WK_SetAnmSeq( p_button_clwk , APP_COMMON_BARICON_RETURN_ON );
      GFL_CLACT_WK_SetAutoAnmFlag( p_button_clwk , TRUE);
      PMSND_PlaySystemSE(SEQ_SE_CANCEL1);
      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      if( GFL_CLACT_WK_CheckAnmActive(p_button_clwk) == FALSE )
      {
        p_wk->sub_seq++;
      }
    }
    break;
  case 2:
    {
      // ブラックアウトの開始
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
      p_wk->sub_seq++;
    }
    break;
  case 3:
    if( !GFL_FADE_CheckFade() ){
      p_wk->p_param->result = MB_RESULT_RETURN;
      return GFL_PROC_RES_FINISH;
    }
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロールボタンシーケンスメイン( SEQ_MDLBOX_SCROLL_BUTTON )
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_SCROLLBUTTON_Main( MEDALBOXSYS_WORK *p_wk )
{
  switch( p_wk->sub_seq )
  {
  case 0:
    {
      // メダル名を非表示に
      ClearMedalName( p_wk );

      // カーソルを非表示に
      SetCursorState( p_wk , CURSOR_STATE_SCROLL , MDLID_MAX );

      p_wk->scroll_speed = AUTO_SCROLL_SPEED_MIN;
      p_wk->sub_seq++;
    }
    // タッチフレームはスルーして、処理を継続する
  case 1:
    {
      u32 tp_x,tp_y;
      if( GFL_UI_TP_GetPointCont( &tp_x ,&tp_y )  )
      {
        fx32 page_ratio;
        fx32 fx_page_pos;

        // スクロールバーの範囲制限
        if( tp_x < UI_SCROLBTN_POS_X )
        {
          p_wk->scroll_button_pos = 0;
        }
        else if( (tp_x - UI_SCROLBTN_POS_X) > SCROLLBAR_LENGTH )
        {
          p_wk->scroll_button_pos = SCROLLBAR_LENGTH;
        }
        else
        {
          p_wk->scroll_button_pos = tp_x - UI_SCROLBTN_POS_X;
        }

        // 目標ページに居る場合は、カーソル移動を行わない(ぶれ防止)
        page_ratio = FX_Div( (p_wk->scroll_button_pos<< FX32_SHIFT) ,  (SCROLLBAR_LENGTH<< FX32_SHIFT));
        fx_page_pos = FX_Mul( ((p_wk->medal_data.page_max-1) << FX32_SHIFT) ,  page_ratio );
        if( FX_Whole(fx_page_pos) == p_wk->top_page_num )
        {
          return GFL_PROC_RES_CONTINUE;
        }

        // スクロールバー位置を目標に移動
        move_speed_scroll( p_wk );
      }
      else
      {
        p_wk->sub_seq++;
      }
    }
    break;
  case 2:
    {
      // スクロールボタン位置まで強制スクロール
      SCROLL_RESULT result;
      result = move_speed_scroll( p_wk );
      if( result == SCROLL_RESULT_END )
      {
 
        // 戻り先を現在のカーソル位置に設定
        p_wk->cursor_pos_tmp = p_wk->cursor_pos;

        // カーソルを消す
        SetCursorState( p_wk , CURSOR_STATE_NOT_SELECT , MDLID_MAX );

        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_MAIN );
      }
    }
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロールバー位置を目標にページを加速度移動させる
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static SCROLL_RESULT move_speed_scroll( MEDALBOXSYS_WORK *p_wk )
{
  GFL_CLACTPOS pos;
  fx32 page_ratio;
  fx32 fx_page_pos;
  u32  page_gap;

  // ページ位置を取得
  page_ratio = FX_Div( (p_wk->scroll_button_pos<< FX32_SHIFT) ,  (SCROLLBAR_LENGTH<< FX32_SHIFT));
  fx_page_pos = FX_Mul( ((p_wk->medal_data.page_max-1) << FX32_SHIFT) ,  page_ratio );

  // スクロールボタンの位置を設定
  pos.x = UI_SCROLBTN_POS_X + p_wk->scroll_button_pos;
  pos.y = UI_SCROLBTN_POS_Y;
  GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_SCROLBTN] , &pos , CLSYS_DEFREND_MAIN );

  // 方向を調べる
  if( FX_Whole(fx_page_pos) < p_wk->top_page_num )
  {
    SetScrollDirection( p_wk , SCROLL_DIR_LEFT );
  }
  else if( FX_Whole(fx_page_pos) > p_wk->top_page_num )
  {
    SetScrollDirection( p_wk , SCROLL_DIR_RIGHT );
  }
  else
  {
    // オフセットの初期化
    p_wk->page_offset = 0;

    // スクロールするオブジェクトの位置を反映(オフセット分新着エフェクトを動かす)
    RefreshMedalScrollPos( p_wk );
    MoveNewEffectPos( p_wk , FX_Whole( p_wk->page_offset) );

    // ページのリフレッシュ
    PageRefresh( p_wk , TRUE , FALSE);

    // スクロール終了
    return SCROLL_RESULT_END;

  }

  // ページの距離を計算する
  page_gap = MATH_ABS((FX_Whole(fx_page_pos) - p_wk->top_page_num));

  // 速度調整
  if( page_gap > SPEED_GAIN_PAGE )
  {
    p_wk->scroll_speed += AUTO_SCROLL_SPEED_UP;

    // 最高速度制限
    if( p_wk->scroll_speed > AUTO_SCROLL_SPEED_MAX )
    {
      p_wk->scroll_speed = AUTO_SCROLL_SPEED_MAX;
    }
  }
  else
  {
    p_wk->scroll_speed -= AUTO_SCROLL_SPEED_DOWN;

    // 最低速度制限
    if( p_wk->scroll_speed < AUTO_SCROLL_SPEED_MIN )
    {
      p_wk->scroll_speed = AUTO_SCROLL_SPEED_MIN;
    }
  }

  // スクロールを行う
  {
    BOOL end = ScrollPage( p_wk , p_wk->scroll_speed * p_wk->scroll_dir );
    if( end )
    {
      // 音を鳴らす
      PMSND_PlaySE( SEQ_SE_MD_02 );

      // 加速度が最大の時のみスキップを行う
      if(p_wk->scroll_speed == AUTO_SCROLL_SPEED_MAX)
      {
        // スキップするページ数
        u16 skip_page = page_gap/3;

        ONOUE_Printf("スキップページ数%u\n",skip_page);

        // 移動後のトップページの差を調べる
        page_gap = MATH_ABS((FX_Whole(fx_page_pos) - p_wk->top_page_num));

        // スキップ処理を行うかを判定
        if( FX_Whole(fx_page_pos) < p_wk->top_page_num )
        {
          // 1以上の値が出た場合は加速
          p_wk->top_page_num -= skip_page;
        }
        else if( FX_Whole(fx_page_pos) > p_wk->top_page_num )
        {
          // 1以上の値が出た場合は加速
          p_wk->top_page_num += skip_page;
        }
        else
        {
          // ギャップが0の時はスクロール終了
          return SCROLL_RESULT_END;
        }
      }
    }
  }
  // スクロール中
  return SCROLL_RESULT_ACTIVE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メニュー用シーケンス(SEQ_MDLBOX_MENU)
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_MENU_Main( MEDALBOXSYS_WORK *p_wk )
{
  s32 ret; // RETURNボタン

  switch( p_wk->sub_seq )
  {
  // 検索ボタンアニメーション
  case 0:
    {
      // カーソルの存在してるページを取得
      u8 curpos_in_page = GetCursolSelectUnitPos( p_wk );

      // 決定音
      PMSND_PlaySE( SEQ_SE_DECIDE1 );

      // 光沢を消す
			GFL_BG_SetVisible( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M), VISIBLE_OFF );
      
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_SEARCH] , APP_COMMON_BARICON_ON_PASSIVE );
      GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_SEARCH] , TRUE);

      // トップページの設定
      if( (p_wk->page[curpos_in_page].medal_max > 0) && (p_wk->cursor_state == CURSOR_STATE_SELECT) )
      {
        p_wk->modechange_top_page_num =  p_wk->page[curpos_in_page].page_num;
      }
      else
      {
        p_wk->modechange_top_page_num = 0;
      }

      // 戻り先を現在のカーソル位置に設定
      p_wk->cursor_pos_tmp = p_wk->cursor_pos;

      // カーソルを消す
      SetCursorState( p_wk , CURSOR_STATE_NOT_SELECT , MDLID_MAX );
      
      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      if( GFL_CLACT_WK_CheckAnmActive(p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_SEARCH]) == FALSE )
      {
        p_wk->sub_seq++;
      }
    }
    break;
  case 2:
    {
      // OBJウィンドウは無効化
      GX_SetVisibleWnd( GX_WNDMASK_NONE );

      // メニュ面としてのプライオリティーを設定
      GFL_BG_SetPriority( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) , MEDALBOXSYS_BG_PRIORITY_ETC_M );

      // エフェクト面の位置を元に戻しておく
      GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) ,GFL_BG_SCROLL_X_SET , 0 );

      G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ , -8 );

      // メニューを開く
      GFL_BG_ClearFrame( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) );
      OpenSearchMenu( p_wk );
			GFL_BG_SetVisible( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M), VISIBLE_ON );
      
      p_wk->sub_seq++;
    }
    break;
  case 3:
    {
      int i;

      // タスクメニューのアップデート
      APP_TASKMENU_UpdateMenu( p_wk->p_menu_wk );

      // 終了判定
      if(APP_TASKMENU_IsFinish(p_wk->p_menu_wk))
      {
        ret = APP_TASKMENU_GetCursorPos(p_wk->p_menu_wk); // カーソル位置取得

        // メニュー終了
        APP_TASKMENU_CloseMenu(p_wk->p_menu_wk);             // メニュー終了

        // ウィンドウのクリアー
        GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_menu_win), 0 );

        // タスクメニュー用リソース開放
        APP_TASKMENU_RES_Delete( p_wk->p_taskmenu_res );

        // ビットマップウィンドウの削除
        GFL_BMPWIN_Delete( p_wk->p_menu_win );

        // エフェクトの読み込み
        MDLSYS_EFFETC_ReLoad( p_wk );

        G2_SetBlendBrightness( GX_BLEND_PLANEMASK_NONE , 0 );

        // メニュー数の初期化
        p_wk->menu_cnt = 0;
      
        {
          // OBJウィンドウを有効化
          GX_SetVisibleWnd( GX_WNDMASK_OW );

          // エフェクト面としてのプライオリティーを設定
          GFL_BG_SetPriority( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) , MEDALBOXSYS_BG_PRIORITY_ETC_M_EFFETC );
        }

        //　シーケンスを進める
        p_wk->sub_seq++;

        // 次の動作を選択する
        {
          u32 msg_id = sc_menu_data[p_wk->medal_data.sort_mode].msgid[ret];

          switch( msg_id )
          {
          // どちらのメニューを選んでも同じモード切替処理を呼び出す
          case MENU_MSG_MODE_NORMAL:
          case MENU_MSG_MODE_LIST:
            MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_MODE_CHANGE );
            break;
          case MENU_MSG_ONLY_TITLE:
            MEDALDATASYS_SortMedal( &p_wk->medal_data , SORT_MODE_TITLE_ONLY );
            MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SORT );
            break;
          case MENU_MSG_ONLY_HINT:
            MEDALDATASYS_SortMedal( &p_wk->medal_data , SORT_MODE_HINT_ONLY );
            MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SORT );
            break;
          case MENU_MSG_TYPE:
            MEDALDATASYS_SortMedal( &p_wk->medal_data , SORT_MODE_TYPE );
            MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SORT );
            break;
          case MENU_MSG_NEW:
            MEDALDATASYS_SortMedal( &p_wk->medal_data , SORT_MODE_NEW );
            MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SORT );
            break;
          case MENU_MSG_NAME:
            MEDALDATASYS_SortMedal( &p_wk->medal_data , SORT_MODE_NAME );
            MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SORT );
            break;
          case MENU_MSG_DEFAULT:
            MEDALDATASYS_SortMedal( &p_wk->medal_data , SORT_MODE_DEFAULT );
            MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SORT );
            break;
          // デフォルトとキャンセルは同じ処理を行う
          default:
          case MENU_MSG_CANCEL:
             
            // カーソル位置をせっていする
            if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
            {
              // 戻るときは同じ位置にカーソルを表示する
              SetCursor( p_wk , p_wk->cursor_pos  , TRUE );
            }
            else
            {
              // 戻り先を現在のカーソル位置に設定
              p_wk->cursor_pos_tmp = p_wk->cursor_pos;
            }
            MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_MAIN );
            break;
          }
        }

      }
    }
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  検索ボタンメニューを開く
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void OpenSearchMenu( MEDALBOXSYS_WORK *p_wk )
{
  APP_TASKMENU_INITWORK wk;
  s32 i;
  SORT_MODE sort_mode = p_wk->medal_data.sort_mode;
  STRBUF * str;
  PRINTSYS_LSB font_colro = GetMedalInfoFontColor();
  
  // メニュー数の初期化
  p_wk->menu_cnt = 0;

  // メニューデータ
  for( i = 0; i < SEARCH_MENU_MAX ; i++)
  {
    if( sc_menu_data[sort_mode].msgid[i] !=MENU_NO_USE )
    {
      // メニュートップ（モードチェンジの項目は、現在のモードによって読み込むメッセージを変える
      if( i == 0 )
      {
        if( p_wk->box_mode == BOXMODE_NORMAL )
        {
          p_wk->menulist[p_wk->menu_cnt].str      = GFL_MSG_CreateString( p_wk->p_app_msg, MENU_MSG_MODE_LIST );
        }
        else
        {
          p_wk->menulist[p_wk->menu_cnt].str      = GFL_MSG_CreateString( p_wk->p_app_msg, MENU_MSG_MODE_NORMAL );
        }
      }
      else
      {
        // トップ以外は、メニューデータに従い作成
        p_wk->menulist[p_wk->menu_cnt].str      = GFL_MSG_CreateString( p_wk->p_app_msg, sc_menu_data[sort_mode].msgid[i] );
      }
      p_wk->menulist[p_wk->menu_cnt].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
      p_wk->menulist[p_wk->menu_cnt].type     = APP_TASKMENU_WIN_TYPE_NORMAL;
      p_wk->menu_cnt++;
    }
  }

  // 最後には戻るアイコンを追加
  p_wk->menulist[p_wk->menu_cnt-1].type = APP_TASKMENU_WIN_TYPE_RETURN;

  wk.heapId   = p_wk->heapID;
  wk.itemNum  = p_wk->menu_cnt;
  wk.itemWork = p_wk->menulist;
  wk.posType  = ATPT_RIGHT_DOWN;
  wk.charPosX = 32;
  wk.charPosY = 24;
  wk.w        = APP_TASKMENU_PLATE_WIDTH;
  wk.h        = APP_TASKMENU_PLATE_HEIGHT;

  
  // アプリケーションタスクメニュー用リソース作成
  p_wk->p_taskmenu_res = APP_TASKMENU_RES_Create( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M),
      APP_TASKMENU_USE_PALETE , p_wk->p_font, p_wk->p_print_que , p_wk->heapID );

  // メニューワーク
  p_wk->p_menu_wk = APP_TASKMENU_OpenMenu( &wk, p_wk->p_taskmenu_res );

  // メニュー用BMPWIN作成
  p_wk->p_menu_win = GFL_BMPWIN_Create( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) ,
      1   , 24-5   ,
      17  , 4  ,
      SYSTEM_FONT_PAL , GFL_BMP_CHRAREA_GET_B );

  // スクリーンの作成
  GFL_BMPWIN_MakeScreen( p_wk->p_menu_win );
 
  // フレーム用グラフィックをセットする
  BmpWinFrame_GraphicSet( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) , 1 , APP_WINFRAME_USE_PALETE , MENU_TYPE_SYSTEM , p_wk->heapID );

  // ウィンドウ枠の表示
  BmpWinFrame_Write( p_wk->p_menu_win , WINDOW_TRANS_ON_V, 1, APP_WINFRAME_USE_PALETE );

  // 説明文の作成
  str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALBOX_07 );
 
  // ウィンドウのクリアー
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_menu_win), 15 );

  // 転送
  GFL_BMPWIN_TransVramCharacter( p_wk->p_menu_win );

  // 文字列を直接描画
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_menu_win) , 0 , 0 , str , p_wk->p_font );

  // 転送
  GFL_BMPWIN_TransVramCharacter( p_wk->p_menu_win );
     
  // 転送リクエスト
  GFL_BG_LoadScreenV_Req(MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M));

  // バッファーの消去 
  GFL_STR_DeleteBuffer( str );

  // 文字バッファーの開放
  for( i = 0; i < p_wk->menu_cnt ; i++ )
  {
    GFL_STR_DeleteBuffer( p_wk->menulist[i].str );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  モードチェンジシーケンス(SEQ_MDLBOX_MODE_CHANGE)
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_MODECHANGE_Main( MEDALBOXSYS_WORK *p_wk )
{
  switch( p_wk->sub_seq )
  {
  case 0:
     
    // 移動音
    PMSND_PlaySE( SEQ_SE_MD_04 );

    // メダル名を非表示に
    ClearMedalName( p_wk );

    // 一押しボタンを非表示に
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI], FALSE );

    p_wk->sub_seq++;
    // 次のフレームをそのまま実行
  case 1:
    {
      BOOL          cap_scroll_end        = FALSE;
      BOOL          fontbord_scroll_end   = TRUE;
      GFL_CLACTPOS  cap_pos;

      // ==================================
      // フタ部分のスクロール
      p_wk->cap_offset += CAP_SCROLL_SPEED;

      // フタの位置を設定
      cap_pos.x = UI_CAP_POS_X + FX_Whole(p_wk->cap_offset);
      cap_pos.y = UI_CAP_POS_Y;

      // 移動範囲制限
      if( cap_pos.x > UI_CAP_POS_X)
      {
        // オフセットを初期化
        p_wk->cap_offset = 0;

        // フタの位置を設定
        cap_pos.x = UI_CAP_POS_X + FX_Whole(p_wk->cap_offset);
        cap_pos.y = UI_CAP_POS_Y;

        cap_scroll_end = TRUE;
      }

      // フタのスクロールイン
      GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CAP] , &cap_pos , CLSYS_DEFREND_MAIN );

   
      // ==================================
      // リストモードのときは文字盤を閉じる
      if( p_wk->box_mode == BOXMODE_LIST )
      {
        fontbord_scroll_end = FALSE;

        // 文字盤の移動
        p_wk->fontbord_offset += FONTBORD_SCROLL_SPEED;
        if( FX_Whole(p_wk->fontbord_offset) > 0 )
        {
          p_wk->fontbord_offset = 0;
          fontbord_scroll_end = TRUE;
        }

        // 上画面のスクロール
        GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M) ,GFL_BG_SCROLL_X_SET , FX_Whole(p_wk->fontbord_offset) );
        GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M) ,GFL_BG_SCROLL_X_SET , FX_Whole(p_wk->fontbord_offset) );

      }

      // 次のシーケンスへ移動
      if( cap_scroll_end && fontbord_scroll_end )
      {
        p_wk->sub_seq++;
      }
    }
    break;
  case 2:
    {
      // ==================================
      // モード切替
      if( p_wk->box_mode == BOXMODE_NORMAL )
      {
        p_wk->box_mode =  BOXMODE_LIST;
      }
      else
      {
        p_wk->box_mode =  BOXMODE_NORMAL;
      }

      // トップページ番号を設定
      p_wk->top_page_num = p_wk->modechange_top_page_num;

      // ページをリフレッシュ
      PageRefresh( p_wk , FALSE , TRUE);

      // メダル名を非表示に
      ClearMedalName( p_wk );
 
      // 一押しボタンを非表示に
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI], FALSE );

      p_wk->tmp_frame =0;
      p_wk->sub_seq++;
    }
    break;
  case 3:
    {
      // 演出用待機
      p_wk->tmp_frame++;
      if( p_wk->tmp_frame > HIT_STOP_FRAME )
      {
        p_wk->sub_seq++;
      }
    }
    break;
  case 4:
    {
      BOOL          cap_scroll_end        = FALSE;
      BOOL          fontbord_scroll_end   = TRUE;
      GFL_CLACTPOS  cap_pos;

      // ==================================
      // フタ部分のスクロール
      p_wk->cap_offset -= CAP_SCROLL_SPEED;

      // フタの位置を設定
      cap_pos.x = UI_CAP_POS_X + FX_Whole(p_wk->cap_offset);
      cap_pos.y = UI_CAP_POS_Y;

      // 移動範囲制限
      if( cap_pos.x < -CAP_WIDTH)
      {
        p_wk->cap_offset = FX32_CONST(-CAP_WIDTH);
        
        // フタの位置を設定
        cap_pos.x = UI_CAP_POS_X + FX_Whole(p_wk->cap_offset);
        cap_pos.y = UI_CAP_POS_Y;

        cap_scroll_end = TRUE;
      }

      // ==================================
      // 文字盤のスクロール
      GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CAP] , &cap_pos , CLSYS_DEFREND_MAIN );


      // リストの時はフタをあける
      if( p_wk->box_mode == BOXMODE_LIST )
      {
        fontbord_scroll_end = FALSE;

        // 文字盤の移動
        p_wk->fontbord_offset -= FONTBORD_SCROLL_SPEED;

        if( FX_Whole(p_wk->fontbord_offset) < -FONTBORD_WIDTH )
        {
          p_wk->fontbord_offset = FX32_CONST(-FONTBORD_WIDTH);
          fontbord_scroll_end = TRUE;
        }

        // 上画面のスクロール
        GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M) ,GFL_BG_SCROLL_X_SET , FX_Whole(p_wk->fontbord_offset) );
        GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M) ,GFL_BG_SCROLL_X_SET , FX_Whole(p_wk->fontbord_offset) );
      }

      // カーソル位置を補正（移動した時点でカーソル位置は見えていなくてもかえる）
      if( p_wk->box_mode == BOXMODE_NORMAL )
      {
        p_wk->cursor_pos %= MEDALBOX_DRAW_PAGE_NUM_LIST;
      }
      // 次のシーケンスへ移動
      if( cap_scroll_end && fontbord_scroll_end )
      {

        // メダル名を表示するためにリフレッシュ
        PageRefresh( p_wk , TRUE , TRUE);

        // タッチキーで終了した場合はカーソルを表示する
        if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
        {
          // カーソル位置は必ず0番に初期化
          SetCursor( p_wk , p_wk->cursor_pos  , TRUE );
        }
        else
        {
          // カーソル位置を0番に初期化
          p_wk->cursor_pos_tmp = p_wk->cursor_pos;
        }

        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_MAIN );
      }
    }
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  一押しボタン登録メイン(SEQ_PUSUMEDALBUTTON)
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_PUSHMEDALBUTTON_Main( MEDALBOXSYS_WORK *p_wk )
{

  switch( p_wk->sub_seq )
  {
  case 0:
    {
      // 光沢を消す
      GFL_BG_SetVisible( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M), VISIBLE_OFF );
      // OBJウィンドウは無効化
      GX_SetVisibleWnd( GX_WNDMASK_NONE );

      // メニュ面としてのプライオリティーを設定
      GFL_BG_SetPriority( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) , MEDALBOXSYS_BG_PRIORITY_ETC_M );

      // エフェクト面の位置を元に戻しておく
      GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) ,GFL_BG_SCROLL_X_SET , 0 );
      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      STRBUF * str;
      
      if( p_wk->pushmedal_set_type != PUSHMEDAL_TYPE_HINT )
      {
        // 登録音
        PMSND_PlaySE( SEQ_SE_SAVE );
      }
      else
      {
        // ビープ音
        PMSND_PlaySE( SEQ_SE_BEEP );
      }

      // メニュー用BMPWIN作成
      p_wk->p_menu_win = GFL_BMPWIN_Create( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) ,
          2   , 24-6   ,
          28  , 4  ,
          SYSTEM_FONT_PAL , GFL_BMP_CHRAREA_GET_B );
      
      // スクリーンの作成
      GFL_BMPWIN_MakeScreen( p_wk->p_menu_win );

      // フレーム用グラフィックをセットする
      BmpWinFrame_GraphicSet( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) , 1 , APP_WINFRAME_USE_PALETE , MENU_TYPE_SYSTEM , p_wk->heapID );

      // ウィンドウ枠の表示
      BmpWinFrame_Write( p_wk->p_menu_win , WINDOW_TRANS_ON_V, 1, APP_WINFRAME_USE_PALETE );

      // 説明文の作成
      switch( p_wk->pushmedal_set_type )
      {
      case PUSHMEDAL_TYPE_SET:
        str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALBOX_05 );
        break;
      case PUSHMEDAL_TYPE_REMOVE:
        str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALBOX_06 );
        break;
      case PUSHMEDAL_TYPE_HINT:
        str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALBOX_05_01 );
        break;
      }

      // ウィンドウのクリアー
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_menu_win), 15 );

      // 転送
      GFL_BMPWIN_TransVramCharacter( p_wk->p_menu_win );

      // プリントストリームの作成
      p_wk->p_print_stream = PRINTSYS_PrintStream( p_wk->p_menu_win , 0 , 0 , str ,
          p_wk->p_font , MSGSPEED_GetWait() , p_wk->p_tcb_sys , 0 , p_wk->heapID , 15 );

      // 転送
      GFL_BMPWIN_TransVramCharacter( p_wk->p_menu_win );

      // 転送リクエスト
      GFL_BG_LoadScreenV_Req(MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M));

      // バッファーの消去 
      GFL_STR_DeleteBuffer( str );

      // ページのリフレッシュ
      PageRefresh( p_wk ,FALSE , TRUE);

      GFL_BG_SetVisible( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M), VISIBLE_ON );
      p_wk->sub_seq++;
    }
    break;
  case 2:
    {
      int i;

      // カーソル点滅アニメ
      for( i = 0; i <  CURSOR_BRINK_SPEED ; i++ )
      {
        BLINKPALANM_Main( p_wk->p_blinkanm_wk );
      }

      // プリントストリーム終了判定
      if( p_wk->p_print_stream == NULL )
      {
        GFL_BG_SetVisible( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M), VISIBLE_OFF );
        p_wk->sub_seq++;
      }
    }
    break;
  case 3:
    {
      // ビットマップウィンドウの削除
      GFL_BMPWIN_Delete( p_wk->p_menu_win );

      // エフェクトの読み込み
      MDLSYS_EFFETC_ReLoad( p_wk );


      {
        // OBJウィンドウを有効化
        GX_SetVisibleWnd( GX_WNDMASK_OW );

        // エフェクト面としてのプライオリティーを設定
        GFL_BG_SetPriority( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M) , MEDALBOXSYS_BG_PRIORITY_ETC_M_EFFETC );
      }

      // 表示を有効化
      GFL_BG_SetVisible( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M), VISIBLE_ON );

      MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_MAIN );
      p_wk->sub_seq++;
    }

    break;
  }
  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  イチオシメダルを設定する
 *
 *	@param	*p_wk 
 *	@param	medal_id 
 *	@param	set_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetPushMedalData( MEDALBOXSYS_WORK *p_wk ,MedalID medal_id , PUSHMEDAL_TYPE set_type )
{
  // 設定方法の保存
  p_wk->pushmedal_set_type = set_type;

  // 設定方法
  if( set_type == PUSHMEDAL_TYPE_SET )
  {
    p_wk->medal_data.push_medalid = medal_id;
  }
  else if( set_type == PUSHMEDAL_TYPE_REMOVE )
  {
    p_wk->medal_data.push_medalid = MDLID_MAX;
  }
  else if( set_type == PUSHMEDAL_TYPE_HINT )
  {
    // 何も行わない
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ソートメインシーケンス(SEQ_MDLBOX_SORT)
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_SORT_Main( MEDALBOXSYS_WORK *p_wk )
{
  switch( p_wk->sub_seq )
  {
  case 0:
     
    // 移動音
    PMSND_PlaySE( SEQ_SE_MD_04 );

    // メダル名を非表示に
    ClearMedalName( p_wk );

    // 一押しボタンを非表示に
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI], FALSE );

    p_wk->sub_seq++;
    // 次のフレームをそのまま実行
  case 1:
    {
      BOOL          cap_scroll_end        = FALSE;
      BOOL          fontbord_scroll_end   = TRUE;
      GFL_CLACTPOS  cap_pos;

      // ==================================
      // フタ部分のスクロール
      p_wk->cap_offset += CAP_SCROLL_SPEED;

      // フタの位置を設定
      cap_pos.x = UI_CAP_POS_X + FX_Whole(p_wk->cap_offset);
      cap_pos.y = UI_CAP_POS_Y;

      // 移動範囲制限
      if( cap_pos.x > UI_CAP_POS_X)
      {
        p_wk->cap_offset = 0;

        // フタの位置を設定
        cap_pos.x = UI_CAP_POS_X + FX_Whole(p_wk->cap_offset);
        cap_pos.y = UI_CAP_POS_Y;

        cap_scroll_end = TRUE;
      }

      // フタのスクロールイン
      GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CAP] , &cap_pos , CLSYS_DEFREND_MAIN );


      // ==================================
      // リストモードのときは文字盤を閉じる
      if( p_wk->box_mode == BOXMODE_LIST )
      {
        fontbord_scroll_end = FALSE;

        // 文字盤の移動
        p_wk->fontbord_offset += FONTBORD_SCROLL_SPEED;
        if( FX_Whole(p_wk->fontbord_offset) > 0 )
        {
          p_wk->fontbord_offset = 0;
          fontbord_scroll_end = TRUE;
        }

        // 上画面のスクロール
        GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M) ,GFL_BG_SCROLL_X_SET , FX_Whole(p_wk->fontbord_offset) );
        GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M) ,GFL_BG_SCROLL_X_SET , FX_Whole(p_wk->fontbord_offset) );

      }

      // 次のシーケンスへ移動
      if( cap_scroll_end && fontbord_scroll_end )
      {
        p_wk->sub_seq++;
      }
    }
    break;
  case 2:
    {
    
      // トップページに戻す
      p_wk->top_page_num = 0;

      // ページをリフレッシュ
      PageRefresh( p_wk , FALSE , TRUE);

      // メダル名を非表示に
      ClearMedalName( p_wk );

      // 一押しボタンを非表示に
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI], FALSE );

      p_wk->tmp_frame=0;
      p_wk->sub_seq++;
    }
    break;
  case 3:
    {
      // 演出用待機
      p_wk->tmp_frame++;
      if( p_wk->tmp_frame > HIT_STOP_FRAME )
      {
        p_wk->sub_seq++;
      }
    }
    break;
  case 4:
    {
      BOOL          cap_scroll_end        = FALSE;
      BOOL          fontbord_scroll_end   = TRUE;
      GFL_CLACTPOS  cap_pos;

      // ==================================
      // フタ部分のスクロール
      p_wk->cap_offset -= CAP_SCROLL_SPEED;

      // フタの位置を設定
      cap_pos.x = UI_CAP_POS_X + FX_Whole(p_wk->cap_offset);
      cap_pos.y = UI_CAP_POS_Y;

      // 移動範囲制限
      if( cap_pos.x < -CAP_WIDTH)
      {
        p_wk->cap_offset = FX32_CONST(-CAP_WIDTH);

        // フタの位置を設定
        cap_pos.x = UI_CAP_POS_X + FX_Whole(p_wk->cap_offset);
        cap_pos.y = UI_CAP_POS_Y;

        cap_scroll_end = TRUE;
      }

      // ==================================
      // 文字盤のスクロール
      GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CAP] , &cap_pos , CLSYS_DEFREND_MAIN );


      // リストの時はフタをあける
      if( p_wk->box_mode == BOXMODE_LIST )
      {
        fontbord_scroll_end = FALSE;

        // 文字盤の移動
        p_wk->fontbord_offset -= FONTBORD_SCROLL_SPEED;

        if( FX_Whole(p_wk->fontbord_offset) < -FONTBORD_WIDTH )
        {
          p_wk->fontbord_offset = FX32_CONST(-FONTBORD_WIDTH);
          fontbord_scroll_end = TRUE;
        }

        // 上画面のスクロール
        GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M) ,GFL_BG_SCROLL_X_SET , FX_Whole(p_wk->fontbord_offset) );
        GFL_BG_SetScrollReq( MDLSYS_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M) ,GFL_BG_SCROLL_X_SET , FX_Whole(p_wk->fontbord_offset) );
      }

      // 次のシーケンスへ移動
      if( cap_scroll_end && fontbord_scroll_end )
      {
        // タッチキーで終了した場合はカーソルを表示する
        if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
        {
          // カーソル位置は必ず0番に初期化
          SetCursor( p_wk , 0  , TRUE );
        }
        else
        {
          // カーソル位置を0番に初期化
          p_wk->cursor_pos_tmp = 0;
        }

        // メダル名を表示するためにリフレッシュ
        PageRefresh( p_wk , TRUE , TRUE);
        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_MAIN );
      }
    }
    break;
  }

  return GFL_PROC_RES_CONTINUE;

}

#ifdef PM_DEBUG
// ============================================================
// 
// デバッグ関数
//
// ============================================================

static void DBEG_MDLSYS_DebugInit( MEDALBOXSYS_WORK *p_wk )
{
//  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CAP], FALSE );

}

static void DBEG_MDLSYS_DebugMain( MEDALBOXSYS_WORK *p_wk )
{
  const int trg = GFL_UI_KEY_GetTrg();
  const int cont = GFL_UI_KEY_GetCont();

  switch( p_wk->debgu_seq )
  {
  default:
    {
      p_wk->debgu_seq = 0;
    }
    break;
  }
  
  // デバッグシーケンス切替
  if( trg & PAD_BUTTON_SELECT )
  {
    p_wk->debgu_seq ++;
    
    switch( p_wk->debgu_seq )
    {
    case 0: OS_Printf("デバッグモード：グラフィック操作\n"); break;
    case 1: OS_Printf("デバッグモード：シーケンス切替\n"); break;
    default:OS_Printf("デバッグモード：グラフィック操作\n"); break;
    }
  }
}
#endif


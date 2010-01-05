//============================================================================================
/**
 * @file		box2_bmp.c
 * @brief		ボックス画面 BMP関連
 * @author	Hiroyuki Nakamura
 * @date		09.10.05
 *
 *	モジュール名：BOX2BMP
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "ui/print_tool.h"
#include "item/item.h"

#include "msg/msg_boxmenu.h"
#include "msg/msg_boxmes.h"

#include "box2_main.h"
#include "box2_bmp.h"
#include "box2_obj.h"
#include "box_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================

// タイトル
#define	BMPWIN_TITLE_FRM	( GFL_BG_FRAME0_S )
#define	BMPWIN_TITLE_PX		( 1 )
#define	BMPWIN_TITLE_PY		( 0 )
#define	BMPWIN_TITLE_SX		( 30 )
#define	BMPWIN_TITLE_SY		( 3 )
#define	BMPWIN_TITLE_PAL	( BOX2MAIN_BG_PAL_SYSFNT_S )
// 図鑑番号
#define	BMPWIN_ZKNNUM_FRM	( GFL_BG_FRAME0_S )
#define	BMPWIN_ZKNNUM_PX	( 1 )
#define	BMPWIN_ZKNNUM_PY	( 4 )
#define	BMPWIN_ZKNNUM_SX	( 7 )
#define	BMPWIN_ZKNNUM_SY	( 2 )
#define	BMPWIN_ZKNNUM_PAL	( BOX2MAIN_BG_PAL_SYSFNT_S )
// ポケモン名
#define	BMPWIN_NAME_FRM		( GFL_BG_FRAME0_S )
#define	BMPWIN_NAME_PX		( 1 )
#define	BMPWIN_NAME_PY		( 6 )
#define	BMPWIN_NAME_SX		( 8 )
#define	BMPWIN_NAME_SY		( 2 )
#define	BMPWIN_NAME_PAL		( BOX2MAIN_BG_PAL_SYSFNT_S )
// ニックネーム
#define	BMPWIN_NICKNAME_FRM	( GFL_BG_FRAME0_S )
#define	BMPWIN_NICKNAME_PX	( 1 )
#define	BMPWIN_NICKNAME_PY	( 10 )
#define	BMPWIN_NICKNAME_SX	( 8 )
#define	BMPWIN_NICKNAME_SY	( 2 )
#define	BMPWIN_NICKNAME_PAL	( BOX2MAIN_BG_PAL_SYSFNT_S )
// 性別
#define	BMPWIN_SEX_FRM		( GFL_BG_FRAME0_S )
#define	BMPWIN_SEX_PX			( 9 )
#define	BMPWIN_SEX_PY			( 10 )
#define	BMPWIN_SEX_SX			( 2 )
#define	BMPWIN_SEX_SY			( 2 )
#define	BMPWIN_SEX_PAL		( BOX2MAIN_BG_PAL_SYSFNT_S )
// レベル
#define	BMPWIN_LV_FRM		( GFL_BG_FRAME0_S )
#define	BMPWIN_LV_PX		( 11 )
#define	BMPWIN_LV_PY		( 10 )
#define	BMPWIN_LV_SX		( 6 )
#define	BMPWIN_LV_SY		( 2 )
#define	BMPWIN_LV_PAL		( BOX2MAIN_BG_PAL_SYSFNT_S )
//「せいかく」
#define	BMPWIN_SEIKAKU_STR_FRM	( GFL_BG_FRAME0_S )
#define	BMPWIN_SEIKAKU_STR_PX		( 1 )
#define	BMPWIN_SEIKAKU_STR_PY		( 12 )
#define	BMPWIN_SEIKAKU_STR_SX		( 8 )
#define	BMPWIN_SEIKAKU_STR_SY		( 2 )
#define	BMPWIN_SEIKAKU_STR_PAL	( BOX2MAIN_BG_PAL_SYSFNT_S )
// 性格
#define	BMPWIN_SEIKAKU_FRM	( GFL_BG_FRAME0_S )
#define	BMPWIN_SEIKAKU_PX		( 2 )
#define	BMPWIN_SEIKAKU_PY		( 14 )
#define	BMPWIN_SEIKAKU_SX		( 8 )
#define	BMPWIN_SEIKAKU_SY		( 2 )
#define	BMPWIN_SEIKAKU_PAL	( BOX2MAIN_BG_PAL_SYSFNT_S )
//「とくせい」
#define	BMPWIN_TOKUSEI_STR_FRM	( GFL_BG_FRAME0_S )
#define	BMPWIN_TOKUSEI_STR_PX		( 1 )
#define	BMPWIN_TOKUSEI_STR_PY		( 16 )
#define	BMPWIN_TOKUSEI_STR_SX		( 11 )
#define	BMPWIN_TOKUSEI_STR_SY		( 2 )
#define	BMPWIN_TOKUSEI_STR_PAL	( BOX2MAIN_BG_PAL_SYSFNT_S )
// 特性
#define	BMPWIN_TOKUSEI_FRM	( GFL_BG_FRAME0_S )
#define	BMPWIN_TOKUSEI_PX		( 2 )
#define	BMPWIN_TOKUSEI_PY		( 18 )
#define	BMPWIN_TOKUSEI_SX		( 11 )
#define	BMPWIN_TOKUSEI_SY		( 2 )
#define	BMPWIN_TOKUSEI_PAL	( BOX2MAIN_BG_PAL_SYSFNT_S )
//「もちもの」
#define	BMPWIN_ITEM_STR_FRM		( GFL_BG_FRAME0_S )
#define	BMPWIN_ITEM_STR_PX		( 1 )
#define	BMPWIN_ITEM_STR_PY		( 20 )
#define	BMPWIN_ITEM_STR_SX		( 12 )
#define	BMPWIN_ITEM_STR_SY		( 2 )
#define	BMPWIN_ITEM_STR_PAL		( BOX2MAIN_BG_PAL_SYSFNT_S )
// 持ち物
#define	BMPWIN_ITEM_FRM		( GFL_BG_FRAME0_S )
#define	BMPWIN_ITEM_PX		( 2 )
#define	BMPWIN_ITEM_PY		( 22 )
#define	BMPWIN_ITEM_SX		( 12 )
#define	BMPWIN_ITEM_SY		( 2 )
#define	BMPWIN_ITEM_PAL		( BOX2MAIN_BG_PAL_SYSFNT_S )
//「もっているわざ」
#define	BMPWIN_WAZA_STR_FRM		( GFL_BG_FRAME0_S )
#define	BMPWIN_WAZA_STR_PX		( 19 )
#define	BMPWIN_WAZA_STR_PY		( 14 )
#define	BMPWIN_WAZA_STR_SX		( 11 )
#define	BMPWIN_WAZA_STR_SY		( 2 )
#define	BMPWIN_WAZA_STR_PAL		( BOX2MAIN_BG_PAL_SYSFNT_S )
// 技
#define	BMPWIN_WAZA_FRM		( GFL_BG_FRAME0_S )
#define	BMPWIN_WAZA_PX		( 19 )
#define	BMPWIN_WAZA_PY		( 16 )
#define	BMPWIN_WAZA_SX		( 11 )
#define	BMPWIN_WAZA_SY		( 8 )
#define	BMPWIN_WAZA_PAL		( BOX2MAIN_BG_PAL_SYSFNT_S )

/*
// アイテム名（説明ウィンドウのタブ部分）
#define	BMPWIN_ITEMLABEL_FRM	( GFL_BG_FRAME2_S )
#define	BMPWIN_ITEMLABEL_PX		( 1 )
#define	BMPWIN_ITEMLABEL_PY		( 1 )
#define	BMPWIN_ITEMLABEL_SX		( 12 )
#define	BMPWIN_ITEMLABEL_SY		( 2 )
#define	BMPWIN_ITEMLABEL_PAL	( 1 )
// アイテム説明
#define	BMPWIN_ITEMINFO_FRM		( GFL_BG_FRAME2_S )
#define	BMPWIN_ITEMINFO_PX		( 4 )
#define	BMPWIN_ITEMINFO_PY		( 3 )
#define	BMPWIN_ITEMINFO_SX		( 27 )
#define	BMPWIN_ITEMINFO_SY		( 6 )
#define	BMPWIN_ITEMINFO_PAL		( 1 )
*/

//「てもちポケモン」
#define	BMPWIN_TEMOCHI_FRM		( GFL_BG_FRAME0_M )
#define	BMPWIN_TEMOCHI_PX			( 0 )
#define	BMPWIN_TEMOCHI_PY			( 0 )//( 21 )
#define	BMPWIN_TEMOCHI_SX			( 12 )
#define	BMPWIN_TEMOCHI_SY			( 3 )
#define	BMPWIN_TEMOCHI_PAL		( BOX2MAIN_BG_PAL_SELWIN )
//「ボックスリスト」
#define	BMPWIN_BOXLIST_FRM		( GFL_BG_FRAME0_M )
#define	BMPWIN_BOXLIST_PX			( 0 )//( 12 )
#define	BMPWIN_BOXLIST_PY			( 0 )//( 21 )
#define	BMPWIN_BOXLIST_SX			( 12 )
#define	BMPWIN_BOXLIST_SY			( 3 )
#define	BMPWIN_BOXLIST_PAL		( BOX2MAIN_BG_PAL_SELWIN )
//「もどる」
#define	BMPWIN_MODORU_FRM		( GFL_BG_FRAME1_M )
#define	BMPWIN_MODORU_PX		( 0 )//( 24 )
#define	BMPWIN_MODORU_PY		( 0 )//( 21 )
#define	BMPWIN_MODORU_SX		( 8 )
#define	BMPWIN_MODORU_SY		( 3 )
#define	BMPWIN_MODORU_PAL		( BOX2MAIN_BG_PAL_SELWIN )
//「やめる」
#define	BMPWIN_YAMERU_FRM		( GFL_BG_FRAME1_M )
#define	BMPWIN_YAMERU_PX		( 0 )//( 24 )
#define	BMPWIN_YAMERU_PY		( 0 )//( 36 )
#define	BMPWIN_YAMERU_SX		( BMPWIN_MODORU_SX )
#define	BMPWIN_YAMERU_SY		( BMPWIN_MODORU_SY )
#define	BMPWIN_YAMERU_PAL		( BOX2MAIN_BG_PAL_SELWIN )
//「とじる」
#define	BMPWIN_TOZIRU_FRM		( GFL_BG_FRAME1_M )
#define	BMPWIN_TOZIRU_PX		( 0 )//( 24 )
#define	BMPWIN_TOZIRU_PY		( 0 )//( 21 )
#define	BMPWIN_TOZIRU_SX		( BMPWIN_MODORU_SX )
#define	BMPWIN_TOZIRU_SY		( BMPWIN_MODORU_SY )
#define	BMPWIN_TOZIRU_PAL		( BOX2MAIN_BG_PAL_SELWIN )

/*
// 手持ちポケモンフレーム・いれかえ
#define	BMPWIN_PARTY_CNG_FRM	( GFL_BG_FRAME1_M )
#define	BMPWIN_PARTY_CNG_PX		( 2 )
#define	BMPWIN_PARTY_CNG_PY		( 15 )
#define	BMPWIN_PARTY_CNG_SX		( 7 )
#define	BMPWIN_PARTY_CNG_SY		( 2 )
#define	BMPWIN_PARTY_CNG_PAL	( 1 )
*/
/*
//「ボックスをきりかえる」ボタン
#define	BMPWIN_BOXMV_BTN_FRM	( GFL_BG_FRAME1_M )
#define	BMPWIN_BOXMV_BTN_PX		( 0 )
#define	BMPWIN_BOXMV_BTN_PY		( 0 )//( 21 )
#define	BMPWIN_BOXMV_BTN_SX		( 17 )
#define	BMPWIN_BOXMV_BTN_SY		( 3 )
#define	BMPWIN_BOXMV_BTN_PAL	( BOX2MAIN_BG_PAL_SELWIN )

// Ｙボタンステータス
#define	BMPWIN_Y_ST_BTN_FRM		( GFL_BG_FRAME1_M )
#define	BMPWIN_Y_ST_BTN_PX		( 0 )
#define	BMPWIN_Y_ST_BTN_PY		( 0 )
#define	BMPWIN_Y_ST_BTN_SX		( 17 )
#define	BMPWIN_Y_ST_BTN_SY		( 3 )
#define	BMPWIN_Y_ST_BTN_PAL		( 2 )
*/

// メニュー
#define	BMPWIN_MENU_FRM		( GFL_BG_FRAME1_M )//( GFL_BG_FRAME0_M )
#define	BMPWIN_MENU_PX		( 0 )//( 21 )
#define	BMPWIN_MENU_PY		( 0 )//( 8 )
#define	BMPWIN_MENU_SX		( 11 )
#define	BMPWIN_MENU_SY		( 3 )
#define	BMPWIN_MENU_PAL		( BOX2MAIN_BG_PAL_SELWIN )

// マーキング決定
#define	BMPWIN_MARK_ENTER_FRM		( GFL_BG_FRAME1_M )//( GFL_BG_FRAME0_M )
//#define	BMPWIN_MARK_ENTER_PX		( 2 )
//#define	BMPWIN_MARK_ENTER_PY		( 9 )
//#define	BMPWIN_MARK_ENTER_SX		( 7 )
//#define	BMPWIN_MARK_ENTER_SY		( 3 )
#define	BMPWIN_MARK_ENTER_PX		( 0 )
#define	BMPWIN_MARK_ENTER_PY		( 9 )
#define	BMPWIN_MARK_ENTER_SX		( 11 )
#define	BMPWIN_MARK_ENTER_SY		( 3 )
#define	BMPWIN_MARK_ENTER_PAL		( 1 )
// マーキングキャンセル
#define	BMPWIN_MARK_CANCEL_FRM	( GFL_BG_FRAME1_M )//( GFL_BG_FRAME0_M )
//#define	BMPWIN_MARK_CANCEL_PX		( 2 )
//#define	BMPWIN_MARK_CANCEL_PY		( 12 )
//#define	BMPWIN_MARK_CANCEL_SX		( 7 )
//#define	BMPWIN_MARK_CANCEL_SY		( 3 )
#define	BMPWIN_MARK_CANCEL_PX		( 0 )
#define	BMPWIN_MARK_CANCEL_PY		( 12 )
#define	BMPWIN_MARK_CANCEL_SX		( 11 )
#define	BMPWIN_MARK_CANCEL_SY		( 3 )
#define	BMPWIN_MARK_CANCEL_PAL	( 1 )

// メッセージ１
#define	BMPWIN_MSG1_FRM		( GFL_BG_FRAME0_M )
#define	BMPWIN_MSG1_PX		( 1 )
#define	BMPWIN_MSG1_PY		( 21 )
#define	BMPWIN_MSG1_SX		( 30 )
#define	BMPWIN_MSG1_SY		( 2 )
#define	BMPWIN_MSG1_PAL		( BOX2MAIN_BG_PAL_SYSFNT )
// メッセージ２
#define	BMPWIN_MSG2_PX		( 1 )
#define	BMPWIN_MSG2_PY		( 19 )
#define	BMPWIN_MSG2_SX		( 18 )
#define	BMPWIN_MSG2_SY		( 4 )
// メッセージ３
#define	BMPWIN_MSG3_SX		( 19 )
#define	BMPWIN_MSG3_SY		( 2 )
// メッセージ４
#define	BMPWIN_MSG4_PX		( 1 )
#define	BMPWIN_MSG4_PY		( 19 )
#define	BMPWIN_MSG4_SX		( 30 )
#define	BMPWIN_MSG4_SY		( 4 )

/*
// ボックス移動のメニュー
#define	BMPWIN_BOXMV_MENU_FRM	( GFL_BG_FRAME1_M )//( GFL_BG_FRAME0_M )
#define	BMPWIN_BOXMV_MENU_PX	( 22 )
#define	BMPWIN_BOXMV_MENU_PY	( 16 )
#define	BMPWIN_BOXMV_MENU_SX	( 9 )
#define	BMPWIN_BOXMV_MENU_SY	( 4 )
#define	BMPWIN_BOXMV_MENU_PAL	( BOX2MAIN_BG_PAL_SELWIN )

// トレイ名
#define	BMPWIN_TRAY_NAME_FRM	( GFL_BG_FRAME3_M )
#define	BMPWIN_TRAY_NAME_PX		( 0 )
#define	BMPWIN_TRAY_NAME_PY		( 2 )
#define	BMPWIN_TRAY_NAME_SX		( 21 )
#define	BMPWIN_TRAY_NAME_SY		( 3 )
#define	BMPWIN_TRAY_NAME_PAL	( 0 )
*/

//#define	FNTCOL_SELWIN_ON_B	( 11 )		// ボタン塗りつぶしカラー番号（選択時）
#define	FNTCOL_SELWIN_OFF_B	( 4 )		// ボタン塗りつぶしカラー番号（非選択時）

#define	FRMBUTTON_BG_COL	( 13 )		// フレーム内ボタンの背景カラー

/*
#define	FNTCOL_FNTOAM		( PRINTSYS_LSB_Make( 1, 2, 0 ) )		// フォントカラー：黒抜
#define	FNTCOL_SELWIN_ON	( PRINTSYS_LSB_Make(14,15,0) )			// フォントカラー：ボタン選択時
*/
#define	FCOL_N_BLACK			( PRINTSYS_LSB_Make(1,2,0) )				// フォントカラー：黒抜
#define	FCOL_W_BLACK			( PRINTSYS_LSB_Make(1,2,15) )				// フォントカラー：黒白
#define	FCOL_N_BLUE				( PRINTSYS_LSB_Make(5,6,0) )				// フォントカラー：青抜
#define	FCOL_N_RED				( PRINTSYS_LSB_Make(3,4,0) )				// フォントカラー：赤抜
//#define	FCOL_TRAYNAME			( PRINTSYS_LSB_Make(2,1,0) )				// フォントカラー：ボックス名
//#define	FCOL_SELWIN_OFF		( PRINTSYS_LSB_Make(14,15,0) )			// フォントカラー：ボタン非選択時
#define	FCOL_SELWIN_OFF		( PRINTSYS_LSB_Make(15,12,0) )			// フォントカラー：ボタン非選択時
//#define	FCOL_YSTWIN				( PRINTSYS_LSB_Make(9,10,0) )				// フォントカラー：Ｙステータスボタン
//#define	FCOL_YSTWIN				( PRINTSYS_LSB_Make(10,15,0) )				// フォントカラー：Ｙステータスボタン
//#define	FCOL_RED_BUTTON		( PRINTSYS_LSB_Make(10,12,0) )				// フォントカラー：赤ボタン
//#define	FCOL_FRMBUTTN_OFF	( PRINTSYS_LSB_Make(9,10,FRMBUTTON_BG_COL) )	// フォントカラー：フレーム内ボタン
//#define	FCOL_FRMBUTTN_OFF	( PRINTSYS_LSB_Make(10,15,FRMBUTTON_BG_COL) )	// フォントカラー：フレーム内ボタン
//#define	FCOL_SUB_WIN			( PRINTSYS_LSB_Make(1,2,0) )				// フォントカラー：上画面ウィンドウ用

#define	FCOL_FNTOAM				( PRINTSYS_LSB_Make(1,2,0) )		// フォントカラー：ＯＡＭフォント黒抜
#define	FCOL_FNTOAM_RED		( PRINTSYS_LSB_Make(5,4,0) )		// フォントカラー：ＯＡＭフォント赤抜


/*
// メッセージ表示モード
enum {
	STRPRINT_MODE_LEFT = 0,		// 左詰
	STRPRINT_MODE_RIGHT,		// 右詰
	STRPRINT_MODE_CENTER		// 中央
};
*/

#define	POKE_NOMARK_PY	( 3 )		//「No.」表示Y座標
#define	POKE_LVMARK_PY	( 3 )		// レベル表示Ｙ座標

#define	TITLE_STR_PY	( 8 )	// タイトル表示Y座標

/*
//「ポケモンいどう」ボタンキャラ
#define	PMV_UL_CGX_POS	( 0x57 * 0x20 )
#define	PMV_CL_CGX_POS	( 0x5b * 0x20 )
#define	PMV_DL_CGX_POS	( 0x59 * 0x20 )
#define	PMV_UC_CGX_POS	( 0x58 * 0x20 )
#define	PMV_CC_CGX_POS	( 0x5c * 0x20 )
#define	PMV_DC_CGX_POS	( 0x5a * 0x20 )
#define	PMV_UR_CGX_POS	( 0x6b * 0x20 )
#define	PMV_CR_CGX_POS	( 0x69 * 0x20 )
#define	PMV_DR_CGX_POS	( 0x6a * 0x20 )
*/


//============================================================================================
//	グローバル
//============================================================================================
static const u8	BoxBmpWinData[][6] =
{
/** 上画面 **/
	{	// ポケモン名
		BMPWIN_NAME_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_PAL,
	},
	{	// ニックネーム
		BMPWIN_NICKNAME_FRM, BMPWIN_NICKNAME_PX, BMPWIN_NICKNAME_PY,
		BMPWIN_NICKNAME_SX, BMPWIN_NICKNAME_SY, BMPWIN_NICKNAME_PAL,
	},
	{	// レベル
		BMPWIN_LV_FRM, BMPWIN_LV_PX, BMPWIN_LV_PY,
		BMPWIN_LV_SX, BMPWIN_LV_SY, BMPWIN_LV_PAL,
	},
	{	// タイトル
		BMPWIN_TITLE_FRM, BMPWIN_TITLE_PX, BMPWIN_TITLE_PY,
		BMPWIN_TITLE_SX, BMPWIN_TITLE_SY, BMPWIN_TITLE_PAL,
	},

	{	// 性別
		BMPWIN_SEX_FRM, BMPWIN_SEX_PX, BMPWIN_SEX_PY,
		BMPWIN_SEX_SX, BMPWIN_SEX_SY, BMPWIN_SEX_PAL,
	},
	{	// 図鑑番号
		BMPWIN_ZKNNUM_FRM, BMPWIN_ZKNNUM_PX, BMPWIN_ZKNNUM_PY,
		BMPWIN_ZKNNUM_SX, BMPWIN_ZKNNUM_SY, BMPWIN_ZKNNUM_PAL,
	},
	{	// 性格
		BMPWIN_SEIKAKU_FRM, BMPWIN_SEIKAKU_PX, BMPWIN_SEIKAKU_PY,
		BMPWIN_SEIKAKU_SX, BMPWIN_SEIKAKU_SY, BMPWIN_SEIKAKU_PAL,
	},
	{	// 特性
		BMPWIN_TOKUSEI_FRM, BMPWIN_TOKUSEI_PX, BMPWIN_TOKUSEI_PY,
		BMPWIN_TOKUSEI_SX, BMPWIN_TOKUSEI_SY, BMPWIN_TOKUSEI_PAL,
	},
	{	// 持ち物
		BMPWIN_ITEM_FRM, BMPWIN_ITEM_PX, BMPWIN_ITEM_PY,
		BMPWIN_ITEM_SX, BMPWIN_ITEM_SY, BMPWIN_ITEM_PAL,
	},
	{	// 技
		BMPWIN_WAZA_FRM, BMPWIN_WAZA_PX, BMPWIN_WAZA_PY,
		BMPWIN_WAZA_SX, BMPWIN_WAZA_SY, BMPWIN_WAZA_PAL,
	},
	{	//「もっているわざ」
		BMPWIN_WAZA_STR_FRM, BMPWIN_WAZA_STR_PX, BMPWIN_WAZA_STR_PY,
		BMPWIN_WAZA_STR_SX, BMPWIN_WAZA_STR_SY, BMPWIN_WAZA_STR_PAL,
	},
	{	//「せいかく」
		BMPWIN_SEIKAKU_STR_FRM, BMPWIN_SEIKAKU_STR_PX, BMPWIN_SEIKAKU_STR_PY,
		BMPWIN_SEIKAKU_STR_SX, BMPWIN_SEIKAKU_STR_SY, BMPWIN_SEIKAKU_STR_PAL,
	},
	{	//「とくせい」
		BMPWIN_TOKUSEI_STR_FRM, BMPWIN_TOKUSEI_STR_PX, BMPWIN_TOKUSEI_STR_PY,
		BMPWIN_TOKUSEI_STR_SX, BMPWIN_TOKUSEI_STR_SY, BMPWIN_TOKUSEI_STR_PAL,
	},
	{	//「もちもの」
		BMPWIN_ITEM_STR_FRM, BMPWIN_ITEM_STR_PX, BMPWIN_ITEM_STR_PY,
		BMPWIN_ITEM_STR_SX, BMPWIN_ITEM_STR_SY, BMPWIN_ITEM_STR_PAL,
	},

/*
	{	// アイテム名（説明ウィンドウのタブ部分）
		BMPWIN_ITEMLABEL_FRM, BMPWIN_ITEMLABEL_PX, BMPWIN_ITEMLABEL_PY,
		BMPWIN_ITEMLABEL_SX, BMPWIN_ITEMLABEL_SY, BMPWIN_ITEMLABEL_PAL,
	},
	{	// アイテム説明
		BMPWIN_ITEMINFO_FRM, BMPWIN_ITEMINFO_PX, BMPWIN_ITEMINFO_PY,
		BMPWIN_ITEMINFO_SX, BMPWIN_ITEMINFO_SY,BMPWIN_ITEMINFO_PAL,
	},
	{	// アイテム名（説明ウィンドウのタブ部分）（スワップ用）
		BMPWIN_ITEMLABEL_FRM, BMPWIN_ITEMLABEL_PX, BMPWIN_ITEMLABEL_PY,
		BMPWIN_ITEMLABEL_SX, BMPWIN_ITEMLABEL_SY, BMPWIN_ITEMLABEL_PAL,
	},
	{	// アイテム説明（スワップ用）
		BMPWIN_ITEMINFO_FRM, BMPWIN_ITEMINFO_PX, BMPWIN_ITEMINFO_PY,
		BMPWIN_ITEMINFO_SX, BMPWIN_ITEMINFO_SY,BMPWIN_ITEMINFO_PAL,
	},
*/

/** 下画面 **/
	{	// メニュー１
		BMPWIN_MENU_FRM, BMPWIN_MENU_PX, BMPWIN_MENU_PY,
		BMPWIN_MENU_SX, BMPWIN_MENU_SY, BMPWIN_MENU_PAL,
	},
	{	// メニュー２
		BMPWIN_MENU_FRM, BMPWIN_MENU_PX, BMPWIN_MENU_PY,
		BMPWIN_MENU_SX, BMPWIN_MENU_SY, BMPWIN_MENU_PAL,
	},
	{	// メニュー３
		BMPWIN_MENU_FRM, BMPWIN_MENU_PX, BMPWIN_MENU_PY,
		BMPWIN_MENU_SX, BMPWIN_MENU_SY, BMPWIN_MENU_PAL,
	},
	{	// メニュー４
		BMPWIN_MENU_FRM, BMPWIN_MENU_PX, BMPWIN_MENU_PY,
		BMPWIN_MENU_SX, BMPWIN_MENU_SY, BMPWIN_MENU_PAL,
	},
	{	// メニュー５
		BMPWIN_MENU_FRM, BMPWIN_MENU_PX, BMPWIN_MENU_PY,
		BMPWIN_MENU_SX, BMPWIN_MENU_SY, BMPWIN_MENU_PAL,
	},
	{	// メニュー６
		BMPWIN_MENU_FRM, BMPWIN_MENU_PX, BMPWIN_MENU_PY,
		BMPWIN_MENU_SX, BMPWIN_MENU_SY, BMPWIN_MENU_PAL,
	},

	{	// マーキング決定
		BMPWIN_MARK_ENTER_FRM, BMPWIN_MARK_ENTER_PX, BMPWIN_MARK_ENTER_PY,
		BMPWIN_MARK_ENTER_SX, BMPWIN_MARK_ENTER_SY, BMPWIN_MARK_ENTER_PAL,
	},
	{	// マーキングキャンセル
		BMPWIN_MARK_CANCEL_FRM, BMPWIN_MARK_CANCEL_PX, BMPWIN_MARK_CANCEL_PY,
		BMPWIN_MARK_CANCEL_SX, BMPWIN_MARK_CANCEL_SY, BMPWIN_MARK_CANCEL_PAL,
	},

	{	//「てもちポケモン」
		BMPWIN_TEMOCHI_FRM, BMPWIN_TEMOCHI_PX, BMPWIN_TEMOCHI_PY,
		BMPWIN_TEMOCHI_SX, BMPWIN_TEMOCHI_SY, BMPWIN_TEMOCHI_PAL,
	},
	{	//「ボックスリスト」
		BMPWIN_BOXLIST_FRM, BMPWIN_BOXLIST_PX, BMPWIN_BOXLIST_PY,
		BMPWIN_BOXLIST_SX, BMPWIN_BOXLIST_SY, BMPWIN_BOXLIST_PAL,
	},

	{	// メッセージ１
		BMPWIN_MSG1_FRM, BMPWIN_MSG1_PX, BMPWIN_MSG1_PY,
		BMPWIN_MSG1_SX, BMPWIN_MSG1_SY, BMPWIN_MSG1_PAL,
	},
	{	// メッセージ２
		BMPWIN_MSG1_FRM, BMPWIN_MSG2_PX, BMPWIN_MSG2_PY,
		BMPWIN_MSG2_SX, BMPWIN_MSG2_SY, BMPWIN_MSG1_PAL,
	},
	{	// メッセージ３
		BMPWIN_MSG1_FRM, BMPWIN_MSG1_PX, BMPWIN_MSG1_PY,
		BMPWIN_MSG3_SX, BMPWIN_MSG3_SY, BMPWIN_MSG1_PAL,
	},
	{	// メッセージ４
		BMPWIN_MSG1_FRM, BMPWIN_MSG4_PX, BMPWIN_MSG4_PY,
		BMPWIN_MSG4_SX, BMPWIN_MSG4_SY, BMPWIN_MSG1_PAL,
	},
};








//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ関連初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_Init( BOX2_SYS_WORK * syswk )
{
	const u8 * dat;
	u32	i;

	GFL_BMPWIN_Init( HEAPID_BOX_APP );

	dat = BoxBmpWinData[0];
	for( i=0; i<BOX2BMPWIN_ID_MAX; i++ ){
		syswk->app->win[i].win = GFL_BMPWIN_Create(
															dat[0], dat[1], dat[2],
															dat[3], dat[4], dat[5],
//															GFL_BMP_CHRAREA_GET_F );
															GFL_BMP_CHRAREA_GET_B );
		dat += 6;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ関連削除
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_Exit( BOX2_SYS_WORK * syswk )
{
/*
	u16	i;

	for( i=0; i<BOX2BMPWIN_ID_MAX; i++ ){
		GFL_BMPWIN_Delete( &syswk->app->win[i] );
	}

	FontProc_UnloadFont( FONT_TOUCH );
*/
	u32	i;

	for( i=0; i<BOX2BMPWIN_ID_MAX; i++ ){
		GFL_BMPWIN_Delete( syswk->app->win[i].win );
	}

	GFL_BMPWIN_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		プリントメイン
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_PrintUtilTrans( BOX2_APP_WORK * appwk )
{
	u32	i;

	PRINTSYS_QUE_Main( appwk->que );
	for( i=0; i<BOX2BMPWIN_ID_MAX; i++ ){
		PRINT_UTIL_Trans( &appwk->win[i], appwk->que );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BMPWINスクリーン転送
 *
 * @param		util		PRINT_UTIL
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PrintScreenTrans( PRINT_UTIL * util )
{
	GFL_BMPWIN_MakeScreen( util->win );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(util->win) );
}

static void ClearScreen( PRINT_UTIL * util )
{
	GFL_BMPWIN_ClearScreen( util->win );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(util->win) );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列表示（固定文字列）
 *
 * @param		appwk		ボックス画面アプリワーク
 * @param		winID		BMPWIN ID
 * @param		man			メッセージマネージャ
 * @param		msgID		文字列ＩＤ
 * @param		x				表示基準Ｘ座標
 * @param		y				表示基準Ｙ座標
 * @param		font		フォント
 * @param		col			カラー
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void StrPrint(
				BOX2_APP_WORK * appwk, u32 winID,
				GFL_MSGDATA * man, u32 msgID, u32 x, u32 y, GFL_FONT * font, PRINTSYS_LSB col )
{
	STRBUF * str = GFL_MSG_CreateString( man, msgID );

	PRINT_UTIL_PrintColor( &appwk->win[winID], appwk->que, x, y, str, font, col );

	GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列表示（可変文字列）
 *
 * @param		appwk		ボックス画面アプリワーク
 * @param		winID		BMPWIN ID
 * @param		man			メッセージマネージャ
 * @param		msgID		文字列ＩＤ
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 * @param		font		フォント
 * @param		col			カラー
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExStrPrint(
				BOX2_APP_WORK * appwk, 	u32 winID,
				GFL_MSGDATA * man, u32 msgID, u32 x, u32 y, GFL_FONT * font, PRINTSYS_LSB col )
{
	STRBUF * str = GFL_MSG_CreateString( man, msgID );

	WORDSET_ExpandStr( appwk->wset, appwk->exp, str );

	PRINT_UTIL_PrintColor( &appwk->win[winID], appwk->que, x, y, appwk->exp, font, col );

	GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ関連のメッセージマネージャー作成
 *
 * @param		none
 *
 * @return	メッセージマネージャー
 */
//--------------------------------------------------------------------------------------------
static GFL_MSGDATA * BoxMsgManGet(void)
{
	return GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_boxmes_dat, HEAPID_BOX_APP );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		タイトル表示
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_TitlePut( BOX2_SYS_WORK * syswk )
{
	BOX2_APP_WORK * appwk;
	GFL_MSGDATA * man;

	appwk = syswk->app;

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[BOX2BMPWIN_ID_TITLE].win), 0 );

	man = BoxMsgManGet();

	StrPrint(
		appwk, BOX2BMPWIN_ID_TITLE, man,
		msg_boxmes_03_01+syswk->dat->callMode,
		0, TITLE_STR_PY, appwk->font, FCOL_N_BLACK );

	GFL_MSG_Delete( man );

	PrintScreenTrans( &syswk->app->win[BOX2BMPWIN_ID_TITLE] );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		デフォルト文字列表示
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_DefStrPut( BOX2_SYS_WORK * syswk )
{
	BOX2_APP_WORK * appwk;

	appwk = syswk->app;

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[BOX2BMPWIN_ID_WAZA_STR].win), 0 );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[BOX2BMPWIN_ID_SEIKAKU_STR].win), 0 );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[BOX2BMPWIN_ID_TOKUSEI_STR].win), 0 );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[BOX2BMPWIN_ID_ITEM_STR].win), 0 );

	StrPrint(
		appwk, BOX2BMPWIN_ID_WAZA_STR, appwk->mman,
		mes_box_subst_01_15, 0, 0, appwk->font, FCOL_N_BLACK );

	StrPrint(
		appwk, BOX2BMPWIN_ID_SEIKAKU_STR, appwk->mman,
		mes_box_subst_01_06, 0, 0, appwk->font, FCOL_N_BLACK );

	StrPrint(
		appwk, BOX2BMPWIN_ID_TOKUSEI_STR, appwk->mman,
		mes_box_subst_01_07, 0, 0, appwk->font, FCOL_N_BLACK );

	StrPrint(
		appwk, BOX2BMPWIN_ID_ITEM_STR, appwk->mman,
		mes_box_subst_01_08, 0, 0, appwk->font, FCOL_N_BLACK );

/*
	PrintScreenTrans( &syswk->app->win[BOX2BMPWIN_ID_WAZA_STR] );
	PrintScreenTrans( &syswk->app->win[BOX2BMPWIN_ID_SEIKAKU_STR] );
	PrintScreenTrans( &syswk->app->win[BOX2BMPWIN_ID_TOKUSEI_STR] );
	PrintScreenTrans( &syswk->app->win[BOX2BMPWIN_ID_ITEM_STR] );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモン名表示
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	info	表示データ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeNamePut( BOX2_APP_WORK * appwk, BOX2_POKEINFO_DATA * info, u32 winID )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[winID].win), 0 );

	if( info->tamago == 0 ){
		WORDSET_RegisterPokeMonsNamePPP( appwk->wset, 0, info->ppp );
		ExStrPrint(
			appwk, winID, appwk->mman, mes_boxmenu_01_01, 0, 0, appwk->font, FCOL_N_BLACK );
	}else{
		GFL_BMPWIN_TransVramCharacter( appwk->win[winID].win );
		return;
	}

	PrintScreenTrans( &appwk->win[winID] );
}

//--------------------------------------------------------------------------------------------
/**
 * ニックネーム表示
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	info	表示データ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeNickNamePut( BOX2_APP_WORK * appwk, BOX2_POKEINFO_DATA * info, u32 winID )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[winID].win), 0 );

	WORDSET_RegisterPokeNickNamePPP( appwk->wset, 0, info->ppp );
	ExStrPrint(
		appwk, winID, appwk->mman, mes_boxmenu_01_03, 0, 0, appwk->font, FCOL_N_BLACK );

	PrintScreenTrans( &appwk->win[winID] );
}

//--------------------------------------------------------------------------------------------
/**
 * レベル表示
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	info	表示データ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeLvPut( BOX2_APP_WORK * appwk, BOX2_POKEINFO_DATA * info, u32 winID )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[winID].win), 0 );

	if( info->tamago == 0 ){
		u32	npx;

		ExStrPrint(
			appwk, winID, appwk->mman, mes_box_subst_01_13, 0, POKE_LVMARK_PY, appwk->nfnt, FCOL_N_BLACK );
		npx = PRINTSYS_GetStrWidth( appwk->exp, appwk->nfnt, 0 );
		WORDSET_RegisterNumber(
			appwk->wset, 0, info->lv, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
		ExStrPrint(
			appwk, winID, appwk->mman, mes_box_subst_01_09, npx, 0, appwk->font, FCOL_N_BLACK );
	}else{
		GFL_BMPWIN_TransVramCharacter( appwk->win[winID].win );
		return;
	}

	PrintScreenTrans( &appwk->win[winID] );
}

//--------------------------------------------------------------------------------------------
/**
 * 性別表示
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	info	表示データ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeSexPut( BOX2_APP_WORK * appwk, BOX2_POKEINFO_DATA * info, u32 winID )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[winID].win), 0 );

	if( info->tamago == 0 && info->sex_put == 1 ){
		if( info->sex == PTL_SEX_MALE ){
			StrPrint(
				appwk, winID, appwk->mman, mes_box_subst_01_01, 0, 0, appwk->font, FCOL_N_BLUE );
		}else if( info->sex == PTL_SEX_FEMALE ){
			StrPrint(
				appwk, winID, appwk->mman, mes_box_subst_01_02, 0, 0, appwk->font, FCOL_N_RED );
		}else{
			GFL_BMPWIN_TransVramCharacter( appwk->win[winID].win );
			return;
		}
	}else{
		GFL_BMPWIN_TransVramCharacter( appwk->win[winID].win );
		return;
	}

	PrintScreenTrans( &appwk->win[winID] );
}

//--------------------------------------------------------------------------------------------
/**
 * 図鑑番号表示
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	info	表示データ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeNumberPut( BOX2_SYS_WORK * syswk, BOX2_POKEINFO_DATA * info, u32 winID )
{
	u32	mode;
	u32	num;
	
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(syswk->app->win[winID].win), 0 );

	if( info->tamago == 0 ){
//		mode = PMNumber_GetMode( syswk->dat->savedata );
//		num  = PMNumber_GetPokeNumber( mode, info->mons );
		num = 999;

		if( num != 0 ){
			u32	npx;

			ExStrPrint(
				syswk->app, winID, syswk->app->mman,
				mes_box_subst_01_14, 0, POKE_NOMARK_PY, syswk->app->nfnt, FCOL_N_BLACK );
			npx = PRINTSYS_GetStrWidth( syswk->app->exp, syswk->app->nfnt, 0 );
			WORDSET_RegisterNumber(
				syswk->app->wset, 0, num, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
			ExStrPrint(
				syswk->app, winID, syswk->app->mman,
				mes_box_subst_01_10, npx, 0, syswk->app->font, FCOL_N_BLACK );
		}else{
			GFL_BMPWIN_TransVramCharacter( syswk->app->win[winID].win );
			return;
		}
	}else{
		GFL_BMPWIN_TransVramCharacter( syswk->app->win[winID].win );
		return;
	}

	PrintScreenTrans( &syswk->app->win[winID] );
}

//--------------------------------------------------------------------------------------------
/**
 * 性格表示
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	info	表示データ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeSeikakuPut( BOX2_APP_WORK * appwk, BOX2_POKEINFO_DATA * info, u32 winID )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[winID].win), 0 );

	if( info->tamago == 0 ){
		WORDSET_RegisterSeikaku( appwk->wset, 0, info->seikaku );
		ExStrPrint(
			appwk, winID, appwk->mman,
			mes_box_subst_01_04, 0, 0, appwk->font, FCOL_N_BLACK );
	}else{
		StrPrint(
			appwk, winID, appwk->mman,
			mes_box_subst_01_12, 0, 0, appwk->font, FCOL_N_BLACK );
	}

	PrintScreenTrans( &appwk->win[winID] );
}

//--------------------------------------------------------------------------------------------
/**
 * 特性表示
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	info	表示データ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeTokuseiPut( BOX2_APP_WORK * appwk, BOX2_POKEINFO_DATA * info, u32 winID )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[winID].win), 0 );

	if( info->tamago == 0 ){
		WORDSET_RegisterTokuseiName( appwk->wset, 0, info->tokusei );
		ExStrPrint(
			appwk, winID, appwk->mman,
			mes_box_subst_01_03, 0, 0, appwk->font, FCOL_N_BLACK );
	}else{
		StrPrint(
			appwk, winID, appwk->mman,
			mes_box_subst_01_12, 0, 0, appwk->font, FCOL_N_BLACK );
	}

	PrintScreenTrans( &appwk->win[winID] );
}

//--------------------------------------------------------------------------------------------
/**
 * 持ち物表示
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	info	表示データ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeItemPut( BOX2_APP_WORK * appwk, BOX2_POKEINFO_DATA * info, u32 winID )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[winID].win), 0 );

	if( info->item != ITEM_DUMMY_DATA ){
		WORDSET_RegisterItemName( appwk->wset, 0, info->item );
		ExStrPrint(
			appwk, winID, appwk->mman,
			mes_box_subst_01_05, 0, 0, appwk->font, FCOL_N_BLACK );
	}else{
		StrPrint(
			appwk, winID, appwk->mman,
			mes_box_subst_01_11, 0, 0, appwk->font, FCOL_N_BLACK );
	}

	PrintScreenTrans( &appwk->win[winID] );
}

static void PokeWazaPut( BOX2_APP_WORK * appwk, BOX2_POKEINFO_DATA * info, u32 winID )
{
	GFL_MSGDATA * man;
	u32	i;

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[winID].win), 0 );

	man = GFL_MSG_Create(
					GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wazaname_dat, HEAPID_BOX_APP_L );

	if( info->tamago == 0 ){
		for( i=0; i<4; i++ ){
			StrPrint( appwk, winID, man, info->waza[i], 0, i*16, appwk->font, FCOL_N_BLACK );
		}
	}else{
		StrPrint( appwk, winID, man, mes_box_subst_01_12, 0, 0, appwk->font, FCOL_N_BLACK );
	}

	GFL_MSG_Delete( man );

	PrintScreenTrans( &appwk->win[winID] );
}

//--------------------------------------------------------------------------------------------
/**
 * 上画面ポケモンデータ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	info	表示データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_PokeDataPut( BOX2_SYS_WORK * syswk, BOX2_POKEINFO_DATA * info )
{
	PokeNamePut( syswk->app, info, BOX2BMPWIN_ID_NAME );
	PokeNickNamePut( syswk->app, info, BOX2BMPWIN_ID_NICKNAME );
	PokeLvPut( syswk->app, info, BOX2BMPWIN_ID_LV );
	PokeSexPut( syswk->app, info, BOX2BMPWIN_ID_SEX );
	PokeNumberPut( syswk, info, BOX2BMPWIN_ID_ZKNNUM );
	PokeSeikakuPut( syswk->app, info, BOX2BMPWIN_ID_SEIKAKU );
	PokeTokuseiPut( syswk->app, info, BOX2BMPWIN_ID_TOKUSEI );
	PokeItemPut( syswk->app, info, BOX2BMPWIN_ID_ITEM );
	PokeWazaPut( syswk->app, info, BOX2BMPWIN_ID_WAZA );

	PrintScreenTrans( &syswk->app->win[BOX2BMPWIN_ID_WAZA_STR] );
	PrintScreenTrans( &syswk->app->win[BOX2BMPWIN_ID_SEIKAKU_STR] );
	PrintScreenTrans( &syswk->app->win[BOX2BMPWIN_ID_TOKUSEI_STR] );
	PrintScreenTrans( &syswk->app->win[BOX2BMPWIN_ID_ITEM_STR] );
}

//--------------------------------------------------------------------------------------------
/**
 * 上画面ポケモンデータ非表示
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_PokeDataOff( BOX2_APP_WORK * appwk )
{
	ClearScreen( &appwk->win[BOX2BMPWIN_ID_NAME] );
	ClearScreen( &appwk->win[BOX2BMPWIN_ID_NICKNAME] );
	ClearScreen( &appwk->win[BOX2BMPWIN_ID_LV] );
	ClearScreen( &appwk->win[BOX2BMPWIN_ID_SEX] );
	ClearScreen( &appwk->win[BOX2BMPWIN_ID_ZKNNUM] );
	ClearScreen( &appwk->win[BOX2BMPWIN_ID_SEIKAKU] );
	ClearScreen( &appwk->win[BOX2BMPWIN_ID_TOKUSEI] );
	ClearScreen( &appwk->win[BOX2BMPWIN_ID_ITEM] );
	ClearScreen( &appwk->win[BOX2BMPWIN_ID_WAZA] );

	ClearScreen( &appwk->win[BOX2BMPWIN_ID_WAZA_STR] );
	ClearScreen( &appwk->win[BOX2BMPWIN_ID_SEIKAKU_STR] );
	ClearScreen( &appwk->win[BOX2BMPWIN_ID_TOKUSEI_STR] );
	ClearScreen( &appwk->win[BOX2BMPWIN_ID_ITEM_STR] );
}

//--------------------------------------------------------------------------------------------
/**
 * 上画面技表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	info	表示データ
 *
 * @return	表示したBMPWINのID
 */
//--------------------------------------------------------------------------------------------
/*
u32 BOX2BMP_PokeSkillWrite( BOX2_SYS_WORK * syswk, BOX2_POKEINFO_DATA * info )
{
	NNSG2dCharacterData * chr;
	void * buf;
	u8 * raw;
	GFL_MSGDATA * man;
	GFL_BMP_DATA * bmp;
	GFL_BMP_DATA * rect;
	u32	winID;
	u16	i, j;

	if( syswk->app->subdisp_win_swap == 0 ){
		winID = BOX2BMPWIN_ID_WAZA1;
	}else{
		winID = BOX2BMPWIN_ID_WAZA1_2;
	}
	syswk->app->subdisp_win_swap ^= 1;

	if( info->tamago == 0 ){
		buf = GFL_ARC_UTIL_LoadBGCharacter(
						ARCID_BOX2_GRA, NARC_box_gra_box_s_bg3_lz_NCGR, TRUE, &chr, HEAPID_BOX_APP );
		raw = chr->pRawData;
		man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wazaname_dat, HEAPID_BOX_APP );

		for( i=0; i<4; i++ ){
			for( j=0; j<BMPWIN_WAZA_SX; j++ ){
				bmp  = GFL_BMPWIN_GetBmp( syswk->app->win[winID+i].win );
				rect = GFL_BMP_CreateWithData( &raw[0x20*0x0b], 8, 8, GFL_BMP_16_COLOR, HEAPID_BOX_APP );
				GFL_BMP_Print( rect, bmp, 0, 0, 8*j, 0, 8, 8, GF_BMPPRT_NOTNUKI );
				GFL_BMP_Delete( rect );
				rect = GFL_BMP_CreateWithData( &raw[0x20*0x01], 8, 8, GFL_BMP_16_COLOR, HEAPID_BOX_APP );
				GFL_BMP_Print( rect, bmp, 0, 0, 8*j, 8, 8, 8, GF_BMPPRT_NOTNUKI );
				GFL_BMP_Delete( rect );
			}
			StrPrint(
				syswk->app, winID+i, man, info->waza[i], 0, 0, syswk->app->font, FCOL_SUB_WIN );
		}
		syswk->app->subdisp_waza_put = 1;

		GFL_MSG_Delete( man );
		GFL_HEAP_FreeMemory( buf );
	}else{
		syswk->app->subdisp_waza_put = 0;
	}

	return winID;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 上画面アイテム情報表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	item	アイテム番号
 *
 * @return	表示したBMPWINのID
 */
//--------------------------------------------------------------------------------------------
/*
u32 BOX2BMP_PokeItemInfoWrite( BOX2_SYS_WORK * syswk, u16 item )
{
	PRINT_UTIL * pu;
	GFL_BMPWIN * win;
	u32	winID;

	if( syswk->app->subdisp_win_swap == 0 ){
		winID = BOX2BMPWIN_ID_ITEMLABEL;
	}else{
		winID = BOX2BMPWIN_ID_ITEMLABEL2;
	}
	pu = &syswk->app->win[winID];

	syswk->app->subdisp_win_swap ^= 1;

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(pu[0].win), 13 );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(pu[1].win), 13 );

	if( item != ITEM_DUMMY_DATA ){
		ITEM_GetItemName( syswk->app->exp, item, HEAPID_BOX_APP );
		PRINT_UTIL_PrintColor(
			&pu[0], syswk->app->que, 0, 0, syswk->app->exp, syswk->app->font, FCOL_SUB_WIN );
		ITEM_GetInfo( syswk->app->exp, item, HEAPID_BOX_APP );
		PRINT_UTIL_PrintColor(
			&pu[1], syswk->app->que, 0, 0, syswk->app->exp, syswk->app->font, FCOL_SUB_WIN );
	}

	return winID;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボタン作成（ボタン全体がＢＭＰ）
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		winID		BMPWIN ID
 * @param		strID		文字列ＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MakeBmpButton( BOX2_SYS_WORK * syswk, u32 winID, const u32 strID, u16 cgx, PRINTSYS_LSB	col )
{
	GFL_BMPWIN * win;
	GFL_BMP_DATA * bmp;
	GFL_BMP_DATA * button;
	STRBUF * str;
	void * buf;
	u8 * raw;
	NNSG2dCharacterData * chr;
	u8	frm, px, py, sx, sy;
	u8	i;

	win = syswk->app->win[ winID ].win;
	bmp = GFL_BMPWIN_GetBmp( win );
	frm = GFL_BMPWIN_GetFrame( win );
	px  = GFL_BMPWIN_GetPosX( win );
	py  = GFL_BMPWIN_GetPosY( win );
	sx  = GFL_BMPWIN_GetSizeX( win );
	sy  = GFL_BMPWIN_GetSizeY( win );

	GFL_BMP_Clear( bmp, FNTCOL_SELWIN_OFF_B );

	buf = GFL_ARC_UTIL_LoadBGCharacter( ARCID_BOX2_GRA, NARC_box_gra_selwin_lz_NCGR, TRUE, &chr, HEAPID_BOX_APP_L );
	raw = chr->pRawData;
	cgx *= 0x20;

	// 左上
	button = GFL_BMP_CreateWithData( &raw[cgx+0x20*0], 8, 8, GFL_BMP_16_COLOR, HEAPID_BOX_APP_L );
	GFL_BMP_Print( button, bmp, 0, 0, 0, 0, 8, 8, GF_BMPPRT_NOTNUKI );
	GFL_BMP_Delete( button );
	// 右上
	button = GFL_BMP_CreateWithData( &raw[cgx+0x20*2], 8, 8, GFL_BMP_16_COLOR, HEAPID_BOX_APP_L );
	GFL_BMP_Print( button, bmp, 0, 0, (sx-1)*8, 0, 8, 8, GF_BMPPRT_NOTNUKI );
	GFL_BMP_Delete( button );
	// 左下
	button = GFL_BMP_CreateWithData( &raw[cgx+0x20*9], 8, 8, GFL_BMP_16_COLOR, HEAPID_BOX_APP_L );
	GFL_BMP_Print( button, bmp, 0, 0, 0, (sy-1)*8, 8, 8, GF_BMPPRT_NOTNUKI );
	GFL_BMP_Delete( button );
	// 右下
	button = GFL_BMP_CreateWithData( &raw[cgx+0x20*11], 8, 8, GFL_BMP_16_COLOR, HEAPID_BOX_APP_L );
	GFL_BMP_Print( button, bmp, 0, 0, (sx-1)*8, (sy-1)*8, 8, 8, GF_BMPPRT_NOTNUKI );
	GFL_BMP_Delete( button );

	// 左
	button = GFL_BMP_CreateWithData( &raw[cgx+0x20*3], 8, 8, GFL_BMP_16_COLOR, HEAPID_BOX_APP_L );
	for( i=1; i<sy-1; i++ ){
		GFL_BMP_Print( button, bmp, 0, 0, 0, i*8, 8, 8, GF_BMPPRT_NOTNUKI );
	}
	GFL_BMP_Delete( button );
	// 右
	button = GFL_BMP_CreateWithData( &raw[cgx+0x20*5], 8, 8, GFL_BMP_16_COLOR, HEAPID_BOX_APP_L );
	for( i=1; i<sy-1; i++ ){
		GFL_BMP_Print( button, bmp, 0, 0, (sx-1)*8, i*8, 8, 8, GF_BMPPRT_NOTNUKI );
	}
	GFL_BMP_Delete( button );
	// 上
	button = GFL_BMP_CreateWithData( &raw[cgx+0x20*1], 8, 8, GFL_BMP_16_COLOR, HEAPID_BOX_APP_L );
	for( i=1; i<sx-1; i++ ){
		GFL_BMP_Print( button, bmp, 0, 0, i*8, 0, 8, 8, GF_BMPPRT_NOTNUKI );
	}
	GFL_BMP_Delete( button );
	// 下
	button = GFL_BMP_CreateWithData( &raw[cgx+0x20*10], 8, 8, GFL_BMP_16_COLOR, HEAPID_BOX_APP_L );
	for( i=1; i<sx-1; i++ ){
		GFL_BMP_Print( button, bmp, 0, 0, i*8, (sy-1)*8, 8, 8, GF_BMPPRT_NOTNUKI );
	}
	GFL_BMP_Delete( button );

	GFL_HEAP_FreeMemory( buf );

	str = GFL_MSG_CreateString( syswk->app->mman, strID );
	PRINTTOOL_PrintColor(
		&syswk->app->win[winID], syswk->app->que, 
		sx*8/2, 4, str, syswk->app->font, col, PRINTTOOL_MODE_CENTER );
	GFL_STR_DeleteBuffer( str );

//	GFL_BMPWIN_TransVramCharacter( win );
}

#define	BMPBUTTON_CHR_NORMAL		( 0 )
#define	BMPBUTTON_CHR_TOUCHBAR	( BOX2MAIN_SELWIN_CGX_SIZ )

static void ButtonPut_S( BOX2_SYS_WORK * syswk, u32 winID, const u32 strID )
{
	MakeBmpButton( syswk, winID, strID, BMPBUTTON_CHR_NORMAL, FCOL_SELWIN_OFF );
}

static void PutTouchBarButton( BOX2_SYS_WORK * syswk, u32 winID, const u32 strID )
{
	MakeBmpButton( syswk, winID, strID, BMPBUTTON_CHR_TOUCHBAR, FCOL_SELWIN_OFF );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボタンＢＧウィンドウフレーム作成
 *
 * @param		wk			BGウィンフレームワーク
 * @param		index		BGウィンフレームの番号
 * @param		win			BMPWIN
 *
 * @return	none
 *
 *	枠なしのウィンドウ
 */
//--------------------------------------------------------------------------------------------
static void ButtonBgFrmMake2( BGWINFRM_WORK * wk, u32 index, GFL_BMPWIN * win )
{
	BGWINFRM_Add(
		wk, index,
		GFL_BMPWIN_GetFrame( win ),
		GFL_BMPWIN_GetSizeX( win ),
		GFL_BMPWIN_GetSizeY( win ) );
	BGWINFRM_BmpWinOn( wk, index, win );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧウィンドウフレーム作成：各メニュー
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_PokeMenuBgFrmWkMake( BOX2_APP_WORK * appwk )
{
	ButtonBgFrmMake2( appwk->wfrm, BOX2MAIN_WINFRM_MENU1, appwk->win[BOX2BMPWIN_ID_MENU1].win );
	ButtonBgFrmMake2( appwk->wfrm, BOX2MAIN_WINFRM_MENU2, appwk->win[BOX2BMPWIN_ID_MENU2].win );
	ButtonBgFrmMake2( appwk->wfrm, BOX2MAIN_WINFRM_MENU3, appwk->win[BOX2BMPWIN_ID_MENU3].win );
	ButtonBgFrmMake2( appwk->wfrm, BOX2MAIN_WINFRM_MENU4, appwk->win[BOX2BMPWIN_ID_MENU4].win );
	ButtonBgFrmMake2( appwk->wfrm, BOX2MAIN_WINFRM_MENU5, appwk->win[BOX2BMPWIN_ID_MENU5].win );
	ButtonBgFrmMake2( appwk->wfrm, BOX2MAIN_WINFRM_MENU6, appwk->win[BOX2BMPWIN_ID_MENU6].win );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧウィンドウフレーム作成：「てもちポケモン」
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 *
 * @li	バトルボックスの場合は「バトルボックス」
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_TemochiButtonBgFrmWkMake( BOX2_SYS_WORK * syswk )
{
	if( syswk->dat->callMode == BOX_MODE_BATTLE ){
		PutTouchBarButton( syswk, BOX2BMPWIN_ID_TEMOCHI, mes_boxbutton_01_03 );
	}else{
		PutTouchBarButton( syswk, BOX2BMPWIN_ID_TEMOCHI, mes_boxbutton_01_01 );
	}
	ButtonBgFrmMake2( syswk->app->wfrm, BOX2MAIN_WINFRM_POKE_BTN, syswk->app->win[BOX2BMPWIN_ID_TEMOCHI].win );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧウィンドウフレーム作成：「ボックスリスト」
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_BoxListButtonBgFrmWkMake( BOX2_SYS_WORK * syswk )
{
	PutTouchBarButton( syswk, BOX2BMPWIN_ID_BOXLIST, mes_boxbutton_01_02 );
	ButtonBgFrmMake2( syswk->app->wfrm, BOX2MAIN_WINFRM_BOXLIST_BTN, syswk->app->win[BOX2BMPWIN_ID_BOXLIST].win );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンフレームにBMPWINをセット
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2BMP_PartyCngButtonFrmPut( BOX2_APP_WORK * appwk )
{
	BGWINFRM_BmpWinOn( appwk->wfrm, BOX2MAIN_WINFRM_PARTY, appwk->win[BMPWIN_PARTY_CNG].win );
}
*/





//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイ名表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		cgx			背景キャラデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2BMP_TrayNamePut( BOX2_SYS_WORK * syswk, u8 * cgx )
{
	GFL_BMPWIN * win;
	u8 * chr;
	STRBUF * str;
	u8	sx, sy;

	win = syswk->app->win[BOX2BMPWIN_ID_TRAY_NAME].win;
	chr = GFL_BMP_GetCharacterAdrs( GFL_BMPWIN_GetBmp(win) );
	sx  = GFL_BMPWIN_GetScreenSizeX( win );
	sy  = GFL_BMPWIN_GetScreenSizeY( win );
	GFL_STD_MemCopy32( cgx, chr, sx*sy*0x20 );

	str = GFL_STR_CreateBuffer( BOX_TRAYNAME_BUFSIZE, HEAPID_BOX_APP );
	BOXDAT_GetBoxName( syswk->dat->sv_box, syswk->tray, str );
	PRINTTOOL_PrintColor(
		&syswk->app->win[BOX2BMPWIN_ID_TRAY_NAME], syswk->app->que,
		sx*8/2, sy*8/2-8, str, syswk->app->font, FCOL_TRAYNAME, PRINTTOOL_MODE_CENTER );

	GFL_STR_DeleteBuffer( str );
}
void BOX2BMP_TrayNamePut( BOX2_SYS_WORK * syswk, u8 * cgx, u32 chr, u32 sx, u32 sy )
{
	GF_BGL_BMPWIN * win;
	STRBUF * str;
	u32	i;

	win = sys_AllocMemoryLo( HEAPID_BOX_APP, sizeof(GF_BGL_BMPWIN) );
	GF_BGL_BmpWinAdd( syswk->app->bgl, win, GFL_BG_FRAME3_M, 0, 0, sx, sy, 0, chr );
	MI_CpuCopy32( cgx, win->chrbuf, sx*sy*0x20 );

	str = STRBUF_Create( BOX_TRAYNAME_BUFSIZE, HEAPID_BOX_APP );
	BOXDAT_GetBoxName( syswk->dat->sv_box, syswk->tray, str );
	StrPrintCore( win, str, sx*8/2, sy*8/2-8, FONT_SYSTEM, FNTCOL_TRAYNAME, STRPRINT_MODE_CENTER );
	STRBUF_Delete( str );

	GF_BGL_BmpWinCgxOn( win );
	GF_BGL_BmpWinDel( win );
	sys_FreeMemoryEz( win );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ボックス移動処理のボックス名・収納数表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_BoxNameWrite( BOX2_SYS_WORK * syswk, u32 tray, u32 idx )
{
	STRBUF * str;
	u32	x;

	str = GFL_STR_CreateBuffer( BOX_TRAYNAME_BUFSIZE, HEAPID_BOX_APP_L );

	BOXDAT_GetBoxName( syswk->dat->sv_box, tray, str );
	GFL_BMP_Clear( syswk->app->fobj[idx].bmp, 0 );
	x = ( BOX2OBJ_FNTOAM_BOXNAME_SX * 8 - PRINTSYS_GetStrWidth( str, syswk->app->font, 0 ) ) / 2;
	PRINTSYS_PrintColor( syswk->app->fobj[idx].bmp, x, 0, str, syswk->app->font, FCOL_FNTOAM );

	GFL_STR_DeleteBuffer( str );

	BmpOam_ActorBmpTrans( syswk->app->fobj[idx].oam );
}

void BOX2BMP_BoxMoveNameWrite( BOX2_SYS_WORK * syswk, u32 pos )
{
	pos = syswk->box_mv_pos + pos;
	if( pos >= syswk->trayMax ){
		pos -= syswk->trayMax;
	}

	BOX2BMP_BoxNameWrite( syswk, pos, BOX2MAIN_FNTOAM_TRAY_NAME );

/*
	STRBUF * str;
	u32	x;

	pos = syswk->box_mv_pos + pos;
	if( pos >= syswk->trayMax ){
		pos -= syswk->trayMax;
	}

	str = GFL_STR_CreateBuffer( BOX_TRAYNAME_BUFSIZE, HEAPID_BOX_APP );

	BOXDAT_GetBoxName( syswk->dat->sv_box, pos, str );
	GFL_BMP_Clear( syswk->app->fobj[BOX2MAIN_FNTOAM_TRAY_NAME].bmp, 0 );
	x = ( BOX2OBJ_FNTOAM_BOXNAME_SX * 8 - PRINTSYS_GetStrWidth( str, syswk->app->font, 0 ) ) / 2;
	PRINTSYS_PrintColor( syswk->app->fobj[BOX2MAIN_FNTOAM_TRAY_NAME].bmp, x, 0, str, syswk->app->font, FCOL_FNTOAM );

	GFL_STR_DeleteBuffer( str );

	BmpOam_ActorBmpTrans( syswk->app->fobj[BOX2MAIN_FNTOAM_TRAY_NAME].oam );
*/
}

// 格納数書き込み
void BOX2BMP_WriteTrayNum( BOX2_SYS_WORK * syswk, u32 tray, u32 idx )
{
	STRBUF * str;
	u32	num;

	str = GFL_MSG_CreateString( syswk->app->mman, mes_boxmenu_02_20 );
	num = BOXDAT_GetPokeExistCount( syswk->dat->sv_box, tray );

	WORDSET_RegisterNumber( syswk->app->wset, 0, num, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( syswk->app->wset, syswk->app->exp, str );

	GFL_BMP_Clear( syswk->app->fobj[idx].bmp, 0 );
	if( num == BOX_MAX_POS ){
		PRINTSYS_PrintColor(
			syswk->app->fobj[idx].bmp, 0, 0, syswk->app->exp, syswk->app->nfnt, FCOL_FNTOAM_RED );
	}else{
		PRINTSYS_PrintColor(
			syswk->app->fobj[idx].bmp, 0, 0, syswk->app->exp, syswk->app->nfnt, FCOL_FNTOAM );
	}

	GFL_STR_DeleteBuffer( str );

	BmpOam_ActorBmpTrans( syswk->app->fobj[idx].oam );
}



//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンいどう」ボタン作成（持ち物整理モードの場合は「もちものせいり」）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2BMP_ButtonPutIdou( BOX2_SYS_WORK * syswk )
{
	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		PokeMoveButtonPut( syswk, BOX2BMPWIN_ID_BOXLIST, mes_boxbutton_01_05 );
	}else{
		PokeMoveButtonPut( syswk, BOX2BMPWIN_ID_BOXLIST, mes_boxbutton_01_02 );
	}
}
*/



//--------------------------------------------------------------------------------------------
/**
 * ＢＧウィンドウフレーム内のボタンに文字列描画
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	winID	BMPWIN ID
 * @param	strID	文字列ＩＤ
 *
 * @return	none
 *
 *	マーキングフレーム、手持ちポケモンフレームなど
 */
//--------------------------------------------------------------------------------------------
/*
static void FrmStrPut( BOX2_APP_WORK * appwk, u32 winID, u32 strID )
{
	STRBUF * str;
	u8	px;

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(appwk->win[winID].win), FRMBUTTON_BG_COL );
	px  = GFL_BMPWIN_GetSizeX( appwk->win[winID].win ) * 8 / 2;
	str = GFL_MSG_CreateString( appwk->mman, strID );
	PRINTTOOL_PrintColor(
		&appwk->win[winID], appwk->que, 
		px, 0, str, appwk->font, FCOL_FRMBUTTN_OFF, PRINTTOOL_MODE_CENTER );
	GFL_STR_DeleteBuffer( str );

//	GFL_BMPWIN_TransVramCharacter( &appwk->win[winID] );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * マーキングフレームのボタン描画
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_MarkingButtonPut( BOX2_SYS_WORK * syswk )
{
/*
	FrmStrPut( appwk, BMPWIN_MARK_ENTER, mes_boxbutton_03_01 );
	FrmStrPut( appwk, BMPWIN_MARK_CANCEL, mes_boxbutton_03_02 );
*/
	ButtonPut_S( syswk, BMPWIN_MARK_ENTER, mes_boxbutton_03_01 );
	ButtonPut_S( syswk, BMPWIN_MARK_CANCEL, mes_boxbutton_03_02 );

	BGWINFRM_PaletteChange(
		syswk->app->wfrm, BOX2MAIN_WINFRM_MARK,
		BMPWIN_MARK_ENTER_PX, BMPWIN_MARK_ENTER_PY,
		BMPWIN_MARK_ENTER_SX, BMPWIN_MARK_ENTER_SY,
		BOX2MAIN_BG_PAL_SELWIN );
	BGWINFRM_PaletteChange(
		syswk->app->wfrm, BOX2MAIN_WINFRM_MARK,
		BMPWIN_MARK_CANCEL_PX, BMPWIN_MARK_CANCEL_PY,
		BMPWIN_MARK_CANCEL_SX, BMPWIN_MARK_CANCEL_SY,
		BOX2MAIN_BG_PAL_SELWIN );
}



//--------------------------------------------------------------------------------------------
/**
 * @brief		マーキングフレームににBMPWINをセット
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_MarkingButtonFrmPut( BOX2_SYS_WORK * syswk )
{
	BGWINFRM_BmpWinOn( syswk->app->wfrm, BOX2MAIN_WINFRM_MARK, syswk->app->win[BMPWIN_MARK_ENTER].win );
	BGWINFRM_BmpWinOn( syswk->app->wfrm, BOX2MAIN_WINFRM_MARK, syswk->app->win[BMPWIN_MARK_CANCEL].win );
	BOX2BMP_MarkingButtonPut( syswk );
}


//--------------------------------------------------------------------------------------------
/**
 * メニュー作成
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	list	メニューリスト
 * @param	max		メニュー数
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_MenuStrPrint( BOX2_SYS_WORK * syswk, const BOX2BMP_BUTTON_LIST * list, u32 max )
{
	u32	i;
	u16	sx, sy;

	BGWINFRM_SizeGet( syswk->app->wfrm, BOX2MAIN_WINFRM_MENU1, &sx, &sy );

	for( i=0; i<max; i++ ){
		if( list[max-1-i].type == BOX2BMP_BUTTON_TYPE_WHITE ){
			ButtonPut_S( syswk, BOX2BMPWIN_ID_MENU6-i, list[max-1-i].strID );
			BGWINFRM_PaletteChange(
				syswk->app->wfrm, BOX2MAIN_WINFRM_MENU6-i, 0, 0, sx, sy, BOX2MAIN_BG_PAL_SELWIN );
		}else{
/*
			PokeMoveButtonPut( syswk, BOX2BMPWIN_ID_MENU6-i, list[max-1-i].strID );
			BGWINFRM_PaletteChange(
				syswk->app->wfrm, BOX2MAIN_WINFRM_MENU6-i, 0, 0, sx, sy, 2 );
*/
		}
	}
	// いらないものは消す
	for( i=max; i<6; i++ ){
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(syswk->app->win[BOX2BMPWIN_ID_MENU6-i].win), 0 );
		GFL_BMPWIN_TransVramCharacter( syswk->app->win[BOX2BMPWIN_ID_MENU6-i].win );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * メニュー非表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	max		メニュー数
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_MenuVanish( BOX2_SYS_WORK * syswk, u32 max )
{
	u32	i;

	for( i=0; i<max; i++ ){
		ClearScreen( &syswk->app->win[BOX2BMPWIN_ID_MENU5-i] );
	}
}



//============================================================================================
//	システムウィンドウ
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * システムウィンドウ表示
 *
 * @param	win		BMPWIN
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SysWinPut( BOX2_SYS_WORK * syswk, GFL_BMPWIN * win )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(win), 15 );
	BmpWinFrame_Write(
		win, WINDOW_TRANS_OFF,
		GFL_ARCUTIL_TRANSINFO_GetPos(syswk->app->syswinInfo), BOX2MAIN_BG_PAL_SYSWIN );
}

//--------------------------------------------------------------------------------------------
/**
 * システムウィンドウ非表示（ＶＢＬＡＮＫで転送）
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_SysWinVanish( BOX2_APP_WORK * appwk, u32 winID )
{
	BmpWinFrame_Clear( appwk->win[winID].win, WINDOW_TRANS_ON_V );
}

//--------------------------------------------------------------------------------------------
/**
 * システムウィンドウ非表示（転送なし）
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2BMP_SysWinVanishEz( BOX2_APP_WORK * appwk, u32 winID )
{
	BmpWinFrame_Clear( appwk->win[winID].win, WINDOW_TRANS_OFF );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * システムウィンドウ非表示（即転送）
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_SysWinVanishTrans( BOX2_APP_WORK * appwk, u32 winID )
{
	BmpWinFrame_Clear( appwk->win[winID].win, WINDOW_TRANS_ON );
}

//--------------------------------------------------------------------------------------------
/**
 * システムメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	strID	文字列ＩＤ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SysMesPut( BOX2_SYS_WORK * syswk, u32 strID, u32 winID )
{
	GFL_MSGDATA * man = BoxMsgManGet();

	SysWinPut( syswk, syswk->app->win[winID].win );
	ExStrPrint(
		syswk->app, winID, man, strID, 0, 0, syswk->app->font, FCOL_W_BLACK );
	PrintScreenTrans( &syswk->app->win[winID] );

	GFL_MSG_Delete( man );

	BOX2OBJ_VanishTouchBarButton( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックス終了メッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	winID	BMPWIN ID
 * @param	flg		タッチ or キー
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_BoxEndMsgPut( BOX2_SYS_WORK * syswk, u32 winID, BOOL flg )
{
	if( flg == TRUE ){
		SysMesPut( syswk, msg_boxmes_01_12, winID );
	}else{
		SysMesPut( syswk, msg_boxmes_01_13, winID );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム取得チェックメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	item	アイテム番号
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ItemGetCheckMsgPut( BOX2_SYS_WORK * syswk, u32 item, u32 winID )
{
	WORDSET_RegisterItemName( syswk->app->wset, 0, item );
	SysMesPut( syswk, msg_boxmes_01_24, winID );
}

//--------------------------------------------------------------------------------------------
/**
 * メール取得エラーメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_MailGetErrorPut( BOX2_SYS_WORK * syswk, u32 winID )
{
	SysMesPut( syswk, msg_boxmes_01_25, winID );
}

//--------------------------------------------------------------------------------------------
/**
 * メール移動エラーメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_MailMoveErrorPut( BOX2_SYS_WORK * syswk, u32 winID )
{
	SysMesPut( syswk, msg_boxmes_04_01, winID );
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム取得メッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	item	アイテム番号
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ItemGetMsgPut( BOX2_SYS_WORK * syswk, u32 item, u32 winID )
{
	WORDSET_RegisterItemName( syswk->app->wset, 0, item );
	SysMesPut( syswk, msg_boxmes_01_16, winID );
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム取得エラーメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ItemGetErrorPut( BOX2_SYS_WORK * syswk, u32 winID )
{
	SysMesPut( syswk, msg_boxmes_01_15, winID );
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムセットメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	item	アイテム番号
 * @param	winID	BMPWIN ID
 *
 * @return	none
 *
 *	item = 0 のときは、はっきんだまを持たせようとしたときのエラーとする
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ItemSetMsgPut( BOX2_SYS_WORK * syswk, u32 item, u32 winID )
{
	if( item == 0 ){
		WORDSET_RegisterItemName( syswk->app->wset, 0, ITEM_HAKKINDAMA );
		SysMesPut( syswk, msg_boxmes_05_01, winID );
	}else if( ITEM_CheckMail( item ) == TRUE ){
		SysMesPut( syswk, msg_boxmes_06_06, winID );
	}else{
		WORDSET_RegisterItemName( syswk->app->wset, 0, item );
		SysMesPut( syswk, msg_boxmes_01_17, winID );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * マーキングメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_MarkingMsgPut( BOX2_SYS_WORK * syswk, u32 winID )
{
	SysMesPut( syswk, msg_boxmes_01_37, winID );
}

//--------------------------------------------------------------------------------------------
/**
 * 逃がすメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	msgID	メッセージＩＤ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_PokeFreeMsgPut( BOX2_SYS_WORK * syswk, u32 msgID, u32 winID )
{
	u32	str;

	switch( msgID ){
	case BOX2BMP_MSGID_POKEFREE_CHECK:	// ほんとうに　にがしますか？
		str = msg_boxmes_01_03;
		break;

	case BOX2BMP_MSGID_POKEFREE_ENTER:	// @0を　そとに　にがしてあげた
		str = msg_boxmes_01_04;
		WORDSET_RegisterPokeNickNamePPP(
			syswk->app->wset, 0, BOX2MAIN_PPPGet(syswk,syswk->tray,syswk->get_pos) );
		break;

	case BOX2BMP_MSGID_POKEFREE_BY:		// ばいばい　@0！
		str = msg_boxmes_01_05;
		WORDSET_RegisterPokeNickNamePPP(
			syswk->app->wset, 0, BOX2MAIN_PPPGet(syswk,syswk->tray,syswk->get_pos) );
		break;

	case BOX2BMP_MSGID_POKEFREE_EGG:	// タマゴを　にがすことは　できません！
		str = msg_boxmes_01_33;
		break;

	case BOX2BMP_MSGID_POKEFREE_RETURN:	// @0が　もどってきちゃった！
		str = msg_boxmes_01_34;
		WORDSET_RegisterPokeNickNamePPP(
			syswk->app->wset, 0, BOX2MAIN_PPPGet(syswk,syswk->tray,syswk->get_pos) );
		break;

	case BOX2BMP_MSGID_POKEFREE_FEAR:	// しんぱい　だったのかな……
		str = msg_boxmes_01_35;
		break;

	case BOX2BMP_MSGID_POKEFREE_ONE:	// たたかうポケモンが　いなくなります！
		str = msg_boxmes_01_07;
		break;
	}

	SysMesPut( syswk, str, winID );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックステーマ変更メッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	msgID	メッセージＩＤ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_BoxThemaMsgPut( BOX2_SYS_WORK * syswk, u32 msgID, u32 winID )
{
	u32	str;

	switch( msgID ){
	case BOX2BMP_MSGID_THEMA_INIT:		// このボックスを　どうしますか？
		str = msg_boxmes_01_08;
		break;

	case BOX2BMP_MSGID_THEMA_JUMP:		// どのボックスに　ジャンプしますか？
		str = msg_boxmes_01_09;
		break;

	case BOX2BMP_MSGID_THEMA_MES:			// どのテーマに　しますか？
		str = msg_boxmes_01_10;
		break;

	case BOX2BMP_MSGID_THEMA_WALL:		// どの　かべがみに　しますか？
		str = msg_boxmes_01_11;
		break;
	}

	SysMesPut( syswk, str, winID );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモン選択メッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		ポケモンの位置
 * @param	msgID	メッセージＩＤ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_PokeSelectMsgPut( BOX2_SYS_WORK * syswk, u32 pos, u32 msgID, u32 winID )
{
	u32	str;

	switch( msgID ){
	case BOX2BMP_MSGID_PARTYIN_INIT:	// ポケモンを　タッチ！
		str = msg_boxmes_01_19;
		break;

	case BOX2BMP_MSGID_PARTYIN_MENU:	// @01を　どうする？
		str = msg_boxmes_01_01;
		WORDSET_RegisterPokeNickNamePPP(
			syswk->app->wset, 0, BOX2MAIN_PPPGet(syswk,syswk->tray,syswk->get_pos) );
		break;

	case BOX2BMP_MSGID_PARTYIN_ERR:		// てもちが　いっぱいです！
		str = msg_boxmes_01_06;
		break;

	case BOX2BMP_MSGID_PARTYOUT_INIT:	// どこへ　あずける？
		str = msg_boxmes_01_20;
		break;

	case BOX2BMP_MSGID_PARTYOUT_BOXMAX:	// このボックスは　いっぱいだ！
		str = msg_boxmes_01_14;
		break;

/*		
	case BOX2BMP_MSGID_PARTYOUT_CAPSULE:	// ボールカプセルを　はずしてください！
		str = msg_boxmes_01_31;
		break;
*/

	case BOX2BMP_MSGID_PARTYOUT_MAIL:	// メールを　はずしてください！
		str = msg_boxmes_01_32;
		break;
	}

	SysMesPut( syswk, str, winID );
}

//--------------------------------------------------------------------------------------------
/**
 * 道具整理メッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	msgID	メッセージＩＤ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ItemArrangeMsgPut( BOX2_SYS_WORK * syswk, u32 msgID, u32 winID )
{
	u32	str;

	switch( msgID ){
	case BOX2BMP_MSGID_ITEM_A_SET:				// どうぐを　もたせます
		str = msg_boxmes_01_30;
		break;

	case BOX2BMP_MSGID_ITEM_A_INIT:				// @0を　どうしますか？
		str = msg_boxmes_01_26;
		WORDSET_RegisterItemName( syswk->app->wset, 0, syswk->app->get_item );
		break;

	case BOX2BMP_MSGID_ITEM_A_RET_CHECK:	// @0を　しまいますか？
		str = msg_boxmes_01_28;
		WORDSET_RegisterItemName( syswk->app->wset, 0, syswk->app->get_item );
		break;

	case BOX2BMP_MSGID_ITEM_A_RET:			// @0を　バッグに　いれた！
		str = msg_boxmes_01_29;
		WORDSET_RegisterItemName( syswk->app->wset, 0, syswk->app->get_item );
		break;

	case BOX2BMP_MSGID_ITEM_A_MAIL:			// メールをしまうことは　できません！
		str = msg_boxmes_01_25;
		break;

	case BOX2BMP_MSGID_ITEM_A_EGG:			// タマゴは　どうぐを　もてません！
		str = msg_boxmes_01_36;
		break;

	case BOX2BMP_MSGID_ITEM_A_MAX:			// バッグが　いっぱいです！
		str = msg_boxmes_01_15;
		break;

	case BOX2BMP_MSGID_ITEM_A_MAIL_MOVE:	// メールは　いどうできません！
		str = msg_boxmes_06_04;
		break;
	}

	SysMesPut( syswk, str, winID );
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANKで表示を実行するメッセージをセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	winID	BMPWIN ID
 * @param	msgID	メッセージＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2BMP_VBlankMsgSet( BOX2_SYS_WORK * syswk, u32 winID, u32 msgID )
{
	GFL_MSGDATA * man;
	STRBUF * str;

	man = BoxMsgManGet();

	switch( msgID ){
	case BOX2BMP_MSGID_VBLANK_AZUKERU:		// ボックスに　おけます！
		msgID = msg_boxmes_01_38;
		break;
	case BOX2BMP_MSGID_VBLANK_TSURETEIKU:	// てもちに　くわえることが　できます！
		msgID = msg_boxmes_01_39;
		break;

	case BOX2BMP_MSGID_VBLANK_AZUKERU_ERR:	// このボックスは　いっぱいです！
		msgID = msg_boxmes_01_40;
		break;
	case BOX2BMP_MSGID_VBLANK_TSURETEIKU_ERR:	// てもちは　いっぱいです！
		msgID = msg_boxmes_01_41;
		break;

	case BOX2BMP_MSGID_VBLANK_ITEM_SET:		// どうぐを　もたせます
		msgID = msg_boxmes_01_30;
		break;

	case BOX2BMP_MSGID_VBLANK_ITEM_INIT:	// @0を　どうしますか？
		msgID = msg_boxmes_01_26;
		WORDSET_RegisterItemName( syswk->app->wset, 0, syswk->app->get_item );
		break;

	case BOX2BMP_MSGID_VBLANK_MAIL_ERR:		// メールを　もっています！
		msgID = msg_boxmes_06_01;
		break;

	case BOX2BMP_MSGID_VBLANK_CAPSULE_ERR:	// ボールカプセルを　もっています！
		msgID = msg_boxmes_06_02;
		break;

	case BOX2BMP_MSGID_VBLANK_BATTLE_POKE_ERR:	// たたかうポケモンが　いなくなります！
		msgID = msg_boxmes_06_03;
		break;

	case BOX2BMP_MSGID_VBLANK_MAIL_MOVE:		// メールは　いどうできません！
		msgID = msg_boxmes_06_04;
		break;

	case BOX2BMP_MSGID_VBLANK_ITEM_MOVE:		// ポケモンに　セット！
		msgID = msg_boxmes_06_05;
		break;
	}

	ExStrPrint( syswk->app, winID, man, msgID, 0, 0, syswk->app->font, FCOL_W_BLACK );

	GFL_MSG_Delete( man );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * VBLANKでメッセージを表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2BMP_VBlankMsgPut( BOX2_SYS_WORK * syswk, u32 winID )
{
	BmpWinFrame_Write(
		syswk->app->win[winID].win, WINDOW_TRANS_ON_V,
		GFL_ARCUTIL_TRANSINFO_GetPos(syswk->app->syswinInfo), BOX2MAIN_BG_PAL_SYSWIN );
}
*/



void BOX2BMP_PutPokeMoveErrMsg( BOX2_SYS_WORK * syswk, u32 errID, u32 winID )
{
	u32	str;

	switch( errID ){
	case BOX2MAIN_ERR_CODE_MAIL:			// メールを持っている
		str = msg_boxmes_06_01;
		break;

	case BOX2MAIN_ERR_CODE_BATTLE:		// 戦えるポケモンがいなくなる
		str = msg_boxmes_06_03;
		break;

	case BOX2MAIN_ERR_CODE_EGG:				// タマゴはえらべない（バトルボックス専用）
		str = msg_boxmes_06_07;
		break;
	}

	SysMesPut( syswk, str, winID );
}

void BOX2BMP_SleepSelectMsgPut( BOX2_SYS_WORK * syswk )
{
	WORDSET_RegisterPokeNickNamePPP( syswk->app->wset, 0, BOX2MAIN_PPPGet(syswk,syswk->tray,syswk->get_pos) );
	SysMesPut( syswk, msg_boxmes_06_08, BOX2BMPWIN_ID_MSG4 );
}

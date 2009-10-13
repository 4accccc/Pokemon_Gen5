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

#include "system/main.h"

#include "box2_main.h"
#include "box2_bmp.h"


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
// ポケモン名
#define	BMPWIN_NAME_FRM		( GFL_BG_FRAME0_S )
#define	BMPWIN_NAME_PX		( 8 )
#define	BMPWIN_NAME_PY		( 5 )
#define	BMPWIN_NAME_SX		( 7 )
#define	BMPWIN_NAME_SY		( 2 )
#define	BMPWIN_NAME_PAL		( BOX2MAIN_BG_PAL_SYSFNT_S )
// ニックネーム
#define	BMPWIN_NICKNAME_FRM	( GFL_BG_FRAME0_S )
#define	BMPWIN_NICKNAME_PX	( 1 )
#define	BMPWIN_NICKNAME_PY	( 7 )
#define	BMPWIN_NICKNAME_SX	( 8 )
#define	BMPWIN_NICKNAME_SY	( 2 )
#define	BMPWIN_NICKNAME_PAL	( BOX2MAIN_BG_PAL_SYSFNT_S )
// レベル
#define	BMPWIN_LV_FRM		( GFL_BG_FRAME0_S )
#define	BMPWIN_LV_PX		( 4 )
#define	BMPWIN_LV_PY		( 9 )
#define	BMPWIN_LV_SX		( 6 )
#define	BMPWIN_LV_SY		( 2 )
#define	BMPWIN_LV_PAL		( BOX2MAIN_BG_PAL_SYSFNT_S )
// 性別
#define	BMPWIN_SEX_FRM		( GFL_BG_FRAME0_S )
#define	BMPWIN_SEX_PX		( 16 )
#define	BMPWIN_SEX_PY		( 5 )
#define	BMPWIN_SEX_SX		( 2 )
#define	BMPWIN_SEX_SY		( 2 )
#define	BMPWIN_SEX_PAL		( BOX2MAIN_BG_PAL_SYSFNT_S )
// 図鑑番号
#define	BMPWIN_ZKNNUM_FRM	( GFL_BG_FRAME0_S )
#define	BMPWIN_ZKNNUM_PX	( 1 )
#define	BMPWIN_ZKNNUM_PY	( 5 )
#define	BMPWIN_ZKNNUM_SX	( 6 )
#define	BMPWIN_ZKNNUM_SY	( 2 )
#define	BMPWIN_ZKNNUM_PAL	( BOX2MAIN_BG_PAL_SYSFNT_S )
// 性格
#define	BMPWIN_SEIKAKU_FRM	( GFL_BG_FRAME0_S )
#define	BMPWIN_SEIKAKU_PX	( 1 )
#define	BMPWIN_SEIKAKU_PY	( 13 )
#define	BMPWIN_SEIKAKU_SX	( 8 )
#define	BMPWIN_SEIKAKU_SY	( 2 )
#define	BMPWIN_SEIKAKU_PAL	( BOX2MAIN_BG_PAL_SYSFNT_S )
// 特性
#define	BMPWIN_TOKUSEI_FRM	( GFL_BG_FRAME0_S )
#define	BMPWIN_TOKUSEI_PX	( 1 )
#define	BMPWIN_TOKUSEI_PY	( 17 )
#define	BMPWIN_TOKUSEI_SX	( 11 )
#define	BMPWIN_TOKUSEI_SY	( 2 )
#define	BMPWIN_TOKUSEI_PAL	( BOX2MAIN_BG_PAL_SYSFNT_S )
// 持ち物
#define	BMPWIN_ITEM_FRM		( GFL_BG_FRAME0_S )
#define	BMPWIN_ITEM_PX		( 1 )
#define	BMPWIN_ITEM_PY		( 21 )
#define	BMPWIN_ITEM_SX		( 12 )
#define	BMPWIN_ITEM_SY		( 2 )
#define	BMPWIN_ITEM_PAL		( BOX2MAIN_BG_PAL_SYSFNT_S )
//「せいかく」
#define	BMPWIN_SEIKAKU_STR_FRM	( GFL_BG_FRAME0_S )
#define	BMPWIN_SEIKAKU_STR_PX	( 1 )
#define	BMPWIN_SEIKAKU_STR_PY	( 11 )
#define	BMPWIN_SEIKAKU_STR_SX	( 8 )
#define	BMPWIN_SEIKAKU_STR_SY	( 2 )
#define	BMPWIN_SEIKAKU_STR_PAL	( BOX2MAIN_BG_PAL_SYSFNT_S )
//「とくせい」
#define	BMPWIN_TOKUSEI_STR_FRM	( GFL_BG_FRAME0_S )
#define	BMPWIN_TOKUSEI_STR_PX	( 1 )
#define	BMPWIN_TOKUSEI_STR_PY	( 15 )
#define	BMPWIN_TOKUSEI_STR_SX	( 11 )
#define	BMPWIN_TOKUSEI_STR_SY	( 2 )
#define	BMPWIN_TOKUSEI_STR_PAL	( BOX2MAIN_BG_PAL_SYSFNT_S )
//「もちもの」
#define	BMPWIN_ITEM_STR_FRM		( GFL_BG_FRAME0_S )
#define	BMPWIN_ITEM_STR_PX		( 1 )
#define	BMPWIN_ITEM_STR_PY		( 19 )
#define	BMPWIN_ITEM_STR_SX		( 12 )
#define	BMPWIN_ITEM_STR_SY		( 2 )
#define	BMPWIN_ITEM_STR_PAL		( BOX2MAIN_BG_PAL_SYSFNT_S )
// 技
#define	BMPWIN_WAZA_FRM		( GFL_BG_FRAME2_S )
#define	BMPWIN_WAZA_SX		( 11 )
#define	BMPWIN_WAZA_SY		( 2 )
#define	BMPWIN_WAZA_PAL		( 1 )
#define	BMPWIN_WAZA1_PX		( 1 )
#define	BMPWIN_WAZA1_PY		( 1 )
#define	BMPWIN_WAZA2_PX		( 1 )
#define	BMPWIN_WAZA2_PY		( 3 )
#define	BMPWIN_WAZA3_PX		( 1 )
#define	BMPWIN_WAZA3_PY		( 5 )
#define	BMPWIN_WAZA4_PX		( 1 )
#define	BMPWIN_WAZA4_PY		( 7 )
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

//「てもちポケモン」
#define	BMPWIN_TEMOCHI_FRM		( GFL_BG_FRAME1_M )
#define	BMPWIN_TEMOCHI_PX		( 0 )
#define	BMPWIN_TEMOCHI_PY		( 0 )//( 21 )
#define	BMPWIN_TEMOCHI_SX		( 11 )
#define	BMPWIN_TEMOCHI_SY		( 3 )
#define	BMPWIN_TEMOCHI_PAL		( BOX2MAIN_BG_PAL_SELWIN )
//「ポケモンいどう」
#define	BMPWIN_IDOU_FRM			( GFL_BG_FRAME1_M )
#define	BMPWIN_IDOU_PX			( 0 )//( 12 )
#define	BMPWIN_IDOU_PY			( 0 )//( 21 )
#define	BMPWIN_IDOU_SX			( 11 )
#define	BMPWIN_IDOU_SY			( 3 )
#define	BMPWIN_IDOU_PAL			( 2 )
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

// 手持ちポケモンフレーム・いれかえ
#define	BMPWIN_PARTY_CNG_FRM	( GFL_BG_FRAME1_M )
#define	BMPWIN_PARTY_CNG_PX		( 2 )
#define	BMPWIN_PARTY_CNG_PY		( 15 )
#define	BMPWIN_PARTY_CNG_SX		( 7 )
#define	BMPWIN_PARTY_CNG_SY		( 2 )
#define	BMPWIN_PARTY_CNG_PAL	( 1 )

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

// メニュー１
#define	BMPWIN_MENU1_FRM	( GFL_BG_FRAME0_M )
#define	BMPWIN_MENU1_PX		( 0 )//( 21 )
#define	BMPWIN_MENU1_PY		( 0 )//( 8 )
#define	BMPWIN_MENU1_SX		( 11 )
#define	BMPWIN_MENU1_SY		( 3 )
#define	BMPWIN_MENU1_PAL	( BOX2MAIN_BG_PAL_SELWIN )
// メニュー２
#define	BMPWIN_MENU2_FRM	( GFL_BG_FRAME0_M )
#define	BMPWIN_MENU2_PX		( 0 )//( 21 )
#define	BMPWIN_MENU2_PY		( 0 )//( 11 )
#define	BMPWIN_MENU2_SX		( 11 )
#define	BMPWIN_MENU2_SY		( 3 )
#define	BMPWIN_MENU2_PAL	( BOX2MAIN_BG_PAL_SELWIN )
// メニュー３
#define	BMPWIN_MENU3_FRM	( GFL_BG_FRAME0_M )
#define	BMPWIN_MENU3_PX		( 0 )//( 21 )
#define	BMPWIN_MENU3_PY		( 0 )//( 14 )
#define	BMPWIN_MENU3_SX		( 11 )
#define	BMPWIN_MENU3_SY		( 3 )
#define	BMPWIN_MENU3_PAL	( BOX2MAIN_BG_PAL_SELWIN )
// メニュー４
#define	BMPWIN_MENU4_FRM	( GFL_BG_FRAME0_M )
#define	BMPWIN_MENU4_PX		( 0 )//( 21 )
#define	BMPWIN_MENU4_PY		( 0 )//( 17 )
#define	BMPWIN_MENU4_SX		( 11 )
#define	BMPWIN_MENU4_SY		( 3 )
#define	BMPWIN_MENU4_PAL	( BOX2MAIN_BG_PAL_SELWIN )
// メニュー５
#define	BMPWIN_MENU5_FRM	( GFL_BG_FRAME0_M )
#define	BMPWIN_MENU5_PX		( 0 )//( 21 )
#define	BMPWIN_MENU5_PY		( 0 )//( 17 )
#define	BMPWIN_MENU5_SX		( 11 )
#define	BMPWIN_MENU5_SY		( 3 )
#define	BMPWIN_MENU5_PAL	( BOX2MAIN_BG_PAL_SELWIN )

// マーキング決定
#define	BMPWIN_MARK_ENTER_FRM	( GFL_BG_FRAME0_M )
#define	BMPWIN_MARK_ENTER_PX	( 2 )
#define	BMPWIN_MARK_ENTER_PY	( 11 )
#define	BMPWIN_MARK_ENTER_SX	( 7 )
#define	BMPWIN_MARK_ENTER_SY	( 2 )
#define	BMPWIN_MARK_ENTER_PAL	( 1 )
// マーキングキャンセル
#define	BMPWIN_MARK_CANCEL_FRM	( GFL_BG_FRAME0_M )
#define	BMPWIN_MARK_CANCEL_PX	( 2 )
#define	BMPWIN_MARK_CANCEL_PY	( 15 )
#define	BMPWIN_MARK_CANCEL_SX	( 7 )
#define	BMPWIN_MARK_CANCEL_SY	( 2 )
#define	BMPWIN_MARK_CANCEL_PAL	( 1 )

// メッセージ１
#define	BMPWIN_MSG1_FRM		( GFL_BG_FRAME0_M )
#define	BMPWIN_MSG1_PX		( 2 )
#define	BMPWIN_MSG1_PY		( 21 )
#define	BMPWIN_MSG1_SX		( 27 )
#define	BMPWIN_MSG1_SY		( 2 )
#define	BMPWIN_MSG1_PAL		( BOX2MAIN_BG_PAL_TALKFNT )
// メッセージ２
#define	BMPWIN_MSG2_PX		( 2 )
#define	BMPWIN_MSG2_PY		( 1 )
// メッセージ３
#define	BMPWIN_MSG3_SX		( 19 )
#define	BMPWIN_MSG3_SY		( 2 )

// ボックス移動のメニュー
#define	BMPWIN_BOXMV_MENU_FRM	( GFL_BG_FRAME0_M )
#define	BMPWIN_BOXMV_MENU_PX	( 22 )
#define	BMPWIN_BOXMV_MENU_PY	( 16 )
#define	BMPWIN_BOXMV_MENU_SX	( 9 )
#define	BMPWIN_BOXMV_MENU_SY	( 4 )
#define	BMPWIN_BOXMV_MENU_PAL	( BOX2MAIN_BG_PAL_SELWIN )

/*
#define	FNTCOL_SELWIN_ON_B	( 11 )		// ボタン塗りつぶしカラー番号（選択時）
#define	FNTCOL_SELWIN_OFF_B	( 4 )		// ボタン塗りつぶしカラー番号（非選択時）

#define	FRMBUTTON_BG_COL	( 13 )		// フレーム内ボタンの背景カラー

#define	FNTCOL_N_BLACK		( PRINTSYS_LSB_MAKE( 1, 2, 0 ) )		// フォントカラー：黒抜
//↑[GS_CONVERT_TAG]
#define	FNTCOL_W_BLACK		( PRINTSYS_LSB_MAKE( 1, 2, 15 ) )		// フォントカラー：黒白
//↑[GS_CONVERT_TAG]
#define	FNTCOL_N_BLUE		( PRINTSYS_LSB_MAKE(FBMP_COL_BLUE,FBMP_COL_BLU_SDW,0) )	// フォントカラー：青抜
//↑[GS_CONVERT_TAG]
#define	FNTCOL_N_RED		( PRINTSYS_LSB_MAKE(FBMP_COL_RED,FBMP_COL_RED_SDW,0) )		// フォントカラー：赤抜
//↑[GS_CONVERT_TAG]
#define	FNTCOL_TRAYNAME		( PRINTSYS_LSB_MAKE( 2, 1, 0 ) )		// フォントカラー：ボックス名
//↑[GS_CONVERT_TAG]
#define	FNTCOL_FNTOAM		( PRINTSYS_LSB_MAKE( 1, 2, 0 ) )		// フォントカラー：黒抜
//↑[GS_CONVERT_TAG]
#define	FNTCOL_SELWIN_ON	( PRINTSYS_LSB_MAKE(14,15,0) )			// フォントカラー：ボタン選択時
//↑[GS_CONVERT_TAG]
#define	FNTCOL_SELWIN_OFF	( PRINTSYS_LSB_MAKE(14,15,0) )			// フォントカラー：ボタン非選択時
//↑[GS_CONVERT_TAG]
#define	FNTCOL_YSTWIN		( PRINTSYS_LSB_MAKE(9,10,0) )			// フォントカラー：Ｙステータスボタン
//↑[GS_CONVERT_TAG]
#define	FNTCOL_FRMBUTTN_OFF	( PRINTSYS_LSB_MAKE(9,10,FRMBUTTON_BG_COL) )	// フォントカラー：フレーム内ボタン
//↑[GS_CONVERT_TAG]
#define	FNTCOL_SUB_WIN		( PRINTSYS_LSB_MAKE( 1, 2, 0 ) )		// フォントカラー：上画面ウィンドウ用
//↑[GS_CONVERT_TAG]


// メッセージ表示モード
enum {
	STRPRINT_MODE_LEFT = 0,		// 左詰
	STRPRINT_MODE_RIGHT,		// 右詰
	STRPRINT_MODE_CENTER		// 中央
};

#define	POKE_NOMARK_PY	( 5 )	//「No.」表示Y座標

#define	TITLE_STR_PY	( 8 )	// タイトル表示Y座標

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

//「Ｙようすをみる」ボタンキャラ
#define	YST_UL_CGX_POS	( 0x3d * 0x20 )
#define	YST_CL_CGX_POS	( 0x5d * 0x20 )
#define	YST_DL_CGX_POS	( 0x7d * 0x20 )
#define	YST_UR_CGX_POS	( 0x68 * 0x20 )
#define	YST_DR_CGX_POS	( 0x66 * 0x20 )
#define	YST_CR_CGX_POS	( 0x67 * 0x20 )
#define	YST_CC_CGX_POS	( 0x65 * 0x20 )
#define	YST_UC_CGX_POS	( 0x61 * 0x20 )
#define	YST_DC_CGX_POS	( 0x64 * 0x20 )
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
	{	// 技１
		BMPWIN_WAZA_FRM, BMPWIN_WAZA1_PX, BMPWIN_WAZA1_PY,
		BMPWIN_WAZA_SX, BMPWIN_WAZA_SY, BMPWIN_WAZA_PAL,
	},
	{	// 技２
		BMPWIN_WAZA_FRM, BMPWIN_WAZA2_PX, BMPWIN_WAZA2_PY,
		BMPWIN_WAZA_SX, BMPWIN_WAZA_SY, BMPWIN_WAZA_PAL,
	},
	{	// 技３
		BMPWIN_WAZA_FRM, BMPWIN_WAZA3_PX, BMPWIN_WAZA3_PY,
		BMPWIN_WAZA_SX, BMPWIN_WAZA_SY, BMPWIN_WAZA_PAL,
	},
	{	// 技４
		BMPWIN_WAZA_FRM, BMPWIN_WAZA4_PX, BMPWIN_WAZA4_PY,
		BMPWIN_WAZA_SX, BMPWIN_WAZA_SY, BMPWIN_WAZA_PAL,
	},
	{	// 技１（スワップ用）
		BMPWIN_WAZA_FRM, BMPWIN_WAZA1_PX, BMPWIN_WAZA1_PY,
		BMPWIN_WAZA_SX, BMPWIN_WAZA_SY, BMPWIN_WAZA_PAL,
	},
	{	// 技２（スワップ用）
		BMPWIN_WAZA_FRM, BMPWIN_WAZA2_PX, BMPWIN_WAZA2_PY,
		BMPWIN_WAZA_SX, BMPWIN_WAZA_SY, BMPWIN_WAZA_PAL,
	},
	{	// 技３（スワップ用）
		BMPWIN_WAZA_FRM, BMPWIN_WAZA3_PX, BMPWIN_WAZA3_PY,
		BMPWIN_WAZA_SX, BMPWIN_WAZA_SY, BMPWIN_WAZA_PAL,
	},
	{	// 技４（スワップ用）
		BMPWIN_WAZA_FRM, BMPWIN_WAZA4_PX, BMPWIN_WAZA4_PY,
		BMPWIN_WAZA_SX, BMPWIN_WAZA_SY, BMPWIN_WAZA_PAL,
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

/** 下画面 **/
	{	//「てもちポケモン」
		BMPWIN_TEMOCHI_FRM, BMPWIN_TEMOCHI_PX, BMPWIN_TEMOCHI_PY,
		BMPWIN_TEMOCHI_SX, BMPWIN_TEMOCHI_SY, BMPWIN_TEMOCHI_PAL,
	},
	{	//「ポケモンいどう」
		BMPWIN_IDOU_FRM, BMPWIN_IDOU_PX, BMPWIN_IDOU_PY,
		BMPWIN_IDOU_SX, BMPWIN_IDOU_SY, BMPWIN_IDOU_PAL,
	},
	{	//「もどる」
		BMPWIN_MODORU_FRM, BMPWIN_MODORU_PX, BMPWIN_MODORU_PY,
		BMPWIN_MODORU_SX, BMPWIN_MODORU_SY, BMPWIN_MODORU_PAL,
	},
	{	//「やめる」
		BMPWIN_YAMERU_FRM, BMPWIN_YAMERU_PX, BMPWIN_YAMERU_PY,
		BMPWIN_YAMERU_SX, BMPWIN_YAMERU_SY, BMPWIN_YAMERU_PAL,
	},
	{	//「とじる」
		BMPWIN_TOZIRU_FRM, BMPWIN_TOZIRU_PX, BMPWIN_TOZIRU_PY,
		BMPWIN_TOZIRU_SX, BMPWIN_TOZIRU_SY, BMPWIN_TOZIRU_PAL,
	},

	{	// メニュー１
		BMPWIN_MENU1_FRM, BMPWIN_MENU1_PX, BMPWIN_MENU1_PY,
		BMPWIN_MENU1_SX, BMPWIN_MENU1_SY, BMPWIN_MENU1_PAL,
	},
	{	// メニュー２
		BMPWIN_MENU2_FRM, BMPWIN_MENU2_PX, BMPWIN_MENU2_PY,
		BMPWIN_MENU2_SX, BMPWIN_MENU2_SY, BMPWIN_MENU2_PAL,
	},
	{	// メニュー３
		BMPWIN_MENU3_FRM, BMPWIN_MENU3_PX, BMPWIN_MENU3_PY,
		BMPWIN_MENU3_SX, BMPWIN_MENU3_SY, BMPWIN_MENU3_PAL,
	},
	{	// メニュー４
		BMPWIN_MENU4_FRM, BMPWIN_MENU4_PX, BMPWIN_MENU4_PY,
		BMPWIN_MENU4_SX, BMPWIN_MENU4_SY, BMPWIN_MENU4_PAL,
	},
	{	// メニュー５
		BMPWIN_MENU5_FRM, BMPWIN_MENU5_PX, BMPWIN_MENU5_PY,
		BMPWIN_MENU5_SX, BMPWIN_MENU5_SY, BMPWIN_MENU5_PAL,
	},

	{	// マーキング決定
		BMPWIN_MARK_ENTER_FRM, BMPWIN_MARK_ENTER_PX, BMPWIN_MARK_ENTER_PY,
		BMPWIN_MARK_ENTER_SX, BMPWIN_MARK_ENTER_SY, BMPWIN_MARK_ENTER_PAL,
	},
	{	// マーキングキャンセル
		BMPWIN_MARK_CANCEL_FRM, BMPWIN_MARK_CANCEL_PX, BMPWIN_MARK_CANCEL_PY,
		BMPWIN_MARK_CANCEL_SX, BMPWIN_MARK_CANCEL_SY, BMPWIN_MARK_CANCEL_PAL,
	},

	{	// 手持ちポケモンフレーム・いれかえ
		BMPWIN_PARTY_CNG_FRM, BMPWIN_PARTY_CNG_PX, BMPWIN_PARTY_CNG_PY,
		BMPWIN_PARTY_CNG_SX, BMPWIN_PARTY_CNG_SY, BMPWIN_PARTY_CNG_PAL,
	},

	{	// メッセージ１
		BMPWIN_MSG1_FRM, BMPWIN_MSG1_PX, BMPWIN_MSG1_PY,
		BMPWIN_MSG1_SX, BMPWIN_MSG1_SY, BMPWIN_MSG1_PAL,
	},
	{	// メッセージ２
		BMPWIN_MSG1_FRM, BMPWIN_MSG2_PX, BMPWIN_MSG2_PY,
		BMPWIN_MSG1_SX, BMPWIN_MSG1_SY, BMPWIN_MSG1_PAL,
	},
	{	// メッセージ３
		BMPWIN_MSG1_FRM, BMPWIN_MSG1_PX, BMPWIN_MSG1_PY,
		BMPWIN_MSG3_SX, BMPWIN_MSG3_SY, BMPWIN_MSG1_PAL,
	},
	{	// メッセージ４
		BMPWIN_MSG1_FRM, BMPWIN_MSG1_PX, BMPWIN_MSG1_PY,
		BMPWIN_MSG1_SX, BMPWIN_MSG1_SY, BMPWIN_MSG1_PAL,
	},

	{	// ボックス移動のメニュー
		BMPWIN_BOXMV_MENU_FRM, BMPWIN_BOXMV_MENU_PX, BMPWIN_BOXMV_MENU_PY,
		BMPWIN_BOXMV_MENU_SX, BMPWIN_BOXMV_MENU_SY, BMPWIN_BOXMV_MENU_PAL,
	},

	{	//「ボックスをきりかえる」
		BMPWIN_BOXMV_BTN_FRM, BMPWIN_BOXMV_BTN_PX, BMPWIN_BOXMV_BTN_PY,
		BMPWIN_BOXMV_BTN_SX, BMPWIN_BOXMV_BTN_SY, BMPWIN_BOXMV_BTN_PAL,
	},
	{	// Ｙボタンステータス
		BMPWIN_Y_ST_BTN_FRM, BMPWIN_Y_ST_BTN_PX, BMPWIN_Y_ST_BTN_PY,
		BMPWIN_Y_ST_BTN_SX, BMPWIN_Y_ST_BTN_SY, BMPWIN_Y_ST_BTN_PAL,
	}
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
/*
	u32	i;

	FontProc_LoadFont( FONT_TOUCH, HEAPID_BOX_APP );

	for( i=0; i<BOX2BMPWIN_ID_MAX; i++ ){
		GF_BGL_BmpWinAddEx( syswk->app->bgl, &syswk->app->win[i], &BoxBmpWinData[i] );
	}
*/

	const u8 * dat;
	u32	i;

	GFL_BMPWIN_Init( HEAPID_BOX_APP );

	dat = BoxBmpWinData[0];
	for( i=0; i<BOX2BMPWIN_ID_MAX; i++ ){
		syswk->app->win[i].win = GFL_BMPWIN_Create(
															dat[0], dat[1], dat[2],
															dat[3], dat[4], dat[5],
															GFL_BMP_CHRAREA_GET_F );
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
 * @brief		メニューＢＧウィンドウフレーム作成
 *
 * @param		wk			BGウィンフレームワーク
 * @param		index		BGウィンフレームの番号
 * @param		win			BMPWIN
 *
 * @return	none
 *
 *	枠ありのウィンドウ
 */
//--------------------------------------------------------------------------------------------
static void MenuBgFrmWkSet2( BGWINFRM_WORK * wk, u32 index, GFL_BMPWIN * win )
{
/*
	u16 * scr;
	u16	chr;
	u16	pal;
	u16	sx, sy;
	u16	i, j;

	scr = BGWINFRM_FrameBufGet( wk, index );
	BGWINFRM_SizeGet( wk, index, &sx, &sy );

	chr = GFL_BMPWIN_GetChrNum( win );
//↑[GS_CONVERT_TAG]
	pal = win->palnum << 12;

	scr[0]			= pal + BOX2MAIN_SELWIN_CGX_OFF;
	scr[sx-1]		= pal + BOX2MAIN_SELWIN_CGX_OFF + 2;
	scr[sx*(sy-1)]	= pal + BOX2MAIN_SELWIN_CGX_OFF + 9;
	scr[sx*sy-1]	= pal + BOX2MAIN_SELWIN_CGX_OFF + 11;

	for( i=0; i<sx-2; i++ ){
		scr[i+1]			= pal + BOX2MAIN_SELWIN_CGX_OFF + 1;
		scr[sx*(sy-1)+i+1]	= pal + BOX2MAIN_SELWIN_CGX_OFF + 10;
	}

	for( i=0; i<sy-2; i++ ){
		scr[sx*(i+1)]	= pal + BOX2MAIN_SELWIN_CGX_OFF + 3;
		scr[sx*(i+2)-1]	= pal + BOX2MAIN_SELWIN_CGX_OFF + 5;
	}

	for( i=0; i<sy-2; i++ ){
		for( j=0; j<sx-2; j++ ){
			scr[(sx*(i+1)) + j+1] = pal + chr;
			chr++;
		}
	}
*/
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
/*
	BGWINFRM_Add(
		wk, index,
		GFL_BMPWIN_GetFrame( win ),
//↑[GS_CONVERT_TAG]
		GFL_BMPWIN_GetSizeX( win ),
//↑[GS_CONVERT_TAG]
		GFL_BMPWIN_GetSizeY( win ) );
//↑[GS_CONVERT_TAG]
	BGWINFRM_BmpWinOn( wk, index, win );
*/
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

	MenuBgFrmWkSet2( appwk->wfrm, BOX2MAIN_WINFRM_BOXMV_MENU, appwk->win[BOX2BMPWIN_ID_BOXMV_MENU].win );

	ButtonBgFrmMake2( appwk->wfrm, BOX2MAIN_WINFRM_BOXMV_BTN, appwk->win[BOX2BMPWIN_ID_BOXMV_BTN].win );
	ButtonBgFrmMake2( appwk->wfrm, BOX2MAIN_WINFRM_Y_ST_BTN, appwk->win[BOX2BMPWIN_ID_Y_STATUS].win );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧウィンドウフレーム作成：「てもちポケモン」
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_TemochiButtonBgFrmWkMake( BOX2_APP_WORK * appwk )
{
	ButtonBgFrmMake2( appwk->wfrm, BOX2MAIN_WINFRM_POKE_BTN, appwk->win[BOX2BMPWIN_ID_TEMOCHI].win );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧウィンドウフレーム作成：「ポケモンいどう」
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_IdouButtonBgFrmWkMake( BOX2_APP_WORK * appwk )
{
	ButtonBgFrmMake2( appwk->wfrm, BOX2MAIN_WINFRM_MV_BTN, appwk->win[BOX2BMPWIN_ID_IDOU].win );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧウィンドウフレーム作成：「もどる」「やめる」
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ModoruButtonBgFrmWkMake( BOX2_APP_WORK * appwk )
{
	ButtonBgFrmMake2( appwk->wfrm, BOX2MAIN_WINFRM_RET_BTN, appwk->win[BOX2BMPWIN_ID_MODORU].win );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧウィンドウフレーム作成：「とじる」
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ToziruButtonBgFrmWkMake( BOX2_SYS_WORK * syswk )
{
	BOX2_APP_WORK * appwk = syswk->app;

	ButtonBgFrmMake2( appwk->wfrm, BOX2MAIN_WINFRM_CLOSE_BTN, appwk->win[BOX2BMPWIN_ID_TOZIRU].win );

//	BOX2BMP_ButtonPutToziru( syswk, FALSE );
}






























#if 0
#include "system/fontproc.h"
#include "system/window.h"
#include "poketool/poke_number.h"
#include "itemtool/item.h"
#include "msgdata/msg.naix"
#include "msgdata/msg_boxmenu.h"
#include "msgdata/msg_boxmes.h"

#include "box2_obj.h"
#include "box_gra.naix"




//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static GFL_MSGDATA * BoxMsgManGet(void);
//↑[GS_CONVERT_TAG]






//--------------------------------------------------------------------------------------------
/**
 * 文字列表示（バッファ指定）
 *
 * @param	win		BMPWIN
 * @param	str		文字列バッファ
 * @param	x		表示基準Ｘ座標
 * @param	y		表示基準Ｙ座標
 * @param	fnt		フォント
 * @param	col		カラー
 * @param	mode	表示モード
 *
 * @return	none
 *
 * @li	mode = STRPRINT_MODE_LEFT	: 左詰
 * @li	mode = STRPRINT_MODE_RIGHT	: 右詰
 * @li	mode = STRPRINT_MODE_CENTER	: 中央
 */
//--------------------------------------------------------------------------------------------
static void StrPrintCore(
				GF_BGL_BMPWIN * win, STRBUF * str,
				u32 x, u32 y, FONT_TYPE fnt, PRINTSYS_LSB col, u32 mode )
//↑[GS_CONVERT_TAG]
{
	if( mode == STRPRINT_MODE_RIGHT ){
		x -= FontProc_GetPrintStrWidth( fnt, str, 0 );
	}else if( mode == STRPRINT_MODE_CENTER ){
		x -= ( FontProc_GetPrintStrWidth( fnt, str, 0 ) / 2 );
	}
	GF_STR_PrintColor( win, fnt, str, x, y, MSG_NO_PUT, col, NULL );
}

//--------------------------------------------------------------------------------------------
/**
 * 文字列表示（固定文字列）
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	man		メッセージマネージャ
 * @param	winID	BMPWIN ID
 * @param	msgID	文字列ＩＤ
 * @param	x		表示基準Ｘ座標
 * @param	y		表示基準Ｙ座標
 * @param	fnt		フォント
 * @param	col		カラー
 * @param	mode	表示モード
 *
 * @return	none
 *
 * @li	mode = STRPRINT_MODE_LEFT	: 左詰
 * @li	mode = STRPRINT_MODE_RIGHT	: 右詰
 * @li	mode = STRPRINT_MODE_CENTER	: 中央
 */
//--------------------------------------------------------------------------------------------
static void StrPrint(
				BOX2_APP_WORK * appwk, GFL_MSGDATA * man,
//↑[GS_CONVERT_TAG]
				u32 winID, u32 msgID, u32 x, u32 y, FONT_TYPE fnt, PRINTSYS_LSB col, u32 mode )
//↑[GS_CONVERT_TAG]
{
	STRBUF * str = MSGMAN_AllocString( man, msgID );

	StrPrintCore( &appwk->win[winID], str, x, y, fnt, col, mode );

	STRBUF_Delete( str );
}

//--------------------------------------------------------------------------------------------
/**
 * 文字列表示（可変文字列）
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	man		メッセージマネージャ
 * @param	winID	BMPWIN ID
 * @param	msgID	文字列ＩＤ
 * @param	x		表示基準Ｘ座標
 * @param	y		表示基準Ｙ座標
 * @param	fnt		フォント
 * @param	col		カラー
 * @param	mode	表示モード
 *
 * @return	none
 *
 * @li	mode = STRPRINT_MODE_LEFT	: 左詰
 * @li	mode = STRPRINT_MODE_RIGHT	: 右詰
 * @li	mode = STRPRINT_MODE_CENTER	: 中央
 */
//--------------------------------------------------------------------------------------------
static void ExStrPrint(
				BOX2_APP_WORK * appwk, GFL_MSGDATA * man,
//↑[GS_CONVERT_TAG]
				u32 winID, u32 msgID, u32 x, u32 y, FONT_TYPE fnt, PRINTSYS_LSB col, u32 mode )
//↑[GS_CONVERT_TAG]
{
	STRBUF * str = MSGMAN_AllocString( man, msgID );

	WORDSET_ExpandStr( appwk->wset, appwk->expbuf, str );

	StrPrintCore( &appwk->win[winID], appwk->expbuf, x, y, fnt, col, mode );

	STRBUF_Delete( str );
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
	GFL_BMP_Clear( &appwk->win[winID], 0 );
//↑[GS_CONVERT_TAG]

	if( info->tamago == 0 ){
		WORDSET_RegisterPokeMonsName( appwk->wset, 0, info->ppp );
		ExStrPrint(
			appwk, appwk->mman, winID,
			mes_boxmenu_01_01, 0, 0, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );
	}

	GFL_BMPWIN_MakeTransWindowVReq( &appwk->win[winID] );
//↑[GS_CONVERT_TAG]
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
	GFL_BMP_Clear( &appwk->win[winID], 0 );
//↑[GS_CONVERT_TAG]

	WORDSET_RegisterPokeNickName( appwk->wset, 0, info->ppp );
	ExStrPrint(
		appwk, appwk->mman, winID,
		mes_boxmenu_01_03, 0, 0, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );

	GFL_BMPWIN_MakeTransWindowVReq( &appwk->win[winID] );
//↑[GS_CONVERT_TAG]
}

#define	POKE_LVMARK_PY	( 5 )	// レベル表示Ｙ座標

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
	GFL_BMP_Clear( &appwk->win[winID], 0 );
//↑[GS_CONVERT_TAG]

	if( info->tamago == 0 ){
		NUMFONT_WriteMark( appwk->nfnt, NUMFONT_MARK_LV, &appwk->win[winID], 0, POKE_LVMARK_PY );
		WORDSET_RegisterNumber(
			appwk->wset, 0, info->lv, 3, StrNumberDispType_LEFT, StrNumberCodeType_DEFAULT );
//↑[GS_CONVERT_TAG]
//↑[GS_CONVERT_TAG]
		ExStrPrint(
			appwk, appwk->mman, winID,
			mes_box_subst_01_09, NUMFONT_MARK_WIDTH, 0, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );
	}

	GFL_BMPWIN_MakeTransWindowVReq( &appwk->win[winID] );
//↑[GS_CONVERT_TAG]
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
	GFL_BMP_Clear( &appwk->win[winID], 0 );
//↑[GS_CONVERT_TAG]

	if( info->tamago == 0 && info->sex_put == 1 ){
		if( info->sex == PARA_MALE ){
			StrPrint(
				appwk, appwk->mman,
				winID, mes_box_subst_01_01, 0, 0, FONT_SYSTEM, FNTCOL_N_BLUE, STRPRINT_MODE_LEFT );
		}else if( info->sex == PARA_FEMALE ){
			StrPrint(
				appwk, appwk->mman,
				winID, mes_box_subst_01_02, 0, 0, FONT_SYSTEM, FNTCOL_N_RED, STRPRINT_MODE_LEFT );
		}
	}

	GFL_BMPWIN_MakeTransWindowVReq( &appwk->win[winID] );
//↑[GS_CONVERT_TAG]
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
	
	GFL_BMP_Clear( &syswk->app->win[winID], 0 );
//↑[GS_CONVERT_TAG]

	if( info->tamago == 0 ){
		mode = PMNumber_GetMode( syswk->dat->savedata );
		num  = PMNumber_GetPokeNumber( mode, info->mons );

		if( num != 0 ){
			NUMFONT_WriteMark(
				syswk->app->nfnt, NUMFONT_MARK_NO, &syswk->app->win[winID], 0, POKE_NOMARK_PY );

			WORDSET_RegisterNumber(
				syswk->app->wset, 0, num, 3, StrNumberDispType_ZERO, StrNumberCodeType_DEFAULT );
//↑[GS_CONVERT_TAG]
//↑[GS_CONVERT_TAG]
			ExStrPrint(
				syswk->app, syswk->app->mman, winID,
				mes_box_subst_01_10, NUMFONT_MARK_WIDTH, 0, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );
		}
	}

	GFL_BMPWIN_MakeTransWindowVReq( &syswk->app->win[winID] );
//↑[GS_CONVERT_TAG]
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
	GFL_BMP_Clear( &appwk->win[winID], 0 );
//↑[GS_CONVERT_TAG]

	if( info->tamago == 0 ){
		WORDSET_RegisterSeikaku( appwk->wset, 0, info->seikaku );
		ExStrPrint(
			appwk, appwk->mman, winID,
			mes_box_subst_01_04, 0, 0, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );
	}else{
		StrPrint(
			appwk, appwk->mman,
			winID, mes_box_subst_01_12, 0, 0, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );
	}

	GFL_BMPWIN_MakeTransWindowVReq( &appwk->win[winID] );
//↑[GS_CONVERT_TAG]
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
	GFL_BMP_Clear( &appwk->win[winID], 0 );
//↑[GS_CONVERT_TAG]

	if( info->tamago == 0 ){
		WORDSET_RegisterTokuseiName( appwk->wset, 0, info->tokusei );
		ExStrPrint(
			appwk, appwk->mman, winID,
			mes_box_subst_01_03, 0, 0, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );
	}else{
		StrPrint(
			appwk, appwk->mman,
			winID, mes_box_subst_01_12, 0, 0, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );
	}

	GFL_BMPWIN_MakeTransWindowVReq( &appwk->win[winID] );
//↑[GS_CONVERT_TAG]
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
	GFL_BMP_Clear( &appwk->win[winID], 0 );
//↑[GS_CONVERT_TAG]

	if( info->item != ITEM_DUMMY_DATA ){
		WORDSET_RegisterItemName( appwk->wset, 0, info->item );
		ExStrPrint(
			appwk, appwk->mman, winID,
			mes_box_subst_01_05, 0, 0, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );
	}else{
		StrPrint(
			appwk, appwk->mman,
			winID, mes_box_subst_01_11, 0, 0, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );
	}

	GFL_BMPWIN_MakeTransWindowVReq( &appwk->win[winID] );
//↑[GS_CONVERT_TAG]
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
	GF_BGL_BmpWinOffVReq( &appwk->win[BOX2BMPWIN_ID_NAME] );
	GF_BGL_BmpWinOffVReq( &appwk->win[BOX2BMPWIN_ID_NICKNAME] );
	GF_BGL_BmpWinOffVReq( &appwk->win[BOX2BMPWIN_ID_LV] );
	GF_BGL_BmpWinOffVReq( &appwk->win[BOX2BMPWIN_ID_SEX] );
	GF_BGL_BmpWinOffVReq( &appwk->win[BOX2BMPWIN_ID_ZKNNUM] );
	GF_BGL_BmpWinOffVReq( &appwk->win[BOX2BMPWIN_ID_SEIKAKU] );
	GF_BGL_BmpWinOffVReq( &appwk->win[BOX2BMPWIN_ID_TOKUSEI] );
	GF_BGL_BmpWinOffVReq( &appwk->win[BOX2BMPWIN_ID_ITEM] );
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
u32 BOX2BMP_PokeSkillWrite( BOX2_SYS_WORK * syswk, BOX2_POKEINFO_DATA * info )
{
	NNSG2dCharacterData * chr;
	void * buf;
	u8 * raw;

	GFL_MSGDATA * man;
//↑[GS_CONVERT_TAG]
	u32	winID;
	u16	i, j;

	if( syswk->app->subdisp_win_swap == 0 ){
		winID = BOX2BMPWIN_ID_WAZA1;
	}else{
		winID = BOX2BMPWIN_ID_WAZA1_2;
	}
	syswk->app->subdisp_win_swap ^= 1;

	buf = GFL_ARC_UTIL_LoadBGCharacter or GFL_ARC_UTIL_LoadOBJCharacter( ARC_BOX2_GRA, NARC_box_gra_box_s_bg3_lz_NCGR, TRUE, &chr, HEAPID_BOX_APP );
//↑[GS_CONVERT_TAG]
	raw = chr->pRawData;

	man = MSGMAN_Create( GflMsgLoadType_NORMAL, ARC_MSG, NARC_msg_wazaname_dat, HEAPID_BOX_APP );
//↑[GS_CONVERT_TAG]

	if( info->tamago == 0 ){
		for( i=0; i<4; i++ ){
			for( j=0; j<BMPWIN_WAZA_SX; j++ ){
				GF_BGL_BmpWinPrintEx(
					&syswk->app->win[winID+i],
					&raw[0x20*0x0b],
					0, 0, 8, 8,
					8*j, 0, 8, 8,
					0xff );
				GF_BGL_BmpWinPrintEx(
					&syswk->app->win[winID+i],
					&raw[0x20*0x01],
					0, 0, 8, 8,
					8*j, 8, 8, 8,
					0xff );
			}
			StrPrint(
				syswk->app, man, winID+i, info->waza[i],
				0, 0, FONT_SYSTEM, FNTCOL_SUB_WIN, STRPRINT_MODE_LEFT );
			GFL_BMPWIN_TransVramCharacter( &syswk->app->win[winID+i] );
//↑[GS_CONVERT_TAG]
		}
		syswk->app->subdisp_waza_put = 1;
	}else{
		syswk->app->subdisp_waza_put = 0;
	}

	MSGMAN_Delete( man );

	GFL_HEAP_FreeMemory( buf );
//↑[GS_CONVERT_TAG]

	return winID;
}

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
u32 BOX2BMP_PokeItemInfoWrite( BOX2_SYS_WORK * syswk, u16 item )
{
	GF_BGL_BMPWIN * win;
	u32	winID;

	if( syswk->app->subdisp_win_swap == 0 ){
		winID = BOX2BMPWIN_ID_ITEMLABEL;
	}else{
		winID = BOX2BMPWIN_ID_ITEMLABEL2;
	}
	win = &syswk->app->win[winID];

	syswk->app->subdisp_win_swap ^= 1;

	GFL_BMP_Clear( &win[0], 13 );
//↑[GS_CONVERT_TAG]
	GFL_BMP_Clear( &win[1], 13 );
//↑[GS_CONVERT_TAG]

	if( item != ITEM_DUMMY_DATA ){
		GetItemName( syswk->app->expbuf, item, HEAPID_BOX_APP );
		StrPrintCore(
			&win[0], syswk->app->expbuf, 0, 0, FONT_SYSTEM, FNTCOL_SUB_WIN, STRPRINT_MODE_LEFT );
		GFL_BMPWIN_TransVramCharacter( &win[0] );
//↑[GS_CONVERT_TAG]

		ItemInfoGet( syswk->app->expbuf, item, HEAPID_BOX_APP );
		StrPrintCore(
			&win[1], syswk->app->expbuf, 0, 0, FONT_SYSTEM, FNTCOL_SUB_WIN, STRPRINT_MODE_LEFT );
		GFL_BMPWIN_TransVramCharacter( &win[1] );
//↑[GS_CONVERT_TAG]
	}

	return winID;
}

//--------------------------------------------------------------------------------------------
/**
 * タイトル表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_TitlePut( BOX2_SYS_WORK * syswk )
{
	GFL_MSGDATA * man = BoxMsgManGet();
//↑[GS_CONVERT_TAG]

	GFL_BMP_Clear( &syswk->app->win[BOX2BMPWIN_ID_TITLE], 0 );
//↑[GS_CONVERT_TAG]

	StrPrint(
		syswk->app, man, BOX2BMPWIN_ID_TITLE,
		msg_boxmes_03_01+syswk->dat->mode,
		0, TITLE_STR_PY, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );

	MSGMAN_Delete( man );

	GFL_BMPWIN_MakeTransWindow( &syswk->app->win[BOX2BMPWIN_ID_TITLE] );
//↑[GS_CONVERT_TAG]
}

//--------------------------------------------------------------------------------------------
/**
 * デフォルト文字列表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_DefStrPut( BOX2_SYS_WORK * syswk )
{
	GFL_BMP_Clear( &syswk->app->win[BOX2BMPWIN_ID_SEIKAKU_STR], 0 );
//↑[GS_CONVERT_TAG]
	GFL_BMP_Clear( &syswk->app->win[BOX2BMPWIN_ID_TOKUSEI_STR], 0 );
//↑[GS_CONVERT_TAG]
	GFL_BMP_Clear( &syswk->app->win[BOX2BMPWIN_ID_ITEM_STR], 0 );
//↑[GS_CONVERT_TAG]

	StrPrint(
		syswk->app, syswk->app->mman,
		BOX2BMPWIN_ID_SEIKAKU_STR, mes_box_subst_01_06,
		0, 0, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );

	StrPrint(
		syswk->app, syswk->app->mman,
		BOX2BMPWIN_ID_TOKUSEI_STR, mes_box_subst_01_07,
		0, 0, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );

	StrPrint(
		syswk->app, syswk->app->mman,
		BOX2BMPWIN_ID_ITEM_STR, mes_box_subst_01_08,
		0, 0, FONT_SYSTEM, FNTCOL_N_BLACK, STRPRINT_MODE_LEFT );

	GFL_BMPWIN_MakeTransWindow( &syswk->app->win[BOX2BMPWIN_ID_SEIKAKU_STR] );
//↑[GS_CONVERT_TAG]
	GFL_BMPWIN_MakeTransWindow( &syswk->app->win[BOX2BMPWIN_ID_TOKUSEI_STR] );
//↑[GS_CONVERT_TAG]
	GFL_BMPWIN_MakeTransWindow( &syswk->app->win[BOX2BMPWIN_ID_ITEM_STR] );
//↑[GS_CONVERT_TAG]
}


//--------------------------------------------------------------------------------------------
/**
 * トレイ名表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	cgx		背景キャラデータ
 * @param	chr		転送キャラ位置
 * @param	sx		Ｘサイズ
 * @param	sy		Ｙサイズ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_TrayNamePut( BOX2_SYS_WORK * syswk, u8 * cgx, u32 chr, u32 sx, u32 sy )
{
	GF_BGL_BMPWIN * win;
	STRBUF * str;
	u32	i;

	win = GFL_HEAP_AllocMemoryLo( HEAPID_BOX_APP, sizeof(GF_BGL_BMPWIN) );
//↑[GS_CONVERT_TAG]
	GF_BGL_BmpWinAdd( syswk->app->bgl, win, BOX2MAIN_BGF_WALL_M, 0, 0, sx, sy, 0, chr );
	MI_CpuCopy32( cgx, win->chrbuf, sx*sy*0x20 );

	str = STRBUF_Create( BOX_TRAYNAME_BUFSIZE, HEAPID_BOX_APP );
	BOXDAT_GetBoxName( syswk->box, syswk->tray, str );
	StrPrintCore( win, str, sx*8/2, sy*8/2-8, FONT_SYSTEM, FNTCOL_TRAYNAME, STRPRINT_MODE_CENTER );
	STRBUF_Delete( str );

	GFL_BMPWIN_TransVramCharacter( win );
//↑[GS_CONVERT_TAG]
	GFL_BMPWIN_Delete( win );
//↑[GS_CONVERT_TAG]
	GFL_HEAP_FreeMemory( win );
//↑[GS_CONVERT_TAG]
}

//--------------------------------------------------------------------------------------------
/**
 * ボックス移動処理のボックス名・収納数表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_BoxMoveNameNumWrite( BOX2_SYS_WORK * syswk )
{
	GF_BGL_BMPWIN	win;
	STRBUF * str;

	// 名前
	GF_BGL_BmpWinInit( &win );
	GF_BGL_BmpWinObjAdd(
		syswk->app->bgl, &win,
		BOX2OBJ_FNTOAM_BOXNAME_SX, BOX2OBJ_FNTOAM_BOXNAME_SY, 0, 0 );

	str = STRBUF_Create( BOX_TRAYNAME_BUFSIZE, HEAPID_BOX_APP );
	BOXDAT_GetBoxName( syswk->box, syswk->box_mv_pos, str );
	StrPrintCore(
		&win, str, BOX2OBJ_FNTOAM_BOXNAME_SX*8/2, 0,
		FONT_SYSTEM, FNTCOL_FNTOAM, STRPRINT_MODE_CENTER );
	STRBUF_Delete( str );

	BOX2OBJ_FontOamResetBmp( syswk->app, &win, BOX2MAIN_FNTOAM_TRAY_NAME );

	GFL_BMPWIN_Delete( &win );
//↑[GS_CONVERT_TAG]

	// 数字
	GF_BGL_BmpWinInit( &win );
	GF_BGL_BmpWinObjAdd(
		syswk->app->bgl, &win,
		BOX2OBJ_FNTOAM_BOXNUM_SX, BOX2OBJ_FNTOAM_BOXNUM_SY, 0, 0 );

	str = MSGMAN_AllocString( syswk->app->mman, mes_boxmenu_02_20 );

	WORDSET_RegisterNumber(
		syswk->app->wset, 0,
		BOXDAT_GetPokeExistCount(syswk->box,syswk->box_mv_pos),
		2, NUM_MODE_LEFT, StrNumberCodeType_DEFAULT );
//↑[GS_CONVERT_TAG]
	WORDSET_RegisterNumber(
		syswk->app->wset, 1,
		BOX_MAX_POS,
		2, NUM_MODE_LEFT, StrNumberCodeType_DEFAULT );
//↑[GS_CONVERT_TAG]

	WORDSET_ExpandStr( syswk->app->wset, syswk->app->expbuf, str );

	StrPrintCore(
		&win, syswk->app->expbuf, BOX2OBJ_FNTOAM_BOXNUM_SX*8/2, 0,
		FONT_SYSTEM, FNTCOL_FNTOAM, STRPRINT_MODE_CENTER );
	STRBUF_Delete( str );

	BOX2OBJ_FontOamResetBmp( syswk->app, &win, BOX2MAIN_FNTOAM_TRAY_NUM );

	GFL_BMPWIN_Delete( &win );
//↑[GS_CONVERT_TAG]
}

//--------------------------------------------------------------------------------------------
/**
 * 壁紙変更処理の名前表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_WallPaperNameWrite( BOX2_SYS_WORK * syswk )
{
	GF_BGL_BMPWIN	win;
	STRBUF * str;

	// 名前
	GF_BGL_BmpWinInit( &win );
	GF_BGL_BmpWinObjAdd( syswk->app->bgl, &win, 12, 2, 0, 0 );

	if( syswk->app->wallpaper_pos >= BOX_NORMAL_WALLPAPER_MAX ){
		if( BOXDAT_GetDaisukiKabegamiFlag( syswk->box, syswk->app->wallpaper_pos-BOX_NORMAL_WALLPAPER_MAX ) == FALSE ){
			str = MSGMAN_AllocString( syswk->app->mman, mes_boxmenu_04_31 );
		}else{
			str = MSGMAN_AllocString( syswk->app->mman, mes_boxmenu_04_07+syswk->app->wallpaper_pos );
		}
	}else{
		str = MSGMAN_AllocString( syswk->app->mman, mes_boxmenu_04_07+syswk->app->wallpaper_pos );
	}

	StrPrintCore(
		&win, str, 12*8/2, 0,
		FONT_SYSTEM, FNTCOL_FNTOAM, STRPRINT_MODE_CENTER );
	STRBUF_Delete( str );

	BOX2OBJ_FontOamResetBmp( syswk->app, &win, BOX2MAIN_FNTOAM_TRAY_NAME );

	GFL_BMPWIN_Delete( &win );
//↑[GS_CONVERT_TAG]
}

//--------------------------------------------------------------------------------------------
/**
 * ボタン作成（ボタン全体がＢＭＰ）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	winID	BMPWIN ID
 * @param	strID	文字列ＩＤ
 * @param	sw		選択フラグ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ButtonPut_S( BOX2_SYS_WORK * syswk, u32 winID, const u32 strID, BOOL sw )
{
	GF_BGL_BMPWIN * win;
	STRBUF * str;
	void * buf;
	u8 * raw;
	NNSG2dCharacterData * chr;
	PRINTSYS_LSB	col;
//↑[GS_CONVERT_TAG]
	u16	cgx;
	u8	frm, px, py, sx, sy;
	u8	i;

	win = &syswk->app->win[ winID ];
	frm = GFL_BMPWIN_GetFrame( win );
//↑[GS_CONVERT_TAG]
	px  = GFL_BMPWIN_GetPosX( win );
//↑[GS_CONVERT_TAG]
	py  = GFL_BMPWIN_GetPosY( win );
//↑[GS_CONVERT_TAG]
	sx  = GFL_BMPWIN_GetSizeX( win );
//↑[GS_CONVERT_TAG]
	sy  = GFL_BMPWIN_GetSizeY( win );
//↑[GS_CONVERT_TAG]

	if( sw == TRUE ){
		GFL_BMP_Clear( win, FNTCOL_SELWIN_ON_B );
//↑[GS_CONVERT_TAG]
		col = FNTCOL_SELWIN_ON;
		cgx = BOX2MAIN_SELWIN_CGX_SIZ * 0x20;
	}else{
		GFL_BMP_Clear( win, FNTCOL_SELWIN_OFF_B );
//↑[GS_CONVERT_TAG]
		col = FNTCOL_SELWIN_OFF;
		cgx = 0;
	}

	buf = GFL_ARC_UTIL_LoadBGCharacter or GFL_ARC_UTIL_LoadOBJCharacter( ARC_BOX2_GRA, NARC_box_gra_selwin_lz_NCGR, TRUE, &chr, HEAPID_BOX_APP );
//↑[GS_CONVERT_TAG]
	raw = chr->pRawData;

	GF_BGL_BmpWinPrintEx( win, &raw[cgx+0x20* 0], 0, 0, 8, 8, 0, 0, 8, 8, 0xff );				// 左上
	GF_BGL_BmpWinPrintEx( win, &raw[cgx+0x20* 2], 0, 0, 8, 8, (sx-1)*8, 0, 8, 8, 0xff );		// 右上

	GF_BGL_BmpWinPrintEx( win, &raw[cgx+0x20* 9], 0, 0, 8, 8, 0, (sy-1)*8, 8, 8, 0xff );		// 左下
	GF_BGL_BmpWinPrintEx( win, &raw[cgx+0x20*11], 0, 0, 8, 8, (sx-1)*8, (sy-1)*8, 8, 8, 0xff );	// 右下

	for( i=1; i<sy-1; i++ ){
		GF_BGL_BmpWinPrintEx( win, &raw[cgx+0x20*3], 0, 0, 8, 8, 0, i*8, 8, 8, 0xff );				// 左
		GF_BGL_BmpWinPrintEx( win, &raw[cgx+0x20*5], 0, 0, 8, 8, (sx-1)*8, i*8, 8, 8, 0xff );		// 右
	}
	for( i=1; i<sx-1; i++ ){
		GF_BGL_BmpWinPrintEx( win, &raw[cgx+0x20* 1], 0, 0, 8, 8, i*8, 0, 8, 8, 0xff );				// 上
		GF_BGL_BmpWinPrintEx( win, &raw[cgx+0x20*10], 0, 0, 8, 8, i*8, (sy-1)*8, 8, 8, 0xff );		// 下
	}

	GFL_HEAP_FreeMemory( buf );
//↑[GS_CONVERT_TAG]

	str = MSGMAN_AllocString( syswk->app->mman, strID );
	StrPrintCore( win, str, sx*8/2, 4, FONT_TOUCH, col, STRPRINT_MODE_CENTER );
	STRBUF_Delete( str );

	GFL_BMPWIN_TransVramCharacter( win );
//↑[GS_CONVERT_TAG]
}

//--------------------------------------------------------------------------------------------
/**
 * ボタン作成（枠あり）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	winID	BMPWIN ID
 * @param	strID	文字列ＩＤ
 * @param	sw		選択フラグ
 * @param	mode	表示モード
 *
 * @return	none
 *
 * @li	mode = STRPRINT_MODE_LEFT	: 左詰
 * @li	mode = STRPRINT_MODE_RIGHT	: 右詰
 * @li	mode = STRPRINT_MODE_CENTER	: 中央
 */
//--------------------------------------------------------------------------------------------
static void ButtonPut( BOX2_SYS_WORK * syswk, u32 winID, const u32 strID, BOOL sw, u8 mode )
{
	GF_BGL_BMPWIN * win;
	STRBUF * str;
	PRINTSYS_LSB	col;
//↑[GS_CONVERT_TAG]
	u16	cgx;
	u8	frm, px, py, sx, sy;

	win = &syswk->app->win[ winID ];
	frm = GFL_BMPWIN_GetFrame( win );
//↑[GS_CONVERT_TAG]
	px  = GFL_BMPWIN_GetPosX( win );
//↑[GS_CONVERT_TAG]
	py  = GFL_BMPWIN_GetPosY( win );
//↑[GS_CONVERT_TAG]
	sx  = GFL_BMPWIN_GetSizeX( win );
//↑[GS_CONVERT_TAG]
	sy  = GFL_BMPWIN_GetSizeY( win );
//↑[GS_CONVERT_TAG]

	if( sw == TRUE ){
		GFL_BMP_Clear( win, FNTCOL_SELWIN_ON_B );
//↑[GS_CONVERT_TAG]
		col = FNTCOL_SELWIN_ON;
		cgx = BOX2MAIN_SELWIN_CGX_ON;
	}else{
		GFL_BMP_Clear( win, FNTCOL_SELWIN_OFF_B );
//↑[GS_CONVERT_TAG]
		col = FNTCOL_SELWIN_OFF;
		cgx = BOX2MAIN_SELWIN_CGX_OFF;
	}

	str = MSGMAN_AllocString( syswk->app->mman, strID );

	if( mode == STRPRINT_MODE_CENTER ){
		StrPrintCore( win, str, sx*8/2, 0, FONT_TOUCH, col, mode );
	}else{
		StrPrintCore( win, str, 0, 0, FONT_TOUCH, col, mode );
	}

	STRBUF_Delete( str );

	GFL_BMPWIN_TransVramCharacter( win );
//↑[GS_CONVERT_TAG]
}

//--------------------------------------------------------------------------------------------
/**
 * 「てもちポケモン」ボタン作成
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	sw		選択フラグ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ButtonPutTemochi( BOX2_SYS_WORK * syswk, BOOL sw )
{
	ButtonPut_S( syswk, BOX2BMPWIN_ID_TEMOCHI, mes_boxbutton_01_01, sw );
}

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンいどう」ボタン作成（持ち物整理モードの場合は「もちものせいり」）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	winID	BMPWIN ID
 * @param	strID	文字列ＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeMoveButtonPut( BOX2_SYS_WORK * syswk, u32 winID, u32 strID )
{
	STRBUF * str;
	NNSG2dCharacterData * chr;
	GF_BGL_BMPWIN * win;
	void * buf;
	u8 * raw;
	u8	sx;
	u8	i;

	win = &syswk->app->win[ winID ];
	sx  = GFL_BMPWIN_GetSizeX( win );
//↑[GS_CONVERT_TAG]

	buf = GFL_ARC_UTIL_LoadBGCharacter or GFL_ARC_UTIL_LoadOBJCharacter(
//↑[GS_CONVERT_TAG]
			ARC_BOX2_GRA, NARC_box_gra_box_m_bg1_lz_NCGR, TRUE, &chr, HEAPID_BOX_APP );
	raw = chr->pRawData;

	GF_BGL_BmpWinPrintEx( win, &raw[PMV_UL_CGX_POS], 0, 0, 8*3, 8, 0,  0, 8*3, 8, 0xff );	// 左上
	GF_BGL_BmpWinPrintEx( win, &raw[PMV_CL_CGX_POS], 0, 0, 8*3, 8, 0,  8, 8*3, 8, 0xff );	// 左中
	GF_BGL_BmpWinPrintEx( win, &raw[PMV_DL_CGX_POS], 0, 0, 8*3, 8, 0, 16, 8*3, 8, 0xff );	// 左下

	GF_BGL_BmpWinPrintEx( win, &raw[PMV_UR_CGX_POS], 0, 0, 8, 8, (sx-1)*8,  0, 8, 8, 0xff );	// 右上
	GF_BGL_BmpWinPrintEx( win, &raw[PMV_CR_CGX_POS], 0, 0, 8, 8, (sx-1)*8,  8, 8, 8, 0xff );	// 右中
	GF_BGL_BmpWinPrintEx( win, &raw[PMV_DR_CGX_POS], 0, 0, 8, 8, (sx-1)*8, 16, 8, 8, 0xff );	// 右下

	for( i=1; i<sx-1; i++ ){
		GF_BGL_BmpWinPrintEx( win, &raw[PMV_UC_CGX_POS], 0, 0, 8, 8, i*8,  0, 8, 8, 0xff );	// 上
		GF_BGL_BmpWinPrintEx( win, &raw[PMV_CC_CGX_POS], 0, 0, 8, 8, i*8,  8, 8, 8, 0xff );	// 中
		GF_BGL_BmpWinPrintEx( win, &raw[PMV_DC_CGX_POS], 0, 0, 8, 8, i*8, 16, 8, 8, 0xff );	// 下
	}

	GFL_HEAP_FreeMemory( buf );
//↑[GS_CONVERT_TAG]

	str = MSGMAN_AllocString( syswk->app->mman, strID );
	StrPrintCore( win, str, sx*8/2, 4, FONT_TOUCH, FNTCOL_YSTWIN, STRPRINT_MODE_CENTER );
	STRBUF_Delete( str );

	GFL_BMPWIN_TransVramCharacter( win );
//↑[GS_CONVERT_TAG]
}

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンいどう」ボタン作成（持ち物整理モードの場合は「もちものせいり」）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	sw		選択フラグ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ButtonPutIdou( BOX2_SYS_WORK * syswk, BOOL sw )
{
	if( syswk->dat->mode == BOX_MODE_ITEM ){
		PokeMoveButtonPut( syswk, BOX2BMPWIN_ID_IDOU, mes_boxbutton_01_05 );
	}else{
		PokeMoveButtonPut( syswk, BOX2BMPWIN_ID_IDOU, mes_boxbutton_01_02 );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 「もどる」ボタン作成
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	sw		選択フラグ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ButtonPutModoru( BOX2_SYS_WORK * syswk, BOOL sw )
{
	ButtonPut_S( syswk, BOX2BMPWIN_ID_MODORU, mes_boxbutton_01_03, sw );
}

//--------------------------------------------------------------------------------------------
/**
 * 「やめる」ボタン作成
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	sw		選択フラグ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ButtonPutYameru( BOX2_SYS_WORK * syswk, BOOL sw )
{
	ButtonPut_S( syswk, BOX2BMPWIN_ID_YAMERU, mes_boxbutton_04_03, sw );
}

//--------------------------------------------------------------------------------------------
/**
 * 「とじる」ボタン作成
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	sw		選択フラグ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ButtonPutToziru( BOX2_SYS_WORK * syswk, BOOL sw )
{
	ButtonPut_S( syswk, BOX2BMPWIN_ID_TOZIRU, mes_boxbutton_02_07, sw );
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

	BGWINFRM_SizeGet( syswk->app->wfrmwk, BOX2MAIN_WINFRM_MENU1, &sx, &sy );

	for( i=0; i<max; i++ ){
		if( list[max-1-i].type == BOX2BMP_BUTTON_TYPE_WHITE ){
			ButtonPut_S( syswk, BOX2BMPWIN_ID_MENU5-i, list[max-1-i].strID, FALSE );
			BGWINFRM_PaletteChange(
				syswk->app->wfrmwk, BOX2MAIN_WINFRM_MENU5-i, 0, 0, sx, sy, BOX2MAIN_BG_PAL_SELWIN );
		}else{
			PokeMoveButtonPut( syswk, BOX2BMPWIN_ID_MENU5-i, list[max-1-i].strID );
			BGWINFRM_PaletteChange(
				syswk->app->wfrmwk, BOX2MAIN_WINFRM_MENU5-i, 0, 0, sx, sy, 2 );
		}
	}
	// いらないものは消す
	for( i=max; i<5; i++ ){
		GFL_BMP_Clear( &syswk->app->win[BOX2BMPWIN_ID_MENU5-i], 0 );
//↑[GS_CONVERT_TAG]
		GFL_BMPWIN_TransVramCharacter( &syswk->app->win[BOX2BMPWIN_ID_MENU5-i] );
//↑[GS_CONVERT_TAG]
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
		GF_BGL_BmpWinOffVReq( &syswk->app->win[BOX2BMPWIN_ID_MENU5-i] );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 道具整理メニュー作成
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	item	アイテム番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ItemArrangeMenuStrPrint( BOX2_SYS_WORK * syswk, u16 item )
{
	GFL_BMP_Clear( &syswk->app->win[BOX2BMPWIN_ID_MENU1], 0 );
//↑[GS_CONVERT_TAG]
	GFL_BMP_Clear( &syswk->app->win[BOX2BMPWIN_ID_MENU2], 0 );
//↑[GS_CONVERT_TAG]
	GFL_BMP_Clear( &syswk->app->win[BOX2BMPWIN_ID_MENU3], 0 );
//↑[GS_CONVERT_TAG]
	GFL_BMP_Clear( &syswk->app->win[BOX2BMPWIN_ID_MENU4], 0 );
//↑[GS_CONVERT_TAG]
	GFL_BMPWIN_TransVramCharacter( &syswk->app->win[BOX2BMPWIN_ID_MENU1] );
//↑[GS_CONVERT_TAG]
	GFL_BMPWIN_TransVramCharacter( &syswk->app->win[BOX2BMPWIN_ID_MENU2] );
//↑[GS_CONVERT_TAG]
	GFL_BMPWIN_TransVramCharacter( &syswk->app->win[BOX2BMPWIN_ID_MENU3] );
//↑[GS_CONVERT_TAG]
	GFL_BMPWIN_TransVramCharacter( &syswk->app->win[BOX2BMPWIN_ID_MENU4] );
//↑[GS_CONVERT_TAG]

	if( item == 0 ){
		ButtonPut_S( syswk, BOX2BMPWIN_ID_MENU5, mes_boxbutton_08_02, FALSE );
	}else{
		ButtonPut_S( syswk, BOX2BMPWIN_ID_MENU5, mes_boxbutton_08_01, FALSE );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」ボタン作成
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_BoxMoveButtonPrint( BOX2_SYS_WORK * syswk )
{
	ButtonPut_S( syswk, BOX2BMPWIN_ID_BOXMV_BTN, mes_boxbutton_01_04, FALSE );
}

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」作成
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_BoxMoveMenuPrint( BOX2_SYS_WORK * syswk )
{
	ButtonPut( syswk, BOX2BMPWIN_ID_BOXMV_MENU, mes_boxbutton_05_01, FALSE, STRPRINT_MODE_LEFT );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックステーマメニュー作成
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_BoxThemaMenuPrint( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_BoxMoveMenuPrint( syswk );
	ButtonPut_S( syswk, BOX2BMPWIN_ID_MENU4, mes_boxmenu_03_02, FALSE );
	ButtonPut_S( syswk, BOX2BMPWIN_ID_MENU5, mes_boxmenu_03_03, FALSE );
}

//--------------------------------------------------------------------------------------------
/**
 * 壁紙メニュー作成
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_WallPaperChgMenuPrint( BOX2_SYS_WORK * syswk )
{
	ButtonPut( syswk, BOX2BMPWIN_ID_BOXMV_MENU, mes_boxbutton_06_01, FALSE, STRPRINT_MODE_LEFT );
}

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンを預ける」メニュー作成
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_PartyOutMenuPrint( BOX2_SYS_WORK * syswk )
{
	ButtonPut( syswk, BOX2BMPWIN_ID_BOXMV_MENU, mes_boxbutton_07_01, FALSE, STRPRINT_MODE_LEFT );
}


//--------------------------------------------------------------------------------------------
/**
 * ＢＧウィンドウフレーム内のボタンに文字列描画
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	winID	BMPWIN ID
 * @param	strID	文字列ＩＤ
 * @param	sw		選択フラグ
 *
 * @return	none
 *
 *	マーキングフレーム、手持ちポケモンフレームなど
 */
//--------------------------------------------------------------------------------------------
static void FrmStrPut( BOX2_APP_WORK * appwk, u32 winID, u32 strID, BOOL sw )
{
	u8	px;

	GFL_BMP_Clear( &appwk->win[winID], FRMBUTTON_BG_COL );
//↑[GS_CONVERT_TAG]

	px = GFL_BMPWIN_GetSizeX( &appwk->win[winID] ) * 8 / 2;
//↑[GS_CONVERT_TAG]
	StrPrint(
		appwk, appwk->mman, winID, strID, px, 0,
		FONT_TOUCH, FNTCOL_FRMBUTTN_OFF, STRPRINT_MODE_CENTER );

	GFL_BMPWIN_TransVramCharacter( &appwk->win[winID] );
//↑[GS_CONVERT_TAG]
}

//--------------------------------------------------------------------------------------------
/**
 * マーキングフレームのボタン描画
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_MarkingButtonPut( BOX2_APP_WORK * appwk )
{
	FrmStrPut( appwk, BMPWIN_MARK_ENTER, mes_boxbutton_03_01, FALSE );
	FrmStrPut( appwk, BMPWIN_MARK_CANCEL, mes_boxbutton_03_02, FALSE );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレームに「いれかえ」描画
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_PartyChangeButtonPut( BOX2_APP_WORK * appwk )
{
	FrmStrPut( appwk, BMPWIN_PARTY_CNG, mes_boxbutton_04_01, FALSE );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレームに「やめる」描画
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_PartySelectButtonPut( BOX2_APP_WORK * appwk )
{
	FrmStrPut( appwk, BMPWIN_PARTY_CNG, mes_boxbutton_04_02, FALSE );
}

//--------------------------------------------------------------------------------------------
/**
 * マーキングフレームににBMPWINをセット
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_MarkingButtonFrmPut( BOX2_APP_WORK * appwk )
{
	BGWINFRM_BmpWinOn( appwk->wfrmwk, BOX2MAIN_WINFRM_MARK, &appwk->win[BMPWIN_MARK_ENTER] );
	BGWINFRM_BmpWinOn( appwk->wfrmwk, BOX2MAIN_WINFRM_MARK, &appwk->win[BMPWIN_MARK_CANCEL] );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレームにBMPWINをセット
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_PartyCngButtonFrmPut( BOX2_APP_WORK * appwk )
{
	BGWINFRM_BmpWinOn( appwk->wfrmwk, BOX2MAIN_WINFRM_PARTY, &appwk->win[BMPWIN_PARTY_CNG] );
}

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタン作成
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	sw		選択フラグ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_ButtonPutYStatus( BOX2_SYS_WORK * syswk, BOOL sw )
{
	STRBUF * str;
	NNSG2dCharacterData * chr;
	GF_BGL_BMPWIN * win;
	void * buf;
	u8 * raw;
	u8	sx;
	u8	i;

	win = &syswk->app->win[ BOX2BMPWIN_ID_Y_STATUS ];
	sx  = GFL_BMPWIN_GetSizeX( win );
//↑[GS_CONVERT_TAG]

	buf = GFL_ARC_UTIL_LoadBGCharacter or GFL_ARC_UTIL_LoadOBJCharacter(
//↑[GS_CONVERT_TAG]
			ARC_BOX2_GRA, NARC_box_gra_box_m_bg1_lz_NCGR, TRUE, &chr, HEAPID_BOX_APP );
	raw = chr->pRawData;

	// 左３キャラ
	GF_BGL_BmpWinPrintEx( win, &raw[YST_UL_CGX_POS], 0, 0, 8*3, 8, 0, 0, 8*3, 8, 0xff );
	GF_BGL_BmpWinPrintEx( win, &raw[YST_CL_CGX_POS], 0, 0, 8*3, 8, 0, 8, 8*3, 8, 0xff );
	GF_BGL_BmpWinPrintEx( win, &raw[YST_DL_CGX_POS], 0, 0, 8*3, 8, 0, 16, 8*3, 8, 0xff );

	GF_BGL_BmpWinPrintEx( win, &raw[YST_UR_CGX_POS], 0, 0, 8, 8, (sx-1)*8,  0, 8, 8, 0xff );	// 右上
	GF_BGL_BmpWinPrintEx( win, &raw[YST_CR_CGX_POS], 0, 0, 8, 8, (sx-1)*8,  8, 8, 8, 0xff );	// 右中
	GF_BGL_BmpWinPrintEx( win, &raw[YST_DR_CGX_POS], 0, 0, 8, 8, (sx-1)*8, 16, 8, 8, 0xff );	// 右下

	for( i=3; i<sx-1; i++ ){
		GF_BGL_BmpWinPrintEx( win, &raw[YST_UC_CGX_POS], 0, 0, 8, 8, i*8,  0, 8, 8, 0xff );	// 上
		GF_BGL_BmpWinPrintEx( win, &raw[YST_CC_CGX_POS], 0, 0, 8, 8, i*8,  8, 8, 8, 0xff );	// 中
		GF_BGL_BmpWinPrintEx( win, &raw[YST_DC_CGX_POS], 0, 0, 8, 8, i*8, 16, 8, 8, 0xff );	// 下
	}

	GFL_HEAP_FreeMemory( buf );
//↑[GS_CONVERT_TAG]

	str = MSGMAN_AllocString( syswk->app->mman, mes_boxbutton_02_01 );
	StrPrintCore( win, str, sx*8/2, 4, FONT_TOUCH, FNTCOL_YSTWIN, STRPRINT_MODE_CENTER );
	STRBUF_Delete( str );

	GFL_BMPWIN_TransVramCharacter( win );
//↑[GS_CONVERT_TAG]
}


//============================================================================================
//	システムウィンドウ
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * メッセージ関連のメッセージマネージャー作成
 *
 * @param	none
 *
 * @return	メッセージマネージャー
 */
//--------------------------------------------------------------------------------------------
static GFL_MSGDATA * BoxMsgManGet(void)
//↑[GS_CONVERT_TAG]
{
	return MSGMAN_Create( GflMsgLoadType_DIRECT, ARC_MSG, NARC_msg_boxmes_dat, HEAPID_BOX_APP );
//↑[GS_CONVERT_TAG]
}

//--------------------------------------------------------------------------------------------
/**
 * システムウィンドウ表示
 *
 * @param	win		BMPWIN
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SysWinPut( GF_BGL_BMPWIN * win )
{
	GFL_BMP_Clear( win, 15 );
//↑[GS_CONVERT_TAG]
	BmpTalkWinScreenSet( win, BOX2MAIN_SYSWIN_CGX_POS, BOX2MAIN_BG_PAL_TALKWIN );
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
	BmpWinFrame_Clear( &appwk->win[winID], WINDOW_TRANS_OFF );
//↑[GS_CONVERT_TAG]
	GFL_BG_LoadScreenV_Req( appwk->bgl, GFL_BMPWIN_GetFrame(&appwk->win[winID]) );
//↑[GS_CONVERT_TAG]
//↑[GS_CONVERT_TAG]
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
void BOX2BMP_SysWinVanishEz( BOX2_APP_WORK * appwk, u32 winID )
{
	BmpWinFrame_Clear( &appwk->win[winID], WINDOW_TRANS_OFF );
//↑[GS_CONVERT_TAG]
}

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
	BmpWinFrame_Clear( &appwk->win[winID], WINDOW_TRANS_ON );
//↑[GS_CONVERT_TAG]
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
//↑[GS_CONVERT_TAG]

	SysWinPut( &syswk->app->win[winID] );
	ExStrPrint(
		syswk->app, man, winID, strID,
		0, 0, FONT_TALK, FNTCOL_W_BLACK, STRPRINT_MODE_LEFT );
	GFL_BMPWIN_MakeTransWindowVReq( &syswk->app->win[winID] );
//↑[GS_CONVERT_TAG]
	MSGMAN_Delete( man );
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
		WORDSET_RegisterPokeNickName(
			syswk->app->wset, 0, BOX2MAIN_PPPGet(syswk,syswk->tray,syswk->get_pos) );
		break;

	case BOX2BMP_MSGID_POKEFREE_BY:		// ばいばい　@0！
		str = msg_boxmes_01_05;
		WORDSET_RegisterPokeNickName(
			syswk->app->wset, 0, BOX2MAIN_PPPGet(syswk,syswk->tray,syswk->get_pos) );
		break;

	case BOX2BMP_MSGID_POKEFREE_EGG:	// タマゴを　にがすことは　できません！
		str = msg_boxmes_01_33;
		break;

	case BOX2BMP_MSGID_POKEFREE_RETURN:	// @0が　もどってきちゃった！
		str = msg_boxmes_01_34;
		WORDSET_RegisterPokeNickName(
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
	case BOX2BMP_MSGID_THEMA_INIT:		// ボックスを　どうする？
		str = msg_boxmes_01_08;
		break;

	case BOX2BMP_MSGID_THEMA_WALL:		// どの　かべがみに　する？
		str = msg_boxmes_01_11;
		break;
	}

	SysMesPut( syswk, str, winID );
}

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンをつれていく」メッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		ポケモンの位置
 * @param	msgID	メッセージＩＤ
 * @param	winID	BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BMP_PokePartyInMsgPut( BOX2_SYS_WORK * syswk, u32 pos, u32 msgID, u32 winID )
{
	u32	str;

	switch( msgID ){
	case BOX2BMP_MSGID_PARTYIN_INIT:	// ポケモンを　タッチ！
		str = msg_boxmes_01_19;
		break;

	case BOX2BMP_MSGID_PARTYIN_MENU:	// @01を　どうする？
		str = msg_boxmes_01_01;
		WORDSET_RegisterPokeNickName(
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
		
	case BOX2BMP_MSGID_PARTYOUT_CAPSULE:	// ボールカプセルを　はずしてください！
		str = msg_boxmes_01_31;
		break;

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
void BOX2BMP_VBlankMsgSet( BOX2_SYS_WORK * syswk, u32 winID, u32 msgID )
{
	GFL_MSGDATA * man;
//↑[GS_CONVERT_TAG]
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

	str = MSGMAN_AllocString( man, msgID );
	WORDSET_ExpandStr( syswk->app->wset, syswk->app->expbuf, str );
	STRBUF_Delete( str );

	GFL_BMP_Clear( &syswk->app->win[winID], 15 );
//↑[GS_CONVERT_TAG]

	GF_STR_PrintColor(
		&syswk->app->win[winID], FONT_TALK,
		syswk->app->expbuf, 0, 0, MSG_NO_PUT, FNTCOL_W_BLACK, NULL );

	MSGMAN_Delete( man );
}

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
void BOX2BMP_VBlankMsgPut( BOX2_SYS_WORK * syswk, u32 winID )
{
	BmpTalkWinScreenSet(
		&syswk->app->win[winID], BOX2MAIN_SYSWIN_CGX_POS, BOX2MAIN_BG_PAL_TALKWIN );
	GFL_BMPWIN_MakeTransWindowVReq( &syswk->app->win[winID] );
//↑[GS_CONVERT_TAG]
}
#endif

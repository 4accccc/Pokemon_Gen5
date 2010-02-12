//============================================================================================
/**
 * @file		zknsearch_ui.c
 * @brief		図鑑検索画面 ＵＩ関連
 * @author	Hiroyuki Nakamura
 * @date		10.02.09
 *
 *	モジュール名：ZKNSEARCHUI
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "ui/touchbar.h"

#include "zknsearch_main.h"
#include "zknsearch_ui.h"


//============================================================================================
//	定数定義
//============================================================================================


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void MainCallBack_On( void * work, int now_pos, int old_pos );
static void MainCallBack_Off( void * work, int now_pos, int old_pos );
static void MainCallBack_Move( void * work, int now_pos, int old_pos );
static void MainCallBack_Touch( void * work, int now_pos, int old_pos );


//============================================================================================
//	グローバル
//============================================================================================

// メインページのカーソル移動データ
static const CURSORMOVE_DATA MainCursorMoveData[] =
{
	{ 16,  0, 16, 3,	5, 1, 0, 0,	{   0,  23, 128, 255 } },		// 00: 並び
	{ 16,  3, 16, 3,	0, 2, 1, 1,	{  24,  47, 128, 255 } },		// 01: 名前
	{ 16,  6, 16, 3,	1, 3, 2, 2,	{  48,  71, 128, 255 } },		// 02: タイプ
	{ 16,  9, 16, 3,	2, 4, 3, 3,	{  72,  95, 128, 255 } },		// 03: 色
	{ 16, 12, 16, 5,	3, 5, 4, 4,	{  96, 135, 128, 255 } },		// 04: 形
	{ 16, 18, 16, 3,	4, 0, 6, 6,	{ 144, 167, 128, 255 } },		// 05: スタート
	{  0, 18, 16, 3,	4, 0, 5, 5,	{ 144, 167,   0, 127 } },		// 06: リセット

	{ 0, 0, 0, 0,	7, 7, 7, 7, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_05, TOUCHBAR_ICON_X_05+TOUCHBAR_ICON_WIDTH-1 } },	// 07: Ｙ
	{ 0, 0, 0, 0,	8, 8, 8, 8, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_X_06+TOUCHBAR_ICON_WIDTH-1 } },	// 08: Ｘ
	{ 0, 0, 0, 0,	9, 9, 9, 9, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },	// 09: 戻る

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};

static const CURSORMOVE_CALLBACK MainCallBack = {
	MainCallBack_On,
	MainCallBack_Off,
	MainCallBack_Move,
	MainCallBack_Touch,
};


void ZKNSEARCHUI_MainCursorMoveInit( ZKNSEARCHMAIN_WORK * wk, u32 pos )
{
	BOOL	flg;

	if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
		wk->cmwk = CURSORMOVE_Create(
								MainCursorMoveData, &MainCallBack, wk, FALSE, pos, HEAPID_ZUKAN_SEARCH );
	}else{
		wk->cmwk = CURSORMOVE_Create(
								MainCursorMoveData, &MainCallBack, wk, TRUE, pos, HEAPID_ZUKAN_SEARCH );
		ZKNSEARCHUI_ChangeCursorPalette( wk, pos, ZKNSEARCHMAIN_MBG_PAL_BUTTON_CUR2 );
	}
	CURSORMOVE_VanishModeSet( wk->cmwk );
}

void ZKNSEARCHUI_CursorMoveExit( ZKNSEARCHMAIN_WORK * wk )
{
	if( wk->cmwk != NULL ){
		CURSORMOVE_Exit( wk->cmwk );
		wk->cmwk = NULL;
	}
}


void ZKNSEARCHUI_ChangeCursorPalette( ZKNSEARCHMAIN_WORK * wk, u32 pos, u32 pal )
{
	if( pos != CURSORMOVE_ONOFF_DIRECT ){
		const CURSORMOVE_DATA * dat = CURSORMOVE_GetMoveData( wk->cmwk, pos );
		GFL_BG_ChangeScreenPalette( GFL_BG_FRAME1_M, dat->px, dat->py, dat->sx, dat->sy, pal );
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_M );
	}
}

static void MainCallBack_On( void * work, int now_pos, int old_pos )
{
	ZKNSEARCHMAIN_WORK * wk  = work;

	ZKNSEARCHUI_ChangeCursorPalette( wk, now_pos, ZKNSEARCHMAIN_MBG_PAL_BUTTON_CUR2 );
	BLINKPALANM_InitAnimeCount( wk->blink );
}

static void MainCallBack_Off( void * work, int now_pos, int old_pos )
{
	ZKNSEARCHMAIN_WORK * wk;

	wk  = work;

	ZKNSEARCHUI_ChangeCursorPalette( wk, old_pos, ZKNSEARCHMAIN_MBG_PAL_BUTTON_DEF );
	ZKNSEARCHUI_ChangeCursorPalette( wk, now_pos, ZKNSEARCHMAIN_MBG_PAL_BUTTON_DEF );
}

static void MainCallBack_Move( void * work, int now_pos, int old_pos )
{
	ZKNSEARCHMAIN_WORK * wk = work;

	ZKNSEARCHUI_ChangeCursorPalette( wk, old_pos, ZKNSEARCHMAIN_MBG_PAL_BUTTON_DEF );
	ZKNSEARCHUI_ChangeCursorPalette( wk, now_pos, ZKNSEARCHMAIN_MBG_PAL_BUTTON_CUR2 );
	BLINKPALANM_InitAnimeCount( wk->blink );
}

static void MainCallBack_Touch( void * work, int now_pos, int old_pos )
{
	ZKNSEARCHMAIN_WORK * wk = work;

	ZKNSEARCHUI_ChangeCursorPalette( wk, old_pos, ZKNSEARCHMAIN_MBG_PAL_BUTTON_DEF );
}

//============================================================================================
/**
 * @file		dpc_ui.c
 * @brief		殿堂入り確認画面 ＵＩ関連
 * @author	Hiroyuki Nakamura
 * @date		10.03.29
 *
 *	モジュール名：DPCUI
 */
//============================================================================================
#include <gflib.h>

#include "ui/touchbar.h"

#include "dpc_main.h"
#include "dpc_ui.h"


//============================================================================================
//	定数定義
//============================================================================================


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void CursorMoveCallBack_On( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Off( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Move( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Touch( void * work, int now_pos, int old_pos );


//============================================================================================
//	グローバル
//============================================================================================

static const CURSORMOVE_DATA CursorMoveData[] =
{
	{   0,  0, 0, 0,	 3, 0, 1, 2, { 80, 167,  80, 175 } },		// 00: ポケモン１
	{  88,  8, 0, 0,	 4, 1, 1, 0, { 80, 167,   0,  79 } },		// 01: ポケモン２
	{ -96,  8, 0, 0,	 5, 2, 0, 2, { 80, 167, 176, 255 } },		// 02: ポケモン３
	{   0, 88, 0, 0,	 3, 0, 4, 5, {  0,  79,  80, 175 } },		// 03: ポケモン４
	{  88, 80, 0, 0,	 4, 1, 4, 3, {  0,  79,   0,  79 } },		// 04: ポケモン５
	{ -96, 80, 0, 0,	 5, 2, 3, 5, {  0,  79, 176, 255 } },		// 05: ポケモン６

	{ 0, 0, 0, 0,	6, 6, 6, 6, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, 8, 31 } },			// 06: ページ左
	{ 0, 0, 0, 0,	7, 7, 7, 7, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, 88, 111 } },		// 07: ページ右

	{ 0, 0, 0, 0,	8, 8, 8, 8, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_X_06+TOUCHBAR_ICON_WIDTH-1 } },			// 08: 戻る１
	{ 0, 0, 0, 0,	9, 9, 9, 9, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },			// 09: 戻る２

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};

static const CURSORMOVE_CALLBACK CursorMoveCallBack = {
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	CursorMoveCallBack_Move,
	CursorMoveCallBack_Touch,
};





void DPCUI_Init( DPCMAIN_WORK * wk )
{
	wk->cmwk = CURSORMOVE_Create( CursorMoveData, &CursorMoveCallBack, wk, TRUE, 0, HEAPID_DENDOU_PC );
}

void DPCUI_Exit( DPCMAIN_WORK * wk )
{
	CURSORMOVE_Exit( wk->cmwk );
}

u32 DPCUI_Main( DPCMAIN_WORK * wk )
{
	u32	ret = CURSORMOVE_MainCont( wk->cmwk );

	if( ret != CURSORMOVE_NONE ){
		return ret;
	}

/*
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
		return DPCUI_ID_PAGE_LEFT;
	}
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
		return DPCUI_ID_PAGE_RIGHT;
	}
*/


	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
		return DPCUI_ID_EXIT;
	}

	return CURSORMOVE_NONE;
}

static void CursorMoveCallBack_On( void * work, int now_pos, int old_pos )
{
}

static void CursorMoveCallBack_Off( void * work, int now_pos, int old_pos )
{
}

static void CursorMoveCallBack_Move( void * work, int now_pos, int old_pos )
{
	DPCMAIN_WORK * wk;
	const CURSORMOVE_DATA * dat;

	if( now_pos < DPCUI_ID_POKE1 || now_pos > DPCUI_ID_POKE6 ){
		return;
	}

	wk  = work;
	dat = CURSORMOVE_GetMoveData( wk->cmwk, now_pos );

	GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_X_SET, (s8)dat->px );
	GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_Y_SET, (s8)dat->py );
}

static void CursorMoveCallBack_Touch( void * work, int now_pos, int old_pos )
{
	DPCMAIN_WORK * wk;
	const CURSORMOVE_DATA * dat;

	if( now_pos < DPCUI_ID_POKE1 || now_pos > DPCUI_ID_POKE6 ){
		return;
	}

	wk  = work;
	dat = CURSORMOVE_GetMoveData( wk->cmwk, now_pos );

	GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_X_SET, (s8)dat->px );
	GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_Y_SET, (s8)dat->py );
}

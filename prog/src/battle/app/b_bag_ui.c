//============================================================================================
/**
 * @file	b_bag_ui.c
 * @brief	戦闘用バッグ画面 インターフェース関連
 * @author	Hiroyuki Nakamura
 * @date	09.09.25
 */
//============================================================================================
#include <gflib.h>

/*
#include "arc_def.h"
#include "system/palanm.h"
*/

#include "b_app_tool.h"

#include "b_bag.h"
#include "b_bag_main.h"
#include "b_bag_ui.h"


//============================================================================================
//============================================================================================


//============================================================================================
//============================================================================================
static void VanishCursor( BBAG_WORK * wk, BOOL flg );
static void MoveCursor( BBAG_WORK * wk, int pos );

//static void DummyCallBack( void * work, int nowPos, int oldPos );
static void CallBack_On( void * work, int nowPos, int oldPos );
static void CallBack_Off( void * work, int nowPos, int oldPos );
static void CallBack_Touch( void * work, int nowPos, int oldPos );
static void CallBack_Move( void * work, int nowPos, int oldPos );
static void CallBack_OnItemList( void * work, int nowPos, int oldPos );


//============================================================================================
//============================================================================================

// ポケット選択画面移動テーブル
static const CURSORMOVE_DATA P1_CursorPosTbl[] =
{
	{  64,  42, 112+20,  44+16, 0, 1, 0, 2, {  1*8,  9*8-1,  0*8, 16*8-1 } },					// 0 : HP回復ポケット
	{  64, 114, 112+20,  44+16, 0, 4, 1, 3, { 10*8, 18*8-1,  0*8, 16*8-1 } },					// 1 : 状態回復ポケット
	{ 192,  42, 112+20,  44+16, 2, 3, 0, 2, {  1*8,  9*8-1, 16*8, 32*8-1 } },					// 2 : ボールポケット
	{ 192, 114, 112+20,  44+16, 2, 5, 1, 3, { 10*8, 18*8-1, 16*8, 32*8-1 } },					// 3 : 戦闘用ポケット
	{ 108, 176, 184+20,  24+16, 1, 4, 4, 5, { 19*8, 24*8-1,  1*8, 26*8-1 } },					// 4 : 最後に使用した道具
	{ 240, 174,  32+20,  28+16, CURSORMOVE_RETBIT|3, 5, 4, 5, { 19*8, 24*8-1, 27*8, 32*8-1 } },	// 5 : 戻る
};
static const CURSORMOVE_CALLBACK P1_CallBack = {
	CallBack_On,
	CallBack_Off,
	CallBack_Move,
	CallBack_Touch,
};

// アイテム選択画面移動テーブル
static const CURSORMOVE_DATA P2_CursorPosTbl[] =
{
	{  64,  32, 112+20,  38+16, 0, 2, 0, 1, {  1*8,  7*8-1,  0*8, 16*8-1 } },					// 0 : アイテム１
	{ 192,  32, 112+20,  38+16, 1, 3, 0, 1, {  1*8,  7*8-1, 16*8, 32*8-1 } },					// 1 : アイテム２
	{  64,  80, 112+20,  38+16, 0, 4, 2, 3, {  7*8, 13*8-1,  0*8, 16*8-1 } },					// 2 : アイテム３
	{ 192,  80, 112+20,  38+16, 1, 5, 2, 3, {  7*8, 13*8-1, 16*8, 32*8-1 } },					// 3 : アイテム４
	{  64, 128, 112+20,  38+16, 2, 6, 4, 5, { 13*8, 19*8-1,  0*8, 16*8-1 } },					// 4 : アイテム５
	{ 192, 128, 112+20,  38+16, 3, 6, 4, 5, { 13*8, 19*8-1, 16*8, 32*8-1 } },					// 5 : アイテム６
	{ 240, 174,  32+20,  28+16, CURSORMOVE_RETBIT|5, 6, 6, 6, { 19*8, 24*8-1, 27*8, 32*8-1 } },	// 6 : 戻る
	{ 0, 0, 0+20, 0+16, 7, 7, 7, 7, { 19*8, 24*8-1,  0*8,  5*8-1 } },	// 7 : 前へ
	{ 0, 0, 0+20, 0+16, 8, 8, 8, 8, { 19*8, 24*8-1,  5*8, 10*8-1 } },	// 8 : 次へ
};
static const CURSORMOVE_CALLBACK P2_CallBack = {
	CallBack_OnItemList,
	CallBack_Off,
	CallBack_Move,
	CallBack_Touch,
};

// アイテム選択画面移動テーブル
static const CURSORMOVE_DATA P3_CursorPosTbl[] =
{
	{ 108, 176, 184+20,  24+16, 0, 0, 0, 1, { 19*8, 24*8-1,  1*8, 26*8-1 } },	// 0 : 使う
	{ 240, 174,  32+20,  28+16, 1, 1, 0, 1, { 19*8, 24*8-1, 27*8, 32*8-1 } },	// 1 : 戻る
};
static const CURSORMOVE_CALLBACK P3_CallBack = {
	CallBack_On,
	CallBack_Off,
	CallBack_Move,
	CallBack_Touch,
};



static const CURSORMOVE_DATA * const PointTable[] = {
	P1_CursorPosTbl,
	P2_CursorPosTbl,
	P3_CursorPosTbl,
};

static const CURSORMOVE_CALLBACK * const CallBackTable[] = {
	&P1_CallBack,
	&P2_CallBack,
	&P3_CallBack,
};





void BBAGUI_Init( BBAG_WORK * wk, u32 page, u32 pos )
{
	wk->cmwk = CURSORMOVE_Create(
								PointTable[page],
								CallBackTable[page],
								wk,
								wk->cursor_flg,
								pos,
								wk->dat->heap );
	CURSORMOVE_VanishModeSet( wk->cmwk );
	MoveCursor( wk, pos );
}

void BBAGUI_Exit( BBAG_WORK * wk )
{
	CURSORMOVE_Exit( wk->cmwk );
}

void BBAGUI_ChangePage( BBAG_WORK * wk, u32 page, u32 pos )
{
	if( wk->cursor_flg == FALSE ){
		pos = 0;
	}else{
		switch( page ){
		case BBAG_PAGE_POCKET:		// ポケット選択ページ
			pos = wk->poke_id;
			break;
		case BBAG_PAGE_MAIN:			// アイテム選択ページ
			pos = wk->dat->item_pos[wk->poke_id];
			break;
		case BBAG_PAGE_ITEM:			// アイテム使用ページ
			pos = 0;
			break;
		}
	}

	BBAGUI_Exit( wk );
	BBAGUI_Init( wk, page, pos );
}


static void VanishCursor( BBAG_WORK * wk, BOOL flg )
{
	wk->cursor_flg = flg;
	BAPPTOOL_VanishCursor( wk->cpwk, flg );
}

static void MoveCursor( BBAG_WORK * wk, int pos )
{
	const CURSORMOVE_DATA *	dat = CURSORMOVE_GetMoveData( wk->cmwk, pos );
	BAPPTOOL_MoveCursorPoint( wk->cpwk, dat );
}


/*
static void DummyCallBack( void * work, int nowPos, int oldPos )
{
}
*/

static void CallBack_On( void * work, int nowPos, int oldPos )
{
	BBAG_WORK * wk = work;

	MoveCursor( wk, nowPos );
	VanishCursor( wk, TRUE );
}

static void CallBack_Off( void * work, int nowPos, int oldPos )
{
	BBAG_WORK * wk = work;

	VanishCursor( wk, FALSE );
}

static void CallBack_Touch( void * work, int nowPos, int oldPos )
{
	BBAG_WORK * wk = work;

	VanishCursor( wk, FALSE );
}

static void CallBack_Move( void * work, int nowPos, int oldPos )
{
	BBAG_WORK * wk = work;

	MoveCursor( wk, nowPos );
}

static void CallBack_OnItemList( void * work, int nowPos, int oldPos )
{
	BBAG_WORK * wk = work;

	if( nowPos == BBAG_UI_P2_LEFT || nowPos == BBAG_UI_P2_RIGHT ){
		nowPos = 	BBAG_UI_P2_ITEM1;
		CURSORMOVE_PosSet( wk->cmwk, nowPos );
	}

	MoveCursor( wk, nowPos );
	VanishCursor( wk, TRUE );
}

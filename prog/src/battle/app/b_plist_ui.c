//============================================================================================
/**
 * @file	b_plist_ui.c
 * @brief	戦闘用ポケモンリスト画面 インターフェース関連
 * @author	Hiroyuki Nakamura
 * @date	09.09.29
 */
//============================================================================================
#include <gflib.h>

#include "b_app_tool.h"

#include "b_plist.h"
#include "b_plist_main.h"
#include "b_plist_ui.h"


//============================================================================================
//============================================================================================


//============================================================================================
//============================================================================================
static void VanishCursor( BPLIST_WORK * wk, BOOL flg );
static void MoveCursor( BPLIST_WORK * wk, int pos );
static void CallBack_On( void * work, int nowPos, int oldPos );
static void CallBack_Off( void * work, int nowPos, int oldPos );
static void CallBack_Touch( void * work, int nowPos, int oldPos );
static void CallBack_Move( void * work, int nowPos, int oldPos );


//============================================================================================
//============================================================================================



// ポケモン選択画面カーソル移動データ
static const CURSORMOVE_DATA PokeSel_CursorMoveTbl[] =
{
	{  64,  24, 120+20,  40+16, 6, 2, 6, 1, {  0*8,  6*8-1,  0*8, 16*8-1 } },		// 0
	{ 192,  32, 120+20,  40+16, 4, 3, 0, 2, {  1*8,  7*8-1, 16*8, 32*8-1 } },		// 1
	{  64,  72, 120+20,  40+16, 0, 4, 1, 3, {  6*8, 12*8-1,  0*8, 16*8-1 } },		// 2
	{ 192,  80, 120+20,  40+16, 1, 5, 2, 4, {  7*8, 13*8-1, 16*8, 32*8-1 } },		// 3
	{  64, 120, 120+20,  40+16, 2, 1, 3, 5, { 12*8, 18*8-1,  0*8, 16*8-1 } },		// 4
	{ 192, 128, 120+20,  40+16, 3, 6, 4, 6, { 13*8, 19*8-1, 16*8, 32*8-1 } },		// 5
	{ 236, 172,  32+20,  32+16, 5, 0, 5, 0, { 19*8, 24*8-1, 27*8, 32*8-1 } },		// 6（戻る）
};
static const CURSORMOVE_CALLBACK P1_CallBack = {
	CallBack_On,
	CallBack_Off,
	CallBack_Move,
	CallBack_Touch,
};

// 入れ替え画面
static const CURSORMOVE_DATA Chg_CursorMoveTbl[] =
{
	{ 128,  76, 232+20, 128+16, 0, CURSORMOVE_RETBIT|1, 0, 0, {  1*8, 18*8-1,  1*8, 31*8-1 } },		// 0 : いれかえる
	{  52, 172,  96+20,  32+16, 0, 1, 1, 2, { 19*8, 24*8-1,  0*8, 13*8-1 } },						// 1 : 強さを見る
	{ 156, 172,  96+20,  32+16, 0, 2, 1, 3, { 19*8, 24*8-1, 13*8, 26*8-1 } },						// 2 : 技を見る
	{ 236, 172,  32+20,  32+16, 0, 3, 2, 3, { 19*8, 24*8-1, 27*8, 32*8-1 } },						// 3 : 戻る
};

// ステータスメインページのタッチパネル座標
static const CURSORMOVE_DATA StMain_CursorPosTbl[] =
{
	{  20, 172,  32+20,  32+16, 0, 0, 0, 1, { 19*8, 24*8-1,  0*8,  5*8-1 } },		// 0 : 上
	{  60, 172,  32+20,  32+16, 1, 1, 0, 2, { 19*8, 24*8-1,  5*8, 10*8-1 } },		// 1 : 下
	{ 148, 172,  96+20,  32+16, 2, 2, 1, 3, { 19*8, 24*8-1, 12*8, 25*8-1 } },		// 2 : 技を見る
	{ 236, 172,  32+20,  32+16, 3, 3, 2, 3, { 19*8, 24*8-1, 27*8, 32*8-1 } },		// 3 : 戻る
};

// ステータス技選択ページのタッチパネル座標
static const CURSORMOVE_DATA StWazaSel_CursorPosTbl[] =
{
	{  64,  72, 120+20,  40+16, 0, 2, 0, 1, {  6*8, 12*8-1,  0*8, 16*8-1 } },		// 0 : 技１
	{ 192,  72, 120+20,  40+16, 1, 3, 0, 1, {  6*8, 12*8-1, 16*8, 32*8-1 } },		// 1 : 技２
	{  64, 120, 120+20,  40+16, 0, CURSORMOVE_RETBIT|4, 2, 3, { 12*8, 18*8-1,  0*8, 16*8-1 } },		// 2 : 技３
	{ 192, 120, 120+20,  40+16, 1, CURSORMOVE_RETBIT|7, 2, 3, { 12*8, 18*8-1, 16*8, 32*8-1 } },		// 3 : 技４
	{  20, 172,  32+20,  32+16, 2, 4, 4, 5, { 19*8, 24*8-1,  0*8,  5*8-1 } },		// 4 : 上
	{  60, 172,  32+20,  32+16, 2, 5, 4, 6, { 19*8, 24*8-1,  5*8, 10*8-1 } },		// 5 : 下
	{ 148, 172,  96+20,  32+16, 3, 6, 5, 7, { 19*8, 24*8-1, 12*8, 25*8-1 } },		// 6 : 強さを見る
	{ 236, 172,  32+20,  32+16, 3, 7, 6, 7, { 19*8, 24*8-1, 27*8, 32*8-1 } },		// 7 : 戻る
};

// ステータス技詳細ページのタッチパネル座標
static const CURSORMOVE_DATA StWazaInfo_CursorPosTbl[] =
{
	{ 108, 160,  32+20,  16+16, 0, 2, 0, 1, { 19*8, 21*8-1, 11*8, 16*8-1 } },		// 0 : 技１
	{ 148, 160,  32+20,  16+16, 1, 3, 0, 4, { 19*8, 21*8-1, 16*8, 21*8-1 } },		// 1 : 技２
	{ 108, 176,  32+20,  16+16, 0, 2, 2, 3, { 21*8, 23*8-1, 11*8, 16*8-1 } },		// 2 : 技３
	{ 148, 176,  32+20,  16+16, 1, 3, 2, 4, { 21*8, 23*8-1, 16*8, 21*8-1 } },		// 3 : 技４
	{ 236, 172,  32+20,  32+16, 4, 4, CURSORMOVE_RETBIT|3, 4, { 19*8, 24*8-1, 27*8, 32*8-1 } },		// 4 : 戻る
};

// 技回復ページのタッチパネル座標
static const CURSORMOVE_DATA PPRcv_CursorPosTbl[] =
{
	{  64,  72, 120+20,  40+16, 0, 2, 0, 1, {  6*8, 12*8-1,  0*8, 16*8-1 } },						// 0 : 技１回復
	{ 192,  72, 120+20,  40+16, 1, 3, 0, 1, {  6*8, 12*8-1, 16*8, 32*8-1 } },						// 1 : 技２回復
	{  64, 120, 120+20,  40+16, 0, 4, 2, 3, { 12*8, 18*8-1,  0*8, 16*8-1 } },						// 2 : 技３回復
	{ 192, 120, 120+20,  40+16, 1, 4, 2, 3, { 12*8, 18*8-1, 16*8, 32*8-1 } },						// 3 : 技４回復
	{ 236, 172,  32+20,  32+16, CURSORMOVE_RETBIT|3, 4, 4, 4, { 19*8, 24*8-1, 27*8, 32*8-1 } },		// 4 : 戻る
};

// 技忘れ選択ページのタッチパネル座標
static const CURSORMOVE_DATA DelSel_CursorPosTbl[] =
{
	{  64,  72, 120+20,  40+16, 0, 2, 0, 1, {  6*8, 12*8-1,  0*8, 16*8-1 } },						// 0 : 技１画面へ
	{ 192,  72, 120+20,  40+16, 1, 3, 0, 1, {  6*8, 12*8-1, 16*8, 32*8-1 } },						// 1 : 技２画面へ
	{  64, 120, 120+20,  40+16, 0, 4, 2, 3, { 12*8, 18*8-1,  0*8, 16*8-1 } },						// 2 : 技３画面へ
	{ 192, 120, 120+20,  40+16, 1, 5, 2, 3, { 12*8, 18*8-1, 16*8, 32*8-1 } },						// 3 : 技４画面へ
	{ 128, 168, 120+20,  40+16, 2, 4, 4, 5, { 18*8, 24*8-1,  8*8, 24*8-1 } },						// 4 : 技５画面へ
	{ 236, 172,  32+20,  32+16, 3, 5, 4, 5, { 19*8, 24*8-1, 27*8, 32*8-1 } },						// 5 : 戻る
};

// 技忘れ決定ページのタッチパネル座標
static const CURSORMOVE_DATA DelInfo_CursorPosTbl[] =
{
	{ 104, 172, 200+20,  32+16, 0, 0, 0, 1, { 19*8, 24*8-1,  0*8, 26*8-1 } },						// 0 : わすれる
	{ 236, 172,  32+20,  32+16, 1, 1, 0, 1, { 19*8, 24*8-1, 27*8, 32*8-1 } },						// 1 : 戻る
};


static const CURSORMOVE_DATA * const PointTable[] = {
	PokeSel_CursorMoveTbl,		// ポケモン選択画面
	Chg_CursorMoveTbl,				// 入れ替え画面
	StMain_CursorPosTbl,			// ステータスメインページのタッチパネル座標
	StWazaSel_CursorPosTbl,		// ステータス技選択ページのタッチパネル座標
	StWazaInfo_CursorPosTbl,	// ステータス技詳細ページのタッチパネル座標
	PPRcv_CursorPosTbl,				// 技回復ページのタッチパネル座標
	DelSel_CursorPosTbl,			// 技忘れ選択ページのタッチパネル座標
	DelInfo_CursorPosTbl,			// 技忘れ決定ページのタッチパネル座標
};

static const CURSORMOVE_CALLBACK * const CallBackTable[] = {
	&P1_CallBack,
	&P1_CallBack,
	&P1_CallBack,
	&P1_CallBack,
	&P1_CallBack,
	&P1_CallBack,
	&P1_CallBack,
	&P1_CallBack,
};





void BPLISTUI_Init( BPLIST_WORK * wk, u32 page, u32 pos )
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

void BPLISTUI_Exit( BPLIST_WORK * wk )
{
	CURSORMOVE_Exit( wk->cmwk );
}

void BPLISTUI_ChangePage( BPLIST_WORK * wk, u32 page, u32 pos )
{
	BPLISTUI_Exit( wk );
	BPLISTUI_Init( wk, page, pos );
}




static void VanishCursor( BPLIST_WORK * wk, BOOL flg )
{
	wk->cursor_flg = flg;
	BAPPTOOL_VanishCursor( wk->cpwk, flg );
}

static void MoveCursor( BPLIST_WORK * wk, int pos )
{
	const CURSORMOVE_DATA *	dat = CURSORMOVE_GetMoveData( wk->cmwk, pos );
	BAPPTOOL_MoveCursorPoint( wk->cpwk, dat );
}



static void CallBack_On( void * work, int nowPos, int oldPos )
{
	BPLIST_WORK * wk = work;

	MoveCursor( wk, nowPos );
	VanishCursor( wk, TRUE );
}

static void CallBack_Off( void * work, int nowPos, int oldPos )
{
	BPLIST_WORK * wk = work;

	VanishCursor( wk, FALSE );
}

static void CallBack_Touch( void * work, int nowPos, int oldPos )
{
	BPLIST_WORK * wk = work;

	VanishCursor( wk, FALSE );
}

static void CallBack_Move( void * work, int nowPos, int oldPos )
{
	BPLIST_WORK * wk = work;

	MoveCursor( wk, nowPos );
}

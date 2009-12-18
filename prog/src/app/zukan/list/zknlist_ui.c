//============================================================================================
/**
 * @file		zknlist_ui.c
 * @brief		図鑑リスト画面 インターフェイス関連
 * @author	Hiroyuki Nakamura
 * @date		09.12.14
 *
 *	モジュール名：ZKNLISTUI
 */
//============================================================================================
#include <gflib.h>

#include "zknlist_main.h"
#include "zknlist_ui.h"


//============================================================================================
//	プロトタイプ宣言
//============================================================================================


//============================================================================================
//	グローバル
//============================================================================================

// タッチ座標テーブル
static const GFL_UI_TP_HITTBL TouchHitTbl[] =
{
	{   0, 167, 128, 247 },		// 00: リスト
	{   0, 167, 248, 255 },		// 01: レール

	{   0,  95,   0,  95 },		// 02: ポケモン正面絵

	{   0,  23,  96, 127 },		// 03: ポケモンアイコン
	{  24,  47,  96, 127 },		// 04: ポケモンアイコン
	{  48,  71,  96, 127 },		// 05: ポケモンアイコン
	{  72,  95,  96, 127 },		// 06: ポケモンアイコン
	{  96, 119,  96, 127 },		// 07: ポケモンアイコン
	{ 120, 143,  96, 127 },		// 08: ポケモンアイコン
	{ 144, 167,  96, 127 },		// 09: ポケモンアイコン

	{ 168, 191,   0,  71 },		// 10: スタート
	{ 168, 191,  72, 135 },		// 11: セレクト
	{ 168, 191, 136, 159 },		// 12: 左
	{ 168, 191, 160, 183 },		// 13: 右
	{ 168, 191, 184, 207 },		// 14: Ｙ
	{ 168, 191, 208, 231 },		// 15: Ｘ
	{ 168, 191, 232, 255 },		// 16: 戻る

	{ GFL_UI_TP_HIT_END, 0, 0, 0 }
};




int ZKNLISUI_ListMain( ZKNLISTMAIN_WORK * wk )
{
	int	ret;

	ret = GFL_UI_TP_HitTrg( TouchHitTbl );
	
	if( ret != GFL_UI_TP_HIT_NONE ){
		return ret;
	}

/*
	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP  ){
		return ZKNLISTUI_ID_LIST_UP;
	}

	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
		return ZKNLISTUI_ID_LIST_DOWN;
	}

	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
		return ZKNLISTUI_ID_LEFT;
	}

	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
		return ZKNLISTUI_ID_RIGHT;
	}
*/

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
		return ZKNLISTUI_ID_ICON1;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		return ZKNLISTUI_ID_RETURN;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
		return ZKNLISTUI_ID_X;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
		return ZKNLISTUI_ID_Y;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
		return ZKNLISTUI_ID_START;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		return ZKNLISTUI_ID_SELECT;
	}

	return ret;
}


static void CursorMoveCallBack( void * work, int now_pos, int old_pos )
{
}

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

#include "ui/touchbar.h"

#include "zknlist_main.h"
#include "zknlist_ui.h"


//============================================================================================
//	定数定義
//============================================================================================

// タッチバー上のボタン座標
#define	TB_SX			( 24 )
#define	TB_SY			( 24 )
#define	TB_PY			( TOUCHBAR_ICON_Y )
#define	TB_PX07		( TOUCHBAR_ICON_X_07 )
#define	TB_PX06		( TB_PX07 - TB_SX )
#define	TB_PX05		( TB_PX06 - TB_SX )


//============================================================================================
//	グローバル
//============================================================================================

// タッチ座標テーブル
static const GFL_UI_TP_HITTBL TouchHitTbl[] =
{
	{   0,  95,   0,  95 },		// 00: ポケモン正面絵

	{   0,  23,  96, 127 },		// 01: ポケモンアイコン
	{  24,  47,  96, 127 },		// 02: ポケモンアイコン
	{  48,  71,  96, 127 },		// 03: ポケモンアイコン
	{  72,  95,  96, 127 },		// 04: ポケモンアイコン
	{  96, 119,  96, 127 },		// 05: ポケモンアイコン
	{ 120, 143,  96, 127 },		// 06: ポケモンアイコン
	{ 144, 167,  96, 127 },		// 07: ポケモンアイコン

	{ 168, 191,   0,  71 },		// 08: スタート
	{ 168, 191,  72, 135 },		// 09: セレクト

	{ TB_PY, TB_PY+TB_SY-1, TB_PX05, TB_PX05+TB_SX-1 },		// 10: Ｙ
	{ TB_PY, TB_PY+TB_SY-1, TB_PX06, TB_PX06+TB_SX-1 },		// 11: Ｘ
	{ TB_PY, TB_PY+TB_SY-1, TB_PX07, TB_PX07+TB_SX-1 },		// 12: 戻る

	{ GFL_UI_TP_HIT_END, 0, 0, 0 }
};


//--------------------------------------------------------------------------------------------
/**
 * @brief		図鑑リスト画面入力処理
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	入力結果
 */
//--------------------------------------------------------------------------------------------
int ZKNLISTUI_ListMain( ZKNLISTMAIN_WORK * wk )
{
	int	ret;

	ret = GFL_UI_TP_HitTrg( TouchHitTbl );

	if( ret != GFL_UI_TP_HIT_NONE ){
		// 検索
		if( ret == ZKNLISTUI_ID_START ){
			// バージョンアップ前
			if( ZUKANSAVE_GetGraphicVersionUpFlag( wk->dat->savedata ) == FALSE ){
				return GFL_UI_TP_HIT_NONE;
			}
		}
		// モード切替
		if( ret == ZKNLISTUI_ID_SELECT ){
			// 全国図鑑未取得時
			if( ZUKANSAVE_GetZenkokuZukanFlag( wk->dat->savedata ) == FALSE ){
				return GFL_UI_TP_HIT_NONE;
			}
			// 検索リスト時
			if( wk->dat->callMode == ZKNLIST_CALL_SEARCH ){
				return GFL_UI_TP_HIT_NONE;
			}
		}
		// Ｙ登録
		if( ret == ZKNLISTUI_ID_Y ){
			// 検索リスト時
			if( wk->dat->callMode == ZKNLIST_CALL_SEARCH ){
				return GFL_UI_TP_HIT_NONE;
			}
			if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY ){
				GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
				FRAMELIST_SetCursorPosPaletteOff( wk->lwk );
			}
		}
		GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
		return ret;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
		return ZKNLISTUI_ID_RETURN;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
		return ZKNLISTUI_ID_X;
	}

	// 検索
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
		// バージョンアップ後
		if( ZUKANSAVE_GetGraphicVersionUpFlag( wk->dat->savedata ) == TRUE ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
			return ZKNLISTUI_ID_START;
		}
	}

	// モード切替
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		// 全国図鑑未取得時
		if( ZUKANSAVE_GetZenkokuZukanFlag( wk->dat->savedata ) == FALSE ){
			return GFL_UI_TP_HIT_NONE;
		}
		// 検索リスト時
		if( wk->dat->callMode == ZKNLIST_CALL_SEARCH ){
			return GFL_UI_TP_HIT_NONE;
		}
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
		return ZKNLISTUI_ID_SELECT;
	}

	// Ｙ登録
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
		// 検索リスト時
		if( wk->dat->callMode == ZKNLIST_CALL_SEARCH ){
			return GFL_UI_TP_HIT_NONE;
		}
		if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
			FRAMELIST_SetCursorPosPalette( wk->lwk );
		}else{
			GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
		}
		return ZKNLISTUI_ID_Y;
	}

	return ret;
}

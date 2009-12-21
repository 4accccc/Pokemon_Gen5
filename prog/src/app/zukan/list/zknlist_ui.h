//============================================================================================
/**
 * @file		zknlist_ui.h
 * @brief		図鑑リスト画面 インターフェイス関連
 * @author	Hiroyuki Nakamura
 * @date		09.12.14
 *
 *	モジュール名：ZKNLISTUI
 */
//============================================================================================
#pragma	once

enum {
	ZKNLISTUI_ID_LIST = 0,	// 00: リスト
	ZKNLISTUI_ID_RAIL,			// 01: レール

	ZKNLISTUI_ID_POKE,			// 02: ポケモン正面絵

	ZKNLISTUI_ID_ICON1,			// 03: ポケモンアイコン
	ZKNLISTUI_ID_ICON2,			// 04: ポケモンアイコン
	ZKNLISTUI_ID_ICON3,			// 05: ポケモンアイコン
	ZKNLISTUI_ID_ICON4,			// 06: ポケモンアイコン
	ZKNLISTUI_ID_ICON5,			// 07: ポケモンアイコン
	ZKNLISTUI_ID_ICON6,			// 08: ポケモンアイコン
	ZKNLISTUI_ID_ICON7,			// 09: ポケモンアイコン

	ZKNLISTUI_ID_START,			// 10: スタート
	ZKNLISTUI_ID_SELECT,		// 11: セレクト
	ZKNLISTUI_ID_LEFT,			// 12: 左
	ZKNLISTUI_ID_RIGHT,			// 13: 右
	ZKNLISTUI_ID_Y,					// 14: Ｙ
	ZKNLISTUI_ID_X,					// 15: Ｘ
	ZKNLISTUI_ID_RETURN,		// 16: 戻る

	ZKNLISTUI_ID_LIST_UP,		// 上キー
	ZKNLISTUI_ID_LIST_DOWN,	// 下キー
};



extern int ZKNLISUI_ListMain( ZKNLISTMAIN_WORK * wk );


extern BOOL ZKNLISTUI_CheckRailHit( u32 * x, u32 * y );

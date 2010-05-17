//============================================================================================
/**
 * @file		b_plist_ui.h
 * @brief		戦闘用ポケモンリスト画面 インターフェース関連
 * @author	Hiroyuki Nakamura
 * @date		09.09.29
 */
//============================================================================================
#pragma once


//============================================================================================
//	定数定義
//============================================================================================

// ボタンＩＤ
enum {
	BPLIST_UI_LIST_POKE1 = 0,
	BPLIST_UI_LIST_POKE2,
	BPLIST_UI_LIST_POKE3,
	BPLIST_UI_LIST_POKE4,
	BPLIST_UI_LIST_POKE5,
	BPLIST_UI_LIST_POKE6,
	BPLIST_UI_LIST_RETURN,

	BPLIST_UI_CHG_ENTER = 0,	// いれかえる
	BPLIST_UI_CHG_STATUS,			// つよさをみる
	BPLIST_UI_CHG_WAZA,				// わざをみる
	BPLIST_UI_CHG_RETURN,			// もどる

	BPLIST_UI_STATUS_UP = 0,	// 前のポケモンへ
	BPLIST_UI_STATUS_DOWN,		// 次のポケモンへ
	BPLIST_UI_STATUS_WAZA,		// わざをみる
	BPLIST_UI_STATUS_RETURN,	// もどる

	BPLIST_UI_WAZA_SEL1 = 0,	// 技１
	BPLIST_UI_WAZA_SEL2,			// 技２
	BPLIST_UI_WAZA_SEL3,			// 技３
	BPLIST_UI_WAZA_SEL4,			// 技４
	BPLIST_UI_WAZA_UP,				// 前のポケモンへ
	BPLIST_UI_WAZA_DOWN,			// 次のポケモンへ
	BPLIST_UI_WAZA_STATUS,		// つよさをみる
	BPLIST_UI_WAZA_RETURN,		// もどる

	BPLIST_UI_WAZAINFO_SEL1 = 0,	// 技１
	BPLIST_UI_WAZAINFO_SEL2,			// 技２
	BPLIST_UI_WAZAINFO_SEL3,			// 技３
	BPLIST_UI_WAZAINFO_SEL4,			// 技４
	BPLIST_UI_WAZAINFO_RETURN,		// もどる

	BPLIST_UI_PPRCV_WAZA1 = 0,	// 技１
	BPLIST_UI_PPRCV_WAZA2,			// 技２
	BPLIST_UI_PPRCV_WAZA3,			// 技３
	BPLIST_UI_PPRCV_WAZA4,			// 技４
	BPLIST_UI_PPRCV_RETURN,			// もどる

	BPLIST_UI_WAZADEL_SEL1 = 0,	// 技１
	BPLIST_UI_WAZADEL_SEL2,			// 技２
	BPLIST_UI_WAZADEL_SEL3,			// 技３
	BPLIST_UI_WAZADEL_SEL4,			// 技４
	BPLIST_UI_WAZADEL_SEL5,			// 技５
	BPLIST_UI_WAZADEL_RETURN,		// もどる

	BPLIST_UI_DELINFO_ENTER = 0,	// わすれる
	BPLIST_UI_DELINFO_RETURN,			// もどる
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動初期化
 *
 * @param		wk    ワーク
 * @param		page	ページ
 * @param		pos		初期位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BPLISTUI_Init( BPLIST_WORK * wk, u32 page, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動削除
 *
 * @param		wk    ワーク
 * @param		page	ページ
 * @param		pos		初期位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BPLISTUI_Exit( BPLIST_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動切り替え
 *
 * @param		wk    ワーク
 * @param		page	ページ
 * @param		pos		初期位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BPLISTUI_ChangePage( BPLIST_WORK * wk, u32 page, u32 pos );

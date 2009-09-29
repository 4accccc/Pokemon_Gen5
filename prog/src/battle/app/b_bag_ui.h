//============================================================================================
/**
 * @file		b_bag_ui.h
 * @brief		戦闘用バッグ画面 インターフェース関連
 * @author	Hiroyuki Nakamura
 * @date		09.09.25
 */
//============================================================================================
#pragma	once


enum {
	BBAG_UI_P1_HP_POCKET = 0,
	BBAG_UI_P1_STATUS_POCKET,
	BBAG_UI_P1_BALL_POCKET,
	BBAG_UI_P1_BATTLE_POCKET,
	BBAG_UI_P1_LAST_ITEM,
	BBAG_UI_P1_RETURN,

	BBAG_UI_P2_ITEM1 = 0,
	BBAG_UI_P2_ITEM2,
	BBAG_UI_P2_ITEM3,
	BBAG_UI_P2_ITEM4,
	BBAG_UI_P2_ITEM5,
	BBAG_UI_P2_ITEM6,
	BBAG_UI_P2_RETURN,
	BBAG_UI_P2_LEFT,
	BBAG_UI_P2_RIGHT,

	BBAG_UI_P3_USE = 0,
	BBAG_UI_P3_RETURN,
};

extern void BBAGUI_Init( BBAG_WORK * wk, u32 page, u32 pos );

extern void BBAGUI_Exit( BBAG_WORK * wk );

extern void BBAGUI_ChangePage( BBAG_WORK * wk, u32 page, u32 pos );

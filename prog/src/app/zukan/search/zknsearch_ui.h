//============================================================================================
/**
 * @file		zknsearch_ui.h
 * @brief		図鑑検索画面 ＵＩ関連
 * @author	Hiroyuki Nakamura
 * @date		10.02.09
 *
 *	モジュール名：ZKNSEARCHUI
 */
//============================================================================================
#pragma	once

enum {
	// トップ画面
	ZKNSEARCHUI_ROW = 0,
	ZKNSEARCHUI_NAME,
	ZKNSEARCHUI_TYPE,
	ZKNSEARCHUI_COLOR,
	ZKNSEARCHUI_FORM,
	ZKNSEARCHUI_START,
	ZKNSEARCHUI_RESET,

	ZKNSEARCHUI_Y,
	ZKNSEARCHUI_X,
	ZKNSEARCHUI_RET,

	// リスト画面
	ZKNSEARCH_LIST_RET = 0,

	ZKNSEARCH_LIST_ENTER1,
	ZKNSEARCH_LIST_ENTER2,
	ZKNSEARCH_LIST_ENTER3,
	ZKNSEARCH_LIST_ENTER4,
	ZKNSEARCH_LIST_ENTER5,
	ZKNSEARCH_LIST_ENTER6,
	ZKNSEARCH_LIST_ENTER7,

	ZKNSEARCH_LIST_CANCEL,
	ZKNSEARCH_LIST_RESET,
};

extern void ZKNSEARCHUI_MainCursorMoveInit( ZKNSEARCHMAIN_WORK * wk, u32 pos );

extern void ZKNSEARCHUI_CursorMoveExit( ZKNSEARCHMAIN_WORK * wk );

extern void ZKNSEARCHUI_ChangeCursorPalette( ZKNSEARCHMAIN_WORK * wk, u32 pos, u32 pal );

extern u32 ZKNSEARCHUI_MenuMain( ZKNSEARCHMAIN_WORK * wk );

extern BOOL ZKNSEARCHUI_Result( ZKNSEARCHMAIN_WORK * wk );

extern int ZKNSEARCHUI_ListMain( ZKNSEARCHMAIN_WORK * wk );
extern int ZKNSEARCHUI_FormListMain( ZKNSEARCHMAIN_WORK * wk );

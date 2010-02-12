//============================================================================================
/**
 * @file		zknsearch_ui.h
 * @brief		ê}ä”åüçıâÊñ  ÇtÇhä÷òA
 * @author	Hiroyuki Nakamura
 * @date		10.02.09
 *
 *	ÉÇÉWÉÖÅ[ÉãñºÅFZKNSEARCHUI
 */
//============================================================================================
#pragma	once

enum {
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
};

extern void ZKNSEARCHUI_MainCursorMoveInit( ZKNSEARCHMAIN_WORK * wk, u32 pos );

extern void ZKNSEARCHUI_CursorMoveExit( ZKNSEARCHMAIN_WORK * wk );

extern void ZKNSEARCHUI_ChangeCursorPalette( ZKNSEARCHMAIN_WORK * wk, u32 pos, u32 pal );

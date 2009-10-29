//============================================================================================
/**
 * @file	b_app_tool.h
 * @brief	戦闘用アプリ共通処理
 * @author	Hiroyuki Nakamura
 * @date	06.03.30
 */
//============================================================================================
#ifndef B_APP_TOOL_H
#define B_APP_TOOL_H

//#include "battle_cursor.h"
#include "print/printsys.h"
#include "system/cursor_move.h"


//============================================================================================
//	定数定義
//============================================================================================
//typedef struct _BAPP_CURSOR_MVWK	BAPP_CURSOR_MVWK;

typedef struct _BAPP_CURSOR_PUT_WORK	BAPP_CURSOR_PUT_WORK;

//#define	BAPP_CMV_CANCEL		( 0xfffffffe )	// キャンセル
//#define	BAPP_CMV_NONE		( 0xffffffff )	// 動作し

//#define	BAPP_CMV_RETBIT		( 0x80 )		// カーソル移動で前回の位置へ戻る

/*
typedef struct {
	GFL_BMPWIN * win;
	BOOL	flg;
}BAPP_BMPWIN_QUE;
*/
/*
typedef struct {
	u8	px;
	u8	py;
	u8	sx;
	u8	sy;
}BAPP_CURSOR_PUT;
*/

#define	BAPPTOOL_CURSOR_MAX		( 4 )		// カーソルで使用するセルアクターの数


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワーク作成
 *
 * @param	heap	ヒープID
 *
 * @return	カーソル移動ワーク
 */
//--------------------------------------------------------------------------------------------
//extern BAPP_CURSOR_MVWK * BAPP_CursorMoveWorkAlloc( u32 heap );

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワーク解放
 *
 * @param	heap	ヒープID
 *
 * @return	カーソル移動ワーク
 */
//--------------------------------------------------------------------------------------------
//extern void BAPP_CursorMoveWorkFree( BAPP_CURSOR_MVWK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワーク取得：カーソルデータ
 *
 * @param	wk		選択カーソル移動ワーク
 *
 * @return	カーソルデータ
 */
//--------------------------------------------------------------------------------------------
//extern BCURSOR_PTR BAPP_CursorMvWkGetBCURSOR_PTR( BAPP_CURSOR_MVWK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワーク取得：カーソル位置
 *
 * @param	wk		選択カーソル移動ワーク
 *
 * @return	カーソル位置
 */
//--------------------------------------------------------------------------------------------
//extern u8 BAPP_CursorMvWkGetPos( BAPP_CURSOR_MVWK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワーク取得：カーソルON/OFF
 *
 * @param	wk		選択カーソル移動ワーク
 *
 * @return	カーソルON/OFF
 */
//--------------------------------------------------------------------------------------------
//extern u8 BAPP_CursorMvWkGetFlag( BAPP_CURSOR_MVWK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワークセット：カーソルON/OFF
 *
 * @param	wk		選択カーソル移動ワーク
 * @param	flg		ON/OFFフラグ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BAPP_CursorMvWkSetFlag( BAPP_CURSOR_MVWK * wk, u8 flg );

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワークセット：カーソルデータ
 *
 * @param	wk		選択カーソル移動ワーク
 * @param	flg		カーソルデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BAPP_CursorMvWkSetBCURSOR_PTR( BAPP_CURSOR_MVWK * wk, BCURSOR_PTR cur );

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワークセット：カーソル位置
 *
 * @param	wk		選択カーソル移動ワーク
 * @param	pos		カーソル位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BAPP_CursorMvWkSetPos( BAPP_CURSOR_MVWK * wk, u8 pos );

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル位置初期化
 *
 * @param	wk		選択カーソル移動ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BAPP_CursorMvWkPosInit( BAPP_CURSOR_MVWK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワークセット：カーソル位置ワーク
 *
 * @param	wk		選択カーソル移動ワーク
 * @param	pwk		カーソル位置ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BAPP_CursorMvWkSetPoint( BAPP_CURSOR_MVWK * wk, const POINTSEL_WORK * pwk );

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワークセット：移動テーブル
 *
 * @param	wk		選択カーソル移動ワーク
 * @param	mv_tbl	移動テーブル
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BAPP_CursorMvWkSetMvTbl( BAPP_CURSOR_MVWK * wk, u32 mv_tbl );

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動
 *
 * @param	wk		選択カーソル移動ワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
//extern u32 BAPP_CursorMove( BAPP_CURSOR_MVWK * wk );



extern BAPP_CURSOR_PUT_WORK * BAPPTOOL_CreateCursor( HEAPID heapID );
extern void BAPPTOOL_FreeCursor( BAPP_CURSOR_PUT_WORK * wk );
//extern void BAPPTOOL_SetCursorPutData( BAPP_CURSOR_PUT_WORK * wk, const BAPP_CURSOR_PUT * put );
extern void BAPPTOOL_AddCursor( BAPP_CURSOR_PUT_WORK * wk, GFL_CLUNIT * clunit, u32 chrRes, u32 palRes, u32 celRes );
extern void BAPPTOOL_DelCursor( BAPP_CURSOR_PUT_WORK * wk );
extern void BAPPTOOL_VanishCursor( BAPP_CURSOR_PUT_WORK * wk, BOOL flg );
//extern void BAPPTOOL_MoveCursor( BAPP_CURSOR_PUT_WORK * wk, u32 point );
extern void BAPPTOOL_MoveCursorPoint( BAPP_CURSOR_PUT_WORK * wk, const CURSORMOVE_DATA * dat );



//extern void BAPPTOOL_PrintQueOn( BAPP_BMPWIN_QUE * dat );
//extern void BAPPTOOL_PrintQueTrans( PRINT_UTIL * util, PRINT_QUE * que, u32 max );
extern void BAPPTOOL_PrintScreenTrans( PRINT_UTIL * util );
extern void BAPPTOOL_PrintUtilTrans( PRINT_UTIL * util, PRINT_QUE * que, u32 max );

#define	BAPPTOOL_SET_STR_SCRN_END		( 0xff )		// 転送ウィンドウ終了

extern void BAPPTOOL_SetStrScrn( PRINT_UTIL * util, const u8 * tbl );


#endif	// B_APP_TOOL_H

//======================================================================
/**
 * @file	bmp_cursor.h
 * @brief	BitmapWindowにメニュー選択用のカーソルを描画する仕組み
 * @author	taya
 * @date	2006.01.12
 */
//======================================================================
#ifndef __BMP_CURSOR_H__
#define __BMP_CURSOR_H__

//#include "gflib\msg_print.h"

#include <tcbl.h>
#include "print\printsys.h"
#include "print\gf_font.h"

//======================================================================
//
//======================================================================
typedef struct _BMPCURSOR	BMPCURSOR;

//======================================================================
//
//======================================================================
//------------------------------------------------------------------
/**
 * Bitmapカーソルを作成
 * @param   heapID		作成先ヒープＩＤ
 * @retval  BMPCURSOR*		作成されたカーソルオブジェクトへのポインタ
 */
//------------------------------------------------------------------
extern BMPCURSOR * BmpCursor_Create( u32 heapID );

//------------------------------------------------------------------
/**
 * Bitmapカーソルを破棄
 * @param   bmpCursor		Bitmapカーソルオブジェクトへのポインタ
 */
//------------------------------------------------------------------
extern void BmpCursor_Delete( BMPCURSOR* bmpCursor );

//------------------------------------------------------------------
/**
 * Bitmapカーソルのカラー変更
 * @param   bmpCursor		Bitmapカーソルオブジェクトへのポインタ
 * @param   color			カラー値（GF_PRINTCOLOR_MAKEマクロを使用）
 */
//------------------------------------------------------------------
#if 0
extern void BmpCursor_ChangeColor(
	BMPCURSOR* bmpCursor, GF_PRINTCOLOR color );
#endif

//------------------------------------------------------------------
/**
 * BitmapカーソルをBitmapWindowに描画する
 * @param   bmpCursor	Bitmapカーソルオブジェクトへのポインタ
 * @param   win			BitmapWindow
 * @param   x			描画Ｘ座標（ドット単位）
 * @param   y			描画Ｙ座標（ドット単位）
 */
//------------------------------------------------------------------
#if 0
extern void BmpCursor_Print(
	const BMPCURSOR *bmpCursor, GFL_BMPWIN *win, u32 x, u32 y );
#endif
extern void BmpCursor_Print( const BMPCURSOR *bmpCursor, u32 x, u32 y,
	PRINT_UTIL *printUtil, PRINT_QUE *printQue, GFL_FONT *fontHandle );


extern void BmpCursor_SetCursorFont(
	BMPCURSOR *bmpCursor, const STRBUF *str, u32 heap_id );
extern void BmpCursor_SetCursorFontMsg(
	BMPCURSOR *bmpCursor, GFL_MSGDATA *msgdata, u32 strID );

#endif	//__BMP_CURSOR_H__

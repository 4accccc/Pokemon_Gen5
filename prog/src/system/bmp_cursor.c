//======================================================================
/*
 * @file	bmp_cursor.c
 * @brief	BitmapWindowにメニュー選択用のカーソルを描画する
 * @author	taya
 * @date	2006.01.12
 */
//======================================================================
#include <gflib.h>
#include <textprint.h>

#include "include/system/gfl_use.h"
#include "arc/arc_def.h"
#include "system/bmp_cursor.h"

//======================================================================
//	
//======================================================================
enum {
	CURSOR_BMP_SIZE = 0x20 * 4,

	DEFAULT_LETTER_COLOR = 0x01,
	DEFAULT_SHADOW_COLOR = 0x02,
	DEFAULT_GROUND_COLOR = 0x0F,
};

#if 0
struct _BMPCURSOR {
	GF_PRINTCOLOR  color;
	STRBUF*        strbuf;
};
#else
struct _BMPCURSOR {
	int font_init;
	STRBUF*        strbuf;
};
#endif

//======================================================================
//
//======================================================================
BMPCURSOR * BmpCursor_Create( u32 heapID )
{
#if 0	//dp
	static const STRCODE str[] = {
		cursor_, EOM_, 
	};
	
	BMPCURSOR*  bmpCursor = sys_AllocMemory( heapID, sizeof(BMPCURSOR) );
	if( bmpCursor )
	{
		bmpCursor->color = GF_PRINTCOLOR_MAKE(DEFAULT_LETTER_COLOR, DEFAULT_SHADOW_COLOR, DEFAULT_GROUND_COLOR);
		bmpCursor->strbuf = STRBUF_Create( 4, heapID );
		STRBUF_SetStringCode( bmpCursor->strbuf, str );
	}
	return bmpCursor;
#else
	BMPCURSOR *bmpCursor = GFL_HEAP_AllocMemory( heapID, sizeof(BMPCURSOR) );
	
	if( bmpCursor ){
//		static STRCODE code[] = {'A'};
		bmpCursor->font_init = FALSE;
//		bmpCursor->strbuf = GFL_STR_CreateBuffer( 8, heapID );
//		GFL_STR_SetStringCode( bmpCursor->strbuf, code );	
//		GFL_STR_SetStringCodeOrderLength( bmpCursor->strbuf, code, 1 );	
	}
	
	return bmpCursor;
#endif
}

void BmpCursor_Delete( BMPCURSOR* bmpCursor )
{
#if 0
	GF_ASSERT(bmpCursor!=NULL);
	if(bmpCursor)
	{
		if( bmpCursor->strbuf )
		{
			STRBUF_Delete( bmpCursor->strbuf );
		}
		sys_FreeMemoryEz( bmpCursor );
	}
#else
	GF_ASSERT(bmpCursor!=NULL);
	GFL_STR_DeleteBuffer( bmpCursor->strbuf );
	GFL_HEAP_FreeMemory( bmpCursor );
#endif
}

#if 0	//dp
void BmpCursor_ChangeColor( BMPCURSOR* bmpCursor, GF_PRINTCOLOR color )
{
	GF_ASSERT(bmpCursor!=NULL);
	if(bmpCursor)
	{
		bmpCursor->color = color;
	}
}
#endif

#if 0	//dp
void BmpCursor_Print( const BMPCURSOR *bmpCursor, GFL_BMPWIN *win, u32 x, u32 y )
{
	GF_STR_PrintColor( win, FONT_SYSTEM, bmpCursor->strbuf, x, y, MSG_NO_PUT, bmpCursor->color, NULL );
	GF_BGL_BmpWinCgxOn( win );
}
#endif

void BmpCursor_Print( const BMPCURSOR *bmpCursor, u32 x, u32 y,
		PRINT_UTIL *printUtil, PRINT_QUE *printQue, GFL_FONT *fontHandle )
{
	if( bmpCursor->font_init == TRUE ){
		PRINT_UTIL_Print( printUtil, printQue, x, y,
			bmpCursor->strbuf, fontHandle );
	}
}

void BmpCursor_SetCursorFont(
	BMPCURSOR *bmpCursor, const STRBUF *str, u32 heap_id )
{
	u32 len = GFL_STR_GetBufferLength( str );
	GF_ASSERT( len != 0 );
	GF_ASSERT( bmpCursor->font_init == FALSE );
	bmpCursor->strbuf = GFL_STR_CreateBuffer( len, heap_id );
	GFL_STR_CopyBuffer( bmpCursor->strbuf, str );
	bmpCursor->font_init = TRUE;
}

void BmpCursor_SetCursorFontMsg(
	BMPCURSOR *bmpCursor, GFL_MSGDATA *msgdata, u32 strID )
{
	GF_ASSERT( bmpCursor->font_init == FALSE );
	bmpCursor->strbuf = GFL_MSG_CreateString( msgdata, strID );
	bmpCursor->font_init = TRUE;
}

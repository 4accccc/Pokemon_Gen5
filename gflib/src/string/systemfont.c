//=============================================================================================
/**
 * @file	systemfont.c
 * @brief	ビットマップ文字取得システム
 * 
 */
//=============================================================================================
#include "gflib.h"

#include "systemfont.h"
//------------------------------------------------------------------
/**
 * フォントデータヘッダ構造体（フォントアーカイブの先頭に配置されている）
 */
//------------------------------------------------------------------
typedef struct {
	u32	bitDataOffs;	// フォントデータテーブルへのバイト単位オフセット
	u32	widthTblOffs;	// 文字幅テーブルへのバイト単位オフセット
	u32	letterMax;		// 登録文字数
	u8	maxWidth;		// 最大文字幅（ドット）
	u8	maxHeight;		// 最大文字高さ（ドット）
	u8	letterCharX;	// １文字の横キャラ数
	u8	letterCharY;	// １文字の縦キャラ数
}FONT_HEADER;

//------------------------------------------------------------------
/**
 * ワーク構造体
 */
//------------------------------------------------------------------
typedef struct {
	FONT_HEADER*	header;
	u32				fontCol;
	u32				baseCol;
}FONT_SYSWORK;

FONT_SYSWORK* fsw = NULL;

//------------------------------------------------------------------
static const u32 mask_table[256] = {
	0x00000000, 0xF0000000, 0x0F000000, 0xFF000000, 0x00F00000, 0xF0F00000, 0x0FF00000, 0xFFF00000, 
	0x000F0000, 0xF00F0000, 0x0F0F0000, 0xFF0F0000, 0x00FF0000, 0xF0FF0000, 0x0FFF0000, 0xFFFF0000, 
	0x0000F000, 0xF000F000, 0x0F00F000, 0xFF00F000, 0x00F0F000, 0xF0F0F000, 0x0FF0F000, 0xFFF0F000, 
	0x000FF000, 0xF00FF000, 0x0F0FF000, 0xFF0FF000, 0x00FFF000, 0xF0FFF000, 0x0FFFF000, 0xFFFFF000, 
	0x00000F00, 0xF0000F00, 0x0F000F00, 0xFF000F00, 0x00F00F00, 0xF0F00F00, 0x0FF00F00, 0xFFF00F00, 
	0x000F0F00, 0xF00F0F00, 0x0F0F0F00, 0xFF0F0F00, 0x00FF0F00, 0xF0FF0F00, 0x0FFF0F00, 0xFFFF0F00, 
	0x0000FF00, 0xF000FF00, 0x0F00FF00, 0xFF00FF00, 0x00F0FF00, 0xF0F0FF00, 0x0FF0FF00, 0xFFF0FF00, 
	0x000FFF00, 0xF00FFF00, 0x0F0FFF00, 0xFF0FFF00, 0x00FFFF00, 0xF0FFFF00, 0x0FFFFF00, 0xFFFFFF00, 
	0x000000F0, 0xF00000F0, 0x0F0000F0, 0xFF0000F0, 0x00F000F0, 0xF0F000F0, 0x0FF000F0, 0xFFF000F0, 
	0x000F00F0, 0xF00F00F0, 0x0F0F00F0, 0xFF0F00F0, 0x00FF00F0, 0xF0FF00F0, 0x0FFF00F0, 0xFFFF00F0, 
	0x0000F0F0, 0xF000F0F0, 0x0F00F0F0, 0xFF00F0F0, 0x00F0F0F0, 0xF0F0F0F0, 0x0FF0F0F0, 0xFFF0F0F0, 
	0x000FF0F0, 0xF00FF0F0, 0x0F0FF0F0, 0xFF0FF0F0, 0x00FFF0F0, 0xF0FFF0F0, 0x0FFFF0F0, 0xFFFFF0F0, 
	0x00000FF0, 0xF0000FF0, 0x0F000FF0, 0xFF000FF0, 0x00F00FF0, 0xF0F00FF0, 0x0FF00FF0, 0xFFF00FF0, 
	0x000F0FF0, 0xF00F0FF0, 0x0F0F0FF0, 0xFF0F0FF0, 0x00FF0FF0, 0xF0FF0FF0, 0x0FFF0FF0, 0xFFFF0FF0, 
	0x0000FFF0, 0xF000FFF0, 0x0F00FFF0, 0xFF00FFF0, 0x00F0FFF0, 0xF0F0FFF0, 0x0FF0FFF0, 0xFFF0FFF0, 
	0x000FFFF0, 0xF00FFFF0, 0x0F0FFFF0, 0xFF0FFFF0, 0x00FFFFF0, 0xF0FFFFF0, 0x0FFFFFF0, 0xFFFFFFF0, 
	0x0000000F, 0xF000000F, 0x0F00000F, 0xFF00000F, 0x00F0000F, 0xF0F0000F, 0x0FF0000F, 0xFFF0000F, 
	0x000F000F, 0xF00F000F, 0x0F0F000F, 0xFF0F000F, 0x00FF000F, 0xF0FF000F, 0x0FFF000F, 0xFFFF000F, 
	0x0000F00F, 0xF000F00F, 0x0F00F00F, 0xFF00F00F, 0x00F0F00F, 0xF0F0F00F, 0x0FF0F00F, 0xFFF0F00F, 
	0x000FF00F, 0xF00FF00F, 0x0F0FF00F, 0xFF0FF00F, 0x00FFF00F, 0xF0FFF00F, 0x0FFFF00F, 0xFFFFF00F, 
	0x00000F0F, 0xF0000F0F, 0x0F000F0F, 0xFF000F0F, 0x00F00F0F, 0xF0F00F0F, 0x0FF00F0F, 0xFFF00F0F, 
	0x000F0F0F, 0xF00F0F0F, 0x0F0F0F0F, 0xFF0F0F0F, 0x00FF0F0F, 0xF0FF0F0F, 0x0FFF0F0F, 0xFFFF0F0F, 
	0x0000FF0F, 0xF000FF0F, 0x0F00FF0F, 0xFF00FF0F, 0x00F0FF0F, 0xF0F0FF0F, 0x0FF0FF0F, 0xFFF0FF0F, 
	0x000FFF0F, 0xF00FFF0F, 0x0F0FFF0F, 0xFF0FFF0F, 0x00FFFF0F, 0xF0FFFF0F, 0x0FFFFF0F, 0xFFFFFF0F, 
	0x000000FF, 0xF00000FF, 0x0F0000FF, 0xFF0000FF, 0x00F000FF, 0xF0F000FF, 0x0FF000FF, 0xFFF000FF, 
	0x000F00FF, 0xF00F00FF, 0x0F0F00FF, 0xFF0F00FF, 0x00FF00FF, 0xF0FF00FF, 0x0FFF00FF, 0xFFFF00FF, 
	0x0000F0FF, 0xF000F0FF, 0x0F00F0FF, 0xFF00F0FF, 0x00F0F0FF, 0xF0F0F0FF, 0x0FF0F0FF, 0xFFF0F0FF, 
	0x000FF0FF, 0xF00FF0FF, 0x0F0FF0FF, 0xFF0FF0FF, 0x00FFF0FF, 0xF0FFF0FF, 0x0FFFF0FF, 0xFFFFF0FF, 
	0x00000FFF, 0xF0000FFF, 0x0F000FFF, 0xFF000FFF, 0x00F00FFF, 0xF0F00FFF, 0x0FF00FFF, 0xFFF00FFF, 
	0x000F0FFF, 0xF00F0FFF, 0x0F0F0FFF, 0xFF0F0FFF, 0x00FF0FFF, 0xF0FF0FFF, 0x0FFF0FFF, 0xFFFF0FFF, 
	0x0000FFFF, 0xF000FFFF, 0x0F00FFFF, 0xFF00FFFF, 0x00F0FFFF, 0xF0F0FFFF, 0x0FF0FFFF, 0xFFF0FFFF, 
	0x000FFFFF, 0xF00FFFFF, 0x0F0FFFFF, 0xFF0FFFFF, 0x00FFFFFF, 0xF0FFFFFF, 0x0FFFFFFF, 0xFFFFFFFF, 
};


//------------------------------------------------------------------
/**
 *
 * 初期化
 *		フォントのデータ展開およびワーク確保
 *
 * @param	fontDataPath	フォントデータのパス
 */
//------------------------------------------------------------------
void
	GFL_FONT_sysInit
		( const char* fontDataPath )
{
	FSFile	file;

	fsw = GFL_HEAP_AllocMemory( GFL_HEAPID_SYSTEM, sizeof(FONT_SYSWORK) );

	//フォントデータのロード
	FS_InitFile( &file );
	
	if( FS_OpenFile( &file, fontDataPath ) == TRUE ){
		u32 size = FS_GetLength( &file );
		fsw->header = GFL_HEAP_AllocMemory( GFL_HEAPID_SYSTEM, size );
		FS_ReadFile( &file, fsw->header, size );
		FS_CloseFile( &file );
	//	OS_Printf("システムフォントをロードしました\n");
	} else {
		OS_Panic("Panic: Can't open file %s\n", fontDataPath);
	}
}


//------------------------------------------------------------------
/**
 *
 * 終了
 *		フォントおよびワーク破棄
 *
 */
//------------------------------------------------------------------
void
	GFL_FONT_sysExit
		( void )
{
	GFL_HEAP_FreeMemory( fsw->header );
	GFL_HEAP_FreeMemory( fsw );
}


//------------------------------------------------------------------
/**
 *
 * フォントデータ取得
 *
 * @param   fcode	フォントコード
 * @param   dst		取得用データポインタ
 */
//------------------------------------------------------------------
void 
	GFL_FONT_GetData
		( STRCODE fcode, GFL_FONT_DATA* dst )
{
	u8*		src = (u8*)((u32)fsw->header + fsw->header->bitDataOffs + 
						(fcode * SYSTEMFONT_DATA_SIZE/4) );
	u32*	buf = (u32*)dst->data;
	u32		fcol = fsw->fontCol;
	u32		bcol = fsw->baseCol;
	u32		mask;

	mask = mask_table[ *src++ ];
	*buf++ = (fcol & mask)|(bcol & (mask^0xffffffff));
	mask = mask_table[ *src++ ];
	*buf++ = (fcol & mask)|(bcol & (mask^0xffffffff));
	mask = mask_table[ *src++ ];
	*buf++ = (fcol & mask)|(bcol & (mask^0xffffffff));
	mask = mask_table[ *src++ ];
	*buf++ = (fcol & mask)|(bcol & (mask^0xffffffff));
	mask = mask_table[ *src++ ];
	*buf++ = (fcol & mask)|(bcol & (mask^0xffffffff));
	mask = mask_table[ *src++ ];
	*buf++ = (fcol & mask)|(bcol & (mask^0xffffffff));
	mask = mask_table[ *src++ ];
	*buf++ = (fcol & mask)|(bcol & (mask^0xffffffff));
	mask = mask_table[ *src++ ];
	*buf++ = (fcol & mask)|(bcol & (mask^0xffffffff));

	dst->sizex = *(u8*)((u32)fsw->header + fsw->header->widthTblOffs + fcode );
}


//------------------------------------------------------------------
/**
 *
 * フォント最大横幅の取得
 *
 */
//------------------------------------------------------------------
u8
	GFL_FONT_GetSizeMaxX
		( void )
{
	return fsw->header->maxWidth;
}


//------------------------------------------------------------------
/**
 *
 * フォント最大横幅の取得
 *
 */
//------------------------------------------------------------------
u8
	GFL_FONT_GetSizeMaxY
		( void )
{
	return fsw->header->maxHeight;
}


//------------------------------------------------------------------
/**
 *
 * フォントスペース幅の取得
 *
 */
//------------------------------------------------------------------
u8
	GFL_FONT_GetSizeSPC
		( void )
{
	return 7;
}


//------------------------------------------------------------------
/**
 *
 * フォントカラーナンバーの設定
 *
 * @param   fontCol	文字色
 * @param   baseCol	背景色
 */
//------------------------------------------------------------------
void
	GFL_FONT_SetColor
		( int fontCol, int baseCol )
{
	fsw->fontCol =	(fontCol<<28)|(fontCol<<24)|(fontCol<<20)|(fontCol<<16)|
					(fontCol<<12)|(fontCol<<8 )|(fontCol<<4 )|(fontCol<<0 );
	fsw->baseCol =	(baseCol<<28)|(baseCol<<24)|(baseCol<<20)|(baseCol<<16)|
					(baseCol<<12)|(baseCol<<8 )|(baseCol<<4 )|(baseCol<<0 );
}






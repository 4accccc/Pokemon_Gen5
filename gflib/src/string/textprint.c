//=============================================================================================
/**
 * @file	msgprint.c
 * @brief	ビットマップ文字列表示用システム
 * 
 */
//=============================================================================================
#include "gflib.h"

//#include "msg_print.h"

//==================================================================
//	変数 ＆ 関数宣言
//==================================================================
#define SYSTEMFONT_SIZEX		(8)
#define SYSTEMFONT_SIZEY		(8)
#define SYSTEMFONT_DATA_SIZE	(0x20)

enum{
	GFL_TEXT_WRITE_16 = 0,
	GFL_TEXT_WRITE_256,
};

typedef struct {
	STRBUF*		string;
	void*		bmp;
	u16			writex;
	u16			writey;
	u8			spacex;
	u8			spacey;
	u8			colorF:4;
	u8			colorB:4;
	u8			mode;
}TEXT_PARAM;

typedef struct {
	u8			fdata[SYSTEMFONT_DATA_SIZE];
	u8			fdatasizex;
	u8			fdatasizey;
	u16			nowx;
	u16			nowy;
}TEXT_WORK;

void		GFL_TEXT_Print ( TEXT_PARAM* text );
static void FontDataGet ( STRCODE fcode, u32* dst, u8* sizex, u32 colorFtmp , u32 colorBtmp );
static const u32 mask_table[256];

static const u8 GFL_TEXT_DataTable[] = {0};
static const u8 GFL_TEXT_SizeTable[] = {0};
//------------------------------------------------------------------
/**
 * ビットマップの内部CGX領域に文字を描画する
 *
 * @param   
 * @param   
 * @param   
 *
 * @retval  
 */
//------------------------------------------------------------------
void
	GFL_TEXT_Print
		( TEXT_PARAM* text )
{
	const STRCODE* strdat	= GFL_STR_GetStringCodePointer( text->string );
	TEXT_WORK* tw	= GFL_HEAP_AllocMemory( GFL_HEAPID_SYSTEM, sizeof(TEXT_WORK) );
	u32 colorFtmp,colorBtmp;
	STRCODE fcode;

	//描画位置初期化
	tw->nowx = text->writex;
	tw->nowy = text->writey;

	colorFtmp = (text->colorF<<28)|(text->colorF<<24)|(text->colorF<<20)|(text->colorF<<16)|
				(text->colorF<<12)|(text->colorF<<8 )|(text->colorF<<4 )|(text->colorF<<0 );
	colorBtmp = (text->colorB<<28)|(text->colorB<<24)|(text->colorB<<20)|(text->colorB<<16)|
				(text->colorB<<12)|(text->colorB<<8 )|(text->colorB<<4 )|(text->colorB<<0 );

	//フォントデータ取得
	while( (fcode = *strdat) != EOM_ ) 
	{
		strdat++;

		switch(fcode){

		case EOM_:	/* 終了コード */
			GFL_HEAP_FreeMemory( tw );
			return;

		case CR_:	/* 改行コード */
			tw->nowx = text->writex;
			tw->nowy += ( SYSTEMFONT_SIZEY + text->spacey ); 
			break;

		default:
			FontDataGet( fcode, (u32*)tw->fdata, &tw->fdatasizex, colorFtmp, colorBtmp );

#if 0
			if( tw->mode == GFL_TEXT_WRITE_16 ){
				GFL_BMP_PrintMain
			} else {
				GFL_BMP_PrintMain256
			}
#endif
			//次の文字の描画位置を設定
			text->writex += ( tw->fdatasizex + text->spacex );
			break;
		}
	}
}


static void 
	FontDataGet
		( STRCODE fcode, u32* dst, u8* sizex, u32 colorFtmp , u32 colorBtmp )
{
	u8*	src = (u8*)&GFL_TEXT_DataTable[ fcode * SYSTEMFONT_DATA_SIZE ];
	u32 mask;
	
	mask = mask_table[ *src++ ];
	*dst++ = (colorFtmp & mask)|(colorBtmp & (mask^0xffff));
	mask = mask_table[ *src++ ];
	*dst++ = (colorFtmp & mask)|(colorBtmp & (mask^0xffff));
	mask = mask_table[ *src++ ];
	*dst++ = (colorFtmp & mask)|(colorBtmp & (mask^0xffff));
	mask = mask_table[ *src++ ];
	*dst++ = (colorFtmp & mask)|(colorBtmp & (mask^0xffff));
	mask = mask_table[ *src++ ];
	*dst++ = (colorFtmp & mask)|(colorBtmp & (mask^0xffff));
	mask = mask_table[ *src++ ];
	*dst++ = (colorFtmp & mask)|(colorBtmp & (mask^0xffff));
	mask = mask_table[ *src++ ];
	*dst++ = (colorFtmp & mask)|(colorBtmp & (mask^0xffff));
	mask = mask_table[ *src++ ];
	*dst++ = (colorFtmp & mask)|(colorBtmp & (mask^0xffff));

	*sizex = GFL_TEXT_SizeTable[ fcode ];
}

static const u32 mask_table[256] = {
	0x00000000, 0x0000000F, 0x000000F0, 0x000000FF, 0x00000F00, 0x00000F0F, 0x00000FF0, 0x00000FFF,
	0x0000F000, 0x0000F00F, 0x0000F0F0, 0x0000F0FF, 0x0000FF00, 0x0000FF0F, 0x0000FFF0, 0x0000FFFF,
	0x000F0000, 0x000F000F, 0x000F00F0, 0x000F00FF, 0x000F0F00, 0x000F0F0F, 0x000F0FF0, 0x000F0FFF,
	0x000FF000, 0x000FF00F, 0x000FF0F0, 0x000FF0FF, 0x000FFF00, 0x000FFF0F, 0x000FFFF0, 0x000FFFFF,
	0x00F00000, 0x00F0000F, 0x00F000F0, 0x00F000FF, 0x00F00F00, 0x00F00F0F, 0x00F00FF0, 0x00F00FFF,
	0x00F0F000, 0x00F0F00F, 0x00F0F0F0, 0x00F0F0FF, 0x00F0FF00, 0x00F0FF0F, 0x00F0FFF0, 0x00F0FFFF,
	0x00FF0000, 0x00FF000F, 0x00FF00F0, 0x00FF00FF, 0x00FF0F00, 0x00FF0F0F, 0x00FF0FF0, 0x00FF0FFF,
	0x00FFF000, 0x00FFF00F, 0x00FFF0F0, 0x00FFF0FF, 0x00FFFF00, 0x00FFFF0F, 0x00FFFFF0, 0x00FFFFFF,
	0x0F000000, 0x0F00000F, 0x0F0000F0, 0x0F0000FF, 0x0F000F00, 0x0F000F0F, 0x0F000FF0, 0x0F000FFF,
	0x0F00F000, 0x0F00F00F, 0x0F00F0F0, 0x0F00F0FF, 0x0F00FF00, 0x0F00FF0F, 0x0F00FFF0, 0x0F00FFFF,
	0x0F0F0000, 0x0F0F000F, 0x0F0F00F0, 0x0F0F00FF, 0x0F0F0F00, 0x0F0F0F0F, 0x0F0F0FF0, 0x0F0F0FFF,
	0x0F0FF000, 0x0F0FF00F, 0x0F0FF0F0, 0x0F0FF0FF, 0x0F0FFF00, 0x0F0FFF0F, 0x0F0FFFF0, 0x0F0FFFFF,
	0x0FF00000, 0x0FF0000F, 0x0FF000F0, 0x0FF000FF, 0x0FF00F00, 0x0FF00F0F, 0x0FF00FF0, 0x0FF00FFF,
	0x0FF0F000, 0x0FF0F00F, 0x0FF0F0F0, 0x0FF0F0FF, 0x0FF0FF00, 0x0FF0FF0F, 0x0FF0FFF0, 0x0FF0FFFF,
	0x0FFF0000, 0x0FFF000F, 0x0FFF00F0, 0x0FFF00FF, 0x0FFF0F00, 0x0FFF0F0F, 0x0FFF0FF0, 0x0FFF0FFF,
	0x0FFFF000, 0x0FFFF00F, 0x0FFFF0F0, 0x0FFFF0FF, 0x0FFFFF00, 0x0FFFFF0F, 0x0FFFFFF0, 0x0FFFFFFF,
	0xF0000000, 0xF000000F, 0xF00000F0, 0xF00000FF, 0xF0000F00, 0xF0000F0F, 0xF0000FF0, 0xF0000FFF,
	0xF000F000, 0xF000F00F, 0xF000F0F0, 0xF000F0FF, 0xF000FF00, 0xF000FF0F, 0xF000FFF0, 0xF000FFFF,
	0xF00F0000, 0xF00F000F, 0xF00F00F0, 0xF00F00FF, 0xF00F0F00, 0xF00F0F0F, 0xF00F0FF0, 0xF00F0FFF,
	0xF00FF000, 0xF00FF00F, 0xF00FF0F0, 0xF00FF0FF, 0xF00FFF00, 0xF00FFF0F, 0xF00FFFF0, 0xF00FFFFF,
	0xF0F00000, 0xF0F0000F, 0xF0F000F0, 0xF0F000FF, 0xF0F00F00, 0xF0F00F0F, 0xF0F00FF0, 0xF0F00FFF,
	0xF0F0F000, 0xF0F0F00F, 0xF0F0F0F0, 0xF0F0F0FF, 0xF0F0FF00, 0xF0F0FF0F, 0xF0F0FFF0, 0xF0F0FFFF,
	0xF0FF0000, 0xF0FF000F, 0xF0FF00F0, 0xF0FF00FF, 0xF0FF0F00, 0xF0FF0F0F, 0xF0FF0FF0, 0xF0FF0FFF,
	0xF0FFF000, 0xF0FFF00F, 0xF0FFF0F0, 0xF0FFF0FF, 0xF0FFFF00, 0xF0FFFF0F, 0xF0FFFFF0, 0xF0FFFFFF,
	0xFF000000, 0xFF00000F, 0xFF0000F0, 0xFF0000FF, 0xFF000F00, 0xFF000F0F, 0xFF000FF0, 0xFF000FFF,
	0xFF00F000, 0xFF00F00F, 0xFF00F0F0, 0xFF00F0FF, 0xFF00FF00, 0xFF00FF0F, 0xFF00FFF0, 0xFF00FFFF,
	0xFF0F0000, 0xFF0F000F, 0xFF0F00F0, 0xFF0F00FF, 0xFF0F0F00, 0xFF0F0F0F, 0xFF0F0FF0, 0xFF0F0FFF,
	0xFF0FF000, 0xFF0FF00F, 0xFF0FF0F0, 0xFF0FF0FF, 0xFF0FFF00, 0xFF0FFF0F, 0xFF0FFFF0, 0xFF0FFFFF,
	0xFFF00000, 0xFFF0000F, 0xFFF000F0, 0xFFF000FF, 0xFFF00F00, 0xFFF00F0F, 0xFFF00FF0, 0xFFF00FFF,
	0xFFF0F000, 0xFFF0F00F, 0xFFF0F0F0, 0xFFF0F0FF, 0xFFF0FF00, 0xFFF0FF0F, 0xFFF0FFF0, 0xFFF0FFFF,
	0xFFFF0000, 0xFFFF000F, 0xFFFF00F0, 0xFFFF00FF, 0xFFFF0F00, 0xFFFF0F0F, 0xFFFF0FF0, 0xFFFF0FFF,
	0xFFFFF000, 0xFFFFF00F, 0xFFFFF0F0, 0xFFFFF0FF, 0xFFFFFF00, 0xFFFFFF0F, 0xFFFFFFF0, 0x0FFFFFFF,
};





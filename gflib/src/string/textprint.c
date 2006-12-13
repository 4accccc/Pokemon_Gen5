//=============================================================================================
/**
 * @file	textprint.c
 * @brief	ビットマップ文字列表示用システム
 * 
 */
//=============================================================================================
#include "gflib.h"

#include "textprint.h"
//------------------------------------------------------------------
/**
 * ワーク構造体
 */
//------------------------------------------------------------------
typedef struct {
	GFL_FONT_DATA		fdata;
	u16					nowx;
	u16					nowy;
}TEXT_WORK;

TEXT_WORK* tw;

//------------------------------------------------------------------
/**
 *
 * 初期化
 *
 */
//------------------------------------------------------------------
void
	GFL_TEXT_sysInit
		( const char* fontDataPath )
{
	GFL_FONT_sysInit( fontDataPath );
	tw = GFL_HEAP_AllocMemory( GFL_HEAPID_SYSTEM, sizeof(TEXT_WORK) );
}


//------------------------------------------------------------------
/**
 *
 * 終了
 *
 */
//------------------------------------------------------------------
void
	GFL_TEXT_sysExit
		( void )
{
	GFL_HEAP_FreeMemory( tw );
	GFL_FONT_sysExit();
}


//------------------------------------------------------------------
/**
 * ビットマップの内部CGX領域に文字を描画する(STRBUF)
 *
 * @param   text		文字列
 * @param   param		描画用パラメータ
 */
//------------------------------------------------------------------
void
	GFL_TEXT_Print
		( STRBUF* text, GFL_TEXT_PRINTPARAM* param )
{
	STRCODE* strdat = (STRCODE*)GFL_STR_GetStringCodePointer( text );
	GFL_TEXT_PrintCode( strdat, param );
}


//------------------------------------------------------------------
/**
 * ビットマップの内部CGX領域に文字を描画する(STRCODE)
 *
 * @param   textcode	文字列
 * @param   param		描画用パラメータ
 */
//------------------------------------------------------------------
void
	GFL_TEXT_PrintCode
		( STRCODE* textcode, GFL_TEXT_PRINTPARAM* param )
{
	u8				sizemaxY	= GFL_FONT_GetSizeMaxY();
	u8				sizeSPC		= GFL_FONT_GetSizeSPC();
	STRCODE			fcode;
	GFL_BMP_DATA	bmpfont;

	//描画位置初期化
	tw->nowx = param->writex;
	tw->nowy = param->writey;

	//色設定
	GFL_FONT_SetColor( param->colorF, param->colorB );

	//フォントデータ取得
	while( (fcode = *textcode ) != EOM_ ) 
	{
		textcode++;

		switch(fcode){

		case EOM_:	/* 終了コード */
			GFL_HEAP_FreeMemory( tw );
			return;

		case CR_:	/* 改行コード */
			tw->nowx = param->writex;
			tw->nowy += ( sizemaxY + param->spacey ); 
			break;

		case spc_:	/* 改行コード */
			tw->nowx += sizeSPC;
			break;

		default:
			GFL_FONT_GetData( fcode - STR_DEFAULT_CODE_MAX, &tw->fdata );
			//フォントビットマップ情報作成
			bmpfont.adrs = tw->fdata.data;
			bmpfont.size_x = tw->fdata.sizex;
			bmpfont.size_y = sizemaxY;

			if( param->mode == GFL_TEXT_WRITE_16 ){
				GFL_BMP_PrintMain(	&bmpfont, param->bmp, 0, 0, tw->nowx, tw->nowy, 
										bmpfont.size_x, bmpfont.size_y, 0 );
			} else {
				GFL_BMP_PrintMain256(	&bmpfont, param->bmp, 0, 0, tw->nowx, tw->nowy, 
											bmpfont.size_x, bmpfont.size_y, 0 );
			}
			//次の文字の描画位置を設定
			tw->nowx += ( tw->fdata.sizex + param->spacex );
			break;
		}
	}
}




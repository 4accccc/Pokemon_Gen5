//============================================================================================
/**
 * @file		print_tool.c
 * @brief		文字描画ツール
 * @author	Hiroyuki Nakamura
 * @date		09.10.01
 *
 *	モジュール名：PRINTTOOL
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "print/wordset.h"
#include "msg/msg_print_tool.h"

#include "print_tool.h"


//============================================================================================
//	定数定義
//============================================================================================
#define	NUM_KETA				( 8 )							// 数字表示の最大桁数　8桁
#define	NUM_EXPBUF_SIZ	( NUM_KETA + 2 )	// 桁表示のバッファサイズ　8桁+EOM+なんとなく偶数で。


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static u16 PutSizeGet( const STRBUF * buf, GFL_FONT * font, u32 mode );


//--------------------------------------------------------------------------------------------
/**
 * @brief		基準座標を指定して文字列描画
 *
 * @param		wk			PRINT_UTIL
 * @param		que			PRINT_QUE
 * @param		x				表示基準Ｘ座標
 * @param		y				表示基準Ｙ座標
 * @param		buf			文字列
 * @param		font		フォント
 * @param		mode		表示モード
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void PRINTTOOL_Print(
				PRINT_UTIL * wk, PRINT_QUE * que, u16 x, u16 y,
				const STRBUF * buf, GFL_FONT * font, u32 mode )
{
	x -= PutSizeGet( buf, font, mode );
	PRINT_UTIL_Print( wk, que, x, y, buf, font );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		基準座標を指定して文字列描画（カラー指定）
 *
 * @param		wk			PRINT_UTIL
 * @param		que			PRINT_QUE
 * @param		x				表示基準Ｘ座標
 * @param		y				表示基準Ｙ座標
 * @param		buf			文字列
 * @param		font		フォント
 * @param		color		文字色
 * @param		mode		表示モード
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void PRINTTOOL_PrintColor(
				PRINT_UTIL * wk, PRINT_QUE * que, u16 x, u16 y,
				const STRBUF * buf, GFL_FONT * font, PRINTSYS_LSB color, u32 mode )
{
	x -= PutSizeGet( buf, font, mode );
	PRINT_UTIL_PrintColor( wk, que, x, y, buf, font, color );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		[ ?? / ?? ] 表示
 *
 * @param		wk			PRINT_UTIL
 * @param		que			PRINT_QUE
 * @param		font		フォント
 * @param		x				表示基準Ｘ座標
 * @param		y				表示基準Ｙ座標
 * @param		nowPrm	現在の値
 * @param		maxPrm	最大値
 * @param		heapID	ヒープＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void PRINTTOOL_PrintFraction(
					PRINT_UTIL * wk, PRINT_QUE * que, GFL_FONT * font,
					u16 x, u16 y, u32 nowPrm, u32 maxPrm, HEAPID heapID )
{
	GFL_MSGDATA * mman;
	WORDSET * wset;
	STRBUF * str;
	STRBUF * buf;
	u16	siz;

	mman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_print_tool_dat, heapID );
	wset = WORDSET_Create( heapID );
	buf  = GFL_STR_CreateBuffer( NUM_EXPBUF_SIZ, heapID );

	//「／」
	str = GFL_MSG_CreateString( mman, str_slash );
	siz = PRINTSYS_GetStrWidth( str, font, 0 );
	PRINTTOOL_Print( wk, que, x, y, str, font, PRINTTOOL_MODE_CENTER );
	GFL_STR_DeleteBuffer( str );
	// nowPrm
	str = GFL_MSG_CreateString( mman, str_now_param );
	WORDSET_RegisterNumber( wset, 0, nowPrm, NUM_KETA, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wset, buf, str );
	PRINTTOOL_Print( wk, que, x-(siz/2), y, buf, font, PRINTTOOL_MODE_RIGHT );
	GFL_STR_DeleteBuffer( str );
	// maxPrm
	str = GFL_MSG_CreateString( mman, str_max_param );
	WORDSET_RegisterNumber( wset, 0, maxPrm, NUM_KETA, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wset, buf, str );
	PRINTTOOL_Print( wk, que, x+siz, y, buf, font, PRINTTOOL_MODE_LEFT );
	GFL_STR_DeleteBuffer( str );

	GFL_STR_DeleteBuffer( buf );
	WORDSET_Delete( wset );
	GFL_MSG_Delete( mman );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		[ ?? / ?? ] 表示（カラー指定）
 *
 * @param		wk			PRINT_UTIL
 * @param		que			PRINT_QUE
 * @param		font		フォント
 * @param		x				表示基準Ｘ座標
 * @param		y				表示基準Ｙ座標
 * @param		color		文字色
 * @param		nowPrm	現在の値
 * @param		maxPrm	最大値
 * @param		heapID	ヒープＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void PRINTTOOL_PrintFractionColor(
					PRINT_UTIL * wk, PRINT_QUE * que, GFL_FONT * font,
					u16 x, u16 y, PRINTSYS_LSB color, u32 nowPrm, u32 maxPrm, HEAPID heapID )
{
	GFL_MSGDATA * mman;
	WORDSET * wset;
	STRBUF * str;
	STRBUF * buf;
	u16	siz;

	mman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_print_tool_dat, heapID );
	wset = WORDSET_Create( heapID );
	buf  = GFL_STR_CreateBuffer( NUM_EXPBUF_SIZ, heapID );

	//「／」
	str = GFL_MSG_CreateString( mman, str_slash );
	siz = PRINTSYS_GetStrWidth( str, font, 0 );
	PRINTTOOL_PrintColor( wk, que, x, y, str, font, color, PRINTTOOL_MODE_CENTER );
	GFL_STR_DeleteBuffer( str );
	// nowPrm
	str = GFL_MSG_CreateString( mman, str_now_param );
	WORDSET_RegisterNumber( wset, 0, nowPrm, NUM_KETA, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wset, buf, str );
	PRINTTOOL_PrintColor( wk, que, x-(siz/2), y, buf, font, color, PRINTTOOL_MODE_RIGHT );
	GFL_STR_DeleteBuffer( str );
	// maxPrm
	str = GFL_MSG_CreateString( mman, str_max_param );
	WORDSET_RegisterNumber( wset, 0, maxPrm, NUM_KETA, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wset, buf, str );
	PRINTTOOL_PrintColor( wk, que, x+siz, y, buf, font, color, PRINTTOOL_MODE_LEFT );
	GFL_STR_DeleteBuffer( str );

	GFL_STR_DeleteBuffer( buf );
	WORDSET_Delete( wset );
	GFL_MSG_Delete( mman );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列表示サイズから各モードの補正値を取得
 *
 * @param		buf			文字列
 * @param		font		フォント
 * @param		mode		表示モード
 *
 * @return	補正値
 */
//--------------------------------------------------------------------------------------------
static u16 PutSizeGet( const STRBUF * buf, GFL_FONT * font, u32 mode )
{
	// 右詰
	if( mode == PRINTTOOL_MODE_RIGHT ){
		return PRINTSYS_GetStrWidth( buf, font, 0 );
	// 中央
	}else if( mode == PRINTTOOL_MODE_CENTER ){
		return ( PRINTSYS_GetStrWidth( buf, font, 0 ) / 2 );
	}
	return 0;
}

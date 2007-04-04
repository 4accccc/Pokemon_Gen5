//============================================================================================
/**
 * @file	sjisconv.c
 * @brief	ＳｊｉｓコードからＧＦＬシステムフォントコードへの変換
 */
//============================================================================================
#include "gflib.h"
#include "systemfontcode.h"

#include "sjisconv_inter.h"


#define UNKNOWN_CHARCODE ( period_ )
#define IGNORE_CHARCODE  ( 0xfffe )

static STRCODE convert1( const unsigned char sjisCode );
static STRCODE convert2( const unsigned short sjisCode );

//------------------------------------------------------------------
/**
 * 文字列コンバート（ Sjis > SystemFontCode ）
 *
 * @param   src				変換前Sjis文字列ポインタ
 * @param   dst				変換後STRCODE文字列ポインタ
 * @param   dstCodeCount	STRCODE格納最大文字数（オーバーフロー回避用）
 *
 */
//------------------------------------------------------------------
void 
	GFL_STR_CONV_Sjis2Systemfontcode
		( const char* src, STRCODE* dst, int dstCodeCount )
{
	STRCODE code;
	int	i = 0;

	while( (*src != '\0')&&(i < (dstCodeCount-1)) ){
		unsigned char sjis = *src;
		src++;

		// 2byte文字の1byte目(ASCII/JISローマ字、半角カタカナエリアのチェック)
		if( ((sjis >= 0x81) && (sjis <= 0x9f)) || ((sjis >= 0xe0) && (sjis <= 0xff)) )
		{
			code = convert2( (sjis << 8) + (short)(*src & 0xff) );// 2byte領域コンバート
			src++;
		} else {
			code = convert1( sjis );// 1byte領域コンバート
		}
		if( code != IGNORE_CHARCODE ){
			*dst = code;
			dst++;
			i++;
		}
	}
	*dst = EOM_;
}


//------------------------------------------------------------------
/**
 * １文字コンバート【1byte領域】（ Sjis > SystemFontCode ）
 *
 * @param   sjisCode
 *
 * @retval	fontCode  
 */
//------------------------------------------------------------------
static const struct {
	unsigned char  sjis_code;
	STRCODE        font_code;
}code_translate_tbl[] = {
	{  ' ',   h_spc_ },
	{  '\n',  CR_ },
	{  '?',   hate_ },
	{  '=',   equal_ },
	{  '_',   uber_ },
	{  '[',   kakukako_ },
	{  ']',   kakukakot_ },
	{  '@',   atmark_ },
	{  ':',   colon_ },
	{  ';',   semicolon_ },
	{  '\'',  yen_ },
	{  '\r',  IGNORE_CHARCODE },
};

static STRCODE convert1( const unsigned char sjisCode )
{
	if((sjisCode >= 'a') && (sjisCode <= 'z' ))		return a__ + (sjisCode - 'a');
	if((sjisCode >= 'A') && (sjisCode <= 'Z' ))		return A__ + (sjisCode - 'A');
	if((sjisCode >= '0') && (sjisCode <= '9' ))		return h_n0_ + (sjisCode - '0');
	if((sjisCode >= '!') && (sjisCode <= '/' ))		return gyoe_ + (sjisCode - '!');
	{
		int i;

		for(i=0; i<NELEMS(code_translate_tbl); i++)
		{
			if( sjisCode == code_translate_tbl[i].sjis_code )
			{
				return code_translate_tbl[i].font_code;
				break;
			}
		}
	}
	return UNKNOWN_CHARCODE;
}


//------------------------------------------------------------------
/**
 * １文字コンバート【2byte領域】（ Sjis > SystemFontCode ）
 *
 * @param   sjisCode
 *
 * @retval	fontCode  
 */
//------------------------------------------------------------------
static STRCODE convert2( const unsigned short sjisCode )
{
	if((sjisCode >= 'ぁ') && (sjisCode <= 'ん' ))	return aa_ + (sjisCode - 'ぁ');
	if((sjisCode >= 'ァ') && (sjisCode <= 'ン' ))	return AA_ + (sjisCode - 'ァ');
	if((sjisCode >= '０') && (sjisCode <= '９' ))	return n0_ + (sjisCode - '０');
	if(sjisCode == '　') return spc_;
	if(sjisCode == 'ー') return bou_;

	return UNKNOWN_CHARCODE;
}



//==============================================================================
/**
 *
 *@file		sjisconv_inter.h
 *@brief	文字列変換
 *
 */
//==============================================================================
#ifndef __SJISCONV_H__
#define __SJISCONV_H__

#include "strbuf.h"

//------------------------------------------------------------------
/**
 * 文字列コンバート（ Sjis > SystemFontCode ）
 *
 * @param   src				変換前Sjis文字列ポインタ
 * @param   dst				変換後STRCODE文字列ポインタ
 * @param   dstcodesize		STRCODE文字列最大サイズ（オーバーフロー回避用）
 *
 */
//------------------------------------------------------------------
extern void GFL_STR_CONV_Sjis2Systemfontcode( const char* src, STRCODE* dst, int dstcodesCount );

#endif



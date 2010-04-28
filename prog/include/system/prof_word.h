//======================================================================
/**
 * @file	prof_word.c
 * @brief	不正文字列検査
 * @author	ariizumi
 * @data	100428
 *
 * モジュール名：PROF_WORD
 */
//======================================================================
#pragma once

//  不正文字列チェック
//  TRUE 不正 FALSE 有効
extern const BOOL PROF_WORD_CheckProfanityWord( const STRBUF *strBuf , const HEAPID heapId );
extern const BOOL PROF_WORD_CheckProfanityWordCode( const STRCODE *code , const HEAPID heapId );

//　不正数字チェック
//  TRUE 不正 FALSE 有効
extern const BOOL PROF_WORD_CheckProfanityNumber( const STRBUF *strBuf , const HEAPID heapId );
extern const BOOL PROF_WORD_CheckProfanityNumberCode( const STRCODE *strCode , const u16 str_len, const HEAPID heapId );

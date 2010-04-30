//======================================================================
/**
 * @file	prof_word.h
 * @brief	不正文字列検査
 * @author	ariizumi
 * @data	100428
 *
 * モジュール名：PROF_WORD
 */
//======================================================================
#pragma once

typedef struct _PROF_WORD_WORK PROF_WORD_WORK;
//イントロ用、初期化
extern PROF_WORD_WORK* PROF_WORD_AllocWork( const HEAPID heapId );
extern void PROF_WORD_FreeWork( PROF_WORD_WORK* work );

//  不正文字列チェック
//  TRUE 不正 FALSE 有効
//  workはNULLでも動きます(中でファイルをAllocします
extern const BOOL PROF_WORD_CheckProfanityWord( PROF_WORD_WORK *work , const STRBUF *strBuf , const HEAPID heapId );
extern const BOOL PROF_WORD_CheckProfanityWordCode( PROF_WORD_WORK *work , const STRCODE *strCode, const HEAPID heapId );

//　不正数字チェック
//  TRUE 不正 FALSE 有効
extern const BOOL PROF_WORD_CheckProfanityNumber( const STRBUF *strBuf , const HEAPID heapId );
extern const BOOL PROF_WORD_CheckProfanityNumberCode( const STRCODE *strCode , const u16 str_len, const HEAPID heapId );


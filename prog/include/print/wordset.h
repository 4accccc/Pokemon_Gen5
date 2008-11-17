//============================================================================================
/**
 * @file	wordset.h
 * @brief	単語モジュール処理
 * @author	taya
 * @date	2005.09.28
 */
//============================================================================================
#ifndef __WORDSET_H__
#define __WORDSET_H__


#include "poke_tool/poke_tool.h"
#include "print/str_tool.h"

typedef struct _WORDSET  WORDSET;


enum {
	WORDSET_DEFAULT_SETNUM = 8,		// デフォルトバッファ数
	WORDSET_DEFAULT_BUFLEN = 32,	// デフォルトバッファ長（文字数）
};


extern WORDSET* WORDSET_CreateEx( u32 word_max, u32 buflen, u32 heapID );
extern void WORDSET_Delete( WORDSET* wordset );
extern void WORDSET_RegisterWord( WORDSET* wordset, u32 bufID, const STRBUF* word, u32 sex, BOOL singular_flag, u32 lang );
extern void WORDSET_RegisterPokeMonsName( WORDSET* wordset, u32 bufID, POKEMON_PASO_PARAM* ppp );
extern void WORDSET_RegisterPokeNickName( WORDSET* wordset, u32 bufID, POKEMON_PASO_PARAM* ppp );
extern void WORDSET_RegisterPokeOyaName( WORDSET* wordset,  u32 bufID, POKEMON_PASO_PARAM* ppp );
extern void WORDSET_RegisterWazaName( WORDSET* wordset, u32 bufID, u32 wazaID );
extern void WORDSET_RegisterTokuseiName( WORDSET* wordset, u32 bufID, u32 tokuseiID );
extern void WORDSET_RegisterNumber( WORDSET* wordset, u32 bufID, s32 number, u32 keta, StrNumberDispType dispType, StrNumberCodeType codeType );
extern void WORDSET_ExpandStr( const WORDSET* wordset, STRBUF* dstbuf, const STRBUF* srcbuf );
extern void WORDSET_ClearAllBuffer( WORDSET* wordset );



#endif

//============================================================================================
/**
 * @file	  debug_wordsearch.h
 * @brief	  デバッグ用、文字列検索により GFL_SKB を使いやすくするルーチン
            どのオーバーレイからでも困らないように
            ヘッダーインライン形式になっている
 * @author	k.ohno
 * @date	  2009.08.19
 */
//============================================================================================

#pragma once

//------------------------------------------------------------------------------
/**
 * @brief   文字比較
 * @param   const STRBUF* str1   比較文字列
 * @param   const STRBUF* str2   比較文字列
 * @param   u32 len              長さ
 * @retval  探したらTRUE
 */
//------------------------------------------------------------------------------

static inline BOOL DEBUG_WORDSEARCH_sub_strncmp( const STRBUF* str1, const STRBUF* str2, u32 len )
{
	if( GFL_STR_GetBufferLength(str1) < len ){
		return FALSE;
	}
	if( GFL_STR_GetBufferLength(str2) < len ){
		return FALSE;
	}

	{
		const STRCODE *p1 = GFL_STR_GetStringCodePointer( str1 );
		const STRCODE *p2 = GFL_STR_GetStringCodePointer( str2 );
		u32 i;
		for(i=0; i<len; ++i){
			if( *p1++ != *p2++ ){ return FALSE; }
		}
		return TRUE;
	}
	return FALSE;
}


//------------------------------------------------------------------------------
/**
 * @brief   目標文字列のサーチ
 * @param   GFL_MSGDATA *CompMsgManager;	探すＭＳＧが格納されているマネージャー
 * @param   STRBUF* fullword;             取得する為のテンポラリ文字列
 * @param   const STRBUF* word         探す文字列
 * @param   int dir                    方向 1 か -1
 * @param   int* startidx              探す最初のindex位置 探した時のindex位置
 * @retval  探したらTRUE
 */
//------------------------------------------------------------------------------

static inline BOOL DEBUG_WORDSEARCH_sub_search( GFL_MSGDATA *CompMsgManager, STRBUF* fullword, const STRBUF* word, int dir, int* startidx )
{
	int word_len = GFL_STR_GetBufferLength( word );
	if( word_len )
	{
		int str_cnt, match_cnt, i, allcnt=0;

		str_cnt = GFL_MSG_GetStrCount( CompMsgManager ); //文字列全体

		i = *startidx;
		while( allcnt < str_cnt )
		{
			i += dir;
			if(i < 0){
				i = str_cnt-1;
			}
			else if(i >= str_cnt){
				i = 0;
			}
			GFL_MSG_GetString( CompMsgManager, i, fullword );
			if( DEBUG_WORDSEARCH_sub_strncmp( word, fullword, GFL_STR_GetBufferLength(word) ) ){
				*startidx = i;
				return TRUE;
			}
			allcnt++;
		}
	}
	return FALSE;
}



//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		msgsearch.c
 *	@brief	MSGから文字列を検索するモジュール
 *	@author	Toru=Nagihashi
 *	@data		2009.10.30
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "ui/msgsearch.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define MSGSEARCH_BUFFER_SIZE	(128)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	MSGデータから文字列を検索するワーク
//=====================================
struct _MSGSEARCH_WORK
{	
	STRBUF	*p_word;
	u32 msg_max;
	GFL_MSGDATA *p_msg_tbl[0];
};

//=============================================================================
/**
 *					PRIVATE
*/
//=============================================================================
static BOOL sub_strncmp( const STRBUF* str1, const STRBUF* str2, u32 len );

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	MSG検索	初期化
 *
 *	@param	GFL_MSGDATA *p_msg_tbl	検索するメッセージデータのテーブル
 *	@param	msg_max									検索するメッセージデータの数
 *	@param	heapID									ヒープID 
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
MSGSEARCH_WORK *MSGSEARCH_Init( GFL_MSGDATA *p_msg_tbl[], u32 msg_max, HEAPID heapID )
{	
	u32 size;
	MSGSEARCH_WORK	*p_wk;

	size	= sizeof(MSGSEARCH_WORK) + msg_max * sizeof(GFL_MSGDATA *);
	p_wk	= GFL_HEAP_AllocMemory( heapID, size );
	GFL_STD_MemClear( p_wk, size );

	p_wk->msg_max	= msg_max;
	{	
		int i;
		for( i = 0; i < msg_max; i++ )
		{	
			p_wk->p_msg_tbl[i]	= p_msg_tbl[i];
		}
	}

	p_wk->p_word		= GFL_STR_CreateBuffer( MSGSEARCH_BUFFER_SIZE, heapID );

	return	p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	MSG検索	破棄
 *
 *	@param	MSGSEARCH_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void MSGSEARCH_Exit( MSGSEARCH_WORK *p_wk )
{	
	GFL_STR_DeleteBuffer( p_wk->p_word );

	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	MSG検索	メッセージ１つのみ検索
 *
 *	@param	MSGSEARCH_WORK *p_wk
 *	@param	検索するメッセージのインデックス（INITでの登録テーブルのインデックス）
 *	@param	検索開始するインデックス				（0ならば全件検索）
 *	@param	STRBUF *cp_buf		検索文字列
 *	@param	*p_result_tbl			検索結果テーブル
 *	@param	tbl_max						検索結果テーブルの数
 *
 *	@return	検索結果	０ならば見つからなかった。　ｎならばｎ発見し結果をテーブルに入れた数
 */
//-----------------------------------------------------------------------------
u32 MSGSEARCH_Search( const MSGSEARCH_WORK *cp_wk, u16 msg_idx, u16 start_str_idx, const STRBUF *cp_buf, MSGSEARCH_RESULT *p_result_tbl, u32 tbl_max )
{	
	int match_cnt	= 0;
	u32 word_len	= GFL_STR_GetBufferLength( cp_buf );

	GFL_STD_MemFill32( p_result_tbl, -1, sizeof(MSGSEARCH_RESULT)*tbl_max );

	if( word_len )
	{	
		int search_idx;
		int result_idx;
		u32	str_max;
		GFL_MSGDATA	*p_msg;

		str_max = GFL_MSG_GetStrCount( cp_wk->p_msg_tbl[ msg_idx ] );
		p_msg		= cp_wk->p_msg_tbl[ msg_idx ];

		search_idx	= 0;
		result_idx	= 0;
		while( 1 )
		{	
			//終了条件
			if( search_idx >= str_max )
			{	
				break;
			}
			if( result_idx >= tbl_max )
			{	
				break;
			}

			GFL_MSG_GetString( p_msg, search_idx, cp_wk->p_word );

      if( sub_strncmp( cp_buf, cp_wk->p_word, GFL_STR_GetBufferLength(cp_buf) ) )
			{
				if( result_idx < tbl_max )
				{	
					p_result_tbl[ result_idx ].msg_idx	= msg_idx;
					p_result_tbl[ result_idx ].str_idx	= search_idx;
					result_idx++;
				}
				match_cnt++;
      }
			search_idx++;
		}
	}

	return match_cnt;
}
//----------------------------------------------------------------------------
/**
 *	@brief	MSG検索	すべてのメッセージを検索した
 *
 *	@param	MSGSEARCH_WORK *p_wk
 *	@param	STRBUF *cp_buf		検索文字列
 *	@param	*p_result_tbl			検索結果テーブル
 *	@param	tbl_max						検索結果テーブルの数
 *
 *	@return	検索結果	０ならば見つからなかった。　ｎならばｎ発見し結果をテーブルに入れた数
 */
//-----------------------------------------------------------------------------
u32 MSGSEARCH_SearchAll( const MSGSEARCH_WORK *cp_wk, const STRBUF *cp_buf, MSGSEARCH_RESULT *p_result_tbl, u32 tbl_max )
{	
	MSGSEARCH_RESULT *p_result;
	u32 tbl_use;
	s32 tbl_rest;
	u32 match_cnt;
	int i;

	match_cnt	= 0;
	tbl_use		= 0;
	tbl_rest	= tbl_max;

	GFL_STD_MemFill32( p_result_tbl, -1, sizeof(MSGSEARCH_RESULT)*tbl_max );

	for( i = 0; i < cp_wk->msg_max; i++ )
	{	
		//テーブル使用量計算
		tbl_rest	-= tbl_use;

		//もし残量が0以下ならば終了
		if( tbl_rest <= 0 )
		{	
			break;
		}
		//使用テーブル先頭ポインタ
		p_result	= &p_result_tbl[ match_cnt ];

		//サーチ
		tbl_use		= MSGSEARCH_Search( cp_wk, i, 0, cp_buf, p_result, tbl_rest );

		//サーチした量を検索数に追加
		match_cnt	+= tbl_use;
	}

  return match_cnt;	
}
//----------------------------------------------------------------------------
/**
 *	@brief	MSG検索	何個存在するか検索
 *							上記Search SearchAll関数の戻り値との違いは、
 *							こちらは全部検索して数を返すのに対し、
 *							上記関数は、テーブルから溢れたらすぐに検索終了してしまう点
 *
 *	@param	MSGSEARCH_WORK *p_wk
 *	@param	STRBUF *cp_buf		検索文字列
 *
 *	@return	検索結果	ｎ個発見した
 */
//-----------------------------------------------------------------------------
u32 MSGSEARCH_SearchNum( const MSGSEARCH_WORK *cp_wk, const STRBUF *cp_buf )
{	
	int i, j;
	u32 match_cnt;

	match_cnt	= 0;
	for( i = 0; i < cp_wk->msg_max; i++ )
	{	
		u32 str_max;
		GFL_MSGDATA	*p_msg;

		p_msg	= cp_wk->p_msg_tbl[ i ];
		str_max	= GFL_MSG_GetStrCount( p_msg );
		for( j = 0; j < str_max; j++ )
		{	
			GFL_MSG_GetString( p_msg, j, cp_wk->p_word );
			if( sub_strncmp( cp_buf, cp_wk->p_word, GFL_STR_GetBufferLength(cp_buf) ) )
			{	
				match_cnt++;
			}
		}
	}

	return match_cnt;
}
//----------------------------------------------------------------------------
/**
 *	@brief	MSG検索	検索結果から文字列を作成する
 *
 *	@param	const MSGSEARCH_WORK *cp_wk				ワーク
 *	@param	MSGSEARCH_RESULT *cp_result				検索結果
 *
 *	@return	STRBUF
 */
//-----------------------------------------------------------------------------
STRBUF *MSGSEARCH_CreateString( const MSGSEARCH_WORK *cp_wk, const MSGSEARCH_RESULT *cp_result )
{	
	GF_ASSERT_MSG( 0 <= cp_result_tbl->msg_idx && cp_result_tbl->msg_idx < cp_wk->msg_max, "登録されたメッセージテーブルをオーバーしています　%d < %d", cp_result_tbl->msg_idx, cp_wk->msg_max );
	return	GFL_MSG_CreateString( cp_wk->p_msg_tbl[ cp_result_tbl->msg_idx ], cp_result_tbl->str_idx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	MSG検索	検索結果から文字列を受け取る
 *
 *	@param	const MSGSEARCH_WORK *cp_wk				ワーク
 *	@param	MSGSEARCH_RESULT *cp_result				検索結果
 *	@param	*p_str														文字列受け取りバッファ
 */
//-----------------------------------------------------------------------------
void MSGSEARCH_GetString( const MSGSEARCH_WORK *cp_wk, const MSGSEARCH_RESULT *cp_result, STRBUF *p_str )
{	
	GF_ASSERT_MSG( 0 <= cp_result_tbl->msg_idx && cp_result_tbl->msg_idx < cp_wk->msg_max, "登録されたメッセージテーブルをオーバーしています　%d < %d", cp_result_tbl->msg_idx, cp_wk->msg_max );
	GFL_MSG_GetString( cp_wk->p_msg_tbl[ cp_result_tbl->msg_idx ], cp_result_tbl->str_idx, p_str );
}
//=============================================================================
/**
 *			PRIVATE
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	const STRBUF* str1	検索文字列
 *	@param	STRBUF* str2				検索元文字列
 *	@param	len									文字列長
 *
 *	@return	TRUEならば一致			さもなくば不一致
 */
//-----------------------------------------------------------------------------
static BOOL sub_strncmp( const STRBUF* str1, const STRBUF* str2, u32 len )
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


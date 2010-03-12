//=============================================================================
/**
 *
 *	@file		pmsi_search.c
 *	@brief  簡易会話入力 検索モジュール
 *	@author	hosaka genya
 *	@data		2009.11.14
 *
 */
//=============================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system\main.h"
#include "system\pms_word.h"
#include "system\pms_data.h"

#include "ui/msgsearch.h"

#include "pmsi_initial_data.h"
#include "pms_input_prv.h"

#include "pmsi_search.h"

#include "pms_abc_gmm_def.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

enum
{ 
  WORD_CODE_MAX = PMS_INPUTWORD_MAX,  ///< 入力できる最大文字数
  WORD_SEARCH_MAX = PMS_ABC_GMMROW_MAX,
};  

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
///	メインワーク
//==============================================================
struct _PMS_INPUT_SEARCH {
  // [IN]
  const PMS_INPUT_WORK* mwk;
  const PMS_INPUT_DATA* dwk;
  HEAPID heap_id;
  // [PRIVATE]
  GFL_MSGDATA* msg_tbl[ PMS_ABC_GMMTBL_MAX ];
  MSGSEARCH_WORK* search_wk;
  MSGSEARCH_RESULT search_result[ WORD_SEARCH_MAX ];
  PMS_WORD result_word[ WORD_SEARCH_MAX+1 ];
  STRBUF* str_search;
  u16 word_code[ WORD_CODE_MAX ];
  u16 padding_u16;
  u8 word_code_pos;
  u8 search_hitnum;
  u8 padding[2];
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================

#ifdef PM_DEBUG
#define DEBUG_PRINT
static void debug_word_code_print( PMS_INPUT_SEARCH* wk )
{ 
  int i;

  for( i=0; i<WORD_CODE_MAX; i++ )
  {
    HOSAKA_Printf("word_code:");
    HOSAKA_Printf("%d ", wk->word_code[i] );
    HOSAKA_Printf("\n");
  }
}
#endif



//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  検索モジュール 生成
 *
 *	@param	const PMS_INPUT_WORK* mwk
 *	@param	const PMS_INPUT_DATA* dwk
 *	@param	heap_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
PMS_INPUT_SEARCH* PMSI_SEARCH_Create( const PMS_INPUT_WORK* mwk, const PMS_INPUT_DATA* dwk, HEAPID heap_id )
{
  int i;
  PMS_INPUT_SEARCH* wk;
  
  wk = GFL_HEAP_AllocClearMemory( heap_id, sizeof( PMS_INPUT_SEARCH ) );

  wk->mwk = mwk;
  wk->dwk = dwk;
  wk->heap_id =  heap_id;

  PMSI_SEARCH_ClearWord( wk );

  // GMM生成
  for( i=0; i<PMS_ABC_GMMTBL_MAX; i++ )
  {
    wk->msg_tbl[i] = GFL_MSG_Create( GFL_MSG_LOAD_FAST, ARCID_MESSAGE, pms_abc_gmmtbl[i], heap_id );
  }

  wk->search_wk = MSGSEARCH_Init( wk->msg_tbl, PMS_ABC_GMMTBL_MAX, wk->heap_id );
  
  wk->str_search = GFL_STR_CreateBuffer( WORD_CODE_MAX*2+EOM_CODESIZE, heap_id );

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  検索モジュール 削除
 *
 *	@param	PMS_INPUT_SEARCH* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSI_SEARCH_Delete( PMS_INPUT_SEARCH* wk )
{ 
  int i;
  
  GFL_STR_DeleteBuffer( wk->str_search );
  
  MSGSEARCH_Exit( wk->search_wk );
  
  for( i=0; i<PMS_ABC_GMMTBL_MAX; i++ )
  {
    GFL_MSG_Delete( wk->msg_tbl[i] );
  }

  GFL_HEAP_FreeMemory( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  一文字登録
 *
 *	@param	PMS_INPUT_SEARCH* wk
 *	@param	word_code 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSI_SEARCH_AddWord( PMS_INPUT_SEARCH* wk, u16 word_code )
{
  GF_ASSERT(wk);
  GF_ASSERT( wk->word_code_pos < WORD_CODE_MAX );

  wk->word_code[ wk->word_code_pos ] = word_code;

  wk->word_code_pos = MATH_CLAMP( wk->word_code_pos+1, 0, WORD_CODE_MAX-1 );

#ifdef DEBUG_PRINT
  debug_word_code_print( wk );
#endif
}

//-----------------------------------------------------------------------------
/**
 *	@brief  一文字削除
 *
 *	@param	PMS_INPUT_SEARCH* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL PMSI_SEARCH_DelWord( PMS_INPUT_SEARCH* wk )
{
  GF_ASSERT(wk);
  GF_ASSERT( wk->word_code_pos < WORD_CODE_MAX );

  // 入力されていなければ何もせずに抜ける
  if( wk->word_code_pos == 0 )
  {
    return FALSE;
  }

  // 末尾が空なら1文字前へ
  if( wk->word_code[ WORD_CODE_MAX-1 ] == INI_DIS )
  {
    wk->word_code_pos--;
  }
  
  wk->word_code[ wk->word_code_pos ] = INI_DIS;

#ifdef DEBUG_PRINT
  debug_word_code_print( wk );
#endif
  
  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  入力されている文字数を得る
 *
 *	@param	const PMS_INPUT_SEARCH* wk 
 *
 *	@retval 入力されている文字数
 */
//-----------------------------------------------------------------------------
u8 PMSI_SEARCH_GetCharNum( const PMS_INPUT_SEARCH* wk )
{
  return wk->word_code_pos;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  入力された文字情報をクリア
 *
 *	@param	PMS_INPUT_SEARCH* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSI_SEARCH_ClearWord( PMS_INPUT_SEARCH* wk )
{
  int i;

  // 入力情報初期化
  for( i=0; i<WORD_CODE_MAX; i++ )
  {
    wk->word_code[i] = INI_DIS;
  }

  wk->word_code_pos = 0;
  
  // 検索結果を0にしておく 
  wk->search_hitnum = 0;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  入力された文字列をout_bufにセット
 *
 *	@param	PMSI_INPUT_SEARCH* wk
 *	@param	out_buf 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSI_SEARCH_GetInputWord( PMS_INPUT_SEARCH* wk, STRBUF* out_buf )
{
  int i;
  STRCODE code[ WORD_CODE_MAX + 1 ];

  for( i=0; i<WORD_CODE_MAX; i++ )
  {
    if( wk->word_code[i] == INI_DIS )
    {
      // 終了文字追加
      code[i] = GFL_STR_GetEOMCode();
      break;
    }

    code[i] = PMSI_INITIAL_DAT_GetStrCode( wk->word_code[i] );
  }
  
  // 最後に終了文字追加
  code[ WORD_CODE_MAX ] = GFL_STR_GetEOMCode();
  
  // 文字列セット
  GFL_STR_SetStringCode( out_buf, code );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  無効文字を後ろに回す
 *
 *	@param	void* elem1
 *	@param	elem2 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static s32 qsort_result( void* elem1, void* elem2 )
{
  MSGSEARCH_RESULT* p1 = (MSGSEARCH_RESULT*) elem1;
  MSGSEARCH_RESULT* p2 = (MSGSEARCH_RESULT*) elem2;
  
  // 等しい
  if( p1->str_idx  == p2->str_idx )
  {
    return 0;
  }
  // 降順
  else if( p1->str_idx > p2->str_idx )
  {
    return 1;
  }
  else
  {
    return -1;
  }


  GF_ASSERT(0);
    
  return 0;

}


//-----------------------------------------------------------------------------
/**
 *	@brief  検索開始
 *
 *	@param	PMSI_INPUT_SEARCH*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL PMSI_SEARCH_Start( PMS_INPUT_SEARCH*wk )
{
  int i;
  u16 search_idx;
  
  // 1文字目で検索対象のGMMを変更
  search_idx = wk->word_code[0];
  HOSAKA_Printf("search idx =%d\n", search_idx );
 
  // 無効文字なら検索しない
  if( search_idx >= INI_SEARCH_MAX )
  {
    wk->search_hitnum = 0; // 結果を0に
    return FALSE;
  }

  // 文字生成
  GFL_STR_ClearBuffer( wk->str_search );
  PMSI_SEARCH_GetInputWord( wk, wk->str_search );

#if 0
  {
    const STRCODE *p1 = GFL_STR_GetStringCodePointer( wk->str_search );
    
    // 生成された文字を表示
    while( *p1 != GFL_STR_GetEOMCode() )
    {
      HOSAKA_Printf("---->0x%x \n", *p1 );
      p1++;
    }
  }
#endif
  
  // 検索
  {
    int i;
    u32 hitnum;
    u32 desable_cnt = 0;
    PMS_WORD rslt_tbl[ WORD_SEARCH_MAX+1 ] = {0};

    hitnum = MSGSEARCH_Search( wk->search_wk, search_idx, 0, wk->str_search, wk->search_result ,WORD_SEARCH_MAX );
    
    HOSAKA_Printf("search hitnum = %d \n\n", hitnum);

    // 検索結果からPMS_WORDを生成
    for( i=0; i<hitnum; i++ )
    {
      rslt_tbl[i] = PMSI_DATA_GetWordToOriginalPos( wk->dwk, search_idx, wk->search_result[i].str_idx );
      HOSAKA_Printf("[%d] word=%d str_idx=%d \n",i, rslt_tbl[i], wk->search_result[i].str_idx );
    }

    // テーブル末尾に末尾データ
    rslt_tbl[hitnum] = PMSI_DATA_GetWordTableEndData( wk->dwk );

    // 無効データを排除
    wk->search_hitnum = PMSI_DATA_GetInitialEnableWordTable( wk->dwk, rslt_tbl, wk->result_word );
    
    HOSAKA_Printf("search enable hitnum = %d \n", wk->search_hitnum );
  
    return wk->search_hitnum > 0;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  検索ヒット数を返す
 *
 *	@param	PMS_INPUT_SEARCH* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u16 PMSI_SEARCH_GetResultCount( PMS_INPUT_SEARCH* wk )
{
  GF_ASSERT( wk );

  return wk->search_hitnum;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  検索結果から文字列を生成
 *
 *	@param	PMS_INPUT_SEARCH* wk
 *	@param	result_idx  
 *	@param  dst_buf
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSI_SEARCH_GetResultString( PMS_INPUT_SEARCH* wk, u8 result_idx, STRBUF* dst_buf )
{
  u32 word_idx;

  GF_ASSERT( wk );
  GF_ASSERT( result_idx <  wk->search_hitnum );
  GF_ASSERT( wk->word_code[0] != INI_DIS );

  word_idx = wk->result_word[ result_idx ];

  HOSAKA_Printf("getresult result_idx=%d word_idx=%d \n", result_idx, word_idx );

  PMSI_DATA_GetWordString( wk->dwk, word_idx, dst_buf );

#if 0
  {
    STRBUF* buf;

    buf = MSGSEARCH_CreateString( wk->search_wk, &wk->search_result[ result_idx ] );
    GFL_STR_CopyBuffer( dst_buf, buf );
    GFL_STR_DeleteBuffer( buf );
  }
#endif


}

//-----------------------------------------------------------------------------
/**
 *	@brief  検索結果のインデックスから PMS_WORD を取得
 *
 *	@param	PMS_INPUT_SEARCH* wk
 *	@param	cur_pos
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
PMS_WORD PMSI_SEARCH_GetWordCode( PMS_INPUT_SEARCH* wk, u32 cur_pos )
{
  GF_ASSERT( wk );
  GF_ASSERT( cur_pos <  wk->search_hitnum );
  GF_ASSERT( wk->word_code[0] != INI_DIS );

  HOSAKA_Printf("getwordcode cur_pos=%d word=%d \n", cur_pos, wk->result_word[cur_pos] );

  return wk->result_word[ cur_pos ];
}

//=============================================================================
/**
 *								static関数
 */
//=============================================================================



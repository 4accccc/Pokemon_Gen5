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
  WORD_CODE_MAX = 7,  ///< 入力できる最大文字数
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
 *	@param	PMS_INPUT_DATA* dwk
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

  // 入力情報初期化
  for( i=0; i<WORD_CODE_MAX; i++ )
  {
    wk->word_code[i] = INI_DIS;
  }

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
  STRCODE code[ WORD_CODE_MAX + 1 ];
  
  // 1文字目で検索対象のGMMを変更
  search_idx = wk->word_code[0];
  HOSAKA_Printf("search idx =%d\n", search_idx );
 
  // 無効文字なら検索しない
  if( search_idx == INI_DIS )
  {
    return FALSE;
  }

  // 絞り込み文字列生成
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
  code[ WORD_SEARCH_MAX ] = GFL_STR_GetEOMCode();
    
  // 文字生成
  GFL_STR_ClearBuffer( wk->str_search );
  GFL_STR_SetStringCode( wk->str_search, code );

#if 1
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
    wk->search_hitnum = MSGSEARCH_Search( wk->search_wk, search_idx, 0, wk->str_search, wk->search_result ,WORD_SEARCH_MAX );
  
    HOSAKA_Printf("hitnum = %d \n\n", wk->search_hitnum);
  
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
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
STRBUF* PMSI_SEARCH_CreateResultString( PMS_INPUT_SEARCH* wk, u8 result_idx )
{
  GF_ASSERT( wk );
  GF_ASSERT( result_idx <  wk->search_hitnum );

  return MSGSEARCH_CreateString( wk->search_wk, &wk->search_result[ result_idx ] );
}

//=============================================================================
/**
 *								static関数
 */
//=============================================================================



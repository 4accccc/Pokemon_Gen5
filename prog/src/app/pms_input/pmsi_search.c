//=============================================================================
/**
 *
 *	@file		pmsi_search.c
 *	@brief  簡易会話入力 検索モジュール
 *	@author		hosaka genya
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
  WORD_CODE_MAX = 7,
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
  // [PRIVATE]
  GFL_MSGDATA* msg_tbl[ PMS_ABC_GMMTBL_MAX ];
  u16 word_code[ WORD_CODE_MAX ];
  u16 padding_u16;
  u8 word_code_pos;
  u8 padding[3];
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
    HOSAKA_Printf("word_code[%d]=%d \n",i, wk->word_code[i] );
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

//  wk->search_wk =  MSGSEARCH_Init( wk->msg_tbl, PMS_ABC_GMMTBL_MAX, heap_id );

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
 *	@brief  検索開始
 *
 *	@param	PMSI_INPUT_SEARCH*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSI_SEARCH_Start( PMS_INPUT_SEARCH*wk )
{
  return ;
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

//=============================================================================
/**
 *								static関数
 */
//=============================================================================



//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_person.c
 *	@brief  ローカルライブキャスターで話しかける人物抽象システム
 *	@author	Toru=Nagihashi
 *	@data		2011.01.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"

//自分のモジュール
#include "local_tvt_local_def.h"
#include "local_tvt_graphic.h"
#include "local_tvt_util.h"

//外部公開
#include "local_tvt_person.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	人物システム
//=====================================
struct _LOCAL_TVT_PERSON_SYS
{
  HEAPID              heapID;
  GFL_CLUNIT          *p_clunit;
  LOCAL_TVT_PERSON    person;
  LOCAL_TVT_MESSAGE   *p_message;
  LOCAL_TVT_CHARA_SYS *p_chara_sys;
  LOCAL_TVT_SEQ       *p_seq;
  LOCAL_TVT_LIST      *p_list;

  u32                 select;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================


//シーケンス関数
static void LOCAL_TVT_SEQ_Greeting( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs );
static void LOCAL_TVT_SEQ_SelectTalk( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs );
static void LOCAL_TVT_SEQ_Talking( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *	@brief  人物システム  初期化
 *
 *	@param	const LOCAL_TVT_PERSON *cp_person キャラ実装型
 *	@param	*p_message    メッセージシステム
 *	@param	*p_chara_sys  キャラシステム
 *	@param  p_clunit      CLユニット
 *	@param	heapID  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_PERSON_SYS * LOCAL_TVT_PERSON_SYS_Init( const LOCAL_TVT_PERSON *cp_person, LOCAL_TVT_MESSAGE *p_message , LOCAL_TVT_CHARA_SYS *p_chara_sys, GFL_CLUNIT *p_clunit, HEAPID heapID )
{
  LOCAL_TVT_PERSON_SYS *p_wk  = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_PERSON_SYS) );
  p_wk->heapID      = heapID;
  p_wk->p_message   = p_message;
  p_wk->p_chara_sys = p_chara_sys;
  p_wk->p_clunit    = p_clunit;
  p_wk->person      = *cp_person;

  //シーケンス管理
  p_wk->p_seq = LOCAL_TVT_SEQ_Init( p_wk, LOCAL_TVT_SEQ_Greeting, heapID );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物システム  破棄
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_PERSON_SYS_Exit( LOCAL_TVT_PERSON_SYS *p_wk )
{
  //シーケンス破棄
  LOCAL_TVT_SEQ_Exit( p_wk->p_seq );

  GFL_HEAP_FreeMemory( p_wk->person.p_wk_adrs );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物システム  メイン処理
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_PERSON_SYS_Main( LOCAL_TVT_PERSON_SYS *p_wk )
{
  LOCAL_TVT_SEQ_Main( p_wk->p_seq );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物システム  終了検知
 *
 *	@param	const LOCAL_TVT_PERSON_SYS *cp_wk   ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_PERSON_SYS_IsEnd( const LOCAL_TVT_PERSON_SYS *cp_wk )
{
  return LOCAL_TVT_SEQ_IsEnd( cp_wk->p_seq );
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物システム  終了リクエスト
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_PERSON_End( LOCAL_TVT_PERSON_SYS *p_wk )
{
  LOCAL_TVT_SEQ_End( p_wk->p_seq );
}
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージを取得
 *
 *	@param	const LOCAL_TVT_PERSON_SYS *cp_wk ワーク
 *
 *	@return メッセージ
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_MESSAGE * LOCAL_TVT_PERSON_GetMessage( const LOCAL_TVT_PERSON_SYS *cp_wk )
{
  return cp_wk->p_message;
}
//----------------------------------------------------------------------------
/**
 *	@brief  キャラ管理を取得
 *
 *	@param	const LOCAL_TVT_PERSON_SYS *cp_wk ワーク
 *
 *	@return キャラ管理
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_CHARA_SYS * LOCAL_TVT_PERSON_GetCharaSys( const LOCAL_TVT_PERSON_SYS *cp_wk )
{
  return cp_wk->p_chara_sys;
}

//----------------------------------------------------------------------------
/**
 *	@brief  CLUNIT取得
 *
 *	@param	const LOCAL_TVT_PERSON_SYS *cp_wk   ワーク
 *
 *	@return CLUNIT
 */
//-----------------------------------------------------------------------------
GFL_CLUNIT * LOCAL_TVT_PERSON_GetClunit( const LOCAL_TVT_PERSON_SYS *cp_wk )
{
  return cp_wk->p_clunit;
}

//=============================================================================
/**
 *  シーケンス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  挨拶
 *
 *	@param	LOCAL_TVT_SEQ *p_seqwk  シーケンスワーク
 *	@param	*p_seq                  シーケンス
 *	@param	*p_wk_adrs              自分のワーク
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_SEQ_Greeting( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_GREET,
    SEQ_END,
  };

  LOCAL_TVT_PERSON_SYS  *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_GREET:
    if( p_wk->person.greetingFunction( p_wk, p_wk->person.p_wk_adrs ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_END:
    LOCAL_TVT_SEQ_SetNext( p_seqwk, LOCAL_TVT_SEQ_SelectTalk );
    break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  会話選択
 *
 *	@param	LOCAL_TVT_SEQ *p_seqwk  シーケンスワーク
 *	@param	*p_seq                  シーケンス
 *	@param	*p_wk_adrs              自分のワーク
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_SEQ_SelectTalk( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };

  LOCAL_TVT_PERSON_SYS  *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_INIT:
    {
      int i;
      LOCAL_TVT_LIST_PARAM  param;
      GFL_STD_MemClear( &param, sizeof(LOCAL_TVT_LIST_PARAM) );
      param.p_message = p_wk->p_message;
      param.p_clunit  = p_wk->p_clunit;
      param.p_msgdata = p_wk->person.p_list_msg_data;
      param.max       = p_wk->person.list_num;
      for( i = 0; i < p_wk->person.list_num; i++ )
      {
        param.msgID[i]  = p_wk->person.list_msgID[i];
      }
      p_wk->p_list  = LOCAL_TVT_LIST_Init( &param, p_wk->heapID );
      (*p_seq)++;
    }
    break;

  case SEQ_MAIN:
    LOCAL_TVT_LIST_Main( p_wk->p_list );
    if( LOCAL_TVT_LIST_IsEnd( p_wk->p_list ) )
    {
      p_wk->select  = LOCAL_TVT_LIST_GetSelect( p_wk->p_list );
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    LOCAL_TVT_LIST_Exit( p_wk->p_list );
    LOCAL_TVT_SEQ_SetNext( p_seqwk, LOCAL_TVT_SEQ_Talking);
    break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  会話
 *
 *	@param	LOCAL_TVT_SEQ *p_seqwk  シーケンスワーク
 *	@param	*p_seq                  シーケンス
 *	@param	*p_wk_adrs              自分のワーク
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_SEQ_Talking( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_TALK,
    SEQ_END,
  };

  LOCAL_TVT_PERSON_SYS  *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_TALK:
    if( p_wk->person.selectFunction[p_wk->select]( p_wk, p_wk->person.p_wk_adrs ) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    LOCAL_TVT_SEQ_SetNext( p_seqwk, LOCAL_TVT_SEQ_SelectTalk );
    break;

  default:
    GF_ASSERT(0);
  }
}

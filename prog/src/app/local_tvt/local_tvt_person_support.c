//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_person_support.c
 *	@brief  各人物処理：ベル
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
#include "local_tvt_person_support.h"

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
///	ベル用ワーク
//=====================================
typedef struct 
{
  s32 seq;
  LOCAL_TVT_POKELIST *p_pokelist;
  GAMEDATA            *p_gamedata;
  HEAPID              heapID;
} LOCAL_TVT_SUPPORT;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static LOCAL_TVT_SUPPORT *LOCAL_TVT_SUPPORT_Init( GAMEDATA *p_gamedata, HEAPID heapID );

static BOOL LOCAL_TVT_SUPPORT_Greeting( LOCAL_TVT_PERSON_SYS *p_person, void *p_wk_adrs );

static BOOL LOCAL_TVT_SUPPORT_Talk( LOCAL_TVT_PERSON_SYS *p_person, void *p_wk_adrs );
static BOOL LOCAL_TVT_SUPPORT_CheckFriend( LOCAL_TVT_PERSON_SYS *p_person, void *p_wk_adrs );
static BOOL LOCAL_TVT_SUPPORT_CheckEffort( LOCAL_TVT_PERSON_SYS *p_person, void *p_wk_adrs );
static BOOL LOCAL_TVT_SUPPORT_Farewell( LOCAL_TVT_PERSON_SYS *p_person, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *	@brief  パーソンワークを設定
 *
 *	@param	LOCAL_TVT_PERSON *p_person  パーソンワーク
 *	@param  GAMEDATA  ゲームデータ
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_SUPPORT_SetPerson( LOCAL_TVT_PERSON *p_person, GFL_MSGDATA *p_msg, GAMEDATA *p_gamedata, HEAPID heapID )
{
  GFL_STD_MemClear( p_person, sizeof(LOCAL_TVT_PERSON));
  p_person->p_wk_adrs = LOCAL_TVT_SUPPORT_Init( p_gamedata, heapID );
  p_person->greetingFunction  = LOCAL_TVT_SUPPORT_Greeting;
  p_person->selectFunction[0] = LOCAL_TVT_SUPPORT_Talk;
  p_person->list_msgID[0]     = 0;
  p_person->selectFunction[1] = LOCAL_TVT_SUPPORT_CheckFriend;
  p_person->list_msgID[1]     = 1;
  p_person->selectFunction[2] = LOCAL_TVT_SUPPORT_CheckEffort;
  p_person->list_msgID[2]     = 2;
  p_person->selectFunction[3] = LOCAL_TVT_SUPPORT_Farewell;
  p_person->list_msgID[3]     = 3;
  p_person->list_num  = 4;
  p_person->p_list_msg_data = p_msg;
}


//=============================================================================
/**
 *					プライベート
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ベル用ワークを作成
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static LOCAL_TVT_SUPPORT *LOCAL_TVT_SUPPORT_Init( GAMEDATA *p_gamedata, HEAPID heapID )
{
  LOCAL_TVT_SUPPORT *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_SUPPORT));
  p_wk->heapID      = heapID;
  p_wk->p_gamedata  = p_gamedata;
  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  挨拶
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SUPPORT_Greeting( LOCAL_TVT_PERSON_SYS *p_person, void *p_wk_adrs )
{
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  はなす
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SUPPORT_Talk( LOCAL_TVT_PERSON_SYS *p_person, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_SUPPORT *p_wk = p_wk_adrs;

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    {
      LOCAL_TVT_POKELIST_PARAM  param;
      GFL_STD_MemClear( &param, sizeof(LOCAL_TVT_POKELIST_PARAM));
      param.p_message = LOCAL_TVT_PERSON_GetMessage( p_person );
      param.p_clunit  = LOCAL_TVT_PERSON_GetClunit( p_person );
      param.p_party   = GAMEDATA_GetMyPokemon( p_wk->p_gamedata );

      p_wk->p_pokelist  = LOCAL_TVT_POKELIST_Init( &param, p_wk->heapID );
    }
    p_wk->seq++;
    break;
  case SEQ_MAIN:
    LOCAL_TVT_POKELIST_Main( p_wk->p_pokelist );
    switch( LOCAL_TVT_POKELIST_GetResult( p_wk->p_pokelist ) )
    {
    case LOCAL_TVT_POKELIST_RESULT_DECIDE:
      p_wk->seq = SEQ_END;
      break;
    case LOCAL_TVT_POKELIST_RESULT_CANCEL:
      p_wk->seq = SEQ_END;
    }
    break;
  case SEQ_END:
    LOCAL_TVT_POKELIST_Exit( p_wk->p_pokelist );
    p_wk->seq = 0;
    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  なかよしかおしえて
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SUPPORT_CheckFriend( LOCAL_TVT_PERSON_SYS *p_person, void *p_wk_adrs )
{
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  どりょくをおしえて
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SUPPORT_CheckEffort( LOCAL_TVT_PERSON_SYS *p_person, void *p_wk_adrs )
{
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  さよなら
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SUPPORT_Farewell( LOCAL_TVT_PERSON_SYS *p_person, void *p_wk_adrs )
{
  LOCAL_TVT_PERSON_End( p_person );
  return TRUE;
}

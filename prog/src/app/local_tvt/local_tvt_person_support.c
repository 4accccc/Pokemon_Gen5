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
static void LOCAL_TVT_SUPPORT_Exit( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );

static BOOL LOCAL_TVT_SUPPORT_Greeting( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );

static BOOL LOCAL_TVT_SUPPORT_Talk( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );
static BOOL LOCAL_TVT_SUPPORT_CheckFriend( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );
static BOOL LOCAL_TVT_SUPPORT_CheckEffort( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );
static BOOL LOCAL_TVT_SUPPORT_Farewell( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *	@brief  パーソンワークを設定
 *
 *	@param	LOCAL_TVT_PERSON *p_person  パーソンワーク
 *	@param  GAMEDATA  ゲームデータ
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_SUPPORT_SetPerson( LOCAL_TVT_PERSON *p_person, const GFL_MSGDATA *cp_msg, GAMEDATA *p_gamedata, HEAPID heapID )
{
  GFL_STD_MemClear( p_person, sizeof(LOCAL_TVT_PERSON));
  p_person->p_wk_adrs     = LOCAL_TVT_SUPPORT_Init( p_gamedata, heapID );
  p_person->ExitFunction  = LOCAL_TVT_SUPPORT_Exit;
  p_person->greetingFunction  = LOCAL_TVT_SUPPORT_Greeting;
  p_person->selectFunction[0] = LOCAL_TVT_SUPPORT_Talk;
  p_person->list_msgID[0]     = CALL_BELL_01;
  p_person->selectFunction[1] = LOCAL_TVT_SUPPORT_CheckFriend;
  p_person->list_msgID[1]     = CALL_BELL_02;
  p_person->selectFunction[2] = LOCAL_TVT_SUPPORT_CheckEffort;
  p_person->list_msgID[2]     = CALL_BELL_03;
  p_person->selectFunction[3] = LOCAL_TVT_SUPPORT_Farewell;
  p_person->list_msgID[3]     = CALL_BELL_04;
  p_person->list_num  = 4;
  p_person->list_start_msgID  = CALL_BELL_SELECT_01;
  p_person->cp_list_msg_data  = cp_msg;
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
 *	@brief  ベル用ワークを破棄
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param  const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_SUPPORT_Exit( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  LOCAL_TVT_SUPPORT *p_wk       = p_wk_adrs;
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  挨拶
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param  const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SUPPORT_Greeting( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_SUPPORT *p_wk       = p_wk_adrs;
  LOCAL_TVT_CHARA* p_bell = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      u16 msgID = LOCAL_TVT_CALLDATA_GetGreetingMsgID( cp_param->p_data, LOCAL_TVT_PERSON_BELL );
      LOCAL_TVT_CHARA_StartTalk( p_bell, cp_msg, msgID );
    }
    p_wk->seq++;
    break;

  case SEQ_MAIN:
    if( LOCAL_TVT_CHARA_WaitTalk( p_bell, cp_param->p_chara_sys ) )
    {
      p_wk->seq++;
    }
    break;

  case SEQ_END:
    p_wk->seq = 0;
    return TRUE;

  default:
    GF_ASSERT(0);
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  はなす
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SUPPORT_Talk( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_SUPPORT *p_wk       = p_wk_adrs;
  LOCAL_TVT_CHARA* p_bell = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      u16 msgID = LOCAL_TVT_CALLDATA_GetTalkMsgID( cp_param->p_data, LOCAL_TVT_PERSON_BELL );
      LOCAL_TVT_CHARA_StartTalk( p_bell, cp_msg, msgID );
    }
    p_wk->seq++;
    break;

  case SEQ_MAIN:
    if( LOCAL_TVT_CHARA_WaitTalk( p_bell, cp_param->p_chara_sys ) )
    {
      p_wk->seq++;
    }
    break;

  case SEQ_END:
    p_wk->seq = 0;
    return TRUE;

  default:
    GF_ASSERT(0);
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  なかよしかおしえて
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SUPPORT_CheckFriend( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_POKE_INIT,
    SEQ_POKE_MAIN,
    SEQ_POKE_END,
    SEQ_RESULT_INIT,
    SEQ_RESULT_WAIT,
  };
  LOCAL_TVT_SUPPORT *p_wk = p_wk_adrs;
  LOCAL_TVT_CHARA* p_bell = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_MSG_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      LOCAL_TVT_CHARA_StartTalk( p_bell, cp_msg, CALL_BELL_NATSUKI_00 );
    }
    p_wk->seq++;
    break;
 case SEQ_MSG_WAIT:
    if( LOCAL_TVT_CHARA_WaitTalk( p_bell, cp_param->p_chara_sys ) )
    {
      p_wk->seq++;
    }
    break;
  case SEQ_POKE_INIT:
    if( p_wk->p_pokelist != NULL )
    {
      //作成してるなら再スタート
      LOCAL_TVT_POKELIST_SetActive( p_wk->p_pokelist, TRUE );
      LOCAL_TVT_POKELIST_ReStart( p_wk->p_pokelist );
    }
    else 
    {
      //作成してないなら作成
      LOCAL_TVT_POKELIST_PARAM  param;
      GFL_STD_MemClear( &param, sizeof(LOCAL_TVT_POKELIST_PARAM));
      param.p_message = cp_param->p_message;
      param.p_clunit  = cp_param->p_clunit;
      param.p_party   = GAMEDATA_GetMyPokemon( p_wk->p_gamedata );

      p_wk->p_pokelist  = LOCAL_TVT_POKELIST_Init( &param, p_wk->heapID );
    }
    p_wk->seq++;
    break;
  case SEQ_POKE_MAIN:
    LOCAL_TVT_POKELIST_Main( p_wk->p_pokelist );
    switch( LOCAL_TVT_POKELIST_GetResult( p_wk->p_pokelist ) )
    {
    case LOCAL_TVT_POKELIST_RESULT_DECIDE:
      LOCAL_TVT_POKELIST_SetActive( p_wk->p_pokelist, FALSE );
      p_wk->seq = SEQ_RESULT_INIT;
      break;
    case LOCAL_TVT_POKELIST_RESULT_CANCEL:
      p_wk->seq = SEQ_POKE_END;
    }
    break;
  case SEQ_POKE_END:
    LOCAL_TVT_POKELIST_Exit( p_wk->p_pokelist );
    p_wk->p_pokelist  = NULL;
    p_wk->seq = 0;
    return TRUE;
  case SEQ_RESULT_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      const POKEMON_PARAM *cp_pp = LOCAL_TVT_POKELIST_GetSelect( p_wk->p_pokelist );
      const u32 natsuki = PP_Get( cp_pp, ID_PARA_friend, NULL );
      u16 msgID;

      //なつきにより文章変化
      if( natsuki == 0 )
      {
        msgID = CALL_BELL_NATSUKI_01;
      }
      else if( 1 <= natsuki && natsuki <= 49 )
      {
        msgID = CALL_BELL_NATSUKI_02;
      }
      else if( 50 <= natsuki && natsuki <= 99 )
      {
        msgID = CALL_BELL_NATSUKI_03;
      }
      else if( 100 <= natsuki && natsuki <= 149 )
      {
        msgID = CALL_BELL_NATSUKI_04;
      }
      else if( 150 <= natsuki && natsuki <= 199 )
      {
        msgID = CALL_BELL_NATSUKI_05;
      }
      else if( 200 <= natsuki && natsuki <= 254 )
      {
        msgID = CALL_BELL_NATSUKI_06;
      }
      else if( natsuki == PTL_FRIEND_MAX )
      {
        msgID = CALL_BELL_NATSUKI_07;
      }
      else
      {
        GF_ASSERT(0);
        msgID = CALL_BELL_NATSUKI_07;
      }

      LOCAL_TVT_MESSAGE_RegisterPlayerName( cp_param->p_message, p_wk->p_gamedata );
      LOCAL_TVT_MESSAGE_RegisterPokeNickName( cp_param->p_message, cp_pp, 1 );
      LOCAL_TVT_CHARA_StartTalk( p_bell, cp_msg, msgID );
      p_wk->seq++;
    }
    break;
  case SEQ_RESULT_WAIT:
    if( LOCAL_TVT_CHARA_WaitTalk( p_bell, cp_param->p_chara_sys ) )
    {
      p_wk->seq = SEQ_MSG_INIT;
    }
    break;
  default:
    GF_ASSERT( 0 );
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  どりょくをおしえて
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SUPPORT_CheckEffort( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_POKE_INIT,
    SEQ_POKE_MAIN,
    SEQ_POKE_END,
    SEQ_RESULT_ALL_INIT,
    SEQ_RESULT_ALL_WAIT,
    SEQ_RESULT_252_INIT,
    SEQ_RESULT_252_WAIT,
  };
  LOCAL_TVT_SUPPORT *p_wk = p_wk_adrs;
  LOCAL_TVT_CHARA* p_bell = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_MSG_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      LOCAL_TVT_CHARA_StartTalk( p_bell, cp_msg, CALL_BELL_DORYOKU_00 );
    }
    p_wk->seq++;
    break;
 case SEQ_MSG_WAIT:
    if( LOCAL_TVT_CHARA_WaitTalk( p_bell, cp_param->p_chara_sys ) )
    {
      p_wk->seq++;
    }
    break;
  case SEQ_POKE_INIT:
    if( p_wk->p_pokelist != NULL )
    {
      //作成してるなら再スタート
      LOCAL_TVT_POKELIST_SetActive( p_wk->p_pokelist, TRUE );
      LOCAL_TVT_POKELIST_ReStart( p_wk->p_pokelist );
    }
    else 
    {
      //作成してないなら作成
      LOCAL_TVT_POKELIST_PARAM  param;
      GFL_STD_MemClear( &param, sizeof(LOCAL_TVT_POKELIST_PARAM));
      param.p_message = cp_param->p_message;
      param.p_clunit  = cp_param->p_clunit;
      param.p_party   = GAMEDATA_GetMyPokemon( p_wk->p_gamedata );

      p_wk->p_pokelist  = LOCAL_TVT_POKELIST_Init( &param, p_wk->heapID );
    }
    p_wk->seq++;
    break;
  case SEQ_POKE_MAIN:
    LOCAL_TVT_POKELIST_Main( p_wk->p_pokelist );
    switch( LOCAL_TVT_POKELIST_GetResult( p_wk->p_pokelist ) )
    {
    case LOCAL_TVT_POKELIST_RESULT_DECIDE:
      LOCAL_TVT_POKELIST_SetActive( p_wk->p_pokelist, FALSE );
      p_wk->seq = SEQ_RESULT_ALL_INIT;
      break;
    case LOCAL_TVT_POKELIST_RESULT_CANCEL:
      p_wk->seq = SEQ_POKE_END;
    }
    break;
  case SEQ_POKE_END:
    LOCAL_TVT_POKELIST_Exit( p_wk->p_pokelist );
    p_wk->p_pokelist  = NULL;
    p_wk->seq = 0;
    return TRUE;
  case SEQ_RESULT_ALL_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      const POKEMON_PARAM *cp_pp = LOCAL_TVT_POKELIST_GetSelect( p_wk->p_pokelist );

      u32 msgID;
      {
        int i;
        u32 exp_max = 0;
        for( i = 0; i < PTL_ABILITY_MAX; i++ )
        {
          exp_max += PP_Get( cp_pp, ID_PARA_hp_exp+i, NULL );
        }
        if( exp_max == 510 )
        {
          msgID = CALL_BELL_DORYOKU_02;
        }
        else
        {
          msgID = CALL_BELL_DORYOKU_01;
        }
      }

      LOCAL_TVT_MESSAGE_RegisterPlayerName( cp_param->p_message, p_wk->p_gamedata );
      LOCAL_TVT_MESSAGE_RegisterPokeNickName( cp_param->p_message, cp_pp, 1 );
      LOCAL_TVT_CHARA_StartTalk( p_bell, cp_msg, msgID );
      p_wk->seq++;
    }
    break;
  case SEQ_RESULT_ALL_WAIT:
    if( LOCAL_TVT_CHARA_WaitTalk( p_bell, cp_param->p_chara_sys ) )
    {
      p_wk->seq = SEQ_RESULT_252_INIT;
    }
    break;
  case SEQ_RESULT_252_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      const POKEMON_PARAM *cp_pp = LOCAL_TVT_POKELIST_GetSelect( p_wk->p_pokelist );

      u32 msgID;
      {
        int i;
        u32 max_num = 0;
        u32 exp_max = 0;
        for( i = 0; i < PTL_ABILITY_MAX; i++ )
        {
          const u32 exp   = PP_Get( cp_pp, ID_PARA_hp_exp+i, NULL );
          const u32 msgID = CALL_BELL_DORYOKU_05+i;
          exp_max += exp;
          if( exp >= 252 )
          {
            LOCAL_TVT_MESSAGE_RegisterWord( cp_param->p_message, cp_msg, msgID, max_num );
            max_num++;
          }
        }
        if( max_num == 1 )
        {
          if( exp_max == 510 )
          {
            msgID = CALL_BELL_DORYOKU_03;
          }
          else
          {
            msgID = CALL_BELL_DORYOKU_11;
          }
        }
        else if( max_num == 2 )
        {
          if( exp_max == 510 )
          {
            msgID = CALL_BELL_DORYOKU_04;
          }
          else
          {
            msgID = CALL_BELL_DORYOKU_12;
          }
        }
        else
        {
          p_wk->seq = SEQ_MSG_INIT;
          break;
        }
      }
      LOCAL_TVT_CHARA_StartTalk( p_bell, cp_msg, msgID );
      p_wk->seq++;
    }
    break;
  case SEQ_RESULT_252_WAIT:
    if( LOCAL_TVT_CHARA_WaitTalk( p_bell, cp_param->p_chara_sys ) )
    {
      p_wk->seq = SEQ_MSG_INIT;
    }
    break;
  default:
    GF_ASSERT( 0 );
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  さよなら
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SUPPORT_Farewell( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_SUPPORT *p_wk       = p_wk_adrs;
  LOCAL_TVT_CHARA* p_bell = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      u16 msgID = LOCAL_TVT_CALLDATA_GetFarewellMsgID( cp_param->p_data, LOCAL_TVT_PERSON_BELL );
      LOCAL_TVT_CHARA_StartTalk( p_bell, cp_msg, msgID );
    }
    p_wk->seq++;
    break;

  case SEQ_MAIN:
    if( LOCAL_TVT_CHARA_WaitTalk( p_bell, cp_param->p_chara_sys ) )
    {
      p_wk->seq++;
    }
    break;

  case SEQ_END:
    p_wk->seq = 0;
    LOCAL_TVT_PERSON_End( p_person );
    return TRUE;

  default:
    GF_ASSERT(0);
  }

  return FALSE;
}

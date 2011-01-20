//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_person_chelen.c
 *	@brief  各人物処理：チェレン
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
#include "poke_tool/poketype_def.h"
#include "poke_tool/poketype.h"

//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "msg/msg_tvt_type.h"
#include "msg/msg_tvt_tokusei.h"

//自分のモジュール
#include "local_tvt_local_def.h"
#include "local_tvt_graphic.h"
#include "local_tvt_util.h"

//外部公開
#include "local_tvt_person_chelen.h"

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
///	チェレン用ワーク
//=====================================
typedef struct 
{
  s32 seq;
  LOCAL_TVT_POKELIST *p_pokelist;
  GAMEDATA            *p_gamedata;
  HEAPID              heapID;
  GFL_MSGDATA         *p_msgdata_type;
  GFL_MSGDATA         *p_msgdata_tokusei;
} LOCAL_TVT_CHELEN;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static LOCAL_TVT_CHELEN *LOCAL_TVT_CHELEN_Init( GAMEDATA *p_gamedata, HEAPID heapID );
static void LOCAL_TVT_CHELEN_Exit( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );

static BOOL LOCAL_TVT_CHELEN_Greeting( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );

static BOOL LOCAL_TVT_CHELEN_Talk( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );
static BOOL LOCAL_TVT_CHELEN_CheckType( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );
static BOOL LOCAL_TVT_CHELEN_CheckTokusei( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );
static BOOL LOCAL_TVT_CHELEN_Farewell( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );

static u16 Type_GetMsgID( u16 type1, u16 type2 );
//----------------------------------------------------------------------------
/**
 *	@brief  パーソンワークを設定
 *
 *	@param	LOCAL_TVT_PERSON *p_person  パーソンワーク
 *	@param  GAMEDATA  ゲームデータ
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CHELEN_SetPerson( LOCAL_TVT_PERSON *p_person, const GFL_MSGDATA *cp_msg, GAMEDATA *p_gamedata, HEAPID heapID )
{
  GFL_STD_MemClear( p_person, sizeof(LOCAL_TVT_PERSON));
  p_person->p_wk_adrs     = LOCAL_TVT_CHELEN_Init( p_gamedata, heapID );
  p_person->ExitFunction  = LOCAL_TVT_CHELEN_Exit;
  p_person->greetingFunction  = LOCAL_TVT_CHELEN_Greeting;
  p_person->selectFunction[0] = LOCAL_TVT_CHELEN_Talk;
  p_person->list_msgID[0]     = CALL_CHELEN_01;
  p_person->selectFunction[1] = LOCAL_TVT_CHELEN_CheckType;
  p_person->list_msgID[1]     = CALL_CHELEN_02;
  p_person->selectFunction[2] = LOCAL_TVT_CHELEN_CheckTokusei;
  p_person->list_msgID[2]     = CALL_CHELEN_03;
  p_person->selectFunction[3] = LOCAL_TVT_CHELEN_Farewell;
  p_person->list_msgID[3]     = CALL_CHELEN_04;
  p_person->list_num  = 4;
  p_person->cp_list_msg_data = cp_msg;
}

//=============================================================================
/**
 *					プライベート
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  チェレン用ワークを作成
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static LOCAL_TVT_CHELEN *LOCAL_TVT_CHELEN_Init( GAMEDATA *p_gamedata, HEAPID heapID )
{
  LOCAL_TVT_CHELEN *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_CHELEN));
  p_wk->heapID      = heapID;
  p_wk->p_gamedata  = p_gamedata;

  p_wk->p_msgdata_type    = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE , NARC_message_tvt_type_dat, heapID );
  p_wk->p_msgdata_tokusei = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE , NARC_message_tvt_tokusei_dat, heapID );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  チェレン用ワークを破棄
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param  const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_CHELEN_Exit( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  LOCAL_TVT_CHELEN *p_wk       = p_wk_adrs;
  GFL_MSG_Delete( p_wk->p_msgdata_tokusei );
  GFL_MSG_Delete( p_wk->p_msgdata_type );
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
static BOOL LOCAL_TVT_CHELEN_Greeting( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_CHELEN *p_wk       = p_wk_adrs;
  LOCAL_TVT_CHARA* p_bell = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      u16 msgID = LOCAL_TVT_CALLDATA_GetGreetingMsgID( cp_param->p_data, LOCAL_TVT_PERSON_CHELEN );
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
static BOOL LOCAL_TVT_CHELEN_Talk( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_CHELEN *p_wk       = p_wk_adrs;
  LOCAL_TVT_CHARA* p_bell = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      u16 msgID = LOCAL_TVT_CALLDATA_GetTalkMsgID( cp_param->p_data, LOCAL_TVT_PERSON_CHELEN );
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
 *	@brief  タイプをおしえて
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_CHELEN_CheckType( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
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
  LOCAL_TVT_CHELEN *p_wk = p_wk_adrs;
  LOCAL_TVT_CHARA* p_bell = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_MSG_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      LOCAL_TVT_CHARA_StartTalk( p_bell, cp_msg, CALL_CHELEN_TYPE_01 );
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
      const POKEMON_PARAM *cp_pp = LOCAL_TVT_POKELIST_GetSelect( p_wk->p_pokelist );
      u16 msgID = Type_GetMsgID( PP_Get( cp_pp, ID_PARA_type1, NULL ), PP_Get( cp_pp, ID_PARA_type2, NULL ) );

      LOCAL_TVT_MESSAGE_RegisterPlayerName( cp_param->p_message, p_wk->p_gamedata );
      LOCAL_TVT_MESSAGE_RegisterPokeNickName( cp_param->p_message, cp_pp, 1 );
      LOCAL_TVT_CHARA_StartTalk( p_bell, p_wk->p_msgdata_type, msgID );
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
 *	@brief  特性をおしえて
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_CHELEN_CheckTokusei( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
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
  LOCAL_TVT_CHELEN *p_wk = p_wk_adrs;
  LOCAL_TVT_CHARA* p_bell = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_MSG_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      LOCAL_TVT_CHARA_StartTalk( p_bell, cp_msg, CALL_CHELEN_TOKUSEI_01 );
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
      const POKEMON_PARAM *cp_pp = LOCAL_TVT_POKELIST_GetSelect( p_wk->p_pokelist );
      u16 msgID = TOKUSEI_STR_000 + PP_Get( cp_pp, ID_PARA_speabino, NULL );

      LOCAL_TVT_MESSAGE_RegisterPlayerName( cp_param->p_message, p_wk->p_gamedata );
      LOCAL_TVT_MESSAGE_RegisterPokeNickName( cp_param->p_message, cp_pp, 1 );
      LOCAL_TVT_CHARA_StartTalk( p_bell, p_wk->p_msgdata_tokusei, msgID );
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
 *	@brief  さよなら
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_CHELEN_Farewell( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_CHELEN *p_wk       = p_wk_adrs;
  LOCAL_TVT_CHARA* p_bell = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      u16 msgID = LOCAL_TVT_CALLDATA_GetFarewellMsgID( cp_param->p_data, LOCAL_TVT_PERSON_CHELEN );
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


//----------------------------------------------------------------------------
/**
 *	@brief  タイプからメッセージを取得
 *
 *	@param	type1 タイプ１
 *	@param	type2 タイプ２
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static u16 Type_GetMsgID( u16 type1, u16 type2 )
{
  int i = 0;
  int j = 0;
  int cnt = 0;

  if( type1 > type2 )
  {
    u32 tmp = type1;
    type1 = type2;
    type2 = tmp;
  }
  for( i = 0; i < type1; i++ )
  {
    cnt += (POKETYPE_MAX-i);
  }
  cnt += type2-i;

  OS_TPrintf( "type=%d %d cnt=%d \n", type1, type2, cnt );

  return TYPE_STR_001 + cnt;
}

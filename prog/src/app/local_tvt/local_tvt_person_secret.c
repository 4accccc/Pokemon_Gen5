//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_person_secret.c
 *	@brief  各人物処理：隠しキャラ
 *	@author	Toru=Nagihashi
 *	@data		2011.01.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "gamesystem/game_data.h"
#include "savedata/questionnaire_save.h"
#include "savedata/questionnaire_index.h"
#include "app/research_radar/question_id.h"
#include "app/research_radar/answer_id.h"

//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "msg/msg_typename.h"
#include "../resource/fldmapdata/flagwork/work_define.h"

//自分のモジュール
#include "local_tvt_local_def.h"
#include "local_tvt_graphic.h"
#include "local_tvt_util.h"
#include "local_tvt_secret_data.h"

//外部公開
#include "local_tvt_person_secret.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//女性の場合メッセージIDに足す数
#define SECRET_MSGID_FEMALE_ADD   (CALL_SECRET_GREET_FR_01-CALL_SECRET_GREET_R_01)

//主人公からかけた場合IDに足す数
#define SECRET_MSGID_RECIVE_ADD   (CALL_SECRET_GREET_R_04-CALL_SECRET_GREET_R_01)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	隠しキャラ用ワーク
//=====================================
typedef struct 
{
  s32   seq;
  s32   sub_seq;
  BOOL  is_talk;
  u32   talk_msgID;
  LOCAL_TVT_POKELIST        *p_pokelist;
  GAMEDATA                  *p_gamedata;
  LOCAL_TVT_SECRET_MODE     mode;
  HEAPID                    heapID;
  const QUESTIONNAIRE_ANSWER_WORK *cp_quest; //アンケート質問
} LOCAL_TVT_SECRET;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static LOCAL_TVT_SECRET *LOCAL_TVT_SECRET_Init( GAMEDATA *p_gamedata, LOCAL_TVT_SECRET_MODE mode, HEAPID heapID );
static void LOCAL_TVT_SECRET_Exit( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );

static BOOL LOCAL_TVT_SECRET_Greeting( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );
static BOOL LOCAL_TVT_SECRET_Talk( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );
static BOOL LOCAL_TVT_SECRET_Farewell( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );
static BOOL LOCAL_TVT_SECRET_TrueFace( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );
static BOOL LOCAL_TVT_SECRET_ForceTalk( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );

static BOOL Local_Tvt_Secret_GreetingInner( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, LOCAL_TVT_SECRET *p_wk, s32 *p_seq );
static BOOL Local_Tvt_Secret_TalkInner( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, LOCAL_TVT_SECRET *p_wk, s32 *p_seq );
static BOOL Local_Tvt_Secret_FarewellInner( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, LOCAL_TVT_SECRET *p_wk, s32 *p_seq );

//隠しキャラのデータ取得
static PERSON_STATE_SECRET_DATE Local_Tvt_Secret_GetDateEvent( const LOCAL_TVT_CALLDATA *cp_data );

static u32 Local_Tvt_Secret_GetTalkMsgID( const QUESTIONNAIRE_ANSWER_WORK *cp_quest, const LOCAL_TVT_CALLDATA *cp_data, LOCAL_TVT_MESSAGE *p_message );

static void Local_Tvt_Secret_SetWord( const LOCAL_TVT_CALLDATA *cp_data, LOCAL_TVT_MESSAGE *p_message, u32 state, u32 answerID, BOOL is_answer );

static u32 Local_Tvt_Secret_Qanswer_GetTalkMsgID( const LOCAL_TVT_CALLDATA *cp_data, u32 state, BOOL is_answer );
static u32 Local_Tvt_Secret_DailyRnd_GetTalkMsgID( const LOCAL_TVT_CALLDATA *cp_data, u32 state );

static void Local_Tvt_Secret_Qanswer_Check( const QUESTIONNAIRE_ANSWER_WORK *cp_quest, u8 state, u32 *p_answerID, BOOL *p_is_answer, BOOL *p_is_limit );

//----------------------------------------------------------------------------
/**
 *	@brief  パーソンワークを設定
 *
 *	@param	LOCAL_TVT_PERSON *p_person  パーソンワーク
 *	@param  GAMEDATA  ゲームデータ
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_SECRET_SetPerson( LOCAL_TVT_PERSON *p_person, const GFL_MSGDATA *cp_msg, GAMEDATA *p_gamedata, const LOCAL_TVT_CALLDATA *cp_data, LOCAL_TVT_SECRET_MODE mode, HEAPID heapID )
{
  GFL_STD_MemClear( p_person, sizeof(LOCAL_TVT_PERSON));
  p_person->p_wk_adrs     = LOCAL_TVT_SECRET_Init( p_gamedata, mode, heapID );
  p_person->ExitFunction  = LOCAL_TVT_SECRET_Exit;

  {
    u16 state = LOCAL_TVT_CALLDATA_GetState( cp_data, LOCAL_TVT_PERSON_SECRET );
    u32 index = LTVT_SECREAT_DATA_SearchIndex( state );
    LTVT_SECREAT_TALK talk = LTVT_SECREAT_DATA_GetTalkProg( index );
    u32 sex   = LOCAL_TVT_CALLDATA_GetSecretSex( cp_data );

    if( LTVT_SECREAT_TALK_RING_OFF == talk )
    {
      p_person->greetingFunction  = LOCAL_TVT_SECRET_TrueFace;
    }
    else if( LTVT_SECREAT_TALK_FORCE_TALK == talk
        || LTVT_SECREAT_TALK_QANSWER == talk)
    {
      p_person->greetingFunction  = LOCAL_TVT_SECRET_ForceTalk;
    }
    else
    {
      p_person->greetingFunction  = LOCAL_TVT_SECRET_Greeting;
      p_person->selectFunction[0] = LOCAL_TVT_SECRET_Talk;
      p_person->list_msgID[0]     = CALL_SECRET_01;
      p_person->selectFunction[1] = LOCAL_TVT_SECRET_Farewell;
      p_person->list_msgID[1]     = CALL_SECRET_02;
      p_person->list_num          = 2;
      p_person->list_start_msgID  = LOCAL_TVT_CALLDATA_GetSpecialMsgID( cp_data, LOCAL_TVT_PERSON_SECRET );
      //女性の場合は会話番号を足す
      if( sex == PTL_SEX_FEMALE )
      {
        p_person->list_start_msgID += SECRET_MSGID_FEMALE_ADD;
      }
      p_person->cp_list_msg_data  = cp_msg;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  顔出し設定
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_data 人物データ
 *
 *	@return 顔出しタイプ
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_CHARA_TYPE LOCAL_TVT_SECRET_GetFace( const LOCAL_TVT_CALLDATA *cp_data )
{
  u32 sex   = LOCAL_TVT_CALLDATA_GetSecretSex( cp_data );
  u16 state = LOCAL_TVT_CALLDATA_GetState( cp_data, LOCAL_TVT_PERSON_SECRET );
  u32 index = LTVT_SECREAT_DATA_SearchIndex( state );

  switch( LTVT_SECREAT_DATA_GetFace( index ) )
  {
  default:
    GF_ASSERT(0);
  case LTVT_SECREAT_FACE_SOUND:
    return LTCT_SOUNDONLY;
  case LTVT_SECREAT_FACE_MASK:
    if( sex == PM_MALE )
    {
      return LTCT_SECRET_M_MASK;
    }
    else
    {
      return LTCT_SECRET_F_MASK;
    }
  case LTVT_SECREAT_FACE_TRUE:
    if( sex == PM_MALE )
    {
      return LTCT_SECRET_M;
    }
    else
    {
      return LTCT_SECRET_F;
    }
  case LTVT_SECREAT_FACE_MASK2:
    if( sex == PM_MALE )
    {
      return LTCT_SECRET_M_MASK2;
    }
    else
    {
      return LTCT_SECRET_F_MASK2;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_data 
 *
 *	@return TRUE？？？  FALSE通常の名前
 */
//-----------------------------------------------------------------------------
u16 LOCAL_TVT_SECRET_GetDispName( const LOCAL_TVT_CALLDATA *cp_data )
{
  u32 sex   = LOCAL_TVT_CALLDATA_GetSecretSex( cp_data );
  if( LOCAL_TVT_CALLDATA_IsFirst( cp_data ) )
  {
    return LTCT_SOUNDONLY;
  }
  else
  {
    if( sex == PM_MALE )
    {
      return LTCT_SECRET_M_MASK;
    }
    else
    {
      return LTCT_SECRET_F_MASK;
    }
  }
}

//=============================================================================
/**
 *					プライベート
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラ用ワークを作成
 *
 *	@param	HEAPID heapID ヒープID
 *	@param  モード
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static LOCAL_TVT_SECRET *LOCAL_TVT_SECRET_Init( GAMEDATA *p_gamedata, LOCAL_TVT_SECRET_MODE mode, HEAPID heapID )
{
  LOCAL_TVT_SECRET *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_SECRET));
  p_wk->heapID      = heapID;
  p_wk->p_gamedata  = p_gamedata;
  {

    QUESTIONNAIRE_SAVE_WORK *p_q_sv = SaveData_GetQuestionnaire(GAMEDATA_GetSaveControlWork(p_wk->p_gamedata));
    p_wk->cp_quest     = Questionnaire_GetAnswerWork(p_q_sv);
  }
  p_wk->mode        = mode;
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラ用ワークを破棄
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param  const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_SECRET_Exit( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  LOCAL_TVT_SECRET *p_wk       = p_wk_adrs;
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
static BOOL LOCAL_TVT_SECRET_Greeting( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  LOCAL_TVT_SECRET *p_wk       = p_wk_adrs;

  return Local_Tvt_Secret_GreetingInner( p_person, cp_param, p_wk, &p_wk->seq );
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
static BOOL LOCAL_TVT_SECRET_Talk( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_TALK,
    SEQ_FAREWELL,
    SEQ_END,
  };
  LOCAL_TVT_SECRET *p_wk       = p_wk_adrs;

  switch( p_wk->seq )
  {
  case SEQ_TALK:
    if( Local_Tvt_Secret_TalkInner( p_person, cp_param, p_wk, &p_wk->sub_seq ) )
    {
      p_wk->seq++;
    }
    break;
    
  //隠しキャラは話すを選んだらそのままさよならの処理へいく
  case SEQ_FAREWELL:
    if( Local_Tvt_Secret_FarewellInner( p_person, cp_param, p_wk, &p_wk->sub_seq ) )
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
 *	@brief  さよなら
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SECRET_Farewell( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  LOCAL_TVT_SECRET *p_wk = p_wk_adrs;
  return Local_Tvt_Secret_FarewellInner( p_person, cp_param, p_wk, &p_wk->seq );
}

//----------------------------------------------------------------------------
/**
 *	@brief  挨拶
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL Local_Tvt_Secret_GreetingInner( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, LOCAL_TVT_SECRET *p_wk, s32 *p_seq )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_CHARA* p_secret = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( *p_seq )
  {
  case SEQ_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      u32 sex   = LOCAL_TVT_CALLDATA_GetSecretSex( cp_param->p_data );
      u32 msgID = LOCAL_TVT_CALLDATA_GetGreetingMsgID( cp_param->p_data, LOCAL_TVT_PERSON_SECRET );

#ifdef BUGFIX_SYSTEM160_20120421
      {
        GAMEDATA *p_gamedata = LOCAL_TVT_CALLDATA_GetGameData( cp_param->p_data );
        LOCAL_TVT_MESSAGE_RegisterPlayerName( cp_param->p_message, p_gamedata );
      }
#endif

#if 0 //かならず最初の１０回は相手からかけて、その後は自分しかかけないので
      //エクセルで管理することにした
      //かけてくる場合番号を足す
      if( p_wk->mode == LOCAL_TVT_SECRET_MODE_SEND )
      {
        msgID += SECRET_MSGID_RECIVE_ADD;
      }
#endif
      //女性の場合は会話番号を足す
      if( sex == PTL_SEX_FEMALE )
      {
        msgID += SECRET_MSGID_FEMALE_ADD;
      }
      LOCAL_TVT_CHARA_StartTalk( p_secret, cp_msg, msgID );
    }
    (*p_seq)++;
    break;

  case SEQ_MAIN:
    if( LOCAL_TVT_CHARA_WaitTalk( p_secret, cp_param->p_chara_sys ) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    (*p_seq)  = 0;
    return TRUE;

  default:
    GF_ASSERT(0);
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  話す
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL Local_Tvt_Secret_TalkInner( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, LOCAL_TVT_SECRET *p_wk, s32 *p_seq )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_CHARA* p_secret = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( *p_seq )
  {
  case SEQ_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      u32 sex   = LOCAL_TVT_CALLDATA_GetSecretSex( cp_param->p_data );
      u32 msgID;

      //デート終了してたら初期化
      LOCAL_TVT_CALLDATA_ResetSecretDate( cp_param->p_data );

      if( p_wk->is_talk == FALSE )
      {
        //何回も変わらないように最初の１回だけ受け取る
        p_wk->talk_msgID = Local_Tvt_Secret_GetTalkMsgID( p_wk->cp_quest, cp_param->p_data, cp_param->p_message );
      }
      msgID = p_wk->talk_msgID;

      //女性の場合は会話番号を足す
      if( sex == PTL_SEX_FEMALE )
      {
        msgID += SECRET_MSGID_FEMALE_ADD;
      }
      LOCAL_TVT_CHARA_StartTalk( p_secret, cp_msg, msgID );

      //話しかけた
      p_wk->is_talk = TRUE;
    }
    (*p_seq)++;
    break;

  case SEQ_MAIN:
    if( LOCAL_TVT_CHARA_WaitTalk( p_secret, cp_param->p_chara_sys ) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    (*p_seq)  = 0;
    return TRUE;

  default:
    GF_ASSERT(0);
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
static BOOL Local_Tvt_Secret_FarewellInner( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, LOCAL_TVT_SECRET *p_wk, s32 *p_seq )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_CHARA* p_secret = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( (*p_seq) )
  {
  case SEQ_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      u32 sex   = LOCAL_TVT_CALLDATA_GetSecretSex( cp_param->p_data );
      u16 msgID = LOCAL_TVT_CALLDATA_GetFarewellMsgID( cp_param->p_data, LOCAL_TVT_PERSON_SECRET );
      PERSON_STATE_SECRET_DATE date = Local_Tvt_Secret_GetDateEvent( cp_param->p_data );

      //デートに誘っていたら（会話していたら）
      //メッセージを帰る
      if( (date == PERSON_STATE_SECRET_DATE_FIRST || date == PERSON_STATE_SECRET_DATE_REGULAR ) && p_wk->is_talk )
      {
        msgID = CALL_SECRET_TALK_SP_01;
      }

      //女性の場合は会話番号を足す
      if( sex == PTL_SEX_FEMALE )
      {
        msgID += SECRET_MSGID_FEMALE_ADD;
      }
      LOCAL_TVT_CHARA_StartTalk( p_secret, cp_msg, msgID );
    }
    (*p_seq)++;
    break;

  case SEQ_MAIN:
    if( LOCAL_TVT_CHARA_WaitTalk( p_secret, cp_param->p_chara_sys ) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    (*p_seq) = 0;
  
    //話しかけた場合は状態変更
    if( p_wk->is_talk )
    {
      u16 state = LOCAL_TVT_CALLDATA_GetState( cp_param->p_data, LOCAL_TVT_PERSON_SECRET );
      u32 index = LTVT_SECREAT_DATA_SearchIndex( state );

      PERSON_STATE_SECRET_DATE  date = Local_Tvt_Secret_GetDateEvent( cp_param->p_data );

      //約束をしたならば、フラグを立てる
      if( date != PERSON_STATE_SECRET_DATE_NONE )
      {
        LOCAL_TVT_CALLDATA_SetSecretDateEvent( cp_param->p_data, date );
        if( date == PERSON_STATE_SECRET_DATE_FIRST
            || date == PERSON_STATE_SECRET_DATE_REGULAR )
        {
          //定期デートは１日１回デートON
          LOCAL_TVT_CALLDATA_SetSecretOnceDate( cp_param->p_data );
        }
      }

      //話しかけたので好感度アップ
      LOCAL_TVT_CALLDATA_AddState( cp_param->p_data, LOCAL_TVT_PERSON_SECRET, LOCAL_TVT_SECRET_PHASE_RANGE_MAX );
    }

    //隠しキャラ用ゾーンIDを再設定
    GAMEDATA_SetLiveCasterSecretCallZoneID( p_wk->p_gamedata );

    //終了
    LOCAL_TVT_PERSON_End( p_person );
    return TRUE;

  default:
    GF_ASSERT(0);
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  素顔ですぐきる
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SECRET_TrueFace( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_SECRET *p_wk       = p_wk_adrs;
  LOCAL_TVT_CHARA* p_secret = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      u32 sex   = LOCAL_TVT_CALLDATA_GetSecretSex( cp_param->p_data );
      u32 msgID = LOCAL_TVT_CALLDATA_GetTalkMsgID( cp_param->p_data, LOCAL_TVT_PERSON_SECRET );

      //女性の場合は会話番号を足す
      if( sex == PTL_SEX_FEMALE )
      {
        msgID += SECRET_MSGID_FEMALE_ADD;
      }
      LOCAL_TVT_CHARA_StartTalk( p_secret, cp_msg, msgID );
    }
    p_wk->seq++;
    break;

  case SEQ_MAIN:
    if( LOCAL_TVT_CHARA_WaitTalk( p_secret, cp_param->p_chara_sys ) )
    {
      p_wk->seq++;
    }
    break;

  case SEQ_END:
    p_wk->seq = 0;

    //話しかけたので好感度アップ
    LOCAL_TVT_CALLDATA_AddState( cp_param->p_data, LOCAL_TVT_PERSON_SECRET, LOCAL_TVT_SECRET_PHASE_RANGE_MAX );

    //終了
    LOCAL_TVT_PERSON_End( p_person );
    return TRUE;

  default:
    GF_ASSERT(0);
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  強制的に挨拶→話す→さよならになる
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SECRET_ForceTalk( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_GREET,
    SEQ_TALK,
    SEQ_FAREWELL,
    SEQ_END,
  };
  LOCAL_TVT_SECRET *p_wk       = p_wk_adrs;

  switch( p_wk->seq )
  {
  case SEQ_GREET:
    if( Local_Tvt_Secret_GreetingInner( p_person, cp_param, p_wk, &p_wk->sub_seq ) )
    {
      p_wk->seq++;
    }
    break;

  case SEQ_TALK:
    if( Local_Tvt_Secret_TalkInner( p_person, cp_param, p_wk, &p_wk->sub_seq ) )
    {
      p_wk->seq++;
    }
    break;
    
  case SEQ_FAREWELL:
    if( Local_Tvt_Secret_FarewellInner( p_person, cp_param, p_wk, &p_wk->sub_seq ) )
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
 *	@brief  現在のフラグ等を見て、デートイベントが発生する
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_data データ
 *
 *	@return 現在のイベント
 */
//-----------------------------------------------------------------------------
static PERSON_STATE_SECRET_DATE Local_Tvt_Secret_GetDateEvent( const LOCAL_TVT_CALLDATA *cp_data )
{
  u16 state = LOCAL_TVT_CALLDATA_GetState( cp_data, LOCAL_TVT_PERSON_SECRET );
  u32 index = LTVT_SECREAT_DATA_SearchIndex( state );
  PERSON_STATE_SECRET_DATE date = LTVT_SECREAT_DATA_GetDate( index );

  //１日１回デートフラグがたっていたらデートはなし！
  if( LOCAL_TVT_CALLDATA_GetSecretOnceDate( cp_data ) )
  {
    date = PERSON_STATE_SECRET_DATE_NONE;
  }

  return date;
}
//----------------------------------------------------------------------------
/**
 *	@brief  話すときのメッセージID取得
 *
 *	@param	const QUESTIONNAIRE_ANSWER_WORK *cp_quest アンケート
 *	@param	LOCAL_TVT_CALLDATA *cp_data   データ
 *
 *	@return MSGID(男性のもの)
 */
//-----------------------------------------------------------------------------
static u32 Local_Tvt_Secret_GetTalkMsgID( const QUESTIONNAIRE_ANSWER_WORK *cp_quest, const LOCAL_TVT_CALLDATA *cp_data, LOCAL_TVT_MESSAGE *p_message )
{
  BOOL is_answer;
  BOOL is_limit;
  u32 answerID;

  u32 sex   = LOCAL_TVT_CALLDATA_GetSecretSex( cp_data );
  u16 state = LOCAL_TVT_CALLDATA_GetState( cp_data, LOCAL_TVT_PERSON_SECRET );
  u32 index = LTVT_SECREAT_DATA_SearchIndex( state );
  LTVT_SECREAT_TALK talk = LTVT_SECREAT_DATA_GetTalkProg( index );
  PERSON_STATE_SECRET_DATE date = Local_Tvt_Secret_GetDateEvent( cp_data );

  //アンケートチェック
  Local_Tvt_Secret_Qanswer_Check( cp_quest, state, &answerID, &is_answer, &is_limit );

  //単語登録
  Local_Tvt_Secret_SetWord( cp_data, p_message, state, answerID, is_answer );

  //デートのときの会話
  switch( date )
  {
  case PERSON_STATE_SECRET_DATE_REGULAR:   //定期デート
    return CALL_SECRET_TALK_SP_00;
  }

  //デートじゃないときの会話
  switch( talk )
  {
  default:
    GF_ASSERT(0);
  case LTVT_SECREAT_TALK_RING_OFF:
  case LTVT_SECREAT_TALK_FORCE_TALK:
  case LTVT_SECREAT_TALK_NONE:
    return LOCAL_TVT_CALLDATA_GetTalkMsgID( cp_data, LOCAL_TVT_PERSON_SECRET );

  case LTVT_SECREAT_TALK_QANSWER:
    if( is_limit )
    {
      return Local_Tvt_Secret_Qanswer_GetTalkMsgID( cp_data, LOCAL_TVT_PERSON_SECRET, is_answer );
    }
    else
    {
      return LOCAL_TVT_CALLDATA_GetTalkMsgID( cp_data, LOCAL_TVT_PERSON_SECRET );
    }

  case LTVT_SECREAT_TALK_DAILY_RND:     //日常会話ランダム
    return Local_Tvt_Secret_DailyRnd_GetTalkMsgID( cp_data, state );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  単語登録
 *
 *	@param	const QUESTIONNAIRE_ANSWER_WORK *cp_quest アンケート
 *	@param	LOCAL_TVT_CALLDATA *cp_data データ
 *	@param	*p_message  メッセージ
 *	@param	state   状態
 *	@param  答えID
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_Secret_SetWord( const LOCAL_TVT_CALLDATA *cp_data, LOCAL_TVT_MESSAGE *p_message, u32 state, u32 answerID, BOOL is_answer )
{
  GAMEDATA *p_gamedata = LOCAL_TVT_CALLDATA_GetGameData( cp_data );

  //名前を単語登録
  LOCAL_TVT_MESSAGE_RegisterPlayerName( p_message, p_gamedata );

  //１段階目は最初の８つはプレイヤーのアンケートに答える
  if( state == ZERO_ORIGIN(9) )
  {
    //バッジの数を聞く
    u32 badge = LOCAL_TVT_CALLDATA_GetBadgeCnt( cp_data );
    LOCAL_TVT_MESSAGE_RegisterNumber( p_message, badge, 1, 1 );
  }
  else if( state == ZERO_ORIGIN(22) )
  {
    static const u32 sc_type_msgID[]  =
    {
      TYPENAME_012,//くさ
      TYPENAME_010,//ほのお
      TYPENAME_011,//みず
    };

    EVENTWORK *p_ev = GAMEDATA_GetEventWork(p_gamedata);
    const u16 *cp_evwk = EVENTWORK_GetEventWorkAdrs( p_ev, WK_SYS_FIRST_POKETYPE );
    GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE , NARC_message_typename_dat, GFL_HEAP_LOWID( HEAPID_LOCAL_TVT ) );
    LOCAL_TVT_MESSAGE_RegisterWord( p_message, p_msg, sc_type_msgID[ *cp_evwk ], 1 );
    GFL_MSG_Delete( p_msg );
  }
  else if( state < LOCAL_TVT_SECRET_PHASE_1_END )
  {
    if( is_answer )
    {
      LOCAL_TVT_MESSAGE_RegisterQuestionnaire( p_message, 1, answerID );
    }
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  アンケートメッセージ
 *
 *	@param	const QUESTIONNAIRE_ANSWER_WORK *cp_quest アンケート
 *	@param	LOCAL_TVT_CALLDATA *cp_data データ
 *	@param	state 状態
 *
 *	@return メッセージ
 */
//-----------------------------------------------------------------------------
static u32 Local_Tvt_Secret_Qanswer_GetTalkMsgID( const LOCAL_TVT_CALLDATA *cp_data, u32 state, BOOL is_answer )
{
  u16 msgID = LOCAL_TVT_CALLDATA_GetTalkMsgID( cp_data, LOCAL_TVT_PERSON_SECRET );

  if( is_answer == FALSE )
  {
    //答えていなければ、１こ次に聞いてない時用の会話があるので次のメッセージIDにする
    ++msgID;
  }

  return msgID;
}

//----------------------------------------------------------------------------
/**
 *	@brief  日常ランダムメッセージ
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_data データ
 *	@param	state   状態
 *
 *	@return メッセージ
 */
//-----------------------------------------------------------------------------
static u32 Local_Tvt_Secret_DailyRnd_GetTalkMsgID( const LOCAL_TVT_CALLDATA *cp_data, u32 state )
{
  if( GFUser_GetPublicRand( 2 ) )
  {
    return CALL_SECRET_TALK_31 + GFUser_GetPublicRand( 9 );
  }
  else
  {
    return CALL_SECRET_TALK_41 + GFUser_GetPublicRand( 9 );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アンケートをチェック
 *
 *	@param	const QUESTIONNAIRE_ANSWER_WORK *cp_quest アンケート
 *	@param	state 状態
 *	@param	*p_answerID   答えID
 *	@param	*p_is_answer  アンケートに答えているか
 *	@param	*p_is_limit   状態がアンケート範囲内か
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_Secret_Qanswer_Check( const QUESTIONNAIRE_ANSWER_WORK *cp_quest, u8 state, u32 *p_answerID, BOOL *p_is_answer, BOOL *p_is_limit )
{
  static const int sc_quest_tbl[] =
  {
    QUESTION_ID_SEX,
    QUESTION_ID_HOBBY,
    QUESTION_ID_POKEMON_TYPE,
    QUESTION_ID_MUSIC,
    QUESTION_ID_FAVORITE_POKEMON,
    QUESTION_ID_DAY_OFF,
    QUESTION_ID_FUN,
    QUESTION_ID_PERSON,
  };

  if( state >= NELEMS(sc_quest_tbl) )
  {
    *p_is_limit = FALSE;
    return;
  }
  else
  {
    u32 ansIdx  = QuestionnaireAnswer_ReadBit( cp_quest, sc_quest_tbl[state] );
    *p_is_limit = TRUE;
    if( ansIdx == 0 )
    {
      *p_answerID   = 0;
      *p_is_answer  = FALSE;
    }
    else
    {
      *p_answerID   = QIDATA_Get_AnswerID( sc_quest_tbl[state], ansIdx-1 );
      *p_is_answer  = TRUE;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  呼び出し音回数
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_data データ
 *
 *	@return SE回数
 */
//-----------------------------------------------------------------------------
u32 LOCAL_TVT_SECRET_GetCallSECnt( const LOCAL_TVT_CALLDATA *cp_data )
{
  u16 state = LOCAL_TVT_CALLDATA_GetState( cp_data, LOCAL_TVT_PERSON_SECRET );
  u32 index = LTVT_SECREAT_DATA_SearchIndex( state );
  return LTVT_SECREAT_DATA_GetCallSECnt( index );
}

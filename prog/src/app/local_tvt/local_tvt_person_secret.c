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
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "gamesystem/game_data.h"
#include "savedata/questionnaire_save.h"
#include "app/research_radar/question_id.h"
#include "app/research_radar/answer_id.h"

//アーカイブ
#include "arc_def.h"

//自分のモジュール
#include "local_tvt_local_def.h"
#include "local_tvt_graphic.h"
#include "local_tvt_util.h"

//外部公開
#include "local_tvt_person_secret.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//隠しキャラの状態定義
typedef enum
{
  LOCAL_TVT_SECRET_STATE_START    = 0,  
  LOCAL_TVT_SECRET_STATE_1        = 9,    //START～音声のみの会話　個人情報を聞く
  LOCAL_TVT_SECRET_STATE_1_EVENT  = 10,    //START～音声のみの会話　受け渡しイベント
  LOCAL_TVT_SECRET_STATE_2        = 20,   //1～顔の表示ONの会話 個人情報を教えてくれる
  LOCAL_TVT_SECRET_STATE_3        = 29,   //2～顔の表示ONの会話 仕事の悩み相談
  LOCAL_TVT_SECRET_STATE_3_EVENT  = 30,   //2～顔の表示ONの会話 デートイベント
  LOCAL_TVT_SECRET_STATE_4        = 39,   //3～顔の表示ONの会話 日常会話（ランダム）
  LOCAL_TVT_SECRET_STATE_4_EVENT  = 40,   //3～音声のみの会話 音声のみの応答をする
  LOCAL_TVT_SECRET_STATE_5        = 49,   //4～顔の表示ONの会話 state_4と同じ
  LOCAL_TVT_SECRET_STATE_5_EVENT  = 50,   //4～素顔でライブキャスター
}LOCAL_TVT_SECRET_STATE;

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
  s32 seq;
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

//----------------------------------------------------------------------------
/**
 *	@brief  パーソンワークを設定
 *
 *	@param	LOCAL_TVT_PERSON *p_person  パーソンワーク
 *	@param  GAMEDATA  ゲームデータ
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_SECRET_SetPerson( LOCAL_TVT_PERSON *p_person, const GFL_MSGDATA *cp_msg, GAMEDATA *p_gamedata, LOCAL_TVT_SECRET_MODE mode,HEAPID heapID )
{
  GFL_STD_MemClear( p_person, sizeof(LOCAL_TVT_PERSON));
  p_person->p_wk_adrs     = LOCAL_TVT_SECRET_Init( p_gamedata, mode, heapID );
  p_person->ExitFunction  = LOCAL_TVT_SECRET_Exit;
  p_person->greetingFunction  = LOCAL_TVT_SECRET_Greeting;
  p_person->selectFunction[0] = LOCAL_TVT_SECRET_Talk;
  p_person->list_msgID[0]     = CALL_SECRET_01;
  p_person->selectFunction[1] = LOCAL_TVT_SECRET_Farewell;
  p_person->list_msgID[1]     = CALL_SECRET_02;
  p_person->list_num          = 2;
  p_person->list_start_msgID  = CALL_SECRET_SELECT_01;
  p_person->cp_list_msg_data  = cp_msg;
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
LOCAL_TVT_SECRET_FACE LOCAL_TVT_SECRET_GetFace( const LOCAL_TVT_CALLDATA *cp_data )
{
  u16 state = LOCAL_TVT_CALLDATA_GetState( cp_data, LOCAL_TVT_PERSON_SECRET );
  u32 sex   = LOCAL_TVT_CALLDATA_GetSecretSex( cp_data );
  if( state <= LOCAL_TVT_SECRET_STATE_1 )
  {
    return LOCAL_TVT_SECRET_FACE_INVISIBLE;
  }
  else if( state == LOCAL_TVT_SECRET_STATE_5_EVENT )
  {
    if( sex == PTL_SEX_MALE )
    {
      return LOCAL_TVT_SECRET_FACE_VISIBLE_M;
    }
    else
    {
      return LOCAL_TVT_SECRET_FACE_VISIBLE_F;
    }
  }
  else
  {
    if( sex == PTL_SEX_MALE )
    {
      return LOCAL_TVT_SECRET_FACE_HIDE_M;
    }
    else
    {
      return LOCAL_TVT_SECRET_FACE_HIDE_F;
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
      u32 msgID = LOCAL_TVT_CALLDATA_GetGreetingMsgID( cp_param->p_data, LOCAL_TVT_PERSON_SECRET );
      //かけてくる場合番号を足す
      if( p_wk->mode == LOCAL_TVT_SECRET_MODE_SEND )
      {
        msgID += SECRET_MSGID_RECIVE_ADD;
      }
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
static BOOL LOCAL_TVT_SECRET_Talk( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
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
      u16 msgID = LOCAL_TVT_CALLDATA_GetTalkMsgID( cp_param->p_data, LOCAL_TVT_PERSON_SECRET );
      u32 sex   = LOCAL_TVT_CALLDATA_GetSecretSex( cp_param->p_data );
      u16 state = LOCAL_TVT_CALLDATA_GetState( cp_param->p_data, LOCAL_TVT_PERSON_SECRET );
      if( (LOCAL_TVT_SECRET_STATE_3_EVENT < state && state <= LOCAL_TVT_SECRET_STATE_4)
          || (LOCAL_TVT_SECRET_STATE_4_EVENT < state && state <= LOCAL_TVT_SECRET_STATE_5) )
      {
        //第３段階のときは会話はランダム
        if( GFUser_GetPublicRand( 1 ) )
        {
          msgID = CALL_SECRET_TALK_31 + GFUser_GetPublicRand( 9 );
        }
        else
        {
          msgID = CALL_SECRET_TALK_41 + GFUser_GetPublicRand( 9 );
        }
      }
      else if( CALL_SECRET_TALK_01 <= msgID && msgID <= CALL_SECRET_TALK_08 )
      {
        static const int sc_quest_tbl[] =
        {
          QUESTION_ID_HOBBY,
          QUESTION_ID_STRONG_CLASS,
          QUESTION_ID_COLOR,
          QUESTION_ID_SPORTS,
          QUESTION_ID_FOOD,
          QUESTION_ID_WEAK_CLASS,
          QUESTION_ID_PLACE,
          QUESTION_ID_FAVORITE_POKEMON,
        };
        u32 answerID  = QuestionnaireAnswer_ReadBit( p_wk->cp_quest, sc_quest_tbl[state] );
        GF_ASSERT( state < NELEMS(sc_quest_tbl) );
        //１～８問目まではアンケートの結果を話す
        if( ANSWER_ID_000 == answerID )
        {
          msgID++;
        }
        else
        {
          LOCAL_TVT_MESSAGE_RegisterQuestionnaire( cp_param->p_message, 0, answerID );
        }
      }

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
      u16 msgID = LOCAL_TVT_CALLDATA_GetFarewellMsgID( cp_param->p_data, LOCAL_TVT_PERSON_SECRET );
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
    //電話をかけたので足す１
    LOCAL_TVT_CALLDATA_AddState( cp_param->p_data, LOCAL_TVT_PERSON_SECRET, LOCAL_TVT_SECRET_STATE_5_EVENT );
    //終了
    LOCAL_TVT_PERSON_End( p_person );
    return TRUE;

  default:
    GF_ASSERT(0);
  }

  return FALSE;
}

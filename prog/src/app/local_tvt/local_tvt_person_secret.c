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
#include "savedata/questionnaire_index.h"
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
  LOCAL_TVT_SECRET_PHASE_1, //音声のみの会話　個人情報を聞く
  LOCAL_TVT_SECRET_PHASE_2, //顔の表示ONの会話 個人情報を教えてくれる
  LOCAL_TVT_SECRET_PHASE_3, //顔の表示ONの会話 仕事の悩み相談
  LOCAL_TVT_SECRET_PHASE_4, //顔の表示ONの会話 日常会話（ランダム）
  LOCAL_TVT_SECRET_PHASE_MAX,
}LOCAL_TVT_SECRET_PHASE;

//状態の範囲 Local_Tvt_Secret_GetPhaseに追加すること
typedef enum
{
  LOCAL_TVT_SECRET_PHASE_RANGE_1_START  = 0,  
  LOCAL_TVT_SECRET_PHASE_RANGE_1_END    = 9,
  LOCAL_TVT_SECRET_PHASE_RANGE_2_START  = 10,   
  LOCAL_TVT_SECRET_PHASE_RANGE_2_END    = 19, 
  LOCAL_TVT_SECRET_PHASE_RANGE_3_START  = 20,   
  LOCAL_TVT_SECRET_PHASE_RANGE_3_END    = 29, 
  LOCAL_TVT_SECRET_PHASE_RANGE_4_START  = 30,   
  LOCAL_TVT_SECRET_PHASE_RANGE_4_END    = 49, 

  LOCAL_TVT_SECRET_PHASE_RANGE_MAX  = LOCAL_TVT_SECRET_PHASE_RANGE_4_END,
}LOCAL_TVT_SECRET_PHASE_RANGE;

//特殊イベント Local_Tvt_Secret_GetNowEventに追加すること
typedef enum
{
  LOCAL_TVT_SECRET_EVENT_NONE             = 0,
  LOCAL_TVT_SECRET_EVENT_PHASE1_DATE      = LOCAL_TVT_SECRET_PHASE_RANGE_1_END, //状態１のデート
  LOCAL_TVT_SECRET_EVENT_PHASE3_DATE      = LOCAL_TVT_SECRET_PHASE_RANGE_3_END, //状態３のデート
  LOCAL_TVT_SECRET_EVENT_PHASE4_SOUNDONLY = 39, //状態４の音声電話
  LOCAL_TVT_SECRET_EVENT_PHASE4_FACE      = LOCAL_TVT_SECRET_PHASE_RANGE_4_END, //状態４の素顔電話
  LOCAL_TVT_SECRET_EVENT_PHASE4_DATE      = 0xFF, //状態４の定期デート

}LOCAL_TVT_SECRET_EVENT;

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
static BOOL LOCAL_TVT_SECRET_Displeased( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );

//隠しキャラのデータ取得
static LOCAL_TVT_SECRET_PHASE Local_Tvt_Secret_GetPhase( const LOCAL_TVT_CALLDATA *cp_data );
static LOCAL_TVT_SECRET_EVENT Local_Tvt_Secret_GetNowEvent( const LOCAL_TVT_CALLDATA *cp_data );

static u32 Local_Tvt_Secret_GetTalkMsgID( const QUESTIONNAIRE_ANSWER_WORK *cp_quest, const LOCAL_TVT_CALLDATA *cp_data, LOCAL_TVT_MESSAGE *p_message );

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

  if( LOCAL_TVT_CALLDATA_GetSecretDispleased( cp_data ) )
  {
    p_person->greetingFunction  = LOCAL_TVT_SECRET_Displeased;
  }
  else
  {
    p_person->greetingFunction  = LOCAL_TVT_SECRET_Greeting;
    p_person->selectFunction[0] = LOCAL_TVT_SECRET_Talk;
    p_person->list_msgID[0]     = CALL_SECRET_01;
    p_person->selectFunction[1] = LOCAL_TVT_SECRET_Farewell;
    p_person->list_msgID[1]     = CALL_SECRET_02;
    p_person->list_num          = 2;
    p_person->list_start_msgID  = CALL_SECRET_SELECT_01;
    p_person->cp_list_msg_data  = cp_msg;
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
LOCAL_TVT_SECRET_FACE LOCAL_TVT_SECRET_GetFace( const LOCAL_TVT_CALLDATA *cp_data )
{
  u32 sex   = LOCAL_TVT_CALLDATA_GetSecretSex( cp_data );
  LOCAL_TVT_SECRET_PHASE phase  = Local_Tvt_Secret_GetPhase( cp_data );
  LOCAL_TVT_SECRET_EVENT event  = Local_Tvt_Secret_GetNowEvent( cp_data );

  if( phase == LOCAL_TVT_SECRET_PHASE_1 )
  {
    //１段階目は音声のみ
    return LOCAL_TVT_SECRET_FACE_INVISIBLE;
  }

  switch( event )
  {
  case LOCAL_TVT_SECRET_EVENT_PHASE4_SOUNDONLY:
    //音声会話
    return LOCAL_TVT_SECRET_FACE_INVISIBLE;

  case LOCAL_TVT_SECRET_EVENT_PHASE4_FACE:
    //素顔で会話
    if( sex == PTL_SEX_MALE )
    {
      return LOCAL_TVT_SECRET_FACE_VISIBLE_M;
    }
    else
    {
      return LOCAL_TVT_SECRET_FACE_VISIBLE_F;
    }

  default:
    //顔を隠しつつ会話
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
      u32 sex   = LOCAL_TVT_CALLDATA_GetSecretSex( cp_param->p_data );
      u32 msgID;

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
    //話しかけた
    p_wk->is_talk = TRUE;

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
  
    //話しかけた場合は状態変更
    if( p_wk->is_talk )
    {
      //約束をしたならば、フラグを立てる
      {
        LOCAL_TVT_SECRET_EVENT  event = Local_Tvt_Secret_GetNowEvent( cp_param->p_data );
        switch( event )
        {
        case LOCAL_TVT_SECRET_EVENT_PHASE1_DATE:
          LOCAL_TVT_CALLDATA_SetSecretDateEvent( cp_param->p_data, PERSON_STATE_SECRET_DATE_FIRST );
          break;
        case LOCAL_TVT_SECRET_EVENT_PHASE3_DATE:
        case LOCAL_TVT_SECRET_EVENT_PHASE4_DATE:
          LOCAL_TVT_CALLDATA_SetSecretDateEvent( cp_param->p_data, PERSON_STATE_SECRET_DATE_REGULAR );
          break;
        }
      }

      //話しかけたので好感度アップ
      LOCAL_TVT_CALLDATA_AddState( cp_param->p_data, LOCAL_TVT_PERSON_SECRET, LOCAL_TVT_SECRET_PHASE_RANGE_MAX );
    }

    //１日１回会話ON
    LOCAL_TVT_CALLDATA_SetSecretOnceCall( cp_param->p_data );

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
 *	@brief  不機嫌なとき
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_SECRET_Displeased( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
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
      u16 msgID = CALL_SECRET_TALK_SP_01;

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
    //１日１回会話ON
    LOCAL_TVT_CALLDATA_SetSecretOnceCall( cp_param->p_data );

    //不機嫌が治った！
    LOCAL_TVT_CALLDATA_SetSecretDispleased( cp_param->p_data, FALSE );

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
 *	@brief  仲良し段階を取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_data データ
 *
 *	@return フェーズ  ０～３段階
 */
//-----------------------------------------------------------------------------
static LOCAL_TVT_SECRET_PHASE Local_Tvt_Secret_GetPhase( const LOCAL_TVT_CALLDATA *cp_data )
{
  u16 state = LOCAL_TVT_CALLDATA_GetState( cp_data, LOCAL_TVT_PERSON_SECRET );

  if( LOCAL_TVT_SECRET_PHASE_RANGE_1_START <= state 
      && state <= LOCAL_TVT_SECRET_PHASE_RANGE_1_END )
  {
    return LOCAL_TVT_SECRET_PHASE_1; //１段階
  }
  else if( LOCAL_TVT_SECRET_PHASE_RANGE_2_START <= state 
      && state <= LOCAL_TVT_SECRET_PHASE_RANGE_2_END )
  {
    return LOCAL_TVT_SECRET_PHASE_2;  //２段階目
  }
  else if( LOCAL_TVT_SECRET_PHASE_RANGE_3_START <= state 
      && state <= LOCAL_TVT_SECRET_PHASE_RANGE_3_END )
  {
    return LOCAL_TVT_SECRET_PHASE_3;  //３段階目
  }
  else if( LOCAL_TVT_SECRET_PHASE_RANGE_4_START <= state 
      && state <= LOCAL_TVT_SECRET_PHASE_RANGE_4_END )
  {
    return LOCAL_TVT_SECRET_PHASE_4;  //４段階目
  }
  else
  {
    GF_ASSERT(0);
    return LOCAL_TVT_SECRET_PHASE_1; //念のため
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  現在のイベントを取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_data データ
 *
 *	@return 現在のイベント
 */
//-----------------------------------------------------------------------------
static LOCAL_TVT_SECRET_EVENT Local_Tvt_Secret_GetNowEvent( const LOCAL_TVT_CALLDATA *cp_data )
{
  static const LOCAL_TVT_SECRET_EVENT sc_event_tbl[]  =
  {
    LOCAL_TVT_SECRET_EVENT_PHASE1_DATE,
    LOCAL_TVT_SECRET_EVENT_PHASE3_DATE,
    LOCAL_TVT_SECRET_EVENT_PHASE4_SOUNDONLY,
    LOCAL_TVT_SECRET_EVENT_PHASE4_FACE,
  };
  u16 state = LOCAL_TVT_CALLDATA_GetState( cp_data, LOCAL_TVT_PERSON_SECRET );
  int i;

  //段階によるイベントを調査
  for( i = 0; i < NELEMS(sc_event_tbl); i++ )
  {
    if( sc_event_tbl[i] == state )
    {
      return sc_event_tbl[i];
    }
  }

  //定期デートが可能か
  //条件
  //・４段階目
  //・機嫌が悪くない
  //・１季節に１回
  {
    LOCAL_TVT_SECRET_PHASE phase  = Local_Tvt_Secret_GetPhase( cp_data );
    BOOL is_season  = LOCAL_TVT_CALLDATA_GetSecretSeasonDate( cp_data );
    BOOL is_disp    = LOCAL_TVT_CALLDATA_GetSecretDispleased( cp_data );
    if( phase == LOCAL_TVT_SECRET_PHASE_4 && !is_season && !is_disp )
    {
      return LOCAL_TVT_SECRET_EVENT_PHASE4_DATE;
    }
  }

  //何も無い
  return LOCAL_TVT_SECRET_EVENT_NONE;
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
  u32 sex   = LOCAL_TVT_CALLDATA_GetSecretSex( cp_data );
  LOCAL_TVT_SECRET_PHASE phase  = Local_Tvt_Secret_GetPhase( cp_data );
  LOCAL_TVT_SECRET_EVENT event  = Local_Tvt_Secret_GetNowEvent( cp_data );
  u16 state = LOCAL_TVT_CALLDATA_GetState( cp_data, LOCAL_TVT_PERSON_SECRET );

  //特殊イベントによって会話が変わる
  if( event == LOCAL_TVT_SECRET_EVENT_PHASE4_DATE )
  {
    return CALL_SECRET_TALK_SP_00;
  }

  //段階によって会話内容が違う
  switch( phase )
  {
  case LOCAL_TVT_SECRET_PHASE_1:
    //１段階目は最初の８つはプレイヤーのアンケートに答える
    if( state < 8 )
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
      u32 ansIdx  = QuestionnaireAnswer_ReadBit( cp_quest, sc_quest_tbl[state] );
      GF_ASSERT( state < NELEMS(sc_quest_tbl) );
      if( ansIdx == 0 )
      {
        //答えていなければ、１こ次に聞いてない時用の会話があるので次のメッセージIDにする
        u16 msgID = LOCAL_TVT_CALLDATA_GetTalkMsgID( cp_data, LOCAL_TVT_PERSON_SECRET );
        return ++msgID;
      }
      else
      {
        u32 answerID  = QIDATA_Get_AnswerID( sc_quest_tbl[state], ansIdx-1 );
        LOCAL_TVT_MESSAGE_RegisterQuestionnaire( p_message, 0, answerID );
        return LOCAL_TVT_CALLDATA_GetTalkMsgID( cp_data, LOCAL_TVT_PERSON_SECRET );
      }
    }
    else
    {
      return LOCAL_TVT_CALLDATA_GetTalkMsgID( cp_data, LOCAL_TVT_PERSON_SECRET );
    }

  case LOCAL_TVT_SECRET_PHASE_2:
  case LOCAL_TVT_SECRET_PHASE_3:
    return LOCAL_TVT_CALLDATA_GetTalkMsgID( cp_data, LOCAL_TVT_PERSON_SECRET );

  case LOCAL_TVT_SECRET_PHASE_4:
    //第４段階のときは会話はランダム
    if( GFUser_GetPublicRand( 1 ) )
    {
      return CALL_SECRET_TALK_31 + GFUser_GetPublicRand( 9 );
    }
    else
    {
      return CALL_SECRET_TALK_41 + GFUser_GetPublicRand( 9 );
    }
    break;
  default:
    GF_ASSERT(0);
    return CALL_SECRET_TALK_01;
  }
}

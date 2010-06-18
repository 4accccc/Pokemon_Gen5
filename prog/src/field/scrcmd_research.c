///////////////////////////////////////////////////////////////////////////////
/**
 * @file	 scrcmd_research.c
 * @brief	 スクリプトコマンド: すれ違い調査隊関連
 * @date   2010.03.09
 * @author obata
 */
///////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "system/vm_cmd.h"
#include "scrcmd_research.h"

#include "scrcmd_work.h"
#include "script_local.h"
#include "scrcmd.h"
#include "event_research_team.h"

#include "field/research_team.h"
#include "field/research_team_def.h"
#include "savedata/misc.h" 
#include "savedata/questionnaire_save.h" 
#include "gamesystem/game_beacon.h"

#include "../../../resource/research_radar/data/question_id.h"            // for QUESTION_ID_xxxx
#include "../../../resource/research_radar/data/answer_num_question.cdat" // for AnswerNum_question[]
#include "../../../resource/research_radar/data/answer_id_question.cdat"  // for AnswerID_question[][]


//#define DEBUG_PRINT_ON


static void CheckCurrentResearchFinish( SAVE_CONTROL_WORK* save, u16* RState );
static void CheckRequestFinish_inAdvance( SAVE_CONTROL_WORK* save, u8 RID, u16* RState );
static u32 GetCurrentCount( QUESTIONNAIRE_SAVE_WORK* QSave, u8 QID );
static BOOL CheckNormCount( QUESTIONNAIRE_SAVE_WORK* QSave, u8 RID );
static BOOL CheckMaxCount( QUESTIONNAIRE_SAVE_WORK* QSave, u8 RID );
static BOOL CheckNormTime( SAVE_CONTROL_WORK* save );
static BOOL CheckResearchOne( SAVE_CONTROL_WORK* save );
static u16 GetLackTime( SAVE_CONTROL_WORK* save );
static u16 GetLackCount( SAVE_CONTROL_WORK* save );
static u16 GetPassedTime( SAVE_CONTROL_WORK* save );



//-----------------------------------------------------------------------------
/**
 * @brief  調査隊の隊員ランクを取得する
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGetResearchTeamRank( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*       script   = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  MISC*              misc     = SaveData_GetMisc( save );
  u16* ret;
  
  ret = SCRCMD_GetVMWork( core, work ); // 第一引数: 隊員ランクの格納先

  // 隊員ランクを取得する
  *ret = MISC_CrossComm_GetResearchTeamRank( misc );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( 3, "_GET_RESEARCH_TEAM_RANK ==> %d\n", *ret );
#endif

  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief  調査隊の隊員ランクを1つ上げる
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResearchTeamRankUp( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*       script   = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  MISC*              misc     = SaveData_GetMisc( save );
  int rank;
  
  // 隊員ランクを取得する
  rank = MISC_CrossComm_GetResearchTeamRank( misc );

  // すでに最高ランク
  if( RESEARCH_TEAM_RANK_5 <= rank ) {
    GF_ASSERT(0); // これ以上は上がらない
    return VMCMD_RESULT_CONTINUE;
  }

  // ランクアップ
  rank++;
  MISC_CrossComm_SetResearchTeamRank( misc, rank );

  // ビーコン送信バッファに反映させる
  GAMEBEACON_SendDataUpdate_ResearchTeamRank( rank );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( 3, "_RESEARCH_TEAM_RANK_UP ==> %d\n", rank );
#endif

  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief 受けている調査依頼IDを取得する
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGetResearchRequestID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*       script   = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  MISC*              misc     = SaveData_GetMisc( save );
  u16* ret;
  
  ret = SCRCMD_GetVMWork( core, work ); // 第一引数: 調査依頼IDの格納先

  // 調査中の調査依頼IDを取得
  *ret = MISC_GetResearchRequestID( misc );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( 3, "_GET_RESEARCH_REQUEST_ID ==> %d\n", *ret );
#endif

  return VMCMD_RESULT_CONTINUE;
}


//-----------------------------------------------------------------------------
/**
 * @brief 調査中の質問IDを取得する
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGetResearchQuestionID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*       script   = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  MISC*              misc     = SaveData_GetMisc( save );
  u16* retQ1;
  u16* retQ2;
  u16* retQ3;
  
  // コマンドの引数を取得
  retQ1 = SCRCMD_GetVMWork( core, work ); // 第一引数: 質問ID1の格納先
  retQ2 = SCRCMD_GetVMWork( core, work ); // 第二引数: 質問ID2の格納先
  retQ3 = SCRCMD_GetVMWork( core, work ); // 第三引数: 質問ID3の格納先

  // 調査中の質問IDを取得
  *retQ1 = MISC_GetResearchQuestionID( misc, 0 ); 
  *retQ2 = MISC_GetResearchQuestionID( misc, 1 ); 
  *retQ3 = MISC_GetResearchQuestionID( misc, 2 ); 

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( 3, "_GET_RESEARCH_QUESTION_ID ==> %d, %d, %d\n", *retQ1, *retQ2, *retQ3 );
#endif

  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief 調査隊の調査について, 経過時間[h]を取得する ( MAX 24 )
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGetResearchPassedTime( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*       script   = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );

  u16* ret;
  u16 passedHour;
  
  // コマンドの引数を取得
  ret = SCRCMD_GetVMWork( core, work ); // 第一引数: 経過時間の格納先

  // 経過時間を算出
  passedHour = GetPassedTime( save );

  // 経過時間を返す
  *ret = passedHour;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( 3, "_GET_RESEARCH_PASSED_TIME ==> %d\n", passedHour );
#endif

  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief 調査を開始する
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdStartResearch( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*             work     = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*             script   = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*                gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK*       save     = GAMEDATA_GetSaveControlWork( gameData );
  MISC*                    misc     = SaveData_GetMisc( save );
  QUESTIONNAIRE_SAVE_WORK* qSave    = SaveData_GetQuestionnaire( save );

  int i;
  u16 reqID;
  u16 qID[ MAX_QNUM_PER_RESEARCH_REQ ];

  // コマンドの引数を取得
  reqID  = SCRCMD_GetVMWorkValue( core, work ); // 第一引数: 調査依頼ID
  qID[0] = SCRCMD_GetVMWorkValue( core, work ); // 第二引数: 質問ID1
  qID[1] = SCRCMD_GetVMWorkValue( core, work ); // 第三引数: 質問ID2
  qID[2] = SCRCMD_GetVMWorkValue( core, work ); // 第四引数: 質問ID3

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( 3, "_START_RESEARCH: reqID=%d, qID[0]=%d, qID[1]=%d, qID[2]=%d\n", reqID, qID[0], qID[1], qID[2] );
#endif

  // 調査依頼IDをセーブ
  MISC_SetResearchRequestID( misc, reqID );

 for( i=0; i<MAX_QNUM_PER_RESEARCH_REQ; i++ )
 {
   MISC_SetResearchQuestionID( misc, i, qID[i] ); // 質問IDをセーブ
   MISC_SetResearchCount( misc, i, 0 ); // 調査人数をクリア
 }

  // 調査開始時刻をセーブ
  MISC_SetResearchStartTimeBySecond( misc, GFL_RTC_GetDateTimeBySecond() );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( 3, "_START_RESEARCH: start time=%d\n", GFL_RTC_GetDateTimeBySecond() );
#endif


  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief 調査を終了する
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFinishResearch( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*             work     = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*             script   = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*                gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK*       save     = GAMEDATA_GetSaveControlWork( gameData );
  MISC*                    misc     = SaveData_GetMisc( save );
  QUESTIONNAIRE_SAVE_WORK* qSave    = SaveData_GetQuestionnaire( save );

  int i;

  // セーブデータをクリア
  MISC_SetResearchRequestID( misc, RESEARCH_REQ_ID_NONE );
  for( i=0; i<MAX_QNUM_PER_RESEARCH_REQ; i++ )
  {
    MISC_SetResearchQuestionID( misc, i, QUESTION_ID_DUMMY );
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( 3, "_FINISH_RESEARCH\n" );
#endif

  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief 指定した質問について, 最も多数派の回答IDを取得する
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGetMajorityAnswerOfQuestion( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*             work     = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*             script   = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*                gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK*       save     = GAMEDATA_GetSaveControlWork( gameData );
  QUESTIONNAIRE_SAVE_WORK* qSave    = SaveData_GetQuestionnaire( save );

  u16 qID;
  u16* retWork;
  u16 majorityAnswerID;

  // コマンドの引数を取得
  qID     = SCRCMD_GetVMWorkValue( core, work ); // 第一引数: 質問ID
  retWork = SCRCMD_GetVMWork( core, work );      // 第二引数: 回答IDの格納先

  GF_ASSERT( qID <= QUESTION_ID_MAX );

  // 最も多数派の回答IDを検索する
  {
    int ansNum;
    int ansIdx;
    int maxCount;

    ansNum = AnswerNum_question[ qID ];
    maxCount = 0;
    for( ansIdx=0; ansIdx < ansNum; ansIdx++ )
    {
      int totalCount = QuestionnaireWork_GetTotalAnswerNum( qSave, qID, ansIdx+1 );
      int todayCount = QuestionnaireWork_GetTodayAnswerNum( qSave, qID, ansIdx+1 );
      int sumCount   = totalCount + todayCount;
      if( maxCount <= sumCount ) {
        maxCount = sumCount;
        majorityAnswerID = AnswerID_question[ qID ][ ansIdx ];
      }
#ifdef DEBUG_PRINT_ON
      OS_TFPrintf( 3, 
          "questionID=%d, answerIdx=%d, sumCount=%d(%d+%d)\n", 
          qID, ansIdx, sumCount, totalCount, todayCount );
#endif
    }
  }

  // 最も多数派の回答IDを返す
  *retWork = majorityAnswerID;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( 3, "_GET_MAJORITY_ANSWER ==> %d\n", *retWork );
#endif

  return VMCMD_RESULT_CONTINUE;
} 

//-----------------------------------------------------------------------------
/**
 * @brief 指定した質問についての, 自分の回答を設定する
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdSetMyAnswer( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*              work     = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*              script   = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*                 gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK*        save     = GAMEDATA_GetSaveControlWork( gameData );
  QUESTIONNAIRE_SAVE_WORK*  qSave    = SaveData_GetQuestionnaire( save );
  QUESTIONNAIRE_ANSWER_WORK* myAnswer = Questionnaire_GetAnswerWork( qSave );
  u16 qID;
  u16 aIdx;

  // コマンドの引数を取得
  qID  = SCRCMD_GetVMWorkValue( core, work ); // 第一引数: 質問ID
  aIdx = SCRCMD_GetVMWorkValue( core, work ); // 第二引数: 何番目の回答か

  // 回答をセット
  QuestionnaireAnswer_WriteBit( myAnswer, qID, aIdx );
  GAMEBEACON_SendDataUpdate_Questionnaire( myAnswer );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( 3, "_SET_MY_ANSWER ==> qID=%d, aIdx=%d\n", qID, aIdx );
#endif

  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief  プレイヤーの隊員情報を表示する
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdDispResearchTeamInfo( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work     = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*  script   = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gameSystem = SCRCMD_WORK_GetGameSysWork( work );

  SCRIPT_CallEvent( script, EVENT_DispResearchTeamInfo( gameSystem ) );
  return VMCMD_RESULT_SUSPEND;
}

//-----------------------------------------------------------------------------
/**
 * @brief  指定した依頼を達成しているかどうかをチェックする
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdCheckAchieveRequest( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  const MISC*        misc     = SaveData_GetMiscConst( save );

  u8 RID;
  u16* ret_wk;

  RID = SCRCMD_GetVMWorkValue( core, work ); // 第一引数: 依頼ID
  ret_wk = SCRCMD_GetVMWork( core, work );   // 第二引数: 依頼の達成状況の格納先

  // 指定された依頼が現在調査中である場合
  if( RID == MISC_GetResearchRequestID(misc) ) {
    CheckCurrentResearchFinish( save, ret_wk );
  }
  // 現在調査中でない依頼の場合
  else {
    CheckRequestFinish_inAdvance( save, RID, ret_wk );
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( 3, "_CHECK_ACHIEVE_REQUEST ==> " );
  switch( *ret_wk ) {
  case RESEARCH_REQ_STATE_NULL:             OS_TFPrintf( 3, "NULL" );             break;
  case RESEARCH_REQ_STATE_ACHIEVE:          OS_TFPrintf( 3, "ACHIEVE" );          break;
  case RESEARCH_REQ_STATE_FALSE_NORM_COUNT: OS_TFPrintf( 3, "FALSE_NORM_COUNT" ); break;
  case RESEARCH_REQ_STATE_FALSE_NORM_TIME:  OS_TFPrintf( 3, "FALSE_NORM_TIME" );  break;
  case RESEARCH_REQ_STATE_FALSE_ZERO_COUNT: OS_TFPrintf( 3, "FALSE_ZERO_COUNT" ); break;
  default: GF_ASSERT(0);
  }
  OS_TFPrintf( 3, "\n" );
#endif

  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief  調査中の依頼の不足値を取得する
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGetLackForAchieve( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  const MISC*        misc     = SaveData_GetMiscConst( save );

  u16* ret_wk;
  u16 lack;
  u8 RID;
  u8 RType;

  // コマンド引数を取得
  ret_wk = SCRCMD_GetVMWork( core, work ); // 第一引数: 不足値の格納先

  // 調査タイプを取得
  RID = MISC_GetResearchRequestID( misc );
  RType = RESEARCH_TEAM_GetResearchType( RID );

  // 達成不足値を取得
  switch( RType ) {
  case RESEARCH_REQ_TYPE_TIME:  lack = GetLackTime( save );  break;
  case RESEARCH_REQ_TYPE_COUNT: lack = GetLackCount( save ); break;
  default: GF_ASSERT(0);
  }

  *ret_wk = lack;
  return VMCMD_RESULT_CONTINUE;
}

//=============================================================================
/**
 * @brief 調査中の依頼の達成をチェックする
 *
 * @param save
 * @param RState 調査状況の格納先
 */
//=============================================================================
static void CheckCurrentResearchFinish( SAVE_CONTROL_WORK* save, u16* RState )
{
  QUESTIONNAIRE_SAVE_WORK* QSave = SaveData_GetQuestionnaire( save );
  const MISC*              misc  = SaveData_GetMiscConst( save );

  u8 RID;
  u8 RType;

  RID = MISC_GetResearchRequestID( misc ); // 調査依頼IDを取得
  RType = RESEARCH_TEAM_GetResearchType( RID ); // 依頼タイプを取得

  // 人数調査の場合
  if( RType == RESEARCH_REQ_TYPE_COUNT ) {
    // ノルマ達成
    if( CheckNormCount( QSave, RID ) ) {
      *RState = RESEARCH_REQ_STATE_ACHIEVE;
    }
    // ノルマ未達成
    else {
      *RState = RESEARCH_REQ_STATE_FALSE_NORM_COUNT;
    }
  }
  // 時間調査の場合
  else if( RType == RESEARCH_REQ_TYPE_TIME ) {
    // ノルマ時間が経過
    if( CheckNormTime( save ) ) {
      // 1人以上調査した
      if( CheckResearchOne( save ) ) {
        *RState = RESEARCH_REQ_STATE_ACHIEVE;
      }
      // 1人も調査していない
      else {
        *RState = RESEARCH_REQ_STATE_FALSE_ZERO_COUNT;
      }
    }
    // ノルマ時間が経過していない
    else {
      *RState = RESEARCH_REQ_STATE_FALSE_NORM_TIME;
    }
  }
  else {
    GF_ASSERT(0);
  }
}

//=============================================================================
/**
 * @brief 指定した依頼をすでに達成しているかどうかをチェックする
 *
 * @param save
 * @param RID    チェックする依頼のID
 * @param RState 調査状況の格納先
 */
//=============================================================================
static void CheckRequestFinish_inAdvance( SAVE_CONTROL_WORK* save, u8 RID, u16* RState )
{
  QUESTIONNAIRE_SAVE_WORK* QSave = SaveData_GetQuestionnaire( save );
  const MISC*              misc  = SaveData_GetMiscConst( save );

  u8 RType;

  RType = RESEARCH_TEAM_GetResearchType( RID ); // 依頼タイプを取得

  // 人数調査の場合
  if( RType == RESEARCH_REQ_TYPE_COUNT ) {
    // ノルマ達成
    if( CheckNormCount( QSave, RID ) ) {
      *RState = RESEARCH_REQ_STATE_ACHIEVE;
    }
    // ノルマ未達成
    else {
      *RState = RESEARCH_REQ_STATE_FALSE_NORM_COUNT;
    }
  }
  // 時間調査の場合
  else if( RType == RESEARCH_REQ_TYPE_TIME ) {
    // MAXカウントまで調査している
    if( CheckMaxCount( QSave, RID ) ) {
      *RState = RESEARCH_REQ_STATE_ACHIEVE;
    }
    // ノルマ時間が経過していない
    else {
      *RState = RESEARCH_REQ_STATE_FALSE_NORM_TIME;
    }
  }
  else {
    GF_ASSERT(0);
  }
}

//=============================================================================
/**
 * @brief 現在の調査人数を取得する
 *
 * @param QID 質問ID ( QUESTION_ID_xxxx )
 *
 * @return 指定した質問の, 現時点での調査人数 ( いままで + 今日 )
 */
//=============================================================================
static u32 GetCurrentCount( QUESTIONNAIRE_SAVE_WORK* QSave, u8 QID )
{
  u32 today = QuestionnaireWork_GetTodayCount( QSave, QID );
  u32 total = QuestionnaireWork_GetTotalCount( QSave, QID );
  u32 current = today + total;
  if( QUESTIONNAIRE_TOTAL_COUNT_MAX < current ) { 
    current = QUESTIONNAIRE_TOTAL_COUNT_MAX; 
  }
  return current;
}

//=============================================================================
/**
 * @brief ノルマ人数の達成をチェックする
 *
 * @param QSave
 * @param RID   チェックする調査依頼のID ( RESEARCH_REQ_ID_xxxx )
 *
 * @return 指定した依頼を達成している場合 TRUE
 *         そうでなければ FALSE
 */
//=============================================================================
static BOOL CheckNormCount( QUESTIONNAIRE_SAVE_WORK* QSave, u8 RID )
{
  int i;
  int norm;
  u8 QID[ MAX_QNUM_PER_RESEARCH_REQ ];
  u8 QNum;

  RESEARCH_TEAM_GetQuestionID( RID, QID );
  QNum = RESEARCH_TEAM_GetQuestionNum( RID );
  norm = RESEARCH_TEAM_GetNormCount( RID );

  for( i=0; i<QNum; i++ )
  {
    u32 count = GetCurrentCount( QSave, QID[i] );

#ifdef DEBUG_PRINT_ON
    OS_TFPrintf( 3, 
        "CheckNormCount: QID=%d, norm=%d, count=%d\n", QID[i], norm, count );
#endif

    // ノルマを達成していない
    if( count < norm ) {
      return FALSE;
    }
  } 
  return TRUE;
}

//=============================================================================
/**
 * @brief 調査中の依頼について, ノルマ時間の達成をチェックする
 *
 * @param save
 *
 * @return 調査中の依頼がノルマ時間を達成している場合 TRUE
 *         そうでなければ FALSE
 */
//=============================================================================
static BOOL CheckNormTime( SAVE_CONTROL_WORK* save )
{
  QUESTIONNAIRE_SAVE_WORK* QSave = SaveData_GetQuestionnaire( save );
  const MISC*              misc  = SaveData_GetMiscConst( save );

  int RID;
  int pass;
  int norm;

  RID  = MISC_GetResearchRequestID( misc );
  pass = GetPassedTime( save ); // 経過時間を取得
  norm = RESEARCH_TEAM_GetNormTime( RID ); // ノルマ時間を取得

  GF_ASSERT( RESEARCH_TEAM_GetResearchType(RID) == RESEARCH_REQ_TYPE_TIME );

  // 経過している
  if( norm <= pass ) {
    return TRUE;
  }
  // 経過していない
  else {
    return FALSE;
  }
}

//=============================================================================
/**
 * @brief 調査中の依頼について, 調査の開始から１人でも調査したかどうかをチェックする
 *
 * @param save
 *
 * @return 調査中の依頼を開始してから, １人以上の調査を行っている場合 TRUE
 *         そうでなければ FALSE
 */
//=============================================================================
static BOOL CheckResearchOne( SAVE_CONTROL_WORK* save )
{
  int i;
  u8 RID;
  u8 QNum;
  const MISC* misc;

  // 調査中の質問数を取得
  misc = SaveData_GetMiscConst( save );
  RID  = MISC_GetResearchRequestID( misc );
  QNum = RESEARCH_TEAM_GetQuestionNum( RID );

  // 調査開始からの, 各質問の調査人数をチェック
  for( i=0; i<QNum; i++ )
  {
    // １人も調査していない質問を発見
    if( MISC_GetResearchCount( misc, i ) == 0 ) {
      return FALSE;
    }
  }
  return TRUE;
}

//=============================================================================
/**
 * @brief MAXカウントの達成をチェックする
 *
 * @param QSave
 * @param RID  チェックする依頼のID
 *
 * @return 指定した依頼に該当する全質問をMAXカウントまで調査している場合 TRUE
 *         そうでなければ FALSE
 */
//=============================================================================
static BOOL CheckMaxCount( QUESTIONNAIRE_SAVE_WORK* QSave, u8 RID )
{
  int i;
  u8 QID[ MAX_QNUM_PER_RESEARCH_REQ ];
  u8 QNum;
  int norm;

  RESEARCH_TEAM_GetQuestionID( RID, QID );
  QNum = RESEARCH_TEAM_GetQuestionNum( RID );
  norm = RESEARCH_TEAM_GetNormCount( RID );

  for( i=0; i<QNum; i++ )
  {
    u32 count = GetCurrentCount( QSave, QID[i] );

    // MAXカウントに達していない
    if( count < QUESTIONNAIRE_TOTAL_COUNT_MAX ) {
      return FALSE;
    }
  } 
  return TRUE;
}

//=============================================================================
/**
 * @brief 調査中の依頼のノルマ達成不足人数を取得する
 *
 * @param save
 *
 * @return 調査中の依頼について, ノルマ達成までの不足人数
 */
//=============================================================================
static u16 GetLackCount( SAVE_CONTROL_WORK* save )
{
  QUESTIONNAIRE_SAVE_WORK* QSave = SaveData_GetQuestionnaire( save );
  const MISC*              misc  = SaveData_GetMiscConst( save );

  int i;
  u8 RID;
  u8 QID[ MAX_QNUM_PER_RESEARCH_REQ ];
  u8 QNum;
  u32 norm;
  u32 lack;
  u32 minCount;

  RID  = MISC_GetResearchRequestID( misc );
  norm = RESEARCH_TEAM_GetNormCount( RID );
  QNum = RESEARCH_TEAM_GetQuestionNum( RID );
  RESEARCH_TEAM_GetQuestionID( RID, QID );

  // 一番少ない調査人数を取得
  minCount = QUESTIONNAIRE_TOTAL_COUNT_MAX + QUESTIONNAIRE_TODAY_COUNT_MAX;
  for( i=0; i<QNum; i++ )
  {
    u32 count = GetCurrentCount( QSave, QID[i] );
    if( count < minCount ) { minCount = count; }
  }

  lack = norm - minCount;
  if( lack < 0 ) { lack = 0; }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( 3, 
      "GetLackCount: RID=%d, norm=%d, minCount=%d, lack=%d\n", 
      RID, norm, minCount, lack );
#endif

  return lack;
}

//=============================================================================
/**
 * @brief 調査中の依頼のノルマ達成不足時間を取得する
 */
//=============================================================================
static u16 GetLackTime( SAVE_CONTROL_WORK* save )
{
  QUESTIONNAIRE_SAVE_WORK* QSave = SaveData_GetQuestionnaire( save );
  const MISC*              misc  = SaveData_GetMiscConst( save );

  int RID;
  int pass;
  int norm;
  int lack;

  RID  = MISC_GetResearchRequestID( misc );
  pass = GetPassedTime( save ); // 経過時間を取得
  norm = RESEARCH_TEAM_GetNormTime( RID ); // ノルマ時間を取得

  // 不足時間を算出
  lack = norm - pass; 
  if( lack < 0 ) { lack = 0; }
  return lack;
}

//=============================================================================
/**
 * @brief 調査中の依頼の経過時間を取得する
 */
//=============================================================================
static u16 GetPassedTime( SAVE_CONTROL_WORK* save )
{
  MISC* misc = SaveData_GetMisc( save );

  s64 startSec;
  u16 passedHour;

  // 調査中でない
  if( MISC_GetResearchRequestID( misc ) == RESEARCH_REQ_ID_NONE ) {
    GF_ASSERT(0);
    return 0;
  }
  
  // 調査の開始時刻を取得
  startSec = MISC_GetResearchStartTimeBySecond( misc );

  { 
    s64 nowSec, passedSec;
    RTCDate passedDate;
    RTCTime passedTime;

    // 現在時刻を取得
    nowSec = GFL_RTC_GetDateTimeBySecond();

    // 経過時間[h]を算出
    passedSec = nowSec - startSec;
    RTC_ConvertSecondToDateTime( &passedDate, &passedTime, passedSec );

    if( (0 < passedDate.year) ||  // 1年以上が経過
        (1 < passedDate.month) || // 1月以上が経過
        (1 < passedDate.day) )    // 1日以上が経過
    { 
      passedHour = 24; // 最大24時間とする ( それ以上は必要ないので )
    }
    else
    {
      passedHour = passedTime.hour;
    }
  }

  return passedHour;
}

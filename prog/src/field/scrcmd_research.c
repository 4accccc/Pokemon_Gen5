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

#include "field/research_team_def.h"
#include "savedata/misc.h" 
#include "savedata/questionnaire_save.h" 

#include "../../../resource/research_radar/data/question_id.h"            // for QUESTION_ID_xxxx
#include "../../../resource/research_radar/data/answer_num_question.cdat" // for AnswerNum_question[]
#include "../../../resource/research_radar/data/answer_id_question.cdat"  // for AnswerID_question[][]


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
  MISC*              misc     = SaveData_GetMisc( save );

  u16* ret;
  s64 startSec;
  u16 passedHour;
  
  // コマンドの引数を取得
  ret = SCRCMD_GetVMWork( core, work ); // 第一引数: 経過時間の格納先

  // 調査の開始時刻を取得
  startSec = MISC_GetResearchStartTimeBySecond( misc );

  //--調査中でない--
  if( MISC_GetResearchRequestID( misc ) == RESEARCH_REQ_ID_NONE ) {
    passedHour = 0;
  }
  //--調査中--
  else { 
    s64 nowSec, passedSec;
    RTCDate passedDate;
    RTCTime passedTime;

    // 経過時間[h]を算出
    nowSec = GFL_RTC_GetDateTimeBySecond();
    passedSec = nowSec - startSec;
    RTC_ConvertSecondToDateTime( &passedDate, &passedTime, passedSec );
    if( (0 < passedDate.year) || 
        (0 < passedDate.month) || 
        (0 < passedDate.day) )
    { 
      passedHour = 24; // 最大24時間とする ( それ以上は必要ないので )
    }
    else
    {
      passedHour = passedTime.hour;
    }
  }

  // 経過時間を返す
  *ret = passedHour;

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

  // 調査依頼IDをセーブ
  MISC_SetResearchRequestID( misc, reqID );

 for( i=0; i<MAX_QNUM_PER_RESEARCH_REQ; i++ )
 {
   // 質問IDをセーブ
   MISC_SetResearchQuestionID( misc, i, qID[i] );

   // ダミーID
   if( qID[i] == QUESTION_ID_DUMMY ) { continue; }

   // 開始時の回答人数をセーブ
   {
     int count = QuestionnaireWork_GetTotalCount( qSave, qID[i] );
     MISC_SetResearchStartCount( misc, i, count );
   }
 }

  // 調査開始時刻をセーブ
  MISC_SetResearchStartTimeBySecond( misc, GFL_RTC_GetDateTimeBySecond() );

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

  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief 質問に対する, いままでの回答人数を取得する
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGetTotalCountOfQuestion( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*             work     = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*             script   = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*                gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK*       save     = GAMEDATA_GetSaveControlWork( gameData );
  QUESTIONNAIRE_SAVE_WORK* qSave    = SaveData_GetQuestionnaire( save );

  int i;
  u16 qID;
  u16* retWork;

  // コマンドの引数を取得
  qID     = SCRCMD_GetVMWorkValue( core, work ); // 第一引数: 質問ID
  retWork = SCRCMD_GetVMWork( core, work );      // 第二引数: 回答人数の格納先

  // いままでの回答人数を取得
  *retWork = QuestionnaireWork_GetTotalCount( qSave, qID );

  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief 質問に対する, 調査開始時の回答人数を取得する
 * @param  core
 * @param  wk
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGetStartCountOfQuestion( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*       script   = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  MISC*              misc     = SaveData_GetMisc( save );

  int i;
  u16 qID;
  u16* retWork;

  // コマンドの引数を取得
  qID     = SCRCMD_GetVMWorkValue( core, work ); // 第一引数: 質問ID
  retWork = SCRCMD_GetVMWork( core, work );      // 第二引数: 回答人数の格納先

  // セーブデータから, 該当する質問IDの調査開始時の回答人数を取得する
  for( i=0; i<MAX_QNUM_PER_RESEARCH_REQ; i++ )
  {
    if( MISC_GetResearchQuestionID(misc, i) == qID ) {
      *retWork = MISC_GetResearchStartCount( misc, i );
    }
  }

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
    int count;

    ansNum = AnswerNum_question[ qID ];
    maxCount = 0;
    for( ansIdx=0; ansIdx < ansNum; ansIdx++ )
    {
      count = QuestionnaireWork_GetTotalAnswerNum( qSave, qID, ansIdx - 1 );
      if( maxCount <= count ) {
        maxCount = count;
        majorityAnswerID = AnswerID_question[ qID ][ ansIdx ];
      }
    }
  }

  // 最も多数派の回答IDを返す
  *retWork = majorityAnswerID;

  return VMCMD_RESULT_CONTINUE;
}

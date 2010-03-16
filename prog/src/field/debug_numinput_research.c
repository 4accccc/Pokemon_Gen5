////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  デバッグメニュー『数値入力』 ( すれ違い調査関連 )
 * @file   debug_numinput_research.c
 * @author obata
 * @date   2010.03.13
 */
////////////////////////////////////////////////////////////////////////////////
#include "debug_numinput_research.h"

#include "savedata/save_control.h"       // for SAVE_CONTROL_WORK
#include "savedata/questionnaire_save.h" // for QUESTIONNAIRE_SAVE_WORK 
#include "savedata/misc.h"               // for MISC

#include "app/research_radar/questionnaire_index.h"
#include "field/research_team_def.h"


//------------------------------------------------------------------------------
/**
 * @brief 数値取得関数 ( 数値入力 → 今日のすれ違い調査人数 → Qx )
 *
 * @param gameSystem
 * @param gameData
 * @param questionID 取得する質問のID ( QUESTION_ID_xxxx )
 *
 * @return 質問に対する, 今日の回答人数
 */
//------------------------------------------------------------------------------
u32 DEBUG_GetTodayCountOfQuestion( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 questionID )
{
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* Qsave;

  // セーブデータ取得
  save = GAMEDATA_GetSaveControlWork( gameData );
  Qsave = SaveData_GetQuestionnaire( save );

  // 指定された質問に対する, 今日の回答人数を返す
  return QuestionnaireWork_GetTodayCount( Qsave, questionID );
}
//------------------------------------------------------------------------------
/**
 * @brief 数値設定関数 ( 数値入力 → 今日のすれ違い調査人数 → Qx )
 *
 * @param gameSystem
 * @param gameData
 * @param questionID 設定する質問のID ( QUESTION_ID_xxxx )
 * @param count      設定する数値
 */
//------------------------------------------------------------------------------
void DEBUG_SetTodayCountOfQuestion( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 questionID, u32 count )
{
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* Qsave;
  int addCount;
  int nowCount;

  // セーブデータ取得
  save = GAMEDATA_GetSaveControlWork( gameData );
  Qsave = SaveData_GetQuestionnaire( save );

  // 現在値, 目標値から, 加算値を算出
  nowCount = QuestionnaireWork_GetTodayCount( Qsave, questionID );
  addCount = count - nowCount;

  // 指定された値になるように加算
  QuestionnaireWork_AddTodayCount( Qsave, questionID, addCount );
}
//------------------------------------------------------------------------------
/**
 * @brief 数値取得関数 ( 数値入力 → いままでのすれ違い調査人数 → Qx )
 *
 * @param gameSystem
 * @param gameData
 * @param questionID 取得する質問のID ( QUESTION_ID_xxxx )
 *
 * @return 質問に対する, いままでの回答人数
 */
//------------------------------------------------------------------------------
u32 DEBUG_GetTotalCountOfQuestion( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 questionID )
{
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* Qsave;

  // セーブデータ取得
  save = GAMEDATA_GetSaveControlWork( gameData );
  Qsave = SaveData_GetQuestionnaire( save );

  // 指定された質問に対する, 今日の回答人数を返す
  return QuestionnaireWork_GetTotalCount( Qsave, questionID );
}
//------------------------------------------------------------------------------
/**
 * @brief 数値設定関数 ( 数値入力 → いままでのすれ違い調査人数 → Qx )
 *
 * @param gameSystem
 * @param gameData
 * @param questionID 設定する質問のID ( QUESTION_ID_xxxx )
 * @param count      設定する数値
 */
//------------------------------------------------------------------------------
void DEBUG_SetTotalCountOfQuestion( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 questionID, u32 count )
{
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* Qsave;
  int addCount;
  int nowCount;

  // セーブデータ取得
  save = GAMEDATA_GetSaveControlWork( gameData );
  Qsave = SaveData_GetQuestionnaire( save );

  // 現在値, 目標値から, 加算値を算出
  nowCount = QuestionnaireWork_GetTotalCount( Qsave, questionID );
  addCount = count - nowCount;

  // 指定された値になるように加算
  QuestionnaireWork_AddTotalCount( Qsave, questionID, addCount );
} 
//------------------------------------------------------------------------------
/**
 * @brief 数値取得関数 ( 数値入力 → 今日のすれ違い回答人数 → Ax )
 *
 * @param gameSystem
 * @param gameData
 * @param answerID 取得する回答のID ( ANSWER_ID_xxxx )
 *
 * @return 質問に対する, 今日の回答人数
 */
//------------------------------------------------------------------------------
u32 DEBUG_GetTodayCountOfAnswer( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 answerID )
{
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* Qsave;
  int questionID;
  int answerIdx;

  // セーブデータ取得
  save = GAMEDATA_GetSaveControlWork( gameData );
  Qsave = SaveData_GetQuestionnaire( save );

  // 指定された回答が属する質問のIDを取得
  questionID = GetQuestionID_byAnswerID( answerID );

  // 指定された回答が, 何番目の回答なのかを取得
  answerIdx = GetAnswerIndex_atQuestion( answerID );

  // 指定された質問に対する, 今日の回答人数を返す
  return QuestionnaireWork_GetTodayAnswerNum( Qsave, questionID, answerIdx );
}
//------------------------------------------------------------------------------
/**
 * @brief 数値設定関数 ( 数値入力 → 今日のすれ違い回答人数 → Ax )
 *
 * @param gameSystem
 * @param gameData
 * @param answerID   設定する回答のID ( ANSWER_ID_xxxx )
 * @param count      設定する数値
 */
//------------------------------------------------------------------------------
void DEBUG_SetTodayCountOfAnswer( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 answerID, u32 count )
{
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* Qsave;
  int questionID;
  int answerIdx;
  int nowCount, addCount;

  // セーブデータ取得
  save = GAMEDATA_GetSaveControlWork( gameData );
  Qsave = SaveData_GetQuestionnaire( save );

  // 指定された回答が属する質問のIDを取得
  questionID = GetQuestionID_byAnswerID( answerID );

  // 指定された回答が, 何番目の回答なのかを取得
  answerIdx = GetAnswerIndex_atQuestion( answerID );

  // 現在値, 目標値から, 加算値を算出
  nowCount = QuestionnaireWork_GetTodayAnswerNum( Qsave, questionID, answerIdx );
  addCount = count - nowCount;

  // 指定された値になるように加算
  QuestionnaireWork_AddTodayAnswerNum( Qsave, questionID, answerIdx, addCount );

  // DEBUG:
  OBATA_Printf( "SetTodayCountOfAnswer: qID=%d, aID=%d, aIdx=%d, count=%d\n", 
      questionID, answerID, answerIdx,
      QuestionnaireWork_GetTodayAnswerNum( Qsave, questionID, answerIdx ) );
} 
//------------------------------------------------------------------------------
/**
 * @brief 数値取得関数 ( 数値入力 → いままでのすれ違い回答人数 → Ax )
 *
 * @param gameSystem
 * @param gameData
 * @param answerID 取得する回答のID ( ANSWER_ID_xxxx )
 *
 * @return 質問に対する, いままでの回答人数
 */
//------------------------------------------------------------------------------
u32 DEBUG_GetTotalCountOfAnswer( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 answerID )
{
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* Qsave;
  int questionID;
  int answerIdx;

  // セーブデータ取得
  save = GAMEDATA_GetSaveControlWork( gameData );
  Qsave = SaveData_GetQuestionnaire( save );

  // 指定された回答が属する質問のIDを取得
  questionID = GetQuestionID_byAnswerID( answerID );

  // 指定された回答が, 何番目の回答なのかを取得
  answerIdx = GetAnswerIndex_atQuestion( answerID );

  // 指定された質問に対する, いままでの回答人数を返す
  return QuestionnaireWork_GetTotalAnswerNum( Qsave, questionID, answerIdx );
}
//------------------------------------------------------------------------------
/**
 * @brief 数値設定関数 ( 数値入力 → いままでのすれ違い回答人数 → Ax )
 *
 * @param gameSystem
 * @param gameData
 * @param answerID   設定する回答のID ( ANSWER_ID_xxxx )
 * @param count      設定する数値
 */
//------------------------------------------------------------------------------
void DEBUG_SetTotalCountOfAnswer( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 answerID, u32 count )
{
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* Qsave;
  int questionID;
  int answerIdx;
  int nowCount, addCount;

  // セーブデータ取得
  save = GAMEDATA_GetSaveControlWork( gameData );
  Qsave = SaveData_GetQuestionnaire( save );

  // 指定された回答が属する質問のIDを取得
  questionID = GetQuestionID_byAnswerID( answerID );

  // 指定された回答が, 何番目の回答なのかを取得
  answerIdx = GetAnswerIndex_atQuestion( answerID );

  // 現在値, 目標値から, 加算値を算出
  nowCount = QuestionnaireWork_GetTotalAnswerNum( Qsave, questionID, answerIdx );
  addCount = count - nowCount;

  // 指定された値になるように加算
  QuestionnaireWork_AddTotalAnswerNum( Qsave, questionID, answerIdx, addCount );

  // DEBUG:
  OBATA_Printf( "SetTotalCountOfAnswer: qID=%d, aID=%d, aIdx=%d, count=%d\n", 
      questionID, answerID, answerIdx,
      QuestionnaireWork_GetTotalAnswerNum( Qsave, questionID, answerIdx ) );
}
//------------------------------------------------------------------------------
/**
 * @brief 数値取得関数 ( 数値入力 → すれ違い調査隊 → 隊員ランク )
 *
 * @param gameSystem
 * @param gameData
 * @param param
 *
 * @return 隊員ランク ( RESEARCH_TEMP_RANK_xxxx )
 */
//------------------------------------------------------------------------------
u32 DEBUG_GetResearchTeamRank( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 param )
{
  SAVE_CONTROL_WORK* save;
  MISC* misc;
  int rank;

  // セーブデータ取得
  save = GAMEDATA_GetSaveControlWork( gameData );
  misc = SaveData_GetMisc( save );

  // 隊員ランクを取得
  rank = MISC_CrossComm_GetResearchTeamRank( misc );

  return rank;
}
//------------------------------------------------------------------------------
/**
 * @brief 数値設定関数 ( 数値入力 → すれ違い調査隊 → 隊員ランク )
 *
 * @param gameSystem
 * @param gameData
 * @param answerID   設定する回答のID ( ANSWER_ID_xxxx )
 * @param rank       設定する数値
 */
//------------------------------------------------------------------------------
void DEBUG_SetResearchTeamRank( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 answerID, u32 rank )
{
  SAVE_CONTROL_WORK* save;
  MISC* misc;

  // 引数チェック
  GF_ASSERT( rank < RESEARCH_TEAM_RANK_MAX );

  // セーブデータ取得
  save = GAMEDATA_GetSaveControlWork( gameData );
  misc = SaveData_GetMisc( save );

  // 隊員ランクを設定
  MISC_CrossComm_SetResearchTeamRank( misc, rank );
}

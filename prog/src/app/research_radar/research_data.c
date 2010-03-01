//////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査データ
 * @file   research_data.c
 * @author obata
 * @date   2010.03.01
 */
//////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "research_data.h"


//============================================================================
// ■データ取得関数
//============================================================================

//----------------------------------------------------------------------------
/**
 * @brief 調査項目IDを取得する
 *
 * @param 
 */
//----------------------------------------------------------------------------
u8 RESEARCH_DATA_GetTopicID( const RESEARCH_DATA* data )
{
  return data->topicID;
}

//----------------------------------------------------------------------------
/**
 * @brief 質問IDを取得する
 *
 * @param 
 */
//----------------------------------------------------------------------------
u8 RESEARCH_DATA_GetQuestionID_byIndex( const RESEARCH_DATA* data, u8 qIdx )
{
  GF_ASSERT( qIdx < QUESTION_NUM_PER_TOPIC ); // インデックスエラー

  return data->questionData[ qIdx ].ID;
}

//----------------------------------------------------------------------------
/**
 * @brief 回答の数を取得する
 *
 * @param 
 */
//----------------------------------------------------------------------------
u8 RESEARCH_DATA_GetAnswerNumOfQuestion_byIndex( const RESEARCH_DATA* data, u8 qIdx )
{
  GF_ASSERT( qIdx < QUESTION_NUM_PER_TOPIC ); // インデックスエラー

  return data->questionData[ qIdx ].answerNum;
}

//----------------------------------------------------------------------------
/**
 * @brief 今日の回答人数を取得する
 *
 * @param 
 */
//----------------------------------------------------------------------------
u16 RESEARCH_DATA_GetTodayCountOfQuestion_byIndex( const RESEARCH_DATA* data, u8 qIdx )
{
  GF_ASSERT( qIdx < QUESTION_NUM_PER_TOPIC ); // インデックスエラー

  return data->questionData[ qIdx ].todayCount;
}

//----------------------------------------------------------------------------
/**
 * @brief いままでの回答人数を取得する
 *
 * @param 
 */
//----------------------------------------------------------------------------
u16 RESEARCH_DATA_GetTotalCountOfQuestion_byIndex( const RESEARCH_DATA* data, u8 qIdx )
{
  GF_ASSERT( qIdx < QUESTION_NUM_PER_TOPIC ); // インデックスエラー

  return data->questionData[ qIdx ].totalCount;
}

//----------------------------------------------------------------------------
/**
 * @brief 回答IDを取得する
 *
 * @param 
 */
//----------------------------------------------------------------------------
u16 RESEARCH_DATA_GetAnswerID_byIndex( const RESEARCH_DATA* data, u8 qIdx, u8 aIdx )
{
  // インデックスエラー
  GF_ASSERT( aIdx < RESEARCH_DATA_GetAnswerNumOfQuestion_byIndex(data, qIdx) ); 

  return data->questionData[ qIdx ].answerData[ qIdx ].ID;
}

//----------------------------------------------------------------------------
/**
 * @brief 今日の回答人数を取得する
 *
 * @param 
 */
//----------------------------------------------------------------------------
u16 RESEARCH_DATA_GetTodayCountOfAnswer( const RESEARCH_DATA* data, u8 qIdx, u8 aIdx )
{
  // インデックスエラー
  GF_ASSERT( aIdx < RESEARCH_DATA_GetAnswerNumOfQuestion_byIndex(data, qIdx) );

  return data->questionData[ qIdx ].answerData[ qIdx ].todayCount;
}

//----------------------------------------------------------------------------
/**
 * @brief いままでの回答人数を取得する
 *
 * @param 
 */
//----------------------------------------------------------------------------
u16 RESEARCH_DATA_GetTotalCountOfAnswer( const RESEARCH_DATA* data, u8 qIdx, u8 aIdx )
{
  // インデックスエラー
  GF_ASSERT( aIdx < RESEARCH_DATA_GetAnswerNumOfQuestion_byIndex(data, qIdx) );

  return data->questionData[ qIdx ].answerData[ qIdx ].totalCount;
}


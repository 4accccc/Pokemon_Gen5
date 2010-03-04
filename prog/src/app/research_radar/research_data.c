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

  return data->questionData[ qIdx ].answerData[ aIdx ].ID;
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

  return data->questionData[ qIdx ].answerData[ aIdx ].todayCount;
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

  return data->questionData[ qIdx ].answerData[ aIdx ].totalCount;
}

//----------------------------------------------------------------------------
/**
 * @brief 回答の表示カラー ( R ) を取得する
 */
//----------------------------------------------------------------------------
extern u8 RESEARCH_DATA_GetColorR( const RESEARCH_DATA* data, u8 qIdx, u8 aIdx )
{
  // インデックスエラー
  GF_ASSERT( aIdx < RESEARCH_DATA_GetAnswerNumOfQuestion_byIndex(data, qIdx) );

  return data->questionData[ qIdx ].answerData[ aIdx ].colorR;
}

//----------------------------------------------------------------------------
/**
 * @breif 回答の表示カラー ( G ) を取得する
 */
//----------------------------------------------------------------------------
extern u8 RESEARCH_DATA_GetColorG( const RESEARCH_DATA* data, u8 qIdx, u8 aIdx )
{
  // インデックスエラー
  GF_ASSERT( aIdx < RESEARCH_DATA_GetAnswerNumOfQuestion_byIndex(data, qIdx) );

  return data->questionData[ qIdx ].answerData[ aIdx ].colorG;
}

//----------------------------------------------------------------------------
/**
 * @brief 回答の表示カラー ( B ) を取得する
 */
//----------------------------------------------------------------------------
extern u8 RESEARCH_DATA_GetColorB( const RESEARCH_DATA* data, u8 qIdx, u8 aIdx )
{
  // インデックスエラー
  GF_ASSERT( aIdx < RESEARCH_DATA_GetAnswerNumOfQuestion_byIndex(data, qIdx) );

  return data->questionData[ qIdx ].answerData[ aIdx ].colorB;
}


//----------------------------------------------------------------------------
/**
 * @brief 質問IDのインデックスを取得する ( IDを指定 )
 *
 * @param data
 * @param qID  質問ID
 *
 * @return 指定した質問データのインデックス
 */
//----------------------------------------------------------------------------
u8 RESEARCH_DATA_GetQuestionIndex_byID( const RESEARCH_DATA* data, u8 qID )
{
  int qIdx;

  // 指定された質問IDを検索
  for( qIdx=0; qIdx < QUESTION_NUM_PER_TOPIC; qIdx++ )
  {
    // 発見
    if( data->questionData[ qIdx ].ID == qID )
    {
      return qIdx;
    }
  }

  // 指定したIDは存在しない
  GF_ASSERT(0);
  return 0;
}

//----------------------------------------------------------------------------
/**
 * @brief 回答IDのインデックスを取得する ( IDを指定 )
 *
 * @param data
 * @param qID
 * @param aID
 *
 * @return 指定した質問に対する, 指定した回答データのインデックス
 */
//----------------------------------------------------------------------------
u8 RESEARCH_DATA_GetAnswerIndex_byID( const RESEARCH_DATA* data, u8 qID, u8 aID )
{
  int qIdx, aIdx;
  const QUESTION_DATA* qData;

  // 質問データを取得
  qIdx = RESEARCH_DATA_GetQuestionIndex_byID( data, qID );
  qData = &(data->questionData[ qIdx ]);

  // 指定された回答データを検索
  for( aIdx=0; aIdx < qData->answerNum; aIdx++ )
  {
    // 発見
    if( qData->answerData[ aIdx ].ID == aID )
    {
      return aIdx;
    }
  }

  // 指定された回答データは存在しない
  GF_ASSERT(0);
  return 0;
}

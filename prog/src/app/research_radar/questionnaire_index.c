/////////////////////////////////////////////////////////////////////////////
/**
 * @brief  アンケートの回答インデックス参照関数
 * @file   questionnaire_index.c
 * @author obata
 * @date   2010.02.15
 */
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>
#include "questionnaire_index.h"

#include "../../../../resource/research_radar/data/answer_id.h"              // for ANSWER_ID_xxxx
#include "../../../../resource/research_radar/data/question_id.h"            // for QUESTION_ID_xxxx
#include "../../../../resource/research_radar/data/questionnaire_index.cdat" // for QuestionnaireIndex
#include "../../../../resource/research_radar/data/answer_num_question.cdat" // for AnswerNum_question[]


//---------------------------------------------------------------------------
/**
 * @brief 回答の項目数を取得する
 *
 * @param questionID 質問ID
 *
 * @return 指定した質問の回答選択肢の数
 */
//---------------------------------------------------------------------------
u8 QUESTIONNAIRE_INDEX_GetAnswerNum( u8 questionID )
{
  // IDエラー
  if( QUESTION_ID_MAX < questionID ) {
    GF_ASSERT(0);
    return 0;
  }
  return QuestionnaireIndex[ questionID ].answerNum;
}

//---------------------------------------------------------------------------
/**
 * @brief 回答を表現するのに必要なビット数を取得する
 *
 * @param questionID 質問ID
 *
 * @return 指定した質問の回答を表現するのに必要なビット数
 */
//---------------------------------------------------------------------------
u8 QUESTIONNAIRE_INDEX_GetBitCount( u8 questionID )
{
  // IDエラー
  if( QUESTION_ID_MAX < questionID ) {
    GF_ASSERT(0);
    return 0;
  }
  return QuestionnaireIndex[ questionID ].bitCount;
}

//---------------------------------------------------------------------------
/**
 * @brief セーブデータのオフセット値を取得する ( 回答選択肢の数 )
 *
 * @param questionID 質問ID
 *
 * @return 指定した質問の回答選択肢の数にアクセスするためのオフセット
 */
//---------------------------------------------------------------------------
u8 QUESTIONNAIRE_INDEX_GetAnswerNumOffset( u8 questionID )
{
  // IDエラー
  if( QUESTION_ID_MAX < questionID ) {
    GF_ASSERT(0);
    return 0;
  }
  return QuestionnaireIndex[ questionID ].answerNumOffset;
}

//---------------------------------------------------------------------------
/**
 * @brief セーブデータのオフセット値を取得する ( ビット数 )
 *
 * @param questionID 質問ID
 *
 * @return 指定した質問の回答を表現するのに必要なビット数 に
 * 　　　　アクセスするためのオフセット
 */
//---------------------------------------------------------------------------
u8 QUESTIONNAIRE_INDEX_GetBitCountOffset( u8 questionID )
{
  // IDエラー
  if( QUESTION_ID_MAX < questionID ) {
    GF_ASSERT(0);
    return 0;
  }
  return QuestionnaireIndex[ questionID ].bitCountOffset;
}

//---------------------------------------------------------------------------
/**
 * @brief 指定した回答が所属する質問のIDを取得する
 *
 * @param answerID 回答ID ( ANSWER_ID_xxxx )
 *
 * @return 指定した回答が属する質問のID ( QUESTION_ID_xxxx )
 */
//---------------------------------------------------------------------------
u8 GetQuestionID_byAnswerID( u8 answerID )
{
  int sum;
  int qID;

  // IDエラー
  if( ANSWER_ID_MAX < answerID ) {
    GF_ASSERT(0);
    return 0;
  }

  sum = 0;
  for( qID=0; qID < QUESTION_ID_NUM; qID++ )
  {
    sum += AnswerNum_question[ qID ];
    if( answerID <= sum ) {
      return qID;
    }
  }

  GF_ASSERT(0);
  return 0;
}
//---------------------------------------------------------------------------
/**
 * @brief 指定した回答が, 所属する質問内の何番目の回答なのかを取得する
 *
 * @param answerID 回答ID ( ANSWER_ID_xxxx )
 *
 * @return 指定した回答が, 所属する質問内の何番目の回答なのか 
 *         ( 1 ～ MAX_ANSWER_NUM_PER_QUESTION )
 */
//---------------------------------------------------------------------------
u8 GetAnswerIndex_atQuestion( u8 answerID )
{
  int sum;
  int qID;

  // IDエラー
  if( ANSWER_ID_MAX < answerID ) {
    GF_ASSERT(0);
    return 1;
  }

  sum = 0;
  for( qID=0; qID < QUESTION_ID_NUM; qID++ )
  {
    sum += AnswerNum_question[ qID ];
    if( answerID <= sum ) {
      return sum - answerID + 1;  // 1 Origin
    }
  }

  GF_ASSERT(0);
  return 1;
}

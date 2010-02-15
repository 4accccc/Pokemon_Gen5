/////////////////////////////////////////////////////////////////////////////
/**
 * @brief  アンケートの回答インデックス参照関数
 * @file   questionnaire_index.c
 * @author obata
 * @date   2010.02.15
 */
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../../../resource/research_radar/data/question_id.h"             // for QUESTION_ID_xxxx
#include "../../../resource/research_radar/data/questionnaire_index.h"     // for QUESTIONNAIRE_INDEX
#include "../../../resource/research_radar/data/questionnaire_index.cdat"  // for QuestionnaireIndex


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
  if( QUESTION_ID_MAX < questionID )
  {
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
extern u8 QUESTIONNAIRE_INDEX_GetBitCount( u8 questionID )
{
  // IDエラー
  if( QUESTION_ID_MAX < questionID )
  {
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
  if( QUESTION_ID_MAX < questionID )
  {
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
extern u8 QUESTIONNAIRE_INDEX_GetBitCount( u8 questionID )
{
  // IDエラー
  if( QUESTION_ID_MAX < questionID )
  {
    GF_ASSERT(0);
    return 0;
  }
  return QuestionnaireIndex[ questionID ].bitCountOffset;
}

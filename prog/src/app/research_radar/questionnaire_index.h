///////////////////////////////////////////////////////////////////
/**
 * @brief  アンケート回答インデックス
 * @file   questionnaire_index.h
 * @author obata
 * @date   2010.02.15
 */
///////////////////////////////////////////////////////////////////
#pragma once


//=================================================================
// ■回答ワーク中の 特定の質問に対する回答にアクセスするための情報
//=================================================================
typedef struct
{
  u8 answerNum;        // 回答選択肢の数
  u8 bitCount;         // 回答を表現するのに必要なビット数
  u8 answerNumOffset;  // 項目数インデックス
  u8 bitCountOffset;   // bit数インデックス

} QUESTIONNAIRE_INDEX;


//=================================================================
// ■情報取得関数
//=================================================================

// 回答選択肢の数
extern u8 QUESTIONNAIRE_INDEX_GetAnswerNum( u8 questionID );

// 回答を表現するのに必要なビット数
extern u8 QUESTIONNAIRE_INDEX_GetBitCount( u8 questionID );

// セーブデータ オフセット ( 回答選択肢の数 )
extern u8 QUESTIONNAIRE_INDEX_GetAnswerNumOffset( u8 questionID );

// セーブデータ オフセット ( ビット数 )
extern u8 QUESTIONNAIRE_INDEX_GetBitCountOffset( u8 questionID );


//=================================================================
// ■ツール関数
//=================================================================

// 指定した回答が所属する質問のIDを取得する
extern u8 GetQuestionID_byAnswerID( u8 answerID );

// 指定した回答が, 所属する質問内の何番目の回答なのかを取得する
extern u8 GetAnswerIndex_atQuestion( u8 answerID );

/////////////////////////////////////////////////////////////////////////////
/**
 * @brief  アンケートの回答インデックス参照関数
 * @file   questionnaire_index.h
 * @author obata
 * @date   2010.02.15
 */
/////////////////////////////////////////////////////////////////////////////
#pragma once


// 回答選択肢の数
extern u8 QUESTIONNAIRE_INDEX_GetAnswerNum( u8 questionID );

// 回答を表現するのに必要なビット数
extern u8 QUESTIONNAIRE_INDEX_GetBitCount( u8 questionID );

// セーブデータ オフセット ( 回答選択肢の数 )
extern u8 QUESTIONNAIRE_INDEX_GetAnswerNumOffset( u8 questionID );

// セーブデータ オフセット ( ビット数 )
extern u8 QUESTIONNAIRE_INDEX_GetBitCountOffset( u8 questionID );

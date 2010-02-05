//==============================================================================
/**
 * @file    questionnaire_save.h
 * @brief   すれ違いアンケート用セーブデータのヘッダ
 * @author  matsuda
 * @date    2010.01.20(水)
 */
//==============================================================================
#pragma once


//==============================================================================
//  型定義
//==============================================================================
typedef struct _QUESTIONNAIRE_SAVE_WORK QUESTIONNAIRE_SAVE_WORK;

//==============================================================================
//  構造体定義
//==============================================================================
///アンケートの回答ワーク
typedef struct{
  u8 answer[10];            ///<回答(bit単位で各質問の回答が入っている)
}QUESTIONNAIRE_ANSWER_WORK;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern u32 QuestionnaireSave_GetWorkSize( void );
extern void QuestionnaireSave_WorkInit(void *work);

//--------------------------------------------------------------
//--------------------------------------------------------------
extern QUESTIONNAIRE_SAVE_WORK * SaveData_GetQuestionnaire(SAVE_CONTROL_WORK *sv);
extern QUESTIONNAIRE_ANSWER_WORK * Questionnaire_GetAnswerWork(QUESTIONNAIRE_SAVE_WORK *qsw);

//--------------------------------------------------------------
//--------------------------------------------------------------
extern u32 QuestionnaireWork_ReadBit(const QUESTIONNAIRE_ANSWER_WORK *qanswer, u32 start_bit, u32 bit_size);
extern void QuestionnaireWork_WriteBit(QUESTIONNAIRE_ANSWER_WORK *qanswer, u32 start_bit, u32 bit_size, u32 data);

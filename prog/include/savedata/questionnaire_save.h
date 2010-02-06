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
//  定数定義
//==============================================================================
///調査中の質問がない場合
#define INVESTIGATING_QUESTION_NULL     (0xff)


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
extern u32 QuestionnaireAnswer_ReadBit(const QUESTIONNAIRE_ANSWER_WORK *qanswer, u32 start_bit, u32 bit_size);
extern void QuestionnaireAnswer_WriteBit(QUESTIONNAIRE_ANSWER_WORK *qanswer, u32 start_bit, u32 bit_size, u32 data);

extern void QuestionnaireWork_SetInvestigatingQuestion(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id);
extern int QuestionnaireWork_GetInvestigatingQuestion(QUESTIONNAIRE_SAVE_WORK *qsw);
extern u16 QuestionnaireWork_GetTodayCount(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id);
extern void QuestionnaireWork_AddTodayCount(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int add_count);
extern u16 QuestionnaireWork_GetTotalCount(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id);
extern void QuestionnaireWork_AddTotalCount(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int add_count);
extern u8 QuestionnaireWork_GetTodayAnswerNum(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int answer_type);
extern void QuestionnaireWork_AddTodayAnswerNum(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int answer_type, int add_count);
extern u8 QuestionnaireWork_GetTotalAnswerNum(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int answer_type);
extern void QuestionnaireWork_AddTotalAnswerNum(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int answer_type, int add_count);
extern void QuestionnaireWork_DateChangeUpdate(QUESTIONNAIRE_SAVE_WORK *qsw);

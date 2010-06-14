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


///質問のバリエーション数
#define QUESTIONNAIRE_ITEM_NUM       (30)
///回答のバリエーション数(無回答を除く)
#define QUESTIONNAIRE_ANSWER_NUM     (150)

///本日の調査人数最大数
#define QUESTIONNAIRE_TODAY_COUNT_MAX   (9999)
///今までの調査人数最大数
#define QUESTIONNAIRE_TOTAL_COUNT_MAX   (999999)

///質問毎の本日の調査人数最大数
#define QUESTIONNAIRE_TODAY_ANSWER_COUNT_MAX   (99)
///質問毎の今までの調査人数最大数
#define QUESTIONNAIRE_TOTAL_ANSWER_COUNT_MAX   (9999)


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
extern u32 QuestionnaireAnswer_ReadBit(const QUESTIONNAIRE_ANSWER_WORK *qanswer, u8 question_id);
extern void QuestionnaireAnswer_WriteBit(QUESTIONNAIRE_ANSWER_WORK *qanswer, u8 question_id, u32 data);

extern void QuestionnaireWork_SetInvestigatingQuestion(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int work_index);
extern int QuestionnaireWork_GetInvestigatingQuestion(QUESTIONNAIRE_SAVE_WORK *qsw, int work_index);
extern u16 QuestionnaireWork_GetTodayCount(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id);
extern void QuestionnaireWork_AddTodayCount(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int add_count);
extern u32 QuestionnaireWork_GetTotalCount(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id);
extern void QuestionnaireWork_AddTotalCount(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int add_count);
extern BOOL QuestionnaireWork_IsTodayAnswerNumFull(QUESTIONNAIRE_SAVE_WORK* qsw, int question_id, int answer_type);
extern u8 QuestionnaireWork_GetTodayAnswerNum(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int answer_type);
extern void QuestionnaireWork_AddTodayAnswerNum(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int answer_type, int add_count);
extern u16 QuestionnaireWork_GetTotalAnswerNum(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int answer_type);
extern void QuestionnaireWork_AddTotalAnswerNum(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int answer_type, int add_count);
extern void QuestionnaireWork_DateChangeUpdate(QUESTIONNAIRE_SAVE_WORK *qsw);

//--------------------------------------------------------------
//--------------------------------------------------------------
#ifdef PM_DEBUG
extern void QuestionnaireWork_ClearResearch(QUESTIONNAIRE_SAVE_WORK *qsw); 
#endif

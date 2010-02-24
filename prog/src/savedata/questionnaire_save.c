//==============================================================================
/**
 * @file    questionnaire_save.c
 * @brief   すれ違いアンケート用セーブデータ
 * @author  matsuda
 * @date    2010.01.20(水)
 */
//==============================================================================
#include <gflib.h>
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/questionnaire_save.h"
#include "app/research_radar/questionnaire_index.h"
#include "app/research_radar/research_select_def.h"   //SELECT_TOPIC_MAX_NUM


//==============================================================================
//  定数定義
//==============================================================================
///質問のバリエーション数
#define QUESTIONNAIRE_ITEM_NUM       (30)
///回答のバリエーション数(無回答を除く)
#define QUESTIONNAIRE_ANSWER_NUM     (150)

///本日の調査人数最大数
#define QUESTIONNAIRE_TODAY_COUNT_MAX   (999)
///今までの調査人数最大数
#define QUESTIONNAIRE_TOTAL_COUNT_MAX   (99999)

///質問毎の本日の調査人数最大数
#define QUESTIONNAIRE_TODAY_ANSWER_COUNT_MAX   (99)
///質問毎の今までの調査人数最大数
#define QUESTIONNAIRE_TOTAL_ANSWER_COUNT_MAX   (9999)

//==============================================================================
//  構造体定義
//==============================================================================
///アンケートセーブワーク
struct _QUESTIONNAIRE_SAVE_WORK{
  u16 today_count[QUESTIONNAIRE_ITEM_NUM];        ///<今日の調査人数
  u8 today_answer[QUESTIONNAIRE_ANSWER_NUM];      ///<今日の答え(項目分)
  QUESTIONNAIRE_ANSWER_WORK mine_answer;          ///<自分の答え
  u32 total_count[QUESTIONNAIRE_ITEM_NUM];        ///<今までの調査人数の合計
  u16 total_answer[QUESTIONNAIRE_ANSWER_NUM];     ///<今までの答えの合計(項目分)
  
  u8 investigating_question_id[SELECT_TOPIC_MAX_NUM];    ///<調査中の質問ID
};



//==============================================================================
//
//  
//
//==============================================================================
//------------------------------------------------------------------
/**
 * セーブデータサイズを返す
 *
 * @retval  int		
 */
//------------------------------------------------------------------
u32 QuestionnaireSave_GetWorkSize( void )
{
	return sizeof(QUESTIONNAIRE_SAVE_WORK);
}

//--------------------------------------------------------------
/**
 * @brief   ワーク初期化
 *
 * @param   rec		
 */
//--------------------------------------------------------------
void QuestionnaireSave_WorkInit(void *work)
{
  QUESTIONNAIRE_SAVE_WORK *anketo_save = work;
  int work_index;
  
  GFL_STD_MemClear(anketo_save, sizeof(QUESTIONNAIRE_SAVE_WORK));
  for(work_index = 0; work_index < SELECT_TOPIC_MAX_NUM; work_index++){
    anketo_save->investigating_question_id[work_index] = INVESTIGATING_QUESTION_NULL;
  }
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * アンケートセーブワークへのポインタを取得
 *
 * @param   sv		セーブ制御システムへのポインタ
 *
 * @retval  QUESTIONNAIRE_SAVE_WORK *		アンケートセーブワークへのポインタ
 */
//==================================================================
QUESTIONNAIRE_SAVE_WORK * SaveData_GetQuestionnaire(SAVE_CONTROL_WORK *sv)
{
  return SaveControl_DataPtrGet(sv, GMDATA_ID_QUESTIONNAIRE);
}

//==================================================================
/**
 * 自分の回答ワークへのポインタを取得
 *
 * @param   qsw		アンケートセーブワークへのポインタ
 *
 * @retval  QUESTIONNAIRE_ANSWER_WORK *		回答ワークへのポインタ
 */
//==================================================================
QUESTIONNAIRE_ANSWER_WORK * Questionnaire_GetAnswerWork(QUESTIONNAIRE_SAVE_WORK *qsw)
{
  return &qsw->mine_answer;
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * アンケート回答を読み取る
 *
 * @param   qanswer		  回答ワークへのポインタ
 * @param   question_id 質問ID
 *
 * @retval  u32		回答
 *
 * 自分の回答データの読み取り、ビーコンに入っている回答データの読み取りに使用
 */
//==================================================================
u32 QuestionnaireAnswer_ReadBit(const QUESTIONNAIRE_ANSWER_WORK *qanswer, u8 question_id)
{
  u32 start_bit = QUESTIONNAIRE_INDEX_GetBitCountOffset(question_id);
  u32 bit_size = QUESTIONNAIRE_INDEX_GetBitCount(question_id);
  u32 buf_no = start_bit / 8;
  u32 buf_bit = start_bit % 8;
  u32 ret = 0;
  u32 count = 0;
  
  GF_ASSERT(bit_size < 32);
  
  while(bit_size){
    ret |= ((qanswer->answer[buf_no] >> buf_bit) & 1) << count;
    buf_bit++;
    if(buf_bit > 7){
      buf_no++;
      buf_bit = 0;
    }
    count++;
    bit_size--;
  }
  
  return ret;
}

//==================================================================
/**
 * アンケート回答を書き込む
 *
 * @param   qanswer		  回答ワークへのポインタ
 * @param   question_id 質問ID
 * @param   data		    回答
 * 
 * 自分の回答データの読み取り、ビーコンに入っている回答データの読み取りに使用
 */
//==================================================================
void QuestionnaireAnswer_WriteBit(QUESTIONNAIRE_ANSWER_WORK *qanswer, u8 question_id, u32 data)
{
  u32 start_bit = QUESTIONNAIRE_INDEX_GetBitCountOffset(question_id);
  u32 bit_size = QUESTIONNAIRE_INDEX_GetBitCount(question_id);
  u32 buf_no = start_bit / 8;
  u32 buf_bit = start_bit % 8;
  u32 ret = 0;
  u32 count = 0;
  
  GF_ASSERT(bit_size < 32);
  
  while(bit_size){
    qanswer->answer[buf_no] &= 0xff ^ (1 << buf_bit);  //まずクリア
    qanswer->answer[buf_no] |= (data & 1) << buf_bit;  //そしてセット
    buf_bit++;
    if(buf_bit > 7){
      buf_no++;
      buf_bit = 0;
    }
    data >>= 1;
    bit_size--;
  }
}

//==================================================================
/**
 * 調査中の質問IDをセット
 *
 * @param   qsw		        アンケートセーブワークへのポインタ
 * @param   question_id		質問ID
 * @param   work_index    IDをセットするワーク位置(0～SELECT_TOPIC_MAX_NUM)
 */
//==================================================================
void QuestionnaireWork_SetInvestigatingQuestion(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int work_index)
{
  GF_ASSERT(work_index < SELECT_TOPIC_MAX_NUM);
  qsw->investigating_question_id[work_index] = question_id;
}

//==================================================================
/**
 * 調査中の質問IDを取得する
 *
 * @param   qsw		アンケートセーブワークへのポインタ
 * @param   work_index    IDを取得するワーク位置(0～SELECT_TOPIC_MAX_NUM)
 *
 * @retval  int		調査中の質問ID (何も指定していない場合はINVESTIGATING_QUESTION_NULL)
 */
//==================================================================
int QuestionnaireWork_GetInvestigatingQuestion(QUESTIONNAIRE_SAVE_WORK *qsw, int work_index)
{
  GF_ASSERT(work_index < SELECT_TOPIC_MAX_NUM);
  return qsw->investigating_question_id[work_index];
}

//==================================================================
/**
 * 質問ID毎の今日の調査人数を取得
 *
 * @param   qsw		        アンケートセーブワークへのポインタ
 * @param   question_id		質問ID
 *
 * @retval  u16		調査人数
 */
//==================================================================
u16 QuestionnaireWork_GetTodayCount(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id)
{
  return qsw->today_count[question_id];
}

//==================================================================
/**
 * 質問ID毎の今日の調査人数を加算
 *
 * @param   qsw		        アンケートセーブワークへのポインタ
 * @param   question_id		質問ID
 * @param   add_count     加算数
 */
//==================================================================
void QuestionnaireWork_AddTodayCount(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int add_count)
{
  qsw->today_count[question_id] += add_count;
  if(qsw->today_count[question_id] > QUESTIONNAIRE_TODAY_COUNT_MAX){
    qsw->today_count[question_id] = QUESTIONNAIRE_TODAY_COUNT_MAX;
  }
}

//==================================================================
/**
 * 質問ID毎の今までの合計調査人数を取得
 *
 * @param   qsw		        アンケートセーブワークへのポインタ
 * @param   question_id		質問ID
 *
 * @retval  u16		調査人数
 */
//==================================================================
u16 QuestionnaireWork_GetTotalCount(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id)
{
  return qsw->total_count[question_id];
}

//==================================================================
/**
 * 質問ID毎の今までの合計調査人数を加算
 *
 * @param   qsw		        アンケートセーブワークへのポインタ
 * @param   question_id		質問ID
 * @param   add_count     加算数
 */
//==================================================================
void QuestionnaireWork_AddTotalCount(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int add_count)
{
  qsw->total_count[question_id] += add_count;
  if(qsw->total_count[question_id] > QUESTIONNAIRE_TOTAL_COUNT_MAX){
    qsw->total_count[question_id] = QUESTIONNAIRE_TOTAL_COUNT_MAX;
  }
}

//--------------------------------------------------------------
/**
 * 質問IDと項目Noから対象のバッファの要素番号を取得する
 *
 * @param   question_id		
 * @param   answer_type		
 *
 * @retval  u32		
 */
//--------------------------------------------------------------
static u32 _Get_AnswerNo(int question_id, int answer_type)
{
  u32 answer_no;
  
  answer_no = QUESTIONNAIRE_INDEX_GetAnswerNumOffset(question_id);
  answer_no += answer_type - 1; //無回答を除いたデータ構造になっているため、-1
  GF_ASSERT(answer_no < QUESTIONNAIRE_ANSWER_NUM);
  return answer_no;
}

//==================================================================
/**
 * 指定項目の答えた人数を取得：今日の答え
 *
 * @param   qsw		        アンケートセーブワークへのポインタ
 * @param   question_id		質問ID
 * @param   answer_type		質問ID毎の項目No ※0:回答0  1:回答1  2:回答2...
 *
 * @retval  u8		回答人数
 */
//==================================================================
u8 QuestionnaireWork_GetTodayAnswerNum(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int answer_type)
{
  if(answer_type == 0){ //回答0は無回答の為、固定で0を返す
    GF_ASSERT(0); //無回答は加算していくデータではない為、要求される事はおかしい
    return 0;
  }
  
  return qsw->today_answer[ _Get_AnswerNo(question_id, answer_type) ];
}

//==================================================================
/**
 * 指定項目の答えた人数を加算：今日の答え
 *
 * @param   qsw		        アンケートセーブワークへのポインタ
 * @param   question_id		質問ID
 * @param   answer_type		質問ID毎の項目No ※0:回答0  1:回答1  2:回答2...
 * @param   add_count     加算数
 */
//==================================================================
void QuestionnaireWork_AddTodayAnswerNum(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int answer_type, int add_count)
{
  u32 answer_no;

  if(answer_type == 0){
    GF_ASSERT(0); //無回答は加算していくデータではない為、要求される事はおかしい
    return;
  }

  answer_no = _Get_AnswerNo(question_id, answer_type);
  if(qsw->today_answer[answer_no] + add_count > QUESTIONNAIRE_TODAY_ANSWER_COUNT_MAX){
    qsw->today_answer[answer_no] = QUESTIONNAIRE_TODAY_ANSWER_COUNT_MAX;
  }
  else{
    qsw->today_answer[answer_no] += add_count;
  }
}

//==================================================================
/**
 * 指定項目の答えた人数を取得：今までの合計
 *
 * @param   qsw		        アンケートセーブワークへのポインタ
 * @param   question_id		質問ID
 * @param   answer_type		質問ID毎の項目No
 *
 * @retval  u8		回答人数
 */
//==================================================================
u8 QuestionnaireWork_GetTotalAnswerNum(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int answer_type)
{
  if(answer_type == 0){ //回答0は無回答の為、固定で0を返す
    GF_ASSERT(0); //無回答は加算していくデータではない為、要求される事はおかしい
    return 0;
  }
  
  return qsw->total_answer[ _Get_AnswerNo(question_id, answer_type) ];
}

//==================================================================
/**
 * 指定項目の答えた人数を加算：今までの合計
 *
 * @param   qsw		        アンケートセーブワークへのポインタ
 * @param   question_id		質問ID
 * @param   answer_type		質問ID毎の項目No
 * @param   add_count     加算数
 *
 * @retval  u8		回答人数
 */
//==================================================================
void QuestionnaireWork_AddTotalAnswerNum(QUESTIONNAIRE_SAVE_WORK *qsw, int question_id, int answer_type, int add_count)
{
  u32 answer_no;

  if(answer_type == 0){
    GF_ASSERT(0); //無回答は加算していくデータではない為、要求される事はおかしい
    return;
  }

  answer_no = _Get_AnswerNo(question_id, answer_type);
  if(qsw->total_answer[answer_no] + add_count > QUESTIONNAIRE_TOTAL_ANSWER_COUNT_MAX){
    qsw->total_answer[answer_no] = QUESTIONNAIRE_TOTAL_ANSWER_COUNT_MAX;
  }
  else{
    qsw->total_answer[answer_no] += add_count;
  }
}

//==================================================================
/**
 * 日付が変わった時のデータ反映処理
 *
 * @param   qsw		アンケートセーブワークへのポインタ
 */
//==================================================================
void QuestionnaireWork_DateChangeUpdate(QUESTIONNAIRE_SAVE_WORK *qsw)
{
  int i;
  
  for(i = 0; i < QUESTIONNAIRE_ITEM_NUM; i++){
    qsw->total_count[i] += qsw->today_count[i];
    if(qsw->total_count[i] > QUESTIONNAIRE_TOTAL_COUNT_MAX){
      qsw->total_count[i] = QUESTIONNAIRE_TOTAL_COUNT_MAX;
    }
    qsw->today_count[i] = 0;
  }
  for(i = 0; i < QUESTIONNAIRE_ANSWER_NUM; i++){
    qsw->total_answer[i] += qsw->today_answer[i];
    if(qsw->total_answer[i] > QUESTIONNAIRE_TOTAL_ANSWER_COUNT_MAX){
      qsw->total_answer[i] = QUESTIONNAIRE_TOTAL_ANSWER_COUNT_MAX;
    }
    qsw->today_answer[i] = 0;
  }
}



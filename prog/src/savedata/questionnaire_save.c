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


//==============================================================================
//  定数定義
//==============================================================================
///質問のバリエーション数
#define QUESTIONNAIRE_ITEM_NUM       (30)
///回答のバリエーション数
#define QUESTIONNAIRE_ANSWER_NUM     (150)


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
  
  GFL_STD_MemClear(anketo_save, sizeof(QUESTIONNAIRE_SAVE_WORK));
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
 * 回答ワークへのポインタを取得
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
 * 回答を読み取る
 *
 * @param   qanswer		  回答ワークへのポインタ
 * @param   start_bit		読み取り開始bit位置
 * @param   bit_size		読み取りbitサイズ
 *
 * @retval  u32		回答
 */
//==================================================================
u32 QuestionnaireWork_ReadBit(const QUESTIONNAIRE_ANSWER_WORK *qanswer, u32 start_bit, u32 bit_size)
{
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
}

//==================================================================
/**
 * 回答を書き込む
 *
 * @param   qanswer		  回答ワークへのポインタ
 * @param   start_bit		書き込み開始bit位置
 * @param   bit_size		書き込みbitサイズ
 * @param   data		    回答
 */
//==================================================================
void QuestionnaireWork_WriteBit(QUESTIONNAIRE_ANSWER_WORK *qanswer, u32 start_bit, u32 bit_size, u32 data)
{
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


//==============================================================================
/**
 * @file    radar_save.c
 * @brief   調査レーダーセーブデータ
 * @author  matsuda
 * @date    2010.01.06(水)
 */
//==============================================================================
#include <gflib.h>
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/radar_save.h"


//==============================================================================
//  定数定義
//==============================================================================
///トレーナーID保存数
#define RADAR_TRAINER_ID_MAX      (50)

///調査項目数
#define RADAR_ITEM_MAX            (30)

///今日の答え項目数
#define RADAR_TODAY_ANSWER_MAX    (150)


//==============================================================================
//  構造体定義
//==============================================================================
///調査レーダーセーブデータ
typedef struct{
  u32 trainer_id[RADAR_TRAINER_ID_MAX];       ///<トレーナーID
  
  //今日の統計データ
  u16 today_count[RADAR_ITEM_MAX];            ///<各30項目の今日の調査人数
  u8 today_answer[RADAR_TODAY_ANSWER_MAX];    ///<各30項目の今日の答え
  
  //総合の統計データ
  u32 total_today_count[RADAR_ITEM_MAX];      ///<各30項目の合計の調査人数
  u16 total_today_answer[RADAR_TODAY_ANSWER_MAX]; ///<各30項目の合計の答え150項目
  
  //今日の統計データ
  u8 mine_answer[RADAR_ITEM_MAX];             ///<自分の回答データ各30項目
}RADAR_SAVE_WORK;




//------------------------------------------------------------------
/**
 * セーブデータサイズを返す
 *
 * @retval  int		
 */
//------------------------------------------------------------------
u32 RadarSave_GetWorkSize( void )
{
	return sizeof(RADAR_SAVE_WORK);
}

//--------------------------------------------------------------
/**
 * @brief   ワーク初期化
 *
 * @param   rec		
 */
//--------------------------------------------------------------
void RadarSave_WorkInit(void *work)
{
  RADAR_SAVE_WORK *radar_save = work;
  
  GFL_STD_MemClear(radar_save, sizeof(RADAR_SAVE_WORK));
}


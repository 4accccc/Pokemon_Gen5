//======================================================================
/**
 * @file  pleasure_boat.c
 * @brief 遊覧船
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "pleasure_boat.h"
#include "system/main.h"  //for HEAPID_APP_CONTROL
#include "../../../resource/fldmapdata/script/c03r0801_def.h"  //for SCRID_～
#include "script.h"     //for SCRIPT_SetEventScript

#define WHISTLE_MARGINE (45)

#define PL_BOAT_MODE_TIME (10*30)    //180秒

#define NPC_TOTAL_NUM (15)

typedef struct PL_BOAT_WORK_tag
{
  BOOL WhistleReq;
  int Time;
  int WhistleCount;   //汽笛なった回数
}PL_BOAT_WORK;

typedef enum {
  PL_BOAT_EVT_NONE,
  PL_BOAT_EVT_MSG,
  PL_BOAT_EVT_WHISTLE,
  PL_BOAT_EVT_END,
}PL_BOAT_EVT;

static const u8 WeekTrNum[7] = {
  4,3,4,5,4,6,7
};

static PL_BOAT_EVT GetEvt(PL_BOAT_WORK_PTR work);


//--------------------------------------------------------------
/**
 * @brief	初期化関数
 * @param	void
 * @retval		PL_BOAT_WORK_PTR
 */
//--------------------------------------------------------------
PL_BOAT_WORK_PTR PL_BOAT_Init(void)
{
  PL_BOAT_WORK_PTR work;

  work = GFL_HEAP_AllocClearMemory(HEAPID_APP_CONTROL, sizeof(PL_BOAT_WORK));

  work->WhistleReq = FALSE;
  work->Time = 0;
  work->WhistleCount = 0;
  //イベント時間テーブルをロードするかも
  ;
  return work;
}

//--------------------------------------------------------------
/**
 * @brief	終了関数
 * @param	work      PL_BOAT_WORK_PTR
 * @retval		none
*/
//--------------------------------------------------------------
void PL_BOAT_End(PL_BOAT_WORK_PTR *work)
{
  if ( *work != NULL )
  {
    GFL_HEAP_FreeMemory( *work );
    *work = NULL;
  }
}

//--------------------------------------------------------------
/**
 * @brief	動作関数
 * @param	work      PL_BOAT_WORK_PTR
 * @retval		none
*/
//--------------------------------------------------------------
void PL_BOAT_Main(PL_BOAT_WORK_PTR work)
{
  if ( work == NULL ) return;

  //汽笛のリクエストがかかっている場合はＳＥ再生
  if (work->WhistleReq)
  {
    ;
    work->WhistleCount++;
    work->WhistleReq = FALSE;
  }
}

//--------------------------------------------------------------
/**
 * @brief	時間経過チェック
 * @param	 work      PL_BOAT_WORK_PTR
 * @retval GMEVENT *event    発生イベントポインタ	
*/
//--------------------------------------------------------------
GMEVENT *PL_BOAT_CheckEvt(GAMESYS_WORK *gsys, PL_BOAT_WORK_PTR work)
{
  PL_BOAT_EVT evt_type;
  GMEVENT *event;

  if ( work == NULL ) return NULL;

  //時間経過
  work->Time++;

  //イベント発生時間かをチェックする
  {
    BOOL rc;  //イベント発生フラグとして使用
    evt_type = GetEvt(work);
    switch(evt_type){
    case PL_BOAT_EVT_MSG:
      //船内アナウンス用スクリプトコール
      ;
      rc = TRUE;
      break;
    case PL_BOAT_EVT_WHISTLE:
      //汽笛リクエスト
      work->WhistleReq = TRUE;
      //汽笛はイベントで鳴らすわけではないのでリターンコードはFALSEを返す
      rc = FALSE;
      break;
    case PL_BOAT_EVT_END:
      //時間満了　終了スクリプトコール
      event = SCRIPT_SetEventScript( gsys, SCRID_PRG_C03R0801_TIMEUP, NULL, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
      rc = TRUE;
      break;
    default:
      rc = FALSE;
    }
  
    if (rc) return event;
  }

  return NULL;
}

//--------------------------------------------------------------
/**
 * @brief	強制的に時間経過させる　汽笛時間をまたいた場合は汽笛時間直前までの制限をかける
 * @param   work      PL_BOAT_WORK_PTR
 * @param   inAddSec  加算時間（秒）
 * @retval  none
*/
//--------------------------------------------------------------
void PL_BOAT_AddTimeEvt(PL_BOAT_WORK_PTR work, const int inAddSec)
{
  int margine = work->Time%(WHISTLE_MARGINE*30);
  OS_Printf("now_time = %d::%d SEC\n",work->Time, work->Time/30);
  //時間加算
  margine += (inAddSec*30);
  if ( margine >= WHISTLE_MARGINE*30 )
  {
    //汽笛が鳴る直前まで時間を進行
    work->Time = (work->WhistleCount+1)*(WHISTLE_MARGINE*30)-1;
  }
  else
  {
    work->Time += (inAddSec*30);
  }
  OS_Printf("add_after_time = %d::%d SEC\n",work->Time, work->Time/30);
}

//--------------------------------------------------------------
/**
 * @brief	イベント取得
 * @param	 work      PL_BOAT_WORK_PTR
 * @retval PL_BOAT_EVT    発生イベントタイプ
*/
//--------------------------------------------------------------
static PL_BOAT_EVT GetEvt(PL_BOAT_WORK_PTR work)
{
  //現在時間でイベントが発生するかを調べる
  if ( work->Time >= PL_BOAT_MODE_TIME ){
    return PL_BOAT_EVT_END;
  }

  if ( work->Time % (WHISTLE_MARGINE*30) == 0 ){
    return PL_BOAT_EVT_WHISTLE;
  }
  return PL_BOAT_EVT_NONE;
}

//--------------------------------------------------------------
/**
 * @brief	トレーナー抽選
 * @param	 work      PL_BOAT_WORK_PTR
 * @retval none
*/
//--------------------------------------------------------------
static void EntryTrainer(void)
{
  int tr_num;
  RTCDate date;
  RTC_GetDate( &date ); 
  //曜日別にトレーナー数を決定
  switch( date.week ){
  case RTC_WEEK_MONDAY:
    tr_num = WeekTrNum[0];
    break;
  case RTC_WEEK_TUESDAY:
    tr_num = WeekTrNum[1];
    break;
  case RTC_WEEK_WEDNESDAY:
    tr_num = WeekTrNum[2];
    break;
  case RTC_WEEK_THURSDAY:
    tr_num = WeekTrNum[3];
    break;
  case RTC_WEEK_FRIDAY:
    tr_num = WeekTrNum[4];
    break;
  case RTC_WEEK_SATURDAY:
    tr_num = WeekTrNum[5];
    break;
  case RTC_WEEK_SUNDAY:
    tr_num = WeekTrNum[6];
    break;
  default:
    GF_ASSERT(0);
    tr_num = 0;
  }


  //テーブルからトレーナをエントリ
  ;
  //テーブルから非トレーナーをエントリ
  ;
 
}


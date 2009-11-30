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

//#define FASE_MAX    (5)
//#define FIRST_FASE_TIME (10)
//#define FASE_TIME (45)

#define PL_BOAT_MODE_TIME (10*30)    //180秒

typedef struct PL_BOAT_WORK_tag
{
//  int NowFase;
//  int FaseRestTime;
  BOOL WhistleReq;
//  s64 BaseSec;

  int Time;
}PL_BOAT_WORK;

typedef enum {
  PL_BOAT_EVT_NONE,
  PL_BOAT_EVT_MSG,
  PL_BOAT_EVT_WHISTLE,
  PL_BOAT_EVT_END,
}PL_BOAT_EVT;

#if 0
static int GetDiffTime(PL_BOAT_WORK_PTR work);
static int GetNowFaseTime(PL_BOAT_WORK_PTR work);
#endif

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

//  work->NowFase = 0;
//  work->FaseRestTime = FIRST_FASE_TIME;
  work->WhistleReq = FALSE;
//  work->BaseSec = GFL_RTC_GetDateTimeBySecond();

  work->Time = 0;

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

#if 0
u32 PL_BOAT_GetMsgCntIdx()
{
  //現在時間から取得できるメッセージの管理番号を返す
  ;
}
#endif

#if 0
//--------------------------------------------------------------
/**
 * @brief	時間経過フラッシュ
 * @param	work      PL_BOAT_WORK_PTR
 * @retval	none
*/
//--------------------------------------------------------------
void PL_BOAT_FlushTime(PL_BOAT_WORK_PTR work)
{
  int diff;

  //現在までの消費時間算出
  diff = GetDiffTime(work);

  if (diff < work->FaseRestTime)
  {
    work->FaseRestTime -= diff;
  }
  else
  {
    work->FaseRestTime = 0;
  }
  //時間上書き
  work->BaseSec = GFL_RTC_GetDateTimeBySecond();
}

//--------------------------------------------------------------
/**
 * @brief	経過時間取得
 * @param	work      PL_BOAT_WORK_PTR
 * @retval	int     経過時間（時間差分）
*/
//--------------------------------------------------------------
static int GetDiffTime(PL_BOAT_WORK_PTR work)
{
  s64 sec;
  int diff;
  int fase_time;
  
  fase_time = GetNowFaseTime(work);
  sec = GFL_RTC_GetDateTimeBySecond();
  OS_Printf("sec = %d\n",sec);
  //差分計算
  diff = work->BaseSec - sec;
  //先祖がえりはありえないが念のため
  if (diff < 0){
    GF_ASSERT(0);
    diff = fase_time;
  }

  if (diff > fase_time) diff = fase_time;

  return diff;
}

//--------------------------------------------------------------
/**
 * @brief	現在のフェーズの最大経過秒数
 * @param	work      PL_BOAT_WORK_PTR
 * @retval	int     フェーズの最大秒数
*/
//--------------------------------------------------------------
static int GetNowFaseTime(PL_BOAT_WORK_PTR work)
{
  int fase_time;
  //現在のフェーズで分岐
  if (work->NowFase == 0) fase_time = FIRST_FASE_TIME;
  else fase_time = FASE_TIME;

  return fase_time;
}
#endif

static PL_BOAT_EVT GetEvt(PL_BOAT_WORK_PTR work)
{
  //現在時間でイベントが発生するかを調べる
  if ( work->Time >= PL_BOAT_MODE_TIME ){
    return PL_BOAT_EVT_END;
  }

  if ( work->Time % (36*30) == 0 ){
    return PL_BOAT_EVT_WHISTLE;
  }
  return PL_BOAT_EVT_NONE;
}

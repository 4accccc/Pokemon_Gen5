//======================================================================
/**
 * @file  pleasure_boat_st.c
 * @brief   遊覧船常駐部
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "gamesystem/game_event.h"
#include "pleasure_boat.h"
#include "pleasure_boat_def.h"

#include "arc/fieldmap/fldmmdl_objcode.h"  //for OBJCODE

#include "pl_boat_def.h"
#include "demo\demo3d.h"
#include "demo\demo3d_demoid.h"   //for DEMO3D_ID_～

typedef struct DEMO_EVT_WORK_tag
{
  DEMO3D_PARAM Param;
  PL_BOAT_WORK_PTR Ptr;
  u16 *RetWk;
}DEMO_EVT_WORK;


static GMEVENT_RESULT DemoCallEvt( GMEVENT* event, int* seq, void* work );


//--------------------------------------------------------------
/**
 * @brief　部屋の人物情報を取得　ＯＢＪコード
 * @note    ゾーンチェンジ時にスクリプトからコールされる
 * @param   work      PL_BOAT_WORK_PTR
 * @apram   inRoomIdx 部屋番号
 * @retval  int       ＯＢＪコード
*/
//--------------------------------------------------------------
int PL_BOAT_GetObjCode(PL_BOAT_WORK_PTR work, const int inRoomIdx)
{
  if ( work==NULL ) return BOY1;
  return work->RoomParam[inRoomIdx].ObjCode;
}

//--------------------------------------------------------------
/**
 * @brief　部屋の人物がダブルバトルするかをチェック
 * @param   work      PL_BOAT_WORK_PTR
 * @apram   inRoomIdx 部屋番号
 * @retval  BOOL       TRUEでダブルバトル
*/
//--------------------------------------------------------------
BOOL PL_BOAT_CheckDblBtl(PL_BOAT_WORK_PTR work, const int inRoomIdx)
{
  BOOL rc = FALSE;

  if ( work==NULL ) return rc;

  if ( (inRoomIdx == 2)&&(work->DblBtl[0]) ) rc = TRUE;
  else if ( (inRoomIdx == 7)&&(work->DblBtl[1]) ) rc = TRUE;
    
  return rc;
}

//--------------------------------------------------------------
/**
 * @brief   甲板デモコールイベント作成
 * @param   work      PL_BOAT_WORK_PTR
 * @retval  event     イベントポインタ
*/
//--------------------------------------------------------------
GMEVENT * PL_BOAT_CreateDemoEvt(GAMESYS_WORK *gsys, PL_BOAT_WORK_PTR work, u16 *ret_wk)
{
  DEMO_EVT_WORK *evt_work;
  GMEVENT *event;
  //イベント作成
  event = GMEVENT_Create( gsys, NULL, DemoCallEvt, sizeof(DEMO_EVT_WORK) );
  
  evt_work = GMEVENT_GetEventWork(event);
  evt_work->Ptr = work;
  evt_work->RetWk = ret_wk;
  //現在の船内経過時間を取得してデモパラメータにセット
  evt_work->Param.start_frame = PL_BOAT_GetTime(work);
  OS_Printf("start_frame = %d\n",evt_work->Param.start_frame);
  //デモＩＤセット
  evt_work->Param.demo_id = DEMO3D_ID_C_CRUISER;

  return event;
}

//--------------------------------------------------------------
/**
 * @brief	終了関数
 * @note  全滅処理から呼ばれる場合があるので、常駐領域におく
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
 * @brief   甲板デモコールイベント
 * @param	  event   イベントポインタ
 * @param   seq     シーケンサ
 * @param   work    ワークポインタ
 * @return  GMEVENT_RESULT  イベント結果
*/
//--------------------------------------------------------------
static GMEVENT_RESULT DemoCallEvt( GMEVENT* event, int* seq, void* work )
{
  DEMO_EVT_WORK *evt_work;
  evt_work = GMEVENT_GetEventWork(event);

  switch(*seq){
  case 0:
    //デモプロック
    GMEVENT_CallProc( event, FS_OVERLAY_ID(demo3d), &Demo3DProcData, &evt_work->Param );
    (*seq)++;
    break;
  case 1:
    //デモからデータ取得
    if ( evt_work->Param.result == DEMO3D_RESULT_FINISH )
      *(evt_work->RetWk) = PL_BOAT_DEMO_FINISHED;
    else *(evt_work->RetWk) = PL_BOAT_DEMO_USER_END;

    //経過時間セット
    PL_BOAT_SetTime(evt_work->Ptr, evt_work->Param.end_frame);

    OS_Printf("end_frame = %d\n",evt_work->Param.end_frame);
    //イベント終了
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief　時間取得
 * @note    アプリコールするときに呼ぶ
 * @param   work      PL_BOAT_WORK_PTR
 * @retval  int       経過時間(シンク)
*/
//--------------------------------------------------------------
int PL_BOAT_GetTime(PL_BOAT_WORK_PTR work)
{
  return work->Time;
}

//--------------------------------------------------------------
/**
 * @brief　時間セット
 * @note    アプリから戻ってくるときに呼ぶ
 * @param   work      PL_BOAT_WORK_PTR
 * @param   inTime    経過時間(シンク)
 * @retval  none
*/
//--------------------------------------------------------------
void PL_BOAT_SetTime(PL_BOAT_WORK_PTR work, const int inTime)
{
  work->Time = inTime;
}


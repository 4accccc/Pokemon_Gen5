//============================================================================================
/**
 * @file  event_demo3d.c
 * @brief Demo3Dアプリ呼び出しイベント
 * @date  2010.04.07
 * @author  iwasawa GAME FREAK inc.
 */
//============================================================================================
#include <gflib.h>

#include "gamesystem/game_event.h"
#include "gamesystem/game_comm.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "event_demo3d.h"
#include "demo/demo3d.h"  //Demo3DProcData etc.

typedef struct _EVWK_DEMO3D{
  GAMESYS_WORK* gsys;
  GAME_COMM_SYS_PTR gcsp;

  DEMO3D_PARAM  param;
}EVWK_DEMO3D;

static GMEVENT_RESULT CallDemo3DEvent(GMEVENT * event, int * seq, void *work);

//------------------------------------------------------------------
/*
 *  @brief  デモコールイベント起動
 *
 *  @param  gsys  GAMESYS_WORK*
 *  @param  demo_id デモID DEMO3D_ID_C_CRUISER他
 *  @param  scene_id  シーンID
 *  @param  use_rtc 時間をRTCから取るならTRUE,EV-TIMEから取るならFALSE
 */
//------------------------------------------------------------------
GMEVENT* EVENT_CallDemo3D( GAMESYS_WORK* gsys, u16 demo_id, u16 scene_id, BOOL use_rtc )
{
	GMEVENT * event = GMEVENT_Create( gsys, NULL, CallDemo3DEvent, sizeof(EVWK_DEMO3D));
	EVWK_DEMO3D * wk = GMEVENT_GetEventWork(event);
  
  MI_CpuClear8(wk,sizeof(EVWK_DEMO3D));
  wk->gsys = gsys;
  wk->gcsp = GAMESYSTEM_GetGameCommSysPtr( wk->gsys );
 
  if( use_rtc ){
    DEMO3D_PARAM_SetFromRTC( &wk->param, gsys, demo_id, scene_id );
  }else{
    DEMO3D_PARAM_SetFromEvTime( &wk->param, gsys, demo_id, scene_id );
  }

  if( demo_id == DEMO3D_ID_F_WHEEL ){
    GAMEBEACON_Set_FerrisWheel();
  }
  return event;
}

static GMEVENT_RESULT CallDemo3DEvent(GMEVENT * event, int * seq, void *work)
{
	EVWK_DEMO3D* wk = work;
  
  /*
   *  デモに通信処理負荷をかけないため、問答無用で通信切断
   *  フィールド再構築処理内で、通信は再度接続される
   */
  switch(*seq){
  case 0:
    if( GameCommSys_BootCheck( wk->gcsp ) == GAME_COMM_NO_NULL ){
      (*seq) += 2;
    }else{
      GameCommSys_ExitReq( wk->gcsp );
      (*seq) += 1;
    }
    break;
  case 1:
    if(GameCommSys_BootCheck( wk->gcsp) == GAME_COMM_NO_NULL){
      (*seq) += 1;
    }
    break;
  case 2:
    GAMESYSTEM_CallProc( wk->gsys, FS_OVERLAY_ID(demo3d), &Demo3DProcData, &wk->param );
    (*seq) += 1;
    break;
  case 3:
    // デモの終了を待つ
    if( GAMESYSTEM_IsProcExists( wk->gsys ) != GFL_PROC_MAIN_NULL ) {
      break;
    }
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}



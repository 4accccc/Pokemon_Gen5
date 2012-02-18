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
#include "gamesystem/pm_weather.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"
#include "sound/pm_sndsys.h"
#include "field/field_sound.h"
#include "field/weather_no.h"
#include "event_demo3d.h"
#include "demo/demo3d.h"  //Demo3DProcData etc.

typedef struct _EVWK_DEMO3D{
  GAMESYS_WORK* gsys;
  GAME_COMM_SYS_PTR gcsp;

  DEMO3D_PARAM  param;
  BOOL          net_off_flag;
  u16           demo_id;
}EVWK_DEMO3D;

static GMEVENT_RESULT CallDemo3DEvent(GMEVENT * event, int * seq, void *work);

//------------------------------------------------------------------
/*
 *  @brief  デモコールイベント起動
 *
 *  @param  gsys      GAMESYS_WORK*
 *  @param  parent    親イベントがある場合、そのポインタ
 *  @param  demo_id   デモID DEMO3D_ID_C_CRUISER他
 *  @param  scene_id  シーンID
 *  @param  use_rtc 時間をRTCから取るならTRUE,EV-TIMEから取るならFALSE
 */
//------------------------------------------------------------------
GMEVENT* EVENT_CallDemo3D( GAMESYS_WORK* gsys, GMEVENT* parent, u16 demo_id, u16 scene_id, BOOL use_rtc )
{
	GMEVENT * event = GMEVENT_Create( gsys, parent, CallDemo3DEvent, sizeof(EVWK_DEMO3D));
	EVWK_DEMO3D * wk = GMEVENT_GetEventWork(event);
  
  MI_CpuClear8(wk,sizeof(EVWK_DEMO3D));
  wk->gsys = gsys;
  wk->gcsp = GAMESYSTEM_GetGameCommSysPtr( wk->gsys );
  wk->demo_id = demo_id;

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
    wk->net_off_flag = GAMESYSTEM_GetNetOffEventFlag( wk->gsys );
    GAMESYSTEM_SetNetOffEventFlag( wk->gsys, TRUE );

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
    GAMESYSTEM_SetNetOffEventFlag( wk->gsys, wk->net_off_flag );

    /*
     * GFBTS683対策
     *
     * フェスBGMが鳴っている時にデモ内でBGMを止めてしまうと、
     * 曲が変わらないために次のマップBGM再生リクエストが無視される。
     * この対策として、デモ内でBGMを止める作りになっているフキヨセ・ヤマジの飛行機離陸デモ時は
     * このタイミングでBGMを一度SilentBGMに強制書き換えすることで
     * 次のマップBGM再生リクエストでBGMを復帰させる。
     * イベントフローとして、直後に必ずマップチェンジを通ることを前提としている。
     */
    if( wk->demo_id == DEMO3D_ID_PLANE_TAKE_OFF_FUKIYOSE ||
        wk->demo_id == DEMO3D_ID_PLANE_TAKE_OFF_YAMAJI ){
      GMEVENT_CallEvent( event, EVENT_FSND_PlayEventBGMSilent( wk->gsys, 0 ) );

      if( wk->demo_id  == DEMO3D_ID_PLANE_TAKE_OFF_YAMAJI ){
        /*
         * GFBTS915対策 
         * ヤマジ→フキヨセデモの時、ヤマジにSEを伴う天候があると
         * デモ終了直後のFieldOpenで天候が復帰し、マップチェンジ前に一瞬天候SEが鳴ってしまう。
         * この対策として、ヤマジ飛行機デモの際は、一度天候を晴れに強制書き換えする。
         * イベントフローとして、直後に必ずマップチェンジをはさむため
         * 強制書き換えした天候IDはマップチェンジにより正しく復帰する。
         */
        GAMEDATA_SetWeatherNo( GAMESYSTEM_GetGameData( wk->gsys ), WEATHER_NO_SUNNY );
      }
    }
    (*seq) += 1;
    break;
  case 4:
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}



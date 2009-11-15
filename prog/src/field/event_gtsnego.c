//============================================================================================
/**
 * @file     event_gtsnego.c
 * @brief    イベント：GTSネゴシエーション
 * @author   k.ohno
 * @date     2009.11.15
 */
//============================================================================================


#include <gflib.h>
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"
#include "sound/pm_sndsys.h"

#include "./event_fieldmap_control.h"
#include "field/event_gtsnego.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "poke_tool/monsno_def.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "net_app/gtsnego.h"

#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
//============================================================================================
//============================================================================================
FS_EXTERN_OVERLAY(pokemon_trade);

//----------------------------------------------------------------

#define HEAPID_CORE GFL_HEAPID_APP

enum _EVENT_GTSNEGO {
  _FIELD_CLOSE,
  _CALL_WIFICLUB,
  _WAIT_WIFICLUB,
  _WAIT_NET_END,
  _FIELD_OPEN,
  _FIELD_END
};


//============================================================================================
//
//    サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_GTSNegoMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_GTSNEGO_WORK * dbw = work;
  GAMESYS_WORK * gsys = dbw->gsys;


  switch (*seq) {
  case _FIELD_CLOSE:

    if(GAME_COMM_NO_NULL == GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gsys)))
    {
      dbw->isEndProc = FALSE;
      GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, dbw->fieldmap));
      (*seq)++;
    }


    break;
  case _CALL_WIFICLUB:
 //   GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(pokemon_trade), &GtsNego_ProcData, dbw);
    (*seq)++;
    break;
  case _WAIT_WIFICLUB:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){
      (*seq) ++;
    }
    break;
  case _WAIT_NET_END:
    if(GFL_NET_IsExit()){
      (*seq) ++;
    }
    break;
  case _FIELD_OPEN:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;
  case _FIELD_END:
    return GMEVENT_RES_FINISH;
  default:
    GF_ASSERT(0);
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

/*
 *  @brief  WiFiクラブ呼び出しパラメータセット
 */
static void wifi_SetEventParam( GMEVENT* event, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  BATTLE_SETUP_PARAM * para;
  EVENT_GTSNEGO_WORK * dbw;

  if(GAME_COMM_NO_NULL!= GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gsys))){
    GameCommSys_ExitReq(GAMESYSTEM_GetGameCommSysPtr(gsys));
  }

  dbw = GMEVENT_GetEventWork(event);
  dbw->ctrl = SaveControl_GetPointer();
  NET_PRINT("%x\n",(int)dbw->ctrl);
  dbw->gsys = gsys;
  dbw->fieldmap = fieldmap;
  dbw->event = event;

}

//------------------------------------------------------------------
/*
 *  @brief  WiFiクラブイベントコール
 */
//------------------------------------------------------------------
GMEVENT* EVENT_GTSNego( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap )
{
  GMEVENT * event = GMEVENT_Create(gsys, NULL, EVENT_GTSNegoMain, sizeof(EVENT_GTSNEGO_WORK));
  wifi_SetEventParam( event, gsys, fieldmap );

  return event;
}
//------------------------------------------------------------------
/*
 *  @brief  WiFiクラブイベントチェンジ
 */
//------------------------------------------------------------------
void EVENT_GTSNegoChange(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,GMEVENT * event)
{
  GMEVENT_Change( event, EVENT_GTSNegoMain, sizeof(EVENT_GTSNEGO_WORK) );
  wifi_SetEventParam( event, gsys, fieldmap );
}




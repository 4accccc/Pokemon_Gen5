//============================================================================================
/**
 * @file    event_fmission_app.c
 * @brief	  イベント：フェスミッション詳細画面呼び出しイベント
 * @author  iwasawa	
 * @date	  2011.04.25 
 */
//============================================================================================


#include <gflib.h>
#include "net/network_define.h"
#include "net/net_whpipe.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"

#include "./event_fieldmap_control.h"
#include "./event_fmission_app.h"

#include "sound/pm_sndsys.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "app/fmission_app.h"

//============================================================================================
//============================================================================================

FS_EXTERN_OVERLAY(fmission_app);
FS_EXTERN_OVERLAY(fieldmap);

enum _EVENT_FMISSION_APP {
  _FIELD_FADE_START,
  _FIELD_FADE_CLOSE,
  _PUSH_BGM,
  _CALL_FMISSION_APP,
  _WAIT_FMISSION_APP,
  _FIELD_OPEN,
  _FIELD_FADEIN,
  _BGM_WAIT,
  _FIELD_END,
};

typedef struct {
  GAMESYS_WORK * gsys;
  GAMEDATA * gdata;
  SAVE_CONTROL_WORK *save;
  FMISSION_APP_PARAM prm;
  FMISSION_APP_MODE mode;
} EVENT_FMISSION_APP_WORK;

//============================================================================================
//
//    サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_FMissionAppMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_FMISSION_APP_WORK * faw = work;
  GAMESYS_WORK * gsys = faw->gsys;
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK * fwork = GAMESYSTEM_GetFieldMapWork(gsys);

  switch (*seq) {
  case _FIELD_FADE_START:
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut_Black(gsys, fwork, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    //ビーコンの子→親切り替えをストップ(子の状態で固定リクエスト)
    GFL_NET_WL_PauseBeaconSend( TRUE );
    (*seq)++;
    break;
  case _FIELD_FADE_CLOSE:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, fwork));
    (*seq)++;
    break;
  case _PUSH_BGM:
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayNextBGM(gsys, SEQ_BGM_GAME_SYNC, FSND_FADE_FAST, FSND_FADE_NONE));
    (*seq)++;
    //break throw
  case _CALL_FMISSION_APP:
    MI_CpuClear8(&faw->prm,sizeof(FMISSION_APP_PARAM));
    faw->prm.gsys = faw->gsys;
    faw->prm.gdata = faw->gdata;
    faw->prm.mode = faw->mode;
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(fmission_app), &FMissionAppProcData, &faw->prm);
    (*seq)++;
    break;
  case _WAIT_FMISSION_APP:
    if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
      break;
    }
    GMEVENT_CallEvent(event, EVENT_FSND_PopBGMNoWait(gsys, FSND_FADE_FAST, FSND_FADE_NORMAL));
    (*seq)++;
    break;
  case _FIELD_OPEN:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq)++;
    break;
  case _FIELD_FADEIN:
    {
      GMEVENT* fade_event;

      if( faw->mode == FMISSION_APP_MODE_RESULT ){
        FIELD_SUBSCREEN_WORK* subscreen = FIELDMAP_GetFieldSubscreenWork(fwork);
        FIELD_SUBSCREEN_ChangeForce( subscreen, FIELD_SUBSCREEN_BEACON_VIEW );
      }
      
      fade_event = EVENT_FieldFadeIn_Black(gsys, fwork, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq) ++;
    break;
  case _BGM_WAIT:
    GMEVENT_CallEvent(event, EVENT_FSND_WaitBGMPop(gsys));
    (*seq) ++;
    break;
  case _FIELD_END: 
    //ビーコンの子→親切り替えを再開
    GFL_NET_WL_PauseBeaconSend( FALSE );
    return GMEVENT_RES_FINISH;
  default:
    GF_ASSERT(0);
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/*
 *  @param  gsys
 *  @param  FMISSION_APP_MODE*
 */
//------------------------------------------------------------------
GMEVENT* EVENT_FMissionApp(GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  EVENT_FMISSION_APP_WORK * faw;
  FMISSION_APP_MODE mode = *((FMISSION_APP_MODE*)work);

  event = GMEVENT_Create(gsys, NULL, EVENT_FMissionAppMain, sizeof(EVENT_FMISSION_APP_WORK));
  
  faw = GMEVENT_GetEventWork(event);
  faw->gdata = GAMESYSTEM_GetGameData(gsys);
  faw->save = GAMEDATA_GetSaveControlWork(faw->gdata);
  faw->gsys = gsys;
  faw->mode = mode;

  return event;
}



//============================================================================================
/**
 * @file    event_beacon_detail.c
 * @brief	  イベント：すれ違い詳細画面呼び出しイベント
 * @author  iwasawa	
 * @date	  2009.02.01
 */
//============================================================================================


#include <gflib.h>
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"

#include "./event_fieldmap_control.h"
#include "./event_beacon_detail.h"

#include "sound/pm_sndsys.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "app/beacon_detail.h"

//============================================================================================
//============================================================================================

FS_EXTERN_OVERLAY(beacon_detail);
FS_EXTERN_OVERLAY(fieldmap);

enum _EVENT_CG_WIRELESS {
  _FIELD_FADE_START,
  _FIELD_FADE_CLOSE,
  _PUSH_BGM,
  _CALL_BEACON_DETAIL,
  _WAIT_BEACON_DETAIL,
  _FIELD_FADEIN,
  _POP_BGM,
  _FIELD_END,
};

typedef struct {
  GAMESYS_WORK * gsys;
  GAMEDATA * gdata;
  SAVE_CONTROL_WORK *save;
  BEACON_DETAIL_PARAM prm;
  u8 target;
} EVENT_BEACON_DETAIL_WORK;

//============================================================================================
//
//    サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_BeaconDetailMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_BEACON_DETAIL_WORK * bdw = work;
  GAMESYS_WORK * gsys = bdw->gsys;
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK * fwork = GAMESYSTEM_GetFieldMapWork(gsys);

  switch (*seq) {
  case _FIELD_FADE_START:
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut_Black(gsys, fwork, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq)++;
    break;
  case _FIELD_FADE_CLOSE:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, fwork));
    (*seq)++;
    break;
  case _PUSH_BGM:
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayNextBGM(gsys, SEQ_BGM_GAME_SYNC, FSND_FADE_SHORT, FSND_FADE_NONE));
    (*seq)++;
    //break throw
  case _CALL_BEACON_DETAIL:
    MI_CpuClear8(&bdw->prm,sizeof(BEACON_DETAIL_PARAM));
    bdw->prm.gsys = bdw->gsys;
    bdw->prm.gdata = bdw->gdata;
    bdw->prm.target = bdw->target;
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(beacon_detail), &BeaconDetailProcData, &bdw->prm);
    (*seq)++;
    break;
  case _WAIT_BEACON_DETAIL:
    if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
      break;
    }
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq)++;
    break;
  case _FIELD_FADEIN:
    {
      GMEVENT* fade_event;
      FIELD_SUBSCREEN_WORK * subscreen;

      subscreen = FIELDMAP_GetFieldSubscreenWork(fwork);
/*
      if(bdw->selectType==CG_WIRELESS_RETURNMODE_PALACE){
        if(NULL==Intrude_Check_CommConnect(pComm)){ //つながってない
          FIELD_SUBSCREEN_SetAction( subscreen , FIELD_SUBSCREEN_ACTION_PALACE_WARP);
        }
      }
*/
      fade_event = EVENT_FieldFadeIn_Black(gsys, fwork, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq) ++;
    break;
  case _POP_BGM:
    GMEVENT_CallEvent(event, EVENT_FSND_PopBGM(gsys, FSND_FADE_SHORT, FSND_FADE_NORMAL));
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

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* EVENT_BeaconDetail(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, u8 target )
{
  GMEVENT* event;
  EVENT_BEACON_DETAIL_WORK * bdw;

  event = GMEVENT_Create(gsys, NULL, EVENT_BeaconDetailMain, sizeof(EVENT_BEACON_DETAIL_WORK));
  
  bdw = GMEVENT_GetEventWork(event);
  bdw->gdata = GAMESYSTEM_GetGameData(gsys);
  bdw->save = GAMEDATA_GetSaveControlWork(bdw->gdata);
  bdw->gsys = gsys;
  bdw->target = target;
  return event;
}



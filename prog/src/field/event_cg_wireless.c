//============================================================================================
/**
 * @file    event_cg_wireless.c
 * @brief	  イベント：パレスとTVトランシーバー
 * @author	k.ohno
 * @date	  2009.12.13
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
#include "./event_cg_wireless.h"

#include "sound/pm_sndsys.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "net_app/cg_wireless_menu.h"
#include "field_comm/intrude_work.h"
#include "field_comm/intrude_main.h"


//============================================================================================
//============================================================================================



FS_EXTERN_OVERLAY(cg_wireless);
FS_EXTERN_OVERLAY(fieldmap);


enum _EVENT_CG_WIRELESS {
  _FIELD_FADE_START,
  _FIELD_FADE_CLOSE,
  _CALL_CG_WIRELESS_MENU,
  _CALL_CG_WIRELESS_MENU2,
  _WAIT_CG_WIRELESS_MENU,
  _WAIT_TV,
  _FIELD_FADEOUT,
  _FIELD_OPEN,
  _FIELD_FADEIN,
  _POP_BGM,
  _FIELD_END,
};


//============================================================================================
//
//    サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_CG_WirelessMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_CG_WIRELESS_WORK * dbw = work;
  GAMESYS_WORK * gsys = dbw->gsys;
  GAME_COMM_SYS_PTR pComm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK * pFieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

  switch (*seq) {
  case _FIELD_FADE_START:
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut_Black(gsys, pFieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq) ++;
    break;
  case _FIELD_FADE_CLOSE:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, pFieldmap));
    (*seq) = _CALL_CG_WIRELESS_MENU;
    break;
  case _CALL_CG_WIRELESS_MENU:
    GMEVENT_CallEvent(event, EVENT_FieldSound_PushPlayEventBGM(gsys, SEQ_BGM_GAME_SYNC));
    dbw->push=TRUE;
    (*seq)++;
    //break throw
  case _CALL_CG_WIRELESS_MENU2:
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(cg_wireless), &CG_WirelessMenuProcData, dbw);
    (*seq)++;
    break;
  case _WAIT_CG_WIRELESS_MENU:
    if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
      break;
    }
    switch(dbw->selectType){
    case CG_WIRELESS_RETURNMODE_PALACE:
      if(Intrude_Check_CommConnect(pComm)){ //侵入通信が正常に繋がっているか調べる
        GAMEDATA_SetSubScreenMode(gdata, FIELD_SUBSCREEN_INTRUDE);
      }
      (*seq) = _FIELD_FADEOUT;
      break;
    case  CG_WIRELESS_RETURNMODE_TV:
      {
        GFL_PROC_SysCallProc(FS_OVERLAY_ID(comm_tvt), &COMM_TVT_ProcData, &dbw->aTVT);
        (*seq) = _WAIT_TV;
      }
      break;
    default:
      (*seq) = _FIELD_FADEOUT;
      break;
    }
    break;
  case _WAIT_TV:
    if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
      break;
    }
    (*seq)=_FIELD_FADEOUT;
    break;
  case _FIELD_FADEOUT:
    dbw->isEndProc = FALSE;
    (*seq)++;
    break;
  case _FIELD_OPEN:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;
  case _FIELD_FADEIN:
    {
      GMEVENT* fade_event;
      FIELD_SUBSCREEN_WORK * subscreen;

      subscreen = FIELDMAP_GetFieldSubscreenWork(pFieldmap);

      if(dbw->selectType==CG_WIRELESS_RETURNMODE_PALACE){
        if(NULL==Intrude_Check_CommConnect(pComm)){ //つながってない
          FIELD_SUBSCREEN_SetAction( subscreen , FIELD_SUBSCREEN_ACTION_PALACE_WARP);
        }
      }

      fade_event = EVENT_FieldFadeIn_Black(gsys, pFieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq) ++;
    break;
  case _POP_BGM:
    if(dbw->push){
      GMEVENT_CallEvent(event, EVENT_FieldSound_PopBGM(gsys, FSND_FADEOUT_NONE, FSND_FADEIN_SLOW));
      dbw->push=FALSE;
    }
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
GMEVENT* EVENT_CG_Wireless(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,GMEVENT * prevevent,BOOL bCreate)
{
  GMEVENT * event = prevevent;
  BATTLE_SETUP_PARAM * para;
  EVENT_CG_WIRELESS_WORK * dbw;

  if(bCreate){
    event = GMEVENT_Create(gsys, NULL, EVENT_CG_WirelessMain, sizeof(EVENT_CG_WIRELESS_WORK));
  }
  else{
    GMEVENT_Change( event,EVENT_CG_WirelessMain, sizeof(EVENT_CG_WIRELESS_WORK) );
  }
  dbw = GMEVENT_GetEventWork(event);
  dbw->gameData = GAMESYSTEM_GetGameData(gsys);
  dbw->ctrl = GAMEDATA_GetSaveControlWork(dbw->gameData);
  dbw->gsys = gsys;
  return event;
}



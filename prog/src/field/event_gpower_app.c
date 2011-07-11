//============================================================================================
/**
 * @file    event_gpower_app.c
 * @brief	  イベント：デルパワー使用画面呼び出しイベント
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
#include "./event_gpower_app.h"

#include "sound/pm_sndsys.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "item/itemsym.h"
#include "app/beacon_detail.h"
#include "app/gpower_app.h"

//============================================================================================
//============================================================================================

FS_EXTERN_OVERLAY(beacon_detail);
FS_EXTERN_OVERLAY(gpower_app);
FS_EXTERN_OVERLAY(fieldmap);

enum _EVENT_CG_WIRELESS {
  _FIELD_FADE_START,
  _FIELD_FADE_CLOSE,
  _PUSH_BGM,
  _CALL_GPOWER_APP,
  _WAIT_GPOWER_APP,
  _FIELD_OPEN,
  _FIELD_FADEIN,
  _BGM_WAIT,
  _FIELD_END,
};

typedef struct {
  GAMESYS_WORK * gsys;
  GAMEDATA * gdata;
  SAVE_CONTROL_WORK *save;
  BEACON_DETAIL_PARAM prm;
  GPOWER_APP_PARAM param;
  u8 target;
  GPOWER_APP_MODE mode;
} EVENT_GPOWER_APP_WORK;

//============================================================================================
//
//    サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_GPowerAppMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_GPOWER_APP_WORK * gaw = work;
  GAMESYS_WORK * gsys = gaw->gsys;
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
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayNextBGM(gsys, SEQ_BGM_GAME_SYNC, FSND_FADE_FAST, FSND_FADE_NONE));
    (*seq)++;
    //break throw
  case _CALL_GPOWER_APP:
#if 1
    MI_CpuClear8(&gaw->prm,sizeof(BEACON_DETAIL_PARAM));
    gaw->prm.gsys = gaw->gsys;
    gaw->prm.gdata = gaw->gdata;
    gaw->prm.target = 0;
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(beacon_detail), &BeaconDetailProcData, &gaw->prm);
#else
    gaw->param.mode = gaw->mode;
    {
      int i;
      INTRUDE_SAVE_WORK* isave = SaveData_GetIntrude( gaw->save );
      gaw->param.gpower_num = 0;
      for(i = 0;i < INTRUDE_EQUIP_GPOWER_MAX;i++){
        GPOWER_ID id = ISC_SAVE_GetGPowerID(isave,i);
        if( id != GPOWER_ID_NULL ){
          gaw->param.gpower[i] = id;
          ++gaw->param.gpower_num;
        }
      }
    }
    gaw->param.derudama_num = 
      MYITEM_GetItemNum( GAMEDATA_GetMyItem( gdata ), ITEM_DERUDAMA, GFL_HEAP_LOWID(HEAPID_PROC));

    GAMESYSTEM_CallProc( gsys, FS_OVERLAY_ID(gpower_app), &GPowerAppProcData, &gaw->param );
#endif
    (*seq)++;
    break;
  case _WAIT_GPOWER_APP:
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

      if( gaw->prm.ret_mode ){
        FIELD_SUBSCREEN_WORK* subscreen = FIELDMAP_GetFieldSubscreenWork(fwork);
        FIELD_SUBSCREEN_ChangeForce( subscreen, FIELD_SUBSCREEN_NORMAL );
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
    return GMEVENT_RES_FINISH;
  default:
    GF_ASSERT(0);
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* EVENT_GPowerApp(GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  EVENT_GPOWER_APP_WORK * gaw;
  u32 mode = *((u32*)work);

  event = GMEVENT_Create(gsys, NULL, EVENT_GPowerAppMain, sizeof(EVENT_GPOWER_APP_WORK));
  
  gaw = GMEVENT_GetEventWork(event);
  gaw->gdata = GAMESYSTEM_GetGameData(gsys);
  gaw->save = GAMEDATA_GetSaveControlWork(gaw->gdata);
  gaw->gsys = gsys;
  gaw->mode = mode;

  IWASAWA_Printf("GPowerApp Target = %d\n",mode);
  return event;
}



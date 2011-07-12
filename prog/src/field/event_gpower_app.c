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
#include "gamesystem/g_power.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"
#include "field_comm/intrude_work.h"
#include "./event_fieldmap_control.h"
#include "./event_gpower_app.h"
#include "./event_gpower.h"
#include "./event_move_hilink.h"

#include "sound/pm_sndsys.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "item/itemsym.h"
#include "app/gpower_app.h"

//============================================================================================
//============================================================================================

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
  _GPOWER_USE,
  _HILINK_JUMP,
  _FIELD_END,
};

typedef struct {
  GAMESYS_WORK * gsys;
  GAMEDATA * gdata;
  GPOWER_APP_PARAM param;
  u8 target;
  GPOWER_APP_MODE mode;
} EVENT_GPOWER_APP_WORK;

//============================================================================================
//   アプリコールパラメータセットアップ 
//============================================================================================
static void sub_CallParamSet( GPOWER_APP_PARAM* pp, int mode, GAMESYS_WORK* gsys, GAMEDATA* gdata, HEAPID heapID )
{
  int i,j;
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );

  pp->mode = mode;
  pp->derudama_num = 
    MYITEM_GetItemNum( GAMEDATA_GetMyItem( gdata ), ITEM_DERUDAMA, heapID );
  
  {
    INTRUDE_SAVE_WORK* isave = SaveData_GetIntrude( save );
    pp->gpower_num = 0;
    for(i = 0;i < INTRUDE_EQUIP_GPOWER_MAX;i++){
      GPOWER_ID id = ISC_SAVE_GetGPowerID(isave,i);
      if( id != GPOWER_ID_NULL ){
        pp->gpower[ pp->gpower_num++ ] = id;
      }
    }
  }
  for(i = 0;i < GPOWER_TYPE_MAX; i++){
    GPOWER_ID tmp = GPOWER_Check_OccurType(i);

    if( tmp == GPOWER_ID_NULL ){
      continue;
    }
    if( pp->occur_num >= GPOWER_OCCUR_LIST_MAX ){
      GF_ASSERT(0);
      break;
    }
    pp->occur_list[pp->occur_num].id = tmp;
    pp->occur_list[pp->occur_num++].life = GPOWER_Check_OccurTypeLife( i );
  }
  //寿命の長い順にソート
  for( i = 0;i < (pp->occur_num-1); i++ ){
    GPOWER_OCCUR_LIST tmp;
    for( j = 0; j < (pp->occur_num-1); j++ ){
      if( pp->occur_list[j-1].life > pp->occur_list[j].life ){
        tmp = pp->occur_list[j];
        pp->occur_list[j] = pp->occur_list[j-1];
        pp->occur_list[j-1] = tmp;
      }
    }
  }
  IWASAWA_Printf(" equip = %d, occur = %d\n", pp->gpower_num, pp->occur_num);
}

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
    sub_CallParamSet( &gaw->param, gaw->mode, gsys, gdata, GFL_HEAP_LOWID( HEAPID_PROC ) );
    GAMESYSTEM_CallProc( gsys, FS_OVERLAY_ID(gpower_app), &GPowerAppProcData, &gaw->param );
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

      if( gaw->param.ret_mode == GPOWER_APP_RET_COMM_OFF ){
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

    switch( gaw->param.ret_mode ){
    case GPOWER_APP_RET_GPOWER:
      (*seq) = _GPOWER_USE;
      break;
    case GPOWER_APP_RET_HILINK:
      (*seq) = _HILINK_JUMP;
      break;
    default:
      (*seq) = _FIELD_END;
      break;
    }
    break;
  case _GPOWER_USE:
    {
      GPOWER_EFFECT_PARAM param;
      GMEVENT* call_event;
      param.g_power = gaw->param.ret_gpower;
      param.mine_f = TRUE;
      call_event = GMEVENT_CreateOverlayEventCall( gsys,
          FS_OVERLAY_ID( event_gpower ), EVENT_GPowerEffectStart, &param );

      GMEVENT_ChangeEvent( event, call_event );
    }
    return GMEVENT_RES_CONTINUE;
  
  case _HILINK_JUMP:
    {
      u32 state = Intrude_GetIntrudeStatus( GAMESYSTEM_GetGameCommSysPtr(gsys) );
      GMEVENT_ChangeEvent( event, EVENT_MoveHilink( gsys, state ) );
    }
    return GMEVENT_RES_CONTINUE;

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
  gaw->gsys = gsys;
  gaw->mode = mode;

  IWASAWA_Printf("GPowerApp Target = %d\n",mode);
  return event;
}



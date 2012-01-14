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
  _CALL_FMISSION_APP,
  _WAIT_FMISSION_APP,
  _CLOSE_FMISSION_APP,
  _FIELD_OPEN,
  _FIELD_FADEIN,
  _FIELD_END,
};

typedef struct {
  GAMESYS_WORK * gsys;
  GAMEDATA * gdata;
  SAVE_CONTROL_WORK *save;
  FMISSION_APP_PARAM prm;
  FMISSION_APP_MODE  mode;
  BOOL               comm_f;  //常時通信中が正常に機能中か？
} EVENT_FMISSION_APP_WORK;

extern void sub_AppCallWorkInit( FMISSION_APP_PARAM* prm, GAMESYS_WORK * gsys, GAMEDATA* gdata, FMISSION_APP_MODE mode );

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
      if( faw->mode == FMISSION_APP_MODE_INFO ){
        fade_event = EVENT_FieldFadeOut_Black(gsys, fwork, FIELD_FADE_WAIT);
      }else{
        fade_event = EVENT_FieldFadeOut_White(gsys, fwork, FIELD_FADE_WAIT);
      }
      GMEVENT_CallEvent(event, fade_event);
    }
    if( faw->comm_f ){
      //ビーコンの子→親切り替えをストップ(子の状態で固定リクエスト)
      GFL_NET_WL_PauseBeaconSend( TRUE );
    }
    (*seq)++;
    break;
  case _FIELD_FADE_CLOSE:
    if( faw->mode == FMISSION_APP_MODE_RESULT ){
      //ミッションギミック削除処理
      FES_GIMMICK_Delete( FIELDMAP_GetFesGimmickSys( fwork ) );
    }
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, fwork));
    (*seq)++;
    break;
  case _CALL_FMISSION_APP:
    FSND_HoldBGMVolume_forApp(
      GAMEDATA_GetFieldSound(gdata), GAMESYSTEM_GetIssSystem(gsys) );

    sub_AppCallWorkInit( &faw->prm, faw->gsys, faw->gdata, faw->mode );
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(fmission_app), &FMissionAppProcData, &faw->prm);
    (*seq)++;
    break;
  case _WAIT_FMISSION_APP:
    if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
      break;
    }
    (*seq)++;
    break;
  case _CLOSE_FMISSION_APP:
    {
      u32 bgm = FES_BGM_RECOVER_NONE;
      if( faw->mode == FMISSION_APP_MODE_RESULT ){
        //ミッション終了処理
        bgm = LINK_FESTIVAL_MissionEnd( GAMESYSTEM_GetLinkFestivalWork( faw->gsys ) );
      }
      if( bgm != FES_BGM_RECOVER_NONE ){
        //フェスBGMだったらBGMを元に戻す
        GMEVENT_CallEvent( event, EVENT_FSND_PlayEventBGM( faw->gsys, bgm ));
      }
    }
    (*seq)++;
    break;
  case _FIELD_OPEN:
    FSND_ReleaseBGMVolume_fromApp(
      GAMEDATA_GetFieldSound(gdata), GAMESYSTEM_GetIssSystem(gsys) );
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq)++;
    break;
  case _FIELD_FADEIN:
    {
      GMEVENT* fade_event;

      if( faw->mode == FMISSION_APP_MODE_INFO ){
        fade_event = EVENT_FieldFadeIn_Black(gsys, fwork, FIELD_FADE_WAIT);
      }else{
        FIELD_SUBSCREEN_WORK* subscreen = FIELDMAP_GetFieldSubscreenWork(fwork);
        FIELD_SUBSCREEN_ChangeForce( subscreen, FIELD_SUBSCREEN_NORMAL );
        fade_event = EVENT_FieldFadeIn_White(gsys, fwork, FIELD_FADE_WAIT);
      }
      
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq) ++;
    break;
  case _FIELD_END: 
    if( faw->comm_f ){
      //ビーコンの子→親切り替えを再開
      GFL_NET_WL_PauseBeaconSend( FALSE );
    }
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

  //このイベントが呼び出された時点での通信状態を取得
  {
    GAME_COMM_SYS_PTR comm_sys = GAMESYSTEM_GetGameCommSysPtr(gsys);
    if( ( FIELDCOMM_CheckCommNo( comm_sys ) == GAME_COMM_NO_FIELD_BEACON_SEARCH ) &&
        ( GameCommSys_CheckSystemWaiting( comm_sys ) == FALSE )){
      faw->comm_f = TRUE;
    }else{
      faw->comm_f = FALSE;
    }
  }

  return event;
}

//------------------------------------------------------------------
/*
 *  @param  gsys
 *  @param  FMISSION_APP_MODE*
 */
//------------------------------------------------------------------
void sub_AppCallWorkInit( FMISSION_APP_PARAM* prm, GAMESYS_WORK * gsys, GAMEDATA* gdata, FMISSION_APP_MODE mode )
{
  SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork(gdata);

  MI_CpuClear8(prm,sizeof(FMISSION_APP_PARAM));
  prm->gsys = gsys;
  prm->gdata = gdata;
  prm->mode = mode;

  prm->fest_sys =  GAMESYSTEM_GetLinkFestivalWork( gsys );
  prm->fmwk = (const FEST_MISSION*)LINK_FESTIVAL_GetMissionWork( prm->fest_sys );
  prm->fmission_sv = SAVEDATA_GetFMissionSave(save);
}

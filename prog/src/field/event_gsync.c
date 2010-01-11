//============================================================================================
/**
 * @file    event_gsync.c
 * @brief   イベント：ゲームシンク
 * @author  k.ohno
 * @date    2009.11.15
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
#include "./event_gsync.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "net_app/irc_match/ircbattlematch.h"
#include "net_app/irc_match/ircbattlemenu.h"
#include "net_app/friend_trade/ircbattlefriend.h"
#include "net_app/pokemontrade.h"
#include "net_app/gsync.h"

#include "net/dwc_raputil.h"

#include "sound/pm_sndsys.h"
#include "battle/battle.h"
#include "poke_tool/monsno_def.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"

#include "savedata/dreamworld_data.h"



//============================================================================================
//============================================================================================



FS_EXTERN_OVERLAY(gamesync);
FS_EXTERN_OVERLAY(fieldmap);
FS_EXTERN_OVERLAY(wifi_login);


#define _LOCALMATCHNO (100)

enum _EVENT_IRCBATTLE {
  _FIELD_FADE_START,
  _FIELD_FADE_CLOSE,
  _CALL_GAMESYNC_MENU,
  _WAIT_GAMESYNC_MENU,
  _FIELD_FADEOUT,
  _CALL_IRCBATTLE_MATCH,
  _WAIT_IRCBATTLE_MATCH,
  _BATTLE_MATCH_START,
  _TIMING_SYNC_CALL_BATTLE,
  _FIELD_OPEN,
  _FIELD_FADEIN,
  _FIELD_END,
  _FADEIN_WIFIUTIL,
  _CALL_GAMESYNC,
  _WAIT_GAMESYNCLOGIN,
  _GAMESYNC_MAINPROC,
  _WAIT_GAMESYNC_MAINPROC,
  _GAMESYNC_CALLBOX,
  _GAMESYNC_CALLBOX_WAIT,
};


//============================================================================================
//
//    サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_GSyncMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_GSYNC_WORK * dbw = work;
  GAMESYS_WORK * gsys = dbw->gsys;
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
    (*seq) = _CALL_GAMESYNC_MENU;
    break;
  case _CALL_GAMESYNC_MENU:
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(gamesync), &GameSyncMenuProcData, dbw);
    (*seq)++;
    break;
  case _WAIT_GAMESYNC_MENU:
    if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
      break;
    }
    if(dbw->selectType == GAMESYNC_RETURNMODE_SYNC )
    {
      *seq = _CALL_GAMESYNC;
    }
    else if(dbw->selectType == GAMESYNC_RETURNMODE_UTIL )
    {
      *seq = _FIELD_FADEOUT;
    }
    else{
      *seq = _FIELD_OPEN;
    }
    break;

  case _FIELD_FADEOUT:
    (*seq)++;
    break;
  case _CALL_IRCBATTLE_MATCH:
    
    dbw->PlayBGM = PMSND_GetBGMsoundNo();
    PMSND_Exit();
    
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(wifi_util), &WifiUtilGSyncProcData, dbw);
    (*seq)++;
    break;
  case _WAIT_IRCBATTLE_MATCH:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){
      *seq = _FADEIN_WIFIUTIL;
    }
    break;
  case _FADEIN_WIFIUTIL:
		OS_EnableIrq();
   	PMSND_Init();
    FIELD_SOUND_PlayBGM(dbw->PlayBGM);
    GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_WHITEOUT, 16, 0, 1);
    (*seq) = _CALL_GAMESYNC_MENU;
    break;
  case _FIELD_OPEN:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;
  case _FIELD_FADEIN:
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black(gsys, pFieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq) ++;
    break;
  case _FIELD_END:
    if(dbw->push){
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
      FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
      FIELD_SOUND_PopBGM( fsnd );
      dbw->push=FALSE;
    } 
    return GMEVENT_RES_FINISH;
    break;
  case _CALL_GAMESYNC:
    {
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
      FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
      FIELD_SOUND_PushPlayEventBGM( fsnd, SEQ_BGM_GAME_SYNC );
      dbw->push=TRUE;
    }
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(wifi_login), &WiFiLogin_ProcData, &dbw->aLoginWork);
    (*seq)++;
    break;
  case _WAIT_GAMESYNCLOGIN:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){
      if(dbw->aLoginWork.result == WIFILOGIN_RESULT_CANCEL){
        *seq = _FIELD_OPEN;
      }
      else if(FALSE==DREAMWORLD_SV_GetSleepPokemonFlg(DREAMWORLD_SV_GetDreamWorldSaveData(dbw->ctrl))){
        dbw->selectType = GSYNC_CALLTYPE_POKELIST; 
        (*seq)=_GAMESYNC_MAINPROC;
      }
      else{
        dbw->selectType = GSYNC_CALLTYPE_INFO;  
        (*seq)=_GAMESYNC_MAINPROC;
      }
    }
    break;
 case _GAMESYNC_MAINPROC:
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(gamesync), &G_SYNC_ProcData, dbw);
    (*seq)++;
    break;
  case _WAIT_GAMESYNC_MAINPROC:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){
      if(dbw->selectType==GAMESYNC_RETURNMODE_BOXJUMP){
        (*seq)=_GAMESYNC_CALLBOX;
      }
      else if(dbw->selectType==GAMESYNC_RETURNMODE_EXIT){
        GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
        (*seq)=_CALL_GAMESYNC_MENU;
        if(dbw->push){
          GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
          FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
          FIELD_SOUND_PopBGM( fsnd );
          dbw->push=FALSE;
        }
      }
    }
    break;
  case _GAMESYNC_CALLBOX:
    dbw->boxParam.gamedata = GAMESYSTEM_GetGameData( gsys );
    dbw->boxParam.sv_box = GAMEDATA_GetBoxManager(dbw->boxParam.gamedata);
    dbw->boxParam.pokeparty = GAMEDATA_GetMyPokemon(dbw->boxParam.gamedata);			// 手持ちモケモン
    dbw->boxParam.myitem = GAMEDATA_GetMyItem(dbw->boxParam.gamedata);					// 所持アイテム（バッグで使用）
    dbw->boxParam.mystatus = GAMEDATA_GetMyStatus(dbw->boxParam.gamedata);				// プレイヤーデータ（バッグで使用）
    dbw->boxParam.cfg = SaveData_GetConfig(GAMEDATA_GetSaveControlWork(dbw->boxParam.gamedata));								// コンフィグデータ
    dbw->boxParam.zknMode = 0;								// 図鑑ナンバー表示モード
    dbw->boxParam.callMode = BOX_MODE_SLEEP;	// 寝かせる;					// 呼び出しモード
    GFL_PROC_SysCallProc( FS_OVERLAY_ID(box), &BOX2_ProcData, &dbw->boxParam );
    (*seq)++;
    break;
  case _GAMESYNC_CALLBOX_WAIT:
    dbw->boxNo = dbw->boxParam.retTray;      		// 終了時に開いていたトレイ（寝かせる用）
    dbw->boxIndex = dbw->boxParam.retPoke;   		// 終了時に選択された位置（寝かせる用
    if((BOX_RET_SEL_NONE==dbw->boxParam.retTray) && (BOX_RET_SEL_NONE==dbw->boxParam.retPoke)){
      GFL_NET_Exit(NULL);
      *seq = _FIELD_OPEN;
    }
    else{
      dbw->selectType = GSYNC_CALLTYPE_BOXSET;  // ポケモンセット後
      (*seq) = _GAMESYNC_MAINPROC;
    }
    break;
  default:
    GF_ASSERT(0);
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* EVENT_GSync(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,GMEVENT * prevevent,BOOL bCreate)
{
  GMEVENT * event = prevevent;
  BATTLE_SETUP_PARAM * para;
  EVENT_GSYNC_WORK * dbw;

  if(bCreate){
    event = GMEVENT_Create(gsys, NULL, EVENT_GSyncMain, sizeof(EVENT_GSYNC_WORK));
  }
  else{
    GMEVENT_Change( event,EVENT_GSyncMain, sizeof(EVENT_GSYNC_WORK) );
  }
  dbw = GMEVENT_GetEventWork(event);
  dbw->gameData = GAMESYSTEM_GetGameData(gsys);
  dbw->ctrl = GAMEDATA_GetSaveControlWork(dbw->gameData);
  dbw->gsys = gsys;
  dbw->aLoginWork.gamedata = GAMESYSTEM_GetGameData(gsys);
  dbw->aLoginWork.bDreamWorld = TRUE;
  return event;
}



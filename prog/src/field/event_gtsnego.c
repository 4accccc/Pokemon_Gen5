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

#include "net_app/pokemontrade.h"
#include "field/fieldmap.h"
#include "sound/pm_sndsys.h"

#include "./event_fieldmap_control.h"
#include "field/event_gtsnego.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "poke_tool/monsno_def.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "net_app/gtsnego.h"
#include "net_app/wifi_login.h"

#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
//============================================================================================
//============================================================================================
FS_EXTERN_OVERLAY(pokemon_trade);

//----------------------------------------------------------------

#define HEAPID_CORE GFL_HEAPID_APP

enum _EVENT_GTSNEGO {
  _FIELD_CLOSE,
  _CALL_WIFILOGIN,
  _WAIT_WIFILOGIN,
  _CALL_WIFINEGO,
  _WAIT_WIFINEGO,
  _CALL_TRADE,
  _WAIT_TRADE,
  _WAIT_NET_END,
  _FIELD_OPEN,
  _FIELD_END
};

//-------------------------------------
///	EVENT_GTSNEGO用接続ワーク
//=====================================
typedef struct 
{
  GAMESYS_WORK        *gsys;
  FIELDMAP_WORK       *fieldmap;
  EVENT_GTSNEGO_WORK  gts;
  POKEMONTRADE_PARAM aPokeTr;
  WIFILOGIN_PARAM     login;
} EVENT_GTSNEGO_LINK_WORK;



//============================================================================================
//
//    サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_GTSNegoMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_GTSNEGO_LINK_WORK * dbw = work;
  GAMESYS_WORK * gsys = dbw->gsys;


  switch (*seq) {
  case _FIELD_CLOSE:
    if(GAME_COMM_NO_NULL == GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gsys)))
    {
      GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, dbw->fieldmap));
      dbw->fieldmap=NULL; //クローズしたら使用禁止！
      (*seq)++;
    }
    break;
  case _CALL_WIFILOGIN:
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(wifi_login), &WiFiLogin_ProcData, &dbw->login);
    (*seq)++;
    break;
  case _WAIT_WIFILOGIN:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){
      if( dbw->login.result == WIFILOGIN_RESULT_LOGIN )
      { 
        (*seq) ++;
        NAGI_Printf( "ログインした\n" );
      }
      else
      { 
        (*seq)  = _WAIT_NET_END;
        NAGI_Printf( "ログインしなかった\n" );
      }
    }
    break;
  case _CALL_WIFINEGO:
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(gts_negotiate), &GtsNego_ProcData, &dbw->gts);
    (*seq)++;
    break;
  case _WAIT_WIFINEGO:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){
      (*seq) ++;
    }
    break;
  case _CALL_TRADE:
    dbw->aPokeTr.gsys=gsys;
    dbw->aPokeTr.type = POKEMONTRADE_GTSNEGO;
    dbw->aPokeTr.pNego = &dbw->gts;
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(pokemon_trade), &PokemonTradeWiFiProcData, &dbw->aPokeTr);
    (*seq)++;
    break;
  case _WAIT_TRADE:
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
    GFL_HEAP_FreeMemory(dbw->gts.pStatus[0]);
    GFL_HEAP_FreeMemory(dbw->gts.pStatus[1]);
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
  EVENT_GTSNEGO_LINK_WORK * dbw;

  if(GAME_COMM_NO_NULL!= GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gsys))){
    GameCommSys_ExitReq(GAMESYSTEM_GetGameCommSysPtr(gsys));
  }

  dbw = GMEVENT_GetEventWork(event);
  dbw->gsys = gsys;
  dbw->fieldmap = fieldmap;

  //GTSNEGO引数の設定
  {
    GFL_STD_MemClear( &dbw->gts, sizeof(EVENT_GTSNEGO_WORK) );
    dbw->gts.gsys = gsys;
    dbw->gts.ctrl = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gsys));
    dbw->gts.pStatus[0] = GFL_HEAP_AllocClearMemory(HEAPID_PROC,MyStatus_GetWorkSize());
    dbw->gts.pStatus[1] = GFL_HEAP_AllocClearMemory(HEAPID_PROC,MyStatus_GetWorkSize());
    {
      MYSTATUS * pMy =GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gsys));
      GFL_STD_MemCopy(pMy,dbw->gts.pStatus[0], MyStatus_GetWorkSize());
    }
  }
  //WIFI引数の設定
  { 
    GFL_STD_MemClear( &dbw->login, sizeof(WIFILOGIN_PARAM) );
    dbw->login.gsys = gsys;
    dbw->login.gamedata = GAMESYSTEM_GetGameData(gsys);
  }
}

//------------------------------------------------------------------
/*
 *  @brief  WiFiクラブイベントコール
 */
//------------------------------------------------------------------
GMEVENT* EVENT_GTSNego( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap )
{
  GMEVENT * event = GMEVENT_Create(gsys, NULL, EVENT_GTSNegoMain, sizeof(EVENT_GTSNEGO_LINK_WORK));
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
  GMEVENT_Change( event, EVENT_GTSNegoMain, sizeof(EVENT_GTSNEGO_LINK_WORK) );
  wifi_SetEventParam( event, gsys, fieldmap );
}




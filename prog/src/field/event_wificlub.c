//============================================================================================
/**
 * @file  event_wificlub.c
 * @brief イベント：WiFiクラブ
 * @author  k.ohno
 * @date  2009.03.31
 */
//============================================================================================


#include <gflib.h>
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"
#include "sound/pm_sndsys.h"  //SEQ_BA_TEST_250KB

#include "./event_fieldmap_control.h"
#include "field/event_wificlub.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "battle/battle.h"   // BATTLE_SETUP_PARAM
#include "poke_tool/monsno_def.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
//============================================================================================
//============================================================================================
FS_EXTERN_OVERLAY(wificlub);
FS_EXTERN_OVERLAY(battle);
FS_EXTERN_OVERLAY(ircbattlematch);
extern const GFL_PROC_DATA WifiClubProcData;

//----------------------------------------------------------------
// バトル用定義
extern const NetRecvFuncTable BtlRecvFuncTable[];
//----------------------------------------------------------------

#define HEAPID_CORE GFL_HEAPID_APP

enum _EVENT_IRCBATTLE {
  _FIELD_CLOSE,
  _CALL_WIFICLUB,
  _WAIT_WIFICLUB,
  _WAIT_NET_END,
  _FIELD_OPEN,
  _FIELD_FADEIN,
  _FIELD_END
};

static void _battleParaFree(EVENT_WIFICLUB_WORK *dbw);

//============================================================================================
//
//    サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_WiFiClubMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_WIFICLUB_WORK * dbw = work;
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
    GAMESYSTEM_CallProc(gsys, NO_OVERLAY_ID, &WifiClubProcData, dbw);
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
    _battleParaFree(dbw);
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;
  case _FIELD_FADEIN:
    if(dbw->bFieldEnd){
      GMEVENT_CallEvent(event,EVENT_FieldFadeIn_Black(gsys, dbw->fieldmap, FIELD_FADE_WAIT));
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

/*
 *  @brief  WiFiクラブ呼び出しパラメータセット
 */
static void wifi_SetEventParam( GMEVENT* event, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap,BOOL bFieldEnd )
{
  BATTLE_SETUP_PARAM * para;
  EVENT_WIFICLUB_WORK * dbw;

  if(GAME_COMM_NO_NULL!= GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gsys))){
    GameCommSys_ExitReq(GAMESYSTEM_GetGameCommSysPtr(gsys));
  }

  dbw = GMEVENT_GetEventWork(event);
  dbw->ctrl = SaveControl_GetPointer();
  NET_PRINT("%x\n",(int)dbw->ctrl);
  dbw->gsys = gsys;
  dbw->fieldmap = fieldmap;
  dbw->event = event;
  dbw->bFieldEnd = bFieldEnd;
#if 0
  para = &dbw->para;
  {
    para->rule = BTL_RULE_SINGLE;
    para->competitor = BTL_COMPETITOR_COMM;

    para->commMode = BTL_COMM_DS;
    para->multiMode = 0;

    para->party[ BTL_CLIENT_PLAYER ] = PokeParty_AllocPartyWork( HEAPID_CORE );  ///< プレイヤーのパーティ
    para->party[ BTL_CLIENT_ENEMY1 ] = NULL;   ///< 1vs1時の敵AI, 2vs2時の１番目敵AI用
    para->party[ BTL_CLIENT_PARTNER ] = NULL;  ///< 2vs2時の味方AI（不要ならnull）
    para->party[ BTL_CLIENT_ENEMY2 ] = NULL;   ///< 2vs2時の２番目敵AI用（不要ならnull）
    para->playerStatus[ BTL_CLIENT_PLAYER ] =  GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gsys));

    para->musicDefault = SEQ_BGM_VS_NORAPOKE;   ///< デフォルト時のBGMナンバー
    para->musicPinch = SEQ_BGM_BATTLEPINCH;     ///< ピンチ時のBGMナンバー

    PokeParty_Copy(GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(gsys)), para->party[ BTL_CLIENT_PLAYER ]);
  }
#endif
}

//------------------------------------------------------------------
/*
 *  @brief  WiFiクラブイベントコール
 */
//------------------------------------------------------------------
GMEVENT* EVENT_WiFiClub( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap )
{
  GMEVENT * event = GMEVENT_Create(gsys, NULL, EVENT_WiFiClubMain, sizeof(EVENT_WIFICLUB_WORK));
  wifi_SetEventParam( event, gsys, fieldmap, FALSE );

  return event;
}
//------------------------------------------------------------------
/*
 *  @brief  WiFiクラブイベントチェンジ
 */
//------------------------------------------------------------------
void EVENT_WiFiClubChange(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,GMEVENT * event)
{
  GMEVENT_Change( event, EVENT_WiFiClubMain, sizeof(EVENT_WIFICLUB_WORK) );
  wifi_SetEventParam( event, gsys, fieldmap,TRUE );
}



static void _battleParaFree(EVENT_WIFICLUB_WORK *dbw)
{
#if 0
  BATTLE_SETUP_PARAM * para;

  para = &dbw->para;
  GFL_HEAP_FreeMemory(para->party[ BTL_CLIENT_PLAYER ]);
#endif
}


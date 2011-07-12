//===================================================================================
/**
 *  @file   event_move_hilink.c
 *  @brief  ハイリンク移動イベントコア(Cギアワイヤレス、デルパワー使用画面からイベント接続)
 *  @author Miyuki Iwasawa
 *  @date   11.07.12
 *
 *  ワイヤレス、デルパワー使用画面から「ハイリンク」を選んだ後
 *  侵入ステートによって起こる分岐を処理して、各ステートに応じたイベントを内部コールする
 *
 *  侵入ステートがINTRUDE_CONNECT_MISSION_TARGETの場合、このイベント起動前の
 *  各画面からの復帰時ブラックアウト中に、下画面をFIELD_SUBSCREEN_INTRUDEに変更しておく
 *  必要があるので注意！
 */
//===================================================================================
#include <gflib.h>
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"

#include "./event_cg_wireless.h"
#include "./event_intrude_subscreen.h"

#include "sound/pm_sndsys.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "net_app/cg_wireless_menu.h"
#include "field_comm/intrude_work.h"
#include "field_comm/intrude_main.h"
#include "arc/fieldmap/zone_id.h"
#include "event_intrude.h"
#include "scrcmd_intrude.h"


#include "event_move_hilink.h"

typedef struct{
  GAMESYS_WORK* gsys;
  INTRUDE_CONNECT  state;  ///<侵入ステート
}EVWORK_MOVE_HILINK;


static GMEVENT_RESULT event_MoveHilinkMain(GMEVENT * event, int *  seq, void * work)
{
  EVWORK_MOVE_HILINK * wk = (EVWORK_MOVE_HILINK*)work;
  GAMESYS_WORK * gsys = wk->gsys;
  GAME_COMM_SYS_PTR pComm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK * pFieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

  switch (*seq) {
  case 0:
    {
      GMEVENT* intrude_event;
     
      if( wk->state == INTRUDE_CONNECT_NULL){ //最初の一人としてパレスへワープ
        //パレスへワープする前に誰とも繋がっていないことが保障されるように通信の終了リクエスト＆終了待ちをする
        if(GameCommSys_CheckSystemWaiting(pComm) == FALSE){
          if(GameCommSys_BootCheck(pComm) != GAME_COMM_NO_NULL){
            GameCommSys_ExitReq(pComm);
          }
          else{ //何も通信が起動していないのを待ってイベントコールし、シーケンスを進める
            intrude_event = EVENT_IntrudeTownWarp(gsys, pFieldmap, ZONE_ID_PALACE01, FALSE);
            GMEVENT_CallEvent(event,intrude_event);
            (*seq)++;
          }
        }
      }
      else if( wk->state == INTRUDE_CONNECT_MISSION_PARTNER){ //協力者としてパレスへ
        intrude_event = EVENT_Intrude_MissionStartWait_Warp(gsys);
        GMEVENT_CallEvent(event,intrude_event);
        (*seq)++;
      }
      else{ //パレスへいけない
        if( wk->state != INTRUDE_CONNECT_MISSION_TARGET){
          GMEVENT_CallEvent(event, EVENT_IntrudeNotWarp(gsys));
        }
        (*seq)++;
      }
    }
    break;
  case 1: 
    return GMEVENT_RES_FINISH;
  default:
    GF_ASSERT(0);
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


//=============================================================
/*
 *  @brief  ハイリンクへのワープイベント起動
 *
 *  @param  gsys
 *  @param  state INTRUDE_CONNECT アプリから受け取ったIntrude_GetIntrudeStatus(pComm)の戻り値
 */ 
//=============================================================
GMEVENT*  EVENT_MoveHilink( GAMESYS_WORK* gsys, INTRUDE_CONNECT state )
{
  EVWORK_MOVE_HILINK* wk;

  GMEVENT* event = GMEVENT_Create( gsys, NULL, event_MoveHilinkMain, sizeof(EVWORK_MOVE_HILINK));

  wk = GMEVENT_GetEventWork( event );
  wk->gsys = gsys;
  wk->state = state;

  return event;
}



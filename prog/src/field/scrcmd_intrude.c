//==============================================================================
/**
 * @file    scrcmd_intrude.c
 * @brief   スクリプトコマンド：侵入
 * @author  matsuda
 * @date    2009.10.26(月)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_intrude.h"

#include "field_comm\intrude_types.h"
#include "field_comm\intrude_main.h"
#include "field_comm\intrude_comm_command.h"
#include "field/field_comm/intrude_minimono.h"
#include "field/field_comm/intrude_monolith.h"
#include "field/field_comm/intrude_mission.h"
#include "field/field_comm/intrude_message.h"
#include "field/field_comm/intrude_field.h"

#include "message.naix"
#include "msg/msg_invasion.h"


//==============================================================================
//  構造体定義
//==============================================================================
///イベント制御ワーク：ミッション開始
typedef struct{
  INTRUDE_EVENT_MSGWORK iem;  ///<プリント制御
  BOOL error;                 ///<TRUE:エラー発生
  u16 zone_id;                ///<ゾーンID
  u8 monolith_type;           ///<モノリスタイプ
  u8 padding;
}EVENT_MISSION_START;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT * EVENT_Intrude_MissionStart( GAMESYS_WORK * gsys, u16 monolith_type, ZONEID zone_id );
static GMEVENT_RESULT _event_IntrudeMissionStart( GMEVENT * event, int * seq, void * work );




//==================================================================
/**
 * ミニモノリスセッティング
 *
 * @param   core		仮想マシン制御構造体へのポインタ
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdIntrudeMinimonoSetting( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  
  return VMCMD_RESULT_CONTINUE;
  MINIMONO_AddPosRand(game_comm, fieldWork);
  
  return VMCMD_RESULT_CONTINUE;
}

//==================================================================
/**
 * ミッション開始
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 *
 * 以下の処理を行います。
 * １．親にミッション番号リクエスト
 * ２．ミッション番号受信待ち
 * ３．ミッションメッセージ表示
 */
//==================================================================
VMCMD_RESULT EvCmdIntrudeMissionStart( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  MMDL *mmdl = SCRIPT_GetTargetObj( scw );  //話しかけ時のみ有効
  u16 obj_code = MMDL_GetOBJCode( mmdl );
  ZONEID zone_id = SCRCMD_WORK_GetZoneID( work );
  u16 monolith_type;
  
  monolith_type = (obj_code == BLACKMONOLITH) ? MONOLITH_TYPE_BLACK : MONOLITH_TYPE_WHITE;
  SCRIPT_CallEvent( scw, EVENT_Intrude_MissionStart(gsys, monolith_type, zone_id) );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * イベント：ミッション開始
 *
 * @param   gsys		
 * @param   ret_wk		
 * @param   party_index		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static GMEVENT * EVENT_Intrude_MissionStart(GAMESYS_WORK * gsys, u16 monolith_type, ZONEID zone_id)
{
  GMEVENT *event;
  EVENT_MISSION_START *ems;
  
  event = GMEVENT_Create( gsys, NULL, _event_IntrudeMissionStart, sizeof(EVENT_MISSION_START) );
  ems = GMEVENT_GetEventWork( event );
  ems->monolith_type = monolith_type;
  ems->zone_id = zone_id;
  return event;
}

//--------------------------------------------------------------
/**
 * イベント処理関数：ミッション開始   ※check　削除候補
 *
 * @param   event		
 * @param   seq		
 * @param   work		
 *
 * @retval  GMEVENT_RESULT		
 * 
 * 以下の処理を行います。
 * １．親にミッション番号リクエスト
 * ２．ミッション番号受信待ち
 * ３．ミッションメッセージ表示
 */
//--------------------------------------------------------------
static GMEVENT_RESULT _event_IntrudeMissionStart( GMEVENT * event, int * seq, void * work )
{
  EVENT_MISSION_START *ems = work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  GAME_COMM_SYS_PTR game_comm= GAMESYSTEM_GetGameCommSysPtr(gsys);
  INTRUDE_COMM_SYS_PTR intcomm;
  enum{
    SEQ_INIT,
    SEQ_MISSION_REQ,
    SEQ_RECV_WAIT,
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
  
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if((*seq) > SEQ_INIT && (*seq) < SEQ_MSG_WAIT){
      IntrudeEventPrint_StartStream(&ems->iem, msg_invasion_mission_sys000);
      *seq = SEQ_MSG_WAIT;
      ems->error = TRUE;
    }
  }
  
  switch( *seq ){
  case SEQ_INIT:
    IntrudeEventPrint_SetupFieldMsg(&ems->iem, gsys);
    (*seq)++;
    break;
  case SEQ_MISSION_REQ:
    if(MISSION_RecvCheck(&intcomm->mission) == TRUE || 
        IntrudeSend_MissionReq(intcomm, ems->monolith_type, ems->zone_id) == TRUE){
      *seq = SEQ_RECV_WAIT;
    }
    break;
  case SEQ_RECV_WAIT:
    if(MISSION_RecvCheck(&intcomm->mission) == TRUE){
      *seq = SEQ_MSG_INIT;
    }
    break;
  case SEQ_MSG_INIT:
    {
      GAMEDATA *gdata = GAMESYSTEM_GetGameData(gsys);
      MYSTATUS *target_myst = GAMEDATA_GetMyStatusPlayer(gdata,intcomm->mission.data.target_netid);
      u32 msg_id;

      msg_id = MISSION_GetMissionMsgID(&intcomm->mission);
      if(ems->error == FALSE && intcomm->mission.data.accept_netid != INTRUDE_NETID_NULL){
        WORDSET_RegisterPlayerName( ems->iem.wordset, 0, target_myst );
      }

      IntrudeEventPrint_StartStream(&ems->iem, msg_id);
    }
    *seq = SEQ_MSG_WAIT;
    break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&ems->iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
    if(intcomm != NULL){
      IntrudeField_PlayerDisguise(intcomm, gsys, 20);
    }
    IntrudeEventPrint_ExitFieldMsg(&ems->iem);
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//==================================================================
/**
 * パレス島の初期マップ連結セッティング
 *
 * @param   core		仮想マシン制御構造体へのポインタ
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdIntrudeConnectMapSetting( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  INTRUDE_COMM_SYS_PTR intcomm = NULL;
  
  if(GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_INVASION){
    intcomm = GameCommSys_GetAppWork(game_comm);
  }

  if(intcomm != NULL){
    IntrudeField_ConnectMapInit(fieldWork, gsys, intcomm);
    IntrudeField_ConnectMap(fieldWork, gsys, intcomm);
  }
  
  MONOLITH_AddConnectAllMap(fieldWork);

  return VMCMD_RESULT_CONTINUE;
}

//==================================================================
/**
 * シンボルポケモンセッティング
 *
 * @param   core		仮想マシン制御構造体へのポインタ
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolPokeSet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  
  DEBUG_INTRUDE_BingoPokeSet(fieldWork);
  
  return VMCMD_RESULT_CONTINUE;
}

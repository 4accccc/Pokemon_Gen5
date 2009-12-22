//==============================================================================
/**
 * @file    scrcmd_intrude_field.c
 * @brief   スクリプトコマンド：侵入  ※fieldmapオーバーレイに配置
 * @author  matsuda
 * @date    2009.12.03(木)
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
#include "field\field_comm\intrude_work.h"

#include "message.naix"
#include "msg/msg_invasion.h"

#include "event_intrude.h"


//==============================================================================
//  構造体定義
//==============================================================================
///パレスIN時の強制変装イベント 管理ワーク
typedef struct{
  INTRUDE_EVENT_DISGUISE_WORK ev_diswork;
}PALACE_IN_DISGUISE_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT * EVENT_Intrude_MissionChoiceListReq(GAMESYS_WORK * gsys);
static GMEVENT_RESULT _event_MissionChoiceListReq( GMEVENT * event, int * seq, void * work );
static GMEVENT * EVENT_Intrude_PalaceInDisguise(GAMESYS_WORK * gsys);
static GMEVENT_RESULT _event_PalaceInDisguise( GMEVENT * event, int * seq, void * work );




//==================================================================
/**
 * ミッション選択候補リストを取得
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 *
 * 以下の処理を行います。
 * １．親にミッション選択候補リストをリクエスト
 * ２．ミッション選択候補リスト受信待ち
 */
//==================================================================
VMCMD_RESULT EvCmdIntrudeMissionChoiceListReq( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  
  SCRIPT_CallEvent( scw, EVENT_Intrude_MissionChoiceListReq(gsys) );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * イベント：ミッション選択候補リストを取得
 *
 * @param   gsys		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static GMEVENT * EVENT_Intrude_MissionChoiceListReq(GAMESYS_WORK * gsys)
{
  GMEVENT *event;
  
  event = GMEVENT_Create( gsys, NULL, _event_MissionChoiceListReq, 0 );
  return event;
}

//--------------------------------------------------------------
/**
 * イベント処理関数：ミッション選択候補リストを取得
 *
 * @param   event		
 * @param   seq		
 * @param   work		
 *
 * @retval  GMEVENT_RESULT		
 * 
 * 以下の処理を行います。
 * １．親にミッション選択候補リストをリクエスト
 * ２．ミッション選択候補リスト受信待ち
 */
//--------------------------------------------------------------
static GMEVENT_RESULT _event_MissionChoiceListReq( GMEVENT * event, int * seq, void * work )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  GAME_COMM_SYS_PTR game_comm= GAMESYSTEM_GetGameCommSysPtr(gsys);
  INTRUDE_COMM_SYS_PTR intcomm;
  enum{
    SEQ_INIT,
    SEQ_LIST_REQ,
    SEQ_LIST_RECEIVE_WAIT,
  };
  
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    OS_TPrintf("intcomm=NULL!! ミッション選択候補リストは取得できない\n");
    return GMEVENT_RES_FINISH;
  }
  
  switch( *seq ){
  case SEQ_INIT:
    MISSION_Init_List(&intcomm->mission); //手元のリストは初期化
    if(GFL_NET_GetConnectNum() <= 1){
      OS_TPrintf("ミッションリスト：一人のため、受信はしない\n");
      return GMEVENT_RES_FINISH;  //自分一人の時はこのままFINISH
    }
    (*seq)++;
    //break;
  case SEQ_LIST_REQ:
    if(IntrudeSend_MissionListReq(intcomm, Intrude_GetPalaceArea(intcomm)) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_LIST_RECEIVE_WAIT:
    if(MISSION_MissionList_CheckOcc(
        &intcomm->mission.list[Intrude_GetPalaceArea(intcomm)]) == TRUE){
      return GMEVENT_RES_FINISH;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * 自機の変装しているOBJCODEを取得
 *
 * @param   core		仮想マシン制御構造体へのポインタ
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 *
 * 変装していない時は「０」が返ります
 */
//==================================================================
VMCMD_RESULT EvCmdGetDisguiseCode( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  INTRUDE_COMM_SYS_PTR intcomm;
  
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL || intcomm->intrude_status_mine.disguise_no == DISGUISE_NO_NULL){
    *ret_wk = 0;
  }
  else{
    *ret_wk = intcomm->intrude_status_mine.disguise_no;
  }
  OS_TPrintf("get disguise = %d\n", *ret_wk);
  return VMCMD_RESULT_CONTINUE;
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * パレスIN時の強制変装イベント
 *
 * @param   core		仮想マシン制御構造体へのポインタ
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdPalaceInDisguise( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  
  SCRIPT_CallEvent( scw, EVENT_Intrude_PalaceInDisguise(gsys) );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * イベント：パレスIN時の強制変装イベント
 *
 * @param   gsys		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static GMEVENT * EVENT_Intrude_PalaceInDisguise(GAMESYS_WORK * gsys)
{
  GMEVENT *event;
  PALACE_IN_DISGUISE_WORK *pidw;
  
  event = GMEVENT_Create( gsys, NULL, _event_PalaceInDisguise, sizeof(PALACE_IN_DISGUISE_WORK) );
  pidw = GMEVENT_GetEventWork(event);
  GFL_STD_MemClear(pidw, sizeof(PALACE_IN_DISGUISE_WORK));
  
  return event;
}

//--------------------------------------------------------------
/**
 * イベント処理関数：パレスIN時の強制変装イベント
 *
 * @param   event		
 * @param   seq		
 * @param   work		
 *
 * @retval  GMEVENT_RESULT		
 */
//--------------------------------------------------------------
static GMEVENT_RESULT _event_PalaceInDisguise( GMEVENT * event, int * seq, void * work )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  GAME_COMM_SYS_PTR game_comm= GAMESYSTEM_GetGameCommSysPtr(gsys);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  PALACE_IN_DISGUISE_WORK *pidw = work;
  INTRUDE_COMM_SYS_PTR intcomm;
  enum{
    SEQ_INIT,
    SEQ_MAIN,
  };
  
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
  //シーケンスの流れ適にNULLチェックの必要なし
  //  return GMEVENT_RES_FINISH;
  }
  
  switch( *seq ){
  case SEQ_INIT:
    {
      const MYSTATUS *myst = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(gsys) );
      u16 disguise_code;
      
      disguise_code = Intrude_GetNormalDisguiseObjCode(myst);
      IntrudeEvent_Sub_DisguiseEffectSetup(&pidw->ev_diswork, gsys, fieldWork, disguise_code);
    }
    (*seq)++;
    break;
  case SEQ_MAIN:
    if(IntrudeEvent_Sub_DisguiseEffectMain(&pidw->ev_diswork, intcomm) == TRUE){
      return GMEVENT_RES_FINISH;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

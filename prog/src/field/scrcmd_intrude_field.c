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
#include "field/field_comm/intrude_monolith.h"
#include "field/field_comm/intrude_mission.h"
#include "field/field_comm/intrude_message.h"
#include "field/field_comm/intrude_field.h"
#include "field\field_comm\intrude_work.h"
#include "field/field_comm/intrude_mission_field.h"

#include "message.naix"
#include "msg/msg_invasion.h"
#include "msg/msg_mission_msg.h"

#include "event_intrude.h"
#include "event_intrude_subscreen.h"
#include "fieldmap/zone_id.h"
#include "field/zonedata.h"


//==============================================================================
//  構造体定義
//==============================================================================
///パレスIN時の強制変装イベント 管理ワーク
typedef struct{
  INTRUDE_EVENT_DISGUISE_WORK ev_diswork;
}PALACE_IN_DISGUISE_WORK;

///MissionStartWait制御構造体
typedef struct{
  INTRUDE_EVENT_MSGWORK iem;
}EVENT_MISSION_START_WAIT;

///ミッションリスト受信制御構造体
typedef struct{
  GFL_MSGDATA *msgData;
  FLDMSGWIN *msgWin;
  WORDSET *wordset;
  u16 *ret_wk;      ///<結果代入先　TRUE:正常終了　FALSE:エラー。モノリス画面を開いてはいけない
}EVENT_MISSION_CHOICE_LIST;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT * EVENT_Intrude_MissionChoiceListReq(GAMESYS_WORK * gsys, u16 *ret_wk);
static GMEVENT_RESULT _event_MissionChoiceListReq( GMEVENT * event, int * seq, void * work );

static GMEVENT * EVENT_Intrude_PalaceInDisguise(GAMESYS_WORK * gsys);
static GMEVENT_RESULT _event_PalaceInDisguise( GMEVENT * event, int * seq, void * work );

static GMEVENT_RESULT _event_MissionStartWait( GMEVENT * event, int * seq, void * work );
static void _event_MissionStartWait_Destructor(EVENT_MISSION_START_WAIT *emsw);

static GMEVENT_RESULT _event_MissionStartWaitWarp( GMEVENT * event, int * seq, void * work );




//==================================================================
/**
 * モノリス用にミッション選択候補リストを取得
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
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );
  
  SCRIPT_CallEvent( scw, EVENT_Intrude_MissionChoiceListReq(gsys, ret_wk) );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * イベント：モノリス用にミッション選択候補リストを取得
 *
 * @param   gsys		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static GMEVENT * EVENT_Intrude_MissionChoiceListReq(GAMESYS_WORK * gsys, u16 *ret_wk)
{
  GMEVENT *event;
  EVENT_MISSION_CHOICE_LIST *emcl;
  
  event = GMEVENT_Create( gsys, NULL, _event_MissionChoiceListReq, sizeof(EVENT_MISSION_CHOICE_LIST) );
  emcl = GMEVENT_GetEventWork(event);
  emcl->ret_wk = ret_wk;
  *(emcl->ret_wk) = FALSE;
  
  return event;
}

//--------------------------------------------------------------
/**
 * イベント処理関数：モノリス用にミッション選択候補リストを取得
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
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  GAME_COMM_SYS_PTR game_comm= GAMESYSTEM_GetGameCommSysPtr(gsys);
  EVENT_MISSION_CHOICE_LIST *emcl = work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  INTRUDE_COMM_SYS_PTR intcomm;
  BOOL my_palace = FALSE;
  enum{
    SEQ_INIT,
    SEQ_LIST_REQ,
    SEQ_LIST_RECEIVE_WAIT,
    SEQ_MONOLITH_STATUS_REQ,
    SEQ_MONOLITH_STATUS_RECEIVE_WAIT,
    SEQ_MSG_WAIT,
    SEQ_MSG_LAST_KEY_WAIT,
    SEQ_FINISH,
  };
  
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL && (*seq) > SEQ_INIT && (*seq) < SEQ_MSG_WAIT){
    OS_TPrintf("intcomm=NULL!! ミッション選択候補リストは取得できない\n");
    *seq = SEQ_MSG_WAIT;
  }
  
  switch( *seq ){
  case SEQ_INIT:
	  if(NetErr_App_CheckError()){
      *seq = SEQ_FINISH;  //エラーが発生している時はメッセージを出さずに終了
      break;
    }

    if(intcomm != NULL){
      MISSION_Init_List(&intcomm->mission); //手元のリストは初期化
      IntrudeField_MonolithStatus_Init(&intcomm->monolith_status);
    }
    
    {
      FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(fieldWork);
      emcl->msgData = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_invasion_dat );
      emcl->msgWin = FLDMSGWIN_AddTalkWin( msgBG, emcl->msgData );
      emcl->wordset = WORDSET_Create(HEAPID_PROC);
    }

    PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
	  if(intcomm == NULL || Intrude_GetPalaceArea(gamedata) == GAMEDATA_GetIntrudeMyID(gamedata)){
      FLDMSGWIN_Print( emcl->msgWin, 0, 0, plc_mono_01 );
      my_palace = TRUE;
    }
    else{
      STRBUF *expand_buf = GFL_STR_CreateBuffer(256, HEAPID_FIELDMAP);
      STRBUF *src_buf = GFL_MSG_CreateString( emcl->msgData, plc_mono_02 );
      
      WORDSET_RegisterPlayerName( 
        emcl->wordset, 0, Intrude_GetMyStatus( gamedata, Intrude_GetPalaceArea(gamedata) ) );
      WORDSET_ExpandStr(emcl->wordset, expand_buf, src_buf);
      FLDMSGWIN_PrintStrBuf( emcl->msgWin, 0, 0, expand_buf );
      
      GFL_STR_DeleteBuffer(src_buf);
      GFL_STR_DeleteBuffer(expand_buf);
    }

    if(GFL_NET_GetConnectNum() <= 1){
      OS_TPrintf("ミッションリスト：一人のため、受信はしない\n");
      if(my_palace == FALSE){
        *(emcl->ret_wk) = FALSE;  //他人のモノリスにいるのに自分一人
      }
      else{
        *(emcl->ret_wk) = TRUE;
      }
      *seq = SEQ_MSG_WAIT;
      break;
    }
    (*seq)++;
    //break;
  case SEQ_LIST_REQ:
    if(IntrudeSend_MissionListReq(intcomm, Intrude_GetPalaceArea(gamedata)) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_LIST_RECEIVE_WAIT:
    if(MISSION_MissionList_CheckOcc(
        &intcomm->mission.list[Intrude_GetPalaceArea(gamedata)]) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_MONOLITH_STATUS_REQ:
    if(IntrudeSend_MonolithStatusReq(intcomm, Intrude_GetPalaceArea(gamedata)) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_MONOLITH_STATUS_RECEIVE_WAIT:
    if(IntrudeField_MonolithStatus_CheckOcc(&intcomm->monolith_status) == TRUE){
      *(emcl->ret_wk) = TRUE;
      *seq = SEQ_MSG_WAIT;
    }
    break;
  
  case SEQ_MSG_WAIT:
    if( FLDMSGWIN_CheckPrintTrans(emcl->msgWin) == TRUE ){
      (*seq)++;
    }
    break;
  case SEQ_MSG_LAST_KEY_WAIT:
    if( GFL_UI_KEY_GetTrg() & EVENT_WAIT_LAST_KEY ){
      WORDSET_Delete(emcl->wordset);
      FLDMSGWIN_Delete( emcl->msgWin );
      GFL_MSG_Delete( emcl->msgData );
      (*seq)++;
    }
    break;
  case SEQ_FINISH:
    return GMEVENT_RES_FINISH;
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
 * ミッションエントリーしているかを取得
 *
 * @param   core		仮想マシン制御構造体へのポインタ
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 *
 * TRUE:エントリーしている　FALSE:エントリーしていない
 */
//==================================================================
VMCMD_RESULT EvCmdGetMissionEntry( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  INTRUDE_COMM_SYS_PTR intcomm;
  
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm != NULL && intcomm->mission_start_event_ended == FALSE && MISSION_GetMissionEntry(&intcomm->mission) == TRUE){
    *ret_wk = TRUE;
  }
  else{
    *ret_wk = FALSE;
  }
  return VMCMD_RESULT_CONTINUE;
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * ミッションエントリー後、開始まで待機
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 * 以下の処理を行います。
 * １．ミッションが開始されるまで待つ
 * ２．変身イベントへ移行
 */
//==================================================================
VMCMD_RESULT EvCmdIntrudeMissionStartWait( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  
  SCRIPT_CallEvent( scw, EVENT_Intrude_MissionStartWait(gsys) );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * イベント：ミッションエントリー後、開始まで待機
 *
 * @param   gsys		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
GMEVENT * EVENT_Intrude_MissionStartWait(GAMESYS_WORK * gsys)
{
  GMEVENT *event;
  EVENT_MISSION_START_WAIT *emsw;
  
  event = GMEVENT_Create( gsys, NULL, _event_MissionStartWait, sizeof(EVENT_MISSION_START_WAIT) );
  emsw = GMEVENT_GetEventWork(event);
  GFL_STD_MemClear(emsw, sizeof(EVENT_MISSION_START_WAIT));
  
  return event;
}

//--------------------------------------------------------------
/**
 * イベント処理関数：ミッションエントリー後、開始まで待機
 *
 * @param   event		
 * @param   seq		
 * @param   work		
 *
 * @retval  GMEVENT_RESULT		
 * 
 * 以下の処理を行います。
 * １．ミッションが開始されるまで待つ
 * ２．変身イベントへ移行
 */
//--------------------------------------------------------------
static GMEVENT_RESULT _event_MissionStartWait( GMEVENT * event, int * seq, void * work )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  EVENT_MISSION_START_WAIT *emsw = work;
  INTRUDE_COMM_SYS_PTR intcomm;
  enum{
    SEQ_INIT,
    SEQ_MISSION_START_WAIT,
    SEQ_DISGUISE_EVENT_CHANGE,
    SEQ_FINISH,
  };
  
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL && (*seq) < SEQ_FINISH){
    *seq = SEQ_FINISH;
  }
  
  switch( *seq ){
  case SEQ_INIT:
    intcomm->mission_start_event_ended = TRUE;
    IntrudeEventPrint_SetupFieldMsg(&emsw->iem, gsys);
    if(MISSION_FIELD_CheckStatus(&intcomm->mission) == MISSION_STATUS_EXE){
      *seq = SEQ_DISGUISE_EVENT_CHANGE; //既にミッションが始まっている場合はすぐに変身イベントへ
    }
    else{
      IntrudeEventPrint_Print(&emsw->iem, msg_invasion_mission_sys005, 0, 0);
      (*seq)++;
    }
    break;
  case SEQ_MISSION_START_WAIT:
    {
      MISSION_STATUS mst = MISSION_FIELD_CheckStatus(&intcomm->mission);
      switch(mst){
      case MISSION_STATUS_READY:
        break;
      case MISSION_STATUS_EXE:
      default:  //ミッション参加がギリギリだった場合などで既にミッションが終了している場合がある
        (*seq)++;
        break;
      }
    }
    break;
  case SEQ_DISGUISE_EVENT_CHANGE:
    {
      FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
      GMEVENT *next_event = EVENT_Disguise(gsys, fieldWork, intcomm, HEAPID_FIELDMAP);
      
      _event_MissionStartWait_Destructor(emsw);
      GMEVENT_ChangeEvent(event, next_event);
      return GMEVENT_RES_CONTINUE;  //ChangeEventではFINISHさせてはいけない
    }
    break;
    
  case SEQ_FINISH:
    _event_MissionStartWait_Destructor(emsw);
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 解放処理
 *
 * @param   emsw		
 *
 * 途中でChangeEvent、FINISHで分岐するので終了処理をまとめておく
 */
//--------------------------------------------------------------
static void _event_MissionStartWait_Destructor(EVENT_MISSION_START_WAIT *emsw)
{
  IntrudeEventPrint_ExitFieldMsg(&emsw->iem);
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * ミッション途中参加：ミッション起動しているパレスへワープし、開始まで待機
 *
 * @param   gsys		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
GMEVENT * EVENT_Intrude_MissionStartWait_Warp(GAMESYS_WORK * gsys)
{
  GMEVENT *event;
  
  event = GMEVENT_Create( gsys, NULL, _event_MissionStartWaitWarp, 0 );
  
  return event;
}

//--------------------------------------------------------------
/**
 * イベント処理関数：ミッションエントリー後、開始まで待機
 *
 * @param   event		
 * @param   seq		
 * @param   work		
 *
 * @retval  GMEVENT_RESULT		
 * 
 * 以下の処理を行います。
 * １．ミッションが開始されるまで待つ
 * ２．変身イベントへ移行
 */
//--------------------------------------------------------------
static GMEVENT_RESULT _event_MissionStartWaitWarp( GMEVENT * event, int * seq, void * work )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  INTRUDE_COMM_SYS_PTR intcomm;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  enum{
    SEQ_WARP,
    SEQ_MISSION_START_WAIT,
    SEQ_FINISH,
  };
  
  intcomm = Intrude_Check_CommConnect(game_comm);
  
  switch( *seq ){
  case SEQ_WARP:
    {
      GMEVENT *child_event;
      NetID warp_netid;
      if(intcomm == NULL){
        warp_netid = GAMEDATA_GetIntrudeMyID(gamedata);
      }
      else{
        warp_netid = MISSION_GetMissionTargetNetID(intcomm, &intcomm->mission);
      }
      //GMEVENT_CallEvent(event, EVENT_IntrudeWarpPalace_NetID(gsys, warp_netid));
      child_event = EVENT_IntrudeTownWarp(gsys, fieldWork, ZONE_ID_PALACE01, TRUE);
      if(child_event == NULL){
        return GMEVENT_RES_FINISH;
      }
      GMEVENT_CallEvent(event, child_event);
    }
    (*seq)++;
    break;
  case SEQ_MISSION_START_WAIT:
    //ゾーンIDがパレスでない場合はワープ失敗
    if(ZONEDATA_IsPalace(PLAYERWORK_getZoneID( GAMESYSTEM_GetMyPlayerWork(gsys) )) == TRUE){
      GMEVENT_CallEvent(event, EVENT_Intrude_MissionStartWait(gsys));
    }
    (*seq)++;
    break;
  case SEQ_FINISH:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * 今自分がいるパレスエリアが、自分のパレスエリアを基点(0)とした場合、
 * 何個目のパレスエリアにいるかを取得する
 * 
 * 通信IDとイコールではなく、オフセットである事に注意
 *   ※0の場合は自分のパレスエリアにいる
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdIntrudeGetPalaceAreaOffset( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  
  if(Intrude_GetPalaceArea(gamedata) == GAMEDATA_GetIntrudeMyID(gamedata)){
    *ret_wk = 0;
  }
  else{
    int area_offset = Intrude_GetPalaceArea(gamedata) - GAMEDATA_GetIntrudeMyID(gamedata);
    if(area_offset < 0){
      area_offset += FIELD_COMM_MEMBER_MAX;
    }
    *ret_wk = area_offset;
  }
  return VMCMD_RESULT_CONTINUE;
}

//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * Gパワーを何か装備しているか取得
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 *
 * TRUE:何か装備している
 * FALSE:何も装備していない
 */
//==================================================================
VMCMD_RESULT EvCmdIntrudeCheckEqpGPower( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  INTRUDE_SAVE_WORK * intsave = SaveData_GetIntrude( GAMEDATA_GetSaveControlWork( gamedata ) );
  GPOWER_ID gpower_id;
  
  gpower_id = ISC_SAVE_GetGPowerID(intsave);
  if(gpower_id != GPOWER_ID_NULL){
    *ret_wk = TRUE;
  }
  else{
    *ret_wk = FALSE;
  }
  return VMCMD_RESULT_CONTINUE;
}

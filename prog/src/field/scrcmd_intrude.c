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
#include "field/field_comm/intrude_mission.h"
#include "message.naix"


//==============================================================================
//  定数定義
//==============================================================================
///侵入イベントで使用するヒープID
#define INTRUDE_EVENT_HEAPID        (HEAPID_PROC)
///侵入イベントで使用するメッセージバッファサイズ
#define INTRUDE_EVENT_MSGBUF_SIZE   (256)


//==============================================================================
//  構造体定義
//==============================================================================
///プリント制御構造体
typedef struct{
  WORDSET *wordset;
  STRBUF *msgbuf;
  STRBUF *tmpbuf;
  GFL_MSGDATA *msgdata;
  FLDMSGWIN_STREAM *msgwin_stream;
}INTRUDE_EVENT_MSGWORK;

///イベント制御ワーク：ミッション開始
typedef struct{
  INTRUDE_EVENT_MSGWORK iem;  ///<プリント制御
  BOOL error;                 ///<TRUE:エラー発生
  u8 monolith_type;           ///<モノリスタイプ
  u8 padding[3];
}EVENT_MISSION_START;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT * EVENT_Intrude_MissionStart( GAMESYS_WORK * gsys, u16 monolith_type );
static GMEVENT_RESULT _event_IntrudeMissionStart( GMEVENT * event, int * seq, void * work );



//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * プリント制御セットアップ
 *
 * @param   iem		
 * @param   gsys		
 */
//--------------------------------------------------------------
static void _IntrudePrint_SetupFieldMsg(INTRUDE_EVENT_MSGWORK *iem, GAMESYS_WORK *gsys)
{
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(fieldWork);
  
  iem->wordset = WORDSET_Create(INTRUDE_EVENT_HEAPID);
  iem->msgbuf = GFL_STR_CreateBuffer( INTRUDE_EVENT_MSGBUF_SIZE, INTRUDE_EVENT_HEAPID );
  iem->tmpbuf = GFL_STR_CreateBuffer( INTRUDE_EVENT_MSGBUF_SIZE, INTRUDE_EVENT_HEAPID );
  iem->msgdata = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_invasion_dat );
  iem->msgwin_stream = FLDMSGWIN_STREAM_AddTalkWin( msgBG, iem->msgdata );
}

//--------------------------------------------------------------
/**
 * プリント制御削除
 *
 * @param   iem		
 */
//--------------------------------------------------------------
static void _IntrudePrint_ExitFieldMsg(INTRUDE_EVENT_MSGWORK *iem)
{
  FLDMSGWIN_STREAM_Delete(iem->msgwin_stream);
  FLDMSGBG_DeleteMSGDATA(iem->msgdata);
  GFL_STR_DeleteBuffer(iem->tmpbuf);
  GFL_STR_DeleteBuffer(iem->msgbuf);
  WORDSET_Delete(iem->wordset);
}

//--------------------------------------------------------------
/**
 * プリントストリーム出力
 *
 * @param   iem		
 * @param   msg_id		
 */
//--------------------------------------------------------------
static void _IntrudePrint_StartStream(INTRUDE_EVENT_MSGWORK *iem, u16 msg_id)
{
  GFL_MSG_GetString( iem->msgdata, msg_id, iem->tmpbuf );
  WORDSET_ExpandStr( iem->wordset, iem->msgbuf, iem->tmpbuf );

  FLDMSGWIN_STREAM_PrintStrBufStart( iem->msgwin_stream, 0, 0, iem->msgbuf );
}

//--------------------------------------------------------------
/**
 * プリントストリーム出力待ち
 *
 * @param   iem		
 *
 * @retval  BOOL TRUE=表示終了,FALSE=表示中
 */
//--------------------------------------------------------------
static BOOL _IntrudePrint_WaitStream(INTRUDE_EVENT_MSGWORK *iem)
{
  return FLDMSGWIN_STREAM_Print(iem->msgwin_stream);
}


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
  MMDL **mmdl = SCRIPT_GetMemberWork( scw, ID_EVSCR_TARGET_OBJ );  //話しかけ時のみ有効
  u16 obj_code = MMDL_GetOBJCode( *mmdl );
  u16 monolith_type;
  
  monolith_type = (obj_code == BLACKMONOLITH) ? MONOLITH_TYPE_BLACK : MONOLITH_TYPE_WHITE;
  SCRIPT_CallEvent( scw, EVENT_Intrude_MissionStart(gsys, monolith_type) );
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
static GMEVENT * EVENT_Intrude_MissionStart( GAMESYS_WORK * gsys, u16 monolith_type )
{
  GMEVENT *event;
  EVENT_MISSION_START *ems;
  
  event = GMEVENT_Create( gsys, NULL, _event_IntrudeMissionStart, sizeof(EVENT_MISSION_START) );
  ems = GMEVENT_GetEventWork( event );
  ems->monolith_type = monolith_type;
  return event;
}

//--------------------------------------------------------------
/**
 * イベント処理関数：ミッション開始
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
    SEQ_MISSION_REQ,
    SEQ_RECV_WAIT,
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_END,
  };
  
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if((*seq) < SEQ_MSG_INIT){
      *seq = SEQ_MSG_INIT;
      ems->error = TRUE;
    }
  }
  
  switch( *seq ){
  case SEQ_MISSION_REQ:
    if(MISSION_RecvCheck(&intcomm->mission) == TRUE || 
        IntrudeSend_MissonReq(intcomm, ems->monolith_type) == TRUE){
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
      MYSTATUS *target_myst = GAMEDATA_GetMyStatusPlayer(gdata, intcomm->mission.target_netid);
      u32 msg_id;

      _IntrudePrint_SetupFieldMsg(&ems->iem, gsys);
      
      msg_id = MISSION_GetMissionMsgID(&intcomm->mission);
      if(ems->error == FALSE && intcomm->mission.mission_no != MISSION_NO_NULL){
        WORDSET_RegisterPlayerName( ems->iem.wordset, 0, target_myst );
      }

      _IntrudePrint_StartStream(&ems->iem, msg_id);
    }
    *seq = SEQ_MSG_WAIT;
    break;
  case SEQ_MSG_WAIT:
    if(_IntrudePrint_WaitStream(&ems->iem) == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
    _IntrudePrint_ExitFieldMsg(&ems->iem);
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


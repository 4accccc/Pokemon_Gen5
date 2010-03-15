//==============================================================================
/**
 * @file    event_mission_help.c
 * @brief   ミッション：戦闘中のｘｘｘを助けろ！
 * @author  matsuda
 * @date    2010.02.22(月)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "message.naix"

#include "script.h"
#include "event_fieldtalk.h"

#include "print/wordset.h"
#include "event_comm_talk.h"
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_comm_command.h"
#include "field/field_comm/bingo_system.h"
#include "field/field_comm/intrude_mission.h"
#include "field/field_comm/intrude_message.h"
#include "field\field_comm\intrude_mission_field.h"
#include "field\field_comm\intrude_work.h"
#include "msg/msg_invasion.h"
#include "msg/msg_mission_monolith.h"
#include "msg/msg_mission_msg.h"
#include "field/field_comm/intrude_battle.h"
#include "field/event_fieldmap_control.h" //EVENT_FieldSubProc
#include "item/itemsym.h"
#include "event_intrude.h"
#include "event_comm_common.h"
#include "event_mission_help.h"
#include "event_mission_help_after.h"

#include "../../../resource/fldmapdata/script/common_scr_def.h"

#include "poke_tool/status_rcv.h"



//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	EVENT_MISSION_HELP
//--------------------------------------------------------------
typedef struct
{
  COMMTALK_COMMON_EVENT_WORK ccew;
	BOOL error;
}EVENT_MISSION_HELP;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT CommMissionHelp_MtoT_Talk( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommMissionHelp_TtoM_Talk( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommMissionHelp_MtoT_Talked( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommMissionHelp_TtoM_Talked( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommMissionHelp_MtoT_Battle( GMEVENT *event, int *seq, void *wk );

static GMEVENT_RESULT Intrude_BattleHelpAfterEvent( GMEVENT *event, int *seq, void *wk );


//==============================================================================
//  データ
//==============================================================================
static const struct{
  u16 talk_m_to_t[TALK_TYPE_MAX];       ///<自分がミッション実行者でターゲットに話しかけた
  u16 talked_m_to_t[TALK_TYPE_MAX];     ///<自分がミッション実行者でターゲットから話しかけられた

  u16 talk_t_to_m[TALK_TYPE_MAX];       ///<自分がターゲットでミッション実行者に話しかけた
  u16 talked_t_to_m[TALK_TYPE_MAX];     ///<自分がターゲットでミッション実行者から話しかけられた
  
  u16 battle_m_to_t[TALK_TYPE_MAX];     ///<自分がミッション実行者で戦闘中のターゲットに話しかけた
  u16 recover_m_to_t[TALK_TYPE_MAX];    ///<自分がミッション実行者でターゲット回復後
}MissionHelpMsgID = {
  { //自分がミッション実行者でターゲットに話しかけた
    mis_m04_04_m1,
    mis_m04_04_m2,
    mis_m04_04_m3,
    mis_m04_04_m4,
    mis_m04_04_m5,
  },
  { //自分がミッション実行者でターゲットから話しかけられた
    mis_m04_03_m1,
    mis_m04_03_m2,
    mis_m04_03_m3,
    mis_m04_03_m4,
    mis_m04_03_m5,
  },

  { //自分がターゲットでミッション実行者に話しかけた
    mis_m04_03_t1,
    mis_m04_03_t2,
    mis_m04_03_t3,
    mis_m04_03_t4,
    mis_m04_03_t5,
  },
  { //自分がターゲットでミッション実行者から話しかけられた
    mis_m04_04_t1,
    mis_m04_04_t2,
    mis_m04_04_t3,
    mis_m04_04_t4,
    mis_m04_04_t5,
  },

  { //自分がミッション実行者で戦闘中のターゲットに話しかけた
    mis_m04_01_t1,
    mis_m04_01_t2,
    mis_m04_01_t3,
    mis_m04_01_t4,
    mis_m04_01_t5,
  },
  { //自分がミッション実行者でターゲット回復後
    mis_m04_02_m1,
    mis_m04_02_m2,
    mis_m04_02_m3,
    mis_m04_02_m4,
    mis_m04_02_m5,
  },
};



//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * イベント切り替え：助けろ  (自分がミッション実行者でターゲットに話しかけた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionHelp_MtoT_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  EVENT_MISSION_HELP *talk;
  
  GMEVENT_Change(event, CommMissionHelp_MtoT_Talk, sizeof(EVENT_MISSION_HELP));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
}

//--------------------------------------------------------------
/**
 * ミッション：助けろ (自分がミッション実行者でターゲットに話しかけた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionHelp_MtoT_Talk( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_HELP *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_SEND_ACHIEVE,
    SEQ_RECV_WAIT,
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_LAST_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) < SEQ_LAST_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_invasion_mission_sys002);
      *seq = SEQ_LAST_MSG_WAIT;
      talk->error = TRUE;
    }
  }

	switch( *seq ){
  case SEQ_MSG_INIT:
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 0, Intrude_GetMyStatus(intcomm, talk->ccew.talk_netid) );
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionHelpMsgID.talk_m_to_t[MISSION_FIELD_GetTalkType(intcomm, talk->ccew.talk_netid)]);
		(*seq)++;
		break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
  		(*seq)++;
  	}
		break;
  case SEQ_LAST_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
  	//共通Finish処理
  	EVENT_CommCommon_Finish(intcomm, &talk->ccew);
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * イベント切り替え：助けろ  (自分がターゲットでミッション実行者に話しかけた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionHelp_TtoM_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  EVENT_MISSION_HELP *talk;

  GMEVENT_Change(event, CommMissionHelp_TtoM_Talk, sizeof(EVENT_MISSION_HELP));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
}

//--------------------------------------------------------------
/**
 * ミッション：助けろ (自分がターゲットでミッション実行者に話しかけた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionHelp_TtoM_Talk( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_HELP *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_MSG_INIT,
    SEQ_LAST_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) < SEQ_LAST_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_invasion_mission_sys002);
      *seq = SEQ_LAST_MSG_WAIT;
      talk->error = TRUE;
    }
  }

	switch( *seq ){
  case SEQ_MSG_INIT:
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 0, Intrude_GetMyStatus(intcomm, talk->ccew.talk_netid) );
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionHelpMsgID.talk_t_to_m[MISSION_FIELD_GetTalkType(intcomm, talk->ccew.talk_netid)]);
		(*seq)++;
		break;
  case SEQ_LAST_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
  	//共通Finish処理
  	EVENT_CommCommon_Finish(intcomm, &talk->ccew);
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}


//======================================================================
//	
//======================================================================
//==================================================================
/**
 * イベント切り替え：助けろ (自分がミッション実行者でターゲットから話しかけられた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionHelp_MtoT_Talked(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  EVENT_MISSION_HELP *talk;

  GMEVENT_Change(event, CommMissionHelp_MtoT_Talked, sizeof(EVENT_MISSION_HELP));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
}

//--------------------------------------------------------------
/**
 * ミッション：助けろ (自分がミッション実行者でターゲットから話しかけられた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionHelp_MtoT_Talked( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_HELP *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_MSG_INIT,
    SEQ_LAST_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) < SEQ_LAST_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_invasion_mission_sys002);
      *seq = SEQ_LAST_MSG_WAIT;
      talk->error = TRUE;
    }
  }

	switch( *seq ){
  case SEQ_MSG_INIT:
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 0, Intrude_GetMyStatus(intcomm, talk->ccew.talk_netid) );
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionHelpMsgID.talked_m_to_t[MISSION_FIELD_GetTalkType(intcomm, talk->ccew.talk_netid)]);
		(*seq)++;
		break;
  case SEQ_LAST_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
  	//共通Finish処理
  	EVENT_CommCommon_Finish(intcomm, &talk->ccew);
  	
  	//ミッション失敗：パレスへ戻される
  	if(intcomm != NULL){
      GMEVENT_ChangeEvent(event, EVENT_IntrudeWarpMyPalace(gsys));
      return GMEVENT_RES_CONTINUE;
    }
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}



//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * イベント切り替え：助けろ (自分がターゲットでミッション実行者から話しかけられた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionHelp_TtoM_Talked(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  EVENT_MISSION_HELP *talk;

  GMEVENT_Change(event, CommMissionHelp_TtoM_Talked, sizeof(EVENT_MISSION_HELP));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
}

//--------------------------------------------------------------
/**
 * ミッション：助けろ (自分がターゲットでミッション実行者から話しかけられた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionHelp_TtoM_Talked( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_HELP *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_LAST_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) < SEQ_LAST_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_invasion_mission_sys002);
      *seq = SEQ_LAST_MSG_WAIT;
      talk->error = TRUE;
    }
  }

	switch( *seq ){
  case SEQ_MSG_INIT:
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 0, Intrude_GetMyStatus(intcomm, talk->ccew.talk_netid) );
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionHelpMsgID.talked_t_to_m[MISSION_FIELD_GetTalkType(intcomm, talk->ccew.talk_netid)]);
		(*seq)++;
		break;
  case SEQ_LAST_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
  	//共通Finish処理
  	EVENT_CommCommon_Finish(intcomm, &talk->ccew);
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * イベント切り替え：助けろ （自分がミッション実行者でターゲットが戦闘中に話しかけた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionHelp_TtoM_Battle(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  EVENT_MISSION_HELP *talk;

  GMEVENT_Change(event, CommMissionHelp_MtoT_Battle, sizeof(EVENT_MISSION_HELP));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
}

//--------------------------------------------------------------
/**
 * ミッション：助けろ（自分がミッション実行者でターゲットが戦闘中に話しかけた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionHelp_MtoT_Battle( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_HELP *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_INIT,
    SEQ_SEND_PLAYER_SUPPORT,
    SEQ_SEND_ACHIEVE,
    SEQ_RECV_WAIT,
    SEQ_MSG_WAIT,
    SEQ_LAST_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if((*seq) < SEQ_LAST_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_invasion_mission_sys002);
      *seq = SEQ_LAST_MSG_WAIT;
      talk->error = TRUE;
    }
  }

	switch( *seq ){
  case SEQ_INIT:
    MISSIONDATA_Wordset(intcomm, &talk->ccew.mdata.cdata, &talk->ccew.mdata.target_info, 
      talk->ccew.iem.wordset, talk->ccew.heapID);
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionHelpMsgID.battle_m_to_t[MISSION_FIELD_GetTalkType(intcomm, talk->ccew.talk_netid)]);
    (*seq)++;
    break;
	case SEQ_SEND_PLAYER_SUPPORT:
	  if(IntrudeSend_PlayerSupport(
	      intcomm, talk->ccew.talk_netid, SUPPORT_TYPE_RECOVER_FULL) == TRUE){
      (*seq)++;
    }
    break;
	case SEQ_SEND_ACHIEVE:
    if(IntrudeSend_MissionAchieve(intcomm, &intcomm->mission) == TRUE){//ミッション達成を親に伝える
      (*seq)++;
    }
    break;
  case SEQ_RECV_WAIT:
		if(MISSION_GetAchieveAnswer(&intcomm->mission) != MISSION_ACHIEVE_NULL){
      (*seq)++;
    }
    break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, 
        MissionHelpMsgID.recover_m_to_t[MISSION_FIELD_GetTalkType(intcomm, talk->ccew.talk_netid)]);
      (*seq)++;
    }
    break;
  case SEQ_LAST_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
  	//共通Finish処理
  	EVENT_CommCommon_Finish(intcomm, &talk->ccew);
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}


//==============================================================================
//  
//==============================================================================
///助けてもらった時の戦闘後イベント
typedef struct{
  INTRUDE_EVENT_MSGWORK iem;
  MYSTATUS *myst;
}HELP_AFTER_WORK;

//---------------------------------------------------------------------------------
/**
 * @brief ミッション：助けろ：助けてもらった時の戦闘後イベント
 *
 * @param gsys
 * 
 * @return イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_Intrude_BattleHelpAfterEvent( GAMESYS_WORK* gsys, HEAPID heap_id )
{
  GMEVENT* event;
  HELP_AFTER_WORK* work;
  GAMEDATA* gamedata;
  COMM_PLAYER_SUPPORT *cps;

  gamedata = GAMESYSTEM_GetGameData( gsys );
  cps = GAMEDATA_GetCommPlayerSupportPtr(gamedata);
  
  event = GMEVENT_Create(gsys, NULL, Intrude_BattleHelpAfterEvent, sizeof(HELP_AFTER_WORK));
  work = GMEVENT_GetEventWork( event );
  work->myst = MyStatus_AllocWork(heap_id);
  MyStatus_Copy(COMM_PLAYER_SUPPORT_GetSupportedMyStatus(cps), work->myst);

  IntrudeEventPrint_SetupFieldMsg(&work->iem, gsys);

  return event;
}

//--------------------------------------------------------------
/**
 * ミッション：助けてもらった時の戦闘後イベント
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT Intrude_BattleHelpAfterEvent( GMEVENT *event, int *seq, void *wk )
{
	HELP_AFTER_WORK *work = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	enum{
    SEQ_INIT,
    SEQ_LAST_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
	switch( *seq ){
  case SEQ_INIT:
    WORDSET_RegisterPlayerName( work->iem.wordset, 0, work->myst );
    IntrudeEventPrint_StartStream(&work->iem, msg_invasion_mission_sys007);
    (*seq)++;
    break;
  case SEQ_LAST_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&work->iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
    GFL_HEAP_FreeMemory(work->myst);
    IntrudeEventPrint_ExitFieldMsg(&work->iem);
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//==============================================================================
/**
 * @file    event_comm_talked.c
 * @brief   通信プレイヤーから話しかけられた
 * @author  matsuda
 * @date    2009.10.11(日)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "message.naix"
#include "msg/msg_invasion.h"
#include "msg/msg_mission_msg.h"

#include "script.h"
#include "event_fieldtalk.h"
#include "print/wordset.h"

#include "event_comm_talked.h"
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_comm_command.h"
#include "field/field_comm/intrude_mission.h"
#include "field/field_comm/intrude_message.h"
#include "field/field_comm/intrude_battle.h"
#include "field/event_fieldmap_control.h" //EVENT_FieldSubProc
#include "event_comm_talk.h"
#include "system/main.h"
#include "item/itemsym.h"


#include "../../../resource/fldmapdata/script/common_scr_def.h"

extern MMDLSYS * FIELDMAP_GetMMdlSys( FIELDMAP_WORK *fieldWork );

//======================================================================
//	define
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	COMMTALK_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
	HEAPID heapID;
	u32 scr_id;
	MMDL *fmmdl_player;
	MMDL *fmmdl_talk;
	FIELDMAP_WORK *fieldWork;
  INTRUDE_COMM_SYS_PTR intcomm;
  
	FLDMSGBG *msgBG;
	GFL_MSGDATA *msgData;
	FLDMSGWIN *msgWin;

	INTRUDE_EVENT_MSGWORK iem;

	INTRUDE_BATTLE_PARENT ibp;
	u32 talk_netid;
}COMMTALK_EVENT_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT CommWasTalkedTo( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommWasTalkedToMission( GMEVENT *event, int *seq, void *wk );



//======================================================================
//	イベント：フィールド話し掛けイベント
//======================================================================
//==================================================================
/**
 * 通信プレイヤーから話しかけられたイベント起動
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   intcomm		      侵入システムワークへのポインタ
 * @param   fmmdl_player		自機動作モデル
 * @param   talk_net_id		  話しかけてきた対象のNetID
 * @param   heap_id		      ヒープID
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_CommWasTalkedTo(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id)
{
	COMMTALK_EVENT_WORK *ftalk_wk;
	GMEVENT *event = NULL;
	INTRUDE_TALK_TYPE talk_type;
	
	talk_type = intcomm->recv_talk_first_attack.talk_type;
	if(talk_type == INTRUDE_TALK_TYPE_MISSION){
    switch(intcomm->recv_talk_first_attack.mdata.cdata.type){
    case MISSION_TYPE_ITEM:
    	event = GMEVENT_Create(
    		gsys, NULL,	CommWasTalkedToMission, sizeof(COMMTALK_EVENT_WORK) );
    	break;
    case MISSION_TYPE_BASIC:
      //一方的に回復フラグを渡されるだけなので、話しかけられたときは通常会話にさせる
      talk_type = INTRUDE_TALK_TYPE_NORMAL;
      break;
    }
  }

  if(event == NULL){
  	switch(talk_type){
  	default:
  	  GF_ASSERT(0);
  	  //break;
  	case INTRUDE_TALK_TYPE_NORMAL:
    	event = GMEVENT_Create(
    		gsys, NULL,	CommWasTalkedTo, sizeof(COMMTALK_EVENT_WORK) );
      break;
    case INTRUDE_TALK_TYPE_MISSION: //上で処理が済んでいる
      break;
    }
  }
  
	ftalk_wk = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( ftalk_wk, sizeof(COMMTALK_EVENT_WORK) );
	
	ftalk_wk->heapID = heap_id;
	ftalk_wk->msgBG = FIELDMAP_GetFldMsgBG( fieldWork );
	ftalk_wk->fmmdl_player = fmmdl_player;
	ftalk_wk->fmmdl_talk = CommPlayer_GetMmdl(intcomm->cps, talk_net_id);
	ftalk_wk->fieldWork = fieldWork;
	ftalk_wk->intcomm = intcomm;
	ftalk_wk->talk_netid = talk_net_id;
	
	//侵入システムのアクションステータスを更新
	Intrude_SetActionStatus(intcomm, INTRUDE_ACTION_TALK);

  IntrudeEventPrint_SetupFieldMsg(&ftalk_wk->iem, gsys);

	return( event );
}

//--------------------------------------------------------------
/**
 * フィールド話し掛けイベント
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommWasTalkedTo( GMEVENT *event, int *seq, void *wk )
{
	COMMTALK_EVENT_WORK *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
  enum{
    SEQ_MSG_INIT,
    SEQ_MSG_ANSWER,
    SEQ_MSG_WAIT,
    SEQ_SELECT_WAIT,
    SEQ_BATTLE_YESNO_INIT,
    SEQ_BATTLE_YESNO_MSGWAIT,
    SEQ_BATTLE_YESNO_SELECT,
    SEQ_BATTLE_YES,
    SEQ_BATTLE_START,
    SEQ_ITEM_YESNO_INIT,
    SEQ_ITEM_MSG_WAIT,
    SEQ_ITEM_LAST_WAIT,
    SEQ_NG,
    SEQ_TALK_CANCEL,
    SEQ_FINISH,
  };

  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
  #if 0 //※check　後で作成
    if((*seq) < SEQ_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->iem, msg_talk_cancel);
      *seq = SEQ_MSG_WAIT;
      talk->error = TRUE;
    }
  #endif
  }
	
	switch( *seq ){
	case SEQ_MSG_INIT:
    IntrudeEventPrint_StartStream(&talk->iem, msg_intrude_001);
		(*seq)++;
		break;
	case SEQ_MSG_ANSWER:
	  if(IntrudeSend_TalkAnswer(
	      talk->intcomm, talk->intcomm->talk.talk_netid, talk->intcomm->talk.talk_status) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->iem) == TRUE){
			(*seq)++;
		}
		break;
	case SEQ_SELECT_WAIT:   //相手の選択待ち
    {
      INTRUDE_TALK_STATUS talk_status = Intrude_GetTalkAnswer(intcomm);
      switch(talk_status){
      case INTRUDE_TALK_STATUS_BATTLE:
        *seq = SEQ_BATTLE_YESNO_INIT;
        break;
      case INTRUDE_TALK_STATUS_ITEM:
        *seq = SEQ_ITEM_YESNO_INIT;
        break;
      case INTRUDE_TALK_STATUS_CANCEL:
        *seq = SEQ_TALK_CANCEL;
        break;
      default:  //まだ返事が来ていない
        if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
          if(IntrudeSend_TalkCancel(talk->intcomm->talk.talk_netid) == TRUE){
            *seq = SEQ_TALK_CANCEL;
          }
        }
        break;
      }
    }
	  break;
	case SEQ_BATTLE_YESNO_INIT:
    IntrudeEventPrint_StartStream(&talk->iem, msg_talk_battle_001);
	  (*seq)++;
	  break;
	case SEQ_BATTLE_YESNO_MSGWAIT:
    if(IntrudeEventPrint_WaitStream(&talk->iem) == TRUE){
      IntrudeEventPrint_SetupYesNo(&talk->iem, gsys);
      (*seq)++;
    }
    break;
  case SEQ_BATTLE_YESNO_SELECT:
    {
      FLDMENUFUNC_YESNO yesno = IntrudeEventPrint_SelectYesNo(&talk->iem);
      switch(yesno){
      case FLDMENUFUNC_YESNO_YES:
        IntrudeEventPrint_ExitMenu(&talk->iem);
        *seq = SEQ_BATTLE_YES;
        break;
      case FLDMENUFUNC_YESNO_NO:
        IntrudeEventPrint_ExitMenu(&talk->iem);
        *seq = SEQ_NG;
        break;
      }
    }
    break;
  case SEQ_BATTLE_YES:
    if(IntrudeSend_TalkAnswer(
        talk->intcomm, talk->intcomm->talk.talk_netid, INTRUDE_TALK_STATUS_OK) == TRUE){
      *seq = SEQ_BATTLE_START;
    }
    break;
  case SEQ_BATTLE_START:
    IntrudeEventPrint_ExitFieldMsg(&talk->iem);
    {
      GMEVENT *child_event;
      
      talk->ibp.gsys = gsys;
      talk->ibp.target_netid = talk->talk_netid;
      child_event = EVENT_FieldSubProc(
        gsys, talk->fieldWork, NO_OVERLAY_ID, &IntrudeBattleProcData, &talk->ibp);
      GMEVENT_CallEvent(event, child_event);
    }
	  *seq = SEQ_FINISH;
    break;
    
  case SEQ_ITEM_YESNO_INIT:
    {
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
      MYSTATUS *target_myst = GAMEDATA_GetMyStatusPlayer(gamedata, talk->talk_netid);
      MYITEM_PTR myitem = GAMEDATA_GetMyItem( gamedata );

      MYITEM_AddItem( myitem, ITEM_KIZUGUSURI, 1, HEAPID_PROC );

      WORDSET_RegisterPlayerName( talk->iem.wordset, 0, target_myst );
      WORDSET_RegisterItemName( talk->iem.wordset, 1, ITEM_KIZUGUSURI );
      IntrudeEventPrint_StartStream(&talk->iem, msg_talk_item_001);
    }
    *seq = SEQ_ITEM_MSG_WAIT;
    break;
  case SEQ_ITEM_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->iem) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_ITEM_LAST_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_FINISH;
    }
    break;
  
  case SEQ_NG:
    if(IntrudeSend_TalkAnswer(
        talk->intcomm, talk->intcomm->talk.talk_netid, INTRUDE_TALK_STATUS_NG) == TRUE){
      *seq = SEQ_FINISH;
    }
    break;
	
	case SEQ_TALK_CANCEL:
	  *seq = SEQ_FINISH;
	  break;
	  
	case SEQ_FINISH:
    IntrudeEventPrint_ExitFieldMsg(&talk->iem);

  	//侵入システムのアクションステータスを更新
  	Intrude_SetActionStatus(talk->intcomm, INTRUDE_ACTION_FIELD);
    Intrude_InitTalkWork(talk->intcomm, INTRUDE_NETID_NULL);
		return( GMEVENT_RES_FINISH );
	}
	
	return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * フィールド話し掛けイベント：ミッション
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommWasTalkedToMission( GMEVENT *event, int *seq, void *wk )
{
	COMMTALK_EVENT_WORK *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
  enum{
    SEQ_MSG_INIT,
    SEQ_TALK_ANSWER,
    SEQ_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_FINISH,
  };

  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
  #if 0 //※check　後で作成
    if((*seq) < SEQ_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->iem, msg_talk_cancel);
      *seq = SEQ_MSG_WAIT;
      talk->error = TRUE;
    }
  #endif
  }
	
	switch( *seq ){
  case SEQ_MSG_INIT:
    MISSIONDATA_Wordset(intcomm,
       &intcomm->recv_talk_first_attack.mdata, talk->iem.wordset, talk->heapID);
    IntrudeEventPrint_StartStream(&talk->iem, MissionItemMsgID[0] + 1);
		(*seq)++;
		break;
	case SEQ_TALK_ANSWER:
	  if(IntrudeSend_TalkAnswer(
	      talk->intcomm, talk->intcomm->talk.talk_netid, talk->intcomm->talk.talk_status) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_FINISH;
    }
    break;
	case SEQ_FINISH:
    IntrudeEventPrint_ExitFieldMsg(&talk->iem);

  	//侵入システムのアクションステータスを更新
  	Intrude_SetActionStatus(talk->intcomm, INTRUDE_ACTION_FIELD);
    Intrude_InitTalkWork(talk->intcomm, INTRUDE_NETID_NULL);
		return( GMEVENT_RES_FINISH );
	}
	
	return( GMEVENT_RES_CONTINUE );
}


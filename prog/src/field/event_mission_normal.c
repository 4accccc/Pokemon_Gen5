//==============================================================================
/**
 * @file    event_mission_normal.c
 * @brief   ミッション：通常会話
 * @author  matsuda
 * @date    2010.02.21(日)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"
#include "system/main.h"

#include "message.naix"

#include "script.h"
#include "event_fieldtalk.h"

#include "print/wordset.h"
#include "event_comm_talk.h"
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_comm_command.h"
#include "field/field_comm/bingo_system.h"
#include "field/field_comm/intrude_battle.h"
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
#include "event_mission_normal.h"

#include "../../../resource/fldmapdata/script/common_scr_def.h"

#include "poke_tool/status_rcv.h"



//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	EVENT_MISSION_TALK
//--------------------------------------------------------------
typedef struct
{
  COMMTALK_COMMON_EVENT_WORK ccew;
	BOOL error;

	INTRUDE_TALK_STATUS answer_status;
	INTRUDE_BATTLE_PARENT ibp;
}EVENT_MISSION_NORMAL;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT CommMissionNormal_Talk( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommMissionNormal_Talked( GMEVENT *event, int *seq, void *wk );


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * イベント切り替え：通常会話  (話しかけた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionNormal_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  EVENT_MISSION_NORMAL *talk;
  
  GMEVENT_Change(event, CommMissionNormal_Talk, sizeof(EVENT_MISSION_NORMAL));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
}

//--------------------------------------------------------------
/**
 * ミッション：話し掛けろ (自分がミッション実行者でターゲットに話しかけた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionNormal_Talk( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_NORMAL *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
  enum{
    SEQ_MENU_INIT,
    SEQ_MENU_WAIT,
    SEQ_MENU_SELECT_SEND,
    SEQ_MENU_SELECT_ANSWER_WAIT,
    SEQ_TALK_NG,
    SEQ_TALK_NG_MSGWAIT,
    SEQ_TALK_NG_LAST,
    SEQ_BATTLE_START,
    SEQ_ITEM_INIT,
    SEQ_ITEM_MSG_WAIT,
    SEQ_ITEM_LAST_WAIT,
    SEQ_TALK_CANCEL,
    SEQ_FINISH,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
  #if 0 //※check　後で作成
    if((*seq) < SEQ_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_talk_cancel);
      *seq = SEQ_MSG_WAIT;
      talk->error = TRUE;
    }
  #endif
  }

	switch( *seq ){
	case SEQ_MENU_INIT:
	  IntrudeEventPrint_SetupMainMenu(&talk->ccew.iem, gsys);
	  (*seq)++;
	  break;
	case SEQ_MENU_WAIT:
	  {
      u32 menu_ret = IntrudeEventPrint_SelectMenu(&talk->ccew.iem);
      switch(menu_ret){
      case FLDMENUFUNC_NULL:
        break;
      case FLDMENUFUNC_CANCEL:
      case INTRUDE_TALK_STATUS_CANCEL:
        talk->answer_status = INTRUDE_TALK_STATUS_CANCEL;
        IntrudeEventPrint_ExitMenu(&talk->ccew.iem);
        (*seq)++;
        break;
      default:
        talk->answer_status = menu_ret;
        IntrudeEventPrint_ExitMenu(&talk->ccew.iem);
        IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_talk_battle_002);
        (*seq)++;
        break;
      }
    }
    break;
  case SEQ_MENU_SELECT_SEND:  //メニュー選択結果を送信
    if(IntrudeSend_TalkAnswer(
        intcomm, intcomm->talk.talk_netid, talk->answer_status) == TRUE){
      if(talk->answer_status == INTRUDE_TALK_STATUS_CANCEL){
        *seq = SEQ_TALK_CANCEL;
      }
      else{
        (*seq)++;
      }
    }
	  break;
	case SEQ_MENU_SELECT_ANSWER_WAIT: //メニュー選択結果の返事待ち
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      INTRUDE_TALK_STATUS talk_status = Intrude_GetTalkAnswer(intcomm);
      OS_TPrintf("menu_answer status = %d\n", talk_status);
      switch(talk_status){
      case INTRUDE_TALK_STATUS_OK:
        switch(talk->answer_status){
        case INTRUDE_TALK_STATUS_BATTLE:
          *seq = SEQ_BATTLE_START;
          break;
        case INTRUDE_TALK_STATUS_ITEM:
          *seq = SEQ_ITEM_INIT;
          break;
        default:
          GF_ASSERT(0);
          break;
        }
        break;
      case INTRUDE_TALK_STATUS_NG:
      case INTRUDE_TALK_STATUS_CANCEL:
        *seq = SEQ_TALK_NG;
        break;
      default:  //まだ返事が来ていない
        if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
          if(IntrudeSend_TalkCancel(intcomm->talk.talk_netid) == TRUE){
            *seq = SEQ_TALK_CANCEL;
          }
        }
        break;
      }
    }
		break;
	  
  case SEQ_TALK_NG:   //断られた
    IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_002);
		(*seq)++;
    break;
  case SEQ_TALK_NG_MSGWAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
			(*seq)++;
		}
		break;
	case SEQ_TALK_NG_LAST:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
			(*seq) = SEQ_FINISH;
		}
	  break;
	
	case SEQ_BATTLE_START:
    IntrudeEventPrint_ExitFieldMsg(&talk->ccew.iem);
    {
      GMEVENT *child_event;
      
      talk->ibp.gsys = gsys;
      talk->ibp.target_netid = talk->ccew.talk_netid;
      child_event = EVENT_FieldSubProc(
        gsys, talk->ccew.fieldWork, NO_OVERLAY_ID, &IntrudeBattleProcData, &talk->ibp);
      GMEVENT_CallEvent(event, child_event);
    }
	  *seq = SEQ_FINISH;
	  break;
	
	case SEQ_ITEM_INIT:
    {
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
      MYSTATUS *target_myst = GAMEDATA_GetMyStatusPlayer(gamedata, talk->ccew.talk_netid);

      WORDSET_RegisterPlayerName( talk->ccew.iem.wordset, 0, target_myst );
      WORDSET_RegisterItemName( talk->ccew.iem.wordset, 1, ITEM_KIZUGUSURI );
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_talk_item_002);
    }
    *seq = SEQ_ITEM_MSG_WAIT;
    break;
  case SEQ_ITEM_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_ITEM_LAST_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_FINISH;
    }
    break;
	  
	case SEQ_TALK_CANCEL:   //会話キャンセル
	  *seq = SEQ_FINISH;
	  break;
	  
	case SEQ_FINISH:   //終了処理
  	EVENT_CommCommon_Finish(intcomm, &talk->ccew);
		return( GMEVENT_RES_FINISH );
	}
	
	return( GMEVENT_RES_CONTINUE );
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * イベント切り替え：通常会話  (話しかけられた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionNormal_Talked(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  EVENT_MISSION_NORMAL *talk;
  
  GMEVENT_Change(event, CommMissionNormal_Talked, sizeof(EVENT_MISSION_NORMAL));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
}

//--------------------------------------------------------------
/**
 * フィールド話し掛けイベント
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionNormal_Talked( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_NORMAL *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
  enum{
    SEQ_MSG_INIT,
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
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_talk_cancel);
      *seq = SEQ_MSG_WAIT;
      talk->error = TRUE;
    }
  #endif
  }
	
	switch( *seq ){
  case SEQ_MSG_INIT:
    IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_001);
		(*seq)++;
		break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
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
          if(IntrudeSend_TalkCancel(intcomm->talk.talk_netid) == TRUE){
            *seq = SEQ_TALK_CANCEL;
          }
        }
        break;
      }
    }
	  break;
	case SEQ_BATTLE_YESNO_INIT:
    IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_talk_battle_001);
	  (*seq)++;
	  break;
	case SEQ_BATTLE_YESNO_MSGWAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      IntrudeEventPrint_SetupYesNo(&talk->ccew.iem, gsys);
      (*seq)++;
    }
    break;
  case SEQ_BATTLE_YESNO_SELECT:
    {
      FLDMENUFUNC_YESNO yesno = IntrudeEventPrint_SelectYesNo(&talk->ccew.iem);
      switch(yesno){
      case FLDMENUFUNC_YESNO_YES:
        IntrudeEventPrint_ExitMenu(&talk->ccew.iem);
        *seq = SEQ_BATTLE_YES;
        break;
      case FLDMENUFUNC_YESNO_NO:
        IntrudeEventPrint_ExitMenu(&talk->ccew.iem);
        *seq = SEQ_NG;
        break;
      }
    }
    break;
  case SEQ_BATTLE_YES:
    if(IntrudeSend_TalkAnswer(
        intcomm, intcomm->talk.talk_netid, INTRUDE_TALK_STATUS_OK) == TRUE){
      *seq = SEQ_BATTLE_START;
    }
    break;
  case SEQ_BATTLE_START:
    IntrudeEventPrint_ExitFieldMsg(&talk->ccew.iem);
    {
      GMEVENT *child_event;
      
      talk->ibp.gsys = gsys;
      talk->ibp.target_netid = talk->ccew.talk_netid;
      child_event = EVENT_FieldSubProc(
        gsys, talk->ccew.fieldWork, NO_OVERLAY_ID, &IntrudeBattleProcData, &talk->ibp);
      GMEVENT_CallEvent(event, child_event);
    }
	  *seq = SEQ_FINISH;
    break;
    
  case SEQ_ITEM_YESNO_INIT:
    {
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
      MYSTATUS *target_myst = GAMEDATA_GetMyStatusPlayer(gamedata, talk->ccew.talk_netid);
      MYITEM_PTR myitem = GAMEDATA_GetMyItem( gamedata );

      MYITEM_AddItem( myitem, ITEM_KIZUGUSURI, 1, HEAPID_PROC );

      WORDSET_RegisterPlayerName( talk->ccew.iem.wordset, 0, target_myst );
      WORDSET_RegisterItemName( talk->ccew.iem.wordset, 1, ITEM_KIZUGUSURI );
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_talk_item_001);
    }
    *seq = SEQ_ITEM_MSG_WAIT;
    break;
  case SEQ_ITEM_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
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
        intcomm, intcomm->talk.talk_netid, INTRUDE_TALK_STATUS_NG) == TRUE){
      *seq = SEQ_FINISH;
    }
    break;
	
	case SEQ_TALK_CANCEL:
	  *seq = SEQ_FINISH;
	  break;
	  
	case SEQ_FINISH:
  	EVENT_CommCommon_Finish(intcomm, &talk->ccew);
		return( GMEVENT_RES_FINISH );
	}
	
	return( GMEVENT_RES_CONTINUE );
}


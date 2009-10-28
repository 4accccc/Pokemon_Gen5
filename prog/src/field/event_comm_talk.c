//==============================================================================
/**
 * @file    event_comm_talk.c
 * @brief   通信プレイヤーへの話しかけイベント
 * @author  matsuda
 * @date    2009.10.11(日)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "message.naix"
#include "msg/msg_intrude_comm.h"	//仮

#include "script.h"
#include "event_fieldtalk.h"

#include "print/wordset.h"
#include "event_comm_talk.h"
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_comm_command.h"
#include "field/field_comm/bingo_system.h"
#include "field/field_comm/intrude_mission.h"
#include "field/field_comm/intrude_message.h"
#include "msg/msg_invasion.h"

#include "../../../resource/fldmapdata/script/common_scr_def.h"


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
	BOOL error;
}COMMTALK_EVENT_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT CommTalkEvent( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommBingoEvent( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommMissionAchieveEvent( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommMissionResultEvent( GMEVENT *event, int *seq, void *wk );



//======================================================================
//	イベント：フィールド話し掛けイベント
//======================================================================
//==================================================================
/**
 * 通信プレイヤー話しかけイベント起動
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   intcomm		      侵入システムワークへのポインタ
 * @param   fmmdl_player		自機動作モデル
 * @param   talk_net_id		  話しかけ対象のNetID
 * @param   heap_id		      ヒープID
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_CommTalk(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id)
{
	COMMTALK_EVENT_WORK *ftalk_wk;
	GMEVENT *event;
	
	if(intcomm->intrude_status[talk_net_id].action_status == INTRUDE_ACTION_BINGO_BATTLE){
  	event = GMEVENT_Create(
  		gsys, NULL,	CommBingoEvent, sizeof(COMMTALK_EVENT_WORK) );
    Bingo_Clear_BingoBattleBeforeBuffer(Bingo_GetBingoSystemWork(intcomm));
  	//侵入システムのアクションステータスを更新
  	Intrude_SetActionStatus(intcomm, INTRUDE_ACTION_TALK);
    Intrude_InitTalkWork(intcomm, talk_net_id);
  }
  else if(MISSION_Talk_CheckAchieve(&intcomm->mission, talk_net_id) == TRUE){
  	event = GMEVENT_Create(
  		gsys, NULL,	CommMissionAchieveEvent, sizeof(COMMTALK_EVENT_WORK) );
  }
  else{
  	event = GMEVENT_Create(
  		gsys, NULL,	CommTalkEvent, sizeof(COMMTALK_EVENT_WORK) );
  	//侵入システムのアクションステータスを更新
  	Intrude_SetActionStatus(intcomm, INTRUDE_ACTION_TALK);
    Intrude_InitTalkWork(intcomm, talk_net_id);
  }
	ftalk_wk = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( ftalk_wk, sizeof(COMMTALK_EVENT_WORK) );
	
	ftalk_wk->heapID = heap_id;
	ftalk_wk->msgBG = FIELDMAP_GetFldMsgBG( fieldWork );
	ftalk_wk->fmmdl_player = fmmdl_player;
	ftalk_wk->fmmdl_talk = CommPlayer_GetMmdl(intcomm->cps, talk_net_id);
	ftalk_wk->fieldWork = fieldWork;
	ftalk_wk->intcomm = intcomm;
	
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
static GMEVENT_RESULT CommTalkEvent( GMEVENT *event, int *seq, void *wk )
{
	COMMTALK_EVENT_WORK *talk = wk;
	
	switch( *seq ){
	case 0:
		talk->msgData = FLDMSGBG_CreateMSGDATA(
			talk->msgBG, NARC_message_intrude_comm_dat );
		talk->msgWin = FLDMSGWIN_AddTalkWin( talk->msgBG, talk->msgData );
		FLDMSGWIN_Print( talk->msgWin, 0, 0, msg_intrude_000 );
		(*seq)++;
		break;
	case 1:
	  if(IntrudeSend_Talk(talk->intcomm->talk.talk_netid) == TRUE){
      (*seq)++;
    }
	case 2:
		if( FLDMSGWIN_CheckPrintTrans(talk->msgWin) == TRUE ){
			(*seq)++;
		}
		break;
	case 3:
	  {
      INTRUDE_TALK_STATUS talk_status = Intrude_GetTalkAnswer(talk->intcomm);
      switch(talk_status){
      case INTRUDE_TALK_STATUS_OK:
        *seq = 100;
        break;
      case INTRUDE_TALK_STATUS_NG:
        *seq = 200;
        break;
      }
    }
		break;
	
  case 100:   //返事OK
    FLDMSGWIN_ClearWindow(talk->msgWin);
		FLDMSGWIN_Print( talk->msgWin, 0, 0, msg_intrude_003 );
		(*seq)++;
    break;
  case 101:
		if( FLDMSGWIN_CheckPrintTrans(talk->msgWin) == TRUE ){
			(*seq)++;
		}
		break;
	case 102:
		{
			int trg = GFL_UI_KEY_GetTrg();
			if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
				(*seq) = 300;
			}
		}
	  break;
	  
  case 200:   //断られた
    FLDMSGWIN_ClearWindow(talk->msgWin);
		FLDMSGWIN_Print( talk->msgWin, 0, 0, msg_intrude_002 );
		(*seq)++;
    break;
  case 201:
		if( FLDMSGWIN_CheckPrintTrans(talk->msgWin) == TRUE ){
			(*seq)++;
		}
		break;
	case 202:
		{
			int trg = GFL_UI_KEY_GetTrg();
			if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
				(*seq) = 300;
			}
		}
	  break;
	
	case 300:   //終了処理
		FLDMSGWIN_Delete( talk->msgWin );
		GFL_MSG_Delete( talk->msgData );

  	//侵入システムのアクションステータスを更新
  	Intrude_SetActionStatus(talk->intcomm, INTRUDE_ACTION_FIELD);
    Intrude_InitTalkWork(talk->intcomm, INTRUDE_NETID_NULL);
		return( GMEVENT_RES_FINISH );
	}
	
	return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * ビンゴ乱入話し掛けイベント
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommBingoEvent( GMEVENT *event, int *seq, void *wk )
{
	COMMTALK_EVENT_WORK *talk = wk;
	
	switch( *seq ){
	case 0:
		talk->msgData = FLDMSGBG_CreateMSGDATA(
			talk->msgBG, NARC_message_intrude_comm_dat );
		talk->msgWin = FLDMSGWIN_AddTalkWin( talk->msgBG, talk->msgData );
		FLDMSGWIN_Print( talk->msgWin, 0, 0, msg_intrude_bingo000 );
		(*seq)++;
		break;
	case 1:
	  if(IntrudeSend_BingoIntrusion(talk->intcomm->talk.talk_netid) == TRUE){
      (*seq)++;
    }
	case 2:
		if( FLDMSGWIN_CheckPrintTrans(talk->msgWin) == TRUE ){
			(*seq)++;
		}
		break;
	case 3:
	  {
      BINGO_INTRUSION_ANSWER answer = Bingo_GetBingoIntrusionAnswer(talk->intcomm);
      switch(answer){
      case BINGO_INTRUSION_ANSWER_OK:
        *seq = 100;
        break;
      case BINGO_INTRUSION_ANSWER_NG:
        *seq = 200;
        break;
      }
    }
		break;
	
  case 100:   //返事OK
    FLDMSGWIN_ClearWindow(talk->msgWin);
   	FLDMSGWIN_Print( talk->msgWin, 0, 0, msg_intrude_bingo001 );
	  (*seq)++;
    break;
  case 101:   //乱入用パラメータ要求
    if(IntrudeSend_ReqBingoBattleIntrusionParam(talk->intcomm->talk.talk_netid) == TRUE){
      (*seq)++;
    }
    break;
  case 102:   //乱入用パラメータ受信待ち
    if(Bingo_GetBingoIntrusionParam(talk->intcomm) == TRUE){
      (*seq)++;
    }
    break;
  case 103:
		if( FLDMSGWIN_CheckPrintTrans(talk->msgWin) == TRUE ){
			(*seq)++;
		}
		break;
	case 104:
		{
			int trg = GFL_UI_KEY_GetTrg();
			if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
				(*seq) = 300;
			}
		}
	  break;
	  
  case 200:   //断られた
    FLDMSGWIN_ClearWindow(talk->msgWin);
		FLDMSGWIN_Print( talk->msgWin, 0, 0, msg_intrude_bingo002 );
		(*seq)++;
    break;
  case 201:
		if( FLDMSGWIN_CheckPrintTrans(talk->msgWin) == TRUE ){
			(*seq)++;
		}
		break;
	case 202:
		{
			int trg = GFL_UI_KEY_GetTrg();
			if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
				(*seq) = 300;
			}
		}
	  break;
	
	case 300:   //終了処理
		FLDMSGWIN_Delete( talk->msgWin );
		GFL_MSG_Delete( talk->msgData );

  	//侵入システムのアクションステータスを更新
  	Intrude_SetActionStatus(talk->intcomm, INTRUDE_ACTION_FIELD);
    Intrude_InitTalkWork(talk->intcomm, INTRUDE_NETID_NULL);
		return( GMEVENT_RES_FINISH );
	}
	
	return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * ミッション達成話し掛けイベント
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionAchieveEvent( GMEVENT *event, int *seq, void *wk )
{
	COMMTALK_EVENT_WORK *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_INIT,
    SEQ_SEND_ACHIEVE,
    SEQ_RECV_WAIT,
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if((*seq) > SEQ_INIT && (*seq) < SEQ_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->iem, msg_invasion_mission_sys002);
      *seq = SEQ_MSG_WAIT;
      talk->error = TRUE;
    }
  }

	switch( *seq ){
  case SEQ_INIT:
    IntrudeEventPrint_SetupFieldMsg(&talk->iem, gsys);
    (*seq)++;
    break;
	case SEQ_SEND_ACHIEVE:
    if(IntrudeSend_MissionAchieve(intcomm, &intcomm->mission) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_RECV_WAIT:
		if(MISSION_RecvAchieve(&intcomm->mission) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_MSG_INIT:
    {
      GAMEDATA *gdata = GAMESYSTEM_GetGameData(gsys);
      MYSTATUS *target_myst = GAMEDATA_GetMyStatusPlayer(gdata,intcomm->mission.data.target_netid);
      MYSTATUS *mine_myst = GAMEDATA_GetMyStatus(gdata);
      int my_netid = GAMEDATA_GetIntrudeMyID(gdata);
      u16 msg_id;
      
      msg_id = MISSION_GetAchieveMsgID(&intcomm->mission, my_netid);

      WORDSET_RegisterPlayerName( talk->iem.wordset, 0, target_myst );
      WORDSET_RegisterPlayerName( talk->iem.wordset, 1, mine_myst );

      IntrudeEventPrint_StartStream(&talk->iem, msg_id);
    }
		(*seq)++;
		break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
    IntrudeEventPrint_ExitFieldMsg(&talk->iem);
    if(intcomm != NULL){
      MISSION_Init(&intcomm->mission);
    }
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}


//==================================================================
/**
 * ミッション結果通知イベント起動(自分が達成ではなく通信相手が達成した結果が届いた)
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   intcomm		      侵入システムワークへのポインタ
 * @param   fmmdl_player		自機動作モデル
 * @param   heap_id		      ヒープID
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_CommMissionResult(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, HEAPID heap_id)
{
	COMMTALK_EVENT_WORK *ftalk_wk;
	GMEVENT *event;
	
	event = GMEVENT_Create(
 		gsys, NULL,	CommMissionResultEvent, sizeof(COMMTALK_EVENT_WORK) );

	ftalk_wk = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( ftalk_wk, sizeof(COMMTALK_EVENT_WORK) );
	
	ftalk_wk->heapID = heap_id;
	ftalk_wk->msgBG = FIELDMAP_GetFldMsgBG( fieldWork );
	ftalk_wk->fmmdl_player = fmmdl_player;
	ftalk_wk->fieldWork = fieldWork;
	ftalk_wk->intcomm = intcomm;
	
	return( event );
}

//--------------------------------------------------------------
/**
 * ミッション結果通知イベント(自分が達成ではなく通信相手が達成した結果が届いた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionResultEvent( GMEVENT *event, int *seq, void *wk )
{
	COMMTALK_EVENT_WORK *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_INIT,
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if((*seq) > SEQ_INIT && (*seq) < SEQ_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->iem, msg_invasion_mission_sys002);
      *seq = SEQ_MSG_WAIT;
      talk->error = TRUE;
    }
  }

	switch( *seq ){
  case SEQ_INIT:
    IntrudeEventPrint_SetupFieldMsg(&talk->iem, gsys);
    (*seq)++;
    break;
  case SEQ_MSG_INIT:
    {
      GAMEDATA *gdata = GAMESYSTEM_GetGameData(gsys);
      MYSTATUS *target_myst = GAMEDATA_GetMyStatusPlayer(gdata,intcomm->mission.data.target_netid);
      MYSTATUS *mine_myst = GAMEDATA_GetMyStatus(gdata);
      int my_netid = GAMEDATA_GetIntrudeMyID(gdata);
      u16 msg_id;
      
      msg_id = MISSION_GetAchieveMsgID(&intcomm->mission, my_netid);

      WORDSET_RegisterPlayerName( talk->iem.wordset, 0, target_myst );
      WORDSET_RegisterPlayerName( talk->iem.wordset, 1, mine_myst );

      IntrudeEventPrint_StartStream(&talk->iem, msg_id);
    }
		(*seq)++;
		break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
    IntrudeEventPrint_ExitFieldMsg(&talk->iem);
    if(intcomm != NULL){
      MISSION_Init(&intcomm->mission);
    }
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

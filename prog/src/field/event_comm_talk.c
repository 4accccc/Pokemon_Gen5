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

#include "event_comm_talk.h"
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_comm_command.h"
#include "field/field_comm/bingo_system.h"


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
}COMMTALK_EVENT_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT CommTalkEvent( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommBingoEvent( GMEVENT *event, int *seq, void *wk );



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
  }
  else{
  	event = GMEVENT_Create(
  		gsys, NULL,	CommTalkEvent, sizeof(COMMTALK_EVENT_WORK) );
  }
	ftalk_wk = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( ftalk_wk, sizeof(COMMTALK_EVENT_WORK) );
	
	ftalk_wk->heapID = heap_id;
	ftalk_wk->msgBG = FIELDMAP_GetFldMsgBG( fieldWork );
	ftalk_wk->fmmdl_player = fmmdl_player;
	ftalk_wk->fmmdl_talk = CommPlayer_GetMmdl(intcomm->cps, talk_net_id);
	ftalk_wk->fieldWork = fieldWork;
	ftalk_wk->intcomm = intcomm;
	
	//侵入システムのアクションステータスを更新
	Intrude_SetActionStatus(intcomm, INTRUDE_ACTION_TALK);
  Intrude_InitTalkWork(intcomm, talk_net_id);
	
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
	case 0:	//対象MDLの移動終了待ち
		if( talk->fmmdl_talk != NULL ){
			if( MMDL_CheckStatusBitMove(talk->fmmdl_talk) == TRUE ){
				MMDL_UpdateMove( talk->fmmdl_talk );
				
				if( MMDL_CheckStatusBitMove(talk->fmmdl_talk) == TRUE ){
					break;
				}
			}
		}
		
		{
			MMDLSYS *fmmdlsys = FIELDMAP_GetMMdlSys( talk->fieldWork );
			MMDLSYS_PauseMoveProc( fmmdlsys );
		}
		(*seq)++;
	case 1:
		talk->msgData = FLDMSGBG_CreateMSGDATA(
			talk->msgBG, NARC_message_intrude_comm_dat );
		talk->msgWin = FLDMSGWIN_AddTalkWin( talk->msgBG, talk->msgData );
		FLDMSGWIN_Print( talk->msgWin, 0, 0, msg_intrude_000 );
		(*seq)++;
		break;
	case 2:
	  if(IntrudeSend_Talk(talk->intcomm->talk.talk_netid) == TRUE){
      (*seq)++;
    }
	case 3:
		if( FLDMSGWIN_CheckPrintTrans(talk->msgWin) == TRUE ){
			(*seq)++;
		}
		break;
	case 4:
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

		{
			MMDLSYS *fmmdlsys = FIELDMAP_GetMMdlSys( talk->fieldWork );
			MMDLSYS_ClearPauseMoveProc( fmmdlsys );
		}

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
	case 0:	//対象MDLの移動終了待ち
		if( talk->fmmdl_talk != NULL ){
			if( MMDL_CheckStatusBitMove(talk->fmmdl_talk) == TRUE ){
				MMDL_UpdateMove( talk->fmmdl_talk );
				
				if( MMDL_CheckStatusBitMove(talk->fmmdl_talk) == TRUE ){
					break;
				}
			}
		}
		
		{
			MMDLSYS *fmmdlsys = FIELDMAP_GetMMdlSys( talk->fieldWork );
			MMDLSYS_PauseMoveProc( fmmdlsys );
		}
		(*seq)++;
	case 1:
		talk->msgData = FLDMSGBG_CreateMSGDATA(
			talk->msgBG, NARC_message_intrude_comm_dat );
		talk->msgWin = FLDMSGWIN_AddTalkWin( talk->msgBG, talk->msgData );
		FLDMSGWIN_Print( talk->msgWin, 0, 0, msg_intrude_bingo000 );
		(*seq)++;
		break;
	case 2:
	  if(IntrudeSend_BingoIntrusion(talk->intcomm->talk.talk_netid) == TRUE){
      (*seq)++;
    }
	case 3:
		if( FLDMSGWIN_CheckPrintTrans(talk->msgWin) == TRUE ){
			(*seq)++;
		}
		break;
	case 4:
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

		{
			MMDLSYS *fmmdlsys = FIELDMAP_GetMMdlSys( talk->fieldWork );
			MMDLSYS_ClearPauseMoveProc( fmmdlsys );
		}

  	//侵入システムのアクションステータスを更新
  	Intrude_SetActionStatus(talk->intcomm, INTRUDE_ACTION_FIELD);
    Intrude_InitTalkWork(talk->intcomm, INTRUDE_NETID_NULL);
		return( GMEVENT_RES_FINISH );
	}
	
	return( GMEVENT_RES_CONTINUE );
}


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
#include "msg/msg_intrude_comm.h"	//仮

#include "script.h"
#include "event_fieldtalk.h"

#include "event_comm_talked.h"
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_comm_command.h"


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
static GMEVENT_RESULT CommWasTalkedTo( GMEVENT *event, int *seq, void *wk );



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
	GMEVENT *event;
	
	event = GMEVENT_Create(
		gsys, NULL,	CommWasTalkedTo, sizeof(COMMTALK_EVENT_WORK) );
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
		FLDMSGWIN_Print( talk->msgWin, 0, 0, msg_intrude_001 );
		(*seq)++;
		break;
	case 2:
	  if(IntrudeSend_TalkAnswer(
	      talk->intcomm, talk->intcomm->talk.talk_netid, talk->intcomm->talk.talk_status) == TRUE){
      (*seq)++;
    }
    break;
  case 3:
		if( FLDMSGWIN_CheckPrintTrans(talk->msgWin) == TRUE ){
			(*seq)++;
		}
		break;
	case 4:
		{
			int trg = GFL_UI_KEY_GetTrg();
			if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
				(*seq)++;
			}
		}
		break;
	case 5:
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


//======================================================================
/**
 *
 * @file	event_fieldtalk.c
 * @brief	フィールド話し掛けイベント
 * @author	kagaya
 * @date	2009.01.22
 *
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "message.naix"
#include "msg/msg_d_field.h"	//仮

#include "script.h"
#include "event_fieldtalk.h"


#include "../../../resource/fldmapdata/script/common_scr_def.h"

extern MMDLSYS * FIELDMAP_GetMMdlSys( FIELDMAP_WORK *fieldWork );

//======================================================================
//	define
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	FLDTALK_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
	HEAPID heapID;
	u32 scr_id;
	MMDL *fmmdl_player;
	MMDL *fmmdl_talk;
	FIELDMAP_WORK *fieldWork;

	FLDMSGBG *msgBG;
	GFL_MSGDATA *msgData;
	FLDMSGWIN *msgWin;
}FLDTALK_EVENT_WORK;

//======================================================================
//	proto
//======================================================================
static GMEVENT_RESULT FldTalkEvent( GMEVENT *event, int *seq, void *wk );

//======================================================================
//	イベント：フィールド話し掛けイベント
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド話し掛けイベント起動
 * @param	GAMESYS_WORK
 * @param	fieldWork		FIELDMAP_WORK
 * @param	scr_id			スクリプトID
 * @param	fmmdl_player	自機動作モデル
 * @param	fmmdl_talk		話し掛け対象動作モデル
 * @param	heapID		HEAPID
 * @retval	GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_FieldTalk(
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, u32 scr_id,
		MMDL *fmmdl_player, MMDL *fmmdl_talk, HEAPID heapID )
{
	GMEVENT *event;
	event = SCRIPT_SetEventScript( gsys, scr_id, fmmdl_talk, heapID );
	return( event );
}

#if 0
GMEVENT * EVENT_FieldTalk(
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, u32 scr_id,
		MMDL *fmmdl_player, MMDL *fmmdl_talk, HEAPID heapID )
{
	FLDTALK_EVENT_WORK *ftalk_wk;
	GMEVENT *event;
	
	event = GMEVENT_Create(
		gsys, NULL,	FldTalkEvent, sizeof(FLDTALK_EVENT_WORK) );
	ftalk_wk = GMEVENT_GetEventWork( event );
	MI_CpuClear8( ftalk_wk, sizeof(FLDTALK_EVENT_WORK) );
	
	ftalk_wk->scr_id = scr_id;
	ftalk_wk->heapID = heapID;
	ftalk_wk->msgBG = FIELDMAP_GetFldMsgBG( fieldWork );
	ftalk_wk->fmmdl_player = fmmdl_player;
	ftalk_wk->fmmdl_talk = fmmdl_talk;
	ftalk_wk->fieldWork = fieldWork;
	
	return( event );
}
#endif

//--------------------------------------------------------------
/**
 * フィールド話し掛けイベント
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 */
//--------------------------------------------------------------
static GMEVENT_RESULT FldTalkEvent( GMEVENT *event, int *seq, void *wk )
{
	FLDTALK_EVENT_WORK *work = wk;
	
	switch( (*seq) ){
	case 0:	//対象MDLの移動終了待ち
		if( work->fmmdl_talk != NULL ){
			if( MMDL_CheckMoveBitMove(work->fmmdl_talk) == TRUE ){
				MMDL_UpdateMove( work->fmmdl_talk );
				
				if( MMDL_CheckMoveBitMove(work->fmmdl_talk) == TRUE ){
					break;
				}
			}
		}
		
		{
			MMDLSYS *fmmdlsys = FIELDMAP_GetMMdlSys( work->fieldWork );
			MMDLSYS_PauseMoveProc( fmmdlsys );
		}
		(*seq)++;
	case 1:
		work->msgData = FLDMSGBG_CreateMSGDATA(
			work->msgBG, NARC_message_d_field_dat );
		work->msgWin = FLDMSGWIN_AddTalkWin( work->msgBG, work->msgData );
		FLDMSGWIN_Print( work->msgWin, 0, 0, DEBUG_FIELD_STR01 );
		(*seq)++;
		break;
	case 2:
		if( FLDMSGWIN_CheckPrintTrans(work->msgWin) == TRUE ){
			(*seq)++;
		}
		break;
	case 3:
		{
			int trg = GFL_UI_KEY_GetTrg();
			if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
				(*seq)++;
			}
		}
		break;
	case 4:
		FLDMSGWIN_Delete( work->msgWin );
		GFL_MSG_Delete( work->msgData );

		{
			MMDLSYS *fmmdlsys = FIELDMAP_GetMMdlSys( work->fieldWork );
			MMDLSYS_ClearPauseMoveProc( fmmdlsys );
		}
		return( GMEVENT_RES_FINISH );
	}
	
	return( GMEVENT_RES_CONTINUE );
}


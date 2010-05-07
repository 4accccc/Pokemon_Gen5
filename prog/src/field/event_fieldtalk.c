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

#include "script.h"
#include "event_fieldtalk.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

//======================================================================
//	proto
//======================================================================

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

//============================================================================================
/**
 * @file	event_debug_menu.c
 * @brief	フィールドデバッグメニューの制御イベント
 * @date	2008.11.13
 *
 */
//============================================================================================
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "field_main.h"
#include "field_debug.h"
//======================================================================
//	debug
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
	DEBUG_FLDMENU *d_menu;
	FIELD_MAIN_WORK * fieldWork;
	HEAPID heapID;
	u16 page_id;
}DEBUG_MENU_EVENT_WORK;
//--------------------------------------------------------------
///	イベント：デバッグメニュー処理
//--------------------------------------------------------------
static GMEVENT_RESULT DebugMenuEvent(GMEVENT_CONTROL * event, int * seq, void * work)
{
	DEBUG_MENU_EVENT_WORK * dmew = work;
	switch (*seq) {
	case 0:
		dmew->d_menu = FldDebugMenu_Init(
				dmew->fieldWork, dmew->page_id, dmew->heapID );
		++ *seq;
		break;
	case 1:
		FldDebugMenu_Create( dmew->d_menu );
		++ *seq;
		break;
	case 2:
		if( FldDebugMenu_Main(dmew->d_menu) == TRUE ){
			++ *seq;
		}
		break;
	case 3:
		FldDebugMenu_Delete(dmew->d_menu);
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
void DEBUG_EVENT_DebugMenu(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldWork, HEAPID heapID, u16 page_id)
{
	DEBUG_MENU_EVENT_WORK * dmew;
	GMEVENT_CONTROL * event;
	event = GAMESYSTEM_EVENT_Set(gsys, DebugMenuEvent, sizeof(DEBUG_MENU_EVENT_WORK));
	dmew = GMEVENT_GetEventWork(event);
	dmew->d_menu = NULL;
	dmew->fieldWork = fieldWork;
	dmew->heapID = heapID;
	dmew->page_id = page_id;
}


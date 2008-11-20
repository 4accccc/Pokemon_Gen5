//======================================================================
/**
 * @file	event_debug_menu.h
 * @brief	フィールドデバッグメニューの制御イベント
 * @author	kagaya
 * @date	2008.11.13
 */
//======================================================================
#ifndef __FIELD_DEBUG_H__
#define __FIELD_DEBUG_H__

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field_main.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	DEBUG_FLDMENU
//--------------------------------------------------------------
//typedef struct _TAG_DEBUG_FLDMENU DEBUG_FLDMENU;
typedef struct _TAG_DEBUG_MENU_EVENT_WORK DEBUG_MENU_EVENT_WORK;

//======================================================================
//	extern
//======================================================================
extern GMEVENT * DEBUG_EVENT_DebugMenu( GAMESYS_WORK * gsys,
		FIELD_MAIN_WORK * fieldWork, HEAPID heapID, u16 page_id );

extern void FldDebugMenu_SetCommSystem(
	void *commSys , DEBUG_MENU_EVENT_WORK *d_menu );

#endif //__FIELD_DEBUG_H__

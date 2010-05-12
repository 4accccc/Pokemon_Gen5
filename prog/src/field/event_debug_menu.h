//======================================================================
/**
 * @file	event_debug_menu.h
 * @brief	フィールドデバッグメニューの制御イベント
 * @author	kagaya
 * @date	2008.11.13
 */
//======================================================================
#pragma once

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap_proc.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================

//======================================================================
//	extern
//======================================================================
extern GMEVENT * DEBUG_EVENT_DebugMenu( GAMESYS_WORK * gsys,
		FIELDMAP_WORK * fieldWork, HEAPID heapID, u16 page_id );

extern GMEVENT * EVENT_DEBUG_AllMapCheck( GAMESYS_WORK * gsys, BOOL up_to_flag );

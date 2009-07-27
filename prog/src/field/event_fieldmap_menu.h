//======================================================================
/**
 * @file	event_debug_menu.h
 * @brief	フィールドデバッグメニューの制御イベント
 * @author	kagaya
 * @date	2008.11.13
 */
//======================================================================
#ifndef __EVENT_FIELDMAP_MENU_H__
#define	__EVENT_FIELDMAP_MENU_H__

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
extern GMEVENT * EVENT_FieldMapMenu(
	GAMESYS_WORK *gsys, FIELD_MAIN_WORK *fieldWork, HEAPID heapID );
extern GMEVENT * EVENT_UnionMapMenu(
  GAMESYS_WORK *gsys, FIELD_MAIN_WORK *fieldWork, HEAPID heapID );

#endif //__EVENT_FIELDMAP_MENU_H__

//======================================================================
/**
 *
 * @file	field_debug.h
 * @brief	フィールドデバッグ
 * @author	kagaya
 * @data	08.09.29
 */
//======================================================================
#ifndef __FIELD_DEBUG_H__
#define __FIELD_DEBUG_H__

#include "field_main.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
typedef struct _TAG_DEBUG_FLDMENU DEBUG_FLDMENU;

//======================================================================
//	extern
//======================================================================
extern DEBUG_FLDMENU * FldDebugMenu_Init(
		FIELD_MAIN_WORK *fieldWork, u32 menu_num, u32 heapID );
extern void FldDebugMenu_Delete( DEBUG_FLDMENU *d_menu );
extern void FldDebugMenu_Create( DEBUG_FLDMENU *d_menu );
extern BOOL FldDebugMenu_Main( DEBUG_FLDMENU *d_menu );

extern GMEVENT * DEBUG_EVENT_DebugMenu(GAMESYS_WORK * gsys,
		FIELD_MAIN_WORK * fieldWork, HEAPID heapID, u16 page_id);

#endif //__FIELD_DEBUG_H__

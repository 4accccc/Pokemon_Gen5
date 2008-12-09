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
#include "system/bmp_menulist.h"
#include "fieldmap_local.h"

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
#include "system/bmp_menulist.h"
typedef struct _TAG_DEBUG_MENU_EVENT_WORK DEBUG_MENU_EVENT_WORK;
typedef struct _TAG_DMENU_COMMON_WORK DMENU_COMMON_WORK;
typedef struct _TAG_DEBUG_MENU_LIST DEBUG_MENU_LIST;
//======================================================================
//	extern
//======================================================================
extern GMEVENT * DEBUG_EVENT_DebugMenu( GAMESYS_WORK * gsys,
		FIELD_MAIN_WORK * fieldWork, HEAPID heapID, u16 page_id );

extern void FldDebugMenu_SetCommSystem(
	void *commSys , DEBUG_MENU_EVENT_WORK *d_menu );

extern void DebugMenu_InitCommonMenu(
			DMENU_COMMON_WORK *work,
			const BMPMENULIST_HEADER *pMenuHead,
			BMP_MENULIST_DATA *pMenuListData,
			const DEBUG_MENU_LIST *pMenuList, int menuCount,
			int bmpsize_x, int bmpsize_y,
			int arcDatIDmsg, HEAPID heapID );
extern void DebugMenu_DeleteCommonMenu( DMENU_COMMON_WORK *work );
extern u32 DebugMenu_ProcCommonMenu( DMENU_COMMON_WORK *work );
extern const int DebugMenu_GetWorkSize(void);


#endif //__FIELD_DEBUG_H__

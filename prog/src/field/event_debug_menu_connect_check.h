//======================================================================
/**
 * @file	 event_debug_menu_connect_check.h
 * @brief	 デバッグメニュー『接続チェック』
 * @author obata
 * @date	 2010.05.01
 */
//======================================================================
#ifdef  PM_DEBUG
#pragma once 
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

FS_EXTERN_OVERLAY( debug_connect_check ); 

extern GMEVENT * DEBUG_EVENT_DebugMenu_ConnectCheck( GAMESYS_WORK * gsys, void* wk );

#endif // PM_DEBUG

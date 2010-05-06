#ifdef PM_DEBUG 
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief デバッグメニュー『フィールド技』
 * @file event_debug_menu_fskill.h
 * @author obata
 * @date   2010.05.06
 *
 * ※event_debug_menu.c より移動
 */
///////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "gamesystem/gamesystem.h"

FS_EXTERN_OVERLAY( debug_fskill ); 

//--------------------------------------------------------------
/**
 * イベント：フィールド技
 * @param gsys
 * @param wk
 * @return GMEVENT*
 */
//--------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_DebugMenu_Fskill( GAMESYS_WORK *gsys, void* wk );

#endif // PM_DEBUG 


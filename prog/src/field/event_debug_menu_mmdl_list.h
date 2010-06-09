#ifdef PM_DEBUG 
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief デバッグメニュー『MMDL LIST』
 * @file event_debug_menu_mmdl_list.h
 * @author obata
 * @date   2010.05.06
 *
 * ※event_debug_menu.c より移動
 */
///////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "gamesystem/gamesystem.h"

FS_EXTERN_OVERLAY( debug_menu_mmdl_list ); 

//--------------------------------------------------------------
/**
 * イベント：MMDL LIST
 * @param gsys
 * @param wk
 * @return GMEVENT*
 */
//--------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_DebugMenu_MmdlList( GAMESYS_WORK *gsys, void* wk );

extern GMEVENT * DEBUG_EVENT_DebugMenu_DisguiseList( GAMESYS_WORK *gsys, void* wk );
#endif // PM_DEBUG 

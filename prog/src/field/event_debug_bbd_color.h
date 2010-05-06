#ifdef PM_DEBUG 
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief デバッグメニュー『ビルボード色』
 * @file event_debug_menu_bbd_color.h
 * @author obata
 * @date   2010.05.06
 *
 * ※event_debug_menu.c より移動
 */
///////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "gamesystem/gamesystem.h"

FS_EXTERN_OVERLAY( debug_bbd_color ); 

//--------------------------------------------------------------
/**
 * イベント：ビルボード色
 * @param gsys
 * @param wk
 * @return GMEVENT*
 */
//--------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_DebugMenu_BBD_Color( GAMESYS_WORK *gsys, void* wk );

#endif // PM_DEBUG 

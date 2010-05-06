#ifdef PM_DEBUG 
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief デバッグメニュー『ポケモン作成』
 * @file event_debug_menu_make_poke.h
 * @author obata
 * @date   2010.05.06
 *
 * ※event_debug_menu.c より移動
 */
///////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "gamesystem/gamesystem.h"

FS_EXTERN_OVERLAY( debug_menu_makepoke ); 

//--------------------------------------------------------------
/**
 * イベント：ポケモン作成
 * @param gsys
 * @param wk
 * @return GMEVENT*
 */
//--------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_DebugMenu_MakePoke( GAMESYS_WORK *gsys, void* wk );

#endif // PM_DEBUG 


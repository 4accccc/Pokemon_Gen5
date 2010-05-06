#ifdef PM_DEBUG 
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief デバッグメニュー『ふしぎデータ作成』
 * @file event_debug_menu_mystery_card.h
 * @author obata
 * @date   2010.05.06
 *
 * ※event_debug_menu.c より移動
 */
///////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "gamesystem/gamesystem.h"

FS_EXTERN_OVERLAY( debug_mystery_card ); 

//--------------------------------------------------------------
/**
 * イベント：フィールド技
 * @param gsys
 * @param wk
 * @return GMEVENT*
 */
//--------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_DebugMenu_MakeMysteryCard( GAMESYS_WORK *gsys, void* wk );

#endif // PM_DEBUG 



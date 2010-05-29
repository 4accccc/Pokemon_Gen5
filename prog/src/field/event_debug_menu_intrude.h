//======================================================================
/**
 * @brief  デバッグメニュー「ハイリンク」
 * @file   event_debug_menu_make_egg.h
 * @author tamada GAMEFREAK inc.
 * @date   2010.05.29
 */
//======================================================================
#pragma once

#ifdef  PM_DEBUG
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h" 

FS_EXTERN_OVERLAY( debug_menu_intrude );

//--------------------------------------------------------------
/**
 * @brief ハイリンクイベント生成
 *
 * @parma gameSystem
 * @param wk FIELDMAP_WORKへのポインタ
 */
//--------------------------------------------------------------
extern GMEVENT* DEBUG_EVENT_FLDMENU_Intrude( GAMESYS_WORK* gameSystem, void* wk );

#endif  //PM_DEBUG

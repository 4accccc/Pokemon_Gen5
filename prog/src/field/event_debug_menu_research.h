//======================================================================
/**
 * @brief  デバッグメニュー『すれ違い調査』
 * @file   event_debug_menu_research.c
 * @author obata
 * @date   2010.05.01
 */
//======================================================================
#ifdef  PM_DEBUG
#pragma once 
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

//--------------------------------------------------------------
/**
 * フィールドデバッグメニューイベント起動
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param heapID  HEAPID
 * @param page_id デバッグメニューページ
 * @retval  GMEVENT*
 */
//--------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_DebugMenu_Research( GAMESYS_WORK *gsys, void* wk );


#endif // PM_DEBUG

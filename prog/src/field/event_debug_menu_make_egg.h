////////////////////////////////////////////////////////////////
/**
 * @brief  デバッグメニュー「タマゴ作成」
 * @file   event_debug_menu_make_egg.h
 * @author obata
 * @date   2010.04.16
 */
////////////////////////////////////////////////////////////////
#pragma once
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h" 

FS_EXTERN_OVERLAY( debug_make_egg );

//--------------------------------------------------------------
/**
 * @brief タマゴ作成イベント生成
 *
 * @parma gameSystem
 * @param wk FIELDMAP_WORKへのポインタ
 */
//--------------------------------------------------------------
extern GMEVENT* DEBUG_EVENT_FLDMENU_MakeEgg( GAMESYS_WORK* gameSystem, void* wk );

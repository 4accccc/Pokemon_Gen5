//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		event_debug_menu_resort.h
 *	@brief  レゾナンスリゾート用デバッグメニュー
 *	@author Toru=Nagihashi
 *	@date		2011.02.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "gamesystem/gamesystem.h"

FS_EXTERN_OVERLAY( debug_resort ); 

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
extern GMEVENT * DEBUG_EVENT_DebugMenu_Resort( GAMESYS_WORK *gsys, void *wk );

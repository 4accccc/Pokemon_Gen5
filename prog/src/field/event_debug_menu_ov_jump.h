//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_jump.h
 *	@brief  デバッグメニュージャンプ系オーバーレイ領域
 *	@author	Toru=Nagihashi
 *	@date		2010.10.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "event_debug_local.h"
//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
extern BOOL debugMenuCallProc_DebugSkyJump( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_Jump( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_MapZoneSelect( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_MapSeasonSelect( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_DebugZoneJump( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_AllMapCheck( DEBUG_MENU_EVENT_WORK * p_wk );
extern BOOL debugMenuCallProc_AllConnectCheck( DEBUG_MENU_EVENT_WORK * p_wk );

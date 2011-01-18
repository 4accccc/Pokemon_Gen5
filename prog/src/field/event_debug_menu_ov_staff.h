//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_staff.h
 *	@brief  デバッグメニュースタッフ系オーバーレイ領域
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
extern BOOL debugMenuCallProc_PCScript( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenu_ControlShortCut( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_OpenGTSNegoMenu( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_Zukan( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_RingTone( DEBUG_MENU_EVENT_WORK * p_wk );
extern BOOL debugMenu_WfbcDebug( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_DebugTest_Ichi( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_DebugTest_Onoue( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_MedalBox( DEBUG_MENU_EVENT_WORK *wk );

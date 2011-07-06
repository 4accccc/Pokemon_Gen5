//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_system.h
 *	@brief  デバッグメニューシステム系オーバーレイ領域
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
extern BOOL debugMenuCallProc_NumInput( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_ResearchNumInput( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_RecordNumInput( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_GameEnd( DEBUG_MENU_EVENT_WORK * wk );
extern BOOL debugMenuCallProc_ForceSave( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_UseMemoryDump( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_FieldMemoryDumpStart( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_FieldMemoryDumpEnd( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_ControlRtcList( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_CaptureList( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_ChangePlayerSex( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_FadeSpeedChange( DEBUG_MENU_EVENT_WORK * wk );

//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_app.h
 *	@brief  デバッグメニューアプリ系オーバーレイ領域
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
extern BOOL debugMenuCallProc_UITemplate( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_Demo3d( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_Mobiclip( DEBUG_MENU_EVENT_WORK *p_wk );

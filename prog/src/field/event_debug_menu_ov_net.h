//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_net.h
 *	@brief  デバッグメニュー通信系オーバーレイ領域
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
extern BOOL debugMenuCallProc_OpenClubMenu( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_GPowerList( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_BeaconFriendCode( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_DebugSake( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_Geonet( DEBUG_MENU_EVENT_WORK * p_wk );
extern BOOL debugMenu_ClearWifiHistory( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenu_LiveComm( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenu_LinkFestival( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenu_MissionReset( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenu_Intrude( DEBUG_MENU_EVENT_WORK * wk );
extern BOOL debugMenu_WriteWifiNote( DEBUG_MENU_EVENT_WORK *wk );

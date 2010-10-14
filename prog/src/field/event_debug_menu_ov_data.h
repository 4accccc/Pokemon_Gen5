//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_data.h
 *	@brief  デバッグメニューデータ作成系オーバーレイ領域
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
extern BOOL debugMenuCallProc_DebugMakePoke( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_DebugReWritePoke( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_DebugItem( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_DebugSecretItem( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_DebugPDWItem( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_BoxMax( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_1BoxMax( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_MyItemMax( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_MakeEgg( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_Sodateya( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_DebugMakeUNData( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_MakeMysteryCardList( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_SetBtlBox( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_ChangeName( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_EventPokeCreate( DEBUG_MENU_EVENT_WORK * p_wk );
extern BOOL debugMenuCallProc_SymbolPokeList( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_Musical( DEBUG_MENU_EVENT_WORK * wk );
extern BOOL debugMenu_CreateMusicalShotData( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenu_CreateFeelingData( DEBUG_MENU_EVENT_WORK *wk );

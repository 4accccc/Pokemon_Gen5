//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_field.h
 *	@brief  デバッグメニューフィールド系オーバーレイ領域
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
extern BOOL debugMenuCallProc_FieldAveStress( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_FieldAveStressPalace( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_FieldPosData( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_FieldHitchCheckData( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_ControlLinerCamera( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_ControlDelicateCamera( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_EventFlagScript( DEBUG_MENU_EVENT_WORK *now_wk );
extern BOOL debugMenuCallProc_ScriptSelect( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_MMdlList( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_DisguiseList( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_ControlLight( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_WeatherList( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_ControlFog( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_SubscreenSelect( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_FieldSkillList( DEBUG_MENU_EVENT_WORK *p_wk );
extern BOOL debugMenuCallProc_DebugMvPokemon( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_BBDColor( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_EncEffList( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_Cutin( DEBUG_MENU_EVENT_WORK *wk );
extern BOOL debugMenuCallProc_ZoroaFlagClear( DEBUG_MENU_EVENT_WORK *wk );


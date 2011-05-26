//=============================================================================
/**
 *	@file		src\field\event_debug_menu_ov_pokewood.h
 *	@brief  デバッグメニューポケウッド系オーバーレイ領域
 *	@author	tamada GAMEFREAK inc.
 *	@date		2011.04.28
 *
 */
//=============================================================================
#pragma once
#include "app/pkwd_count.h"

//=============================================================================
//    外部公開
//=============================================================================
extern BOOL debugMenuCallProc_Pokewood( DEBUG_MENU_EVENT_WORK * wk );

extern PKWD_COUNT_PARAM * PKWD_COUNT_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys, u16 * res_work );
extern void PKWD_COUNT_PARAM_Delete( PKWD_COUNT_PARAM * pkwd_param );

#ifdef  PM_DEBUG
extern void PKWD_COUNT_PARAM_Setup_Debug( GAMESYS_WORK * gsys, PKWD_COUNT_PARAM * pkwd_param );
#endif
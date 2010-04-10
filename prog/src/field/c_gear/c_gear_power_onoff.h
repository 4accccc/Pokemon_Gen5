//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		c_gear_power_onoff.h
 *	@brief  CGEAR　電源　ON　OFF
 *	@author	takahashi tomoya
 *	@date		2010.04.09
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif


#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"
#include "field/field_subscreen.h"

FS_EXTERN_OVERLAY(cgear_onoff);

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	電源ON　OFF 処理
//=====================================
typedef struct _CGEAR_POWER_ONOFF CGEAR_POWER_ONOFF;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// 生成　破棄
extern CGEAR_POWER_ONOFF* CGEAR_POWER_ONOFF_Create( FIELD_SUBSCREEN_WORK* p_subscrn,GAMESYS_WORK* p_gamesys, HEAPID heapID );
extern void CGEAR_POWER_ONOFF_Delete( CGEAR_POWER_ONOFF* p_sys );

// メイン動作
extern void CGEAR_POWER_ONOFF_Main( CGEAR_POWER_ONOFF* p_sys, BOOL active );


#ifdef _cplusplus
}	// extern "C"{
#endif




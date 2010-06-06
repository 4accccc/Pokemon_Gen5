//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		debug_wfbc.h
 *	@brief  WFBCオートデバック機能
 *	@author	tomoya takahashi
 *	@date		2010.05.24
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "fieldmap.h"



FS_EXTERN_OVERLAY(debug_menu_wfbc_check);


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

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

extern GMEVENT * DEBUG_WFBC_View( GAMESYS_WORK *gsys );

extern GMEVENT * DEBUG_WFBC_PeopleCheck(
    GAMESYS_WORK *gsys );

extern GMEVENT * DEBUG_WFBC_100SetUp(
    GAMESYS_WORK *gsys );


#ifdef _cplusplus
}	// extern "C"{
#endif




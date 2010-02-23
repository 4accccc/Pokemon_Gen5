//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fldeff_bubble.h
 *	@brief  深海　水泡エフェクト
 *	@author	tomoya takahashi
 *	@date		2010.02.23
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

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

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
extern void * FLDEFF_BUBBLE_Init( FLDEFF_CTRL* p_fectrl, HEAPID heapID );
extern void FLDEFF_BUBBLE_Delete( FLDEFF_CTRL* p_fectrl, void* p_work );

extern void FLDEFF_BUBBLE_SetMMdl(
    MMDL* p_mmdl, FLDEFF_CTRL* p_fectrl );

#ifdef _cplusplus
}	// extern "C"{
#endif




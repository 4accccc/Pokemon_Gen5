//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		rail_slipdown.h
 *	@brief  レール　滑り降り
 *	@author	tomoya takahashi
 *	@date		2010.04.12
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
#include "fldmmdl.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
FS_EXTERN_OVERLAY(field_d09_slipdown);

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	RAIL_SLIPDOWN_WORK
//=====================================
typedef struct _RAIL_SLIPDOWN_WORK RAIL_SLIPDOWN_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


// TCBで動かす。
extern RAIL_SLIPDOWN_WORK* RailSlipDown_Create(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, MMDL* mmdl, BOOL jiki);
extern void RailSlipDown_Delete( RAIL_SLIPDOWN_WORK* p_wk );
extern BOOL RailSlipDown_IsEnd( const RAIL_SLIPDOWN_WORK* cp_wk );

#ifdef _cplusplus
}	// extern "C"{
#endif




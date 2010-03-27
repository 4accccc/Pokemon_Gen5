//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		event_rail_slipdown.h
 *	@brief  チャンピオンマップ　レールマップ　ずり落ちイベント
 *	@author	tomoya takahashi
 *	@date		2009.10.16
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
//-------------------------------------
///	RAIL_SLIPDOWN_WORK
//=====================================
typedef struct _RAIL_SLIPDOWN_WORK RAIL_SLIPDOWN_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

extern GMEVENT* EVENT_RailSlipDown(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap);
extern GMEVENT* EVENT_RailSlipDownObj(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, MMDL* mmdl);


// TCBで動かす。
extern RAIL_SLIPDOWN_WORK* RailSlipDown_Create(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, MMDL* mmdl, BOOL jiki);
extern void RailSlipDown_Delete( RAIL_SLIPDOWN_WORK* p_wk );
extern BOOL RailSlipDown_IsEnd( const RAIL_SLIPDOWN_WORK* cp_wk );


#ifdef _cplusplus
}	// extern "C"{
#endif




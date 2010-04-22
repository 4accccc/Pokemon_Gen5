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

extern GMEVENT* EVENT_RailSlipDown(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, u16 dir);
extern GMEVENT* EVENT_RailSlipDownObj(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, MMDL* mmdl);



#ifdef _cplusplus
}	// extern "C"{
#endif



